//
//  main.cpp
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include <iostream>
#include <chrono>
#include "parameters.h"
#include "simulation.h"

int main(int argc, const char * argv[]) {
    std::cout << "This is running!" << std::endl;
    parameters params;
    params.read_from_config("config.ini");

    simulation sim(params);
    auto T0 = std::chrono::high_resolution_clock::now();
    for (size_t r = 0; r < params.num_replicates; ++r) {
        
        sim.initialize();
        // here, we choose to run once, or do multiple replicates
        sim.run();
       
        std::cout << "repl: " << r << " of " << params.num_replicates << "\n";
    }
        
    auto T1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> DT = (T1 - T0);
    
    std::cout << "this took: " << DT.count() << " seconds\n";
    
    return 0;
}
