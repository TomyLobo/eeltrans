#include <windows.h>
//#include <winuser.h>
//#include <winbase.h>
#ifdef _MSC_VER
#pragma warning( disable:4786 )
#endif
#include "AVSTrans_cpp.h"
#include <fstream>
#include <sstream>
#include <vector>
//#include <algorithm>
using namespace std;
#define StrVec vector<string>

enum EnumReplaceMode {
    rCaseSensitive = 0,
    rCaseInsensitive,
    rPattern,
    rCStyle
};

struct ConfigType {
    EnumMode sMode;
    bool sFilterComments;
    bool sTransFirst;
};

struct ReplacementType {
    string from;
    string to;
    EnumReplaceMode ReplaceMode;
};

bool DoAutoClipboard;
string VarPattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

vector<ReplacementType> Replacements;
map<void *, string> autoprefix;
string apepath;

string parsecommand(string input, int indent, EnumMode Mode);

// --- utility functions ---
string allautoprefix() {
	string retstr;
	for(map<void *, string>::iterator a = autoprefix.begin(); a != autoprefix.end(); ++a) {
		retstr.append(a->second);
		retstr.append(";\r\n");
	}
	return retstr;
}

void strtolower(string& InOutString) {
	for (string::size_type i = 0; i < InOutString.size(); ++i)
		InOutString[i]=tolower(InOutString[i]);
}

void Join(string& OutputString, StrVec& InputVector, const string& Delimiter) {
	switch (InputVector.size()) {
		case 0:
			OutputString = "";
			break;
/*		case 1:
			OutputString = InputVector[0];
			break;*/
		default:
			OutputString = InputVector[0];
			for (StrVec::size_type i = 1; i < InputVector.size(); ++i) {
				OutputString += Delimiter + InputVector[i];
			}
			break;
	}
}

void Split(StrVec& OutputVector, string input, string Delimiter, int limit = -1) {
	OutputVector.clear();
	if (limit < 0) {
		string::size_type lastpos = 0;
		string::size_type pos = input.find(Delimiter);
		while (pos != std::string::npos) {
			OutputVector.push_back(input.substr(lastpos, pos-lastpos));
			lastpos = pos+Delimiter.size();
			pos = input.find(Delimiter, lastpos);
		}
		OutputVector.push_back(input.substr(lastpos));
	} else if (limit > 0) {
		string::size_type lastpos = 0;
		string::size_type pos = input.find(Delimiter);
		for (int i = 1; (i < limit) && (pos != std::string::npos); ++i)  {
			OutputVector.push_back(input.substr(lastpos, pos-lastpos));
			lastpos = pos+Delimiter.size();
			pos = input.find(Delimiter, lastpos);
		}
		OutputVector.push_back(input.substr(lastpos));
	}
}

void SpecTrim(string& s) {
	string::size_type r = s.find_first_not_of(" \x0D\x0A\r\n");
	if (r != string::npos) {
		s.erase(0,r);
		r = s.find_last_not_of(" \x0D\x0A\r\n");
		if (r != string::npos) s.erase(r+1);
	} else {
		s = "";	
	}
}

void stringreplace(string& InOutString, const string& from, const string& to) {
	std::string::size_type pos = InOutString.find(from);
	while(pos != std::string::npos) {
		InOutString.replace(pos, from.size(), to);
		pos = InOutString.find(from, pos+to.size());
	}
}

void stringreplace_insensitive(string& InOutString, string& from, const string& to) {
	string tmp = InOutString;
	strtolower(tmp);
	strtolower(from);
	std::string::size_type pos = tmp.find(from);
	while(pos != std::string::npos) {
		InOutString.replace(pos, from.size(), to);
		tmp.replace(pos, from.size(), to);
		pos = tmp.find(from, pos+to.size());
	}
}

// thx @ Jaak Randmets from #finnish-flash for translating this procedure
string::size_type parsebracket(const string& input) {
	int Brackets = 0;
	for(string::size_type i = 0; i < input.size(); ++i) {
		switch(input[i]) {
			case '(':
				Brackets++;
				break;
			case ')':
				Brackets--;
				if(Brackets == 0) return i;
				break;
		}
	}
	return string::npos;
}

// --- end utility functions --- 

// --- replacement Functions ---

// skipped for now
void PatternReplace(string& InOutString, const string& from, const string& to) {
	
}

