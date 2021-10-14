#pragma once

#include <tuple>

namespace TypeTuples {

    template<class... Ts>
    struct TTuple;

    template<class TT>
    concept TypeTuple = requires {
        typename TT::TypeList;
    };

    namespace Impl {
        template<typename... Ts>
        struct ToTupleStd {
            using Tuple = typename TypeTuples::TTuple<Ts...>;
        };

        template<class... Ts>
        struct ToTupleStd<std::tuple<Ts...>> {
            using Tuple = typename TypeTuples::TTuple<Ts...>;
        };
    }
}

namespace TypeLists {
    template<class TL>
    concept TypeSequence = requires {
        typename TL::Head;
        typename TL::Tail;
    };

    struct Nil {
    };


    template<class TL>
    concept Empty = std::derived_from<TL, Nil>;

    template<class TL>
    concept TypeList = Empty<TL> || TypeSequence<TL>;

    template<class head, TypeList tail>
    struct Cons {
        using Head = head;
        using Tail = tail;
    };

    namespace Impl {
        template<TypeList TL>
        struct ToTupleImpl {
            using Tuple = decltype(std::tuple_cat(std::tuple<typename TL::Head>{},
                                                  typename ToTupleImpl<typename TL::Tail>::Tuple{}));
        };

        template<Empty Ts>
        struct ToTupleImpl<Ts> {
            using Tuple = typename std::tuple<>;
        };

        template<TypeList TL>
        constexpr size_t Length = 1 + Length<typename TL::Tail>;

        template<>
        constexpr size_t Length<Nil> = 0;


        template<size_t N, TypeList TL>
        struct Take {
            using Head = typename TL::Head;
            using Tail = Take<N - 1, typename TL::Tail>;
        };

        template<size_t N, Empty E> requires (N > 1)
        struct Take<N, E> : Nil {
        };

        template<TypeList TL>
        struct Take<1, TL> {
            using Head = typename TL::Head;
            using Tail = Nil;
        };

        template<TypeList TL>
        struct Take<0, TL> : Nil {
        };

        template<TypeList TL, size_t N>
        struct IsLonger {
            constexpr static bool Value = IsLonger<typename TL::Tail, N - 1>::Value;
        };

        template<Empty E, size_t N>
        struct IsLonger<E, N> {
            constexpr static bool Value = false;
        };

        template<TypeSequence TS>
        struct IsLonger<TS, 0> {
            constexpr static bool Value = true;
        };

        template<size_t N, TypeList TL>
        struct Drop {
            using Head = typename Drop<N - 1, typename TL::Tail>::Head;
            using Tail = typename Drop<N - 1, typename TL::Tail>::Tail;
        };

        template<size_t N, TypeList TL> requires (!IsLonger<TL, N>::Value)
        struct Drop<N, TL> : Nil {
        };

        template<TypeSequence TS>
        struct Drop<0, TS> {
            using Head = typename TS::Head;
            using Tail = typename TS::Tail;
        };

        template<size_t N, typename T>
        struct Replicate {
            using Head = T;
            using Tail = Replicate<N - 1, T>;
        };

        template<typename T>
        struct Replicate<0, T> : Nil {
        };

        template<template<typename> typename F, TypeList TL>
        struct Map {
            using Head = F<typename TL::Head>;
            using Tail = Map<F, typename TL::Tail>;
        };

        template<template<typename> typename F, Empty T>
        struct Map<F, T> : Nil {
        };

        template<template<typename> typename P, TypeList TL>
        struct Any {
            static constexpr bool Value = P<typename TL::Head>::Value || Any<P, typename TL::Tail>::Value;
        };

        template<template<typename> typename P, Empty T>
        struct Any<P, T> {
            static constexpr bool Value = false;
        };

        template<template<typename> typename P, TypeList TL>
        struct FilterHelper {
            using Suffix = typename FilterHelper<P, typename TL::Tail>::Suffix;
        };

        template<template<typename> typename P, TypeList TL> requires (P<typename TL::Head>::Value)
        struct FilterHelper<P, TL> {
            using Suffix = TL;
        };

        template<template<typename> typename P, TypeList TL> requires (!Any<P, TL>::Value)
        struct FilterHelper<P, TL> {
            using Suffix = Nil;
        };

        template<template<typename> typename P, TypeList TL>
        struct Filter {
            using Helper = FilterHelper<P, TL>;
            using Head = typename Helper::Suffix::Head;
            using Tail = Filter<P, typename Helper::Suffix::Tail>;
        };

