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

#ifndef ITEM_H
#define ITEM_H
#include <string>
#include <vector>
#include "Basics.h"
#include "Log.h"

using namespace std;

class Item: public Basics
{
    public:
        Item(string pname);
        virtual ~Item();
        vector<string> splitByType(string pname, char typ);
        string content(bool withType, bool withDiff);
        bool isRef(string s);
        char getTyp();
        string getName();
        int getRefdistance();
        void setRefdistance(int dist);
        vector<string> getParts();
        int countDots(string text);
        int getCondLevel();
        bool isStandardVerb();
        bool hasGetSize();
        bool hasForget();
        int getRefFrom();
        int getRefTo();
        int getVerbType();

        vector<string> getSentenceRefnames();
        vector<string> getTrueParts();
        string getSentenceRef();
        bool hasErrors();
    protected:

    private:
        bool isError;
        int verbType; // 0=no verb,1 =standard, 2 = other, 3 = Paragraph
        bool getSize;
        bool forget;
        char typ; // p=plain, r=ref, v=verb, l=Label, c = condition label, n = newline
        string entry; // full name
        string name; // true name without modifiers
        string sentenceref;
        int condLevel;
        int refdistance;
        vector<string> parts; //modifiers incl. true name
        int refFrom;
        int refTo;
        Log log;
//        static vector<string> refs;
//        static vector<string>  gwords;
//        // standard verbs, must not contain: plist, between, nop, stop
//        static vector<string> standard;
//        static vector<string> other;
//        static vector<string> vmods;

};

#endif // ITEM_H
