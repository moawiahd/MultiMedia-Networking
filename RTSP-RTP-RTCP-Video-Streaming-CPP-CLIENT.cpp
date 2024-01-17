#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <termios.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include"CImg.h"
#include<thread>

using namespace cimg_library;
using namespace std;

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);

    // Format the timestamp into a human-readable date and time string
    std::stringstream ss;
    ss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

//Client side
int sequence=1;
int session1;
int Play=0;

//RTCP
//void Jitter();
int T2ts,T1ts,T1a,T2a;
int J2,J1=0;

//RTP
int RTPsn;


struct rtcp{
    unsigned int version:2;   /* protocol version */
    unsigned int p:1;         /* padding flag */
    unsigned int count:5;     /* varies by packet type */
    unsigned int pt;        /* RTCP packet type */
    int16_t length_h;
    //payload
    //Bye
    unsigned int ssrc=1234;
    unsigned int length:8;
    //RR
    unsigned int fraction;  /* fraction lost since last SR/RR */
    int last_seq;         /* extended last seq. no. received */
    int jitter;           /* interarrival jitter */
    //SR
    unsigned int rtp_ts;   /* RTP timestamp */
    unsigned int SSRC=1234;     /* sender generating this report */
    unsigned int psent;    /* packets sent */
    unsigned int osent;    /* octets sent */
 
};

struct RTP_packet{ 
     unsigned int version:2;        //version 2
     unsigned int padding:1;     
     unsigned int extension:1;              
     int SSRC=1234;                      //random number
     unsigned int Ptype:7;         
     unsigned int CSRC_count:4;  
     short int sequence_num;     
     int timestamp;      
     unsigned int marker:1;              //marker bit          
     char Data[100];              //payload 1 frame at a time
     
 };



char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

class rtsp1
{
  public:
    char type[50];
    char linkId[100];
    char version[50];
    int Seq_num;
    int session;
    int range;
    int port_num;

};
class rtsp2
{
  public:
    char version[50];
    int status_Code;
    char status[50];
    int Seq_num;
    int session;
};
void setup(int socket_ID,int port)
{
    rtsp1 r1;
    rtsp2 r2;
    string d="SETUP";
    r1.range=-1;
    strncpy(r1.type, d.c_str(), sizeof(r1.type));
    d="The Movie";
    strncpy(r1.linkId, d.c_str(),sizeof(r1.linkId));
    d="RTSP/1.0";
    strncpy(r1.version, d.c_str(),sizeof(r1.version));
    r1.Seq_num=sequence;
    r1.port_num=port;
    send(socket_ID, &r1, sizeof(r1),0);
    memset(&r2, 0, sizeof(r2));
    recv(socket_ID, (void *)&r2, sizeof(r2),0);
    cout<<r2.version<<" "<<r2.status_Code<<" "<<r2.status<<endl;
    cout<<"Seq_num: "<<r2.Seq_num<<endl;
    cout<<"session_num: "<<r2.session<<endl;
    cout << getCurrentTimestamp() << endl;
    session1=r2.session;
    ++sequence;
    Play=1;
    cout<<"\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    return;
}
void stop(int socket_ID)
{
    rtsp1 r1;
    rtsp2 r2;
    string d="TEARDOWN";
    strncpy(r1.type, d.c_str(),sizeof(r1.type));
    d="The Movie";
    strncpy(r1.linkId, d.c_str(),sizeof(r1.linkId));
    d="RTSP/1.0";
    strncpy(r1.version, d.c_str(),sizeof(r1.version));
    r1.Seq_num=sequence;
    r1.session=session1;
    send(socket_ID, &r1, sizeof(r1),0);
    memset(&r2, 0, sizeof(r2));
    recv(socket_ID, (void *)&r2, sizeof(r2),0);
    cout<<r2.version<<" "<<r2.status_Code<<" "<<r2.status<<endl;
    cout<<"Seq_num: "<<r2.Seq_num<<endl;
    cout<<"session_num: "<<r2.session<<endl;
    close(socket_ID);
    return;
}
void play(int socket_ID,int s=-1)
{
    rtsp1 r1;
    rtsp2 r2;
    string d="PLAY";
    strncpy(r1.type, d.c_str(),sizeof(r1.type));
    d="The Movie";
    strncpy(r1.linkId, d.c_str(),sizeof(r1.linkId));
    d="RTSP/1.0";
    strncpy(r1.version, d.c_str(),sizeof(r1.version));
    r1.Seq_num=sequence;
    r1.session=session1;
    r1.range=s;
    send(socket_ID, &r1, sizeof(r1),0);
    memset(&r2, 0, sizeof(r2));
    recv(socket_ID, (void *)&r2, sizeof(r2),0);
    cout<<r2.version<<" "<<r2.status_Code<<" "<<r2.status<<endl;
    cout<<"Seq_num: "<<r2.Seq_num<<endl;
    cout<<"session_num: "<<r2.session<<endl;
    ++sequence;
    Play=1;
    return;
}
void pause(int socket_ID)
{
    rtsp1 r1;rtsp2 r2;
    string d="PAUSE";
    strncpy(r1.type, d.c_str(),sizeof(r1.type));
    d="The Movie";
    strncpy(r1.linkId, d.c_str(),sizeof(r1.linkId));
    d="RTSP/1.0";
    strncpy(r1.version, d.c_str(),sizeof(r1.version));
    r1.Seq_num=sequence;
    r1.session=session1;
    send(socket_ID, &r1, sizeof(r1),0);
    recv(socket_ID, (void *)&r2, sizeof(r2),0);
    cout<<r2.version<<" "<<r2.status_Code<<" "<<r2.status<<endl;
    cout<<"Seq_num: "<<r2.Seq_num<<endl;
    cout<<"session_num: "<<r2.session<<endl;
    ++sequence;
    Play=0;
    return;
}



