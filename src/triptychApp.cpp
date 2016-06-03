#include "triptychApp.h"


//--------------------------------------------------------------
void triptychApp::setup()
{
    //makeSettings();
    readSettings();
    printSettings();
    
    
	gethostname(hstnm, sizeof hstnm);
	hostname = hstnm;

	std::cout << "\n--------\nMy hostname is " << hostname << "\n--------\n" <<  hostname << endl;

	//--------------- load movie paused
	prepMovie(hostname);
	if (!overlay){
		ofToggleFullscreen();  
	}
	string videoPath = ofToDataPath(movie, true);
	ofxOMXPlayerSettings settings;
	settings.videoPath = videoPath;
	settings.useHDMIForAudio = true;	//default true
	settings.enableTexture = false;		//default true
	settings.enableLooping = true;		//default true
	settings.enableAudio = true;		//default true, save resources by disabling
	omxPlayer.setup(settings);
	
	omxPlayer.setPaused(true);
	playbackOn = false;

	//-------------- sync the machines
	receiver.setup( PORT );

	if ( hostname.compare(master.hostname) == 0){
		std::cout << "Running as master instance\n";
		// wait to hear from slave
		bool waiting_for_slaves = True;
    		for(int i = 0; i<slaves.size(); i++){
			slaves[i].isOnline = false;
			get_ip_from_hostname(slaves[i].hostname.c_str(), slaves[i].ip);
			master.senders[i].setup(slaves[i].ip, PORT);
		}
		while ( waiting_for_slaves ){
			usleep(10000);
			if (are_slaves_online(&receiver, slaves)){
				send_master_reply();
				waiting_for_slaves = false;
			}
		}
		std::cout << "I am the master and I am now in control\n";
		std::cout << "setup complete\n";
	}else{
    		for(int i = 0; i<slaves.size(); i++){
			if ( hostname.compare(slaves[i].hostname) == 0){
				std::cout << "Running as slave instance\n";
				get_ip_from_hostname(master.hostname.c_str(), master.ip);
				slaves[i].senders[0].setup(master.ip, PORT);
				bool is_master_in_control = false;
				std::cout << "Waiting to hear from master so I can give up control.\n";
				while ( ! is_master_in_control ){
					usleep( 10000 );
					send_slave_checkin( &slaves[i] );
					check_for_master_reply(	&receiver, &is_master_in_control );
				}
				std::cout << "I am the slave and I am now controlled by the master\n";
				std::cout << "setup complete\n";
			}
		}
	}
	
	// un-pause
	if ( hostname.compare(master.hostname) == 0){
		usleep(3000000);
		doTogglePause();
	}

	ofHideCursor();
}


//--------------------------------------------------------------
void triptychApp::update()
{
	if(omxPlayer.getCurrentFrame() == omxPlayer.getTotalNumFrames()-1 && hostname.compare(master.hostname) == 0){
		std::cout << "Got to end of movie, rewinding...\n";
		doTogglePause();
		doRewind();
	}
	check_for_messages();

	// hide old messages
	for ( int i=0; i<NUM_MSG_STRINGS; i++){
		if ( timers[i] < ofGetElapsedTimef() ){
			msg_strings[i] = "";
		}
	}		

}


