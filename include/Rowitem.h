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

#ifndef ROWITEM_H
#define ROWITEM_H
#include "Basics.h"
#include <string>

using namespace std;

class Rowitem : public Basics
{
    public:
        Rowitem(string text);
        virtual ~Rowitem();
        int typ;
        int index;
        string plain;
    protected:

    private:
};

#endif // ROWITEM_H
