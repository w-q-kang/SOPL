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

#ifndef OPCODE_H
#define OPCODE_H
#include "Enums.h"
#include <string>

using namespace std;

class Opcode
{
    public:
        Codes op;
        int left;
        string sleft;
        unsigned int operands;
        Opcode();
        virtual ~Opcode();

    protected:

    private:
};

#endif // OPCODE_H
