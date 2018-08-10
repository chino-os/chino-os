//
// Chino Network
//
#include "NetworkManager.hpp"
#include "../kdebug.hpp"
#include <lwip/init.h>
#include <lwip/opt.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include <lwip/ethip6.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/ip4_addr.h>
#include <netif/ethernet.h>
#include <lwip/priv/tcp_priv.h>
#include "../threading/timer.h"

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Network;

#define PHLCON           0x14

extern uint8_t ENC28J60_Init(uint8_t* macaddr);
extern void ENC28J60_Write(uint8_t addr, uint8_t data);
extern void ENC28J60_Read_Buf(uint32_t len, uint8_t* data);
extern void ENC28J60_PHY_Write(uint8_t addr, uint32_t data);
extern void enc28j60_init_send(int len);
extern void ENC28J60_Write_Buf(uint32_t len, uint8_t* data);
extern void enc28j60_start_send(void);
extern int enc28j60_packet_getcount(void);
extern int enc28j60_packet_getlen(void);
extern void enc28j60_finish_receive(void);

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/**
* Helper struct to hold private data used to operate your ethernet interface.
* Keeping the ethernet address of the MAC in this struct is not necessary
* as it is already kept in the struct netif.
* But this is only an example, anyway...
*/
struct ethernetif {
	struct eth_addr *ethaddr;
	/* Add whatever per-interface state that is needed here. */
};

/* Forward declarations. */
static void ethernetif_input(struct netif *netif);

/**
* In this function, the hardware should be initialized.
* Called from ethernetif_init().
*
* @param netif the already initialized lwip network interface structure
*        for this ethernetif
*/
static void low_level_init(struct netif *netif)
{
	auto ethernetif = reinterpret_cast<struct ethernetif*>(netif->state);

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	netif->hwaddr[0] = 0x04;
	netif->hwaddr[1] = 0x02;
	netif->hwaddr[2] = 0x35;
	netif->hwaddr[3] = 0x00;
	netif->hwaddr[4] = 0x00;
	netif->hwaddr[5] = 0x01;

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
	/*
	* For hardware/netifs that implement MAC filtering.
	* All-nodes link-local is handled by default, so we must let the hardware know
	* to allow multicast packets in.
	* Should set mld_mac_filter previously. */
	if (netif->mld_mac_filter != NULL) {
		ip6_addr_t ip6_allnodes_ll;
		ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
		netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
	}
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

	/* Do whatever else is needed to initialize interface. */
	ENC28J60_Init(netif->hwaddr);
	ENC28J60_PHY_Write(PHLCON, 0x0476);
}

/**
* This function should do the actual transmission of the packet. The packet is
* contained in the pbuf that is passed to the function. This pbuf
* might be chained.
*
* @param netif the lwip network interface structure for this ethernetif
* @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
* @return ERR_OK if the packet could be sent
*         an err_t value if the packet couldn't be sent
*
* @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
*       strange results. You might consider waiting for space in the DMA queue
*       to become available since the stack doesn't retry to send a packet
*       dropped because of memory failure (except for the TCP timers).
*/

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	auto ethernetif = reinterpret_cast<struct ethernetif*>(netif->state);
	struct pbuf *q;

	enc28j60_init_send(p->tot_len);

#if ETH_PAD_SIZE
	pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

	for (q = p; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
		time. The size of the data in each pbuf is kept in the ->len
		variable. */
		ENC28J60_Write_Buf(q->len, (uint8_t*)q->payload);
	}

	enc28j60_start_send();

	MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
	if (((u8_t *)p->payload)[0] & 1) {
		/* broadcast or multicast packet*/
		MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
	}
	else {
		/* unicast packet */
		MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
	}
	/* increase ifoutdiscards or ifouterrors on error */

#if ETH_PAD_SIZE
	pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	LINK_STATS_INC(link.xmit);

	return ERR_OK;
}

/**
* Should allocate a pbuf and transfer the bytes of the incoming
* packet from the interface into the pbuf.
*
* @param netif the lwip network interface structure for this ethernetif
* @return a pbuf filled with the received packet (including MAC header)
*         NULL on memory error
*/
static struct pbuf * low_level_input(struct netif *netif)
{
	auto ethernetif = reinterpret_cast<struct ethernetif*>(netif->state);
	struct pbuf *p, *q;
	u16_t len;

	/* Obtain the size of the packet and put it into the "len"
	variable. */
	len = enc28j60_packet_getlen();

#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

