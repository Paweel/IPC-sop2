#include "inf122499_queue.h"

void addEnd(struct list  **head, void * element, int size)
{
	struct list * temp;
	temp = *head;
	if (*head == NULL)
	{
	(*head) = malloc(sizeof(struct list));
	(*head)->data = malloc(size);
	memcpy((*head)->data,element,size);
	(*head)->next = NULL;
	return;
	}
	while (temp->next != NULL) temp = temp->next;
	temp->next = malloc(sizeof(struct list));
	temp->next->data = malloc(size);
	memcpy(temp->next->data,element,size);
	temp->next->next = NULL;
}
void addAsc(struct list **head, void * element, int size, int Larger(void*,void*))
{
	struct list * temp;
	temp = *head;
	struct list * new = malloc(sizeof(struct list));
	new->next = NULL;
	new->data = malloc(size);
	memcpy(new->data, element, size);
	if (*head == NULL)
	{
		(*head) = new;
	}
	else if (Larger(temp->data, element) == 1)
	{
		new->next = temp;
		(*head) = new;
	}
	else
	{
		while (temp->next != NULL)
		{
			if (Larger(temp->next->data, element) == 1) break;
			temp = temp->next;
		}
		new->next = temp->next;
		temp->next = new;
	}
}
int search(struct list *head, void * element, int Comp(void*, void*))
{
	struct list * temp;
	temp = head;
	int returnV = -1;
	int i = 0;
	while (temp != NULL)
	{
		if (Comp(temp->data, element) == 0)
		{
			returnV = i;
			break;
		}
		temp = temp->next;
		i++;
	}
	return returnV;
}
int compInt(void* a, void* b)
{
	if (*(int*)(a) > *(int*)b) return 1;
	if (*(int*)(a) < *(int*)b) return -1;
	return 0;
}
struct list * get(struct list * head, int i)
{
	struct list * temp;
	temp = head;
	while (temp != NULL)
	{
		if (i-- == 0) 
		{
				return temp;
		}
		temp = temp->next;
	}
	return NULL;
}
struct list * del(struct list ** head, int i)
{
	struct list * temp;
	temp = *head;
	while (temp != NULL)
	{
		if (i == 0)
		{
			struct list * toDel = *head;
			*head = temp->next;
			free(toDel->data);
			free(toDel);
			return *head;
		}
		else if (--i == 0) 
		{
			if (temp->next != NULL)
			{
				struct list * toDel = temp->next;
				temp->next = temp->next->next;
				free(toDel->data);
				free(toDel);
				return temp->next;
			}
		}
		temp = temp->next;
	}
	return NULL;
}
struct list * showElem(struct list * head, int i )
{
	struct list * temp = head;
	while (temp != NULL)
	{
		if (i-- == 0) return temp;
		temp = temp -> next;
	}
	return NULL;
}
struct cos
{
	int a1;
	int a2;
	char c;
};
void delList(struct list ** head)
{
	while (del(head,0));
}
/*
   void print(struct list * head)
   {
   struct list * temp;
   int i = 0;
   struct cos *tempCos;
   while ((temp = show(head, i++)) != NULL) 
   {
   tempCos = (temp->data);
   printf("%d: %d\t%d\t%c\n",i,tempCos->a1,tempCos->a2,tempCos->c);
   }

   }
   int main()
   {
   int i;
   struct list * head = NULL;
   struct cos tempCos;
   for(int i=0; i < 10 ; i++)
   {
   tempCos.a1 = i;
   tempCos.a2 = -123450 - i;
   tempCos.c = 'a' + i;
   addEnd(&head,&tempCos,sizeof(tempCos));
   printf("TEST%d\n",i);
   }
   i = 255;
   delList(&head);
   for(int i=0; i < 10 ; i++)
   {
   tempCos.a2 = i;
   tempCos.a1 = -123450 - i;
   tempCos.c = 'A' + i;
   addEnd(&head,&tempCos,sizeof(tempCos));
   printf("TEST%d\n",i);
   }
   print(head);
   return 0;
   }
   */
