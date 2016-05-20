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


#ifndef PLUGINACTIONSHORTCUT_H
#define PLUGINACTIONSHORTCUT_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/GlobalDefines.h"
#include "Global/KeySymbols.h"
NATRON_NAMESPACE_ENTER;

struct PluginActionShortcut
{
    std::string actionID;
    std::string actionLabel;

    KeyboardModifiers modifiers;
    Key key;

    PluginActionShortcut()
    : actionID()
    , actionLabel()
    , modifiers(eKeyboardModifierNone)
    , key(Key_Unknown)
    {

    }

    PluginActionShortcut(const std::string& id, const std::string& label, Key symbol = Key_Unknown, const KeyboardModifiers& mods = KeyboardModifiers(eKeyboardModifierNone))
    : actionID(id)
    , actionLabel(label)
    , modifiers(mods)
    , key(symbol)
    {}
};

NATRON_NAMESPACE_EXIT;

#endif // PLUGINACTIONSHORTCUT_H