        template<template<typename> typename P, TypeList TL> requires (!Any<P, TL>::Value)
        struct Filter<P, TL> : Nil {
        };

        template<TypeList TL, size_t N>
        struct Get {
            using Value = Get<typename TL::Tail, N - 1>;
        };

        template<TypeSequence TS>
        struct Get<TS, 0> {
            using Value = typename TS::Head;
        };

        template<TypeList TL, size_t N> requires(Length<TL> <= N)
        struct Get<TL, N> {
            using Value = Nil;
        };

        template<TypeList TL, TypeList Origin>
        struct CycleHelper {
            using Head = typename TL::Head;
            using Tail = CycleHelper<typename TL::Tail, Origin>;
        };

        template<TypeList Origin>
        struct CycleHelper<Nil, Origin> {
            using Head = typename Origin::Head;
            using Tail = CycleHelper<typename Origin::Tail, Origin>;
        };


        template<TypeList TL>
        struct Tails {
            using Head = TL;
            using Tail = Tails<typename TL::Tail>;
        };

        template<typename T, TypeList TL>
        struct Append {
            using Head = typename TL::Head;
            using Tail = Append<T, typename TL::Tail>;
        };

        template<typename T, Empty E>
        struct Append<T, E> {
            using Head = T;
            using Tail = Nil;
        };

        template<>
        struct Tails<Nil> {
            using Head = Nil;
            using Tail = Nil;
        };

        template<TypeList TL, TypeList Prefix>
        struct InitsHelper {
            using Head = Prefix;
            using Tail = InitsHelper<typename TL::Tail, Append<typename TL::Head, Prefix>>;
        };

        template<Empty E, TypeList Prefix>
        struct InitsHelper<E, Prefix> {
            using Head = Prefix;
            using Tail = Nil;
        };

        template<TypeList TL>
        struct Inits {
            using Head = Nil;
            using Tail = InitsHelper<typename TL::Tail, Cons<typename TL::Head, Nil>>;
        };

        template<Empty E>
        struct Inits<E> {
            using Head = Nil;
            using Tail = Nil;
        };

        template<template<typename, typename> typename OP, TypeList TL, typename Prev>
        struct ScanlHelper {
            using Head = OP<Prev, typename TL::Head>;
            using Tail = ScanlHelper<OP, typename TL::Tail, Head>;
        };

        template<template<typename, typename> typename OP, Empty E, typename Prev>
        struct ScanlHelper<OP, E, Prev> : Nil {

        };

        template<template<typename, typename> typename OP, typename T, TypeList TL>
        struct Scanl {
            using Head = T;
            using Tail = ScanlHelper<OP, TL, T>;
        };

        template<template<typename, typename> typename OP, typename T, TypeList TL>
        struct Foldl {
            using OpResult = OP<T, typename TL::Head>;
            using Value = typename Foldl<OP, OpResult, typename TL::Tail>::Value;
        };

        template<template<typename, typename> typename OP, typename T, Empty E>
        struct Foldl<OP, T, E> {
            using Value = T;
        };

        template<typename... TLs>
        struct Zip {
        };

        template<TypeSequence... TLs>
        struct Zip<TLs...> {
            using Head = TypeTuples::TTuple<typename TLs::Head...>;
            using Tail = Zip<typename TLs::Tail...>;
        };

        template<TypeList... TLs> requires (Empty<TLs> || ...)
        struct Zip<TLs...> : Nil {
        };

        template<template<typename, typename> typename EQ, TypeList TL, typename T>
        struct GroupByHelper {
            using Value = Cons<typename TL::Head, typename GroupByHelper<EQ, typename TL::Tail, T>::Value>;
        };

        template<template<typename, typename> typename EQ, TypeList TL, typename T> requires (!EQ<typename TL::Head, T>::Value)
        struct GroupByHelper<EQ, TL, T> {
            using Value = Nil;
        };

        template<template<typename, typename> typename EQ, Empty E, typename T>
        struct GroupByHelper<EQ, E, T> : Nil {
            using Value = Nil;
        };
    }

    template<TypeTuples::TypeTuple TT>
    using FromTuple = typename TT::TypeList;


    template<TypeList TL>
    using ToTuple = typename TypeTuples::Impl::ToTupleStd<typename Impl::ToTupleImpl<TL>::Tuple>::Tuple;


    template<typename T>
    struct Repeat {
        using Head = T;
        using Tail = Repeat<T>;
    };

    template<size_t N, TypeList TL>
    using Take = Impl::Take<N, TL>;

