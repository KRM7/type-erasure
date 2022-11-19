#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <utility>
#include <functional>
#include <memory>
#include <type_traits>

namespace detail
{
    template<typename Ret, typename... Args>
    struct FunctionImplBase
    {
        virtual Ret invoke(Args...) = 0;
        virtual std::unique_ptr<FunctionImplBase> clone() const = 0;
        virtual ~FunctionImplBase() = default;
    };

    template<typename Callable, typename Ret, typename... Args>
    struct FunctionImpl : public FunctionImplBase<Ret, Args...>
    {
        constexpr explicit FunctionImpl(Callable func) noexcept(std::is_nothrow_move_constructible_v<Callable>) :
            func_(std::move(func))
        {}

        Ret invoke(Args... args) override
        {
            return std::invoke(func_, std::forward<Args>(args)...);
        }

        std::unique_ptr<FunctionImplBase<Ret, Args...>> clone() const override
        {
            return std::make_unique<FunctionImpl<Callable, Ret, Args...>>(func_);
        }

        Callable func_;
    };
}

template<typename...>
class Function;

template<typename R, typename... Args>
class Function<R(Args...)>
{
public:
    constexpr Function() noexcept :
        fptr_(nullptr)
    {}

    template<typename F>
    requires(!std::is_same_v<std::remove_cvref_t<F>, Function> &&   // This is not the copy/move ctor
              std::is_invocable_r_v<R, std::remove_reference_t<F>, Args...>)
    Function(F&& f) :
        fptr_(std::make_unique<detail::FunctionImpl<F, R, Args...>>(std::forward<F>(f)))
    {}

    template<typename F>
    requires(!std::is_same_v<std::remove_cvref_t<F>, Function> &&   // This is not the copy/move assign op
              std::is_invocable_r_v<R, std::remove_reference_t<F>, Args...>)
    Function& operator=(F&& f)
    {
        fptr_ = std::make_unique<detail::FunctionImpl<F, R, Args...>>(std::forward<F>(f));
        return *this;
    }
    
    Function(const Function& other) : fptr_(other.fptr_->clone()) {}

    Function(Function&&) = default;

    Function& operator=(Function rhs) noexcept
    {
        this->swap(rhs);
        return *this;
    }

    R operator()(Args... args)
    {
        return fptr_->invoke(std::forward<Args>(args)...);
    }

    operator bool() const noexcept
    {
        return bool(fptr_);
    }

    void swap(Function& other) noexcept
    {
        fptr_.swap(other.fptr_);
    }

private:
    std::unique_ptr<detail::FunctionImplBase<R, Args...>> fptr_;
};

#endif // !FUNCTION_HPP