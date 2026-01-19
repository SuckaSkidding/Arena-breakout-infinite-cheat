#include <iostream>
#include <print>

#include <overlay.hpp>
#include <game/memory.hpp>
//#include "game/rendering.hpp"

int main() {
    // 1. Initialiser le gestionnaire de mémoire EN PREMIER
   

	overlay::setup();

	std::cin.get();
	return 0;
}