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

#include "Log.h"
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

Log::Log()
{
    //ctor
}

Log::~Log()
{
    //dtor
}

void Log::setFile(string name) {
    filename = name;
    fstream fs (filename, ios::out | ios::trunc);
    fs.close ();
}

void Log::setLevel(Level lv) {
    level = lv;
}

void Log::setUseConsole(bool uc) {
    useconsole = uc;
}
void Log::msg(Level lv, string text) {
    if (lv>=level) {
        if (useconsole) {cerr << datum() << " " << levelString(lv) << text << endl;}
        fstream fs;
        fs.open(filename,ios::app);
        if (fs.is_open()){
            fs << datum() << " " << levelString(lv) <<text << "\n";
        }
        fs.close(); //close the file object.
   }
}

void Log::plain(string text) {
    fstream fs;
    fs.open(filename,ios::app);
    if (fs.is_open()){
        fs <<text << "\n";
        fs.close();
    }
}

string Log::datum() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[20];

    sprintf(buffer, "%02d", ltm->tm_mday);
    string result = buffer;
    sprintf(buffer, "%02d", (ltm->tm_mon) + 1);
    result += ".";
    result += buffer;
    sprintf(buffer, "%02d", (ltm->tm_year) - 100);
    result += ".";
    result += buffer;
    sprintf(buffer, "%02d", (ltm->tm_hour));
    result += " ";
    result += buffer;
    sprintf(buffer, "%02d", (ltm->tm_min));
    result += ":";
    result += buffer;
    sprintf(buffer, "%02d", (ltm->tm_sec));
    result += ":";
    result += buffer;

    return result;

}

void Log::debug(string text) {
          msg(DEBUG, text);
}

void Log::debugtime(string text) {
          if (text == "@init") {
              tstart = chrono::high_resolution_clock::now();
          } else {
              auto tstop = chrono::high_resolution_clock::now();
              auto duration = chrono::duration_cast<chrono::microseconds>(tstop - tstart);
              msg(DEBUG, text + to_string(duration.count()/1000000.0));
          }
}

string Log::levelString(Level level) {
        string result = "";
        switch(level) {
            case DEBUG:
                result = "DEBUG - ";
                break;
            case INFO:
                result = "INFO - ";
                break;
            case WARN:
                result = "WARN - ";
                break;
            case WARNALL:
                result = "WARNALL - ";
                break;
            case ERROR:
                result = "ERROR - ";
                break;
            case FATAL:
                result = "FATAL - ";
                break;
            case TOTAL:
                result = "TOTAL - ";
        }
        return result;
}

Level Log::getLevel() {
    return level;
};