void CStyleReplace(string& InOutString, const string& from, const string& to) {
	StrVec::size_type i;
	string::size_type r, r2, j;
	StrVec Params, Parts;
	string CommandName;
	vector<StrVec::size_type> tokens;
	vector<string::size_type> lengths;
	string tmp;
	
	string rest;

	// Parse the macro identifier
	r = from.find('(');
	CommandName = from.substr(0, r);
	Split(Params, from.substr(r + 1, from.size() - r - 2), ","); // ?
	lengths.resize(Params.size());
	for (i = 0; i < Params.size(); ++i) {
		SpecTrim(Params[i]);
		lengths[i] = Params[i].size();
	}

	bool lastalpha, thisalpha = false;
	string::size_type lastpartpos = 0;
	// Parse the macro token string
	for (j = 0; j < to.size(); ++j) {
		lastalpha = thisalpha;
		thisalpha = VarPattern.find(to[j]) != string::npos;
		if (thisalpha && (!lastalpha)) {
			// this is the beginning of an alphanumeric token -> compare to the given tokens
			for (i = 0; i < Params.size(); ++i) {
				if (to.compare(j, lengths[i], Params[i])==0) {
					// check token length
					if (VarPattern.find(to[j + lengths[i]]) == string::npos) {
						// found one!
						string tmp2 = tmp = to.substr(lastpartpos, j - lastpartpos);
						SpecTrim(tmp2);
						if (!Parts.empty()) {
							if(tmp2.compare(0, 2, "##")==0) {
								r = tmp.find("##");
								tmp = tmp.substr(r + 2);
								SpecTrim(tmp);
							}
						}
						if (tmp2.size() > 2) {
							if (tmp2.compare(tmp2.size() - 2, 2, "##")==0) {
								r = tmp.rfind("##");
								tmp = tmp.substr(0, r);
								SpecTrim(tmp);
							}
						}
						Parts.push_back(tmp);
						tokens.push_back(i);
						lastpartpos = j + Params[i].size();
						break;
					} // if (VarPattern.find(to[j + lengths[i]]) == string::npos)
				} // if (to.compare(j, lengths[i], Params[i])==0)
			} // for i
		} // if (thisalpha && (!lastalpha))
	} // for j
	rest = to.substr(lastpartpos);
	if (!Parts.empty()) {
		tmp = rest;
		SpecTrim(tmp);
		if (tmp.compare(0, 2, "##")==0) {
			r = rest.find("##");
			rest = rest.substr(r + 2);
			SpecTrim(rest);
		}
	}
	StrVec::size_type NumMacroParams = Params.size();
	
	// We have now:
	// CommandName: Name of the current macro
	// Params(): Names of its parameters (not needed anymore)
	// Parts()/tokens()/rest: parts of the token string, to compose it from the parameters
	
	string com_mid, com_last, com_params;
	//Dim com_result As String
	tmp = "";
	r = InOutString.find(CommandName + '(');
	while(r != string::npos) {
		tmp += InOutString.substr(0, r);
		com_mid = InOutString.substr(r);

		r2 = parsebracket(com_mid);
		if (r2 != string::npos) {
			com_last = com_mid.substr(r2 + 1);
			com_mid = com_mid.substr(0, r2 + 1);

			// Now we can be sure that com_mid contains only the string that is to be replaced.
			com_params = com_mid.substr(CommandName.size() + 1, r2 - CommandName.size() - 1);
			string::size_type LastComma = 0;
			int Brackets = 0;
			Params.clear();
			for (j = 0; j < com_params.size(); ++j) {
				switch(com_params[j]) {
					case '(':
						Brackets++;
						break;
					
					case ')':
						Brackets--;
						break;
					
					case ',':
						if (Brackets == 0) {
							Params.push_back(com_params.substr(LastComma, j - LastComma));
							LastComma = j+1;
						}
				}
			} // for j
			Params.push_back(com_params.substr(LastComma));
			if (Params.size() == NumMacroParams) {
				// compose macro
				for (i = 0; i < Parts.size(); ++i) {
					tmp += Parts[i] + Params[tokens[i]];
				}
				tmp += rest;
			} else {
				// wrong paramcount -> go to next occurence
				tmp += com_mid;
			}
			InOutString = com_last;
			r = InOutString.find(CommandName + '(');
		} else { // if (r2 != string::npos)
			// possibly bracket error -> return the rest unchanged and exit
			tmp += com_mid + com_last;
			break;
		} // if (r2 != string::npos) else
	}
	if (r == string::npos) {
		// no further matches -> return the rest unchanged and exit
		tmp += InOutString;
	}
	InOutString = tmp;
}

