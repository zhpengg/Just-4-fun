#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "worker_queue.h"

worker_queue_t *worker_queue_init(http_setting_t *st)
{
	worker_queue_t *wq;
	wq = malloc(sizeof(worker_queue_t) + sizeof(wq_entry_t) * (st->hs_wknum+1));

	if (wq == NULL) {
		perror("malloc");
		exit(-2);
	}

	wq->wq_size = st->hs_wknum + 1;
	wq->wq_head = 0;
	wq->wq_tail = 0;
	return wq;
}

/* 入队之前保证队列不满 */
void wq_enqueue(worker_queue_t *wq, wq_entry_t *entry)
{
	memcpy(&(wq->wq_list[wq->wq_tail]), entry, sizeof(wq_entry_t));
	wq->wq_tail++;
	if (wq->wq_tail == wq->wq_size) {
		wq->wq_tail = 0;
	}
}

wq_entry_t *wq_dequeue(worker_queue_t *wq, wq_entry_t *entry)
{
	memcpy(entry, &(wq->wq_list[wq->wq_head]), sizeof(wq_entry_t));
	wq->wq_head++;
	if (wq->wq_head == wq->wq_size) {
		wq->wq_head = 0;
	}

	return entry;
}

int wq_is_full(worker_queue_t *wq)
{
	//printf("Head %d, Tail %d\n", wq->wq_head, wq->wq_tail);	
	return wq->wq_head == (wq->wq_tail + 1) % wq->wq_size;
}
int wq_is_empty(worker_queue_t *wq)
{
	//printf("Head %d, Tail %d\n", wq->wq_head, wq->wq_tail);	
	return wq->wq_head == wq->wq_tail;
}

/* 测试函数 */
int main_worker_queue () 
{
	worker_queue_t *wq;
	http_setting_t st;
	st.hs_wknum = 5;

	wq = worker_queue_init(&st);

	int i = 0;
	wq_entry_t entry, *pe;
	while (!wq_is_full(wq)) {
		entry.pipefd[0] = i;
		entry.pipefd[1] = i * 5;
		i++;
		wq_enqueue(wq, &entry);
		printf("Enqueue: %d %d\n", entry.pipefd[0], entry.pipefd[1]);
	}

	while (!wq_is_empty(wq)) {
		pe = wq_dequeue(wq, &entry);
		printf("Dequeue: %d %d\n", entry.pipefd[0], entry.pipefd[1]);
	}

	return 0;
}
