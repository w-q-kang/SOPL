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

#ifndef PARSER_H
#define PARSER_H

#include "Basics.h"
#include "Item.h"
#include "Log.h"
#include <string>
#include <vector>

using namespace std;

class Parser : public Basics
{
    public:
        Parser();
        virtual ~Parser();
        int setParse(vector<string> v);
        string getParserString();
        vector<Item> getParse();
    protected:

    private:
        vector<Item> parse;
        Log log;
};

#endif // PARSER_H
