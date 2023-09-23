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

#include "Item.h"
#include "globals.h"

using namespace std;

//ctor
Item::Item(string pname)
{
    isError = false;
    verbType = 0;
    // set name and typ
    entry = pname;
    name = pname;
    refdistance = 0;
    condLevel = 0;
    getSize = false;
    forget = false;
    string refname = pname;
    int len = refname.size();
    refFrom = 0;
    refTo = -1;
    if ((refname[0] == '|') && (refname.back() == '|')) {
        refname = pname.substr(1, len-2);
        getSize = true;
    }

    if ((last(pname) == '.') &&(pname.size() > 1)) {
        typ = 'v';
        name = head(pname);
    } else if ((last(pname) == ':')&&(pname.size() > 1)) {
        typ = 'l';
        name = head(pname);
    } else if ((last(pname) == ')') &&(pname.size() > 1)) {
        typ = 'c';
        name = head(pname);
        condLevel = countDots(name) + 1;
    } else if (pname == "<EOL>") {
        typ = 'n';
        name = pname;
    } else if (contains(pname, gwords)) {
        typ = 'g';
        name = pname;
    } else if (isRef(refname)) {
        typ = 'r';
        name = refname;
    } else {
        typ = 'p';
        name = pname;
    }

    // parts
    try {
        if (typ == 'r') {
            parts = split(refname, '~');
            if (!isRef(parts[0])) {
                typ = 'p';
            } else if ((parts[0] == "ref") && isExtended) {
                if (parts.size() <= 1) {
                    typ = 'p';
                } else {
                    name = parts[0];
                    parts.erase(parts.begin());
                    sentenceref = parts[0];
                    parts.erase(parts.begin());
                }
            } else {
                refdistance = isThat(parts[0]);
                name = parts[0];
                parts.erase(parts.begin());
            }
        } else if (typ == 'v') {
            parts = split(name, '-');
            name = parts[0];
            parts.erase(parts.begin());
            if ((contains(name, standard) && ((name != "freq") || isExtended))){
                verbType = 1;
            } else if (contains(name, other)){
                verbType = 2;
            } else {
                verbType = 3;
            }
        }
    } catch(exception &e) {
        string s = e.what();
        isError = true;
        log.msg(FATAL, s + ": error when lexical analyzing reference " + pname);
    }

    try {
        if (typ == 'r') {
            for (int i=0;i<(int)parts.size(); ++i) {
                if (parts[i] == "first") {
                    refTo = refFrom;
                } else if (parts[i] == "second") {
                    refFrom += 1;
                    refTo = refFrom;
                } else if (parts[i] == "last") {
                    refFrom = refTo;
                } else if (parts[i] == "head") {
                    refTo -= 1;
                } else if (parts[i] == "tail") {
                    refFrom += 1;
                } else {
                    isError = true;
                    log.msg(FATAL, "ref modifier " + parts[i] + " not recognized.");
                }
            }
        }
    } catch(exception &e) {
        string s = e.what();
        isError = true;
        log.msg(FATAL, s + ": error when lexical analyzing reference " + pname);
    }
    try {
        if (typ == 'v') {
            for (int i=0;i<(int)parts.size(); ++i) {
                string part = parts[i];
                if (part == "forget") {forget = true;}
                if (!contains(part, vmods) && ((part[0]!= '>') || !isExtended)) {
                    isError = true;
                    log.msg(FATAL, "verb modifier " + parts[i] + " not recognized.");
                }
            }
        }
    } catch(exception &e) {
        string s = e.what();
        isError = true;
        log.msg(FATAL, s + ": error when lexical analyzing reference " + pname);
    }

}

Item::~Item()
{
    //dtor
}

    bool Item::isRef(string s) {
        bool result = false;
        if (s[0] == '|') {
            result = true;
        } else if (findInString('~', s)) {
            result = true;// tentatively
        } else if (contains(s, refs)) {
            result = (s != "ref") || isExtended;
        } else if (isThat(s) > 0) {
            result = true;
        }
        return result;
    }

    //split the word
    vector<string> Item::splitByType(string pname, char sep) {
        vector<string> liste;
        string myname = pname;
        int len = pname.size();
        if (sep == '-') {myname = pname.substr(0, len - 1);}

        liste = split(myname, sep);
        return liste;
    }

    string Item::content(bool withType, bool withDiff) {
        string s = "";
        int len = parts.size();
        if (withType) {
           s+= typ;
           s+= "|";
        }
        if (withDiff && (refdistance > 0)) {s+="(" + to_string(refdistance) + ")";}
        s += name;

        if (len > 0) {
            for (int i = 0; i < len; ++i) {
                if (i>0) {s+= "&";}
                s += "+" + parts[i];
            }
        }
        if (getSize) {s += "+||";}
        if (typ == 'r') { s += "[" + to_string(refFrom) + ".." + to_string(refTo) + "]";}
        return s;
    }

    char Item::getTyp() {
        return typ;
    }

    string Item::getName() {
        return name;
    }

    int Item::getRefdistance() {
        return refdistance;
    };

    void Item::setRefdistance(int dist) {
        refdistance = dist;
    };

    vector<string> Item::getParts() {
        return parts;
    };

    vector<string> Item::getTrueParts() {
        vector<string> liste;
        for (int i = 0; i < (int) parts.size(); ++i) {
            string p = parts[i];
            if (p[0] != '>') {
                liste.push_back(p);
            }
        }
        return liste;
    };

    int Item::getCondLevel() {
        return condLevel;
    }

    int Item::countDots(string text) {
        int count = 0;
        for (int i = 0; i < (int) text.size(); ++i) {
            if (name[i] == '.') {++count;}
        }
        return count;

    }

    bool Item::isStandardVerb() {
        return verbType == 1;
    };

    bool Item::hasGetSize() {
        return getSize;
    }

     bool Item::hasForget() {
        return forget;
    }

    int Item::getVerbType() {
        return verbType;
    }

    int Item::getRefFrom() {
        return refFrom;
    }

    int Item::getRefTo() {
        return refTo;
    }

    vector<string> Item::getSentenceRefnames() {
        vector<string> result;
        for (int i = 0; i < (int) parts.size(); ++i) {
            string r = parts[i];
            if (r[0] == '>') {
                result.push_back(r.substr(1));
                break;
            }
        }
        return result;
    }


    string Item::getSentenceRef() {
        return sentenceref;
    }

    bool Item::hasErrors() {
        return isError;
    }
