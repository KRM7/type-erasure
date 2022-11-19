#ifndef FUNCTION_REF_HPP
#define FUNCTION_REF_HPP

#include <utility>
#include <functional>
#include <type_traits>
#include <memory>

template<typename...>
class FunctionRef;

template<typename Ret, typename... Args>
class FunctionRef<Ret(Args...)>
{
public:
    constexpr FunctionRef() noexcept :
        callable_(nullptr), invoke_(nullptr)
    {}

    template<typename Callable>
    requires(!std::is_same_v<std::remove_cvref_t<Callable>, FunctionRef> && // This isnt the copy/move ctor
              std::is_invocable_r_v<Ret, std::remove_reference_t<Callable>, Args...>)
    constexpr FunctionRef(Callable&& f) noexcept :
        callable_(std::addressof(f)),
        invoke_(invoke_fn<std::remove_reference_t<Callable>>)
    {}

    template<typename Callable>
    requires(!std::is_same_v<std::remove_cvref_t<Callable>, FunctionRef> && // This isnt the copy/move assignment op
              std::is_invocable_r_v<Ret, std::remove_reference_t<Callable>, Args...>)
    constexpr FunctionRef& operator=(Callable&& f) noexcept
    {
        callable_ = std::addressof(f);
        invoke_ = invoke_fn<std::remove_reference_t<Callable>>;
        return *this;
    }

    Ret operator()(Args... args)
    {
        return invoke_(callable_, std::forward<Args>(args)...);
    }

    /* implicit */ operator bool() const noexcept
    {
        return callable_;
    }

private:
    using InvokeFn = Ret(void*, Args...);

    void* callable_;
    InvokeFn* invoke_;

    template<typename Callable>
    static Ret invoke_fn(void* f, Args... args)
    {
        return std::invoke(*static_cast<Callable*>(f), std::forward<Args>(args)...);
    }
};

#endif // !FUNCTION_REF_HPP