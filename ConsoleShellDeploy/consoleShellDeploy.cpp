#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unistd.h>
#include <cassert>
#include <vector>
#include <utility>
#include <sys/stat.h>
#include <filesystem>

using namespace std;
using namespace filesystem;


void toUpper(string &str) {
    for (auto beg = str.begin(); beg != str.end(); ++beg) {
        *beg = toupper(*beg);
    }
}

unsigned int modfTime(string fileName) {
	struct stat buf;
    return stat(fileName.c_str(),&buf) == 0 ? buf.st_mtime : 0;
}

bool isOlder(string /* exists */ fileName, string /* may exist */ comparedToFileName) {
	int cmpModfTime = modfTime(comparedToFileName); 
	return cmpModfTime? modfTime(fileName) < modfTime(comparedToFileName) : false;
}

uint32_t crc32(const char cstr[]) {
   auto *pstr=cstr;
   uint32_t crc = 0xFFFFFFFF;
   while (*pstr) {  
      crc = crc ^ *pstr++;
      for (int cnt = 8; cnt; cnt--)
         crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
   }
   return ~crc;
}



int main(int argc, char * argv[]) {

	

	// Directories
	// root directory for all files 
	const string ardDevelDir("/home/bvirk/documents/devel/myArduinoCore/");

	
	// directory of commands.cpp(input), commands.h(output), FunctionPointerList.h(output)
	const string libRiExecSrc("Projects/libraries/RiExec/src/");

	// directory of ardShellCmds.tbl(output) "crc32.tbl(output)
	const string consoleShellDir("ConsoleShell/");

	// directory of cpp files
	const string projectsLib("Projects/libraries");


	// file names i RiExec
	string Commands_cppFN(ardDevelDir+libRiExecSrc+"Commands.cpp");
	string Commands_hFN(ardDevelDir+libRiExecSrc+"Commands.h");
	string FunctionPointerList_hFN(ardDevelDir+libRiExecSrc+"FunctionPointerList.h");
	
	// filename in consoleshell
	string ardShellCmds_tblFN(ardDevelDir+consoleShellDir+"ardShellCmds.tbl");
	string crc32_tblFN(ardDevelDir+consoleShellDir+"crc32.tbl");

	// root path of cpp files
	string cppPathFN(ardDevelDir+projectsLib); 
	
	if (isOlder(Commands_cppFN,/*than*/ Commands_hFN)) {
		if (/* cause dummy arg given */ argc >1)
			cerr << "commands.h is uptodate - quiting update" << endl;
		return 0;
	}
	
	
	//Opening the only input streams
	
	ifstream Commands_cpp(Commands_cppFN);
	if (!Commands_cpp.is_open()) {
     	cerr << "Unable to open " << Commands_cppFN << '\n' ;
        return 1;
    }

	
	
	ofstream Commands_h(Commands_hFN);
	//cout << "defining vectors" << endl;
	vector<std::string> funcDecl;
    vector<pair<string,string>> funcNameAlias;
    vector<string> help;
    string line;

	regex funcDeclRegex("^uint8_t\\s+[a-zA-Z0-9]+\\(uint8_t\\s+argc,\\s+char\\s+\\*argv\\[\\]\\)\\s+\\{.*");
	regex helpRegex("^/\\*!.*");
	bool pushNext=false; 

    while ( std::getline(Commands_cpp, line)) {
		if (pushNext) {
			help.push_back(line);
			pushNext=false;
		}
		if (regex_match(line,helpRegex)) {
			pushNext=true;
			//cout << line << "matched" << endl;
		}
		if (regex_match(line,funcDeclRegex))
        	funcDecl.push_back(line);
	}
	regex minusCurly("\\s*\\{.*$");
	regex type("^uint8_t\\s+");
	regex afterBrace("\\(.+$");
	regex aliasDetect(".+\\{//[a-z]+$");
	regex aliasfilter("^.+\\{//");
	regex atDoxyfilter("^[\\s\\*]*@\\w+\\s*");

	//for (auto i = help.begin(); i != help.end(); i++)
	//	cout << *i << endl;
	//return 0;

	
	Commands_h << "#include <Arduino.h>" << endl;
	Commands_h << "\nnamespace cmdFuncs {" << endl;
	if (funcDecl.size()) {
		for (vector<string>::iterator i=funcDecl.begin();i!=funcDecl.end();i++) {
			string cmd(regex_replace(regex_replace(*i, type, ""),afterBrace,""));
			funcNameAlias.push_back(make_pair(
				 cmd
				,regex_match(*i,aliasDetect) ? regex_replace(*i,aliasfilter,"") : cmd));
			Commands_h << regex_replace(*i, minusCurly, ";") << endl;

		}
	}
	Commands_h << "};"<< endl;
	Commands_h.close();
	
	
	ofstream FunctionPointerList_h(FunctionPointerList_hFN);
	ofstream ardShellCmds_tbl(ardShellCmds_tblFN);
	FunctionPointerList_h << "uint8_t (*direct[])(uint8_t,char**) = {";
	string comma(" ");
	int exitcode=0;
	//for (vector<pair<string,string>>::iterator ip = funcNameAlias.begin(); ip != funcNameAlias.end(); ip++) {
	for (auto ip = funcNameAlias.begin(); ip != funcNameAlias.end(); ip++) {
		ardShellCmds_tbl << (*ip).second << " "; 
		string helpFront;
		string upHelpFront;
		if (help.size()) {
			upHelpFront = help.front();
			help.erase(help.begin());
			helpFront = regex_replace(upHelpFront,atDoxyfilter,"");
		} else {
			helpFront = "\"help index coruption\n\"";
			cerr << "some help string was missing" << endl;
			exitcode=1;
		}
		ardShellCmds_tbl << helpFront << endl;
		FunctionPointerList_h << comma << "cmdFuncs::" << (*ip).first;
		comma = ",";
	}
	ardShellCmds_tbl.close();
	FunctionPointerList_h << "};" << endl;
	FunctionPointerList_h.close();
	
	regex cppfile("^mksl\\s.+\\.cpp\\s*$");
	
	ofstream crc32_tbl(crc32_tblFN);
	const string ext(".cpp");
	for (const auto & entry : recursive_directory_iterator(cppPathFN)) {
		string fn(entry.path().filename());
		int len=fn.length();
		if (len >4 && !fn.compare(fn.length()-4,4,ext)) 
			crc32_tbl << crc32(fn.c_str()) << " " << fn << endl;
	}
	crc32_tbl.close();
	
	return exitcode;
}

