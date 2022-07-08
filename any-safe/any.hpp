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
            virtual const std::type_info& type() const = 0;
            virtual ~AnyImplBase() = default;
        };

        template<typename T>
        struct AnyImpl : public AnyImplBase
        {
            AnyImpl(T data) : data_(std::move(data)) {}

            std::unique_ptr<AnyImplBase> clone() const override
            {
                return std::make_unique<AnyImpl>(data_);
            }

            const std::type_info& type() const override
            {
                return typeid(T);
            }

            T data_;
        };
    }

    class Any
    {
    public:
        Any() = default;

        template<typename T>
        requires (!std::is_same_v<std::remove_reference_t<T>, Any> && !std::is_array_v<std::remove_reference_t<T>>)
        Any(T&& data) : data_(std::make_unique<detail::AnyImpl<std::remove_reference_t<T>>>(std::forward<T>(data)))
        {}

        Any(const Any& other) : data_(other.data_->clone())
        {}

        Any& operator=(const Any& rhs)
        {
            if (this == &rhs) return *this;

            Any temp(rhs);
            swap(temp);
            return *this;
        }

        Any(Any&& other) noexcept : Any()
        {
            swap(other);
        }

        Any& operator=(Any&& rhs) noexcept
        {
            if (this == &rhs) return *this;
            swap(rhs);
            return *this;
        }

        bool has_value() const noexcept
        { 
            return bool(data_);
        }

        const std::type_info& type() const noexcept
        {
            return has_value() ? data_->type() : typeid(void);
        }

        void reset() noexcept
        {
            data_.reset(nullptr);
        }

        void swap(Any& other) noexcept
        {
            data_.swap(other.data_);
        }

        template<typename T>
        T get()
        {
            if (dynamic_cast<detail::AnyImpl<T>*>(data_.get()))
            {
                return static_cast<detail::AnyImpl<T>*>(data_.get())->data_;
            }
            throw std::logic_error("Bad cast");
        }

    private:
        std::unique_ptr<detail::AnyImplBase> data_ = nullptr;
    };
}

#endif