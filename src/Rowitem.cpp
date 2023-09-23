/*
Copyright 2022-23 Wolfgang Hermsen

This file is part of the SOPL Interpreter.

    The SOPL Interpreter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The SOPL Interpreter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the SOPL Interpreter.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Rowitem.h"

Rowitem::Rowitem(string text)
{
    index = 0;
    plain = "";
    if (text == "#") {
        typ = 1;
    } else if (text == "#index") {
        typ = 5;
    } else if (text == "#all") {
        typ = 6;
    } else if (text == "#rend") {
        typ = 7;
    }else if (text.substr(0,2) == "#w") {
        typ = 3;
        if (isInt(text.substr(2))) {
            index = stoi(text.substr(2));
        } else {
            typ = 0;
        }
    } else if (text.substr(0,2) == "#r") {
        typ = 4;
        if (isInt(text.substr(2))) {
            index = stoi(text.substr(2));
        } else {
            typ = 0;
        }
    } else if (text.substr(0,1) == "#") {
        typ = 2;
        if (isInt(text.substr(1))) {
            index = stoi(text.substr(1));
        } else {
            typ = 0;
        }
    } else {
        typ = 0;
    }

    if (typ == 0) {plain = text;};
}

Rowitem::~Rowitem()
{
    //dtor
}
