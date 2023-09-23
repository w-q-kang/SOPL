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
    back
*/

#include "Interpreter.h"
#include "globals.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <math.h>
#include <filesystem>
#include <map>

// could
using namespace std;

Interpreter::Interpreter(string wdir, string cdir, bool pLogOutput)
{

    stopExecution = 0;
    logOutput = pLogOutput;
    groupNames = {"do", "for", "use", "with"};
    current = logg.getLevel();
    logg.msg(INFO, "Current log level is " + to_string(current));
    workDir = wdir;
    if (last(workDir) != '\\') {
        workDir.push_back('\\');
    }
    logg.msg(INFO, "Working directory is " + workDir);
    currDir = cdir;
    if (last(currDir) != '\\') {
        currDir.push_back('\\');
    }
    logg.msg(INFO, "Current directory is " + currDir);
}

Interpreter::~Interpreter()
{
}

void Interpreter::setParagraphs(vector<Paragraph> v) {
    paragraphs = v;
    setNames();
};

// should never return FATAL errors any more, since these will be detected when parsing
void Interpreter::setNames() {
    for (int i = 0; i < (int) paragraphs.size(); ++i) {
        Paragraph p = paragraphs[i];
        if (!p.isHeader()) {
            string name = p.getName();
            if (name.size() > 0) {
               if (names.find(name) == names.end()) {
                    names[name] = i;
               } else {
                    logg.msg(FATAL, "paragraph name " + name + " was used multiple times");
                    current = FATAL;
                    break;
                }
            } else {
                logg.msg(FATAL, "paragraph  " + to_string(i) + " is unnamed.");
                current = FATAL;
                break;
            }
        }
    };
}

int Interpreter::execute(vector<string> &pargs) {
    if (current == FATAL ) {return -1;}
    progargs = pargs;
    vector<string> thislist;
    vector<string> dolist;
    vector<string> forlist;
    vector<string> uselist;
    vector<string> withlist;
    execParagraph(0, thislist, dolist, forlist, uselist, withlist, 0);
    return 0;
}

string Interpreter::getNameList() {
    string result = "";
    for (auto & it : names) {
        result += it.first + " = " + to_string(it.second) + "; ";
    }
    return result;
}

vector<string> Interpreter::execParagraph(int index, vector<string> &pthislist, vector<string> &pdolist, vector<string> &pforlist, vector<string> &puselist, vector<string> &pwithlist,
                               int stackLevel) {
    string hint = "begin";
    if (stopExecution > 0) {
        stopExecution = newStopExecution(stopExecution, PARAGRAPH);
        return pthislist;
    }
    int offset=0;
    vector<string> totalrv;
    // needed as starting value for sentence loop
    vector<string> outthislist=pthislist;

    if (debug_c) {logg.debug("entering Paragraph " + to_string(index) + " on level " + to_string(stackLevel));};
    Paragraph p = paragraphs[index];
    unordered_map<int, vector<string>> slists; //sentence result lists, will change
    int scmax = p.getLength();
    int sc= 0;

    while (sc < scmax) {
        Sentence sentence = p.getSentence(sc);
        try {
            hint = "preprocess";
            vector<string> dolist;
            vector<string> forlist;
            vector<string> uselist;
            vector<string> withlist;
            hint = "do";
            if (sentence.doparams.size() > 0) {dolist = getTheParam(sentence.doparams, sc,
                outthislist, pdolist, pforlist, puselist, pwithlist, stackLevel, slists);}

            hint = "for";

            if (sentence.forparams.size() > 0) {forlist = getTheParam(sentence.forparams, sc,
                 outthislist, pdolist, pforlist, puselist, pwithlist,  stackLevel, slists);}
            hint = "use";

            if (sentence.useparams.size() > 0) {uselist = getTheParam(sentence.useparams, sc,
                 outthislist, pdolist, pforlist, puselist, pwithlist,  stackLevel, slists);}
            hint = "with";

            if (sentence.withparams.size() > 0) {withlist = getTheParam(sentence.withparams, sc,
                 outthislist, pdolist, pforlist, puselist, pwithlist,  stackLevel, slists);}

            hint = "execVerb";
            Item item = sentence.getLastItem();

            if (item.hasForget()) {
                //int before = mmemsize(slists) + vmemsize(pdolist) + vmemsize(pforlist) + vmemsize(puselist) + vmemsize(pwithlist) + vmemsize(pthislist) + vmemsize(totalrv);
                //logg.debug("c", "executing forget in paragraph " + to_string(index));
                slists.clear();
                pdolist.clear();pdolist.shrink_to_fit();
                pforlist.clear();pforlist.shrink_to_fit();
                puselist.clear();puselist.shrink_to_fit();
                pwithlist.clear();pwithlist.shrink_to_fit();
                //int after = mmemsize(slists) + vmemsize(pdolist) + vmemsize(pforlist) + vmemsize(puselist) + vmemsize(pwithlist) + vmemsize(pthislist) + vmemsize(totalrv);
                //logg.debug("c", "before: " + to_string(before) + ", after: " + to_string(after));
            }

            tie(offset, outthislist) = executeVerb(item, sentence.getJump(), outthislist, dolist, forlist, uselist, withlist, stackLevel);

            hint = "addRV";
            if (item.getName() == "returnValue") {addToVector(totalrv, outthislist);};

            hint = "isSaved";
            if (sentence.isSaved()) {
                slists[sentence.getSeq()] = outthislist;
            }
        } catch (exception& e) {
            string s(e.what());
            logg.msg(FATAL, s + " in sentence " + to_string(sc) + " of paragraph " + p.getName() + " at level " + to_string(stackLevel));
            logg.msg(FATAL, "error occured when executing " + sentence.getLastItem().getName() + " - hint: " + hint);
            if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->9");};
            stopExecution = 9;
        }

        if ((stopExecution >0) || (offset <= - 1000)) {break;}
        sc+= offset;
    }

    if (debug_c) {logg.debug("leaving Paragraph " + to_string(index));};
    stopExecution = newStopExecution(stopExecution, PARAGRAPH);
    return totalrv;
}

vector<string> Interpreter::getTheParam(vector<Item> &items, int sc, vector<string> &thislist, vector<string> &dolist, vector<string> &forlist, vector<string> &uselist, vector<string> &withlist,
           int stackLevel, unordered_map<int, vector<string>> &slists) {
    vector<string> result;
    string hint = "begin";

    try {
        vector<string> help;
        int len = items.size();

        for (int i = 0; i < len; ++i) {
            Item item = items[i];
            char typ = item.getTyp();
            hint = "item " + to_string(i) + " of typ " + ctos(typ);

            if (typ == 'p') {
                result.push_back(item.getName());
            } else if (typ == 'r') {
               help.clear();
               help = getTheReference(item, sc, thislist, dolist, forlist, uselist, withlist, slists);
               if (debug_r) {logg.debug("ref " + item.getName() + " evaluated to " + join(help, ","));};
               addToVector(result, help);
            } else if ((typ == 'l') || (typ == 'g') || (typ == 'n') || (typ == 'l')|| (typ == 'v')|| (typ == 'c')) {
                // ignore

            } else {
                string s = "";
                s.push_back(typ);
                logg.msg(WARN, "item with unknown type " + s + " is ignored.");
            }
        }
    } catch (exception& e) {
        logg.msg(FATAL, cstos(e.what()) + " in preprocess of sentence " + to_string(sc) + " at level " + to_string(stackLevel) + " - hint: " + hint);
        if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->9");};
        stopExecution = 9;
    }

    return result;
}

vector<string> Interpreter::getTheReference(Item item, int sc, vector<string> &thislist, vector<string> &dolist, vector<string> &forlist, vector<string> &uselist, vector<string> &withlist,
                             unordered_map<int, vector<string>> &slists) {
    vector<string> result;
    string name = item.getName();
    //string help = "";
    int refFrom = item.getRefFrom();
    int refTo = item.getRefTo();
    int ihelp = 0;
    try {
        // clumsy but (hopefully) faster
        if (item.hasGetSize()) {
            if (name == "this") {
                ihelp = thislist.size();
            } else if (name == "params") {
                ihelp = dolist.size();
            } else if (name == "forparams") {
                ihelp = forlist.size();
            } else if (name == "useparams") {
                ihelp = uselist.size();
            } else if (name == "withparams") {
                ihelp = withlist.size();
            } else if (name == "args") {
                ihelp = progargs.size();
            }else {
                int refsc = item.getRefdistance() + 1;
                refsc = sc - refsc;
                ihelp= getSlist(slists, refsc).size();
            }
            ihelp = ihelp + refTo - refFrom + 1;
            result.push_back(to_string(ihelp));
        // not modified => use original
        } else if ((refFrom == 0) && (refTo == -1)) {
            if (name == "this") {
                return thislist;
            } else if (name == "params") {
                return dolist;
            } else if (name == "forparams") {
                return forlist;
            } else if (name == "useparams") {
                return uselist;
            } else if (name == "withparams") {
                return withlist;
            } else if (name == "args") {
                return progargs;
            }else {
                int refsc = item.getRefdistance() + 1;
                refsc = sc - refsc;
                return getSlist(slists, refsc);
            }
          // index
        } else if (refFrom == refTo) {
            ihelp = refFrom;

            if (name == "this") {
                if (ihelp<0) {ihelp = thislist.size() + ihelp;}
                result.push_back(thislist.at(ihelp));
            } else if (name == "params") {
                if (ihelp<0) {ihelp = dolist.size() + ihelp;}
                result.push_back(dolist.at(ihelp));
            } else if (name == "forparams") {
                if (ihelp<0) {ihelp = forlist.size() + ihelp;}
                result.push_back(forlist.at(ihelp));
            } else if (name == "useparams") {
                if (ihelp<0) {ihelp = uselist.size() + ihelp;}
                result.push_back(uselist.at(ihelp));
            } else if (name == "withparams") {
                if (ihelp<0) {ihelp = withlist.size() + ihelp;}
                result.push_back(withlist.at(ihelp));
            } else if (name == "args") {
                if (ihelp<0) {ihelp = progargs.size() + ihelp;}
                result.push_back(progargs.at(ihelp));
            } else {
                int refsc = item.getRefdistance() + 1;
                refsc = sc - refsc;
                if (ihelp<0) {
                    ihelp = getSlist(slists, refsc).size() + ihelp;
                }
                result.push_back(getSlist(slists, refsc).at(ihelp));
            }
          // construct range here
        } else {
             if (name == "this") {
                result = thislist;
            } else if (name == "params") {
                result = dolist;
            } else if (name == "forparams") {
                result = forlist;
            } else if (name == "useparams") {
                result = uselist;
            } else if (name == "withparams") {
                result = withlist;
            } else if (name == "args") {
                result = progargs;
            } else {
                int refsc = item.getRefdistance() + 1;
                refsc = sc - refsc;
                result = getSlist(slists, refsc);
            }
            if (refFrom > 0) {
                result.erase(result.begin(), result.begin() + refFrom);
            }

            if (refTo < -1) {
                for (int i = 1; i < abs(refTo); ++i) {result.pop_back();}
            }
        }
    } catch (exception& e) {
        logg.msg(FATAL, cstos(e.what()) + ": when processing ref.+modifiers for " + name + "[" + to_string(refFrom) + ".." + to_string(refTo) + "]");
        if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->9");};
        stopExecution = 9;
    }

    return result;
};

