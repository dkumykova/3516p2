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

/* define a buffer and buffer handling routines to allow holding messages that
 * sent while previous message is in transit
 */
#define BUFFER_LENGTH 10

struct buffer {
	int count;
	struct msg* read;
	struct msg* write;
	struct msg data[BUFFER_LENGTH];
};

struct buffer bufferA;

void initBuffer(struct buffer* buf);
void messageToBuffer(struct buffer* buf, struct msg message);
struct msg* messageFromBuffer(struct buffer* buf);
void displayBuffer(struct buffer* buf);

#define TIMER 30


int validate_checksum(struct pkt);
int generate_checksum(struct pkt);
void sendACK(int AorB, int ack);

void A_sendMessage(struct msg message);


#define WAIT_DATA 0
#define WAIT_ACK 1

int a_state = 0;
int b_state = 0;
int a_acknum = 0;
int b_acknum = 0;
int a_seqnum = 0;
int b_seqnum = 0;
int prev_seqnum;

int failure_count = 0;

struct msg a_prev_message;
struct pkt a_prev_packet;
struct pkt b_prev_packet;

/*
 * A_output(message), where message is a structure of type msg, containing
 * data to be sent to the B-side. This routine will be called whenever the
 * upper layer at the sending side (A) has a message to send. It is the job
 * of your protocol to insure that the data in such a message is delivered
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
    if (a_state != WAIT_DATA) {
        printf("A_output: unprocessed packet in transit, buffer message: %s\n", message.data);
        messageToBuffer(&bufferA, message);
        return;
        // Have to add to buffer and send later
    }
    A_sendMessage(message);
}

void A_sendMessage(struct msg message) {
    printf("A_output: send packet: %s\n", message.data);
    struct pkt packet;
    packet.seqnum = a_seqnum;
    strncpy(packet.payload, message.data, MESSAGE_LENGTH);
    packet.checksum = generate_checksum(packet);
    a_prev_packet = packet;
    a_state = WAIT_ACK;
    tolayer3(AEntity, packet);
    startTimer(AEntity, TIMER);
}
/*
 * Just like A_output, but residing on the B side.  USED only when the
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
	 printf("B_output: one direction only, ignore.\n");
}

/*
 * A_input(packet), where packet is a structure of type pkt. This routine
 * will be called whenever a packet sent from the B-side (i.e., as a result
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side.
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
    if (a_state != WAIT_ACK) {
        printf("A_input: do not expecting ACK drop.\n");
        return;
    }
    if (! validate_checksum(packet)) {
        printf("A_input: packet corrupted. drop.\n");
        return;
    }

    if (packet.acknum != a_seqnum) {
        printf("A_input: out of sequence. drop.\n");
        return;
    }
    printf("A_input: ACK!.\n");
    stopTimer(AEntity);
    a_seqnum = 1 - a_seqnum;
    //check if we have buffered message

    struct msg* pmessage;

    pmessage = messageFromBuffer(&bufferA);
    if (pmessage != NULL) {
    	A_sendMessage(*pmessage);
    } else {
    	a_state = WAIT_DATA;
    }
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
    if (a_state != WAIT_ACK) {
        printf("A_timerinterrupt: not waiting for ACK. ignore\n");
        return;
    }
    printf("A_timerinterrupt: resend last packet: %s.\n", a_prev_packet.payload);
    stopTimer(AEntity);
    tolayer3(AEntity, a_prev_packet);
    startTimer(AEntity, TIMER);
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	initBuffer(&bufferA);
	a_state = WAIT_DATA;
	a_acknum = 0;
	a_seqnum = 0;
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
    if (! validate_checksum(packet)) {
        printf("B_input: packet corrupted. send NAK.\n");
        sendACK(BEntity, 1 - b_seqnum);
        return;
    }
    if (packet.seqnum != b_seqnum) {
        printf("B_input: out of sequence. send NAK.\n");
        sendACK(BEntity, 1 - b_seqnum);
        return;
    }
    printf("B_input: message: %s\n", packet.payload);
    printf("B_input: send ACK.\n");
    sendACK(BEntity, b_seqnum);
    struct msg message;
    strncpy(message.data, packet.payload, MESSAGE_LENGTH);
    tolayer5(BEntity, message);
    b_seqnum = 1 - b_seqnum;
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
    printf("B_timerinterrupt: not using. ignore.\n");
}

/*
 * The following routine will be called once (only) before any other
 * entity B routines are called. You can use it to do any initialization
 */
void B_init() {
	b_seqnum = 0;
}

void sendACK(int AorB, int ack) {
    struct pkt packet;
    packet.acknum = ack;
    packet.checksum = generate_checksum(packet);
    tolayer3(AorB, packet);
}

int validate_checksum(struct pkt packet) {
	int sum = generate_checksum(packet);
	return (sum == packet.checksum);
}

int generate_checksum(struct pkt packet) {
	char buf[MESSAGE_LENGTH + 2];
	strncpy(buf, packet.payload, MESSAGE_LENGTH);
	buf[MESSAGE_LENGTH] = (char)packet.acknum;
	buf[MESSAGE_LENGTH + 1] = (char)packet.seqnum;
	int crc = crc32(0L, buf, MESSAGE_LENGTH + 2);
	return crc;
}

void initBuffer(struct buffer* buf) {
	buf->read = &(buf->data[0]);
	buf->write = &(buf->data[0]);
	buf->count=0;
}

void messageToBuffer(struct buffer* buf, struct msg message) {
	if ((buf->write + 1 == buf->read) || (buf->write + 1 == buf->read + BUFFER_LENGTH)) {
		printf ("Buffer is full\n");
		messageFromBuffer(buf);
		return;
	}
	printf("Incoming message: %s\n", message.data);
	strncpy(buf->write->data, message.data, MESSAGE_LENGTH);
	buf->write++;
	if (buf->write > &buf->data[0] + BUFFER_LENGTH - 1) {
		buf->write = &buf->data[0];
	}
	buf->count = buf->count + 1;
	printf("Number of buffered Messages = %d\n", buf->count);
}

struct msg* messageFromBuffer(struct buffer* buf) {
	if (buf->read == buf->write) {
		return NULL;
	} else {
		struct msg* retval = buf->read;
		buf->read++;
		if (buf->read > &buf->data[0] + BUFFER_LENGTH - 1) {
			buf->read = &buf->data[0];
		}
		buf->count = buf->count - 1;
		return retval;
	}
}

void displayBuffer(struct buffer* buf) {
	struct msg* current = buf->read;
	char output[MESSAGE_LENGTH];
	while (current != buf->write) {
		strncpy(output, current->data, MESSAGE_LENGTH-1);
		printf ("buffer content at %u : %s\n", current, output);
		current++;
		if (current > &buf->data[0] + BUFFER_LENGTH - 1) {
			current = &buf->data[0];
		}
	}
	int i;
	for (i = 0; i < BUFFER_LENGTH ; i++) {
		strncpy(output, buf->data[i].data, MESSAGE_LENGTH);
		printf("buffer content at index %u : %s\n", i, output);
	}
}

