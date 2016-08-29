/* Papastefanos Serafeim */
/* Ulopoihsh sunarthsewn ouras */

#include <cwmp/cwmp.h>
#include <cwmp/log.h>
#include "cwmp/queue.h"


void queue_add(queue_t *q, void * data, int type, int priority, void * arg1, void *arg2) {
	qnode_t *node;

	cwmp_log_trace("%s(q=%p, data=%p, type=%d, priority=%d, arg1=%p, arg2=%p)",
			__func__, (void*)q, (void*)data,
			type, priority,
			(void*)arg1, (void*)arg2);

	node = (qnode_t *)MALLOC(sizeof(qnode_t));

	if(node == NULL) {
		cwmp_log_error("malloc null");
		return ;
	}

	node->data = data;
	node->arg1 = arg1;
	node->arg2 = arg2;
	node->datatype = type;
	node->priority = priority;
	node->next = NULL;

	pthread_mutex_lock(&q->mutex);

	q->size += 1;

	if(q->first==NULL) {
		q->first = node;
		q->last = node;
	} else {
		qnode_t * first = q->first;
		if(priority >= first->priority)
		{
			node->next = first->next;
			q->first = node;
		}
		else
		{
			q->last->next = node;
			q->last = node;
		}
	}

	pthread_mutex_unlock(& q->mutex);

}

void queue_push(queue_t *q, void * data, int type) {
	qnode_t *node;

	cwmp_log_trace("%s(q=%p, data=%p, type=%d)",
			__func__, (void*)q, (void*)data, type);

	node = (qnode_t *)MALLOC(sizeof(qnode_t));

	if(node == NULL) {
		cwmp_log_error("malloc null");
		return ;
	}

	node->data = data;
	node->arg1 = NULL;
	node->arg2 = NULL;
	node->datatype = type;
	node->priority = QUEUE_PRIORITY_COMMON;
	node->next = NULL;

	pthread_mutex_lock(&q->mutex);

	q->size += 1;

	if(q->first==NULL) {
		q->first = node;
		q->last = node;
	} else {
		q->last->next = node;
		q->last = node;

	}

	pthread_mutex_unlock(& q->mutex);

}


void queue_view(queue_t *q) {
	qnode_t *p;
	cwmp_log_trace("%s(q=%p)", __func__, (void*)q);

	p=q->first;
	if(p==NULL) {
		cwmp_log_debug("queue is empty.");
		return;
	} else {
		cwmp_log_debug("queue size = %d. ", q->size);
		while(p->next!=NULL) {
			cwmp_log_debug(" %s ",p->data);
			p=p->next;
		}
		cwmp_log_debug(" %s ",p->data);
	}
}


int queue_pop(queue_t *q, void ** data, void **arg1, void **arg2) {
	cwmp_log_trace("%s(q=%p, data=%p, arg1=%p, arg2=%p)",
            __func__, (void*)q, (void*)data, (void*)arg1, (void*)arg2);
	if(q->first == NULL) {
		cwmp_log_debug("queue is empty.");
		return -1;
	}
	qnode_t *p;
	int type ;
	pthread_mutex_lock(& q->mutex);

	void *c=q->first->data;
	void *a1=q->first->arg1;
	void *a2=q->first->arg2;
	p=q->first;
	type = p->datatype;
	q->first=q->first->next;
	if(q->first == NULL) q->last = NULL;
	free(p);
	q->size--;
	pthread_mutex_unlock(& q->mutex);

	*data = c;
	if (arg1)
		*arg1 = a1;
	if (arg2)
		*arg2 = a2;
	return type;
}


queue_t *queue_create(pool_t * pool) {
	queue_t *queue = NULL;

	cwmp_log_trace("%s(pool=%p)", __func__, (void*)pool);
	queue = MALLOC(sizeof(queue_t));//(queue_t *)pool_pcalloc(pool, sizeof(queue_t) );
	if(queue == NULL) return NULL;
	queue->first = NULL;
	queue->last = NULL;
	queue->size = 0;


	pthread_mutex_init(& queue->mutex ,NULL);

	return queue;
}

/* Elegxei an h oura einai adeia */
int queue_is_empty(queue_t *q) {
	cwmp_log_trace("%s(q=%p)", __func__, (void*)q);
	return (q->first == NULL);
}

void queue_free(pool_t * pool, queue_t *q) {
	cwmp_log_trace("%s(pool=%p, q=%p)",
			__func__, (void*)pool, (void*)q);

	pthread_mutex_lock(& q->mutex);
	qnode_t *p = q->first;
	while(p->next != NULL) {
		qnode_t *r = p;
		p=p->next;
		free(r);
	}
	pthread_mutex_unlock(& q->mutex);
	pool_pfree(pool, q);
}
