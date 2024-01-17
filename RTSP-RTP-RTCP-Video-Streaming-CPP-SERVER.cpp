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
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <dirent.h>
#include"CImg.h"
using namespace cimg_library;
using namespace std;



//Server side
int sequence;
int session1=4231;
int reposition=0;

bool repo_Flag=0;
bool Play=1;
bool Stop=0;

int numImages = 0;
long long totalSize = 0;

//Bye
bool B=0;


//RTCP
int Tsent,seqn,counter=0;

string getCurrentTimestamp() {
time_t now = time(nullptr);
tm* timeinfo = localtime(&now);

    
    stringstream ss;
    ss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void fileSize_numImages(){
DIR* directory = opendir("vid");
if (directory) {
    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG) { // Check if it's a regular file
            numImages++; // Increment the number of images

            // file path
            string filePath = "vid/";
            filePath += entry->d_name;

            // Calculate the size of the image file
            FILE* imageFile = fopen(filePath.c_str(), "rb");
            if (imageFile) {
                fseek(imageFile, 0, SEEK_END);
                long long imageSize = ftell(imageFile);
                totalSize += imageSize;
                fclose(imageFile);
            }
        }
    }
    closedir(directory);
    
    cout<<"Pkts_num: "<<numImages<<" & Total_size: "<<totalSize<<" Bytes"<<endl;
    
}}

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

void play(int socket_ID,int rp)
{
    rtsp2 r2;
    string d="RTSP/1.0";
    strncpy(r2.version, d.c_str(),sizeof(r2.version));
    r2.status_Code=200;
    d="OK";
    strncpy(r2.status, d.c_str(),sizeof(r2.status));
    r2.session=session1;
    r2.Seq_num=sequence;
    send(socket_ID, &r2, sizeof(r2), 0);
    reposition =20*rp;
    if(rp!=-1)
    repo_Flag=1;
    else 
    repo_Flag=0;
    Play=1;
    return;
}
void pause(int socket_ID)
{
    rtsp2 r2;
    string d="RTSP/1.0";
    strncpy(r2.version, d.c_str(),sizeof(r2.version));
    r2.status_Code=200;
    d="OK";
    strncpy(r2.status, d.c_str(),sizeof(r2.status));
    r2.session=session1;
    r2.Seq_num=sequence;
    send(socket_ID, &r2, sizeof(r2), 0);
    Play=0;
    return;
}
void stop(int socket_ID)
{
    rtsp2 r2;
    string d="RTSP/1.0";
    strncpy(r2.version, d.c_str(),sizeof(r2.version));
    r2.status_Code=200;
    d="OK";
    strncpy(r2.status, d.c_str(),sizeof(r2.status));
    r2.session=session1;
    r2.Seq_num=sequence;
    send(socket_ID, &r2, sizeof(r2), 0);
    Stop=1;
    return;
}

void BYE(int serverSd, sockaddr_in sendSockAddr){
    
    while(1){
    if(B==1){
    struct rtcp Bye;
    Bye.version=2; Bye.p=0; Bye.length=sizeof(Bye); Bye.pt=203;
    sendto(serverSd,   (char*)&Bye, sizeof(Bye), 0,(struct sockaddr *)&sendSockAddr, sizeof(sendSockAddr));
    B=0; 
    } }

}  

void SR(int serverSd,sockaddr_in sendSockAddr){
    rtcp SR;
    SR.version=2; SR.p=0; SR.pt=200;
    while(1){
    sleep(6);
    SR.rtp_ts=Tsent;
    SR.SSRC=seqn;
    SR.psent = counter;
    sendto(serverSd,   (char*)&SR, sizeof(SR), 0,(struct sockaddr *)&sendSockAddr, sizeof(sendSockAddr));    
    }

}

void RR(int serverSd){
   rtcp rr;
    struct sockaddr_in newSockAddr1;
    unsigned int len ;
    len= sizeof(newSockAddr1);
   while(1){
  memset(&rr, 0, sizeof(rr));
  recvfrom(serverSd,  (char*)&rr, sizeof(rr), 0, (struct sockaddr *)& newSockAddr1, &len);
  
   cout<<"~~~~~ RTCP RR PKT INFO : ~~~~~"<<endl;
    cout<<"The last sequence number is: "<<rr.last_seq<<endl;
    cout<<"The Jitter is : "<< rr.jitter<<endl;
   }

}

void RTCP(int port)
{	
    char c[100];
	string da="127.0.0.1";
    strcpy(c, da.c_str());
    char *clientIp =c ;
    struct hostent* host = gethostbyname(clientIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr = *((struct in_addr*)host->h_addr);

    sendSockAddr.sin_port = htons(port);
    int serverSd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverSd<0)cout<<"error"<<endl;


    thread bye(BYE,serverSd,sendSockAddr);
    thread sr(SR,serverSd,sendSockAddr);
    thread rr(RR,serverSd);


}

