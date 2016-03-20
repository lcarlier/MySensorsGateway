#include <SmingCore.h>
#include <user_config.h>

#include "ethernetif.h"
#include "ethernetif_driver.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <lwip/dhcp.h>
err_t etharp_output(struct netif *netif,
                    struct pbuf  *q,
                    ip_addr_t    *ipaddr 
                   );
err_t ethernet_input(struct pbuf  *p,
                     struct netif *netif 
                    );

struct ethernetif w5100_netif_if;
struct netif w5100_netif;

#ifdef __cplusplus
}
#endif

struct ip_addr myip_addr = {0x00000000UL};
struct ip_addr gw_addr   = {0x00000000UL};
struct ip_addr netmask   = {0x00000000UL};

Timer ethTimer;
void ethTimerHandler()
{
    ethernetif_input(&w5100_netif);
}

void w5100_netif_init()
{
  uint8_t mac_address[6] = {0, 0xBA, 0xD0, 0xCA, 0xFE, 1};
  memcpy(w5100_netif_if.address, mac_address, 6);

  // Add our netif to LWIP (netif_add calls our driver initialization function)
  if (netif_add(&w5100_netif, &myip_addr, &netmask, &gw_addr, &w5100_netif_if,
                ethernetif_init, ethernet_input) == NULL) {
      Serial.println("netif_add (w5100_netif_init) failed");
  }

  //netif_set_default(&w5100_netif);
  netif_set_up(&w5100_netif);
  dhcp_start(&w5100_netif);

  ethTimer.initializeMs(1, ethTimerHandler).start(true);
}