void RTP(int port)
{	

	struct sockaddr_in client_address;
	bzero((char*)&client_address, sizeof(client_address));
    	client_address.sin_family = AF_INET; 	// Convert IP to IPv4
    	client_address.sin_addr.s_addr = htonl(INADDR_ANY);	// Translate IP long unsigned int from host byte to network byte
    	client_address.sin_port = htons(port);	// Translate port long unsigned int from host byte to network byte
    	int client_side_socket = socket(AF_INET, SOCK_DGRAM, 0); // Establish connection to the server
	if(client_side_socket < 0)
    	{
        	cerr << "Error connecting to server" << endl;
        	exit(0);
    	}
    
    	int bind_status = bind(client_side_socket, (struct sockaddr*) &client_address, sizeof(client_address));
   	if(bind_status < 0)
    	{
        	cerr << "Error binding socket to local address!" << endl;
        	exit(0);
    	}
  
    	struct sockaddr_in new_socket_addr;
	struct RTP_packet pkt;
	
	char r[100];
    	CImgDisplay disp; 
    	unsigned int length ;
    	length = sizeof(new_socket_addr);
    	bool w=0,v=0;//RS
    	bool so=1;
    	while(so)
    	{
        	char name[1000];
        	if (recvfrom(client_side_socket, (char*)&pkt, sizeof(pkt), 0, (struct sockaddr *)& new_socket_addr, &length) == 0) // no more packets sent
        	{
        		close(client_side_socket); // disconnect
            		break;
        	}
        	
        	RTPsn = pkt.sequence_num;
        	CImg<unsigned char>img(pkt.Data);
        	disp=img;
        
    	}

    exit(0);
   
}

void BYE(rtcp BYEpkt){
    int l = 8 + sizeof(BYEpkt.ssrc);
    cout<<"~~~~~ RTCP BYE PKT INFO : ~~~~~"<<endl;
    cout<<"The SSRC is: "<<BYEpkt.ssrc<<endl;
    cout<<"The length of BYE pkt : "<<l<<endl;
    Play =0;

    
}

