#include <sys/ptrace.h>
#include <sys/wait.h>

#include "unwinder.h"
#include "helpers.h"

#define TARGET_FUNC libunwindstack.call

#define CALL_SF(fn, ...) TARGET_FUNC<decltype(&fn)>(__VA_ARGS__)
#define CALL_ST(ty, ...) TARGET_FUNC<ty>(__VA_ARGS__)
#define CALL_MF(fn, ...) TARGET_FUNC<c_style<decltype(&fn)>::type>(__VA_ARGS__)
#define CALL_MT(ty, ...) TARGET_FUNC<c_style<ty>::type>(__VA_ARGS__)


RemoteUnwinder *RemoteUnwinder::Create(int tid) {
    return new RemoteUnwinder(tid);
}

RemoteUnwinder::RemoteUnwinder(int tid) : libunwindstack(DynamicLibrary("unwindstack")) {
    remote_tid = tid;
    unwinder = CALL_SF(AndroidUnwinder::Create, "_ZN11unwindstack15AndroidUnwinder6CreateEi", tid);

    LOGD("AndroidUnwinder::Create(%d) -> %p", tid, unwinder);
}

void RemoteUnwinder::Attach() const {
    REQUIRE(ptrace(PTRACE_SEIZE, remote_tid, 0, 0));
    REQUIRE(ptrace(PTRACE_INTERRUPT, remote_tid, 0, 0));
    REQUIRE(waitpid(remote_tid, nullptr, 0));

    LOGD("AndroidUnwinder::Attach()");
}

bool RemoteUnwinder::Unwind(AndroidUnwinderData &data) {
    auto result = CALL_MT(
        bool(AndroidUnwinder::*)(std::optional<int>, AndroidUnwinderData &),
        "_ZN11unwindstack15AndroidUnwinder6UnwindENSt3__18optionalIiEERNS_19AndroidUnwinderDataE",
        unwinder, std::optional(remote_tid), &data
    );

    LOGD("RemoteUnwinder::Unwind(%p)", &data);

    return result;
}

void RemoteUnwinder::Detach() const {
    REQUIRE(ptrace(PTRACE_DETACH, remote_tid, 0, 0));
}

std::string RemoteUnwinder::FormatFrame(const FrameData &frame) {
    auto result = CALL_MF(
        AndroidUnwinder::FormatFrame,
        "_ZNK11unwindstack15AndroidUnwinder11FormatFrameERKNS_9FrameDataE",
        unwinder, (const FrameData *) &frame
    );

    LOGD("RemoteUnwinder::FormatFrame(%p)", &frame);

    return result;
}
