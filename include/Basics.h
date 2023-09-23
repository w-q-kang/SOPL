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

#ifndef BASICS_H
#define BASICS_H
#include <string>
#include <vector>

using namespace std;

class Basics
{
    public:
        Basics();
        virtual ~Basics();
        bool findInString(char c, string s);
        bool contains(string s, vector<string> v);
        vector<string> split(string name, char c);
        vector<string> split2(string name, char c1, char c2);
        string head(string name);
        char last(string name);
        int isThat(string s);
        string join(vector<string> &v, string sep);
        string joinint(vector<int> &v, string sep);
        void addToVector(vector<string> &v, vector<string> &w);
        void addToVectorP(vector<string>* v, vector<string>* w);
        string format(string text);
        bool isInt(string text);
        bool testInt(string text);
        bool isLong(string text);
        bool isNumber(string text);
        bool testNum(string text);
        string ctos(char c);
        vector<string> readFile(string name, bool keepEmpty, bool keepEol, bool keepEof);
        vector<string>* readFileP(string name);
        bool writeFile(string filename, vector<string> &v);
        bool isEmpty(string text);
        string getDir(string s);
        bool isPath(string s); // checks if a string is a path (i.e. contains : or / or \)
        int countOcc(vector<string> &v, string search); // count occurences
        string cstos(const char *a);
        vector<string> stov(string s);
        string trimLeft(string text);
        string trimRight(string text);
        string toUpper(string text);
        string toLower(string text);
        bool existsFile(string filename);
    protected:

    private:
        string tformat;
        string dformat;
};

#endif // BASICS_H
