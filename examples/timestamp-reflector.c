#include <time.h>
#include <sys/time.h>
#include "contiki.h"
#include "clock.h"
#include "net/rime/rime.h"
#include "random.h"
#include <string.h>
#include <stdio.h>
#include "dev/button-sensor.h"

#include "dev/leds.h"

struct test_msg{
	int seq_no;
	struct timeval sender_timestamp;
	struct timeval reflector_timestamp;
};

struct timestamp_msg {
    clock_time_t sender_timestamp;
    clock_time_t reflector_timestamp;
};

struct test_msg temp_test;
struct timestamp_msg temp_msg;
int send = 0;
/*---------------------------------------------------------------------------*/
PROCESS(reflector_process, "Reflector process");
AUTOSTART_PROCESSES(&reflector_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct timestamp_msg msg;
  struct timeval t;
 
  gettimeofday(&t,NULL);
  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));

  //msg.reflector_timestamp = clock_time();
  msg.reflector_timestamp = t;
  temp_test = msg;
  //temp_msg = msg;

  send = 1;
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(reflector_process, ev, data)
{
  struct timestamp_msg msg;
    static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  
  broadcast_open(&broadcast, 129, &broadcast_call);
  printf("Reflector started!\n");
  while(1) {
    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 2 + random_rand() % (CLOCK_SECOND * 2));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

      	if(send==1){
		msg = temp_msg;
		packetbuf_copyfrom(&msg, sizeof(msg));
    		broadcast_send(&broadcast);
		printf("Message reflected!\n");
		send = 0;
	}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


