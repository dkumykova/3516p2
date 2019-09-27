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
struct pkt waiting[100];
int waitingIndex = 0;

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
  //after putting on queue, send to b input!
  int i;
  struct pkt *packet = malloc(sizeof(struct pkt));
  packet->acknum = 1; //can start with either 1 or 0
  //fix crc to include all fields
  unsigned int crc = crc32(0, message.data, MESSAGE_LENGTH);
  packet->checksum = crc;
  packet->seqnum = seqNum;
  strncpy(packet->payload, message.data, MESSAGE_LENGTH);
  
  
  printf("-----------original packet message: ");
  for(i = 0; i < MESSAGE_LENGTH; i++){
    printf("%c", packet->payload[i]);
  } 
  printf("seqnum: %d\n", packet->seqnum);
  printf("-----------original message as received from layer5: ");
  for(i = 0; i < MESSAGE_LENGTH; i++){
    printf("%c", message.data[i]);
  } 
  printf("\n");
  printf("processing packets into list instead@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

  //make sure packet gets into list
  strncpy(waiting[seqNum].payload, packet->payload, MESSAGE_LENGTH);
  waiting[seqNum].acknum = packet->acknum;
  waiting[seqNum].checksum = packet->checksum;
  waiting[seqNum].seqnum = packet->seqnum;

  printf("^^^^^^^^^^^^^^^^^^^^^^^^^waiting: all data: %d, %d", waiting[seqNum].checksum, waiting[seqNum].seqnum);
    int j;
    for(j = 0; j < MESSAGE_LENGTH; j++){
      printf("%c", waiting[seqNum].payload[j]);
    }
    printf("\n");

  //place packet into window since it's being sent, i.e. layer 3 window is occupied
  if(window->seqnum == -1){ //first packet in list
    printf("this should only be printed once!!----------------\n\n");

    tolayer3(0, *packet);
    //startimer?
   startTimer(0, 10);
    window = &waiting[seqNum];
    printf("$$$$$$$$$$$$$$$$$$$$$window 1 data: %d, %d ", window->checksum, window->seqnum);
    int j;
    for(j = 0; j < MESSAGE_LENGTH; j++){
      printf("%c", window->payload[j]);
    }
    printf("\n");

    //maybe instead of calling to layer3, call a input directly?
    //call with window instead of packet?
    
  } //else { //add to list
    //place in waiting/being processed list

    
    // strncpy(waiting[seqNum].payload, packet->payload, MESSAGE_LENGTH);
    // waiting[seqNum].acknum = packet->acknum;
    // waiting[seqNum].checksum = packet->checksum;
    // waiting[seqNum].seqnum = packet->seqnum;

    
    

  //}

  //printf("-----------original window: %s, %d", window->payload, window->seqnum);
  //printf("-----------original waiting: %s, %d", waiting[seqNum].payload, waiting[seqNum].seqnum);

  //should end up in b input, sending from a to b
  
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
  printf("A INPUT HAS BEEN CALLED %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

  //receives packet from b_input through layer 3;
  //check if ack of packet is same as the ack of the packet in window right now; if so, then the packet is ok to be sent to layer5
  //flip ack

  //check a timer
   if(getTimerStatus(0)){ //still going, no timeout
     printf("A timer is still going\n");
     stopTimer(0);
   }

  unsigned int checkCheckSum = crc32(0, packet.payload, MESSAGE_LENGTH);
  printf("a input check sum calc: %u\n", checkCheckSum);
  printf("packet check sum: %u\n", packet.checksum);

  if((packet.checksum != checkCheckSum) || (window->acknum != packet.acknum)){
    printf("a input packet is corrupt!!!\n");
    //packet is corrupt and needs to be resent from a
    //B_timerinterrupt();
    startTimer(0, 10);
    tolayer3(0, packet);
  } else {
    //packet is fine and has been sent to layer 5; remove from window and send next packet with reversed ack bit
    //waiting[packet.seqnum]; //remove from list TODO

    struct pkt newPack;
    window++;

    //get next packet in list
    if(window->payload != NULL){
      printf("widnow next paylpoad ism't null!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

       strncpy(newPack.payload, window->payload, MESSAGE_LENGTH);
       newPack.seqnum = window->seqnum;
       newPack.checksum = window->checksum;
      
      if(packet.acknum == 1){
        newPack.acknum = 0;
      } else {
        newPack.acknum = 1;
      }

      

      int i;
      printf("values for newPack in a input: %d, %d, ", newPack.seqnum, newPack.checksum);
      for (i=0; i < MESSAGE_LENGTH; i++)  
            printf("%c", newPack.payload[i]);
      printf("\n");

      printf("a input packet is fine, sending to layer 3!!!\n");
      startTimer(0, 10);
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
  printf("A INTERRUPT CALLED*************\n");
   if(getTimerStatus(0)){
     stopTimer(0);
   }

  //resend damaged packet; need to get it from currend window, right? since can only send one at a time
  //do i need to fix whatever the error is or just resend?
  printf("resending packet to B##################");
  startTimer(0, 10);
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

   if(getTimerStatus(1)){
     printf("B timer is still going\n");
     stopTimer(1);
   }

  //check checksum
  unsigned int crc = crc32(0, packet.payload, MESSAGE_LENGTH);
  printf("b input check sum calc: %u\n", crc);
  printf("packet check sum: %u\n", packet.checksum);

  struct msg *message = malloc(sizeof(struct msg));
  strncpy(message->data, packet.payload, MESSAGE_LENGTH);

  if(packet.checksum != crc){
    printf("b input packet is corrupt!!!\n");
    //checksum has been scrambled
    //retransmit packet! 

    ///don't do anything here? can't send nak, so wait for a interrupt to retransmit itself?
    startTimer(1, 10);
    tolayer3(1, packet);
  } else {
    printf("b input packet is fine, sending to both layers!!!\n");
    //packet is theoretically fine, can send back an ACK of same value
    tolayer3(1, packet);
    //start b's timer; use to check if be receives new message of different ack or not
    startTimer(1, 10); 
    //can also send to layer5 here since the packet is ok
    tolayer5(1, *message);
  }


}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  //check if a timer is still running - it shouldn't be, but just in case
  printf("B INTERRUPT CALLED*************\n");
  if(getTimerStatus(1)){
    stopTimer(1);
  }

  //resend damaged packet; need to get it from currend window, right? since can only send one at a time
  //do i need to fix whatever the error is or just resend?
  printf("resending packet to A&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
  startTimer(1, 10);
  //send back to a to be reprocessed
  tolayer3(1, *window);
}

/*
 * The following routine will be called once (only) before any other
 * entity B routines are called. You can use it to do any initialization
 */
void B_init() {
}

