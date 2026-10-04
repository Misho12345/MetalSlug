#pragma once
#include <utility>

namespace mse
{
    namespace detail
    {
        template <typename>
        struct lambda_ptr { using type = void; };

        template <typename T> requires requires(T t) { +t; }
        struct lambda_ptr<T>
        {
            using type = decltype(+std::declval<T>());
        };

        template <typename T>
        using lambda_ptr_t = lambda_ptr<T>::type;
    }

    /**
     * @brief Lambda/function pointer storing
     * @tparam T lambda (impl defined) or function pointer type
     * @notes
     * - Doesn't work with lambdas marked as mutable
     * - Shouldn't be used after captured references expire (if any)
     */
    // TODO: make proper constraints for T
    template <typename T>
    class lambda
    {
        using ptr_t = detail::lambda_ptr_t<T>;

    public:
        lambda(nullptr_t) requires std::is_pointer_v<T> : f{ nullptr } {}

        lambda(T&& l) : f{ std::move(l) } {}

        template <typename... Args>
        // I sadly can't use decltype(auto)
        decltype(std::declval<T>()(std::declval<Args>()...)) operator()(Args&&... args) const
        {
            return f(std::forward<Args>(args)...);
        }

        operator ptr_t() const requires (!std::same_as<ptr_t, void>) { return +f; }

    private:
        T f;
    };
}