vector<string> Interpreter::getSlist(unordered_map<int, vector<string>> &slists, int key) {
    if (slists.find(key) != slists.end()) {
        return slists[key];
    } else {
        logg.msg(FATAL, "could not find sentence reference at " + to_string(key));
        if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->9");};
        stopExecution = 9;
        return {};
    }
};

tuple<int, vector<string>> Interpreter::executeVerb(Item verb, int jump,
                    vector<string> &thislist, vector<string> &dolist, vector<string> &forlist, vector<string> &uselist, vector<string> &withlist,
                    int stackLevel) {
    string name = verb.getName();
    if (debug_s) {logg.debug("executing verb " + name);};
    int offset;
    vector<string> p;
    vector<string> modifiers = verb.getParts();
    string stype = "";
    int verbmode = 0;
    bool echoOut = false;

    for (int i = 0; i < (int) modifiers.size(); ++i) {
        if (modifiers[i] == "int") {
            stype = "long";
            if ((name != "plist")&& (name != "sort")&& (name != "pexpr")) {
                logg.msg(WARN, "modifier int is ignored for verb " + name + ".");
            }
        } else if (modifiers[i] == "float") {
            stype = "float";
            if ((name != "plist")&& (name != "pexpr")) {
                logg.msg(WARN, "modifier float is ignored for verb " + name + ".");
            }
        } else if ((modifiers[i] == "echoIn") || (modifiers[i] == "echo"))  {
            doOut(dolist, true);
            if (logOutput) {logg.plain(join(dolist, " "));}
        } else if (modifiers[i] == "perRow") {
            verbmode = 1;
        } else if (modifiers[i] == "combine") {
            verbmode = 2;
        }

        if ((modifiers[i] == "echoOut") || (modifiers[i] == "echo"))  {
            echoOut = true;
        }
    }

    if (verbmode == 1) {
         tie(offset, p) = executePerRowSentence3(name, verb.isStandardVerb(),jump, stype, thislist, dolist, forlist, uselist, withlist, stackLevel);
    } else if (verbmode == 2) {
         tie(offset, p) = executeCombineSentence(name, verb.isStandardVerb(),jump, stype, thislist, dolist, forlist, uselist, withlist, stackLevel);
    } else {
        tie(offset, p) = executePlainVerb(name, verb.isStandardVerb(), jump, stype, thislist, dolist, forlist, uselist, withlist, stackLevel);
    }

    //vector<string> w;

    if (echoOut)  {
        doOut(p, true);
        if (logOutput) {logg.plain(join(p, " "));}
    }

    return make_tuple(offset, p);
};

tuple<int, vector<string>> Interpreter::executePlainVerb(string name, bool isStandardVerb, int jump, string stype,
                    vector<string> &thislist, vector<string> &dolist, vector<string> &forlist, vector<string> &uselist, vector<string> &withlist,
                    int stackLevel) {
    vector<string> p;
    p.clear();
    int offset = jump;
    int newse = stopExecution;

    try {
        if (isStandardVerb) {
            tie(offset, p) = executeStandardVerb(name, stype, dolist, forlist, offset);
        } else if (name == "nop") {
            p = thislist;
        } else if (name == "stop") {
            offset = -1000;
            newse = 1;
            if (dolist.size() > 0) {
                if (dolist[0] == "program") {
                    newse = 9;
                } else if (dolist[0] == "paragraph") {
                    newse = 3;
                }
            }
            if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->" + to_string(newse));};
            stopExecution = newse;
            p = thislist;
        } else if (name == "between") {
            int len = dolist.size();
            int flen = forlist.size();
            if (flen==0) {
                p = dolist;
            } else if (len>0) {
                int dist = 1;
                int distcount = 0;
                if (uselist.size()>0) {
                    dist = stoi(uselist[0]);
                }
                for (int i = 0; i < len - 1; ++i) {
                    p.push_back(dolist[i]);
                    distcount++;
                    if (dist == distcount) {
                        for (int j = 0; j < flen; ++j) {
                            p.push_back(forlist[j]);
                        }
                        distcount=0;
                    }
                }
                p.push_back(dolist[len-1]);
            }
        } else if (name == "plist") {
            if (forlist.size() == 0) {
               logg.msg(WARN, "initial forlist is empty for verb plist.");
            }
            if (stype == "long") {
                p = bulkExecLong(MULTI, dolist, forlist, uselist, withlist);
            } else if (stype == "float") {
                p = bulkExecFloat(MULTI, dolist, forlist, uselist, withlist);
            }else {
                p = bulkExecString(MULTI, dolist, forlist, uselist, withlist);
            }
        } else {
            int index = names[name];
            p = execParagraph(index, thislist, dolist, forlist, uselist, withlist, stackLevel + 1);
            if (debug_c) {logg.debug("RV=" + join(p, " "));};
        }
    } catch(exception &e) {
      logg.msg(FATAL, cstos(e.what()) + ": error when executing " + name + " for " + join(dolist, " "));
      if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->9");};
      stopExecution = 9;
    }

    // warn for std, but not returnValue
   if (isStandardVerb && (p.size() == 0)) {
       // nowarnall for returnValue, include
      if ((name != "returnValue") && (name != "include")) {
        logg.msg(WARNALL, "the output of standard verb " + name + " is empty.");
        logg.msg(WARNALL, "input dolist was " + join(dolist, " "));
        logg.msg(WARNALL, "input forlist was " + join(forlist, " "));
      }
   }

   return make_tuple(offset, p);
}

