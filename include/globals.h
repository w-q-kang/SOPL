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

#ifndef GLOBALS_H
#define GLOBALS_H
#include <string>
#include <vector>
using namespace std;

extern bool isExtended;
extern vector<string> refs;
extern vector<string> gwords;
extern vector<string> standard;
extern vector<string> other;
extern vector<string> vmods;
extern bool debug_L;
extern bool debug_P;
extern bool debug_I;
extern bool debug_b;
extern bool debug_c;
extern bool debug_i;
extern bool debug_j;
extern bool debug_r;
extern bool debug_s;
extern bool debug_t;
extern bool debug_x;
extern bool debug_y;
#endif // GLOBALS_H