void ClearReplacements() {
	Replacements.clear();
}

void AddReplacement(const string& from, const string& to, const EnumReplaceMode ReplaceMode) {
	ReplacementType tmp;
	tmp.from = from;
	tmp.to = to;
	tmp.ReplaceMode = ReplaceMode;
	Replacements.push_back(tmp);
}

// thx @ Jaak Randmets from #finnish-flash for translating this procedure
void DoReplacements(string& input) {
	for(StrVec::size_type i = 0; i < Replacements.size(); ++i) {
		ReplacementType tmp = Replacements[i];
		switch(tmp.ReplaceMode) {
			case rCaseSensitive:
				stringreplace(input, tmp.from, tmp.to);
				break;
				
			case rCaseInsensitive:
				stringreplace_insensitive(input, tmp.from, tmp.to);
				break;

			case rPattern:
				PatternReplace(input, tmp.from, tmp.to); 
				break;
				
			case rCStyle:
				CStyleReplace(input, tmp.from, tmp.to); 
				break;
		}
	}
}

// --- end replacement functions ---

// thx @ mandrill from #c++.de for translating this procedure
void mkexecs(StrVec& input) {
    unsigned i;
    for (i = 0; i + 2 < input.size(); ++i) {
        input[i] = "exec3(" + input[i] + "," + input[i+1] + "," + input[i+2] + ")";
        input.erase(input.begin() + i + 1, input.begin() + i + 3);
    }
    if (input.size() - i == 2) {
        input[i] = "exec2(" + input[i] + "," + input[i+1] + ")";
        input.erase(input.begin() + i + 1);
    }
}

void CommandSplit(StrVec& Output, string input, bool BracketCheck) {
	string::size_type LastSemicolon = 0, i;
	int Brackets = 0;
	Output.clear();
	input += ';';
	for (i = 0; i < input.size(); ++i) {
		switch(input[i]) {
            case '(':
				Brackets++;
				break;
            
			case ')':
				Brackets--;
				break;
            
			case ';':
                if (Brackets == 0) {
					Output.push_back(input.substr(LastSemicolon, i - LastSemicolon));
                    LastSemicolon = i + 1;
                }
				break;
		}
	}
	if (Brackets != 0) {
//		if (BracketCheck) {
			Output.clear();
//		} else {
//			input = input.substr(LastSemicolon, i - LastSemicolon);
//		}
		stringreplace(input, "=", "dummyequaldummy");
		stringreplace(input, ";", "dummysemicolondummy");
		stringreplace(input, "(", "[");
		stringreplace(input, ")", "]");
		ostringstream tmp; tmp << Brackets;
        Output.push_back("BRACKETS[" + tmp.str() + "]->" + input + "<-BRACKETS");
	}
}

// needs CommandSplit(d!), SpecTrim(d!), parsecommand(d!), mkexecs(d!)
string TransformCode(string input, int indent, EnumMode Mode, bool Parse) {
	StrVec InField, tmpField;
	string tmpItem;
	StrVec outField;
	string tmpLinearForm, tmpAssignForm, tmpExecForm, tmpPlusForm;
	CommandSplit(InField, input, false);
	outField.clear();
	for (StrVec::size_type i = 0; i < InField.size(); ++i) {
		tmpItem = InField[i];
		SpecTrim(tmpItem);
		if (!tmpItem.empty()) {
			tmpLinearForm += tmpItem + ";\r\n";
			tmpItem = parsecommand(tmpItem, indent, Mode);
			Split(tmpField, tmpItem, "=", 2); // a=b=c; changed here (3->2)
			switch(tmpField.size()) {
				case 1: // is already a command => no change
					break;
				case 2: // is an assignment => transform to assign()
					if (tmpField[1].find("=") != string::npos) { // a=b=c; added here
						tmpItem = "assign(" + tmpField[0] + "," + TransformCode(tmpField[1], indent, Mode, true) + ")";
					} else if (Parse || (tmpField[0].find("megabuf[") != string::npos) || (tmpField[0].find("megabuf(") != string::npos) ) {
						tmpItem = "assign(" + tmpField[0] + "," + tmpField[1] + ")";
					}
					break;
				default: // Syntax Error
					tmpItem = "ERR->" + tmpItem + "<-ERR";
			} // switch
			tmpAssignForm += tmpItem + ";\r\n";
			outField.push_back(tmpItem);
		} // if
	} // for
    /*tmpAssignForm = Left$(tmpAssignForm, Abs(Len(tmpAssignForm) - 3))*/
	if (Parse) {
        Join(tmpPlusForm, outField, "+\r\n" + string(indent, ' '));
		while (outField.size() > 1) {
			mkexecs(outField);
		}
		if(outField.empty()) {
			tmpExecForm = "";
		} else {
			tmpExecForm = outField[0];
		}
	} else {
        Join(tmpExecForm, outField, ";\r\n");
        Join(tmpPlusForm, outField, ";\r\n");
	}
    
	switch(Mode) {
		case mLinear:
			return tmpLinearForm;
		
		case mAssign:
			return tmpAssignForm;
		
		case mExec:
			return tmpExecForm;
		
		case mPlus:
			return tmpPlusForm;
		
		default:
			return tmpExecForm;
	}
}

