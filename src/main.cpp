#include <map>

#include <unwindstack/AndroidUnwinder.h>

#include "unwinder.h"


using namespace unwindstack;
using namespace std::literals;

[[noreturn]] void some_func() {
    for (;;) {
        sleep(1);
    }
}


void run_demo() {
    int child_pid = fork();

    if (child_pid == 0) {
        some_func();
    } else {
        auto ru = RemoteUnwinder(child_pid);

        ru.Attach();

        AndroidUnwinderData data;

        ru.Unwind(data);
        ru.Detach();

        for (const auto& frame : data.frames) {
            LOGI("%s", ru.FormatFrame(frame).c_str());
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        run_demo();
    } else {
        int pid = (int) std::strtol(argv[1], nullptr, 10);
        auto ru = RemoteUnwinder(pid);

        ru.Attach();

        AndroidUnwinderData data;

        ru.Unwind(data);
        ru.Detach();

        for (const auto& frame : data.frames) {
            LOGI("%s", ru.FormatFrame(frame).c_str());
        }
    }

    return 0;
}
