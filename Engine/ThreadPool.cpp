/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "ThreadPool.h"

#include <string>
#include <sstream>

#include <QtCore/QAtomicInt>
#include <QtCore/QMutex>
#include <QtCore/QThread>

#include "Engine/AbortableRenderInfo.h"
#include "Engine/Node.h"

NATRON_NAMESPACE_ENTER;


// We patched Qt to be able to derive QThreadPool to control the threads that are spawned to improve performances
// of the EffectInstance::aborted() function
#ifdef QT_CUSTOM_THREADPOOL

struct AbortableThreadPrivate
{
    mutable QMutex abortInfoMutex;
    bool isRenderResponseToUserInteraction;
    AbortableRenderInfoPtr abortInfo;
    EffectInstWPtr treeRoot;
    bool abortInfoValid;
    QThread* thread;
    std::string threadName;
    std::string currentActionName;
    NodeWPtr currentActionNode;

    AbortableThreadPrivate(QThread* thread)
        : abortInfoMutex()
        , isRenderResponseToUserInteraction(false)
        , abortInfo()
        , treeRoot()
        , abortInfoValid(false)
        , thread(thread)
        , threadName()
        , currentActionName()
        , currentActionNode()
    {
    }
};

AbortableThread::AbortableThread(QThread* thread)
    : _imp( new AbortableThreadPrivate(thread) )
{
}

AbortableThread::~AbortableThread()
{
}

void
AbortableThread::setThreadName(const std::string& threadName)
{
    std::stringstream ss;

    ss << threadName << " (" << this << ")";
    _imp->threadName = ss.str();
    _imp->thread->setObjectName( QString::fromUtf8( _imp->threadName.c_str() ) );
}

const std::string&
AbortableThread::getThreadName() const
{
    return _imp->threadName;
}

void
AbortableThread::setCurrentActionInfos(const std::string& actionName,const NodePtr& node)
{
    assert(QThread::currentThread() == _imp->thread);

    QMutexLocker k(&_imp->abortInfoMutex);
    _imp->currentActionName = actionName;
    _imp->currentActionNode = node;
}

void
AbortableThread::getCurrentActionInfos(std::string* actionName,
                                       NodePtr* node) const
{
    QMutexLocker k(&_imp->abortInfoMutex);

    *actionName = _imp->currentActionName;
    *node = _imp->currentActionNode.lock();
}

void
AbortableThread::killThread()
{
    _imp->thread->terminate();
}

void
AbortableThread::setAbortInfo(bool isRenderResponseToUserInteraction,
                              const AbortableRenderInfoPtr& abortInfo,
                              const EffectInstPtr& treeRoot)
{
    {
        QMutexLocker k(&_imp->abortInfoMutex);
        _imp->isRenderResponseToUserInteraction = isRenderResponseToUserInteraction;
        _imp->abortInfo = abortInfo;
        _imp->treeRoot = treeRoot;
        _imp->abortInfoValid = true;
    }
    if (abortInfo) {
        abortInfo->registerThreadForRender(this);
    }
}

void
AbortableThread::clearAbortInfo()
{
    AbortableRenderInfoPtr abortInfo;
    {
        QMutexLocker k(&_imp->abortInfoMutex);
        abortInfo = _imp->abortInfo;
        _imp->abortInfo.reset();
        _imp->treeRoot.reset();
        _imp->abortInfoValid = false;
    }

    if (abortInfo) {
        abortInfo->unregisterThreadForRender(this);
    }
}

bool
AbortableThread::getAbortInfo(bool* isRenderResponseToUserInteraction,
                              AbortableRenderInfoPtr* abortInfo,
                              EffectInstPtr* treeRoot) const
{
    QMutexLocker k(&_imp->abortInfoMutex);

    if (!_imp->abortInfoValid) {
        return false;
    }
    *isRenderResponseToUserInteraction = _imp->isRenderResponseToUserInteraction;
    *abortInfo = _imp->abortInfo;
    *treeRoot = _imp->treeRoot.lock();

    return true;
}

NATRON_NAMESPACE_ANONYMOUS_ENTER

class ThreadPoolThread
    : public QThreadPoolThread
      , public AbortableThread
{
public:

    ThreadPoolThread()
        : QThreadPoolThread()
        , AbortableThread(this)
    {
    }

    virtual ~ThreadPoolThread() {}
};

NATRON_NAMESPACE_ANONYMOUS_EXIT


ThreadPool::ThreadPool()
    : QThreadPool()
{
}

ThreadPool::~ThreadPool()
{
}

QThreadPoolThread*
ThreadPool::createThreadPoolThread() const
{
    ThreadPoolThread* ret = new ThreadPoolThread();

    ret->setThreadName("Global Thread (Pooled)");

    return ret;
}

#endif // ifdef QT_CUSTOM_THREADPOOL

NATRON_NAMESPACE_EXIT;

