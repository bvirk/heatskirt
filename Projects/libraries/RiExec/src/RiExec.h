#ifndef RiExec_h
#define RiExec_h

#define sizeofArray(x) (sizeof(x)/sizeof(x[0])) 


class RiExec {
public:
	#define COMMAND_SIZE 64
	char command[COMMAND_SIZE];		//! buffer seriel input as that line that get '\0' bytes to split in args
	#define MAX_ARG_COUNT 6         //! the command name inclusive - the 5' argument will contain remaining with spaces
	uint8_t argc;					//!< detected numbers of arguments
	char *argv[MAX_ARG_COUNT];		//!< array of pointer to '\0' bytes terminatet string(s) in command
	uint8_t cmdFuncIndex;				//!< the commands index
	uint8_t exitLevel;
	uint8_t cmdNPos;
	
	RiExec();
	
	void runCmd();
	//void consoleOut();
	//void getCmdAndTimeSlice();
	void loop();
	void loop(const char *line);
	bool serialEvent();
	int splittedCount(char str[], const char* delims);
	void catchSplits(char *items[], char str[],int cnt, bool ltrim);
};

#endif