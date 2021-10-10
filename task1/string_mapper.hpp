#pragma once

#include <optional>
#include <string_view>
#include <memory>
#include <cassert>

using namespace std::literals::string_view_literals;

template<size_t max_length>
class String {
public:
    constexpr String(const char* string, size_t length) : length_(std::min(length, max_length)) {
        for (size_t i = 0; i < length; ++i) {
            string_[i] = string[i];
        }
    }

    constexpr operator std::string_view() const {
        return {string_, length_};
    }

    std::size_t length_;
    char string_[max_length];
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

    static constexpr auto get() {
        return target;
    }
};

template<class M, class Target>
concept IsMapping = requires(M m) {
    ([]<class From, Target target> (Mapping<From, target> m) {})(m);
};

template<class M, class Base, class Target>
concept IsMappingAndConvertibleTo = IsMapping<M, Target> && std::is_convertible<typename M::from, Base>::value;

namespace inner {

    template<size_t I, size_t N, class Base, class Target, IsMappingAndConvertibleTo<Base, Target>... Mappings>
    struct MapHelper {
        static std::optional<Target> doMap(const Base &object) {
            try {
                dynamic_cast<const typename std::tuple_element_t<I, std::tuple<Mappings...>>::from &>(object);
                return ((typename std::tuple_element_t<I, std::tuple<Mappings...>>) {}).get();
            } catch (std::bad_cast &e) {
                return MapHelper<I + 1, N, Base, Target, Mappings...>::doMap(object);
            }
        }
    };

    template<size_t N, class Base, class Target, IsMappingAndConvertibleTo<Base, Target>... Mappings>
    struct MapHelper<N, N, Base, Target, Mappings...> {
        static std::optional<Target> doMap(const Base &object) {
            return std::optional<Target>{};
        }
    };
}

template<class Base, class Target, IsMappingAndConvertibleTo<Base, Target>... Mappings>
struct ClassMapper {
    template<int I>
    using Helper = inner::MapHelper<I, sizeof...(Mappings), Base, Target, Mappings...>;
    static std::optional<Target> map(const Base &object) {
        return Helper<0>::doMap(object);
    }
};
