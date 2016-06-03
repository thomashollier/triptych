#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOMXPlayer.h"
#include "ofxOscSender.h"
#include "ofxOscReceiver.h"
#include <unistd.h>
#include <sys/reboot.h>


#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>



#define SETTINGS_FILE "/home/pi/playback_settings.xml"
#define SOURCE_MOVIE "/var/ramdisk/source.mov"
#define PORT 32748
#define NUM_MSG_STRINGS 20

class triptychApp : public ofBaseApp{
    
public:
    
    void setup();
    void update();
    void draw();
    
    char master_ip[32];
    char slave1_ip[32];
    char slave2_ip[32];
    //enum Mode {MASTER, SLAVE1, SLAVE2, SLAVE};
    //Mode mode;
    

    int get_local_ip(char*);
    int get_ip_from_hostname(const char*, char*);
    
    // settings
    bool overlay;
    bool playfromram;

    struct display{
        string type;
        string hostname;
        string filename;
	bool isOnline;
	char ip[32];
	ofxOscSender senders[8];
    };

    void send_slave_checkin(display *) ;
    bool are_slaves_online(ofxOscReceiver *, vector<display>);
    void send_master_reply();
    void check_for_master_reply(ofxOscReceiver *, bool *);
    void send_control_message(ofxOscSender *, string);
    void check_for_messages();

    void doTogglePause();
    void doRewind();
    void doQuit();
    void doReboot();
    void doPoweroff();

    void movieTogglePause();
    void movieRewind();
    void triptychQuit();
    void machineReboot();
    void machinePoweroff();
    
    ofxOMXPlayer omxPlayer;
    void keyPressed(int key);
    void keyReleased(int key);

    
    
private:
    char hstnm[128];
    string hostname; 

    ofxOscReceiver	receiver;
    //ofxOscSender sender;
   // ofxOscSender sender1;
   // ofxOscSender sender2;
    
    int		current_msg_string;
    std::string	msg_strings[NUM_MSG_STRINGS];
    float		timers[NUM_MSG_STRINGS];
    
    bool		playbackOn;
    void prepMovie(string);
    void copyfile(const char *, char  *);
    
    void makeSettings();
    void readSettings();
    void printSettings();
    
    string movie;
    
    vector<display> displays;
    display master;
    vector<display> slaves;
    
    
};