void SR(rtcp SRpkt){
    cout<<"~~~~~ SR PKT INFO : ~~~~~"<<endl;
    cout<<"The SSRC is: "<<SRpkt.SSRC<<endl;
    cout<<"The Octent sent : "<<SRpkt.osent<<endl;
    cout<<"The Time stamp of the pkt : "<<SRpkt.rtp_ts<<endl;
    cout<<"the number of PKT sent from the server :"<<SRpkt.psent;

}

void Jitter(){
    int delay;
    delay=(T2a-T1a)-(T2ts-T1ts);
    J2= J1+ (delay- J1)/16;
    
}

void RR(int clientSd,sockaddr_in clientAddr){
    rtcp rrr;
    rrr.version=2; rrr.p=0;rrr.pt=201;//dtat
    while(1){
    sleep(10);
    rrr.last_seq=RTPsn;
    rrr.jitter=J2;
    sendto(clientSd,   (char*)&rrr, sizeof(rrr), 0,(struct sockaddr *)&clientAddr, sizeof(clientAddr));
    }

}
void RTCP(int port)
{	
    struct sockaddr_in clientAddr;
    bzero((char*)&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(port);
    int clientSd = socket(AF_INET, SOCK_DGRAM, 0);
    if(clientSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    int bindStatus = bind(clientSd, (struct sockaddr*) &clientAddr, sizeof(clientAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }

    struct sockaddr_in newSockAddr;
    unsigned int len ;
    len= sizeof(newSockAddr);
  

    rtcp pkt;
    thread rr(RR,clientSd,clientAddr);

    while(1){

    memset(&pkt, 0, sizeof(pkt));
    recvfrom(clientSd,  (char*)&pkt, sizeof(pkt), 0, (struct sockaddr *)& newSockAddr, &len);

    if(pkt.pt==203)
    BYE(pkt); //2nd thread for rcv Bye

     if(pkt.pt==200)
     SR(pkt);
    
    }
    

    
}

int main(int argc, char *argv[])
{
    //we need 3 things: ip address and 2 port numbers, in that order
    if(argc != 4)
    {
        cout << "Please enter one IpAddress and 2 valid port" << endl; exit(0); 
    } //grab the IP address and port numbers 
    char *serverIp = argv[1]; int port = atoi(argv[2]); 
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    int clientSd1 = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd1,(sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl; return 0;
    }
    cout << "Connected to the server!" << endl;
    int rtpPort=atoi(argv[3]);
    
    
    thread first (RTP,rtpPort);

    setup(clientSd1,rtpPort);
    char input;
     cout<<"select what you want to do"<<endl;
    cout<<" 1   : for stop"<<endl;
    cout<<" 2   : for play"<<endl;
    cout<<" 3   : for reposition"<<endl;
	int s;
    int i=1;
   cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    while (i)
    {
    input=getch();
    switch (input)
    {
    case '1':
        stop(clientSd1);
        cout<<"stop"<<endl;
        i=0;
        break;
    case '2':
    if(!Play){
        play(clientSd1);
        cout<<"play"<<endl;
    }else{
        pause(clientSd1);
        cout<<"pause"<<endl;}
        break;
    case '3':
    	cout<<"Please choose a number from 1 -> 24 : "<<endl;
        cin>>s;
        while(1){
        if(s>24||s<1){
        cout<<"Please enetr valid number"<<endl;
        cin>>s;}
        else break;
        }
        play(clientSd1,s);
        cout<<"reposition   "<<s<<endl;
        break;
    default:
        cout<<"^^Please enter valid input^^"<<endl;
        break;
    }
    cout<<"---------------------------"<<endl;
    
    }
    close(clientSd1);
    
    cout << "Connection closed" << endl;
    exit(0);    
}