tuple<int, vector<string>> Interpreter::executeStandardVerb(string name, string stype,
                vector<string> &dolist, vector<string> &forlist, int offset) {
    int len = dolist.size();
    int flen = forlist.size();
    int ihelp = 0;
    string help = "";
    int myoffset = offset;
    vector<string> p;
    string progress = "begin";

     if (name == "del") {
        try {
            if (flen < 100) {
                for (int i = 0; i < len; ++i) {
                    help = to_string(i+1);
                    if(!contains(help, forlist)) {
                        p.push_back(dolist[i]);
                    }
                }
            } else {
               // if del list is long use more efficient algorithm
                unordered_map<int, int> f;
                f.clear();
                for (int i = 0; i < flen; ++i) {
                    ihelp = stoi(forlist[i]);
                    if (f.find(ihelp) == f.end()) {
                         f.insert(pair<int, int>(ihelp, 0));
                    }
                }
                for (int i = 0; i < len; ++i) {
                    ihelp = i+1;
                    if (f.find(ihelp) == f.end()) {
                        p.push_back(dolist[i]);
                    }
                }

                f.clear();
            }
        } catch (exception& e) {
           logg.msg(FATAL, cstos(e.what()) + ": error when executing del for " + join(dolist, " "));
           if (debug_y) {logg.debug("changed stop execution: " + to_string(stopExecution) + "->9");};
           stopExecution = 9;
        }
    // --------------------------------------
    } else if (name == "expand") {
        //split every char (except space, tab)
        for (int i = 0; i < len; ++i) {
            string x = dolist[i];
            for (int j = 0; j < (int) x.size(); ++j) {
                char c = x[j];
                if ((c != ' ') && (c != '\t')) {p.push_back(ctos(x[j]));};
            }
        }
    // --------------------------------------
    } else if (name == "cond") {
        if (len == 0) {
           if (debug_y) {logg.debug("changed stop execution (in cond): " + to_string(stopExecution) + "->9");};
           logg.msg(FATAL, "condition is empty");
           stopExecution = 9;
        } else {
           if (dolist[0] == "int") {
                vector<string> dl;
                addToVector(dl, dolist);
                dl.erase(dl.begin());
                p = bulkExecLong(SINGLE, dl, vempty, vempty, vempty);
           } else if (dolist[0] == "float") {
                vector<string> dl;
                addToVector(dl, dolist);
                dl.erase(dl.begin());
                p = bulkExecFloat(SINGLE, dl, vempty, vempty, vempty);
           } else {
                p = bulkExecString(SINGLE, dolist, vempty, vempty, vempty);
           }
           // condition fulfilled => goto next
           if (p.size() > 0) {
               int res = stoi(p[0]);
               if (res > 0) {myoffset = 1;}
           }
       }
    // --------------------------------------
     } else if (name == "file") {
        int i = 0;
        int j = 0;
        if ((len>0) && (flen>0)) {
            while ((i<flen) && (j<len)) {
                p = fileop(forlist[i], dolist[j]);
                i++; j++;
                if ((i==flen) && (j<len)) {
                    i=0;
                } else if ((i<flen) && (j==len)) {
                    j=0;
                }
            }
        }
         // --------------------------------------
     } else if (name == "find") {
        try {
            if (flen < 100) {
                for (int i = 0; i < len; ++i) {
                    help = dolist[i];
                    if(contains(help, forlist)) {
                        p.push_back(to_string(i+1));
                    }
                }
            } else {
                // if find list is long use more efficient algorithm
               unordered_map<string, int> f;
               f.clear();
               for (int i = 0; i < flen; ++i) {
                   string key = forlist[i];
                    if (f.find(key) == f.end()) {
                        f.insert(pair<string, int>(key, 0));
                    }
               }
               for (int i = 0; i < len; ++i) {
                   help = dolist[i];
                   if (f.find(help) != f.end()) {
                       p.push_back(to_string(i+1));
                   }
               }
               f.clear();
            }
        } catch (exception& e) {
           logg.msg(FATAL, cstos(e.what()) + ": error when executing find for " + join(dolist, " "));
           if (debug_y) {logg.debug("changed stop execution (find): " + to_string(stopExecution) + "->9");};
           stopExecution = 9;
        }
        // --------------------------------------
     } else if (name == "freq") {
         p=freq(dolist, forlist);
    // --------------------------------------
    } else if (name == "get") {
        try {
            for (int i = 0; i < flen; ++i) {
                ihelp = stoi(forlist[i]) - 1;
                if((ihelp >= 0) && (ihelp < len)) {
                    p.push_back(dolist[ihelp]);
                }
            }
        } catch (exception& e) {
           logg.msg(FATAL, cstos(e.what()) + ": error when executing get for " + join(dolist, " "));
           if (debug_y) {logg.debug("changed stop execution (get): " + to_string(stopExecution) + "->9");};
           stopExecution = 9;
        }
    // --------------------------------------
    } else if (name == "id") {
        p = dolist;
    // --------------------------------------
    } else if (name == "input") {
       string x = join(dolist, " ") + " ";
       cout << x;
       cin >> help;
       p.push_back(help);
    // --------------------------------------
    } else if (name == "include") {
        // do nothing, is already handled
    // --------------------------------------
    } else if (name == "ins") {
        int ix = stoi(forlist[0]);
        for (int i = 0; i < len; ++i) {
            if (ix == i) {
             for (int j = 1; j < flen; ++j) {
                p.push_back(forlist[j]);
             }
            }
            p.push_back(dolist[i]);
        }
        if (ix >= len) {
             for (int j = 1; j < flen; ++j) {
                p.push_back(forlist[j]);
             }
        }
    // --------------------------------------
    } else if (name == "join") {
        if (len > 0) {
            if (flen > 0) {
                help = forlist[0];
				if (help == "#space") {help = " ";}
				p.push_back(join(dolist,help));
            } else {
				p.push_back(join(dolist, ""));
		  	}
        }
    // --------------------------------------
    } else if (name == "mask") {
        if ((len>0)&&(flen>0)) {
            int ihelp = -1;
            for (int i = 0; i < len; ++i) {
                ++ihelp;
                if(ihelp >= flen) {ihelp=0;}
                string shelp = forlist[ihelp];
                int jhelp = stoi(shelp);
 				for (int j=1; j<=jhelp; ++j) {
					p.push_back(dolist[i]);
                }
            }
        } else if (len>0) {
            p = dolist;
        }
    // --------------------------------------
    } else if (name == "minus") {
        if ((len>0)&&(flen>0)) {
            p = vminus(forlist,dolist);
        } else if (flen > 0) {
            p = forlist;
        }
    // --------------------------------------
     }  else if (name == "output") {
        doOut(dolist, true);
        p = dolist;
        if (logOutput) {logg.plain(join(dolist, " "));}
    // --------------------------------------
     }  else if (name == "pexpr") {
        if (stype == "long") {
             p = bulkExecLong(SINGLE, dolist, vempty, vempty, vempty);
        }else if (stype == "float") {
             p = bulkExecFloat(SINGLE, dolist, vempty, vempty, vempty);
        } else {
             p = bulkExecString(SINGLE, dolist, vempty, vempty, vempty);
        }
    // --------------------------------------
    } else if (name == "print") {
        doPrint(dolist, false);
        p = dolist;
        if (logOutput) {logg.plain(join(dolist, " "));}
    // --------------------------------------
       } else if (name == "range") {
        try {
          if (len >= 2) {
            vector<bool> up = getUp(dolist);
            vector<string> first = getRange(dolist[0], dolist[1], "0", up[0]);
            for (int i=0;i<(int)first.size();i++) {
                if (len >= 4) {
                   vector<string> second = getRange(dolist[2], dolist[3], first[i], up[1]);
                   for (int j=0;j<(int) second.size();j++) {
                       if (len >= 6) {
                            vector<string> third = getRange(dolist[4], dolist[5], second[j], up[2]);
                            for (int k=0;k<(int)third.size();k++) {
                                p.push_back(first[i]);
                                p.push_back(second[j]);
                                p.push_back(third[k]);
                            }
                       } else {
                            p.push_back(first[i]);
                            p.push_back(second[j]);
                       }
                   }
                } else {
                   p.push_back(first[i]);
                }
            }
          }
        } catch (exception& e) {
            logg.msg(FATAL, cstos(e.what()) + ": could not execute range on " + join(dolist, " "));
            if (debug_y) {logg.debug("changed stop execution (range): " + to_string(stopExecution) + "->9");};
            stopExecution = 9;
        }
    // --------------------------------------

    } else if (name == "readLines") {
        bool keepEmpty = false;
        bool keepEol = false;
        bool keepEof = false;
        if (forlist.size() > 0) {
            for (int i=0;i<(int)forlist.size();i++) {
                if (forlist[i] == "keepEmpty") {
                    keepEmpty = true;
                } else if (forlist[i] == "keepEol") {
                    keepEol = true;
                } else if (forlist[i] == "keepEof") {
                    keepEof = true;
                }
            }
        }
        if (dolist.size() == 0) {
            logg.msg(FATAL, "file name missing for readLines");
            if (debug_y) {logg.debug("changed stop execution (readLines): " + to_string(stopExecution) + "->9");};
            stopExecution = 9;
        } else {
            try {
                string x = dolist[0];
                if (!isPath(x)) {x=currDir+x;}
                if (debug_x) {logg.debug("read file is '" + x + "'");};
                p = readFile(x, keepEmpty, keepEol, keepEof);
                if (debug_x) {logg.debug("result.size is " + to_string(p.size()));};
           } catch (exception& e) {
                logg.msg(FATAL, cstos(e.what()) + ": could not execute readLines on file " + dolist[0]);
                if (debug_y) {logg.debug("changed stop execution (readLines): " + to_string(stopExecution) + "->9");};
                stopExecution = 9;
            }
        }

    // --------------------------------------
    } else if (name == "reverse") {
       for (int i = len-1; i >= 0; --i) {
            p.push_back(dolist[i]);
       }
    // --------------------------------------
    } else if (name == "returnValue") {
        for (int i = 0; i < len; ++i) {
            p.push_back(dolist[i]);
        }
    // --------------------------------------
    } else if (name == "set") {
       try {
           for (int i = 0; i < flen; i+=2) {
               int ix = stoi(forlist[i]);
               if ((i+1) < flen) {
                    help = forlist[i+1];
                    if ((ix>0)&& (ix<=len)){dolist[ix - 1] = help;}
               } else {
                    logg.msg(WARN, "missing value for set was ignored");
               }
           }
           addToVector(p, dolist);
       } catch(exception& e) {
           logg.msg(FATAL, cstos(e.what()) + ": error when executing set with forlist " + join(forlist, " "));
           if (debug_y) {logg.debug("changed stop execution (set): " + to_string(stopExecution) + "->9");};
           stopExecution = 9;
       }
    // --------------------------------------
    } else if (name == "sort") {
        bool isDesc = false;
        bool hasRows = false;
        int index = 0;
        int rowlen = 1;
        if (flen > 0) {
           for (int i = 0; i < flen; ++i) {
                if (forlist[i] == "desc") {
                    isDesc = true;
                } else if (forlist[i]== "rows") {
                    hasRows = true;
                    index = stoi(forlist[i+1]);
                    rowlen =stoi(forlist[i+2]);
                }
           }
        }
        if (hasRows) {
            if ((stype == "long") || (stype == "int")) {
                sortRowsLong(index, rowlen, isDesc, dolist, p);
            } else {
                sortRowsString(index, rowlen, isDesc, dolist, p);
            }
        } else if (stype == "") {
            try {
                p = dolist;
                if (isDesc) {
                    sort (p.begin(), p.end(), greater <> ());
                } else {
                    sort (p.begin(), p.end());
                }
            } catch (exception& e) {
                logg.msg(FATAL, cstos(e.what()) + ": error when executing sort-string");
                if (debug_y) {logg.debug("changed stop execution (sort): " + to_string(stopExecution) + "->9");};
               stopExecution = 9;
             }
    // --------------------------------------
        } else if (stype == "int") {
            try {
                p = dolist;
                if (isDesc) {
                    sort (p.begin(), p.end(), gtstoi <string> ());
                } else {
                    sort (p.begin(), p.end(), ltstoi <string> ());
                }
            } catch (exception& e) {
               logg.msg(FATAL, cstos(e.what()) + ": error when executing sort-int");
               if (debug_y) {logg.debug("changed stop execution (sort): " + to_string(stopExecution) + "->9");};
               stopExecution = 9;
             }
    // --------------------------------------
        } else if (stype == "long") {
            try {
                p = dolist;
                if (isDesc) {
                    sort (p.begin(), p.end(), gtstoll <string> ());
                } else {
                    sort (p.begin(), p.end(), ltstoll <string> ());
                }
            } catch (exception& e) {
               logg.msg(FATAL, cstos(e.what()) + ": error when executing sort-long");
               if (debug_y) {logg.debug("changed stop execution (sort): " + to_string(stopExecution) + "->9");};
               stopExecution = 9;
             }
        }
        // --------------------------------------
    } else if (name == "split") {
        if(len>0) {
           if(flen>0) {
                help = forlist[0];
                if (help == "#space") {help = " ";}
                char c = help[0];
                for (int i = 0; i < len; ++i) {
                    vector<string> w = split(dolist[i], c);
                    addToVector(p, w);
                }
           } else {
                //split every char
                for (int i = 0; i < len; ++i) {
                    string x = dolist[i];
                    vector<string> w;
                    for (int j = 0; j < (int) x.size(); ++j) {
                        w.push_back(ctos(x[j]));
                    }
                    addToVector(p, w);
                }
           }
        }
    // --------------------------------------
     } else if (name == "time") {
         p = timeverb(dolist);
    // --------------------------------------
    } else if (name == "unique") {
        try {
            progress = "init";
           unordered_map<string, int> f;
           f.clear();
           progress = "set map";
           for (int i = 0; i < len; ++i) {
               string key = dolist[i];
                if (f.find(key) == f.end()) {
                    f.insert(pair<string, int>(key, 0));
                }
           }
           progress = "fill p";
           for (auto iter = f.begin(); iter != f.end(); ++iter){
                p.push_back(iter->first);
           }
           progress = "after p";
           f.clear();
        } catch (exception& e) {
           logg.msg(FATAL, cstos(e.what()) + ": error when executing unique for dolist of length " + to_string(dolist.size()) + " at " + progress);
           if (debug_y) {logg.debug("changed stop execution (unique): " + to_string(stopExecution) + "->9");};
           stopExecution = 9;
        }
     // --------------------------------------
    } else if (name == "writeLines") {
        string x = forlist[0];
        if (!isPath(x)) {x=currDir+x;}
        bool retcode = writeFile(x, dolist);
        if (retcode) {
            p.push_back("1");
        } else {
            p.push_back("0");
        }
    }

    return make_tuple(myoffset, p);
}

