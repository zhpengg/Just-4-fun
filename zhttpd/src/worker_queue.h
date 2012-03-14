#ifndef __WORKER_QUEUE__
#define __WORKER_QUEUE__

#include "global.h"
#include <unistd.h>

struct worker_queue_entry {
	pid_t pid;
	int pipefd[2];
};
typedef struct worker_queue_entry wq_entry_t;

struct worker_queue {
	int wq_size;
	int wq_head;
	int wq_tail;
	wq_entry_t wq_list[0];
};

typedef struct worker_queue worker_queue_t;

worker_queue_t *worker_queue_init(http_setting_t *st);

void wq_enqueue(worker_queue_t *wq, wq_entry_t *entry);

wq_entry_t *wq_dequeue(worker_queue_t *wq, wq_entry_t *entry);

int wq_is_full(worker_queue_t *wq);
int wq_is_empty(worker_queue_t *wq);
#endif
