#pragma once

#include <optional>
#include <string_view>

using namespace std::literals::string_view_literals;

template<size_t max_length>
class String {
public:
    constexpr String(const char* string, size_t length) : string_(string), length_(std::min(length, max_length)) {
    }

    constexpr operator std::string_view() const {
        return {string_, length_};
    }

private:
    const char* string_;
    std::size_t length_;
};

constexpr String<256> operator ""_cstr(const char* literal, size_t length) {
    return {literal, length};
}

static_assert(std::is_literal_type_v<String<256>>);
static_assert(std::is_same_v<String<256>, decltype("smth"_cstr)>);
static_assert("some text"_cstr == "some text"sv);
static_assert(String<128>{"some text", 4} == "some"sv);

template<class From, auto target>
struct Mapping {
    using from = From;

    constexpr auto get() {
        return target;
    }
};

template<class M>
concept IsMapping = requires(M m) {
    ([]<class From, auto target> (Mapping<From, target> m) {})(m);
};

template<class M, class Base>
concept IsMappingAndDerivedOf = IsMapping<M> && std::is_base_of<Base, typename M::from>::value;

template<class Base, class Target, IsMappingAndDerivedOf<Base>... Mappings>
struct ClassMapper {
    static std::optional<Target> map(const Base &object) {
    }
};

class Animal {
public:
    virtual ~Animal() = default;
};

class Cat : public Animal {};
class Cow : public Animal {};
class Dog : public Animal {};
class StBernard : public Dog {};
class Horse : public Animal {};
class RaceHorse : public Horse {};