tuple<int, vector<string>> Interpreter::executePerRowSentence3(string name, bool isStandardVerb, int jump, string stype,
                       vector<string> &thislist, vector<string> &dolist, vector<string> &forlist, vector<string> &uselist, vector<string> &withlist, int stackLevel) {
   		if (debug_t) {logg.debugtime("@init");};

   		if (debug_b) {logg.msg(DEBUG, "starting perRow for Verb " + name);};
        int offset = 1;
        vector<string> p;
		vector<string> result;
        int uselen = uselist.size();
        if (uselen == 0) {
               logg.msg(WARN, "uselist is empty for modifier perRow.");
            }
        Rowdefs rowdefs = getPerRowDefs(dolist);
        vector<Rowitem> rowitems = getPerRowItems(rowdefs.offset, dolist);
        int off = 0;
        int ix = 0;

        if (rowdefs.inits.size() > 0) {
            addToVector(result, rowdefs.inits);
        }
 		vector<string> bhelp;
		string shelp = "";
		string thelp = "";
		string entry = "";
		bool isValid = true;
		Rowitem ri = Rowitem("");
		for (int i=0; i<uselen; i+=rowdefs.rowlength) {
			isValid = true;
			off = 0;
			bhelp.clear();
			p.clear();
			int emptyEntries = 0;
			entry = "";
            for (int j=0; j<(int)rowitems.size(); ++j) {
                ri = rowitems[j];
                switch (ri.typ) {
                   case 0:
                        entry = ri.plain;
                        break;
                   case 1:
                        if (off>=rowdefs.rowlength) {off=0;}
                        if (i+off<uselen){
                           entry = uselist[i+off];
                           ++off;
                        } else if (rowdefs.hasDefuse) {
                           entry = rowdefs.defuse;
                           ++off;
                        } else {
                           bhelp.clear();
                           entry = "";
                           if (debug_x) {logg.debug("#:i+off>=uselen for i=" + to_string(i));};
                           isValid = false;
                        }
                        break;
                   case 2:
                      ix = ri.index-1;
                      if ((i+ix >= 0) && (i + ix <uselen)) {
                         entry = uselist[i+ix];
                      } else if (rowdefs.hasDefuse) {
                         entry = rowdefs.defuse;
                      } else {
                         bhelp.clear();
                         entry = "";
                         if (debug_x) {logg.debug("#<n>:i+ix>=uselen or <0 for i=" + to_string(i));};
                         isValid = false;
                      }
                      break;
                   case 3:
                        ix = ri.index-1;
                      if ((i+ix >= 0) && (i + ix <(int) withlist.size())) {
                         entry = withlist[i+ix];
                      } else if (rowdefs.hasDefwith) {
                         entry = rowdefs.defwith;
                      } else {
                         entry = "";
                         bhelp.clear();
                         if (debug_x) {logg.debug("#<n>:i+ix>=withlen or <0 for i=" + to_string(i));};
                         isValid = false;
                      }
                      break;
                   case 4:
                        ix = ri.index-1;
                        if ((i+ix >= 0) && (i + ix <(int)result.size())) {
                           entry = result[i+ix];
                        } else if (rowdefs.hasDefres) {
                            entry = rowdefs.defres;
                        } else {
                            bhelp.clear();
                            entry = "";
                            if (debug_y) {logg.debug("#<n>:i+ix>=reslen or <0 for i=" + to_string(i));};
                            isValid = false;
                        }
                        break;
                   case 5:
                        entry = to_string(i+1);
                        break;
                   case 6:
                        entry = "#all";
                        break;
                   case 7:
                        if (result.size() > 0) {
                            entry = result.back();
                        } else if (rowdefs.hasDefres) {
                            entry = rowdefs.defres;
                        } else {
                            bhelp.clear();
                            entry = "";
                            if (debug_y) {logg.debug("can't use #rend on empty result list");};
                            isValid = false;
                        }
                        break;
                }
                if (!isValid) {
                    logg.msg(WARNALL, "no valid result for " + name + " in perRow.");
                    break;
                }

                if (entry == "#all") {
                   for (int j=i; j<i+rowdefs.rowlength; j++) {
                        bhelp.push_back(uselist[j]);
                   }
                } else if (entry.size() > 0) {
                    bhelp.push_back(entry);
                } else {
                    ++emptyEntries;
                }
             }

            if (emptyEntries > 0) {
                logg.msg(WARNALL, to_string(emptyEntries) + " empty entries found. Possibly incomplete pattern for " + name + ": " + join(bhelp, ",") +  "was ignored.");
                isValid = false;
            }
			if (isValid) {
                if (isStandardVerb) {
                    tie(offset, p) = executeStandardVerb(name, stype, bhelp, forlist, offset);
                } else if (names.find(name) != names.end()) {
                    int index = names[name];
                    p = execParagraph(index, thislist, bhelp, forlist, uselist, withlist, 0);
                    if (debug_c) {logg.debug("RV=" + join(p, " "));};
                } else {
                    logg.msg(FATAL, "verb not allowed for perRow or unknown: " + name + ".");
                    if (debug_y) {logg.debug("changed stop execution (perRow): " + to_string(stopExecution) + "->9");};
                    stopExecution = 9;
                }
                if (p.size() > 0) {
                    addToVector(result, p);
                } else {
                    logg.msg(WARNALL, "empty result for " + name + " in perRow - ignored.");
                }

                if (stopExecution>0) {break;}
			}
		}

    if (debug_b) {logg.msg(DEBUG, "finished perRow for Verb " + name);};
    stopExecution =newStopExecution(stopExecution, LOOP);
    if (debug_t) {logg.debugtime("time for " + name + "-perRow: ");};
    return make_tuple(offset, result);
  }

vector<string> Interpreter::getRange(string sfrom, string sto, string sprev, bool  up) {
     vector<string> result;

     int x = getRangeIndex(sfrom, sprev);
     int y = getRangeIndex(sto, sprev);

     if ((x<=y) && up){
        for (int j = x; j <= y; ++j) {
            result.push_back(to_string(j));
        }
     } else if ((x>y) && (!up)){
        for (int j = x; j >= y; --j) {
             result.push_back(to_string(j));
        }
    }
    return result;
}

int Interpreter::getRangeIndex(string r, string prev) {
     int result = 0;
     if (r == "*") {
        result = stoi(prev);
     } else if (r == "+") {
        result= stoi(prev) + 1;
     } else if (r == "-") {
        result= stoi(prev) - 1;
     } else  {
        result= stoi(r);
     }
     return result;
}

vector<bool> Interpreter::getUp(vector<string> therange) {
    vector<bool> up;
     int x = 0;
     int y = 0;
     string s = "";
     if (therange.size()>=2) {
        x = stoi(therange[0]);
        y = stoi(therange[1]);
        up.push_back(x<=y);
     }

     if (therange.size()>=4) {
        x = getRangeIndex(therange[2], therange[0]);
        y = getRangeIndex(therange[3], therange[0]);
        s = to_string(x);
        up.push_back(x<=y);
     }

     if (therange.size()>=6) {
        x = getRangeIndex(therange[4], s);
        y = getRangeIndex(therange[5], s);
        up.push_back(x<=y);
     }
    return up;
}

int Interpreter::getDefLength(string text) {
    int result = 1;
    if (text.substr(0, 3) == "row") {
        string s = text.substr(3);
        if (!isInt(s)) {
            logg.msg(ERROR, " row length is not an integer: " + text + " - assume 1.");
        } else {
            result = stoi(s);
        }
    }
    return result;
  }

Rowdefs Interpreter::getPerRowDefs(vector<string>& v) {
      Rowdefs rowdefs;
      rowdefs.offset = 0;
      rowdefs.rowlength = 1;
      rowdefs.hasDefuse = false;
      rowdefs.hasDefwith = false;
      rowdefs.hasDefres = false;
      string text;
      string help;
      bool foundBar = false;
      int len = v.size();
      int i = 0;
      while (i<len) {
        text = v[i];
        if (text.substr(0, 3) == "row") {
            help = text.substr(3);
            if (!isInt(help)) {
                logg.msg(ERROR, " row length is not an integer: " + text + " - assume 1.");
            } else {
                rowdefs.rowlength = stoi(help);
            }
        } else if (text.substr(0, 3) == "def") {
            rowdefs.defuse = text.substr(3);
            rowdefs.hasDefuse = true;
        } else if ((text.substr(0, 4) == "rdef") && isExtended) {
            rowdefs.defres = text.substr(4);
            rowdefs.hasDefres = true;
        } else if ((text.substr(0, 4) == "wdef") && isExtended) {
            rowdefs.defwith = text.substr(4);
            rowdefs.hasDefwith = true;
        } else if ((text.substr(0, 4) == "init") && isExtended) {
            rowdefs.inits.push_back(text.substr(4));
        } else if (text == "|") {
            rowdefs.offset = i + 1;
            foundBar = true;
            break;
        }
        ++i;
    }

    if (!foundBar) {
        logg.msg(ERROR, "no bar separator '|' found in perRow pattern");
         if (debug_y) {logg.debug("changed stop execution (perRow): " + to_string(stopExecution) + "->9");};
        stopExecution = 9;
    }
    return rowdefs;
  }

vector<Rowitem> Interpreter::getPerRowItems(int start, vector<string>& v) {
    vector<Rowitem> result;
    for (int i=start; i<(int) v.size(); ++i) {
        if (isExtended || (v[i] != "#rend")) {
            Rowitem rowitem = Rowitem(v[i]);
            result.push_back(rowitem);
        }
    }
    return result;
  }