//--------------------------------------------------------------
void triptychApp::draw(){
	if(overlay){
		omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
		//omxPlayer.draw(0, 0, 500, 500);
	
		//draw a smaller version in the lower right
		int scaledHeight	= omxPlayer.getHeight()/4;
		int scaledWidth		= omxPlayer.getWidth()/4;
		omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);

		ofDrawBitmapStringHighlight(omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
	}else{
		omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
}

void triptychApp::keyPressed(int key){
	if (key == 32 ){
		std::cout << "ready to toggle pause...\n";
	}else if( key == 114 ){
		std::cout << "ready to rewind...\n";
	}else if( key == 101 ){
		std::cout << "ready to quit\n";
	}else{
		std::cout << "pressed " << key << "\n";
	}
}

void triptychApp::keyReleased(int key){
	if (key == 32){
		doTogglePause();	
	}else if( key == 114 ){
		doRewind();
	}else if( key == 113 ){
		doQuit();
	}else if( key == 119 ){
		doReboot();
	}else if( key == 101 ){
		doPoweroff();
	}else{
		std::cout << "released " << key << "\n";
	}
}

void triptychApp::doTogglePause(){
	std::cout << "triggered doTogglePause.\n";	
	for(int i=0;i<slaves.size();i++){
		send_control_message( & master.senders[i], "pause");	
	}
	movieTogglePause();
	usleep(10000);
}

void triptychApp::doRewind(){
	std::cout << "triggered doRewind.\n";
	for(int i=0;i<slaves.size();i++){
		send_control_message( & master.senders[i], "rewind");	
	}
	movieRewind();
	usleep(10000);
}

void triptychApp::doQuit(){
	std::cout << "triggered doQuit.\n";
	for(int i=0;i<slaves.size();i++){
		send_control_message( & master.senders[i], "quit");	
	}
	triptychQuit();
}

void triptychApp::doReboot(){
	std::cout << "triggered doReboot.\n";
	for(int i=0;i<slaves.size();i++){
		send_control_message( & master.senders[i], "reboot");	
	}
	machineReboot();
}

void triptychApp::doPoweroff(){
	std::cout << "triggered doPoweroff.\n";
	for(int i=0;i<slaves.size();i++){
		send_control_message( & master.senders[i], "poweroff");	
	}
	machinePoweroff();
}

void triptychApp::send_control_message(ofxOscSender *sender, string messageStr){
        ofxOscMessage m;
        m.setAddress("/control");
	m.addStringArg(messageStr);
        (*sender).sendMessage(m);
}

void triptychApp::check_for_messages(){
	while(receiver.hasWaitingMessages() )
	{
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage( &m );

		if ( m.getAddress().compare("/control") == 0 )
		{
			string command = m.getArgAsString(0);
			std::cout << "got the control message: " << command << "\n";
			if(command.compare("pause")==0){
				movieTogglePause();
				break;
			}else if(command.compare("rewind")==0){
				movieRewind();
				break;
			}else if(command.compare("quit")==0){
				triptychQuit();
				break;
			}else if(command.compare("reboot")==0){
				machineReboot();
				break;
			}else if(command.compare("poweroff")==0){
				machinePoweroff();
				break;
			}	
        	}
	}	
}

void triptychApp::movieTogglePause(){
		std::cout << hostname << ": movieTogglePause\n";
		omxPlayer.setPaused(!omxPlayer.isPaused());
}

void triptychApp::movieRewind(){
		std::cout << hostname << ": movieRewind\n";
		omxPlayer.setPaused(true);
		omxPlayer.seekToTimeInSeconds(0);
		omxPlayer.setPaused(false);
}

void triptychApp::triptychQuit(){
		std::cout << "Quitting\n";
		ofExit(0);
}

void triptychApp::machineReboot(){
		std::cout << "Rebooting\n";
		system("sudo reboot");
		std::cout << "Rebooting command given\n";
}

void triptychApp::machinePoweroff(){
		std::cout << "Poweroffing\n";
		system("sudo halt");
		std::cout << "Powering off command given\n";
}

int triptychApp::get_local_ip(char *ip)
{
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;

	getifaddrs (&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family==AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			addr = inet_ntoa(sa->sin_addr);
			//printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
			if (strcmp(ifa->ifa_name, "eth0") == 0){
				strcpy(ip, addr);
			}
		}
	}
	freeifaddrs(ifap);
	return 0;
}

