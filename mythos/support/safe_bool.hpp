#if !defined( MYTHOS_SUPPORT_SAFE_BOOL_HPP )
#define MYTHOS_SUPPORT_SAFE_BOOL_HPP

#define MYTHOS_SAFE_BOOL_OPERATOR(cond)                                         \
    operator mythos::safe_bool::bool_() const                                   \
    {                                                                           \
        return (cond) ? mythos::safe_bool::true_ : mythos::safe_bool::false_;   \
    }

namespace mythos { namespace safe_bool
{
    struct dummy_t { int value; };

    typedef int dummy_t::* bool_;

    bool_ const true_ = &dummy_t::value;
    bool_ const false_ = 0;
}}

#endif // #if !defined( MYTHOS_SUPPORT_SAFE_BOOL_HPP )

