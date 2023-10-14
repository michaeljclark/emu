#undef NDEBUG
#include <assert.h>
#include <uv.h>

static void _thread(void *arg)
{
    assert(arg == 0);
}

void t0()
{
    uv_thread_t thread;
    assert(!uv_thread_create(&thread, _thread, 0));
    assert(uv_thread_join(&thread) == 0);
}

int main(int argc, char **argv)
{
    t0();
}