// needs parsebracket(d!), TransformCode(d!), parsecommand(d!)
string parsecommand(string input, int indent, EnumMode Mode) {
	string tmpoutput;
	
	string::size_type r, r2, LastComma;
	StrVec::size_type i;
	int Brackets = 0;
	string CmdName, CmdParams;
	r = input.find('(');
	if (r != string::npos) {
		r2 = parsebracket(input);
		if (r2 == string::npos) {
			stringreplace(input, "=", "dummyequaldummy");
			stringreplace(input, ";", "dummysemicolondummy");
			stringreplace(input, "(", "[");
			stringreplace(input, ")", "]");
			return "BRACKETS[?]->" + input + "<-BRACKETS";
		}
		CmdName = input.substr(0, r);
        CmdParams = input.substr(r + 1, r2 - r - 1);
        tmpoutput = CmdName + "(";
        LastComma = 0;
		for (i = 0; i < CmdParams.size(); ++i) {
			switch(CmdParams[i]) {
                case '(':
					Brackets++;
					break;
                case ')':
					Brackets--;
					break;
                case ',':
                    if (Brackets == 0) {
                        tmpoutput += TransformCode(CmdParams.substr(LastComma, i - LastComma), indent + 1, Mode, true) + ',';
                        LastComma = i + 1;
                    }
					break;
            }
        }
		return tmpoutput + TransformCode(CmdParams.substr(LastComma), indent + 1, Mode, true) + ')' + parsecommand(input.substr(r2 + 1), indent, mExec);

	} else {
		return input;
	}
}
HINSTANCE g_hDllInstance;


string HandlePreprocessor(string Preprocessor) {
	StrVec tmp;
	Split(tmp, Preprocessor, " ", 3);
	if (tmp.size() > 0) {
		if (tmp[0] == "define") {
			if (tmp.size() == 3) {
				AddReplacement(tmp[1], tmp[2], (tmp[1].find('(') == string::npos) ? rCaseInsensitive : rCStyle);
			}
			return "";
		} else if (tmp[0] == "include") {
			if (tmp.size() >= 2) {
				string filename = apepath + '\\' + Preprocessor.substr(8);
				
				// open and read the file
				ifstream incifstr( filename.c_str() );
				if (incifstr) {
					ostringstream incstrstr;
					incstrstr << incifstr.rdbuf();
					return ";\r\n" + incstrstr.str() + "\r\n;";
				}
			}
		}
	}
	return "";
}

