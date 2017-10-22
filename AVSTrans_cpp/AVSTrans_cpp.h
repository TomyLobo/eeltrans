#pragma warning(disable:4786)
#include <string>
#include <map>
using namespace std;

enum EnumMode {
    mLinear = 0,
    mAssign,
    mExec,
    mPlus
};

string translate(string InputString, EnumMode defMode, bool defTransFirst);
//void SpecTrim(string& s);

extern map<void *, string> autoprefix;
extern string apepath;

#define AVSTransVer "1.25.00"
