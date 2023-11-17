#include "Warlock.hpp"


int main()
{
    Warlock bob("Bob", "the magnificent");  //Compiles
    Warlock jim("Jim", "the nauseating");   //Compiles
    Warlock jack(jim);

    return (0);
}