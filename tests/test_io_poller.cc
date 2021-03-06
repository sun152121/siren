#include <chrono>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include "helper_macros.h"
#include "io_clock.h"
#include "io_poller.h"
#include "test.h"


namespace {

using namespace siren;


SIREN_TEST("Add/Remove io watchers")
{
    struct IOTimerContext : IOTimer {
    };

    struct IOWatcherContext : IOWatcher {
    };

    int r;
    SIREN_UNUSED(r);
    int fds[2];
    r = pipe2(fds, O_NONBLOCK);
    assert(r == 0);
    IOClock ioClock;
    IOTimerContext ioTimerContext;
    ioClock.addTimer(&ioTimerContext, std::chrono::milliseconds(100));
    IOPoller ioPoller;
    IOWatcherContext ioWatcherContext;
    ioPoller.createObject(fds[0]);
    ioPoller.addWatcher(&ioWatcherContext, fds[0], IOCondition::Readable);
    std::vector<IOWatcher *> readyWatchers;
    ioPoller.getReadyWatchers(&ioClock, &readyWatchers);
    SIREN_TEST_ASSERT(readyWatchers.size() == 0);
    std::vector<IOTimer *> expiredTimers;
    ioClock.getExpiredTimers(&expiredTimers);
    SIREN_TEST_ASSERT(expiredTimers.size() == 1);
    SIREN_TEST_ASSERT(expiredTimers[0] == &ioTimerContext);
    char c;
    r = read(fds[0], &c, 1);
    assert(r == -1);

    std::thread t([fds] {
        usleep(100 * 1000);
        char c = 'a';
        int r = write(fds[1], &c, 1);
        SIREN_UNUSED(r);
        assert(r == 1);
    });

    ioPoller.getReadyWatchers(&ioClock, &readyWatchers);
    SIREN_TEST_ASSERT(readyWatchers.size() == 1);
    SIREN_TEST_ASSERT(readyWatchers[0] == &ioWatcherContext);
    t.join();
}

}
