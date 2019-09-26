#include <stdio.h>
#include <stdlib.h>
#include "project2.h"
#include <zlib.h>
#include <string.h>
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

int seqNum = 0;
struct pkt *window;
//should also have a list of the waiting messages so that when one gets transmitted, can send next one and pluck from list
struct pkt waiting[MESSAGE_LENGTH];

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
  //receive message and create a packet to send out
  //add packet to window to keep track of who's been sent/being process
  //after putting on queu, send to b input!
  //end

  int i;
  struct pkt *packet = malloc(sizeof(struct pkt));
  packet->acknum = 1; //can start with either 1 or 0
  unsigned int crc = crc32(0, message.data, MESSAGE_LENGTH);
  packet->checksum = crc;
  // for (i = 0; i < MESSAGE_LENGTH; i++){
  //   packet->payload[i] = message.data[i];
  // }
  //packet->payload = malloc(sizeof(char) * MESSAGE_LENGTH);
  strcpy(packet->payload, message.data);
  packet->seqnum = seqNum;

  printf("-----------original packet: %s, %d", packet->payload, packet->seqnum);
  //place packet into window since it's being sent, i.e. layer 3 window is occupied
  if(window->seqnum == -1){ //first packet in list
    // for (i = 0; i < MESSAGE_LENGTH; i++){
    //   window->payload[i] = message.data[i];
    // }
    //window->payload = (char*)malloc(sizeof(char) * MESSAGE_LENGTH);
    strcpy(window->payload, message.data);
    window->acknum = packet->acknum;
    window->checksum = packet->checksum;
    window->seqnum = packet->seqnum;
  } else { //add to list
    //place in waiting/being processed list
    // for (i = 0; i < MESSAGE_LENGTH; i++){
    //   waiting[seqNum].payload[i] = packet->payload[i];
    // }
    //waiting[seqNum].payload = malloc(sizeof(char) * MESSAGE_LENGTH);
    strcpy(waiting[seqNum].payload, packet->payload);
    waiting[seqNum].acknum = packet->acknum;
    waiting[seqNum].checksum = packet->checksum;
    waiting[seqNum].seqnum = packet->seqnum;
  } 

  printf("-----------original window: %s, %d", window->payload, window->seqnum);
  printf("-----------original waiting: %s, %d", waiting[seqNum].payload, waiting[seqNum].seqnum);

  int j;
  for(j = 0; j < (sizeof(waiting)/sizeof(struct pkt)); j++){
    printf("This is the current waiting list: %s, %d\n", waiting[j].payload, waiting[j].seqnum);
  }

  //should end up in b input, sending from a to b
  tolayer3(0, *packet);
  //startimer?
  //startTimer(0, 5);
  seqNum++;
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
  //don't use!
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
  
  //receives packet from b_input through layer 3;
  //check if ack of packet is same as the ack of the packet in window right now; if so, then the packet is ok to be sent to layer5
  //flip ack

  //check b timer
  // if(getTimerStatus(1)){ //still going, no timeout
  //   printf("B timer is still going\n");
  //   stopTimer(1);
  // } else { //ran out of time
  //   printf("B timer has timed out\n");
  //   B_timerinterrupt();
  // }

  unsigned int checkCheckSum = crc32(0, packet.payload, MESSAGE_LENGTH);
  printf("a input check sum calc: %u\n", checkCheckSum);
  printf("packet check sum: %u\n", packet.checksum);

  if(packet.checksum != checkCheckSum){
    printf("a input packet is corrupt!!!\n");
    //packet is corrupt and needs to be resent from a
    B_timerinterrupt();
  } else {
    //packet is fine and has been sent to layer 5; remove from window and send next packet with reversed ack bit
    waiting[packet.seqnum]; //remove from list
    
    struct pkt newPack;

    //get next packet in list
    if(waiting[packet.seqnum + 1].payload != NULL){
       newPack = waiting[packet.seqnum + 1];
       if(packet.acknum == 1){
        newPack.acknum = 0;
      } else {
        newPack.acknum = 1;
      }

      printf("values for newPack in a input: %s, %d\n", newPack.payload, newPack.seqnum);
      printf("a input packet is fine, sending to layer 3!!!\n");
      //startTimer(0, 5);
      tolayer3(0, newPack);

    } else {
      printf("no more messages in waiting list\n");
      //last message in queue, what do here?
      exit(1);
    }
    
  }

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
  //check if a timer is still running - it shouldn't be, but just in case
  // if(getTimerStatus(0)){
  //   stopTimer(0);
  // }

  //resend damaged packet; need to get it from currend window, right? since can only send one at a time 
  //do i need to fix whatever the error is or just resend?
  //startTimer(0, 5);
  //send back to b, hopefully it doesn't scramble again
  tolayer3(0, *window);

}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  //create queue
  //initialize window to certain value to checlk
  window = malloc(sizeof(struct pkt));
  window->seqnum = -1;
  strcpy(window->payload, "nonsense");
  window->checksum = 0;
  window->acknum = 0;

  //arbitrary array size
  
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
  //check for corruption - checksum! and sequence value
  //send back ack0 or ack1 for

  //get timer status; if timer is still going, that reset timer before sending
  //if timer has run out, call timer interrupt

  // if(getTimerStatus(0)){
  //   printf("A timer is still going\n");
  //   stopTimer(0);
  // } else {
  //   printf("a timer has timed out\n");
  //   A_timerinterrupt();
  // }

  //check checksum
  unsigned int crc = crc32(0, packet.payload, MESSAGE_LENGTH);
  printf("b input check sum calc: %u\n", crc);
  printf("packet check sum: %u\n", packet.checksum);

  int i;
  struct msg *message = malloc(sizeof(struct msg));
  //message->data = malloc(sizeof(char) * MESSAGE_LENGTH);

  // for (i = 0; i < MESSAGE_LENGTH; i++){
  //   message->data[i] = packet.payload[i];
  // }

  strcpy(message->data, packet.payload);

  if(packet.checksum != crc){ 
    printf("b input packet is corrupt!!!\n");
    //checksum has been scrambled
    //retransmit packet! just call the interrupt again?
    A_timerinterrupt();
  } else {
    printf("b input packet is fine, sending to both layers!!!\n");
    //packet is theoretically fine, can send back an ACK of same value
    tolayer3(1, packet);
    //start b's timer; use to check if be receives new message of different ack or not
    //startTimer(1, 5);
    //can also send to layer5 here since the packet is ok
    tolayer5(1, *message);
  }

  

  //tolayer5(1, *message);

}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  //check if a timer is still running - it shouldn't be, but just in case
  if(getTimerStatus(1)){
    stopTimer(1);
  }

  //resend damaged packet; need to get it from currend window, right? since can only send one at a time 
  //do i need to fix whatever the error is or just resend?
  //startTimer(1, 1000);
  //send back to a to be reprocessed
  tolayer3(1, *window);
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
}

