#include "Sim_Engine.h"
#include <string.h> // memset

#define END "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"

#define A 0
#define B 1

int last_seq;

/* Called from layer 5, passed the data to be sent to other side */
void B_output(struct msg message) { /* DON'T IMPLEMENT */
}

/* Called from layer 3, when a packet arrives for layer 4 */
void B_input(struct pkt packet) { // HANDLES RECEIVING PACKET
  struct pkt *ack_pkt;
  // printf("PACKET ARRIVED AT B - Packet from A: cksum: %d\t Packet at B:
  // cksum: %d\n", packet.checksum, cksum((unsigned char *)packet.payload, 20));

  // printf("HOST_B A.seq %d\t B.seq %d\n", packet.seqnum, last_pkt_B->seqnum);

  // printf("Packet at B: %d, %d, %s\n", packet.seqnum, packet.checksum,
  // packet.payload);

  if (packet.checksum != cksum((unsigned char *)packet.payload, 20)) {
    printf("Checksum Error B! %d %d\n", packet.checksum,
           cksum((unsigned char *)packet.payload, 20));
    return;
  }

  // Check if seq number is corrupted
  if (packet.seqnum != 1 && packet.seqnum != 0) {
    printf("Packet seqnum corrupted B\n");
    return;
  }

  // Create acknowledgment packet
  ack_pkt = (struct pkt *)malloc(sizeof(struct pkt));
  ack_pkt->acknum = packet.acknum;
  // memset(ack_pkt->payload, 0, 20); // Zero the payload
  strcpy(ack_pkt->payload, packet.payload);
  ack_pkt->checksum = cksum((unsigned char *)ack_pkt->payload, 20);

  // Check for duplicate packet
  if (packet.seqnum == last_seq) {
    printf(YELLOW "Duplicate Detected at B!\n" END);
    tolayer3(B, *ack_pkt); // Send ack packet (to A)
    return;
  }

  // printf("pct: %d, last_pct: %d\n", packet.seqnum, last_seq);

  // ack packet
  tolayer3(B, *ack_pkt); // Send ack packet (to A)

  // Send to layer 5
  tolayer5(B, packet.payload);

  last_seq = packet.seqnum;

  // Free the acknowledgment packet
  free(ack_pkt);
}

/* Called when B's timer goes off */
void B_timerinterrupt() { /* TODO */
}

/* The following routine will be called once (only) before any other */
/* Host B routines are called. You can use it to do any initialization */
void B_init() { last_seq = 1; }
