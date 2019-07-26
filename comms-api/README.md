# Communications API
##### simplified interface for select functionalities of the ZeroMQ library

### Functionalities

| Transport modes | Communication modes |
| ---- | ---- |
| 1. inproc: in-process (inter-thread) <br>2. ipc: local inter-process | simple pub-sub only|

### Dependencies
+ ZeroMQ packages: (version 4.2.5)
	+ libzmq3-dev
	+ libzmq5
+ JSON-C packages:
	+ json-c-dev
	+ json-c3

This library will be designed to be thread-safe with its intended use

## How to use

>#include <comms.h>


### Functions
	
```
int init_comms()
```
**call exactly once in every process before calling any of the other comms functions**<br>
this function initializes the ZeroMQ context for you
+ returns 0 on success, -1 on failure and sets errno accordingly

```
void *init_node(char *config_path, topic_info_array_t *info_array)
```
 
**call exactly once in every node before using publish() or subscribe()**<br>
initializes the sockets for each topic
+ config_path: path to the config file for the node<br>
+ info_array: reference to a topic_info_array_t<br>
optional, for if you want to receive the config info in a struct
+ returns a node pointer on success, NULL on failure and sets errno accordingly.
>the returned pointer contains state information needed by the library functions<br>
>it should be sent as the last argument to publish() & subscribe()

```
int publish(char *topic, void *msg, size_t size, int flags, void *node)
```
builds a zmq message and sends it on the specified topic
+ topic: name of the topic to send a message on
+ msg: message to send
+ size: size of the message in bytes
+ flags: set flag COMMS_NONBLOCKING to make function non-blocking
+ node: pointer returned by init_node
+ returns 0 on success, -1 on failure and sets errno accordingly
```
msg_t *subscribe(char *topic, int flags, void *node)
```
listens for a message on the specified topic and returns a pointer to a received
message.
+ topic: name of the topic
+ flags: set flag COMMS_NONBLOCKING to make function non-blocking
+ node: pointer returned by init_node
+ returns a struct containing the received message and the message size
```
void close_msg(msg_t *msg)
```
call to deallocate messages retrieved from subscribe() that are no longer needed

### Nodes
	
A node is an entity that needs to communicate with other entities. <br>
The only restriction is that one C thread can only correspond to one node. <br>
-> in other words, a node can be any individual thread.

### Config files

each node needs to have one JSON config file for all of its topics
##### Sample config file setup:
```
{
	"topics": [
		{
			"name":"topic-a"
			"role":"pub"
			"transport":"ipc"
			"address":"/tmp/topic-a"
		},
		{
			"name":"topic-b"
			"role":"sub"
			"transport":"inproc"
			"address":"literally-any-name"
		}
	]
}
```
### Topics

A topic is a channel on which nodes can communicate
 
>	Topics can be either *ipc* or *inproc*<br>
		- an ipc topic is for communication between nodes that are each in separate processes<br>
		- an inproc topic is for communication between nodes that are all in the same process

>	All topics are in the pub/sub model<br>
		- each topic must have exactly 1 node in the publisher role
		- the publisher can publish messages, and all subscribers receive all messages

> Every node in a topic must include the topic in their config file

In the config file, a topic has 4 fields:

	name: a name for the user to identify the topic. 

	role: "pub" or "sub"
			"pub": means the node publishes on that topic -> uses publish()
			"sub": means the node subscribes to that topic -> uses subscribe()

	transport: "ipc" or "inproc"

	address: the format of the address depends on the transport mode
			ipc: path to a file in /tmp
				If the file doesn't already exist, init_node() will automatically create it
				*two ipc topics cannot share the same file
			inproc: the address can be any string
				*two inproc topics in the same process cannot have the same address