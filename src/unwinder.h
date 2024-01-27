#pragma once

#include "unwindstack/AndroidUnwinder.h"

#include "dylib.hpp"

using namespace unwindstack;


class RemoteUnwinder {
public:
    static RemoteUnwinder *Create(int tid);

private:
    DynamicLibrary libunwindstack;

    int remote_tid;
    AndroidUnwinder *unwinder;

public:
    explicit RemoteUnwinder(int tid);

    void Attach() const;

    bool Unwind(AndroidUnwinderData &data);

    void Detach() const;

    std::string FormatFrame(const FrameData& frame);
};
