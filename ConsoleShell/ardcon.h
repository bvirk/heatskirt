#include <string>
#include <exception>

const std::string configDir("/home/bvirk/documents/devel/myArduinoCore/ConsoleShell/"); 
const std::string cmdsMapTbl_FN(configDir+"ardShellCmds.tbl");
const std::string crc32Tbl_FN(configDir+"crc32.tbl");


class : public std::exception {
    virtual const char* what() const throw() {
    return "open";
  }
} openSerExcep;

class : public std::exception {
    virtual const char* what() const throw() {
    return "tcgetattr";
  }
} tcgetattrExcep;

class : public std::exception {
    virtual const char* what() const throw() {
    return "tcsetattr";
  }
} tcsetattrExcep;

#define redforeground "\033[31m"
#define yellowforeground "\033[33m"
#define blueforeground "\033[34m"
#define defaultforeground "\033[39m"

#define brightmode "\033[1m"
#define normalintensity "\033[22m"

class ArdCon {
	char 		inbuf[128];
	char 		cstrBuf[128];
	int 		serial_port;

	void 		dump(int obend, int red=256, int green=256, int blue=256);
	inline void readFailureTest(int readCnt);
	void 		execReply(std::string cmd);
	inline void execReply(unsigned fIx, std::string cmd);
	void initCrcTbl();
	
public:
	int 		timeOutCnt,cntRead;
	bool 		loopAgain, loopLeaved;

	ArdCon();
	~ArdCon();

	static void alertError(std::string);
	void 		init();
	void 		operator()();
	static void printYel(std::string);
	static void printRed(std::string);
	static void fmtError(std::string);
	void 		send(std::string);
	static int 	showError(const char *);
	
};
