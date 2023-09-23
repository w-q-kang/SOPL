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

#ifndef LOG_H
#define LOG_H
#include <string>
#include "Enums.h"
#include <chrono>

using namespace std;

class Log
{
    public:
        static bool useconsole;
        static string filename;
        static Level level;
        Log();
        virtual ~Log();
        void msg(Level lv, string text);
        void plain(string text);
        void debug(string text);
        void debugtime(string text);
        void setFile(string name);
        void setLevel(Level level);
        void setUseConsole(bool uc);
        string levelString(Level level);
        Level getLevel();
        string datum();
        chrono::high_resolution_clock::time_point tstart = chrono::high_resolution_clock::now();
    protected:

    private:

};

#endif // LOG_H
