#include <comms.h>
#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define PUB_PATH "../test/config/pub_inproc.json"	// TODO: make config paths absolute
#define SUB_PATH "../test/config/sub_inproc.json"

void print_configs(topic_info_array_t *configs) {
	printf("number of topics: %d\n", configs->num_topics);
	for(int i=0; i<configs->num_topics; i++) {
		printf("%s\n\trole: %s\n\ttransport: %s\n\taddress: %s\n", 
			configs->topics[i]->name, 
			configs->topics[i]->role, 
			configs->topics[i]->transport, 
			configs->topics[i]->address);
	}
}

void print_message(msg_t *msg) {
	assert(msg != NULL);
	assert(msg->msg != NULL);
	printf("printing message:\n");
	printf("%s", (char *)(msg->msg));
}

void recv(char *topic, int flag, char *s_flag, void *node) {
	printf("receiving message...\n");
	msg_t *msg = subscribe(topic, flag, node);
	if (msg == NULL) perror(s_flag);
	else {
		printf("message received!\n");
		print_message(msg);
		close_msg(msg);
	}
}

void send(char *topic, void *msg, int size, void *node) {
	if (publish("topic", msg, size, 0, node) != 0) {
		perror("publish:");
		exit(1);
	}
	printf("message sent\n");
}

void *publisher() {
	void *node = init_node(PUB_PATH, NULL);
	if (node == NULL) {
		perror("publisher: error in init_node:");
		exit(1);
	}
	printf("publisher socket initialized\n");
	sleep(6);
	if (publish("pub1", "hello\n", 7, 0, node) != 0) {
		perror("publish:");
		exit(1);
	}
	printf("message sent\n");
}

void *subscriber(void *vargp) {
	void *node = init_node(SUB_PATH, NULL);
	if(node == NULL) {
		perror("subscriber: error in init_node:");
		exit(1);
	}
	printf("subscriber socket initialized\n");
	sleep(5);
	recv("sub1", 0, "blocking", node);
}

void *subscriber2(void *vargp) {
	void *node = init_node(SUB_PATH, NULL);
	if (node == NULL) {
		perror("subscriber: error in init_node:");
		exit(1);
	}
	printf("subscriber socket initialized\n");
	sleep(5);
	recv("sub1", COMMS_NONBLOCKING, "non-blocking", node);
}

void main() {
	if (init_comms()) {
		perror("main: error in init_comms:");
		exit(1);
	}
	pthread_t pub_id, sub0_id, sub1_id, sub2_id, sub3_id; 
    pthread_create(&pub_id, NULL, publisher, NULL); 
    pthread_create(&sub0_id, NULL, subscriber, NULL);
    pthread_create(&sub1_id, NULL, subscriber2, NULL);
    sleep(5);
    pthread_create(&sub2_id, NULL, subscriber, NULL);
    pthread_create(&sub3_id, NULL, subscriber2, NULL);
    pthread_join(pub_id, NULL);
    pthread_join(sub0_id, NULL);
    pthread_join(sub1_id, NULL);
    pthread_join(sub2_id, NULL);
    pthread_join(sub3_id, NULL);
    exit(0); 
}
