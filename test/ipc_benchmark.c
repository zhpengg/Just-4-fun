#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define BUFSIZE 2000

static void err_exit(const char *msg)
{
	fprintf(stderr, "%s:%s\n", msg, strerror(errno));
	exit(-1);
}

void benchmark_pipe(const char *src, const char *dst)
{
	int pipefd[2], ret;
	char buf[2000];
	pid_t pid;
	int fdin, fdout;

	if (pipe(pipefd) < 0) {
		err_exit("pipe");
	}

	if ((pid = fork()) < 0) {
		err_exit("fork");
	} else if (pid > 0) {
		close(pipefd[0]);

		if ((fdin = open(src, O_RDONLY)) < 0) {
			err_exit("parent open");
		}

		while ((ret = read(fdin, buf, BUFSIZE)) >= 0) {
			if (ret == 0) {
				break;
			}

			if (write(pipefd[1], buf, ret) != ret) {
				err_exit("paretn write");
			}
		}

		if (ret < 0) {
			err_exit("parent read");
		}
		close(pipefd[1]);
		close(fdin);
	} else {
		close(pipefd[1]);

		if ((fdout = open(dst, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
			err_exit("child open");
		}

		while ((ret = read(pipefd[0], buf, BUFSIZE)) >= 0) {
			if (ret == 0) {
				break;
			}

			if (write(fdout, buf, ret) != ret) {
				err_exit("child write");
			}
		}

		if (ret < 0) {
			err_exit("child read");
		}
		close(pipefd[0]);
		close(fdout);
	}
}

struct mymsg {
	long mtype;
	char buf[BUFSIZE];
};


void benchmark_msg(const char *src, const char *dst)
{	
	pid_t pid;
	int msgid;
	struct mymsg msg;
	int fdin, fdout, ret;
	struct msqid_ds msqds;

	key_t mskey = 0x12345675;
	if ((msgid = msgget(mskey, IPC_CREAT | IPC_EXCL| 0666)) < 0) {
		err_exit("msgget");
	}

	if ((pid = fork()) < 0) {
		err_exit("fork");
	} else if (pid > 0) {
		if ((fdin = open(src, O_RDONLY)) < 0) {
			err_exit("parent open");
		}

		while ((ret = read(fdin, msg.buf, BUFSIZE)) >= 0) {
			if (ret == 0) {
				break;
			}
			msg.mtype = 0x12345;
						
			if (msgsnd(msgid, &msg, ret, 0) < 0) {
				err_exit("msgsnd");
			}
		}

		if (ret < 0) {
			err_exit("parent msgsnd");
		}
		close(fdin);
		/*
		* delete msg queue
		*/
		msgctl(msgid, IPC_RMID, &msqds);

	} else {
		if ((msgid = msgget(mskey, 0)) < 0) {
			err_exit("child msgget");
		}

		if ((fdout = open(dst, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
			err_exit("child open");
		}

		while ((ret = msgrcv(msgid, &msg, BUFSIZE, 0, 0)) >= 0) {
			if (write(fdout, msg.buf, ret) != ret) {
				err_exit("child write");
			}
		}

		if (ret < 0) {
			if (errno != EIDRM)
				err_exit("child read");
		}
		close(fdout);
	}
}

int main(int argc, char **argv)
{
	if (argc != 4) {
		fprintf(stderr, "usage: ipc_benchmark [pipe|msg]  source destination\n");
		exit(1);
	}

	if (strcasecmp("pipe", argv[1]) == 0) {
		benchmark_pipe(argv[2], argv[3]);
	} else {
		benchmark_msg(argv[2], argv[3]);
	}

	return 0;
}
