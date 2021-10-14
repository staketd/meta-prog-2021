#include <iostream>
#include "type_lists.hpp"

template<typename T>
using Starred = T*;

template<class T>
struct Fits { static constexpr bool Value = sizeof(T) <= 4; };

template<class L, class R>
using MaxBySize = std::conditional_t<(sizeof(L) >= sizeof(R)), L, R>;

int main() {
    using TypeTuples::TTuple;
    using TypeLists::Cons;
    using TypeLists::Nil;
    using TypeLists::ToTuple;
    using TypeLists::Repeat;
    static_assert(
            std::same_as<ToTuple<Cons<int, Cons<bool, Cons<float, Nil>>> >, TTuple<int, bool, float>>);
    static_assert(
            std::same_as<TTuple<int, bool, float>, TTuple<int, bool, float>>);

    using TypeLists::FromTuple;
    static_assert(
            std::same_as
                    < ToTuple<FromTuple<TTuple<int, bool, float, char>>>
                            , TTuple<int, bool, float, char>
                    >);
    using TypeLists::Take;
    static_assert(
            std::same_as
                    < ToTuple<Take<5, Repeat<int>>>
            , TTuple<int, int, int, int, int>
              >);

    static_assert(
            std::same_as
                    < ToTuple<Take<2, FromTuple<TTuple<int, bool, float, double>>>>
                            , TTuple<int, bool>
                    >);

    static_assert(
            std::same_as
                    < ToTuple<Take<0, FromTuple<TTuple<int, bool, float, double>>>>
                            , TTuple<>
                    >);

    static_assert(
            std::same_as
                    < ToTuple<Take<10, FromTuple<TTuple<int, bool, float, double>>>>
                            , TTuple<int, bool, float, double>
                    >);

    static_assert(std::same_as<ToTuple<Take<0, Repeat<int>>> , TTuple<>>);

    using TypeLists::Drop;
    static_assert(
            std::same_as
                    < ToTuple<Drop<3, FromTuple<TTuple<float, int, float, int>>>>
            , TTuple<int>
              >);

    static_assert(
            std::same_as
                    < ToTuple<Drop<0, FromTuple<TTuple<float, int, float, int>>>>
                            , TTuple<float, int, float, int>
                    >);

    static_assert(TypeLists::Impl::Length<FromTuple<TTuple<int, int, int>>> == 3);

    static_assert(
            std::same_as
                    < ToTuple<Drop<10, FromTuple<TTuple<float, int, float, int>>>>
                            , TTuple<>
                    >);

    using TypeLists::Replicate;
    static_assert(
            std::same_as
                    < ToTuple<Replicate<3, int>>
                            , TTuple<int, int, int>
                    >);

    using TypeLists::Map;

    using TypeLists::Filter;
    static_assert(std::same_as
                          < ToTuple<Filter<Fits, FromTuple<TTuple<int, bool, uint64_t, char, int64_t, short>>>>
                                  , TTuple<int, bool, char, short>
                          >);

    static_assert(std::same_as
                          < ToTuple<Filter<Fits, Nil>>
                                  , TTuple<>
                          >);

    using TypeLists::Iterate;
    static_assert(std::same_as
                          < ToTuple<Take<4, Iterate<Starred, int>>>
                                  , TTuple<int, int*, int**, int***>
                          >);

    using TypeLists::Get;
    static_assert(
            std::same_as<
                    Get<
                                FromTuple<TTuple<int, float, double>>,
                                3
                            >,
                    Nil
            >
            );


    using TypeLists::Cycle;
    static_assert(std::same_as
                          < ToTuple<Take<5, Cycle<FromTuple<TTuple<int>>>>>
                                  , TTuple<int, int, int, int, int>
                          >);

    using TypeLists::Tails;

    static_assert(std::same_as<ToTuple<Nil>, TTuple<>>);
//    static_assert(std::same_as< ToTuple<Map<ToTuple, Nil>>, TTuple<TTuple<>> >);

    static_assert(
            std::same_as
                    < ToTuple<Map<ToTuple, Tails<FromTuple<TTuple</*int, char, bool,*/ float>>>>>
                            , TTuple
                            </*TTuple<int, char, bool, float>
                                    , TTuple<char, bool, float>
                                    , TTuple<bool, float>
                                    ,*/ TTuple<float>
                                    , TTuple<>
                            >
                    >);

    static_assert(std::same_as
            <
                ToTuple<TypeLists::Impl::Append<double, FromTuple<TTuple<int, double, float>>>>,
                TTuple<int, double, float, double>
            >);

    using TypeLists::Inits;
    static_assert(
            std::same_as
                    < ToTuple<Map<ToTuple, Inits<FromTuple<TTuple<int, char, bool, float>>>>>
                            , TTuple
                            < TTuple<>
                                    , TTuple<int>
                                    , TTuple<int, char>
                                    , TTuple<int, char, bool>
                                    , TTuple<int, char, bool, float>
                            >
                    >);

    using TypeLists::Scanl;
    static_assert(
            std::same_as
                    < ToTuple<Scanl
                            < MaxBySize
                                    , char
                                    , FromTuple<TTuple<char, bool, short, char, int, short, int64_t, int>>
                            >>
                            , TTuple<char, char, char, short, short, int, int, int64_t, int64_t>
                    >);

    using TypeLists::Foldl;
    static_assert(
            std::same_as
                    < Foldl
                            < MaxBySize
                                    , char
                                    , FromTuple<TTuple<char, bool, short, char, int, short, int64_t, int>>
                            >
                            , int64_t
                    >);

    static_assert(
            std::same_as
                    < Foldl
                            < MaxBySize
                                    , char
                                    , Nil
                            >
                            , char
                    >);

    using TypeLists::Zip2;
    static_assert(
            std::same_as
                    < ToTuple<Take<10, Zip2<Repeat<int>, Repeat<float>>>>
                            , ToTuple<Take<10, Repeat<TTuple<int, float>>>>
                    >);

    static_assert(
            std::same_as
                    < ToTuple<Take<10, Zip2<FromTuple<TTuple<bool, char, int>>, Repeat<float>>>>
                            , TTuple
                            < TTuple<bool, float>
                                    , TTuple<char, float>
                                    , TTuple<int, float>
                            >
                    >);

    static_assert(
            std::same_as
                    < ToTuple<Take<10, Zip2<FromTuple<TTuple<bool, char, int>>, Nil>>>
                            , TTuple<>
                    >);
}
