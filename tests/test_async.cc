#include "async.h"
#include "loop.h"
#include "test.h"


namespace {

using namespace siren;


SIREN_TEST("Execute async task")
{
    Loop loop;
    Async async(&loop);

    loop.createFiber([&] () {
        int f = false;

        try {
            async.executeTask([&] () {
                throw 239;
            });
        } catch (int s) {
            f = true;
            SIREN_TEST_ASSERT(s == 239);
        }

        SIREN_TEST_ASSERT(f);
        int t = 100;

        async.executeTask([&] () {
            --t;
        });

        SIREN_TEST_ASSERT(t == 99);

    });

    loop.run();
}

}