int triptychApp::get_ip_from_hostname(const char *hostname , char *ip)
{
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;
 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
 
    if ( (rv = getaddrinfo( hostname , "http" , &hints , &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
 
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        h = (struct sockaddr_in *) p->ai_addr;
        strcpy(ip , inet_ntoa( h->sin_addr ) );
    }
     
    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

void triptychApp::send_slave_checkin(display *d){
	string addressString = "/" + d->hostname;
	ofxOscMessage m;
	m.setAddress(addressString);
	d->senders[0].sendMessage(m);
	//std::cout << "Sent slave checkin message: " << addressString << "\n";

}

bool triptychApp::are_slaves_online(ofxOscReceiver * receiver,  vector<display> slvs){
	// go through all the messages
	while((*receiver).hasWaitingMessages() )
	{
		ofxOscMessage m;
		(*receiver).getNextMessage( &m );
		for(int i = 0; i< slvs.size(); i++){
			// mark all the displays that are online
			if ( m.getAddress().compare( string("/").append(slvs[i].hostname)) == 0 ){
				slvs[i].isOnline = true;
				std::cout << slvs[i].hostname << " is online\n";
				break;
			}
		}
	}
	// if any display is not online, return false
	for(int i = 0; i< slvs.size(); i++){
		if(!slvs[i].isOnline){
			return false;
		}
	}
	//  otherwise return true
	std::cout << "both are online\n";
	return true;
}


void triptychApp::send_master_reply(){
	ofxOscMessage m;
	string msg = "/" + master.hostname;
	m.setAddress(msg);
    	for(int i = 0; i<slaves.size(); i++){
		master.senders[i].setup(slaves[i].ip, PORT);
		master.senders[i].sendMessage(m);
	}
}

void triptychApp::check_for_master_reply(ofxOscReceiver * receiver, bool * is_master_in_control){
        while((*receiver).hasWaitingMessages() )
        {
                // get the next message
                ofxOscMessage m;
                (*receiver).getNextMessage( &m );
		std::cout << m.getAddress() << endl;
		string s = "/"+master.hostname;

                if ( s.compare(m.getAddress()) == 0 )
                {
                        *is_master_in_control= true;
                }
	}
	if ( *is_master_in_control ){
		std::cout << "Heard from master.\n";
	}
}

void triptychApp::prepMovie(string hostname){

	string hstnm = hostname;
	movie = SOURCE_MOVIE;

	if(playfromram){
		if( hstnm.compare(master.hostname) == 0){
			std::cout << "copying " << master.filename  << " to " << SOURCE_MOVIE<< "\n";
			copyfile(master.filename.c_str(), SOURCE_MOVIE);
		}else{
    			for(int i = 0; i<slaves.size(); i++){
				 if( hstnm.compare(slaves[i].hostname) == 0){
					std::cout << "copying " << slaves[i].filename << " to " << SOURCE_MOVIE<< "\n";
					copyfile(slaves[i].filename.c_str(), SOURCE_MOVIE);
				}
			}
		}
	}else{
		if( hstnm.compare(master.hostname) == 0){
			movie = master.filename;
			std::cout << "playing back " << master.filename << "\n";
		}else{
    			for(int i = 0; i<slaves.size(); i++){
				 if( hstnm.compare(slaves[i].hostname) == 0){
					movie = slaves[i].filename;
					std::cout << "playing back " << slaves[i].filename << "\n";
				}
			}
		}
	}
}


void triptychApp::copyfile(const char * src, char * dst){
	const int size = 16384;
	char buffer[size];

	FILE * srcf = fopen(src, "rb");
	FILE * dstf = fopen(dst, "wb");

	while(!feof(srcf))
	{
		int n = fread(buffer, 1, size, srcf);
		fwrite(buffer, 1, n, dstf);
	}
	
	fflush(dstf);
	fclose(dstf);
	fclose(srcf);
}

void triptychApp::readSettings(){
	std::cout << "Reading settings\n";
    ofxXmlSettings settings;
    if(settings.loadFile(SETTINGS_FILE)){
        settings.pushTag("displays");
        int numberOfDisplays = settings.getNumTags("display");
        for(int i = 0; i < numberOfDisplays; i++){
            settings.pushTag("display", i);
            display d;
            d.type = settings.getValue("type", "no type value was read");
            d.hostname = settings.getValue("hostname", "no hostname value was read");
            d.filename = settings.getValue("filename", "no filename value was read");
            displays.push_back(d);
            
            if( strcmp(d.type.c_str(), "master") == 0){
                master = d;
            }
            if( strcmp(d.type.c_str(), "slave") == 0){
                slaves.push_back(d);
            }
            settings.popTag();
        }
        settings.popTag(); //pop position
	settings.pushTag("settings");
	string ol = settings.getValue("overlay", "true");
	if( ol.compare("true") == 0){	
		overlay = true;
	}else if( ol.compare("false") == 0){
		overlay = false;
	}else{
		std::cout << "overlay setting needs to be true or false\n";
	}		
	ol = settings.getValue("playfromram", "true");
	if( ol.compare("true") == 0){	
		playfromram= true;
	}else if( ol.compare("false") == 0){
		playfromram= false;
	}else{
		std::cout << "playfromram setting needs to be true or false\n";
	}		
        settings.popTag(); //pop position
    }
}

void triptychApp::makeSettings(){
std::cout << "making settings file\n";

    ofxXmlSettings positions;
    positions.addTag("displays");
    positions.pushTag("displays");
    
    positions.addTag("display");
    positions.pushTag("display",0);
    positions.addValue("type", "master");
    positions.addValue("hostname", "master");
    positions.addValue("filename", "/home/pi/video/master.mov");
    positions.popTag();//pop display
    positions.addTag("display");
    positions.pushTag("display",1);
    positions.addValue("type", "slave");
    positions.addValue("hostname", "slave1");
    positions.addValue("filename", "/home/pi/video/slave1.mov");
    positions.popTag();//pop display
    positions.addTag("display");
    positions.pushTag("display",2);
    positions.addValue("type", "slave");
    positions.addValue("hostname", "slave2");
    positions.addValue("filename", "/home/pi/video/slave2.mov");
    positions.popTag();//pop display
    
    positions.popTag(); //pop position
    positions.saveFile(SETTINGS_FILE);
}

void triptychApp::printSettings(){
	std::cout << "printing settings\n";
    std::cout << "displays:\n";
    for(int i = 0; i<displays.size(); i++){
        std::cout << "name: " << displays[i].hostname << "\n";
        std::cout << "type: " << displays[i].type << "\n";
        std::cout << "file: " << displays[i].filename << "\n";
    }
    std::cout << "\nmaster:\n";
    std::cout << "name: " <<  master.hostname << "\n";
    std::cout << "type: " <<  master.type << "\n";
    std::cout << "file: " <<  master.filename << "\n";
    std::cout << "\nslaves:\n";
    for(int i = 0; i<slaves.size(); i++){
        std::cout << "name: " <<  slaves[i].hostname << "\n";
        std::cout << "type: " <<  slaves[i].type << "\n";
        std::cout << "file: " <<  slaves[i].filename << "\n";
    }

    std::cout << "\nsettings:\n";
    std::cout << "overlay: " << overlay << "\n";
    std::cout << "playfromram: " << playfromram << "\n";

}