						 /* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	if (p != NULL) {

#if ETH_PAD_SIZE
		pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

											 /* We iterate over the pbuf chain until we have read the entire
											 * packet into the pbuf. */
		for (q = p; q != NULL; q = q->next) {
			/* Read enough bytes to fill this pbuf in the chain. The
			* available data in the pbuf is given by the q->len
			* variable.
			* This does not necessarily have to be a memcpy, you can also preallocate
			* pbufs for a DMA-enabled MAC and after receiving truncate it to the
			* actually received size. In this case, ensure the tot_len member of the
			* pbuf is the sum of the chained pbuf len members.
			*/
			//read data into(q->payload, q->len);
			ENC28J60_Read_Buf(q->len, (uint8_t*)q->payload);
		}
		enc28j60_finish_receive();

		MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
		if (((u8_t *)p->payload)[0] & 1) {
			/* broadcast or multicast packet*/
			MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
		}
		else {
			/* unicast packet*/
			MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
		}
#if ETH_PAD_SIZE
		pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

		LINK_STATS_INC(link.recv);
	}
	else {
		enc28j60_finish_receive();

		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
		MIB2_STATS_NETIF_INC(netif, ifindiscards);
	}

	return p;
}

/**
* This function should be called when a packet is ready to be read
* from the interface. It uses the function low_level_input() that
* should handle the actual reception of bytes from the network
* interface. Then the type of the received packet is determined and
* the appropriate input function is called.
*
* @param netif the lwip network interface structure for this ethernetif
*/
static void ethernetif_input(struct netif *netif)
{
	struct eth_hdr *ethhdr;
	struct pbuf *p;

	auto ethernetif = reinterpret_cast<struct ethernetif*>(netif->state);

	/* move received packet into a new pbuf */
	p = low_level_input(netif);
	/* if no packet could be read, silently ignore this */
	if (p != NULL) {
		/* pass all packets to ethernet_input, which decides what packets it supports */
		if (netif->input(p, netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
			pbuf_free(p);
			p = NULL;
		}
	}
}

/**
* Should be called at the beginning of the program to set up the
* network interface. It calls the function low_level_init() to do the
* actual setup of the hardware.
*
* This function should be passed as a parameter to netif_add().
*
* @param netif the lwip network interface structure for this ethernetif
* @return ERR_OK if the loopif is initialized
*         ERR_MEM if private data couldn't be allocated
*         any other err_t on error
*/
err_t ethernetif_init(struct netif *netif)
{
	struct ethernetif *ethernetif;

	LWIP_ASSERT("netif != NULL", (netif != NULL));

	ethernetif = new (std::nothrow) struct ethernetif();
	if (ethernetif == NULL) {
		LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
		return ERR_MEM;
	}

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

	/*
	* Initialize the snmp variables and counters inside the struct netif.
	* The last argument should be replaced with your link speed, in units
	* of bits per second.
	*/
	MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

	netif->state = ethernetif;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	/* We directly use etharp_output() here to save a function call.
	* You can instead declare your own function an call etharp_output()
	* from it if you have to do some checks before sending (e.g. if link
	* is available...) */
#if LWIP_IPV4
	netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
	netif->linkoutput = low_level_output;

	ethernetif->ethaddr = (struct eth_addr *) & (netif->hwaddr[0]);

	/* initialize the hardware */
	low_level_init(netif);

	return ERR_OK;
}

NetworkManager::NetworkManager()
{
	lwip_init();
}

#define     TCP_PERIOID     CLOCK_SECOND / 4        // TCP 250ms
#define     ARP_PERIOID     CLOCK_SECOND * 5        // ARP 5s

void NetworkManager::Test()
{
	struct netif netif;
	kassert(ethernetif_init(&netif) == ERR_OK);

	ip4_addr_t ipaddr, netmask, gw;

	IP4_ADDR(&ipaddr, 192, 168, 1, 16);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 1, 1);

	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);
	netif_set_default(&netif);
	netif_set_up(&netif);

	timer_typedef tcp_timer, arp_timer;

	/* 设定查询定时器 ARP定时器 */
	timer_set(&tcp_timer, CLOCK_SECOND / 10);       // tcp处理定时器 100ms
	timer_set(&arp_timer, CLOCK_SECOND * 5);        // arp处理定时器 5S

	while (1) {

		if (enc28j60_packet_getcount() != 0) {
			ethernetif_input(&netif);
		}

		// TCP 定时处理
		if (timer_expired(&tcp_timer)) {
			timer_set(&tcp_timer, CLOCK_SECOND / 4);
			g_Logger->PutFormat("TCP ");
			tcp_tmr();
		}

		// ARP 定时处理
		if (timer_expired(&arp_timer)) {
			timer_set(&arp_timer, CLOCK_SECOND * 5);
			g_Logger->PutFormat("ARP ");
			etharp_tmr();
		}
	}
}

void NetworkManager::TryInstallNetworkDevice(ObjectPtr<Device::Device> device)
{
	auto type = device->GetType();
	if (type == DeviceType::NetworkAdapter)
	{
		//struct netif netif;
		//netif_add(&netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL, netif_init, tcpip_input);
	}
}
