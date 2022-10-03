/* csc360_list.h
 * Header file for a Linked List Class
 * Linked List data structure to record background process'
*/

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
	int pid;
	int run_state;
	char* process_name;
	struct Node *next;
}Node;

//Node *head;

Node *new_node(int pid,char* process_name, int run_state);
Node *add_front(Node *list, Node *new);
Node *add_end(Node *list, Node *new);
void remove_node(Node *head, int pid);
Node *find_node(Node *head, int pid);
void free_list(Node *head);

#endif
