//
//  main.cpp
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#define CATCH_CONFIG_MAIN

#include <iostream>
#include "../parameters.h"
#include "../simulation.h"

#include "catch.h"

TEST_CASE("first_test")
{
    parameters params;
    simulation sim(params);
    
    sim.initialize();
    
    REQUIRE(sim.mothers.size() == params.init_population_size);
    REQUIRE(sim.pups.size() == params.init_population_size);
}


/*
int main(int argc, const char * argv[]) {
    std::cout << "This is running!" << std::endl;
    parameters params;
    params.read_from_config("config.ini");

    simulation sim(params);
    
    // here, we choose to run once, or do multiple replicates
    sim.run();
    
    return 0;
}
*/
