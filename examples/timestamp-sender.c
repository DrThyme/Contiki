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

/*---------------------------------------------------------------------------*/
PROCESS(sender_process, "Sender process");
AUTOSTART_PROCESSES(&sender_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  //struct timestamp_msg msg;
  struct test_msg msg;
  long int sec, usec;

    
  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
  
  sec = (msg.reflector_timestamp).tv_sec-(msg.sender_timestamp).tv_sec;
  usec = (msg.reflector_timestamp).tv_usec-(msg.sender_timestamp).tv_usec;
  /*printf("timestamp message received from %d.%d. RRT was '%lu' ms.\n",
         from->u8[0], from->u8[1], ((msg.reflector_timestamp-msg.sender_timestamp)*1000)/CLOCK_SECOND));*/
  printf("timestamp message received from %d.%d. RRT was '%ld' sec and '%ld' usec. Sequence number was: %d\n",
         from->u8[0], from->u8[1],sec,usec,msg.seq_no);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sender_process, ev, data)
{
  static struct etimer et;
  //struct timestamp_msg msg;
  struct test_msg msg;
  int seq_no = 0;
  struct timeval t;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);
  
  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
	
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    //msg.sender_timestamp = clock_time();
    msg.seq_no = seq_no;
    gettimeofday(&t,NULL); 
    msg.sender_timestamp = t;
    packetbuf_copyfrom(&msg, sizeof(msg));
    broadcast_send(&broadcast);
    printf("timestamp message sent\n");
    seq_no++;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


