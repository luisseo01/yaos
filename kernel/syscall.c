#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/page.h>
#include <error.h>

int sys_open(char *filename, int mode)
{
	struct superblock *sb;
	struct inode *inode;
	struct file file;

	if ((sb = search_super(filename)) == NULL)
		return -ERR_PATH;

	if ((inode = kmalloc(sizeof(struct inode))) == NULL)
		return -ERR_ALLOC;

	inode->addr = sb->root_inode;
	inode->sb = sb;
	sb->op->read_inode(inode);

	if (inode->iop->lookup(inode, filename + sb->pathname_len)) {
		if (!(mode & O_CREATE))
			return -ERR_PATH;

		if (!inode->iop->create || inode->iop->create(inode,
					filename + sb->pathname_len, FT_FILE))
			return -ERR_CREATE;
	}

	file.flags = mode;
	inode->fop->open(inode, &file);

	if (GET_FILE_TYPE(inode->mode) == FT_DEV) {
		file.op = getdev(inode->dev)->op;
		if (file.op->open)
			file.op->open(inode, &file);
	}

	return mkfile(&file);
}

int sys_read(int fd, void *buf, size_t size)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->read)
		return -ERR_UNDEF;
	if (!(file->flags & O_RDONLY))
		return -ERR_PERM;

	struct task *parent;
	size_t len;
	int tid;

	parent = current;

	tid = clone(TASK_SYSCALL | STACK_SHARED, &init);

	if (tid > 0) { /* child turning to kernel task,
			  nomore in handler mode */
		len = file->op->read(file, buf, size);
		__set_retval(parent, len);

		set_task_state(parent, TASK_RUNNING);
		runqueue_add(parent);

		sys_kill((unsigned int)current);

		freeze(); /* never reaches here */
	} else if (tid == 0) { /* parent */
		sys_yield(); /* it goes sleep exiting from system call
				to wait for its child's job done
				that returns the result. */
	} else { /* error */
		len = 0;
		/* use errno */
	}

	return len;
}

int sys_write(int fd, void *buf, size_t size)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->write)
		return -ERR_UNDEF;
	if (!(file->flags & O_WRONLY))
		return -ERR_PERM;

	return file->op->write(file, buf, size);
}

int sys_close(int fd)
{
	struct file *file = getfile(fd);

	if (!file)
		return -ERR_UNDEF;

	if (file->op->close)
		file->op->close(file);

	remove_file(file);

	return 0;
}

int sys_seek(int fd, unsigned int offset, int whence)
{
	struct file *file = getfile(fd);

	if (!file || !file->op->seek)
		return -ERR_UNDEF;

	return file->op->seek(file, offset, whence);
}

int sys_reserved()
{
	return -ERR_UNDEF;
}

int sys_test(int a, int b, int c)
{
	return a + b + c;
}

#include <kernel/sched.h>

void *syscall_table[] = {
	sys_reserved,		/* 0 */
	sys_schedule,
	sys_test,
	sys_open,
	sys_read,
	sys_write,		/* 5 */
	sys_close,
	sys_brk,
	sys_yield,
	sys_mknod,
	sys_seek,		/* 10 */
	sys_kill,
	sys_fork,
	//sys_create,
	//sys_mkdir,
};
