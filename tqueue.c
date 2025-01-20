#include "tqueue.h"

typedef struct TQueueNode {
    struct TQueueNode* next;
    void* data;
} TQueueNode;

unsigned long int tqueue_enqueue(TQueue* q, void* data)
{
    TQueueNode* new_element = (TQueueNode*) malloc(sizeof(TQueueNode));
    new_element->data = data;
    unsigned long int index = 0;
    if (*q == NULL) {
        *q = new_element;
        new_element->next = *q;
    } else {
        TQueueNode* head = *q;
        index = 1;
        while(head->next != *q) {
            head = head->next;
            index++;
        }
        head->next = new_element;
        new_element->next = *q;
    }
    return index;
}

void* tqueue_pop(TQueue* q)
{
    if (*q == NULL) {
        return NULL;
    } else {
        TQueueNode* old_head = *q;
        void* ret_val = old_head->data;
        if (old_head->next == old_head) {
            *q = NULL;
        } else {
            *q = old_head->next;
            TQueueNode* head = *q;
            while(head->next != old_head) {
                head = head->next;
            }
            head->next =*q;
        }
        free(old_head);
        return ret_val;
    }
}

TQueue tqueue_at_offset(TQueue q, unsigned long int offset)
{
    if (offset < 0 || q == NULL) return NULL;
    unsigned long int index;
    TQueueNode* head = q;
    for(index=0; index < offset; index++) {
        head = head->next;
        //if (head == q) return NULL;
    }
    return head;
}

unsigned long int tqueue_size(TQueue q)
{
    unsigned long int size = 1;
    if (q == NULL) return 0;
    TQueueNode* head = q;
    while(head->next != q) {
        head = head->next;
        size += 1;
    }
    return size;
}

void* tqueue_get_data(TQueue q)
{
	return (q == NULL) ? NULL : q->data;
}

