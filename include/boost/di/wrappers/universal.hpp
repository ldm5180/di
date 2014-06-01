//
// Copyright (c) 2014 Krzysztof Jusiak (krzysztof at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_DI_DETAIL_WRAPPERS_UNIVERSAL_HPP
#define BOOST_DI_DETAIL_WRAPPERS_UNIVERSAL_HPP

#include "boost/di/aux_/config.hpp"
#include "boost/di/aux_/memory.hpp"
#include "boost/di/named.hpp"

#include <vector>
#include <boost/type.hpp>
#include <boost/non_type.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/aux_/yes_no.hpp>

namespace boost {
namespace di {
namespace wrappers {

namespace detail {

template<typename T, typename TSignature>
class is_convertible
{
    template<typename U> static mpl::aux::yes_tag test(non_type<TSignature, &U::operator()>*);
    template<typename>   static mpl::aux::no_tag test(...);

public:
    typedef is_convertible type;

    BOOST_STATIC_CONSTANT(
        bool
      , value = sizeof(test<T>(0)) == sizeof(mpl::aux::yes_tag)
    );
};

template<typename TValueType, typename T>
struct is_convertible_to_ref
    : mpl::or_<
          is_convertible<TValueType, T&(TValueType::*)(const boost::type<T&>&) const>
        , is_convertible<TValueType, const T&(TValueType::*)(const boost::type<const T&>&) const>
      >
{ };

template<typename T>
struct holder
{
    explicit holder(const T& value)
        : held(value)
    { }

    T held;
};

template<typename TResult, typename T, typename TValueType>
inline const TResult& copy(std::vector<aux::shared_ptr<void> >& refs, const TValueType& value) {
    aux::shared_ptr<holder<TResult> > object(new holder<TResult>(value(boost::type<T>())));
    refs.push_back(object);
    return object->held;
}

template<typename T>
class universal_impl
{
public:
    template<typename TValueType>
    explicit universal_impl(std::vector<aux::shared_ptr<void> >&, const TValueType& value)
    : t(value(boost::type<T>()))
    { }

    operator T() const {
        return t;
    }

private:
    T t;
};

template<typename T>
class universal_impl<const T&>
{
public:
    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >&
                 , const TValueType& value
                 , typename enable_if<is_convertible_to_ref<TValueType, T> >::type* = 0)
        : t(value(boost::type<const T&>()))
    { }

    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >& refs
                 , const TValueType& value
                 , typename disable_if<is_convertible_to_ref<TValueType, T> >::type* = 0)
        : t(copy<T, T, TValueType>(refs, value))
    { }

    operator const T&() const {
        return t;
    }

private:
    const T& t;
};

template<typename T, typename TName>
class universal_impl<named<T, TName> >
{
public:
    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >&, const TValueType& value)
        : t(value(boost::type<T>()))
    { }

    operator T() const {
        return t;
    }

    operator named<T, TName>() const {
        return t;
    }

private:
    T t;
};

template<typename T, typename TName>
class universal_impl<named<const T&, TName> >
{
public:
    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >&
                 , const TValueType& value
                 , typename enable_if<is_convertible_to_ref<TValueType, T> >::type* = 0)
        : t(value(boost::type<const T&>()))
    { }

    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >& refs
                 , const TValueType& value
                 , typename disable_if<is_convertible_to_ref<TValueType, T> >::type* = 0)
        : t(copy<T, T, TValueType>(refs, value))
    { }

    operator named<const T&, TName>() const {
        return t;
    }

private:
    named<const T&, TName> t;
};

template<typename T, typename TName>
class universal_impl<const named<T, TName>&>
{
public:
    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >& refs
                 , const TValueType& value)
        : t(copy<named<T, TName>, T, TValueType>(refs, value))
    { }

    operator const named<T, TName>&() const {
        return t;
    }

private:
    const named<T, TName>& t;
};

template<typename T, typename TName>
class universal_impl<const named<const T&, TName>&>
{
public:
    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >&
                 , const TValueType& value
                 , typename enable_if<is_convertible_to_ref<TValueType, T> >::type* = 0)
        : t(value(boost::type<const named<const T&, TName>&>()))
    { }

    template<typename TValueType>
    universal_impl(std::vector<aux::shared_ptr<void> >& refs
                 , const TValueType& value
                 , typename disable_if<is_convertible_to_ref<TValueType, T> >::type* = 0)
        : t(copy<named<const T&, TName>, T, TValueType>(refs, value))
    { }

    operator const named<const T&, TName>&() const {
        return t;
    }

private:
    const named<const T&, TName>& t;
};

} // namespace detail

template<typename T>
class universal : public detail::universal_impl<T>
{
public:
    typedef universal type;
    typedef T element_type;

    template<typename TValueType>
    universal(std::vector<aux::shared_ptr<void> >& refs, const TValueType& value)
        : detail::universal_impl<T>(refs, value)
    { }
};

} // namespace wrappers
} // namespace di
} // namespace boost

#endif

