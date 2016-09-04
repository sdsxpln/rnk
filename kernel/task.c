/*
 * Copyright (C) 2014  Raphaël Poggi <poggi.raph@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <scheduler.h>
#include <stdio.h>
#include <mm.h>
#include <utils.h>
#include <arch/svc.h>
#include <armv7m/system.h>
#include <spinlock.h>

static struct task *current_task = NULL;
static int task_count = 0;
struct list_node runnable_tasks;

unsigned long task_lock = SPIN_LOCK_INITIAL_VALUE;

static void idle_task(void)
{
	while(1)
		wait_for_interrupt();
}

static void insert_task(struct task *t)
{
	struct task *task = NULL;

	task = list_peek_head_type(&runnable_tasks, struct task, node);

	if (!task_count) {
		debug_printk("runnable list is empty\r\n");
		list_add_head(&runnable_tasks, &task->node);
		return;
	}

	list_for_every_entry(&runnable_tasks, task, struct task, node) {

#ifdef CONFIG_SCHEDULE_ROUND_ROBIN
		if (!list_next(&runnable_tasks, task->node)) {
			verbose_printk("inserting task %d\r\n", t->pid);
			list_add_after(&task->node, &t->node);
			break;
		}
#elif defined(CONFIG_SCHEDULE_PRIORITY)
		verbose_printk("t->priority: %d, task->priority; %d\r\n", t->priority, task->priority);
		if (t->priority > task->priority) {
			verbose_printk("inserting task %d\r\n", t->pid);
			list_add_before(&task->node, &t->node);
			break;
		}
#endif
	}
}

void task_init(void)
{
	list_initialize(&runnable_tasks);
	add_task(&idle_task, 0);
}

void add_task(void (*func)(void), unsigned int priority)
{
	struct task *task = (struct task *)kmalloc(sizeof(struct task));

	memset(task, 0, sizeof(struct task));

	task->state = TASK_RUNNABLE;
	task->pid = task_count;

#ifdef CONFIG_SCHEDULE_PRIORITY
	task->priority = priority;
#elif defined(CONFIG_SCHEDULE_ROUND_ROBIN)
	task->quantum = CONFIG_TASK_QUANTUM;
#endif

	task->start_stack = TASK_STACK_START + (task_count * TASK_STACK_OFFSET);
	task->delay = 0;
	task->func = func;
	task->regs = (struct registers *)kmalloc(sizeof(struct registers));
	task->regs->sp = task->start_stack;
	task->regs->lr = (unsigned int)end_task;
	task->regs->pc = (unsigned int)func;

	/* Creating task context */
	create_context(task->regs, task);

	if (task_count)
		insert_task(task);
	else
		list_add_head(&runnable_tasks, &task->node);

	task_count++;
}

void first_switch_task(void)
{
	SVC_ARG(SVC_TASK_SWITCH, NULL);
}

void switch_task(struct task *task)
{
	if (current_task && (current_task->state != TASK_BLOCKED)) {
		current_task->regs->sp = PSP();
		insert_task(current_task);
	}

	task->state = TASK_RUNNING;
	SET_PSP((void *)task->regs->sp);
	current_task = task;

	if (task->pid != 0)
		remove_runnable_task(task);
}

struct task *get_current_task(void)
{
	return current_task;
}

struct task *find_next_task(void)
{
	struct task *task = NULL;

#ifdef CONFIG_SCHEDULE_PRIORITY
	task = list_peek_head_type(&runnable_tasks, struct task, node);

	if (current_task && current_task->state != TASK_BLOCKED)
		if (task->priority < current_task->priority)
			task = current_task;
#elif defined(CONFIG_SCHEDULE_ROUND_ROBIN)
	list_for_every_entry(&runnable_tasks, task, struct task, node) {
		if ((task->quantum > 0) && (task->pid != 0))
			break;

	if (current_task)
		current_task->quantum = CONFIG_TASK_QUANTUM;

	/* Only idle task is eligible */
	if (!task) {
		task = list_peek_head_type(&runnable_tasks, struct task, node);
	}
#endif

	verbose_printk("next task: %d\r\n", task->pid);

	return task;
}

void insert_runnable_task(struct task *task)
{
	insert_task(task);
	task->state = TASK_RUNNABLE;
}

void remove_runnable_task(struct task *task)
{
	task->regs->sp = PSP();

#ifdef CONFIG_SCHEDULE_ROUND_ROBIN
	current_task->quantum = CONFIG_TASK_QUANTUM;
#endif

	list_delete(&task->node);
}
