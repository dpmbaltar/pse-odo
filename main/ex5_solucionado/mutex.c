#include <xinu.h>

#define N_MUTEX 2
sid32 mutex[N_MUTEX];
int mutex_pid[N_MUTEX];

void mutex_init(void) 
{
	int i;

	for (i=0; i<N_MUTEX; i++) {
		mutex[i] = semcreate(1);
		mutex_pid[i] = -1;
	}

}

void mutex_lock(int n)
{
	wait(mutex[n]);
	mutex_pid[n] = getpid();
}

void mutex_unlock(int n)
{
	if (getpid() == mutex_pid[n]) {
		mutex_pid[n] = -1;
		signal(mutex[n]);
	}
}


