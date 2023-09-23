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

#include "Sentence.h"
#include <algorithm>

using namespace std;

Sentence::Sentence()
{
   header = false;
   saved = false;
   labelType = 0;
   seq =-1;
   name = "";
   condLevel = 0;
   jump = 1;
   refname = "";
}

Sentence::~Sentence()
{
    //dtor
}

void Sentence::addItem(Item item) {
    sentence.push_back(item);
}

Item Sentence::getItem(int index) {
    Item result = Item("");
    if (index < (int) sentence.size()) {
        result = sentence[index];
    }
    return result;
}

Item Sentence::getLastItem() {
    Item result = Item("");
    if (sentence.size()>0) {
        result = sentence[sentence.size() -1];
    }
    return result;
}

vector<Item> Sentence::getSentence() {
    return sentence;
}

void Sentence::setName(string s) {
    name = s;
};

string Sentence::getName() {
    return name;
};

void Sentence::setHeader() {
    header = true;
};

bool Sentence::isHeader() {
    return header;
};

void Sentence::setSaved() {
    saved = true;
};

bool Sentence::isSaved() {
    return saved;
};


void Sentence::setSeq(int s) {
    seq = s;
};

int Sentence::getSeq() {
    return seq;
};

void Sentence::setCondLevel(int s) {
    condLevel = s;
};

int Sentence::getCondLevel() {
    return condLevel;
};

void Sentence::setJump(int s) {
    jump = s;
};

int Sentence::getJump() {
    return jump;
};

void Sentence::setLabelType(int lt) {
    labelType = lt;
};

int Sentence::getLabeltype() {
    return labelType;
};

void Sentence::setRefname(string text) {
    refname = text;
}

string Sentence::getRefname() {
    return refname;
}

string Sentence::getContentString(bool withType, bool withDiff) {
    string result = to_string(seq) + " -";
    //if (header) {result += "Header ";}
    //result += name;
    result += (isSaved()) ? " S" : " N";
    result += " " + to_string(labelType);
    result += " (" + to_string(condLevel) + ")";
    result += " [" + to_string(jump) + "]";
    result += ": ";
    int len = sentence.size();
    for (int i = 0; i < len; ++i) {
        result += sentence[i].content(withType, withDiff) + " ";
    }
    return result;
};

// get all refdistances for tha*t
vector<int> Sentence::getThatRefs() {
    vector<int> result;
    int len = sentence.size();
    for (int i = 0; i < len; ++i) {
        Item item = sentence[i];
        int rd = item.getRefdistance();
        if (rd > 0) {
            // add if not found
            if (find(result.begin(), result.end(), rd) == result.end()) {
                result.push_back(rd);
            }
        }
    }
    return result;
}

void Sentence::setParams() {
    vector<Item> v;
    int len = sentence.size();
        for (int i = 0; i < len; ++i) {
            Item item = sentence[i];
            char typ = item.getTyp();
            if (typ == 'l') {
                // ignore
            } else if (typ == 'g') {
                if (item.getName() == "for") {
                    forparams = v;
                } else if (item.getName() == "use") {
                    useparams = v;
                } else if (item.getName() == "with") {
                    withparams = v;
                } else {
                    log.msg(ERROR, "unknown group name " + item.getName());
                }
                v.clear();
            } else if (typ == 'v') {
                doparams = v;
                v.clear();
            } else if ((typ == 'n') || (typ == 'l')) {
                // should not occur but can be ignored
            } else if (typ == 'c') {
                // not yet handled
            } else if (typ == 'p') {
                v.push_back(item);
            } else if (typ == 'r') {
                v.push_back(item);
            } else {
                string s = "";
                s.push_back(typ);
                log.msg(WARN, "item with unknown type " + s + " is ignored.");
            }
        }
}

int Sentence::getLength() {
    return sentence.size();
};
