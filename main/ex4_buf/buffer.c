#include <xinu.h>

#define BUF_SIZE 10

typedef struct {
    int data[BUF_SIZE];
    int get_index;
    int put_index;
} buffer_t;

buffer_t buf;
sid32 spaces, items;

void buffer_init(void) 
{
	spaces = semcreate(BUF_SIZE);
	items = semcreate(0);
	buf.get_index = 0;
	buf.put_index = 0;
}

void buffer_put(int n)
{
	wait(spaces);

	buf.data[buf.put_index] = n;
	buf.put_index = (buf.put_index + 1) % BUF_SIZE;

	signal(items);
}

int buffer_get()
{
	int r;

	wait(items);

	r = buf.data[buf.get_index];
	buf.get_index = (buf.get_index + 1) % BUF_SIZE;

	signal(spaces);

	return r;
}