// thx @ Jaak Randmets from #finnish-flash for translating this procedure
void HandleComment(ConfigType& Config, string Comment) {
	/*
	 *	if comment starts with $ resume, else simply quit
	 */
	if(Comment[0] == '$') {
		string::size_type splitIndex = Comment.find('=');	// where assignment is
		string::size_type r;								// temporary index
		string tmp;											// temporary string
		
		if (splitIndex == string::npos) {
			
			/*
			 *	if we got only one string check if
			 *	doautoclipboard is mentioned
			 */
			strtolower(Comment);
			if(Comment == "$doautoclipboard") DoAutoClipboard = true;
		} else { // if (splitIndex == string::npos)
			string left  = Comment.substr(1, splitIndex-1);	// left part of the string
			string right = Comment.substr(splitIndex+1);	// right part of the string
			strtolower(left);
			/*
			 *	if both sides are mentioned
			 *	start checking cases, i dunno how could one pass
			 *	string to switch(), so im going to use ifs
			 */
			
			if(left == "avstrans_mode") {
				if     (right == "linear") Config.sMode = mLinear;
				else if(right == "assign") Config.sMode = mAssign;
				else if(right == "exec"  ) Config.sMode = mExec;
				else if(right == "plus"  ) Config.sMode = mPlus;
			} else if(left == "avstrans_filtercomments") {
				if((right == "0") || (right == "no") || (right == "off"))
					Config.sFilterComments = false;
				else if((right == "1") || (right == "on") || (right == "yes"))
					Config.sFilterComments = true;
			} else if(left == "avstrans_transfirst") {
				if((right == "0") || (right == "no") || (right == "off"))
					Config.sTransFirst = false;
				else if((right == "1") || (right == "on") || (right == "yes"))
					Config.sTransFirst = true;					
			} else if(left == "avstrans_replacement") {
				r = right.find("->");
				tmp = right.substr(r+2, right.size());
				if(!tmp.empty()) {
					AddReplacement(right.substr(0, r), tmp, rCaseInsensitive);
				}
			} else if(left == "avstrans_replacement_casesens") {
				r = right.find("->");
				tmp = right.substr(r+2, right.size());
				if(!tmp.empty()) {
					AddReplacement(right.substr(0, r-1), tmp, rCaseSensitive);
				}
			} else if(left == "avstrans_replacement_pattern") {
				r = right.find("->");
				tmp = right.substr(r+2, right.size());
				if(!tmp.empty()) {
					AddReplacement(right.substr(0, r-1), tmp, rPattern);
				}
			} else if(left == "avstrans_replacement_cstyle") {
				r = right.find("->");
				tmp = right.substr(r+2, right.size());
				if(!tmp.empty()) {
					AddReplacement(right.substr(0, r-1), tmp, rCStyle);
				}
			}
		} // if (splitIndex == string::npos) else
	}
}

// thx @ Jaak Randmets from #finnish-flash for translating this procedure
void ReadSettingsFromComments(ConfigType& Config, string& input) {
	string::size_type r, r2 = string::npos;
	r = input.find_first_of("/#");
	while((r != string::npos) && (r2 == string::npos)) {
		switch(input[r]) {
			case '/':
				switch(input[r + 1]) {
					case '*':
						r2 = input.find("*/", r);
						if(r2 != string::npos) {
							input.erase(r, r2 - r + 2);
							r2 = string::npos;
						} else {
							input.erase(r, input.size());
						}
						break;
					case '/':
						r2 = input.find("\r\n", r);
						if(r2 != string::npos) {
							HandleComment(Config, input.substr(r+2, r2 - r - 2));
							input.erase(r, r2 - r + 2);
							r2 = string::npos;
						} else {
							HandleComment(Config, input.substr(r+2));
							input.erase(r, input.size());
						}
						break;
					default:
						input.replace(r, 1, "fgsfoihjoadas");
						//input = input.substr(0, r) + "fgsfoihjoadas" + input.substr(r+2);
						r2 = string::npos;
						break;		
				}
				break;
			case '#':
				r2 = input.find("\r\n", r);
				if(r2 != string::npos) {
					input.replace(r, r2 - r + 2, HandlePreprocessor(input.substr(r+1, r2 - r - 1)));
					r2 = string::npos;
				} else {
					HandlePreprocessor(input.substr(r+1));
					input.replace(r, input.size(), HandlePreprocessor(input.substr(r+1, r2 - r - 1)));
				}
				
				break;
		}
		r = input.find_first_of("/#");
	}
    stringreplace(input, "fgsfoihjoadas", "/");
}

#define defFilterComments true
#define ReadCommentCodes true

//needs ReadSettingsFromComments(d!), DoReplacements(d?), TransformCode(d!)
string translate(string input, EnumMode defMode, bool defTransFirst) {
//Public Function translate(ByVal input As String, defMode As EnumMode, defFilterComments As Boolean, defTransFirst As Boolean, ReadCommentCodes As Boolean) As String
	static bool working;
    ConfigType Config;
	if (working) {
		return "error: translator busy";
	} else {
        input = allautoprefix() + "\r\n" + input;
		working = true;
        DoAutoClipboard = false;
        Config.sMode = defMode;
        Config.sFilterComments = defFilterComments;
        Config.sTransFirst = defTransFirst;
        ClearReplacements();
        if (ReadCommentCodes) {
            ReadSettingsFromComments(Config, input);
        }
        DoReplacements(input);
		input = TransformCode(input, 0, Config.sMode, Config.sTransFirst);
        input += ";\r\n";
		stringreplace(input, "dummyequaldummy","=");
		stringreplace(input, "dummysemicolondummy",";");
		stringreplace(input, "[","(");
		stringreplace(input, "]",")");
        working = false;
		return input;
    }
}
