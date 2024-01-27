#pragma once

#include <cerrno>
#include <cstdio>
#include <cxxabi.h>
#include <android/log.h>

#ifdef USE_LOGCAT
    #define ALOG(level, ...) __android_log_print(level, "RemoteUnwinder", ##__VA_ARGS__)
    #define LOGI(...) ALOG(ANDROID_LOG_INFO, ##__VA_ARGS__)
    #define LOGE(...) ALOG(ANDROID_LOG_ERROR, ##__VA_ARGS__)
    #ifdef NO_DEBUG
        #define LOGD(...)
    #else
        #define LOGD(...) ALOG(ANDROID_LOG_DEBUG, ##__VA_ARGS__)
    #endif
#else
    #define LOGI(fmt, ...) fprintf(stdout, fmt "\n", ##__VA_ARGS__)
    #define LOGE(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
    #ifdef NO_DEBUG
        #define LOGD(...)
    #else
        #define LOGD(fmt, ...) fprintf(stdout, "\x1b[2m" fmt "\x1b[0m\n", ##__VA_ARGS__)
    #endif
#endif

#define FAIL(...) ({ LOGE(__VA_ARGS__); exit(1); })

#define REQUIRE(expr) ({                                            \
    auto result = expr;                                             \
    if (result < 0) { FAIL(#expr "failed: %s", strerror(errno)); }  \
    result;                                                         \
})


template<typename T>
struct as_ptr {
    using type =
        typename std::conditional<
            std::is_reference<T>::value,
            typename std::add_pointer<typename std::remove_reference<T>::type>::type,
            T
        >::type;
};

static_assert(std::is_same<as_ptr<int>::type, int>::value);
static_assert(std::is_same<as_ptr<int *>::type, int *>::value);
static_assert(std::is_same<as_ptr<int &>::type, int *>::value);


template<typename T>
struct c_style;

template<typename T, typename R, typename... Args>
struct c_style<R(T::*)(Args...)> {
    using type = R(*)(T *, typename as_ptr<Args>::type...);
};

template<typename T, typename R, typename... Args>
struct c_style<R(T::*)(Args...) const> {
    using type = R(*)(T *, typename as_ptr<Args>::type...);
};


#define INSPECT_TYPE(type) do {                                                  \
    auto name = typeid(type).name();                                             \
    int status;                                                                  \
    auto *d_name = __cxxabiv1::__cxa_demangle(name, nullptr, nullptr, &status);  \
    LOGD(#type " -> %s\n", d_name && ~status ? d_name : name);                   \
} while(0)


#ifndef NO_DEBUG

namespace debug_typing {
    struct Type {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
        int f1() { return 0; };
        int f2(int &x, int *y) { return x + *y; }
        int f3(int &x, const int *y) const { return x + *y; }
#pragma clang diagnostic pop
    };

    static_assert(
        std::is_same<
            c_style<decltype(&Type::f1)>::type,
            int (*)(Type *)
        >::value
    );

    static_assert(
        std::is_same<
            c_style<decltype(&Type::f2)>::type,
            int (*)(Type *, int *, int *)
        >::value
    );

    static_assert(
        std::is_same<
            c_style<decltype(&Type::f3)>::type,
            int (*)(Type *, int *, const int *)
        >::value
    );

    static_assert(
        std::is_same<
            c_style<std::string (Type::*)(int &x, int *y)>::type,
            std::string (*)(Type *, int *, int *)
        >::value
    );
}

#endif
