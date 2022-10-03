/*
 * csc360_list.c
 * Linked List implementation for Assignment 1 csc360 Fall 2022
 */

#include "csc360_list.h"

Node *new_node(int pid, char* process_name, int run_state) {
	Node *temp = (struct Node*)malloc(sizeof(struct Node));
	temp->pid = pid;
	temp->run_state = run_state;
	temp->process_name = process_name;
	temp->next = NULL;
	return temp;
}

Node *add_front(Node *list, Node *new) {
	new->next = list;
	return new;
}

Node *add_end(Node *list, Node *new) {
	Node *curr;

	if (list == NULL) {
		new->next = NULL;
		return new;
	}

	for (curr = list; curr->next !=NULL; curr = curr->next);
	curr->next = new;
	new->next = NULL;
	return list;
}

void remove_node(Node *head, int pid) {

	if(head ==  NULL) return;//no list
	Node *prev = NULL;
	Node *curr = head;

	while(curr == NULL || curr->pid != pid) {
		if(curr == NULL) {	
			return;
			//pid not in list
		}
		prev = curr;
		curr = curr->next;
	}
	if(curr == head) {
	//target is head
		head = curr->next;
	}
	if(prev != NULL) {
	//target not head
		prev->next = curr->next;
	}
}

Node *find_node(Node *head, int pid) {
	Node *curr = head;
	while(curr == NULL || curr->pid != pid) {
		if(curr == NULL) {
			break;	
		}
		curr = curr->next;
	}
	return curr;
}

void print_list(Node *head) {
	Node* curr = head;
	int process_counter = 0;
	while(curr != NULL) {
		printf("%i:\t%s\n", curr->pid, curr->process_name);
		curr = curr->next;
	}
	printf("Total background jobs:\t%i\n", process_counter);
}

void free_list(Node *head) {

	//Input: any allocated List
	//Output:	None
	//Frees memory for an entire list
	Node *temp;
	while(head != NULL) {
		temp = head;
		head = head->next;
		free(temp);
	}
}
