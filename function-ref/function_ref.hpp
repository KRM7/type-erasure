#ifndef FUNCTION_REF_HPP
#define FUNCTION_REF_HPP

#include <utility>
#include <functional>
#include <type_traits>

template<typename...>
class FunctionRef;

template<typename Ret, typename... Args>
class FunctionRef<Ret(Args...)>
{
public:
    FunctionRef() = default;

    template<typename Callable>
    requires (!std::is_same_v<std::remove_reference_t<Callable>, FunctionRef<Ret, Args...>> &&
              std::is_invocable_r_v<Ret, std::remove_reference_t<Callable>, Args...>)
    FunctionRef(Callable&& f) :
        callable_(&f),
        invoke_([](void* f, Args... args) -> Ret { return std::invoke(*static_cast<std::remove_reference_t<Callable>*>(f), std::forward<Args>(args)...); }) {}

    template<typename Callable>
    requires (!std::is_same_v<std::remove_reference_t<Callable>, FunctionRef<Ret, Args...>> &&
              std::is_invocable_r_v<Ret, std::remove_reference_t<Callable>, Args...>)
    FunctionRef& operator=(Callable&& f)
    {
        callable_ = &f;
        invoke_ = [](void* f, Args... args) -> Ret { return std::invoke(*static_cast<std::remove_reference_t<Callable>*>(f), std::forward<Args>(args)...); };
        return *this;
    }

    Ret operator()(Args... args)
    {
        return invoke_(callable_, std::forward<Args>(args)...);
    }

    operator bool() const noexcept
    {
        return callable_;
    }

private:
    using InvokeFn = Ret(void*, Args...);

    void* callable_   = nullptr;
    InvokeFn* invoke_ = nullptr;
};

#endif // !FUNCTION_REF_HPP