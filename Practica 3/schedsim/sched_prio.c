#include "sched.h"

void sched_init_prio(void){}

void sched_destroy_prio(void){}

static task_t* pick_next_task_prio(runqueue_t* rq, int cpu){
	task_t* t = head_slist(&rq->tasks);
	if(t){
		remove_slist(&rq->tasks,t);
		t->on_rq=FALSE;
		rq->cur_task=t;
	}
	return t;
}

static int compare_task_prio(void *task1, void *task2){
	task_t* tsk1=(task_t*)task1;
	task_t* tsk2=(task_t*)task2;
	return tsk1->prio-tsk2->prio;
}

static void enqueue_task_prio(task_t* t, int cpu, int runnable){
	runqueue_t* rq =get_runqueue_cpu(cpu);
	if (t->on_rq || is_idle_task(t)) {
		return;
	}
	sorted_insert_slist_front(&rq->tasks, t, 1, compare_task_prio);
	t->on_rq=TRUE;

	if (!runnable){
		rq->nr_runnable++;
		t->last_cpu=cpu;
	}
}

static void task_tick_prio(runqueue_t* rq, int cpu){
	task_t* t = rq->cur_task;
	sort_slist(&rq->tasks, 1, compare_task_prio);
	if (is_idle_task(t)){
		return;
	}
	if (t->runnable_ticks_left == 1){
		rq->nr_runnable--;
	}
	rq->need_resched=TRUE;
}

static task_t* steal_task_prio(runqueue_t* rq, int cpu){
	task_t* t = tail_slist(&rq->tasks);

	if(t){
		remove_slist(&rq->tasks,t);
		t->on_rq=FALSE;
		rq->nr_runnable--;
	}
	return t;
}

sched_class_t prio_sched={
	    .sched_destroy=sched_destroy_prio,
	    .pick_next_task=pick_next_task_prio,
	    .enqueue_task=enqueue_task_prio,
	    .task_tick=task_tick_prio,
	    .steal_task=steal_task_prio,
};
