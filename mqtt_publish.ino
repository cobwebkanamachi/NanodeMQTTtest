//NOTE: This is a shlinked version of original mqtt_publish.ino of NanodeMQTT/examples/mqtt_publish/mqtt_publish.ino
//Please change YOUR_MQTT_SRV bellow with your MQTT SERVER's IP address.
//#include <NanodeUNIO.h>
#include <NanodeUIP.h>
#include <NanodeMQTT.h>
#include <psock.h>

NanodeMQTT mqtt(&uip);
struct timer my_timer;

struct hello_world_state {
  struct psock p;
  byte inputbuffer[20];
  char name[20];
  char quest[20];
};
UIPASSERT(sizeof(struct hello_world_state)<=TCP_APP_STATE_SIZE)

/* This function defines what the "hello world" application does
   once a connection has been established.  It doesn't have to
   deal with setting up the connection. */
static int hello_world_connection(struct hello_world_state *s)
{
  PSOCK_BEGIN(&s->p);

  PSOCK_SEND_STR(&s->p, "Hello. What is your name?\n>>> ");
  PSOCK_READTO(&s->p, '\n');
  /* The input buffer is an array of bytes received from the
     network.  It's not a string.  We cheekily cast it to a string
     here on the assumption that we're receiving plain ASCII and
     not UTF8 or anything more fancy... */
  s->inputbuffer[PSOCK_DATALEN(&s->p)]=0;
  strncpy(s->name, (const char *)s->inputbuffer, sizeof(s->name));
  PSOCK_SEND_STR(&s->p, "What is your quest?\n>>> ");
  PSOCK_READTO(&s->p, '\n');
  s->inputbuffer[PSOCK_DATALEN(&s->p)]=0;
  strncpy(s->quest, (const char *)s->inputbuffer, sizeof(s->quest));
  PSOCK_SEND_STR(&s->p, "Hello ");
  PSOCK_SEND_STR(&s->p, s->name);
  PSOCK_SEND_STR(&s->p, "Your quest is: ");
  PSOCK_SEND_STR(&s->p, s->quest);
  PSOCK_CLOSE(&s->p);
  
  PSOCK_END(&s->p);
}

/* This function deals with all incoming events for the "hello
   world" application, including dealing with new connections. */
static void hello_world_appcall(void)
{
  struct hello_world_state *s = (struct hello_world_state *)&(uip_conn->appstate);

  /*
   * If a new connection was just established, we should initialize
   * the protosocket in our application's state structure.
   */
  if(uip_connected()) {
    PSOCK_INIT(&s->p, s->inputbuffer, sizeof(s->inputbuffer)-1);
  }

  /*
   * We run the protosocket function that actually handles the
   * communication. We pass it a pointer to the application state
   * of the current connection.
   */
  hello_world_connection(s);
}


void dhcp_status(int s,const uip_ipaddr_t *dnsaddr) {
  char buf[20]="IP:";
  if (s==DHCP_STATUS_OK) {
    resolv_conf(dnsaddr);
    uip.get_ip_addr_str(buf+3);
    Serial.println(buf);
    //uip.query_name("www.greenend.org.uk");
    // Start the "hello world" app defined earlier
    //uip_listen(UIP_HTONS(1000),hello_world_appcall);
  }
}

void setup() {
    char buf[20];
  byte macaddr[6]= {  
0x00, 0xFF, 0x7A, 0xA5, 0x06, 0xDD
  };
  //NanodeUNIO unio(NANODE_MAC_DEVICE);

  Serial.begin(38400);
  Serial.println("MQTT Publish test");
  
  //unio.read(macaddr, NANODE_MAC_ADDRESS, 6);
  uip.init(macaddr);
  uip.get_mac_str(buf);
  Serial.println(buf);
  // FIXME: use DHCP instead
  //uip.set_ip_addr(10, 100, 10, 10);
  //uip.set_netmask(255, 255, 255, 0);

  uip.wait_for_link();
  Serial.println("Link is up");
  //uip.init_resolv(resolv_found);
  uip.start_dhcp(dhcp_status);
  // Setup a timer - publish every 5 seconds
  timer_set(&my_timer, CLOCK_SECOND * 5);

  // FIXME: resolve using DNS instead
  //mqtt.set_server_addr(10, 100, 10, 1);
  mqtt.set_server_addr(YOUR_MQTT_SRV);
  mqtt.connect();

  Serial.println("setup() done");
}

void loop() {
  uip.poll();

  if(timer_expired(&my_timer)) {
    timer_reset(&my_timer);
    if (mqtt.connected()) {
      Serial.println("Publishing...");
      mqtt.publish("test", "Hello World!");
      Serial.println("Published.");
    }
  }
}