void RTP(int port)
{	

	char ch[100];
	string dest_addr="127.0.0.1";
    	strcpy(ch, dest_addr.c_str()); // copy string to array
    	char *client_ip = ch ; 	// store array in client_ip
    	struct hostent* host = gethostbyname(client_ip); // returns struct of type hostent for given hostname
    	sockaddr_in send_sock_addr;   
    	bzero((char*)&send_sock_addr, sizeof(send_sock_addr)); 
    	send_sock_addr.sin_family = AF_INET; 
    	send_sock_addr.sin_addr = *((struct in_addr*)host->h_addr);
    	send_sock_addr.sin_port = htons(port);
   	int server_side = socket(AF_INET, SOCK_DGRAM, 0); //connect to socket

    	struct RTP_packet pkt;
    	char a[100];
    	int RTP_sequenceNo=0;
    	pkt.CSRC_count=0; pkt.marker=0; pkt.padding=0;pkt.Ptype=26;pkt.version=2;pkt.SSRC=1234;pkt.extension=0;
	unsigned int s_length ;
	s_length = sizeof(send_sock_addr);
	
    	for(int i=1;i<=500;i++)
    	{
    		while(Play < 1){}
        	sprintf(pkt.Data,"vid/image%03d.jpg",i); // print image sequence
        
        	if(Stop == 1)
        	{
        		close(server_side);
        		break;
        	}
        	
        	RTP_sequenceNo++;
        	seqn = pkt.sequence_num = RTP_sequenceNo;
        	Tsent = pkt.timestamp = time(NULL);

		sendto(server_side, (char*)&pkt, sizeof(pkt), 0,(struct sockaddr *)&send_sock_addr, sizeof(send_sock_addr));
        	counter++;
   
        	if(repo_Flag == 1)
        	{
        		i = reposition;
        		repo_Flag = 0;
        	}
        	
        	if(Stop == 1)
        	break;
        	
        	if(i==500)
        	{
            		i=0;
            		Play=0;
            		B=1;
        	}
        
        	usleep(50000);
    }
    
}

void RTSP(int socket_ID, int port)
{
    
    rtsp1 r1;
    rtsp2 r2;
    string d="RTSP/1.0";
    strncpy(r2.version, d.c_str(),sizeof(r2.version));
    r2.status_Code=200;
    d="OK";
    strncpy(r2.status, d.c_str(),sizeof(r2.status));
    r2.session=4231;
    memset(&r1, 0, sizeof(r1));
    recv(socket_ID, (void *)&r1, sizeof(r1), 0);
    r2.Seq_num=r1.Seq_num;
    send(socket_ID, &r2, sizeof(r2), 0);
    cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    pthread_t tid;

    thread first (RTP,r1.port_num);

    cout<<r1.type<<" "<<r1.linkId<<" "<<r1.version<<endl;
    fileSize_numImages();
    cout<<"Seq_num: "<<r1.Seq_num<<endl;
    cout<<"port_num: "<<port<<endl;
    cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    while (1)
    {
    memset(&r1, 0, sizeof(r1));
    recv(socket_ID, (void *)&r1, sizeof(r1), 0);
    if (r1.Seq_num==0)
     break;
     cout<<r1.type<<" "<<r1.linkId<<" "<<r1.version<<endl;
    cout<<"Seq_num: "<<r1.Seq_num<<endl;
    cout<<"port_num: "<<port<<endl;
    sequence =r1.Seq_num;
    if(r1.type[1]=='L')
        play(socket_ID,r1.range);
    else if(r1.type[1]=='A')
        pause(socket_ID);
    else if(r1.type[1]=='E'){
        stop(socket_ID);
        break;
        }
cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;

    }exit(0); 
}
int main(int argc, char *argv[])
{
    //for the server, we only need to specify a port number
    if(argc != 2)
    {
        cout << "Please enter one port number" << endl;
        exit(0);
    }
    //grab the port number
    int port = atoi(argv[1]);
     
    //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
 
    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    cout << "Waiting for a client to connect..." << endl;
   
    listen(serverSd, 5);
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    //accept, create a new socket descriptor to 
    //handle the new connection with client
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }
    cout << "Connected with client!" << endl;
    cout << getCurrentTimestamp() << endl;
    
    RTSP(newSd,port);
    close(newSd);
    cout << "Connection closed..." << endl;
    exit(0); 
    return 0;   
}
