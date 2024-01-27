#pragma once

#include <map>
#include <string>
#include <dlfcn.h>

#include "absl/strings/str_cat.h"

#include "helpers.h"


class DynamicLibrary {
private:
    void *handle = nullptr;
    std::map<std::string, void *> symbol_cache;

public:
    explicit DynamicLibrary(std::string_view libname) {
        auto filename = absl::StrCat("lib", libname, ".so");

        handle = dlopen(filename.c_str(), RTLD_LAZY);
        LOGD("dlopen: %s", filename.c_str());

        if (handle == nullptr) {
            FAIL("dlopen failed: %s", dlerror());
        }
    }

    template<typename F, typename... Args>
    std::invoke_result<F, Args...>::type call(const std::string &symbol, Args... args) {
        auto ptr = symbol_cache.find(symbol);
        void *func;

        if (ptr == symbol_cache.end()) {
            func = dlsym(handle, symbol.c_str());
            LOGD("dlsym: %s", symbol.c_str());

            if (func == nullptr) {
                FAIL("dlsym failed: %s", dlerror());
            }

            symbol_cache[symbol] = func;
        } else {
            func = ptr->second;
        }

        return ((F) func)(args...);
    }
};
