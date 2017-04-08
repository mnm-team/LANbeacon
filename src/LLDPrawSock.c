/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#define _GNU_SOURCE	 /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

#include <linux/ip.h>
#include <linux/udp.h>

#define LLDP_DEST_MAC0	0x01
#define LLDP_DEST_MAC1	0x80
#define LLDP_DEST_MAC2	0xc2
#define LLDP_DEST_MAC3	0x00
#define LLDP_DEST_MAC4	0x00
#define LLDP_DEST_MAC5	0x0e

#define LLDP_ETHER_TYPE	0x88CC

//#define DEFAULT_IF	"enp0s18"
#define BUF_SIZ		2000


// parts of code based on https://gist.github.com/austinmarton/1922600
int sendLLDPrawSock (int LLDPDU_len, char *LANbeaconCustomTLVs)
{
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int frameLength = 0;
	char LLDPethernetFrame[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) LLDPethernetFrame;
//	struct iphdr *iph = (struct iphdr *) (LLDPethernetFrame + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;
	
	struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	char host[NI_MAXHOST];
	
	
	/* Construct the Ethernet header */
	memset(LLDPethernetFrame, 0, BUF_SIZ);

	eh->ether_dhost[0] = LLDP_DEST_MAC0;
	eh->ether_dhost[1] = LLDP_DEST_MAC1;
	eh->ether_dhost[2] = LLDP_DEST_MAC2;
	eh->ether_dhost[3] = LLDP_DEST_MAC3;
	eh->ether_dhost[4] = LLDP_DEST_MAC4;
	eh->ether_dhost[5] = LLDP_DEST_MAC5;
	
	/* Ethertype field */
	eh->ether_type = htons(LLDP_ETHER_TYPE);
	frameLength += sizeof(struct ether_header);
	
	/* Open RAW socket to send on */
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(LLDP_ETHER_TYPE))) == -1) {	
		perror("socket");
	}
	
	/* Packet data */
	memcpy(&LLDPethernetFrame[frameLength], LANbeaconCustomTLVs, LLDPDU_len);
	frameLength += LLDPDU_len; 

	/* End of LLDPDU TLV */
	LLDPethernetFrame[frameLength++] = 0x00;
	LLDPethernetFrame[frameLength++] = 0x00;
	
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;
	/* Destination MAC */
	socket_address.sll_addr[0] = LLDP_DEST_MAC0;
	socket_address.sll_addr[1] = LLDP_DEST_MAC1;
	socket_address.sll_addr[2] = LLDP_DEST_MAC2;
	socket_address.sll_addr[3] = LLDP_DEST_MAC3;
	socket_address.sll_addr[4] = LLDP_DEST_MAC4;
	socket_address.sll_addr[5] = LLDP_DEST_MAC5;

	
	/* Get interfaces */
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	
	while (1) {
		sleep(2);
		
		for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
	
			if (ifa->ifa_addr == NULL)	continue;

			if (!(ifa->ifa_addr->sa_family == AF_PACKET))	continue; 

			/* Get the index of the interface to send on */
			memset(&if_idx, 0, sizeof(struct ifreq));
			strncpy(if_idx.ifr_name, ifa->ifa_name, IFNAMSIZ-1);
			if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
				perror("SIOCGIFINDEX");
			/* Get the MAC address of the interface to send on */
			memset(&if_mac, 0, sizeof(struct ifreq));
			strncpy(if_mac.ifr_name, ifa->ifa_name, IFNAMSIZ-1);
			if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
				perror("SIOCGIFHWADDR");

			/* Ethernet header */
			eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
			eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
			eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
			eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
			eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
			eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];

			/* Index of the network device */
			socket_address.sll_ifindex = if_idx.ifr_ifindex;
		
			/* Send packet */
			if (sendto(sockfd, LLDPethernetFrame, frameLength, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)	printf("Send failed on interface %s\n", ifa->ifa_name);
			else	printf("Successfully sent on interface %s\n", ifa->ifa_name);
	
		}
	}
	
	return 0;
}



// parts of code based on https://gist.github.com/austinmarton/2862515
void recLLDPrawSock(int argc, char *argv[], unsigned char *LLDPreceivedPayload, ssize_t *payloadSize)
{
	char sender[INET6_ADDRSTRLEN];
	int ret, i;
	int sockfd[20];
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct sockaddr_storage their_addr;
	
	char ifName[IFNAMSIZ];
	
	
	/* Header structures */
	struct ether_header *eh = (struct ether_header *) LLDPreceivedPayload;
	struct iphdr *iph = (struct iphdr *) (LLDPreceivedPayload + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (LLDPreceivedPayload + sizeof(struct iphdr) + sizeof(struct ether_header));

	memset(&if_ip, 0, sizeof(struct ifreq));
	
	
	
	int numInterfaces;
	
	struct ifaddrs *ifaddr, *ifa;
	
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	
	for (ifa = ifaddr, numInterfaces = 0; ifa != NULL; ifa = ifa->ifa_next, numInterfaces++) {

		if (ifa->ifa_addr == NULL)	continue;

		if (!(ifa->ifa_addr->sa_family == AF_PACKET))	continue; 
		
		strcpy(ifName, ifa->ifa_name); //TODO
		
		/* Open PF_PACKET socket, listening for EtherType LLDP_ETHER_TYPE */
		if ((sockfd[numInterfaces] = socket(PF_PACKET, SOCK_RAW, htons(LLDP_ETHER_TYPE))) == -1) {
			perror("listener: socket");	
		}

		/* Set interface to promiscuous mode */
		strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
		ioctl(sockfd[numInterfaces], SIOCGIFFLAGS, &ifopts);
		ifopts.ifr_flags |= IFF_PROMISC;
		ioctl(sockfd[numInterfaces], SIOCSIFFLAGS, &ifopts);
	
		/* Allow the socket to be reused - incase connection is closed prematurely */
		if (setsockopt(sockfd[numInterfaces], SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
			perror("setsockopt");
			close(sockfd[numInterfaces]);
			exit(EXIT_FAILURE);
		}
		/* Bind to device */
		if (setsockopt(sockfd[numInterfaces], SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
			perror("SO_BINDTODEVICE");
			close(sockfd[numInterfaces]);
			exit(EXIT_FAILURE);
		}
	}
	
	while (1) {
		for (int r = 0; r <= numInterfaces; r++) {
	//		printf("listener: Waiting to recvfrom...\n");
			*payloadSize = recvfrom(sockfd[0], LLDPreceivedPayload, BUF_SIZ, 0, NULL, NULL);
	//		printf("listener: got packet %lu bytes\n", *payloadSize);

			/* Check if the packet was sent to the LLDP multicast MAC address */
			if (!(eh->ether_dhost[0] == LLDP_DEST_MAC0 &&
					eh->ether_dhost[1] == LLDP_DEST_MAC1 &&
					eh->ether_dhost[2] == LLDP_DEST_MAC2 &&
					eh->ether_dhost[3] == LLDP_DEST_MAC3 &&
					eh->ether_dhost[4] == LLDP_DEST_MAC4 &&
					eh->ether_dhost[5] == LLDP_DEST_MAC5)) {
				continue;
			}

			/* Print packet */
	//		printf("\tData:");
	//		for (i=0; i<*payloadSize; i++) printf("%02x:", LLDPreceivedPayload[i]);
	//		printf("\n");
			return;
		}
	}

	close(sockfd[0]);
}
