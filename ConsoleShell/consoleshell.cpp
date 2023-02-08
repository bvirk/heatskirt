#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <atomic>
#include <termio.h>
#include <fcntl.h>
#include <errno.h>
#include <exception>
#include <ctime>
#include <fstream>
#define UNIX_OFFSET 946684800

#include "ardcon.h"
#include "consoleshell.h"

using namespace std;

map<string,int> ardCmds;
map<int,string> helps;

extern atomic<bool> permitRecieve;
extern ArdCon* ardCon;


Shell::Shell() : exEnsurer(new ArdCon()) {
    while(ardCon == nullptr);
    //cout << "Shell(" << this << ") construction\n";
    exEnsurer->init();
    initTables();
    usleep(100000);
}

Shell::~Shell() {
    //cout << "Shell(" << this << ") destruction\n";
    delete exEnsurer;
}



/**/
char ** Shell::character_name_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, character_name_generator);
}
/**/
char * Shell::character_name_generator(const char *text, int state) {
    static map<string,int>::iterator wit;
    static int len;
    
    if (!state) {
        wit = ardCmds.begin();
        len = strlen(text);
    } 
    while (wit != ardCmds.end()) {
        const char *name = (*wit).first.c_str();
        wit++;
        if (strncmp(name, text, len) == 0) 
            return strdup(name);
    }
    return NULL;
}

/**/
void Shell::delegate(string args) {
    auto spacePos = args.find(' ');
    string cmd(args.substr(0,spacePos)); //OK with string::npos
    int cmdNr = cmdNum(cmd);
    if (cmdNr == -1) {
        if (cmd.length())
            cout << cmd << ": uknown command"<< endl;
        else
            waitForNoInput();
        return;
    }
    if (spacePos != string::npos 
        && args.find(" -h") != string::npos
        && helps.find(cmdNr) != helps.end()) {
            cout << (*helps.find(cmdNr)).second;
    } else {
        if (spacePos == string::npos ) 
            args="";
        else {
            while (spacePos < args.length() && args[spacePos] == ' ')
                spacePos++;
            args = args.substr(spacePos);
        }
        string mes(string({(char)(cmdNr+0x30)})+args+string("\n"));
        ardCon->send(mes);
        
    }
}

void Shell::initTables() {
    ifstream cmdTbl(cmdsMapTbl_FN);
    string line;
    int funcIndex=0;
    while ( getline(cmdTbl, line)) { 
        int spacePos = line.find(" ");
        ardCmds.insert(make_pair(line.substr(0,spacePos),funcIndex  ));
        helps.insert(make_pair(funcIndex++,parse(line.substr(spacePos+1) ) ));
    }
    cmdTbl.close();
}

/**/
void Shell::loop(string autoexecArgs) {
    
    rl_attempted_completion_function = character_name_completion;
    using_history();
    
    permitRecieve.store(true);
    
    
    if (autoexecArgs.length()) {
        waitForNoInput();
        delegate("autoexec " +autoexecArgs);
        
    }
    while (true) {
        waitForNoInput();
        char* input = readline("> ");
        if (!input)   // ctrl-d
            break;
        else
            delegate(input);
        add_history(input);
        free(input);
    }
    waitForNoInput(true);
    putchar('\n');
}

string Shell::parse(const string& s) {
    static vector< pair< string, string > > patterns = {
        { "\\\\" , "\\" },
        { "\\n", "\n" },
        { "\\r", "\r" },
        { "\\t", "\t" },
        { "\\\"", "\"" }
    };
    string result = s;
    for ( const auto & p : patterns ) {
        result = string_replace( result, p.first, p.second );
    }
    return result;
}

string Shell::string_replace( const string & s, const string & findS, const std::string & replaceS )
{
    string result = s;
    auto pos = s.find( findS );
    if ( pos == string::npos ) {
        return result;
    }
    result.replace( pos, findS.length(), replaceS );
    return string_replace( result, findS, replaceS );
}

/**/
void Shell::waitForNoInput(bool threadLeaveLoop) {
    ardCon->timeOutCnt=0;
    //permitRecieve.store(true);
    while ( ardCon->cntRead || ardCon->timeOutCnt < 8 );
        //usleep(100);
    if (threadLeaveLoop) {
        ardCon->loopAgain=false;
        //while(!ardCon->loopLeaved);
    }
}
/**/
string Shell::y2KTime() {
    char buf[20];
    snprintf(buf,20,"%ld",time(nullptr)- UNIX_OFFSET);
    return string(buf);
}

int Shell::cmdNum(string cmd) {
    auto iPair = ardCmds.find(cmd);
    return iPair == ardCmds.end() ? -1 : (*iPair).second;
}

int main(int argc, char * argv[]) {
    
    thread printer((ArdCon()));
        
    try {
        Shell sh;
        cout << "ctrl-d for leaving" << endl;
        sh.loop(sh.y2KTime());
        
    } catch (exception& e) {
        permitRecieve.store(true);
        ardCon->loopAgain=false;
        ArdCon::showError(e.what());
    }
    printer.join();
    return 0;    
}
