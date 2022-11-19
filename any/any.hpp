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
        template<typename T>
        constexpr void* clone_data(void* data) requires(!std::is_array_v<T>)
        {
            return data ? new T(*static_cast<T*>(data)) : nullptr;
        }

        template<typename T>
        constexpr void deleter(void* data) noexcept requires(!std::is_array_v<T>)
        {
            delete static_cast<T*>(data);
        }

        template<typename T>
        const std::type_info& type(void* data) noexcept
        {
            return data ? typeid(*static_cast<T*>(data)) : typeid(void);
        }
    }

    class Any
    {
    public:
        constexpr Any() noexcept :
            data_(nullptr), vptr_(nullptr)
        {}

        template<typename T, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<T>, Any>, bool> = false> // This isnt the copy or move ctor
        Any(T&& data) :
            data_(new std::remove_cvref_t<T>(std::forward<T>(data))),
            vptr_(std::addressof(detail::any_vtable<std::remove_cvref_t<T>>))
        {}

        constexpr Any(const Any& other) :
            data_(other.data_ ? other.vptr_->clone_(other.data_) : nullptr),
            vptr_(other.vptr_)
        {}

        constexpr Any(Any&& other) noexcept : Any()
        {
            swap(other);
        }

        constexpr Any& operator=(Any rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        constexpr void swap(Any& other) noexcept
        {
            std::swap(data_, other.data_);
            std::swap(vptr_, other.vptr_);
        }

        constexpr ~Any() noexcept
        {
            if (data_) vptr_->delete_(data_);
        }

        constexpr bool has_value() const noexcept
        {
            return data_;
        }

        constexpr const std::type_info& type() const noexcept
        {
            return data_ ? vptr_->type_(data_) : typeid(void);
        }

        constexpr void reset() noexcept
        {
            if (data_) vptr_->delete_(data_);
            data_ = nullptr;
            vptr_ = nullptr;
        }

        template<typename T>
        T get()
        {
            if (data_ && typeid(T).name() == type().name())
            {
                return *static_cast<T*>(data_);
            }
            throw std::logic_error("Bad Any cast");
        }

    private:
        struct Vtable
        {
            using CloneFn  = void*(void*);
            using DeleteFn = void(void*);
            using TypeFn   = const std::type_info&(void*);

            CloneFn*  clone_;
            DeleteFn* delete_;
            TypeFn*   type_;

            Vtable() :
                clone_(nullptr), delete_(nullptr), type_(nullptr)
            {}

            Vtable(CloneFn* clone, DeleteFn* deleter, TypeFn* type) :
                clone_(clone), delete_(deleter), type_(type)
            {}

            void reset() noexcept
            {
                clone_  = nullptr;
                delete_ = nullptr;
                type_   = nullptr;
            }

            void swap(Vtable& other) noexcept
            {
                std::swap(clone_, other.clone_);
                std::swap(delete_, other.delete_);
                std::swap(type_, other.type_);
            }
        };

        void* data_;
        const detail::AnyVtable* vptr_;
    };
}

#endif