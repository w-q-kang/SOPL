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

#ifndef ENUMS_H
#define ENUMS_H

// total is used for OPG
enum Level {TOTAL, DEBUG, INFO, WARNALL, WARN, ERROR, FATAL};
// codes for plist/pexpr execution
enum Codes {ROW, VAL, VAR, IDX, AND, OR, NOT, EQ, NEQ, GE, LE, GT, LT, ELVIS,
 PLUS, MINUS, TIMES, DIVIDE, ABS, POWER, MOD, DIV, MIN, MAX, OUT, DROP, SKIPZ, DEFAULT, UGET, WGET, CONCAT, LEN, END,
 SQRT, CEIL, FLOOR, ROUND, SIN, COS, TAN, PI, ARCSIN, ARCCOS, ARCTAN, EXP, LOG, LOG10, SINH, COSH, TANH, ISINT, ISNUM, UPPER, LOWER };
 // modes for plist execution
enum Modes {SINGLE, MULTI};
// note that verbs, refs and vmods are declared in Item
enum StopTypes {LOOP, PARAGRAPH, PROGRAM};
#endif // ENUMS_H
