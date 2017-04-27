#pragma once
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
struct list
{
	void * data;
	struct list * next;
};

void addEnd(struct list ** head, void * element, int size);
struct list *del(struct list ** head, int i);
void delList(struct list ** head);
struct list * showElem(struct list * head, int i);
void addAsc(struct list **head, void * element, int size, int Larger(void*,void*));
int compInt(void* a, void* b);
int search(struct list *head, void * element, int Comp(void*, void*));
struct list * get(struct list *head, int i);
