#pragma once

#include <napi/env.h>
#include <Babylon/Api.h>

#include <functional>
#include <mutex>

namespace Babylon
{
    class JsRuntime
    {
    public:
        class NativeObject
        {
            friend class JsRuntime;
            static constexpr auto JS_NATIVE_NAME = "_native";

        public:
            static Napi::Object BABYLON_API GetFromJavaScript(Napi::Env env)
            {
                return env.Global().Get(JS_NATIVE_NAME).As<Napi::Object>();
            }
        };

        struct InternalState;
        friend struct InternalState;

        // Any JavaScript errors that occur will bubble up as a Napi::Error C++ exception.
        // JsRuntime expects the provided dispatch function to handle this exception,
        // such as with a try/catch and logging the exception message.
        using DispatchFunctionT = std::function<void BABYLON_API (std::function<void BABYLON_API (Napi::Env)>)>;

        // Note: It is the contract of JsRuntime that its dispatch function must be usable
        // at the moment of construction. JsRuntime cannot be built with dispatch function
        // that captures a reference to a not-yet-completed object that will be completed
        // later -- an instance of an inheriting type, for example. The dispatch function
        // must be safely callable as soon as it is passed to the JsRuntime constructor.
        static JsRuntime& BABYLON_API CreateForJavaScript(Napi::Env, DispatchFunctionT);
        static JsRuntime& BABYLON_API GetFromJavaScript(Napi::Env);
        void Dispatch(std::function<void BABYLON_API (Napi::Env)>);

        struct IDisposingCallback
        {
            virtual void Disposing() = 0;
        };
        // Notifies the JsRuntime that the JavaScript environment will begin shutting down.
        // Calling this function will signal callbacks registered with RegisterDisposing.
        static void NotifyDisposing(JsRuntime&);

        // Registers a callback for when the JavaScript environment will begin shutting down.
        static void RegisterDisposing(JsRuntime&, IDisposingCallback*);

        // Unregisters a callback for when the JavaScript environment will begin shutting down.
        static void UnregisterDisposing(JsRuntime&, IDisposingCallback*);
    protected:
        JsRuntime(const JsRuntime&) = delete;
        JsRuntime& operator=(const JsRuntime&) = delete;

    private:
        JsRuntime(Napi::Env, DispatchFunctionT);

        DispatchFunctionT m_dispatchFunction{};
        std::mutex m_mutex{};
        std::vector<IDisposingCallback*> m_disposingCallbacks{};
    };
}
