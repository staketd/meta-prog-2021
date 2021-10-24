#pragma once

#include <optional>
#include <string_view>
#include <memory>
#include <cassert>

using namespace std::literals::string_view_literals;

template<size_t max_length>
class String {
public:
    constexpr String(const char *string, size_t length) : length_(std::min(length, max_length)) {
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

constexpr String<256> operator ""_cstr(const char *literal, size_t length) {
    return {literal, length};
}

static_assert(std::is_literal_type_v<String<256>>);
static_assert(std::is_same_v<String<256>, decltype("smth"_cstr)>);
static_assert("some text"_cstr == "some text"sv);
static_assert(String<128>{"some text", 4} == "some"sv);

template<class From, auto target>
struct Mapping {
    using from = From;
    static constexpr auto MappingValue = target;
};

template<class M, class Target>
concept IsMapping = requires(M m) {
    ([]<class From, Target target>(Mapping<From, target> m) {})(m);
};

template<class M, class Base, class Target>
concept IsMappingAndDerivedFrom = IsMapping<M, Target> && std::derived_from<typename M::from, Base>
                                  && std::is_convertible_v<decltype(M::MappingValue), Target>;

namespace inner {

    template<class Base, class Target, IsMappingAndDerivedFrom<Base, Target>... Mappings>
    struct MapHelper {
        static std::optional<Target> doMap(const Base &object) {
            return std::optional<Target> {};
        }
    };

    template<class Base, class Target, IsMappingAndDerivedFrom<Base, Target> HeadMapping,
            IsMappingAndDerivedFrom<Base, Target>... Mappings>
    struct MapHelper<Base, Target, HeadMapping, Mappings...> {
        static std::optional<Target> doMap(const Base &object) {
            auto *cast = dynamic_cast<const typename HeadMapping::from *>(&object);
            if (cast != nullptr) {
                return HeadMapping::MappingValue;
            } else if (sizeof...(Mappings) == 0) {
                return std::optional<Target>{};
            } else {
                return MapHelper<Base, Target, Mappings...>::doMap(object);
            }
        }
    };
}

template<class Base, class Target, IsMappingAndDerivedFrom<Base, Target>... Mappings>
struct ClassMapper {
    static std::optional<Target> map(const Base &object) {
        return inner::MapHelper<Base, Target, Mappings...>::doMap(object);
    }
};
