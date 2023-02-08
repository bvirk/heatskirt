#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <map>
#include <termio.h>
#include <fcntl.h>
#include <errno.h>
#include <exception>
#include <ctime>
#include <atomic>
#include <cstring>
#include "ardcon.h"
#include <fstream>

using namespace std;

atomic<bool> permitRecieve(false);
ArdCon* ardCon=nullptr;
map<uint32_t, std::string>crc32Index;


void (*execReplFunc[])(string) = {
	 ArdCon::printYel   // Default
	,ArdCon::printRed
	,ArdCon::alertError
	,ArdCon::fmtError};
	


ArdCon::ArdCon() 
    :serial_port(-1)
    ,timeOutCnt(0)
    ,cntRead(0) 
	,loopAgain(true)
    ,loopLeaved(false) { 
	//cout << "Ardcon(" << this << ") construction\n";
	for (int i=0; i<256;i++)
		inbuf[i]=1;
	initCrcTbl();	
	
}

ArdCon::~ArdCon()  {
    if (serial_port != -1) {
        close(serial_port);
		serial_port=-1;
	}
	//cout << "Ardcon(" << this << ") destruction\n";
}


void ArdCon::initCrcTbl() {
	
	ifstream crctbl(crc32Tbl_FN.c_str());
    string line;
	while ( getline(crctbl, line)) { 
        int spacePos = line.find(" ");
        uint32_t crc32 = atoi(line.substr(0,spacePos).c_str());
        crc32Index.insert(make_pair(crc32,line.substr(spacePos+1)));
    }
    crctbl.close();
	
}



