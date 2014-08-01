#include "webservice.h"

DUDA_REGISTER("Dthread Examples", "dthread demo");

void hello(void *data)
{
    duda_request_t *dr = data;
    response->http_status(dr, 200);
    response->printf(dr, "hello dthread");
    response->end(dr, NULL);
}

void cb_dthread(duda_request_t *dr)
{
    int id = dthread->create(hello, dr);
    dthread->resume(id);
}

struct bundle {
    duda_dthread_channel_t *chan;
    duda_request_t *dr;
};

void consumer(void *data)
{
    struct bundle *bdl = data;
    duda_dthread_channel_t *chan = bdl->chan;
    duda_request_t *dr = bdl->dr;
    response->http_status(dr, 200);
    while (!dthread->chan_done(chan)) {
        int *n = dthread->chan_recv(chan);
        response->printf(dr, "%d\n", *n);
        monkey->mem_free(n);
    }
    response->end(dr, NULL);
}

void productor(void *data)
{
    duda_dthread_channel_t *chan = data;
    int i;
    for (i = 0; i < 100; ++i) {
        int *n = monkey->mem_alloc(sizeof(int));
        *n = i + 1;
        dthread->chan_send(chan, n);
    }
    dthread->chan_end(chan);
}

void cb_chan(duda_request_t *dr)
{
    // unbuffered channel
    duda_dthread_channel_t *chan = dthread->chan_create(0, monkey->mem_free);
    struct bundle *bdl = monkey->mem_alloc(sizeof(*bdl));
    bdl->chan = chan;
    bdl->dr = dr;
    int cid = dthread->create(consumer, bdl);
    int pid = dthread->create(productor, chan);
    dthread->chan_set_sender(chan, pid);
    dthread->chan_set_receiver(chan, cid);
    dthread->resume(cid);
    dthread->chan_free(chan);
    monkey->mem_free(bdl);
}

int duda_main()
{
    map->static_add("/hello/", "cb_dthread");
    map->static_add("/chan/", "cb_chan");
    return 0;
}
