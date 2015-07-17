#include <foundation.h>
#include <kernel/page.h>
#include <kernel/init.h>

static void cleanup()
{
	/* Clean up redundant code and data used during initialization */
	free_bootmem();
}

/* when no task in runqueue, this one takes place.
 * do some power saving */
static void idle()
{
	cleanup();

	struct task *task;

	while (1) {
		while (zombie) {
			task = (struct task *)zombie;
			zombie = task->addr;
			destroy(task);
		}

		yield();
	}
}

static void __init sys_init()
{
	extern char _init_func_list;
	unsigned int *p = (unsigned int *)&_init_func_list;

	while (*p)
		((void (*)())*p++)();
}

static void __init load_user_task()
{
	extern char _user_task_list;
	struct task *p;
	unsigned int pri;

	for (p = (struct task *)&_user_task_list;
			get_task_flags(p) & TASK_STATIC; p++) {
		if (p->addr == NULL)
			continue;

		/* share one kernel stack with all tasks to save memory */
		if (alloc_mm(p, &init, STACK_SHARED))
			continue;

		pri = get_task_pri(p);
		set_task_dressed(p, TASK_STATIC | STACK_SHARED, p->addr);
		set_task_pri(p, pri);
		set_task_context(p, wrapper);

		/* make it runnable, and add into runqueue */
		set_task_state(p, TASK_RUNNING);
		runqueue_add(p);
	}
}

static int __init make_init_task()
{
	/* stack must be allocated firstly. and to build root relationship
	 * properly `current` must be set to `init`. */
	current = &init;

	if (alloc_mm(&init, NULL, 0))
		return -ERR_ALLOC;

	set_task_dressed(&init, TASK_STATIC | TASK_KERNEL, idle);
	set_task_context_hard(&init, wrapper);
	set_task_pri(&init, LEAST_PRIORITY);
	set_task_state(&init, TASK_RUNNING);

	/* make it the sole */
	list_link_init(&init.children);
	list_link_init(&init.sibling);

	return 0;
}

#include <fs/fs.h>
#include <kernel/device.h>

static int __init console_init()
{
	int fd;

	fd = sys_open(DEVFS_ROOT CONSOLE, O_RDWR | O_NONBLOCK);
	stdin = stdout = stderr = fd;

	return 0;
}

#include <kernel/softirq.h>

int __init main()
{
	/* keep the calling order below because of dependencies. */
	stdin = stdout = stderr = 0;

	sys_init();
	mm_init();
#ifdef CONFIG_FS
	fs_init();
#endif
	device_init();
	systick_init();
	scheduler_init();

	make_init_task();
	load_user_task(); /* that are registered statically */

	softirq_init();
	console_init();

	/* a banner */
	printk("YAOS %s %s\n", VERSION, MACHINE);

	/* switch from boot stack memory to new one */
	set_user_sp(init.mm.sp);
	set_kernel_sp(init.mm.kernel.sp);

	/* everything ready now */
	sei();
	sys_schedule();

	/* it doesn't really reach up to this point. init task becomes idle
	 * magically by scheduler as its context already set to idle */
	freeze();

	return 0;
}