tuple<int, vector<string>> Interpreter::executeCombineSentence(string name, bool isStandardVerb, int jump, string stype,
                      vector<string> &thislist, vector<string> &dolist, vector<string> &forlist, vector<string> &uselist, vector<string> &withlist, int stackLevel) {
   		 if (debug_t) {logg.debugtime("@init");};
        int offset = 1;
        vector<string> p;
		vector<string> result;
        int uselen = uselist.size();
        int forlen = forlist.size();
        int withlen = withlist.size();
        int dolen = dolist.size();
        int uoff = 0;
        int foff = 0;
        int woff = 0;
		vector<string> bhelp;
		string shelp = "";
		string thelp = "";
		bool hasUse = false;
		bool hasFor = false;
		bool hasWith = false;
		bool goon = true;
		while (goon) {
			bhelp.clear();
		  	for (int j=0; j<dolen; ++j) {
		  		thelp = dolist[j];
				if (thelp[0] == '#') {
					thelp = thelp.substr(1);
					if (thelp == "u") {
						if (uoff <uselen) {
							shelp = uselist[uoff];
							bhelp.push_back(shelp);
							++uoff;
							hasUse = true;
						} else {
						    goon = false;
						    break;
 						}
					} else if (thelp == "f") {
						if (foff <forlen) {
							shelp = forlist[foff];
							bhelp.push_back(shelp);
							++foff;
							hasFor = true;
						} else {
   						    goon = false;
						    break;
 						}
					} else if (thelp == "w") {
						if (woff <withlen) {
							shelp = withlist[woff];
							bhelp.push_back(shelp);
							++woff;
							hasWith = true;
						} else {
   						    goon = false;
						    break;
 						}
					} else {
                        // do nothing, # is ignored
					}
				} else {
					bhelp.push_back(thelp);
				}
		  	}

		  	if (goon) {
                vector<string> fl;
                vector<string> ul;
                vector<string> wl;
                if (!hasFor) {fl = forlist;}
                if (!hasUse) {ul=uselist;}
                if (!hasWith) {wl=withlist;}
                tie(offset, p) = executePlainVerb(name, isStandardVerb, jump, stype, thislist, bhelp, fl, ul, wl, stackLevel);
                addToVector(result, p);

                if (stopExecution>0) {break;}
		  	}
		}

    stopExecution = newStopExecution(stopExecution, LOOP);
     if (debug_t) {logg.debugtime("time for " + name + "-combine: ");};
    return make_tuple(offset, result);
  }

vector<string> Interpreter::vminus(vector<string> a, vector<string> b) {
    vector<string> c;
    vector<string> d;

    try {
        d = b;

        for (int i=0;i< (int) a.size(); ++i) {
            string o = a[i];
            vector<string>::iterator it = find(d.begin(), d.end(), o);
            if (it != d.end()) {
                d.erase(it);
            } else {
                c.push_back(o);
            }
        }
    } catch (exception& e) {
        logg.msg(FATAL, cstos(e.what()) + ": when executing verb minus");
         if (debug_y) {logg.debug("changed stop execution (minus): " + to_string(stopExecution) + "->9");};
        stopExecution = 9;
    }
    return c;
  }

void Interpreter::doPrint(vector<string> v, bool withCrlf) {
    int len = v.size();
    bool suppress = false;
    for (int i = 0; i < len; ++i) {
        string s = format(v[i]);
        if ((i>0) && !suppress) {printf(" ");}
        printf("%s", s.data());
        suppress = false;
        if (last(s) == '\n') {suppress = true;}
    }
    if (withCrlf) {printf("\n");}
  }

void Interpreter::doOut(vector<string> v, bool withCrlf) {
    //setlocale(LC_ALL, "de_DE.UTF-8");
    //cout.imbue(mylocale);
    int len = v.size();
    for (int i = 0; i < len; ++i) {
        if(i>0) {cout << " ";}
        cout << (v[i]);
    }
    if (withCrlf)  {cout << endl;}
    //setlocale(LC_ALL, "C");
}

vector<string> Interpreter::fileop(string op, string filename) {
    vector<string> result;
    string x = filename;
    bool bhelp;
    try {
        if (!isPath(x)) {x=currDir+x;}
        if (op == "delete") {
            if (!filesystem::exists(x)) {
               result.push_back("1");
            } else {
                int res = remove(x.c_str()); // 0=ok
                if (res == 0) {
                    result.push_back("1");
                } else {
                    logg.msg(ERROR, "file " + x + " not deleted.");
                    result.push_back("0");
                }
            }
        } else if (op == "exists") {
            if (filesystem::exists(x)) {
                result.push_back("1");
            } else {
                result.push_back("0");
            }
        } else if (op == "create") {
            ofstream o(x);
            o.close();
            result = fileop("exists", x);
        } else if (op == "deletedir") {
            if (!filesystem::exists(x)) {
                result.push_back("1");
            } else if (!filesystem::is_directory(x)) {
                result.push_back("1");
                logg.msg(ERROR, x + "is not a directory -  no deletion.");
            } else {
                bhelp = filesystem::remove(x);
                if (bhelp) {
                    result.push_back("1");
                } else {
                    result.push_back("0");
                    logg.msg(ERROR, "directory " + x + " not deleted.");
                }
            }
        } else if (op == "existsdir") {
            bhelp = filesystem::is_directory(x);
            if (bhelp) {
                result.push_back("1");
            } else {
                result.push_back("0");
            }
        } else if (op == "createdir") {
            bhelp = filesystem::create_directory(x);
            if (filesystem::is_directory(x)) {
                result.push_back("1");
            } else {
                logg.msg(ERROR, "directory " + x + " not created.");
                result.push_back("0");
            }
        } else if (op == "listdir") {
            if ((filename == "/") || (filename == "./")) {x = currDir;}
            for (const auto & file : filesystem::directory_iterator(x)) {
                string path = file.path().string();
                // result is always absolute path, reduce to relative path
                if (path.substr(0,currDir.size()) == currDir) {
                    path = path.substr(currDir.size()-1);
                    if ((path[0]== '/') || (path[0] == '\\')) {
                        path=path.substr(1);
                    }
                }
                result.push_back(path);
            }
        }
    } catch (exception& e) {
        logg.msg(FATAL, cstos(e.what()) + ": could not execute " + op + " on file " + x);
         if (debug_y) {logg.debug("changed stop execution (fileop): " + to_string(stopExecution) + "->9");};
        stopExecution = 9;
    }
    return result;
}

vector<string> Interpreter::bulkExecLong(Modes mode, vector<string> &ops, vector<string> &liste, vector<string> &uselist, vector<string> &withlist) {
     if (debug_t) {logg.debugtime( "@init");};
    vector<Opcode> codes = toOpcode(ops);
    vector<string> result;
    int len = (mode == MULTI) ? liste.size() : 1;
    int i = 0;
    vector<long long int> ihelp;
    long long int il;
    long long int ir;
    long long int i3;
    int incr = 1;
    bool hasdefault = false;
    long long int def = 0;
    int maxvar = (mode == MULTI) ? getRowLength(ops) : 1;
    Opcode code = codes[0];

    while ((code.op == ROW) || (code.op == DEFAULT)){
        if (mode == SINGLE) {
            logg.msg(ERROR, "Opcodes row/def not allowed in pexpr.");
            return vempty;
        } else if (code.op == ROW) {
            incr = code.left;
            codes.erase(codes.begin());
             if (debug_b) {logg.debug("setting row incr to " + to_string(incr));};
        } else if (code.op == DEFAULT) {
            hasdefault = true;
            def = code.left;
            codes.erase(codes.begin());
             if (debug_b) {logg.debug("setting default to " + to_string(def));};
            maxvar = 1;
        }
        code = codes[0];
    }
     if (debug_b) {logg.debug("plist long, len= " + to_string(len) + ", maxvar = " + to_string(maxvar));};

    try {
        while(i<=len-maxvar) {
            ihelp.clear();
            int j = 0;
            while (j<(int)codes.size()) {
                code = codes[j];
                if (ihelp.size() < code.operands) {
                    if ((ihelp.size() == 1) && ((code.op == AND) || (code.op == OR) || (code.op == PLUS) || (code.op == TIMES)
                        || (code.op == MIN)  || (code.op == MAX))) {
                            // do nothing, just skip operator
                        } else {
                            logg.msg(ERROR, "not enough operands for Opcode " + to_string(code.op));
                            if (debug_y) {logg.debug("changed stop execution (missing operands): " + to_string(stopExecution) + "->9");};
                            stopExecution = 9;
                            break;
                        }
                } else if (code.op == AND) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(((ir>0) && (il>0)) ? 1 : 0);
                } else if (code.op == OR) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(((ir>0) || (il>0)) ? 1 : 0);
                } else if (code.op == NOT) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((ir == 0) ? 1 : 0);
                } else if (code.op == ABS) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((ir >= 0) ? ir : -ir);
                }else if (code.op == PLUS) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(il + ir);
                } else if (code.op == MINUS) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(il - ir);
                } else if (code.op == TIMES) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(il * ir);
                } else if (code.op == DIVIDE) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(il / ir);
                } else if (code.op == MOD) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(il % ir);
                } else if (code.op == MAX) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   if (il > ir) {
                    ihelp.push_back(il);
                   } else {
                    ihelp.push_back(ir);
                   }
                } else if (code.op == MIN) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   if (il < ir) {
                    ihelp.push_back(il);
                   } else {
                    ihelp.push_back(ir);
                   }
                } else if (code.op == DIV) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   i3 = il % ir;
                   il = (il - i3) / ir;
                   ihelp.push_back(il);
                   ihelp.push_back(i3);
                } else if (code.op == POWER) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(lpower(il, ir));
                }else if (code.op == EQ) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((il == ir) ? 1 : 0);
                } else if (code.op == NEQ) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((il != ir) ? 1 : 0);
                } else if (code.op == GE) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((il >= ir) ? 1 : 0);
                } else if (code.op == LE) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((il <= ir) ? 1 : 0);
                } else if (code.op == GT) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((il > ir) ? 1 : 0);
                } else if (code.op == LT) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back((il < ir) ? 1 : 0);
                } else if (code.op == ELVIS) {
                   i3 =  ihelp.back();
                   ihelp.pop_back();
                   ir = ihelp.back();
                   ihelp.pop_back();
                   il =  ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(((il>0)) ? ir : i3);
                } else if (code.op == VAL) {
                   ihelp.push_back(stoll(code.sleft));
                } else if (code.op == VAR) {
                    if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode var not allowed in pexpr.");
                        return vempty;
                    }

                    il = i + code.left - 1;
                    if (hasdefault) {
                      if ((il < 0) || (il >= len)) {
                        ihelp.push_back(def);
                      } else {
                        ihelp.push_back(stoll(liste[il]));
                      }
                    } else {
                        ihelp.push_back(stoll(liste[il]));
                    }
                } else if (code.op == IDX) {
                   if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode idx not allowed in pexpr.");
                        return vempty;
                   }
                   ihelp.push_back(i+1);
                } else if (code.op == UGET) {
                   if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode uget not allowed in pexpr.");
                        return vempty;
                   }
                   ir = ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(stoll(uselist[ir-1]));
                } else if (code.op == WGET) {
                   if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode wget not allowed in pexpr.");
                        return vempty;
                   }
                   ir = ihelp.back();
                   ihelp.pop_back();
                   ihelp.push_back(stoll(withlist[ir-1]));
                } else if (code.op == OUT) {
                    if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode out not allowed in pexpr.");
                        return vempty;
                   }
                   ir = ihelp.back();
                   ihelp.pop_back();
                   result.push_back(to_string(ir));
                } else if (code.op == DROP) {
                   ihelp.pop_back();
                } else if (code.op == END) {
                   break;
                } else if (code.op == SKIPZ) {
                   ir = ihelp.back();
                   ihelp.pop_back();
                   if (ir == 0) {j+= code.left;}
                }
                ++j;
            }
            i+= incr;
        }

    } catch (exception& e) {
          logg.msg(FATAL, cstos(e.what()) + ": error when executing plist/pexpr-long for " + join(ops, " ") + " - ihelp.size = " + to_string(ihelp.size()));
           if (debug_y) {logg.debug("changed stop execution (plist/pexpr-long): " + to_string(stopExecution) + "->9");};
          stopExecution = 9;
    }

    if (ihelp.size() > 0) {
        if (mode == SINGLE) {
            for (int i=0; i<(int) ihelp.size(); ++i) {
               result.push_back(to_string(ihelp[i]));
            }
        } else {
           logg.msg(WARNALL, "not all op results used for plist - " + to_string(ihelp.size()) + " left.");
        }
    }

    if (mode == MULTI) {
         if (debug_t) {logg.debugtime("time;bulk;long;");};
    } else {
         if (debug_t) {logg.debugtime("time;expr;long;");};
    }

    return result;
}

