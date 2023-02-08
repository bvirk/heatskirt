#include <string>

class Shell {
	
public:
	ArdCon* exEnsurer;
	Shell();
	~Shell();

	void loop(std::string autoexecArgs="");
	std::string y2KTime();
private:
	void initTables();
	void delegate(std::string);
	static char ** character_name_completion(const char *text, int start, int end);
	static char *character_name_generator(const char *text, int state);
	int cmdNum(std::string cmd);
	void waitForNoInput(bool threadLeaveLoop=false);
	std::string string_replace(const std::string & s, const std::string & findS, const std::string & replaceS );
	std::string parse(const std::string& s);
};
