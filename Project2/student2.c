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

//ack values: 0 or 1
//sequence number values: 0 or 1
//need a method to store waiting messages in a buffer
//need a way to get messages from that buffer
int currentACK; //0 or 1
int currentSEQ; //0 or 1

int aState; //0 free, 1 if waiting for message (occupied)
int bState;

struct pkt* inTransit; //pointer to the packet currently be transmitted between a and b

//messages waiting in line to be sent
typedef struct{
  struct msg waiting[10]; //those waiting in line
  struct msg *front;
  struct msg *end;
  int index;
  int total;
} line;

line waitingLine;
int createCheckSum(char message[MESSAGE_LENGTH], int ack, int seq);
void sendAcknowledgement(int ack, int entity);

/*
 * A_output(message), where message is a structure of type msg, containing
 * data to be sent to the B-side. This routine will be called whenever the
 * upper layer at the sending side (A) has a message to send. It is the job
 * of your protocol to insure that the data in such a message is delivered
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
  printf("Message in a_output: %s\n", message.data);

  if(aState == 0){ //free and waiting for new data
    
    //create packet
    struct pkt* packet = (struct pkt*)malloc(sizeof(struct pkt));
    packet->acknum = currentACK;
    packet->seqnum = currentSEQ;
    packet->checksum = createCheckSum(message.data, packet->acknum, packet->seqnum);
    strncpy(packet->payload, message.data, MESSAGE_LENGTH);

    printf("packet sent to B input from a output: %s, %d, %d\n", packet->payload, packet->seqnum, packet->acknum);
    tolayer3(AEntity, *packet);
    aState = 1;
    //start timer

  } else { //something is in transit, a is either waiting for an ack or is moving a packet
    printf("A is in use, message placed in line to wait***********************\n");
    //store message in line for the time being
    if(waitingLine.index == 9){
      //reaching end of line, need to loop around and start placing at beginning of buffer again
      waitingLine.index = 0;
    }
    strncpy(waitingLine.waiting[waitingLine.index].data, message.data, MESSAGE_LENGTH);
    //point end at the last placed message in line
    waitingLine.end = &waitingLine.waiting[waitingLine.index];
    waitingLine.index++;
    waitingLine.total++;
    printf("Number of messages currently in line: %d\n", waitingLine.total);
    return;

  }
  

  //account for messages waiting
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
  //check the ack sent from b and compare to current ack; if not the same, it's a nak
  if(packet.acknum != currentACK){
    //NAK, resend packet to B
    return;

  }

  //create next packet to be sent
  struct pkt *next = (struct pkt*)malloc(sizeof(struct pkt));
  //flip em
  next->acknum = 1 - currentACK;
  next->seqnum = 1 - currentSEQ;
  strncpy(next->payload, waitingLine.front->data, MESSAGE_LENGTH);
  next->checksum = createCheckSum(next->payload, next->acknum, next->seqnum);
  
  printf("packet sent to B input from a input: %s, %d, %d\n", next->payload, next->seqnum, next->acknum);
  waitingLine.front++;
  
  //everything is fine and it was a positive ack
  //grab next message on queue and increment first pointer in waiting line
  tolayer3(AEntity, *next);

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
  //check if a timer is still running - it shouldn't be, but just in case
 

}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  
  currentSEQ = 0;
  currentACK = 0;

  //start program off with both sides open and waiting for data
  aState = 0;
  bState = 0;

  //used to keep track of each messages' index within the waitingLine
  waitingLine.index = 0;
  waitingLine.front = &waitingLine.waiting[0];
  
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
  //check the checksum, seqnum, and acknum
  //if any bad, ask for retransmit
  //start timer

  //checksum
  if(packet.checksum != createCheckSum(packet.payload, packet.acknum, packet.seqnum)){

    //send nak
    return;
  }

  //out of order
  if(packet.seqnum != currentSEQ){

  } 

  //wrong ack, not sure if need to check here
  if(packet.acknum != currentACK){

  }

  //if all good, send to layer5
  struct msg *message = (struct msg*)malloc(sizeof(struct msg));
  strncpy(message->data, packet.payload, MESSAGE_LENGTH);

  printf("Message sent to layer 5 from b: %s\n", message->data);
  //let a know that alls good and can grab next message from list to transmite
  sendAcknowledgement(packet.acknum, BEntity);
  tolayer5(BEntity, *message);
  
  


}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  
  
}

/*
 * The following routine will be called once (only) before any other
 * entity B routines are called. You can use it to do any initialization
 */
void B_init() {
}

//generate the checksum for each packet in the system using
//relevant packet information
int createCheckSum(char message[MESSAGE_LENGTH], int ack, int seq){
  char buffer[MESSAGE_LENGTH + 2];
  strncpy(buffer, message, MESSAGE_LENGTH);
  buffer[MESSAGE_LENGTH] = (char)ack;
  buffer[MESSAGE_LENGTH + 1] = (char)seq;
  
  int check = crc32(0L, buffer, MESSAGE_LENGTH + 2);

  return check;
}

//use to send naks and acks between A and B sides
void sendAcknowledgement(int ack, int fromEntity){
  printf("Acknowldgement sent!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  struct pkt *pack = (struct pkt*)malloc(sizeof(struct pkt));
  pack->acknum = ack;
  pack->checksum = createCheckSum(pack->payload, pack->acknum, pack->seqnum);
  tolayer3(fromEntity, *pack);
}