vector<string> Interpreter::bulkExecFloat(Modes mode, vector<string> &ops, vector<string> &liste, vector<string> &uselist, vector<string> &withlist) {
     if (debug_t) {logg.debugtime( "@init");};
    vector<Opcode> codes = toOpcode(ops);
    vector<string> result;
    int len = (mode == MULTI) ? liste.size() : 1;
    int i = 0;
    vector<double> dhelp;
    double dl;
    double dr;
    double d3;
    int incr = 1;
    int ihelp = 0;
    bool hasdefault = false;
    double def = 0.0;
    int maxvar = (mode == MULTI) ? getRowLength(ops) : 1;
    Opcode code = codes[0];

    while ((code.op == ROW) || (code.op == DEFAULT)){
        if (mode == SINGLE) {
            logg.msg(ERROR, "Opcodes row/def not allowed in pexpr.");
            return vempty;
        } else if (code.op == ROW) {
            incr = code.left;
            codes.erase(codes.begin());
             if (debug_b) {logg.debug("setting row incr to " + to_string(incr));};
        } else if (code.op == DEFAULT) {
            hasdefault = true;
            def = code.left;
            codes.erase(codes.begin());
             if (debug_b) {logg.debug("setting default to " + to_string(def));};
            maxvar = 1;
        }
        code = codes[0];
    }
     if (debug_b) {logg.debug("plist/pexpr float, len= " + to_string(len) + ", maxvar = " + to_string(maxvar));};

    try {
        while(i<=len-maxvar) {
            dhelp.clear();
            int j = 0;
            while (j<(int)codes.size()) {
                code = codes[j];
                if (dhelp.size() < code.operands) {
                    if ((dhelp.size() == 1) && ((code.op == AND) || (code.op == OR) || (code.op == PLUS) || (code.op == TIMES)
                        || (code.op == MIN)  || (code.op == MAX))) {
                            // do nothing, just skip operator
                        } else {
                            logg.msg(ERROR, "not enough operands for Opcode " + to_string(code.op));
                            if (debug_y) {logg.debug("changed stop execution (missing operands): " + to_string(stopExecution) + "->9");};
                            stopExecution = 9;
                            break;
                        }
                } else if (code.op == AND) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(((dr>0) && (dl>0)) ? 1 : 0);
                } else if (code.op == OR) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(((dr>0) || (dl>0)) ? 1 : 0);
                } else if (code.op == NOT) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dr == 0) ? 1 : 0);
                } else if (code.op == ABS) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dr >= 0) ? dr : -dr);
                } else if (code.op == CEIL) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(ceil(dr));
                }  else if (code.op == FLOOR) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(floor(dr));
                } else if (code.op == ROUND) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(round(dr));
                } else if (code.op == SQRT) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(sqrt(dr));
                } else if (code.op == SIN) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(sin(dr));
                } else if (code.op == COS) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(cos(dr));
                } else if (code.op == TAN) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(tan(dr));
                } else if (code.op == ARCSIN) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(asin(dr));
                } else if (code.op == ARCCOS) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(acos(dr));
                } else if (code.op == ARCTAN) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(atan(dr));
                } else if (code.op == EXP) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(exp(dr));
                } else if (code.op == LOG) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(log(dr));
                } else if (code.op == LOG10) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(log10(dr));
                } else if (code.op == SINH) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(sinh(dr));
                } else if (code.op == COSH) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(cosh(dr));
                } else if (code.op == TANH) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(tanh(dr));
                } else if (code.op == PLUS) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(dl + dr);
                } else if (code.op == MINUS) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(dl - dr);
                } else if (code.op == TIMES) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(dl * dr);
                } else if (code.op == DIVIDE) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(dl / dr);
                } else if (code.op == MOD) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   d3 = (int) round(dl) % (int) round(dr);
                   dhelp.push_back(d3);
                } else if (code.op == MAX) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   if (dl > dr) {
                    dhelp.push_back(dl);
                   } else {
                    dhelp.push_back(dr);
                   }
                } else if (code.op == MIN) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   if (dl < dr) {
                    dhelp.push_back(dl);
                   } else {
                    dhelp.push_back(dr);
                   }
                } else if (code.op == DIV) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   d3 = (int) round(dl) % (int) round(dr);
                   dl = (dl - d3) / dr;
                   dhelp.push_back(dl);
                   dhelp.push_back(d3);
                } else if (code.op == POWER) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(pow(dl, dr));
                }else if (code.op == EQ) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dl == dr) ? 1 : 0);
                } else if (code.op == NEQ) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dl != dr) ? 1 : 0);
                } else if (code.op == GE) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dl >= dr) ? 1 : 0);
                } else if (code.op == LE) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dl <= dr) ? 1 : 0);
                } else if (code.op == GT) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dl > dr) ? 1 : 0);
                } else if (code.op == LT) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back((dl < dr) ? 1 : 0);
                } else if (code.op == ELVIS) {
                   d3 =  dhelp.back();
                   dhelp.pop_back();
                   dr = dhelp.back();
                   dhelp.pop_back();
                   dl =  dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(((dl>0)) ? dr : d3);
                } else if (code.op == VAL) {
                   dhelp.push_back(stod(code.sleft));
                } else if (code.op == VAR) {
                    if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode var not allowed in pexpr.");
                        return vempty;
                    }

                    ihelp = i + code.left - 1;
                    if (hasdefault) {
                      if ((ihelp < 0) || (ihelp >= len)) {
                        dhelp.push_back(def);
                      } else {
                        dhelp.push_back(stod(liste[ihelp]));
                      }
                    } else {
                        dhelp.push_back(stoll(liste[ihelp]));
                    }
                } else if (code.op == IDX) {
                   if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode idx not allowed in pexpr.");
                        return vempty;
                   }
                   dhelp.push_back(i+1);
                } else if (code.op == UGET) {
                   if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode uget not allowed in pexpr.");
                        return vempty;
                   }
                   ihelp = (int) dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(stod(uselist[ihelp - 1]));
                } else if (code.op == WGET) {
                   if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode wget not allowed in pexpr.");
                        return vempty;
                   }
                   ihelp = (int) dhelp.back();
                   dhelp.pop_back();
                   dhelp.push_back(stod(withlist[ihelp-1]));
                } else if (code.op == OUT) {
                    if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode out not allowed in pexpr.");
                        return vempty;
                   }
                   dr = dhelp.back();
                   dhelp.pop_back();
                   result.push_back(to_string(dr));
                } else if (code.op == DROP) {
                   dhelp.pop_back();
                } else if (code.op == END) {
                   break;
                } else if (code.op == SKIPZ) {
                   dr = dhelp.back();
                   dhelp.pop_back();
                   if (dr == 0) {j+= code.left;}
                }
                ++j;
            }
            i+= incr;
        }

    } catch (exception& e) {
          logg.msg(FATAL, cstos(e.what()) + ": error when executing plist/pexpr-float for " + join(ops, " ") + " - ihelp.size = " + to_string(dhelp.size()));
           if (debug_y) {logg.debug("changed stop execution (plist/pexpr): " + to_string(stopExecution) + "->9");};
          stopExecution = 9;
    }

    if (dhelp.size() > 0) {
        if (mode == SINGLE) {
            for (int i=0; i<(int) dhelp.size(); ++i) {
               result.push_back(to_string(dhelp[i]));
            }
        } else {
           logg.msg(WARNALL, "not all op results used for plist - " + to_string(dhelp.size()) + " left.");
        }
    }

    if (mode == MULTI) {
         if (debug_t) {logg.debugtime("time;bulk;float;");};
    } else {
         if (debug_t) {logg.debugtime("time;expr;float;");};
    }

    return result;
}

