/**
 * source file for init_comms() & init_node()
 */ 

#include <zmq.h>
#include <json-c/json.h>
#include <errno.h> 
#include <stdio.h>
#include <string.h>
#include <assert.h>
 
#include "comms.h" 

#define MAX_CONFIG_SIZE 1024	// TODO: make this a reasonable number
#define NODECODE 12345			/* value of nodecode in node_t struct */


/* -------------------------------GLOBAL VARIABLES------------------------------------ */


/**
 * ZeroMQ context: 
 * 1 unique context per process
 * has to be shared between threads on the same inproc topic
 * contexts are inherently thread-safe
 */
static void *g_context;	


/* -----------------------------------TYPEDEFS---------------------------------------- */


/** struct for a topic (name & socket)
 * name: name of the topic
 * socket: pointer to socket
 */
typedef struct {
	char *name;
	void *socket;
} topic_t;

/** struct to store all topics for a node
 * topics: array of (topic_t *) pointers
 * num_topics: size of array
 */
typedef struct {
	int nodecode;			/* an attempt at solving the (void *) casting problem */
	topic_t **topics;
	int num_topics;
} node_t;


/* --------------------------HELPER FUNCTION PROTOTYPES-------------------------------- */


/** Helper for init_node()
 * parses the JSON config file at PATH into a json_object
 * returns a json_object for the entire config file or NULL if error
 */
static struct json_object *parse_config(char *PATH);

/** Helper for init_node()
 * Iterate topics, which has size n. For each topic in topics:
 *		malloc a topic_info_t to store parsed topic information
 *		create socket using the topic info, and bind/connect it
 *		malloc a topic_t for the socket, and add it to sock_array
 *		if info_array == NULL, deallocate the topic_info_t
 *		else, add the topic_info_t to info_array
 * returns 0 on success, -1 for error & sets errno
 */
static int init_topics(struct array_list *topics, int n, topic_t **sock_array, topic_info_t **info_array);

/** Helper for init_topics()
 * parses information from a struct json_object into a topic_info_t and returns it
 * topic: a json_object, which should contain 1 dictionary that is 1 topic config
 * returns a topic_info_t containing topic's config info on success or NULL if error
 */
static topic_info_t *parse_topic(struct json_object *topic);

/** Helper for init_topics()
 * checks if the file at "/home/robot/Documents/2019-2020-IARRC/comms-api/ipcfiles/"+path exists, and creates the file if it doesn't exist yet
 */
static void ipc_file(char *path);

/** Helper for init_topics()
 * frees all the mallocs that are part of a topic_info_t
 */
static void free_topic_info(topic_info_t *p);


/* -----------------------PUBLIC FUNCTION IMPLEMENTATIONS------------------------------ */


int init_comms() { 
	g_context = zmq_ctx_new(); 
	if (g_context == NULL) return -1;
	else return 0;
}

void *init_node(char *config_path, topic_info_array_t *info_array) {
	
	/* PARSE CONFIG FILE */
	struct json_object *j_configs = parse_config(config_path);					/* j_configs: the entire config file, parsed */
	if ( j_configs == NULL ) return NULL;
	
	/* ALLOCATE node STRUCT */
	node_t *node = (node_t *)malloc(sizeof(node_t));							/* node: the node, to be filled w/ all socket information & returned to the user */
	if (node == NULL) return NULL;
	node->nodecode = NODECODE;
	
	/* EXTRACT ARRAYLIST OF TOPICS */
	struct json_object *j_topics;												/* j_topics: value for key "topics", should contain an arraylist */
	json_bool r = json_object_object_get_ex(j_configs, "topics", &j_topics);
	if (r == FALSE) {
		// TODO: set errno (incorrect config format)
		return NULL;
	}
	struct array_list *topics = json_object_get_array(j_topics);				/* topics: json arraylist extracted from j_topics | <json-c/arraylist.h> */
	if (topics == NULL) {
		// TODO: set errno (incorrect config format)
		return NULL;
	}
	
	/* INITIALIZE node->topics ARRAY */
	node->num_topics = array_list_length(topics);								/* node->num_topics: length of the node->topics array */
	node->topics = (topic_t **)malloc(node->num_topics * sizeof(topic_t *));	/* node->topics: array of (topic_t *) pointers */
	if (node->topics == NULL) return NULL;
	
	/* INITIALIZE info_array->topics ARRAY IF NEEDED */
	if (info_array != NULL) {
		info_array->num_topics = node->num_topics;								/* info_array->num_topics: length of the info_array->topics array */
		info_array->topics = (topic_info_t **)malloc(
			info_array->num_topics * sizeof(topic_info_t *));					/* info_array->topics: array of (topic_info_t *) pointers */
		if (info_array->topics == NULL) return NULL;
	}
	
	/* INITIALIZE & STORE ALL THE TOPICS & THEIR SOCKETS */
	int x;
	if (info_array != NULL) {
		x = init_topics(topics, node->num_topics, node->topics, info_array->topics);
	}
	else {
		x = init_topics(topics, node->num_topics, node->topics, NULL);
	}
	if (x == -1) return NULL;
	
	/* CAST & RETURN NODE */
	return (void *)node;
}


/* -------------------------HELPER FUNCTION IMPLEMENTATIONS---------------------------- */


