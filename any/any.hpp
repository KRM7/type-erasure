#ifndef ANY_HPP
#define ANY_HPP

#include <utility>
#include <type_traits>
#include <typeinfo>
#include <stdexcept>

namespace any
{
    namespace detail
    {
        template<typename T>
        void* clone_data(void* data) requires(!std::is_array_v<T>)
        {
            if (!data) return nullptr;

            return new T(*static_cast<T*>(data));
        }

        template<typename T>
        void deleter(void* data) requires(!std::is_array_v<T>)
        {
            delete static_cast<T*>(data);
        }

        template<typename T>
        const std::type_info& type(void* data)
        {
            return typeid(*static_cast<T*>(data));
        }
    }

    class Any
    {
    public:
        Any() noexcept :
            data_(nullptr), vptr_(nullptr)
        {}

        template<typename T, std::enable_if_t<!std::is_same_v<std::remove_reference_t<T>, Any>, bool> = false,
                             std::enable_if_t<!std::is_array_v<std::remove_reference_t<T>>, bool> = false>
        Any(T&& data) :
            data_(new std::remove_reference_t<T>(std::forward<T>(data))),
            vptr_(new Vtable(detail::clone_data<std::remove_reference_t<T>>,
                             detail::deleter<std::remove_reference_t<T>>,
                             detail::type<std::remove_reference_t<T>>))
        {}

        Any(const Any& other) :
            data_(other.clone_data()),
            vptr_(new Vtable(*other.vptr_))
        {}

        Any& operator=(const Any& rhs)
        {
            if (this == &rhs) return *this;

            Any temp(rhs);
            swap(temp);
            return *this;
        }

        Any(Any&& other) noexcept : Any() { swap(other); }

        Any& operator=(Any&& rhs) noexcept
        {
            if (this == &rhs) return *this;
            swap(rhs);
            return *this;
        }

        ~Any() { reset(); }

        bool has_value() const noexcept { return data_; }

        const std::type_info& type() const noexcept
        {
            return has_value() ? vptr_->type_(data_) : typeid(void);
        }

        void reset() noexcept
        {
            if (has_value())
            {
                vptr_->delete_(data_);
                delete vptr_;
                data_ = vptr_ = nullptr;
            }
        }

        void swap(Any& other) noexcept
        {
            std::swap(data_, other.data_);
            std::swap(vptr_, other.vptr_);
        }

        template<typename T>
        T get()
        {
            if (typeid(T).name() == type().name())
            {
                return *static_cast<T*>(data_);
            }
            throw std::logic_error("Bad cast");
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
        Vtable* vptr_;

        void* clone_data() const { return has_value() ? vptr_->clone_(data_) : nullptr; }
    };
}

#endif