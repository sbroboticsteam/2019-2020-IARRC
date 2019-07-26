/**
 * source file for publish() & subscribe()
 */

#include <zmq.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "comms.h"


/* -----------------------------------TYPEDEFS---------------------------------------- */


/** same types as the ones in init.c
 * see init.c for more info on these
 */

typedef struct {
	char *name;
	void *socket;
} topic_t;

typedef struct {
	topic_t **topics;
	int num_topics;
} node_t;


/* --------------------------HELPER FUNCTION PROTOTYPES-------------------------------- */


/** Helper for both publish() & subscribe()
 * finds the topic_t in node whose name matches name and returns it
 * returns NULL if not found
 */
static topic_t *find_topic(char *name, node_t *node);

/** Helper for publish()
 * function for freeing zmq_msg_t
 */
static void free_message(void *data, void *hint);


/* -----------------------PUBLIC FUNCTION IMPLEMENTATIONS------------------------------ */


int publish(char *topic, void *msg, size_t size, int flags, void *node) {
	/* set flags */
	int flg = 0;
	if ( (flags & COMMS_NONBLOCKING) == COMMS_NONBLOCKING ) { flg = ZMQ_DONTWAIT; }
	/* format msg as a zmq_msg_t */
	zmq_msg_t message;
	if (zmq_msg_init_data(&message, msg, size, free_message, NULL) == -1) { return -1; }
	/* send message */
	topic_t *t = find_topic(topic, (node_t *)node);
	if (t == NULL) {
		// TODO: set errno or print error message
		return -1;
	}
	return zmq_msg_send(&message, t->socket, flg);
}

msg_t *subscribe(char *topic, int flags, void *node) {
	/* set flags */
	int flg = 0;
	if ( (flags & COMMS_NONBLOCKING) == COMMS_NONBLOCKING ) { flg = ZMQ_DONTWAIT; }
	/* receive message */
	topic_t *t = find_topic(topic, (node_t *)node);
	if (t == NULL) {
		// TODO: set errno or print error message
		return NULL;
	}
	zmq_msg_t message;
	int r = zmq_msg_recv(&message, t->socket, flg);
	/* extract msg from message */
	msg_t *msg;
	if ((msg = malloc(sizeof(msg_t))) == NULL) { return NULL; }
	msg->size = zmq_msg_size(&message);
	void *data = zmq_msg_data(&message);
	if ((msg->msg = malloc(msg->size)) == NULL) { return NULL; }
	memcpy(msg->msg, data, msg->size);
	/* close message & return msg */
	zmq_msg_close(&message);
	return msg;
}

void close_msg(msg_t *msg) {
	free(msg->msg);
	free(msg);
}


/* -------------------------HELPER FUNCTION IMPLEMENTATIONS---------------------------- */


static topic_t *find_topic(char *name, node_t *node) {
	for (int i=0; i<node->num_topics; i++) {
		if (strcmp(node->topics[i]->name, name) == 0) {
			return node->topics[i];
		}
	}
	return NULL;
}

static void free_message(void *data, void *hint) {
	free(data);
}