static struct json_object *parse_config(char *path) {
	FILE *fp;
	char buffer[MAX_CONFIG_SIZE];
	struct json_object *parsed_json;
	fp = fopen(path,"r");
	if (fp == NULL) {
		return NULL;
	}
	size_t x = fread(buffer, 1, MAX_CONFIG_SIZE, fp);	/* read at most MAX_CONFIG_SIZE bytes */
	if (ferror(fp)) {
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	parsed_json = json_tokener_parse(buffer);
	if (parsed_json == NULL) {
		// TODO: set errno (file is not in correct json format)
	}
	return parsed_json;
}

static int init_topics(struct array_list *topics, int n, topic_t **sock_array, topic_info_t **info_array) {
	for(int i=0; i<n; i++) {
		
		/* PARSE INFORMATION FROM topics[i] INTO topic_info_t *info */
		topic_info_t *info = parse_topic(array_list_get_idx(topics, i));
		if (info == NULL) return -1;
		
		/* CHECK ROLE & TRANSPORT */
		int type;									
		if (strcmp(info->role, PUB) == 0) { type = ZMQ_PUB; }
		else if (strcmp(info->role, SUB) == 0) { type = ZMQ_SUB; }
		else { 
			// TODO: set errno (topic has invalid role)
			return -1;
		}
		if (strcmp(info->transport, IPC) != 0 && strcmp(info->transport, INPROC) != 0) {
			// TODO: set errno (topic has invalid transport)
			return -1;
		}
		
		/* CREATE A SOCKET FOR THE TOPIC */
		void *socket = zmq_socket(g_context, type);	
		if (socket == NULL) return -1;
		
		/* CONSTRUCT ENDPOINT */
		char *endpoint;
		if (strcmp(info->transport, IPC) != 0) {
			endpoint = malloc(9+strlen(info->address)+1);	/* "inproc://" + address + "\0" */
			if (endpoint == NULL) { return -1; }
			strcpy(endpoint, "inproc://");
		}
		else {
			endpoint = malloc(17+strlen(info->address)+1);	/* "ipc:///home/robot/Documents/2019-2020-IARRC/comms-api/ipcfiles/" + address + "\0" */
			if (endpoint == NULL) { return -1; }
			strcpy(endpoint, "ipc:///home/robot/Documents/2019-2020-IARRC/comms-api/ipcfiles/");
		}
		strcat(endpoint, info->address);
		
		/* CONNECT/BIND THE SOCKET */
		if (strcmp(info->transport, IPC) == 0) {	
			ipc_file(info->address);	/* make sure /home/robot/Documents/2019-2020-IARRC/comms-api/ipcfiles/+address exist for ipc */
		}
		if (type == ZMQ_PUB) {
			if (zmq_bind(socket, endpoint) != 0) return -1;
		}
		else {
			if (zmq_setsockopt(socket, ZMQ_SUBSCRIBE, NULL, 0) != 0) return -1;	/* all subs receive all messages */
			if (zmq_connect(socket, endpoint) != 0) return -1;
		}
		free(endpoint);
		
		/* ADD SOCKET TO sock_array ARRAY */
		topic_t *thistopic = malloc(sizeof(topic_t));
		if (thistopic == NULL) return -1;
		thistopic->socket = socket;
		thistopic->name = malloc(strlen(info->name)+1);
		if (thistopic->name == NULL) return -1;
		strcpy(thistopic->name, info->name);
		assert(sock_array != NULL);
		sock_array[i] = thistopic;
		
		/* TAKE CARE OF info STRUCT */
		if(info_array == NULL) {
			free_topic_info(info);			/* if info_array is NULL, deallocate info */
		}
		else {
			info_array[i] = info;			/* else, add info to info_array */
		}
	}
}

static topic_info_t *parse_topic(struct json_object *topic) {
	struct json_object *j_name;
	struct json_object *j_role;
	struct json_object *j_transport;
	struct json_object *j_address;
	topic_info_t *parsed = malloc(sizeof(topic_info_t));
	if (parsed == NULL) return NULL;
	if (json_object_object_get_ex(topic, "name", &j_name) == FALSE) {
		// TODO: set errno (missing config entry)
		return NULL;
	}
	if (json_object_object_get_ex(topic, "role", &j_role) == FALSE) {
		// TODO: set errno (missing config entry)
		return NULL;
	}
	if (json_object_object_get_ex(topic, "transport", &j_transport) == FALSE) {
		// TODO: set errno (missing config entry)
		return NULL;
	}
	if (json_object_object_get_ex(topic, "address", &j_address) == FALSE) {
		// TODO: set errno (missing config entry)
		return NULL;
	}
	const char *name 		= json_object_get_string(j_name);
	const char *role 		= json_object_get_string(j_role);
	const char *transport 	= json_object_get_string(j_transport);
	const char *address 	= json_object_get_string(j_address);
	parsed->name 		= malloc(strlen(name)+1);
	parsed->role 		= malloc(strlen(role)+1);
	parsed->transport 	= malloc(strlen(transport)+1);
	parsed->address 	= malloc(strlen(address)+1);
	if (parsed->name == NULL || parsed->role == NULL || parsed->transport == NULL || parsed->address == NULL) { return NULL; }
	strcpy(parsed->name, name);
	strcpy(parsed->role, role);
	strcpy(parsed->transport, transport);
	strcpy(parsed->address, address);
	return parsed;
}

static void ipc_file(char *path) {
	// TODO
	// check if /home/robot/Documents/2019-2020-IARRC/comms-api/ipcfiles/path exists & create it otherwise
}

static void free_topic_info(topic_info_t *p) {
	assert(p != NULL && p->name != NULL && p->role != NULL && p->transport != NULL && p->address != NULL);
	free(p->name);
	free(p->role);
	free(p->transport);
	free(p->address);
	free(p);
}
