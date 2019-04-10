#include "AVSTrans_cpp.h"
#include <iostream>

int main(int argc, char* argv[]) {
	std::string input(
		(std::istreambuf_iterator<char>(cin)),
		std::istreambuf_iterator<char>()
	);
	cout << translate(input,mExec,false);
}
