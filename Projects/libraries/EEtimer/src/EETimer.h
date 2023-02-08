#ifndef eetimer_h
#define eetimer_h
#include <time.h>
#define EETIMER_LOG_ADDRESS 0



class EETimer {

	enum timeIndex {TI_YY,TI_MM,TI_DD,TI_HH,TI_NN,TI_SS};

    uint32_t secSReset; 	    //! seconds sinse reset at the dateTime is set. 
	time_t dateTime; 		    //! seconds in y2k epoch
	uint32_t secSResetAtError;	//! seconds sinse reset at the time an error happends
    uint32_t errSource;		    //! setted by setError and cleared by setting time.
                                //! crc32 of source file or 0 for no error
    uint16_t errLineNr;         //! setted by setError - line number in source file
	
    static char ctimeBuf[26];
    
    /**
     * singleton use
     **/
    EETimer(); 

public:
//              _     _ _      
//  _ __  _   _| |__ | (_) ___ 
// | '_ \| | | | '_ \| | |/ __|
// | |_) | |_| | |_) | | | (__ 
// | .__/ \__,_|_.__/|_|_|\___|
// |_|                         


    static bool errHasBeenSet; // intial false to indicate obj has not been set to another value 
                               // than initial EEProm read



    /**
     *  @return time of error setted
     **/
    const char* cerrortime(); 
	
    /**
     *   @return
     **/
    char*clocktime(char hourminsep=':');
    
    /**
     *   @return
     **/
    char *ctime(time_t timestamp);


    /**
     *   @return
     **/
    char *ctime();
    

    /**
     * @return 
     **/
    uint32_t /* crc32*/ errorSource();
    
    
    /**
     * @return 
     **/
    uint16_t errorLine();


	/**
     * @return true if both errNr <> 0 and errHasBeenSet is true. After a boot errNr could be 
     * different from zero both will be false;
     **/
    bool hasSettedError();


    
    /**
     * @return singleton object reference
     **/
    static EETimer & instance();
    

    /**
     *  
     **/
    void setError(uint32_t errSrc, uint16_t errLNr=0);
    void setError(const char file[],uint16_t line );
    

    /**
     * Construct TimedErrorLog object from a timestamp string an EEPROM persitent it.
     *
     * @param timestamp of form [[[[yy.[mm].dd].]hh].nn] ss where non given default to prior 
     *   and optional '.' can be any non digits
     * @return is true on succesfull time set by a valid input
	 **/


    
    const char * setTime(char timestamp[]);

    uint32_t time();


  	/**
     * experimental boot construction
     *
     **/
    void writeEEProm();

    uint32_t crc32(const char cstr[]);


private:
/*************************************
                _            _       
     _ __  _ __(_)_   ____ _| |_ ___ 
    | '_ \| '__| \ \ / / _` | __/ _ \
    | |_) | |  | |\ V / (_| | ||  __/
    | .__/|_|  |_| \_/ \__,_|\__\___|
    |_|                              
***************************************/

};


#endif