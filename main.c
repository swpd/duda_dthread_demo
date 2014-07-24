#include "webservice.h"
#include "packages/coro/coro.h"

DUDA_REGISTER("Coro Examples", "coro demo");

void hello(void *data)
{
    duda_request_t *dr = data;
    response->http_status(dr, 200);
    response->printf(dr, "coro");
    response->end(dr, NULL);
}

void cb_coro(duda_request_t *dr)
{
    int id = coro->create(hello, dr);
    coro->resume(id);
}

int duda_main()
{
    duda_load_package(coro, "coro");
    map->static_add("/", "cb_coro");
    return 0;
}
