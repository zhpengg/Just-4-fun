#ifndef __WAITING_QUEUE__
#define __WAITING_QUEUE__

#include "global.h"

struct waitq_entry {
	//int fd;
	http_conn_t conn;
};
typedef struct waitq_entry waitq_entry_t;

struct waiting_queue {
	int wq_size;
	int wq_head;
	int wq_tail;
	waitq_entry_t wq_entry[0];
};

typedef struct waiting_queue waiting_queue_t;

waiting_queue_t *waitq_init(http_setting_t *st);

void waitq_enqueue(waiting_queue_t *waitq, waitq_entry_t *entry);

waitq_entry_t *waitq_dequeue(waiting_queue_t *waitq, waitq_entry_t *entry);

int waitq_is_full(waiting_queue_t *waitq);

int waitq_is_empty(waiting_queue_t *waitq);

#endif
