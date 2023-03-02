#include "Sim_Engine.h"
#include "msg_queue.h"

#include <string.h>

#define END "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"

#define A 0
#define B 1

#define DEFAULT_TIMEOUT 15

#define RTT_WEIGHT 0.9

#define TIMEOUT (avg_rtt * 2)
//#define TIMEOUT 15

struct pkt *last_pkt;
int seqnum;

// Store average Round-Trip Time
float avg_rtt = 0;
float start_time = 0;
float stop_time = 0;

// packets sent and recieved
int packets_sr = 0;

// Count amount of timeouts
int to_count = 0;

// 0 or 1 if host A can send next packet
int canSend = 1;

// 0 or 1 if timer is on
int time_state = 0;

// Create packet from a message
struct pkt *make_packet(struct msg message);

// Calculate average round trip time for successful packets
float calc_rtt();

// Send next packet in the queue
void send_next();

/* Called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message) { // HANDLES SENDING PACKET

  // Put message in queue
  enqueue(message);

  // printf("Added " YELLOW "%c" END " to queue!\n", message.data[0]);
  //  Print the queue of messages to be sent
  //  print_queue();

  // Sends next message
  if (canSend)
    send_next();
}

/* Called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet) { // HANDLES RECEIVING ACK PACKET
  // printf(GREEN "Ack Packet " END YELLOW "%c" END
  //              " arrived AT A from B - packetcksum % d\taccksum: % d\n ",
  //        packet.payload[0], packet.checksum,
  //        cksum((unsigned char *)packet.payload, 20));

  // Check for corruption with checksum
  if (packet.checksum != cksum((unsigned char *)packet.payload, 20)) {
    // printf("Checksum Error A!\n");
    return;
  }

  //  Check if seq number is corrupted
  if (packet.acknum != 1 && packet.acknum != 0) {
    // printf("Acknum corrupted A!\n");
    // printf("acknum A %d\n", packet.acknum);
    return;
  }

  // Check ack num
  if (packet.acknum !=
      last_pkt->acknum) { // Check for corrupted packet and seq number
    // printf("Ack incorrect!\n");
    return;
  }

  // Flip sequence bit
  seqnum = (seqnum + 1) % 2;

  // Ack has been recieved and verified
  canSend = 1;

  // printf("Stopping timer at A \n");
  if (time_state)
    stoptimer(A); // Stop timer
  time_state = 0;

  // Add another successful packet sent and recieved
  packets_sr++;
  stop_time = time;

  // Calculate average Round-Trip time
  calc_rtt();

  // Send next packet
  send_next();
}

/* Called when A's timer goes off */
void A_timerinterrupt() { /* TODO */ //   HANDLES TIMEOUT

  // Increase count of timeouts
  to_count++;

  // printf(RED "Timeout A (%d) after %f ms: " END "Retransmitting packet: "
  // YELLOW
  //            "%c" END " seqnum: %d\n",
  //        to_count, (time - start_time), last_pkt->payload[0],
  //        last_pkt->seqnum);

  // Send last packet again
  tolayer3(A, *last_pkt);

  start_time = time;

  // Start timer for new packet
  starttimer(A, TIMEOUT);
  time_state = 1;
}

/* The following routine will be called once (only) before any other */
/* Host A routines are called. You can use it to do any initialization */
void A_init() { /* TODO */

  last_pkt = (struct pkt *)malloc(sizeof(struct pkt));
  last_pkt->seqnum = 0;
  last_pkt->acknum = 0;

  avg_rtt = DEFAULT_TIMEOUT;

  seqnum = 0;
}

// Create a packet from message
struct pkt *make_packet(struct msg message) {

  struct pkt *pkt = (struct pkt *)malloc(sizeof(struct pkt));
  pkt->seqnum = seqnum;
  pkt->acknum = seqnum;
  pkt->checksum = cksum((unsigned char *)message.data, 20);
  for (int i = 0; i < 20; i++)
    pkt->payload[i] = message.data[i];

  return pkt;
}

// Sends the next packet in the queue
void send_next() {

  struct pkt *pkt;

  // Check if queue is empty
  if (isEmpty())
    return;

  // Get Next message from queue
  struct msg msg = dequeue();

  // Make packet
  pkt = make_packet(msg);

  // Save last packet
  last_pkt->seqnum = pkt->seqnum;
  last_pkt->acknum = pkt->acknum;
  last_pkt->checksum = pkt->checksum;
  strcpy(last_pkt->payload, pkt->payload);

  // printf("Sending packet " YELLOW "%c" END "!\n", pkt->payload[0]);

  // Send to layer 3
  tolayer3(A, *pkt); // 0 is A

  // Tell host not allowed to send next packet before ack
  canSend = 0;

  start_time = time;

  // Start timer for packet
  starttimer(A, TIMEOUT);
  time_state = 1;

  // Free the packet
  free(pkt);
}

// Calculate average Round-Trip time
float calc_rtt() {
  if (avg_rtt <= 0)
    avg_rtt = DEFAULT_TIMEOUT;
  // avg_rtt = (avg_rtt + (stop_time - start_time)) / packets_sr;
  // avg_rtt = ((avg_rtt + (stop_time - start_time)) / 2);

  avg_rtt = RTT_WEIGHT * avg_rtt + (1 - RTT_WEIGHT) * (stop_time - start_time);

  // printf("Difference: %f\tnum_p: %d\n", (stop_time - start_time),
  // packets_sr);
  // printf("Average rtt: %f\n", avg_rtt);
  return avg_rtt;
}