/**/
void ArdCon::alertError(string cmd) {
	int spacepos = cmd.find(' ');
	uint32_t crc = atol((cmd.substr(0,spacepos)).c_str());
	int lineNr = atoi(cmd.substr(spacepos).c_str());
	
	auto f = crc32Index.find(crc);
   	string fileName(f != crc32Index.end() ? f->second : "file not found" );
   
	cout << brightmode blueforeground "> " redforeground << fileName << ", line=" << lineNr << defaultforeground normalintensity << endl;
}
/**/
void ArdCon::dump(int obend, int red, int green, int blue) {
	cout << "\033[1m\033[4m   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\033[0m";
	char fmt[20];
	for (int p16=0; p16 < obend; p16 += 16) {
		printf("\n\033[1m\033[4m%02X\033[0m ",p16);
		for (int p=p16; p < p16+16; p++) {
			int purpur = red == blue ? blue : 256;
			int cyan = blue == green ? green : 256;
			int yellow = red == green ? green : 256;
			char colorDig =	p == purpur
				? '5'
				: p == cyan
					? '6'
					: p == yellow
						? '3'
						: p == red
							? '1'
							: p == green
								? '2'
								: p == blue
								? '4'
								: '\0';
			strcpy(fmt,colorDig ? "\033[3%cm%02hX\033[37m " : "%02hX " );
			
			if (colorDig)
				printf(fmt,colorDig,inbuf[p] & 0xff);
			else
				printf(fmt,inbuf[p] & 0xff);
		}
		printf("\n\033[1m\033[4m%02X\033[0m ",p16);
		for (int p=p16; p < p16+16; p++) {
			char ch=inbuf[p] & 0xff;
			char displCh = ch < 32 || ch==0x7f ?  '.' : ch;
			printf("%c  ",displCh);
		}
	}
	cout << endl;
}
/**/
void ArdCon::execReply(string cmd) {
	//printf("execreply\n");
	const int xorCrypt = 0xf5;
	uint8_t funcIndex = *cmd.c_str();
	bool checkXorCrypt = (*(uint8_t*)(cmd.c_str()+1) ^ xorCrypt) == funcIndex;
	if (funcIndex && checkXorCrypt && funcIndex <= sizeof(execReplFunc)/sizeof(execReplFunc[0]))
		execReply(funcIndex-1,cmd.substr(2));
	else
		printf("funcindex %d is out of range\n",funcIndex);
	//	(*execReplFunc[1])("index fail");
}
/**/
void ArdCon::execReply(unsigned base0funcIx, string cmd) {
		//while (!permitRecieve.load());
		(*execReplFunc[base0funcIx])(cmd);
}
/**/
void ArdCon::fmtError(string cmd) {
	cout << brightmode blueforeground "> " redforeground << "Printf format error" << defaultforeground normalintensity << endl;
}
/**/
void ArdCon::init() {
	//return true;
	ardCon->serial_port = open("/dev/ttyACM0", O_RDWR);
	if (ardCon->serial_port < 0) 
        throw openSerExcep;
	//cout << "serial port: "<< ardCon->serial_port << endl;
    //{
    //    initFunc = "open";
	//	return false;
	//}
	struct termios tty;
	
	if(tcgetattr(ardCon->serial_port, &tty) != 0) 
        throw tcgetattrExcep;
    
	//Control Modes (c_cflags)
	//========================

	tty.c_cflag &= ~PARENB; // no parity
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication 
	tty.c_cflag |= CS8; // 8 bits per byte
	
	//Hardware Flow Control (CRTSCTS)
	//tty.c_cflag |= CRTSCTS;  // Enable RTS/CTS hardware flow control
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control

	
	//CREAD and CLOCAL
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	

	//Local Modes (c_lflag)
	//======================

	tty.c_lflag &= ~ICANON;         // disable Canonical mode

	//ECHO
	tty.c_lflag &= ~ECHO;   // Disable echo
	tty.c_lflag &= ~ECHOE;  // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	
	//Disable Signal Chars
	tty.c_lflag &= ~(ISIG | IEXTEN);   // Disable interpretation of INTR, QUIT and SUSP


	//Input Modes (c_iflag)
	//=====================
	//Clearing IXOFF, IXON and IXANY disables software flow control, which we don’t want: ?
	// 1: don’t want clearing
	// 2: don-t want software control
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	
	//Disabling Special Handling Of Bytes On Receive
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	//Output Modes (c_oflag)
	//======================
	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	//VMIN and VTIME
	//==============
	tty.c_cc[VTIME] = 2;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;      // Allowed read to recieve 0 bytes.

	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	if (tcsetattr(ardCon->serial_port, TCSANOW, &tty) != 0) 
        throw tcsetattrExcep;
}
/**/
void ArdCon::operator()() {
	ardCon=this;
	int cstrBufIx=0;
	//cout << "thread started\n";
	//cout.flush();
	//int countdown=50;
	while (loopAgain) {
		while (!permitRecieve.load());
		int cntRead = read(serial_port, inbuf, sizeof(inbuf));
		readFailureTest(cntRead);
		//if (countdown) {
		//	countdown--;
		//	cout << '.';
		//	cout.flush();
		//}
		if (cntRead) {
			//dump(64);
			//cout.flush();
			for (int readIx=0; readIx < cntRead; readIx++) {
				char ch=inbuf[readIx];
				if (readIx && ch == '\004') {
					cstrBuf[cstrBufIx]='\0';
					execReply(cstrBuf);	
					cstrBufIx=0;
				} else {
					cstrBuf[cstrBufIx++] = ch;
					if (cstrBufIx == sizeof(cstrBuf)-1) {
						execReply(1,"buffer overrun - sending buffer forced\n");
						cstrBuf[cstrBufIx] = '\0';
						cstrBufIx=0;
						execReply(cstrBuf);
					}	
				}
			}
			timeOutCnt=0;
        } else  // read returned 0 bytes
			timeOutCnt += timeOutCnt<8; 
	}
	loopLeaved=true;
}
/**/
void ArdCon::readFailureTest(int readNr) { // bails out one time
	if (readNr==1) 
		execReply(1,"Read failusere - input may be lost\n");
}
/**/
void ArdCon::printYel(string cmd) {
	cout << brightmode blueforeground "> " yellowforeground << cmd << defaultforeground normalintensity << endl;
	//printf("%s%s%s\n",brightmode blueforeground "> " yellowforeground, cmd.c_str(),defaultforeground normalintensity);
}
/**/
void ArdCon::printRed(string cmd) {
	cout << brightmode blueforeground "> " redforeground << cmd << defaultforeground normalintensity << endl;
	//printf("%s%s%s\n",brightmode blueforeground "> " yellowforeground, cmd.c_str(),defaultforeground normalintensity);
}
/**/
void ArdCon::send(string cmd) {
    if (cmd.length())
    	write(serial_port, cmd.c_str(), cmd.length());
    
}
/**/
int ArdCon::showError(const char* what) {
    cout << "function " << what << " has errno " << errno << ", "  << strerror(errno) << endl;
    return 1;
}
