#include <comms.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PUB_PATH "./test/config/pub_ipc.json"
#define SUB_PATH "./test/config/sub_ipc.json"

void print_configs(topic_info_array_t *configs);
void print_message(msg_t *msg);
void recv(char *topic, int flag, char *s_flag, void *node);
void send(char *topic, void *msg, int size, void *node);

void publisher() {
	init_comms();
	//sleep(2);
	void *pnode = init_node(PUB_PATH, NULL);
	if (pnode == NULL) {
		perror("publisher: error in init_node:");
		exit(1);
	}
	printf("publisher socket initialized\n");
	send("pub1", "hey\n", 5, pnode);
}

void subscriber() {
	init_comms();
	void *snode = init_node(SUB_PATH, NULL);
	if (snode == NULL) {
		perror("subscriber: error in init_node:");
		exit(1);
	}
	printf("subscriber socket initialized\n");
	recv("sub1", 0, "blocking", snode);
}

int main() {
	int ppid, spid;
	int pstatus, sstatus;
	if ((ppid = fork()) == 0) {
		printf("forked: calling publisher\n");
        publisher(); 
	}
	else if ((spid = fork()) == 0) {
		printf("forked: calling subscriber\n");
		subscriber();
	}
	else {
		if (ppid == -1) perror("publisher fork failed:");
		else waitpid(ppid, &pstatus, 0);
		if (spid == -1) perror("subscriber fork failed:");
		else waitpid(spid, &sstatus, 0);
		printf("pstatus: %d\nsstatus: %d\n", pstatus, sstatus);
	}
	return 0;
}



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