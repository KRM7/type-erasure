#ifndef ANY_HPP
#define ANY_HPP

#include <utility>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <stdexcept>

namespace any
{
    namespace detail
    {
        struct AnyImplBase
        {
            virtual std::unique_ptr<AnyImplBase> clone() const = 0;
            virtual const std::type_info& type() const noexcept = 0;
            virtual ~AnyImplBase() = default;
        };

        template<typename T>
        struct AnyImpl : public AnyImplBase
        {
            constexpr AnyImpl(T data) noexcept(std::is_nothrow_move_constructible_v<T>) :
                data_(std::move(data)) {}

            std::unique_ptr<AnyImplBase> clone() const override
            {
                return std::make_unique<AnyImpl>(data_);
            }

            const std::type_info& type() const noexcept override
            {
                return typeid(T);
            }

            T data_;
        };
    }

    class Any
    {
    public:
        constexpr Any() noexcept :
            data_(nullptr)
        {}

        template<typename T>
        requires (!std::is_same_v<std::remove_cvref_t<T>, Any>) // This isnt the copy/move ctor
        Any(T&& data) :
            data_(std::make_unique<detail::AnyImpl<std::remove_cvref_t<T>>>(std::forward<T>(data)))
        {}

        Any(const Any& other) :
            data_(other.data_->clone())
        {}

        Any(Any&&) = default;

        Any& operator=(Any rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        bool has_value() const noexcept
        { 
            return bool(data_);
        }

        const std::type_info& type() const noexcept
        {
            return data_ ? data_->type() : typeid(void);
        }

        void reset() noexcept
        {
            data_ = nullptr;
        }

        void swap(Any& other) noexcept
        {
            data_.swap(other.data_);
        }

        template<typename T>
        T get()
        {
            if (data_ && dynamic_cast<detail::AnyImpl<T>*>(data_.get()))
            {
                return static_cast<detail::AnyImpl<T>*>(data_.get())->data_;
            }
            throw std::logic_error("Bad Any cast");
        }

    private:
        std::unique_ptr<detail::AnyImplBase> data_;
    };
}

#endif