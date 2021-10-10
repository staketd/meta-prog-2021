#include <iostream>
#include "string_mapper.hpp"

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

int main() {
    using MyMapper = ClassMapper<
            Animal, String<256>,
            Mapping<Cat, "meow"_cstr>,
            Mapping<Dog, "woof"_cstr>,
            Mapping<StBernard, "mooo"_cstr>
    >;

    std::shared_ptr<Animal> some_animal{new Dog()};
    std::string_view dog_sound = *MyMapper::map(*some_animal);
    std::cout << dog_sound << std::endl;
    return 0;
}