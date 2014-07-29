#include "webservice.h"
#include "packages/coro/coro.h"

DUDA_REGISTER("Coro Examples", "coro demo");

void hello(void *data)
{
    duda_request_t *dr = data;
    response->http_status(dr, 200);
    response->printf(dr, "hello coro");
    response->end(dr, NULL);
}

void cb_coro(duda_request_t *dr)
{
    int id = coro->create(hello, dr);
    coro->resume(id);
}

struct bundle {
    channel_t *chan;
    duda_request_t *dr;
};

void consumer(void *data)
{
    struct bundle *bdl = data;
    channel_t *chan = bdl->chan;
    duda_request_t *dr = bdl->dr;
    response->http_status(dr, 200);
    while (!coro->chan_done(chan)) {
        int *n = coro->chan_recv(chan);
        response->printf(dr, "%d\n", *n);
        monkey->mem_free(n);
    }
    response->end(dr, NULL);
}

void productor(void *data)
{
    channel_t *chan = data;
    int i;
    for (i = 0; i < 100; ++i) {
        int *n = monkey->mem_alloc(sizeof(int));
        *n = i + 1;
        coro->chan_send(chan, n);
    }
    coro->chan_end(chan);
}

void cb_chan(duda_request_t *dr)
{
    // unbuffered channel
    channel_t *chan = coro->chan_create(0, monkey->mem_free);
    struct bundle *bdl = monkey->mem_alloc(sizeof(*bdl));
    bdl->chan = chan;
    bdl->dr = dr;
    int cid = coro->create(consumer, bdl);
    int pid = coro->create(productor, chan);
    coro->chan_set_sender(chan, pid);
    coro->chan_set_receiver(chan, cid);
    coro->resume(cid);
    coro->chan_free(chan);
    monkey->mem_free(bdl);
}

int duda_main()
{
    duda_load_package(coro, "coro");
    map->static_add("/hello/", "cb_coro");
    map->static_add("/chan/", "cb_chan");
    return 0;
}