    template<size_t N, TypeList TL>
    using Drop = Impl::Drop<N, TL>;

    template<size_t N, typename T>
    using Replicate = Impl::Replicate<N, T>;

    template<template<typename> typename To, TypeList TL>
    using Map = Impl::Map<To, TL>;

    template<template<typename> typename P, TypeList TL>
    using Filter = Impl::Filter<P, TL>;

    template<template<typename> typename F, typename T>
    struct Iterate {
        using Head = T;
        using Tail = Iterate<F, F<T>>;
    };

    template<TypeList TL, size_t N>
    using Get = typename Impl::Get<TL, N>::Value;


    template<TypeList TL>
    struct Cycle {
        using Head = typename Impl::CycleHelper<TL, TL>::Head;
        using Tail = typename Impl::CycleHelper<TL, TL>::Tail;
    };

    template<TypeList TL>
    using Tails = Impl::Tails<TL>;

    template<TypeList TL>
    using Inits = Impl::Inits<TL>;

    template<template<typename, typename> typename OP, typename T, TypeList TL>
    using Scanl = Impl::Scanl<OP, T, TL>;


    template<template<typename, typename> typename OP, typename T, TypeList TL>
    using Foldl = typename Impl::Foldl<OP, T, TL>::Value;

    template<TypeList L, TypeList R>
    struct Zip2 {
        using Head = TypeTuples::TTuple<typename L::Head, typename R::Head>;
        using Tail = Zip2<typename L::Tail, typename R::Tail>;
    };

    template<TypeList TL, Empty E>
    struct Zip2<TL, E> : Nil {

    };

    template<TypeList TL, Empty E>
    struct Zip2<E, TL> : Nil {

    };

    template<TypeList... TLs>
    using Zip = Impl::Zip<TLs...>;

    template<template<typename, typename> typename EQ, TypeList TL>
    struct GroupBy {
        using Head = typename Impl::GroupByHelper<EQ, TL, typename TL::Head>::Value;
        using Tail = GroupBy<EQ, Drop<Impl::Length<Head>, TL>>;
    };

    template<template<typename, typename> typename EQ, Empty E>
    struct GroupBy<EQ, E> : Nil {};

}


namespace TypeTuples {

    namespace Impl {

        template<class head, class... tail>
        struct FromTupleImpl {
            using Head = head;
            using Next = FromTupleImpl<tail...>;
            using TypeList = TypeLists::Cons<Head, typename Next::TypeList>;
        };

        template<class head>
        struct FromTupleImpl<head> {
            using Head = head;
            using TypeList = TypeLists::Cons<Head, TypeLists::Nil>;
        };

        template<>
        struct FromTupleImpl<TypeLists::Nil> {
            using Head = TypeLists::Nil;
            using TypeList = Head;
        };
    }

    template<class... Ts>
    struct TTuple {
        using Helper = Impl::FromTupleImpl<Ts...>;
        using TypeList = typename Helper::TypeList;
    };

    template<>
    struct TTuple<> {
        using TypeList = TypeLists::Nil;
    };

}

template<auto V>
struct ValueTag {
    static constexpr auto Value = V;
};

template<class T, T... ts>
using VTuple = TypeTuples::TTuple<ValueTag<ts>...>;

template<typename T>
using Increment = ValueTag<T::Value + 1>;

template<int N>
struct Ray {
    using Head = ValueTag<N>;
    using Tail = Ray<N + 1>;
};

using Nats = Ray<0>;

template<typename First, typename Second>
struct FibHelper {
    using Head = First;
    using Tail = FibHelper<Second, ValueTag<First::Value + Second::Value>>;
};

using Fib = FibHelper<ValueTag<0>, ValueTag<1>>;

template<typename N, size_t Counter>
struct IsNotPrime {
    static constexpr bool Value = N::Value % Counter == 0 || IsNotPrime<N, Counter + 1>::Value;
};

template<typename N, size_t Counter> requires (N::Value != 1 && Counter * Counter > N::Value)
struct IsNotPrime<N, Counter> {
    static constexpr bool Value = false;
};

template<typename N, size_t Counter> requires (N::Value == 1)
struct IsNotPrime<N, Counter> {
    static constexpr bool Value = true;
};

template<typename T>
struct IsPrime {
    static constexpr bool Value = !IsNotPrime<T, 2>::Value;
};

using Primes = TypeLists::Filter<IsPrime, TypeLists::Take<800, TypeLists::Drop<1, Nats>>>;
