#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "waiting_queue.h"

waiting_queue_t *waitq_init(http_setting_t *st)
{
	waiting_queue_t *waitq;

	waitq = malloc(sizeof(waiting_queue_t) + sizeof(waitq_entry_t) * (st->hs_wqlen + 1));

	if (waitq == NULL) {
		perror("waiting_queue_init");
		exit(-1);
	}

	waitq->wq_size = st->hs_wqlen + 1;
	waitq->wq_head = waitq->wq_tail = 0;

	return waitq;
}

void waitq_enqueue(waiting_queue_t *waitq, waitq_entry_t *entry)
{
	memcpy(&(waitq->wq_entry[waitq->wq_tail]), entry, sizeof(waitq_entry_t));
	waitq->wq_tail++;
	if (waitq->wq_tail == waitq->wq_size) {
		waitq->wq_tail = 0;
	}
}

waitq_entry_t *waitq_dequeue(waiting_queue_t *waitq, waitq_entry_t *entry)
{
	memcpy(entry, &(waitq->wq_entry[waitq->wq_head]), sizeof(waitq_entry_t));
	waitq->wq_head++;
	if (waitq->wq_head == waitq->wq_size) {
		waitq->wq_head = 0;
	}

	return entry;
}

int waitq_is_full(waiting_queue_t *waitq)
{
	return (waitq->wq_tail+1) % waitq->wq_size == waitq->wq_head;
}

int waitq_is_empty(waiting_queue_t *waitq)
{
	return waitq->wq_head == waitq->wq_tail;
}	

