/**
Utility Functions
@author zhpeng 
@version 10.05.26
*/
#ifndef __ARPZ_UTIL__

/*Parse Command line*/
void ParseCmd (int argc, char *argv[], int *mode, char *gwip, char *tgtip, char *rstfile, char *device);

/*Wrapper function for creating a raw socket*/
int CreateRawSocket (int protocol);

/*look up the specified interface and bind the socket on it*/
int BindToInterface (int sockfd, char *device, int protocol);

/*Construct ethernet pakcet header*/
void CreateEtherHeader (void *pkt, char *dstmac, char *srcmac, int protocol);

/*Construct arp packet header*/
void CreateARPHeader (void *pkt, char *srcmac, char *srcip, char *dstmac, char *dstip);

/*Wrapper function for send*/
int SendPacket (int sockfd, void *pkt, int len);

/*IPC function*/
int CreateMsgQu(void);

void DestroyMsgQ(int msgq);

/*Advanced ARP Injection*/
void *SnifferThread (void *arg);

void *InjectionThread (void *arg);
#endif
