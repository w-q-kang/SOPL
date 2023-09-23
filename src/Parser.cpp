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

#include "Parser.h"
#include "globals.h"

using namespace std;

Parser::Parser()
{
}

Parser::~Parser()
{
    //dtor
}

int Parser::setParse(vector<string> v) {
    int result = 0;
     try {
        parse.clear();
        string entry = "";
        int len = v.size();
        if (debug_P) {log.debug("parsing vector " + join(v, " "));};
        for (int i = 0; i < len; ++i) {
            entry = v[i];
            if (debug_i) {log.debug("Entry = " + entry);};
            Item item = Item(entry);
            parse.push_back(item);
            if (item.hasErrors()) {
                result = -1;
                break;
            }
        }
        // add two extra newlines at file end for easier subsequent analysis
        Item item = Item("<EOL>");
        parse.push_back(item);
        parse.push_back(item);
    } catch(exception &e) {
       string s = e.what();
       log.msg(FATAL, s + ": error when trying to execute first parse");
       result = -1;
    }
    return result;
}

string Parser::getParserString() {
        string result = "";
        int len = parse.size();

    try {
        for (int i = 0; i < len; ++i) {
            string name = parse[i].content(true, true);
            result += name + ";";
            if (i < len - 1) {
                if ((parse[i].getTyp() == 'n') && (parse[i+1].getTyp() != 'n')) {result += "\n";}
             }
        }
    } catch(exception &e) {
       string s = e.what();
       log.msg(FATAL, s + ": error when trying to execute first parse");
    }
    return result;
}

vector<Item> Parser::getParse() {
    return parse;
}
