#include <napi.h>
#include <unistd.h>
#include <sys/event.h>
#include <thread>

class Kqueue : public Napi::ObjectWrap<Kqueue> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Kqueue(const Napi::CallbackInfo& info);
    ~Kqueue();

private:
    static Napi::FunctionReference constructor;

    void StartEventLoop();
    void StopEventLoop();

    Napi::Value AddReadFD(const Napi::CallbackInfo& info);
    Napi::Value Close(const Napi::CallbackInfo& info);

    int kq;
    std::thread eventLoopThread;
    bool running = true;
    Napi::ThreadSafeFunction tsfn;
};

Napi::FunctionReference Kqueue::constructor;

Napi::Object Kqueue::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Kqueue", {
        InstanceMethod("addReadFD", &Kqueue::AddReadFD),
        InstanceMethod("close", &Kqueue::Close),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Kqueue", func);
    return exports;
}

Kqueue::Kqueue(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Kqueue>(info) {
    Napi::Env env = info.Env();

    kq = kqueue();
    if (kq == -1) {
        Napi::Error::New(env, "Failed to create kqueue").ThrowAsJavaScriptException();
        return;
    }

    auto jsCallback = info[0].As<Napi::Function>();
    tsfn = Napi::ThreadSafeFunction::New(env, jsCallback, "KqueueCallback", 0, 1);
    StartEventLoop();
}

Kqueue::~Kqueue() {
    StopEventLoop();
}

void Kqueue::StartEventLoop() {
    eventLoopThread = std::thread([this]() {
        struct kevent ev;

        while (running) {
            int n = kevent(kq, NULL, 0, &ev, 1, NULL);
            if (n == -1) {
                // Exit loop on kq close or error
                break;
            }

            int ident = ev.ident;
            tsfn.BlockingCall([ident](Napi::Env env, Napi::Function jsCallback) {
                jsCallback.Call({ Napi::Number::New(env, ident) });
            });
        }
    });
}

void Kqueue::StopEventLoop() {
    running = false;

    if (kq >= 0) {
        close(kq);  // This unblocks kevent
        kq = -1;
    }

    if (eventLoopThread.joinable()) {
        eventLoopThread.join();
    }

    if (tsfn) {
        tsfn.Release();
    }
}

Napi::Value Kqueue::AddReadFD(const Napi::CallbackInfo& info) {
    int fd = info[0].As<Napi::Number>().Int32Value();

    struct kevent ev;
    EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);

    int res = kevent(kq, &ev, 1, NULL, 0, NULL);
    if (res == -1) {
        Napi::Error::New(info.Env(), "Failed to register FD").ThrowAsJavaScriptException();
    }

    return info.Env().Undefined();
}

Napi::Value Kqueue::Close(const Napi::CallbackInfo& info) {
    StopEventLoop();
    return info.Env().Undefined();
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return Kqueue::Init(env, exports);
}

NODE_API_MODULE(kqueue, InitAll)
