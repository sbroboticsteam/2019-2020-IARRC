#include <comms.h>
#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define PATH1 "../test/config/ps1.json"	// TODO: make config paths absolute
#define PATH2 "../test/config/ps2.json"

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
	printf("receiving message on %s...\n", topic);
	msg_t *msg = subscribe(topic, flag, node);
	if (msg == NULL) perror(s_flag);
	else {
		printf("message received!\n");
		print_message(msg);
		close_msg(msg);
	}
}

void send(char *topic, void *msg, int size, void *node) {
	if (publish(topic, msg, size, 0, node) != 0) {
		perror("publish:");
		return;
	}
	printf("message sent on %s\n", topic);
}

void *ps1(void *args) {
	void *node = init_node(PATH1, NULL);
	if (node == NULL) {
		perror("ps1: error in init_node:");
		exit(1);
	}
	printf("ps1 sockets initialized\n");
	sleep(1);

	send("sub1", "hey\n", 5, node);	// should give ENOTSUP
	send("pub2", "hey\n", 5, node);	// should give EINVAL
	send("pub1", "hey\n", 5, node);	// should be success

	recv("pub1", 0, "blocking", node);	// should give ENOTSUP
	recv("sub2", 0, "blocking", node);	// should give EINVAL
	recv("sub2", COMMS_NONBLOCKING, "non-blocking", node);	// should give EINVAL
	recv("pub1", COMMS_NONBLOCKING, "non-blocking", node);	// should give ENOTSUP
	recv("sub1", COMMS_NONBLOCKING, "non-blocking", node);	// should give EAGAIN
	recv("sub1", 0, "blocking", node);	// should be success
}

void *ps2(void *args) {
	void *node = init_node(PATH2, NULL);
	if (node == NULL) {
		perror("ps2: error in init_node:");
		exit(1);
	}
	printf("ps2 sockets initialized\n");
	sleep(1);

	sleep(5);
	send("pub2", "hello\n", 7, node);
	recv("sub2", 0, "blocking", node);
}

int main() {
	if (init_comms()) {
		perror("main: error in init_comms:");
		exit(1);
	}
	pthread_t id1, id2; 
    pthread_create(&id1, NULL, ps1, NULL); 
    pthread_create(&id2, NULL, ps2, NULL);
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
	return 0;
}