vector<string> Interpreter::bulkExecString(Modes mode, vector<string> &ops, vector<string> &liste, vector<string> &uselist, vector<string> &withlist) {
     if (debug_t) {logg.debugtime("@init");};
    vector<Opcode> codes = toOpcode(ops);
    vector<string> result;
    int len = (mode == MULTI) ? liste.size() : 1;
    int i = 0;
    int il = 0;
    vector<string> shelp;
    string sl;
    string sr;
    string s3;
    int incr = 1;
    int maxvar = (mode == MULTI) ? getRowLength(ops) : 1;
    Opcode code = codes[0];
    bool hasdefault = false;
    string def = "";

    while ((code.op == ROW) || (code.op == DEFAULT)){
        if (mode == SINGLE) {
            logg.msg(ERROR, "Opcodes row/def not allowed in pexpr.");
            return vempty;
        } else if (code.op == ROW) {
            incr = code.left;
            codes.erase(codes.begin());
             if (debug_b) {logg.debug("setting row incr to " + to_string(incr));};
        } else if (code.op == DEFAULT) {
            hasdefault = true;
            def = code.sleft;
            codes.erase(codes.begin());
             if (debug_b) {logg.debug("setting default to " + def);};
            maxvar = 1;
        }
        code = codes[0];
    }
     if (debug_b) {logg.debug("plist/pexpr string, len= " + to_string(len) + ", maxvar = " + to_string(maxvar));};

    try {
         while(i<=len-maxvar) {
            shelp.clear();
            int j = 0;
            while (j<(int)codes.size()) {
                code = codes[j];
                if (shelp.size() < code.operands) {
                    if ((shelp.size() == 1) && ((code.op == AND) || (code.op == OR) || (code.op == PLUS) || (code.op == TIMES)
                        || (code.op == MIN)  || (code.op == MAX))) {
                            // do nothing, just skip operator
                        } else {
                            logg.msg(ERROR, "not enough operands for Opcode " + to_string(code.op));
                            if (debug_y) {logg.debug("changed stop execution (missing operands): " + to_string(stopExecution) + "->9");};
                            stopExecution = 9;
                            break;
                        }
                } else if (code.op == AND) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(((sr!="0") && (sl!="0")) ? "1" : "0");
                } else if (code.op == OR) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(((sr!="0") || (sl!="0")) ? "1" : "0");
                } else if (code.op == NOT) {
                   sr = shelp.back();
                   shelp.pop_back();
                   shelp.push_back((sr == "0") ? "1" : "0");
                } else if (code.op == ISINT) {
                   sr = shelp.back();
                   shelp.pop_back();
                   shelp.push_back(isInt(sr) ? "1" : "0");
                } else if (code.op == ISNUM) {
                   sr = shelp.back();
                   shelp.pop_back();
                   shelp.push_back(isNumber(sr) ? "1" : "0");
                } else if (code.op == LEN) {
                   sr = shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(sr.size()));
                }else if (code.op == PLUS) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(stoll(sl) + stoll(sr)));
                } else if (code.op == CONCAT) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(sl + sr);
                } else if (code.op == MINUS) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(stoll(sl) - stoll(sr)));
                } else if (code.op == DIVIDE) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(stoll(sl) / stoll(sr)));
                } else if (code.op == MOD) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(stoll(sl) % stoll(sr)));
                } else if (code.op == POWER) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(lpower(stoll(sl), stoll(sr))));
                } else if (code.op == TIMES) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(to_string(stoll(sl) * stoll(sr)));
                } else if (code.op == MAX) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   if (sl > sr) {
                    shelp.push_back(sl);
                   } else {
                    shelp.push_back(sr);
                   }
                } else if (code.op == MIN) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   if (sl < sr) {
                    shelp.push_back(sl);
                   } else {
                    shelp.push_back(sr);
                   }
                } else if (code.op == EQ) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back((sl == sr) ? "1" : "0");
                 } else if (code.op == NEQ) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back((sl != sr) ? "1" : "0");
                 } else if (code.op == GE) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   if (isLong(sr) && isLong(sl)) {
                        shelp.push_back((stoll(sl) >= stoll(sr) ? "1" : "0"));
                   } else {
                        shelp.push_back(sl >= sr ? "1" : "0");
                   }
                 } else if (code.op == LE) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   if (isLong(sr) && isLong(sl)) {
                        shelp.push_back((stoll(sl) <= stoll(sr) ? "1" : "0"));
                   } else {
                        shelp.push_back(sl <= sr ? "1" : "0");
                   }
                 } else if (code.op == GT) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   if (isLong(sr) && isLong(sl)) {
                        shelp.push_back((stoll(sl) > stoll(sr) ? "1" : "0"));
                   } else {
                        shelp.push_back(sl > sr ? "1" : "0");
                   }

                 } else if (code.op == LT) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   if (isLong(sr) && isLong(sl)) {
                        shelp.push_back((stoll(sl) < stoll(sr) ? "1" : "0"));
                   } else {
                        shelp.push_back(sl < sr ? "1" : "0");
                   }
                } else if (code.op == CONCAT) {
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(sl + sr);
                } else if (code.op == ELVIS) {
                   s3 =  shelp.back();
                   shelp.pop_back();
                   sr = shelp.back();
                   shelp.pop_back();
                   sl =  shelp.back();
                   shelp.pop_back();
                   shelp.push_back(((sl!="0")) ? sr : s3);
                } else if (code.op == VAL) {
                   shelp.push_back(code.sleft);
                } else if (code.op == IDX) {
                     if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode idx not allowed in pexpr.");
                        return vempty;
                    }
                    shelp.push_back(to_string(i+1));
                } else if (code.op == VAR) {
                     if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode var not allowed in pexpr.");
                        return vempty;
                    }
                    il = i + code.left - 1;
                    if (hasdefault) {
                      if ((il < 0) || (il >= len)) {
                        shelp.push_back(def);
                      } else {
                        shelp.push_back(liste[il]);
                      }
                    } else {
                        shelp.push_back(liste[il]);
                    }
                } else if (code.op == UPPER) {
                   sr = shelp.back();
                   shelp.pop_back();
                   shelp.push_back(toUpper(sr));
                } else if (code.op == LOWER) {
                   sr = shelp.back();
                   shelp.pop_back();
                   shelp.push_back(toLower(sr));
                } else if (code.op == UGET) {
                    if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode uget not allowed in pexpr.");
                        return vempty;
                    }

                    sl = shelp.back();
                    il = stoi(sl);
                    shelp.pop_back();
                    shelp.push_back(uselist[il-+1]);
                } else if (code.op == WGET) {
                    if (mode == SINGLE) {
                        logg.msg(ERROR, "Opcode wget not allowed in pexpr.");
                        return vempty;
                    }
                   sl = shelp.back();
                   il = stoi(sl);
                   shelp.pop_back();
                   shelp.push_back(withlist[il-1]);
                } else if (code.op == OUT) {
                   if (mode == SINGLE) {
                       logg.msg(ERROR, "Opcode out not allowed in pexpr.");
                       return vempty;
                   }
                   sr = shelp.back();
                   shelp.pop_back();
                   result.push_back(sr);
                } else if (code.op == DROP) {
                   shelp.pop_back();
                } else if (code.op == END) {
                   break;
                } else if (code.op == SKIPZ) {
                   sr = shelp.back();
                   shelp.pop_back();
                   if (sr == "0") {j+= code.left;}
                }
                ++j;
            }

             i+= incr;
        }
    } catch (exception& e) {
          logg.msg(FATAL, cstos(e.what()) + ": error when executing plist/pexpr for " + join(ops, " ") + " - ihelp.size = " + to_string(shelp.size()));
           if (debug_y) {logg.debug("changed stop execution (plist/pexpr): " + to_string(stopExecution) + "->9");};
          stopExecution = 9;
    }

    if (shelp.size() > 0) {
        if (mode == SINGLE) {
            for (int i=0; i<(int) shelp.size(); ++i) {
               result.push_back(shelp[i]);
            }
        } else {
           logg.msg(WARNALL, "not all op results used for plist - " + to_string(shelp.size()) + " left.");
        }
    }

    if (mode == MULTI) {
         if (debug_t) {logg.debugtime("time;bulk;string;");};
    } else {
         if (debug_t) {logg.debugtime("time;expr;string;");};
    }

    return result;
}


int Interpreter::getRight(string op, int pre) {
    string s = op.substr(pre);
     if (debug_b) {logg.debug("converting " + s);};
    return stoi(s);
 }

 // get  max. var index value
int Interpreter::getRowLength(vector<string> v) {
    int result = 1;
    string code;
    for (int i=0;i<(int) v.size(); ++i) {
        code = v[i];
        if (code.substr(0,3)== "var") {
            result = max(result, stoi(code.substr(3)));
        }
    }
    return result;
 }

