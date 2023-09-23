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

#include "Second.h"
#include <unordered_map>

using namespace std;

Second::Second()
{
}

Second::~Second()
{
}
// group the items to (numbered) sentences
int Second::secondParse(vector<Item> v) {
    int ok = 0;
    sentences.clear();
    Sentence *s;
    int len = v.size();
    char t;
    int count = -1;
    s = new Sentence();
    int scount = 1;
    bool isHeader = true;
    int labelType = 0; // 0=no label, 1=ordinary,2 = inside, 3 = end
    int oldCondLevel = 0;
    int oldLT = 0;
    unordered_map<string, int> snames;
    try {
        for (int i = 0; i < len; ++i) {
            Item item = v[i];
            t = item.getTyp();
            if (t == 'c') {
                labelType = 1;
                if (isEndLabel(item.getName())) {labelType = 3;}
                s->addItem(item);
                oldCondLevel = item.getCondLevel();
                oldLT = labelType;
            } else if (t == 'l') {
                if (s->getName().size() > 0) {
                    log.msg(WARN, "more than one paragraph name found - new names will be ignored.");
                } else {
                    s->setName(item.getName());
                }
                oldCondLevel = 0;
                oldLT = 0;
            } else if (t == 'v') {
               ++count;
               s->addItem(item);
               s->setSeq(count);
               s->setLabelType(labelType);
               s->setCondLevel(oldCondLevel);
               // if the last sentence was an end label, then decrement condlevel by 1
               if (oldLT == 3) {s->setCondLevel(oldCondLevel - 1);}
               vector<string> srefs = item.getSentenceRefnames();

               try {
                   if (srefs.size() > 1) {
                    log.msg(WARN, "you may only define one name for a sentence - extra names will be ignored.");
                   }

                   if (srefs.size() > 0) {
                        s->setRefname(srefs[0]);
                        snames[srefs[0]] = scount;
                   }
               } catch (exception& e) {
                    string s = e.what();
                    log.msg(FATAL, s + ": error when trying to set refname");
                    ok = -1;
                }
               if (isHeader) {s->setHeader();}
               s->setParams();
               sentences.push_back(*s);
               s = new Sentence();
               ++scount;
               labelType = changeLT(labelType, oldCondLevel);
            } else if (t == 'n') {
                if (i>0) {
                  if ((labelType == 1) || (labelType == 3)) {
                     ++count;
                     string x = (labelType == 1) ? "cond." : "nop.";
                     Item item = Item(x);
                     s->addItem(item);
                     s->setSeq(count);
                     s->setLabelType(labelType);
                     s->setCondLevel(oldCondLevel);
                     s->setParams();
                     sentences.push_back(*s);
                     s = new Sentence();
                     ++scount;
                     labelType = changeLT(labelType, oldCondLevel);
                  }
                  if (v[i-1].getTyp() == 'n') {
                     s = new Sentence();
                     ++scount;
                     count=-1;
                     isHeader = false;
                  }
                }
            } else if (t == 'r') {
                try {
                    if (item.getName() == "ref") {
                        string sref = item.getSentenceRef();
                        if(sref.size() > 0) {
                            if (snames.find(sref) != snames.end()) {
                                int dist = scount - snames[sref];
                                 if (dist <= 0) {
                                    log.msg(FATAL, "invalid ref. distance found for item " + item.content(true, true));
                                    ok = -1;
                                } else {
                                    // refdist is Number of a's not true dist, therefore reduce by one
                                    item.setRefdistance(dist-1);
                                    if (dist == 1) {
                                        log.msg(FATAL, "you must not replace 'this' by a named sentence reference - " + sref);
                                        ok = -1;
                                    }
                                }
                            } else {
                                log.msg(FATAL, "sentence reference " + sref + " not found.");
                                ok = -1;
                            }
                        }
                    }
                    s->addItem(item);
                } catch (exception& e) {
                    string s = e.what();
                    log.msg(FATAL, s + ": error when trying to set ref");
                    ok = -1;
                }
            } else {
                s->addItem(item);
            }
        }
    } catch(exception &e) {
       string s = e.what();
       log.msg(FATAL, s + ": error when doing second parse");
       ok = -1;
    }
    return ok;
}

string Second::getContentString() {
    int len = sentences.size();
    string result = "";
    for (int i = 0; i < len; ++i) {
        result += sentences[i].getContentString(false, false) + "\n";
    }
    return result;
}

vector<Sentence> Second::getSentences() {
    return sentences;
}

int Second::changeLT(int labelType, int condLevel) {
    int result = labelType;
    if (labelType == 1) {
        result = 2;
    } else if (labelType == 3) {
        if (condLevel <= 1) {
            result = 0;
        } else {
            result = 2;
        }
    }

    return result;
}

bool Second::isEndLabel(string name) {
    bool result = name == "end";

    if (!result && name.size() >=4) {
        string s = name.substr(0,4);
        result = s == "end.";
    }
    return result;
}
