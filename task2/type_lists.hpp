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


        template<size_t N, TypeList TL>
        struct Take {
            using Head = typename TL::Head;
            using Tail = Take<N - 1, typename TL::Tail>;
        };

        template<size_t N, TypeList TL> requires (N > 1) && std::same_as<typename TL::Tail, Nil>
        struct Take<N, TL> {
            using Head = typename TL::Head;
            using Tail = Nil;
        };

        template<TypeList TL>
        struct Take<1, TL> {
            using Head = typename TL::Head;
            using Tail = Nil;
        };

        template<TypeList TL>
        struct Take<0, TL> : Nil {
        };

        template<TypeList TL>
        constexpr size_t Length = 1 + Length<typename TL::Tail>;

        template<>
        constexpr size_t Length<Nil> = 0;

        template<size_t N, TypeList TL>
        struct Drop {
            using Head = typename Drop<N - 1, typename TL::Tail>::Head;
            using Tail = typename Drop<N - 1, typename TL::Tail>::Tail;
        };

        template<size_t N, TypeList TL> requires (Length<TL> <= N)
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
//            using Head = F<Nil>;
//            using Tail = Nil;
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

//    template<class... Ts>
//    struct TTuple<std::tuple<Ts...>> {
//        using Tuple = TypeTuples::TTuple<Ts...>;
////        using TypeList = TypeLists::FromTuple<Tuple>;
//    };

}

//static_assert(std::same_as<>);