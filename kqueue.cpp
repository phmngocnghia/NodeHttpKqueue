#include <napi.h>
#include <unistd.h>
#include <sys/event.h>

class Kqueue : public Napi::ObjectWrap<Kqueue> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Kqueue(const Napi::CallbackInfo& info);

private:
    static Napi::FunctionReference constructor;

    Napi::Value AddReadFD(const Napi::CallbackInfo& info);
    Napi::Value Wait(const Napi::CallbackInfo& info);

    int kq;  // The kqueue FD
};

// Register JS class
Napi::FunctionReference Kqueue::constructor;

Napi::Object Kqueue::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Kqueue", {
        InstanceMethod("addReadFD", &Kqueue::AddReadFD),
        InstanceMethod("wait", &Kqueue::Wait),
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Kqueue", func);
    return exports;
}

// Constructor
Kqueue::Kqueue(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Kqueue>(info) {
    kq = kqueue();
    if (kq == -1) {
        Napi::Error::New(info.Env(), "Failed to create kqueue").ThrowAsJavaScriptException();
    }
}

// Add a file descriptor to be watched for readability
Napi::Value Kqueue::AddReadFD(const Napi::CallbackInfo& info) {
    int fd = info[0].As<Napi::Number>().Int32Value();

    struct kevent ev;
    EV_SET(&ev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    int res = kevent(kq, &ev, 1, NULL, 0, NULL);
    if (res == -1) {
        Napi::Error::New(info.Env(), "Failed to register FD").ThrowAsJavaScriptException();
    }

    return info.Env().Undefined();
}

// Block until any event is ready
Napi::Value Kqueue::Wait(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    struct kevent ev;
    int n = kevent(kq, NULL, 0, &ev, 1, NULL);
    if (n == -1) {
        Napi::Error::New(env, "kevent() failed").ThrowAsJavaScriptException();
    }

    Napi::Object result = Napi::Object::New(env);
    result.Set("ident", Napi::Number::New(env, ev.ident));
    result.Set("filter", Napi::Number::New(env, ev.filter));
    return result;
}

// Module entry point
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    return Kqueue::Init(env, exports);
}

NODE_API_MODULE(kqueue, InitAll)
