/**
 * source file for publish() & subscribe()
 */

#include <zmq.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "comms.h"

#define NODECODE 12345	/* value of safety in node_t struct */


/* -----------------------------------TYPEDEFS---------------------------------------- */


/** same types as the ones in init.c
 * see init.c for more info on these
 */

typedef struct {
	char *name;
	void *socket;
} topic_t;

typedef struct {
	int nodecode;			/* an attempt at solving the (void *) casting problem */
	topic_t **topics;
	int num_topics;
} node_t;


/* --------------------------HELPER FUNCTION PROTOTYPES-------------------------------- */


/** Helper for both publish() & subscribe()
 * name: name of the topic to find
 * node: the node that should contain the topic
 * returns the topic_t whose name matches name on success, -1 on failure & sets errno accordingly
 */
static topic_t *find_topic(char *name, node_t *node);

/** Helper for publish()
 * passed to zmq_msg_init_data
 */
static void free_data(void *data, void *hint);


/* -----------------------PUBLIC FUNCTION IMPLEMENTATIONS------------------------------ */


int publish(char *topic, void *msg, size_t size, int flags, void *node) {
	if (node == NULL) {
		errno = EINVAL;	/* invalid argument */
		return -1;
	}
	/* set flags */
	int flg = 0;
	if ( (flags & COMMS_NONBLOCKING) == COMMS_NONBLOCKING ) { 
		flg = ZMQ_DONTWAIT; 
	}
	/* format msg as a zmq_msg_t */
	void *data = malloc(size);
	if (data == NULL) return -1;
	memcpy(data, msg, size);
	zmq_msg_t message;
	if (zmq_msg_init_data(&message, data, size, free_data, NULL) == -1) { return -1; }
	/* send message */
	topic_t *t = find_topic(topic, (node_t *)node);
	if (t == NULL) return -1;
	if (zmq_msg_send(&message, t->socket, flg) == -1) {
		return -1;
	}
	else return 0;
}

msg_t *subscribe(char *topic, int flags, void *node) {
	if (node == NULL) {
		errno = EINVAL;	/* invalid argument */
		return NULL;
	}
	/* set flags */
	int flg = 0;
	if ( (flags & COMMS_NONBLOCKING) == COMMS_NONBLOCKING ) { flg = ZMQ_DONTWAIT; }
	/* receive message */
	topic_t *t = find_topic(topic, (node_t *)node);
	if (t == NULL) return NULL;
	zmq_msg_t message;
	zmq_msg_init(&message);
	int r = zmq_msg_recv(&message, t->socket, flg);
	if (r == -1) return NULL;
	/* extract msg from message */
	msg_t *msg = malloc(sizeof(msg_t));
	if (msg == NULL) return NULL; 
	msg->size = zmq_msg_size(&message);
	void *data = zmq_msg_data(&message);
	if ((msg->msg = malloc(msg->size)) == NULL) { return NULL; }
	memcpy(msg->msg, data, msg->size);
	/* close message & return msg */
	zmq_msg_close(&message);
	return msg;
}

void close_msg(msg_t *msg) {
	if (msg == NULL) return;
	free(msg->msg);
	free(msg);
}


/* -------------------------HELPER FUNCTION IMPLEMENTATIONS---------------------------- */


static topic_t *find_topic(char *name, node_t *node) {
	if (node->nodecode != NODECODE) {
		errno = EINVAL;	/* invalid argument: node */
		return NULL;
	}
	for (int i=0; i<node->num_topics; i++) {
		if (strcmp(node->topics[i]->name, name) == 0) {
			return node->topics[i];
		}
	}
	errno = EINVAL;	/* invalid argument: name */
	return NULL;
}

static void free_data(void *data, void *hint) {
	free(data);
}