vector<Opcode> Interpreter::toOpcode(vector<string> v) {
    vector<Opcode> result;
    string code;
    for (int i=0;i<(int) v.size(); ++i) {
        code = v[i];
         if (debug_b) {logg.debug("now processing " + code);};
        Opcode op;
        op.sleft = "";
        op.operands = 2;
        if (code.substr(0,3) == "row") {
            op.op = ROW;
            op.left = getRight(code, 3);
            op.operands = 0;
        } else if (code == "and"){
            op.op = AND;
            op.left = 0;
        } else if (code == "or") {
            op.op = OR;
            op.left = 0;
        } else if (code == "?") {
            op.op = ELVIS;
            op.left = 0;
            op.operands = 3;
         } else if (code == "+") {
            op.op = PLUS;
            op.left = 0;
         } else if (code == "-") {
            op.op = MINUS;
            op.left = 0;
         } else if (code == "*") {
            op.op = TIMES;
            op.left = 0;
         } else if (code == "/") {
            op.op = DIVIDE;
            op.left = 0;
         } else if (code == "&") {
            op.op = CONCAT;
            op.left = 0;
         }  else if (code == "pwr") {
            op.op = POWER;
            op.left = 0;
         } else if (code == "==") {
            op.op = EQ;
            op.left = 0;
         } else if (code == "<>") {
            op.op = NEQ;
            op.left = 0;
         } else if (code == ">=") {
            op.op = GE;
            op.left = 0;
         } else if (code == "<=") {
            op.op = LE;
            op.left = 0;
         } else if (code == ">") {
            op.op = GT;
            op.left = 0;
         } else if (code == "<") {
            op.op = LT;
            op.left = 0;
         } else if (code == "not") {
            op.op = NOT;
            op.left = 0;
            op.operands = 1;
         } else if (code == "len") {
            op.op = LEN;
            op.left = 0;
            op.operands = 1;
         } else if (code == "abs") {
            op.op = ABS;
            op.left = 0;
            op.operands = 1;
         } else if (code == "sqrt") {
            op.op = SQRT;
            op.left = 0;
            op.operands = 1;
         } else if (code == "ceil") {
            op.op = CEIL;
            op.left = 0;
            op.operands = 1;
         } else if (code == "floor") {
            op.op = FLOOR;
            op.left = 0;
            op.operands = 1;
         } else if (code == "round") {
            op.op = ROUND;
            op.left = 0;
            op.operands = 1;
         } else if (code == "sin") {
            op.op = SIN;
            op.left = 0;
            op.operands = 1;
         } else if (code == "cos") {
            op.op = COS;
            op.left = 0;
            op.operands = 1;
         } else if (code == "tan") {
            op.op = TAN;
            op.left = 0;
            op.operands = 1;
         } else if (code == "pi") {
            op.op = VAL;
            op.sleft = "3.141592653589793";
            op.operands = 0;
         } else if (code == "asin") {
            op.op = ARCSIN;
            op.left = 0;
            op.operands = 1;
         } else if (code == "acos") {
            op.op = ARCCOS;
            op.left = 0;
            op.operands = 1;
         } else if (code == "atan") {
            op.op = ARCTAN;
            op.left = 0;
            op.operands = 1;
         } else if (code == "exp") {
            op.op = EXP;
            op.left = 0;
            op.operands = 1;
         } else if (code == "log") {
            op.op = LOG;
            op.left = 0;
            op.operands = 1;
         } else if (code == "log10") {
            op.op = LOG10;
            op.left = 0;
            op.operands = 1;
         } else if (code == "sinh") {
            op.op = SINH;
            op.left = 0;
            op.operands = 1;
         } else if (code == "cosh") {
            op.op = COSH;
            op.left = 0;
            op.operands = 1;
         } else if (code == "tanh") {
            op.op = TANH;
            op.left = 0;
            op.operands = 1;
         } else if (code == "out") {
            op.op = OUT;
            op.left = 0;
            op.operands = 1;
        } else if (code == "%") {
            op.op = MOD;
            op.left = 0;
        } else if ((code == "div") && isExtended) {
            op.op = DIV;
            op.left = 0;
        } else if (code == "min") {
            op.op = MIN;
            op.left = 0;
        } else if (code == "max") {
            op.op = MAX;
            op.left = 0;
        } else if (code == "drop") {
            op.op = DROP;
            op.left = 0;
            op.operands = 1;
        } else if (code == "end") {
            op.op = END;
            op.left = 0;
            op.operands = 0;
        }else if (code.substr(0,4) == "skpz") {
            op.op = SKIPZ;
            op.left = stoi(code.substr(4));
            op.operands = 1;
        } else if (code.substr(0,3)== "val") {
            op.op = VAL;
            op.sleft = code.substr(3);
            op.operands = 0;
        } else if (code.substr(0,3)== "idx") {
            op.op = IDX;
            op.left = 0;
            op.operands = 0;
        } else if (code.substr(0,3)== "var") {
            op.op = VAR;
            op.operands = 0;
            op.left = stoi(code.substr(3));
        } else if ((code == "uget")&& isExtended) {
            op.op = UGET;
            op.left = 0;
            op.operands = 1;
        } else if ((code == "wget")&& isExtended) {
            op.op = WGET;
            op.left = 0;
            op.operands = 1;
        } else if (code == "isint") {
            op.op = ISINT;
            op.left = 0;
            op.operands = 1;
        } else if (code == "isnum") {
            op.op = ISNUM;
            op.left = 0;
            op.operands = 1;
        } else if (code == "upper") {
            op.op = UPPER;
            op.left = 0;
            op.operands = 1;
        } else if (code == "lower") {
            op.op = LOWER;
            op.left = 0;
            op.operands = 1;
        }else if (code.substr(0,3) == "def") {
            op.op = DEFAULT;
            op.sleft = code.substr(3);
            op.operands = 0;
            if (isInt(op.sleft)) {
                op.left = stoi(op.sleft);
                 if (debug_b) {logg.debug("recognized default " + to_string(op.left));};
            } else {
                op.op = VAL;
                op.sleft = code;
                 if (debug_b) {logg.debug(op.sleft + "regarded as var");};
            }
        } else {
            // assume value and push item on stack
            op.op = VAL;
            op.sleft = code;
            op.operands = 0;
        }
        result.push_back(op);
    }

     if (debug_b) {logg.debug("finished toOpcode.");};

    return result;
}

int Interpreter::ipower(int b, int e) {
    int result = 1;
    if (e==0) {return 1;}
    for (int i=1;i<=e; ++i) {
        result *= b;
    }

    return result;
}

long long int Interpreter::lpower(long long int b, long long int e) {
    long long int result = 1;
    if (e==0) {return 1;}
    for (int i=1;i<=e; ++i) {
        result *= b;
    }

    return result;
}

// typ
int Interpreter::newStopExecution(int old, StopTypes stype) {
    int result = old;
    if (stype == PARAGRAPH) {
        if (old==3) {
            result = 2;
        } else if (old<3) {
            result = 0;
        }
    } else if (stype == LOOP) {
        if (old <= 3) {result=0;}
    }
    if (old != result) { if (debug_y) {logg.debug("changed stop execution: " + to_string(old) + "->" + to_string(result));};}
    return result;
}

vector<string> Interpreter::timeverb(vector<string> &tv) {
     vector<string> result;
     string help;
     string help1;
     string help2;
     for (int i = 0; i < (int) tv.size(); ++i) {
        if (tv[i] == "now") {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            result.push_back(datetime.tmtoTime(ltm));
        } else if (tv[i] == "today") {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            result.push_back(datetime.tmtoDate(ltm));
        } else if ((tv[i] == "+")  && (result.size() >=2)){
            help2 = result.back();
            result.pop_back();
            help1 = result.back();
            result.pop_back();
            help = datetime.add(help1, help2);
            result.push_back(help);
        } else if ((tv[i] == "-") && (result.size() >=2)){
            help2 = result.back();
            result.pop_back();
            help1 = result.back();
            result.pop_back();
            help = datetime.subtract(help1, help2);
            result.push_back(help);
        } else if (findInString('m', tv[i]) || findInString('h', tv[i]) || findInString('s', tv[i])) {
            datetime.setTimeFormat(tv[i]);
        } else if (findInString('D', tv[i]) || findInString('M', tv[i]) || findInString('Y', tv[i])
                   || findInString('C', tv[i]) || findInString('W', tv[i]) || findInString('w', tv[i])){
            datetime.setDateFormat(tv[i]);
        } else {
            result.push_back(tv[i]);
        }
     }
    return result;
}

// this function returns its result as the last variable
void Interpreter::sortRowsLong(int index, int rows, bool isDesc, vector<string> &v, vector<string> &p) {
     map<long long int,vector<string>> m;
     vector<string> w;
     long long int idx = 0;
     for (int i = 0; i < (int) v.size(); i=i+rows) {
        w.clear();
        idx = 0;
        for (int j = 0; j < rows; ++j) {
            w.push_back(v[i+j]);
            if (j==index-1) {idx = stoll(v[i+j]);}
        }
        m[idx]= w;
     }

     if (isDesc) {
         for (map<long long int,vector<string>>::reverse_iterator it=m.rbegin(); it!=m.rend(); ++it) {
            addToVector(p,it->second);
        }
      } else {
         for (map<long long int,vector<string>>::iterator it=m.begin(); it!=m.end(); ++it) {
            addToVector(p,it->second);
        }
     }
}

// this function returns its result as the last variable
void Interpreter::sortRowsString(int index, int rows, bool isDesc, vector<string> &v, vector<string> &p) {
     map<string,vector<string>> m;
     vector<string> w;
     string idx = "";
     for (int i = 0; i < (int) v.size(); i=i+rows) {
        w.clear();
        idx = "";
        for (int j = 0; j < rows; ++j) {
            w.push_back(v[i+j]);
            if (j==index-1) {
                idx = v[i+j];
            }
        }
        m[idx]= w;
     }
     if (isDesc) {
         for (map<string,vector<string>>::reverse_iterator it=m.rbegin(); it!=m.rend(); ++it) {
            addToVector(p,it->second);
        }
      } else {
         for (map<string,vector<string>>::iterator it=m.begin(); it!=m.end(); ++it) {
            addToVector(p,it->second);
        }
     }
}

vector<string> Interpreter::freq(vector<string> &dolist,vector<string> &forlist) {
    unordered_map<string, long long int> f;
    vector<string> result;
    string op = forlist[0];
    try {
        if (op == "unify") {
            for (int i = 0; i < (int) dolist.size(); i+=2) {
                string key = dolist[i];
                long long int val = stoll(dolist[i+1]);
                 if (debug_x) {logg.debug("processing " + key);};
                if (f.find(key) != f.end()) {
                   val += f.at(key);
                   f.find(key)->second = val;
                    if (debug_x) {logg.debug("found " + key);};
                } else {
                    f.insert(pair<string, long long int>(key, val));
                     if (debug_x) {logg.debug("insert " + key);};
                }
            }
            for (auto iter = f.begin(); iter != f.end(); ++iter){
                result.push_back(iter->first);
                result.push_back(to_string(iter->second));
            }
         } else if (op == "most") {
             if (debug_x) {logg.debug("start processing most");};
             long long int val = 0;
             vector<string> keys;
            for (int i = 0; i < (int) dolist.size(); i+=2) {
                string key2 = dolist[i];
                if (debug_x) {logg.debug("processing " + key2);};
                long long int val2 = stoll(dolist[i+1]);
                if ((val2 > val) || (i==0)){
                    val = val2;
                    keys.clear();
                    keys.push_back(key2);
                } else if (val2 == val) {
                    keys.push_back(key2);
                }
            }
            for (int j = 0; j < (int) keys.size(); j++) {
                result.push_back(keys[j]);
                result.push_back(to_string(val));
            }
        } else if (op == "least") {
             if (debug_x) {logg.debug("start processing least");};
             long long int val = 0;
             vector<string> keys;
            for (int i = 0; i < (int) dolist.size(); i+=2) {
                string key2 = dolist[i];
                 if (debug_x) {logg.debug("processing " + key2);};
                long long int val2 = stoll(dolist[i+1]);
                if ((val2 < val) || (i==0)) {
                    val = val2;
                    keys.clear();
                    if (debug_x) {logg.debug("found new min " + key2);};
                    keys.push_back(key2);
                } else if (val2 == val) {
                    keys.push_back(key2);
                    if (debug_x) {logg.debug("added new min " + key2);};
                }
            }
            for (int j = 0; j < (int) keys.size(); j++) {
                result.push_back(keys[j]);
                result.push_back(to_string(val));
            }
        }
    } catch (exception& e) {
        logg.msg(FATAL, cstos(e.what()) + ": could not execute " + op + " on freq.");
        stopExecution = 9;
    }

    return result;

}

int Interpreter::vmemsize(vector<string> v) {
    return sizeof(v) + sizeof(string) * v.capacity();
}

int Interpreter::mmemsize(unordered_map<int, vector<string>> slists) {
    int ms = 0;
    auto iter = slists.begin();
    while (iter != slists.end()) {
        ms +=  sizeof(iter->first) + sizeof(iter->second) * iter->second.capacity();
        ++iter;
    }
    return ms;
}
