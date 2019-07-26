/**
 * Interface for comms library
 * include this header file in your code to use the library
 */

#ifndef COMMS_H
#define COMMS_H

#define PUB "pub"
#define SUB "sub"
#define INPROC "inproc"
#define IPC "ipc"

#define COMMS_NONBLOCKING 1


/** struct to hold topic configs for the user
 * name: the name assigned to the topic
 * role: PUB or SUB - the role of the node in the topic
 * transport: IPC or INPROC
 * address: address of the topic
 */
typedef struct {
	char *name;
	char *role;
	char *transport;
	char *address;
} topic_info_t;

/** array of topic_info_t
 * is used to provide user with topic information extracted from the config file
 * editing the values of these structs won't affect the program in any way
 * num_topics: size of array
 * topics: array of (topic_info_t *)
 */
typedef struct {
	int num_topics;
	topic_info_t **topics;
} topic_info_array_t;

/** struct for messages returned by subscribe()
 * size: size of the message in bytes
 * msg: the message
 */
typedef struct {
	size_t size;
	void *msg;
} msg_t;


/**
 * call exactly once in each process before using comms library (this is very important)
 * this function basically just sets up the 0MQ context for you
 * returns 0 on success, -1 on failure and sets errno accordingly
 */
int init_comms();

/**
 * setting up node's topics for use
 * call exactly once for each node before calling publish or subscribe
 * config_path: path to JSON configuration file
 * topic_list: reference to an empty topic_list_t if you want to get topic info, NULL if you don't want the info
 * returns: pointer to a node struct to pass as an argument to publish() & subscribe()
 * 			struct will be mallloc'ed by the init_node function
 * 			assumption: this space won't be deallocated or messed with by the user
 * returns NULL if error & sets errno
 */
void *init_node(char *config_path, topic_info_array_t *info_array);

/**
 * builds a zmq message and sends it on the specified topic
 * topic: name of the topic message is being sent on
 * msg: the message to be sent. Will be passed on to zmq's internal message creation functions
 * size: size of the message in bytes
 * flags: COMMS_NONBLOCKING to make the function non-blocking
 * node: the pointer that was returned by init_node()
 * returns 0 on success, -1 on failure and sets errno accordingly
 */
int publish(char *topic, void *msg, size_t size, int flags, void *node);

/**
 * listens for a message on the specified topic and returns a pointer to a received message
 * topic: name of topic to listen on
 * flags: COMMS_NONBLOCKING to make function non-blocking
 * 			if non-blocking is specified, errno will be set if there's no message
 * node: the pointer that was returned by init_node()
 * returns pointer to received message on success, NULL if error/no message
 */
msg_t *subscribe(char *topic, int flags, void *node);

/** 
 * call to free a message that you received from subscribe() when you don't need it anymore
 */
void close_msg(msg_t *msg);

#endif