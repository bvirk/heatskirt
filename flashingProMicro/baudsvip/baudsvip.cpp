#include <iostream>
#include <cstdio>
#include <exception>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <map>

/* 1200 baud 1/2 sec*/

using namespace std;

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

int serial_port;

void init(char * device, speed_t baudrateCode) {
    //cout << device << " baudcode: " << baudrateCode << endl;
    //return;
	
    serial_port = open(device, O_RDWR);
	if (serial_port < 0) 
        throw openSerExcep;
	//cout << "serial port: "<< ardCon->serial_port << endl;
    //{
    //    initFunc = "open";
	//	return false;
	//}
	struct termios tty;
	
	if(tcgetattr(serial_port, &tty) != 0) 
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

	cfsetispeed(&tty, baudrateCode);
	cfsetospeed(&tty, baudrateCode);
    
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) 
        throw tcsetattrExcep;
}

map<string,speed_t> baudNr({
     {"50",        B50}
    ,{"75",        B75}
    ,{"110",      B110}
    ,{"134",      B134}
    ,{"150",      B150}
    ,{"200",      B200}
    ,{"300",      B300}
    ,{"600",      B600}
    ,{"1200",    B1200}
    ,{"1800",    B1800}
    ,{"2400",    B2400}
    ,{"4800",    B4800}
    ,{"9600",    B9600}
    ,{"19200",  B19200}
    ,{"38400",  B38400}
    ,{"57600",  B57600}
    ,{"115200",B115200}
    ,{"230400",B230400}});




int syntax() {
    cout << "syntax\nbaudsvip device baudrate delayopen delayafterclose\n\ne.c.: baudsvip /dev/ttyACM0 1200 500 500\n"
            "opens /dev/ttyACM0 at 1200 baud in 500ms and waits 500ms before exit\n";
    return 0;
}

int baudsvipOpen(int argc, char *argv[]) {
    if (argc < 5)
        return syntax();
    int delayopen = atoi(argv[3]);
    int delayafter= atoi(argv[4]);
    
    auto speedIr = baudNr.find(argv[2]);
    if (speedIr ==baudNr.end() ) {
        cout << "uknown baud rate\nuse one of:\n";
        for (auto i= baudNr.begin(); i!= baudNr.end(); i++)
            printf("%5s\n",(*i).first.c_str());
        return 0;
    }
    try {
        init(argv[1],(*speedIr).second);
    } catch (exception  &e) {
        cout << e.what() << endl;
        if (serial_port > 0)
            close(serial_port);
        return 1;
    } 
    cout << "waiting " << delayopen << "ms open time\n";
    usleep(1000*delayopen);
    cout << "closing port, now waitng " << delayafter << "ms before exit\n";
    if (serial_port > 0)
        close(serial_port);
    usleep(1000*delayafter);
    return 0;
}


int main (int argc, char *argv[]) {
	switch(getopt(argc, argv, ":ht"))
	{ 
		case 'h':
			return syntax();
		case '?': 
			cout << "unknown option: " << char(optopt) << "\nuse -h for syntax\n";
			return 0;
	}
    return baudsvipOpen(argc, argv);
}
    
 