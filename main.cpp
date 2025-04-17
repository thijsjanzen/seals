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
    auto T0 = std::chrono::high_resolution_clock::now();
   
    
    std::ofstream outx("a");
    
    simulation sim(params);
    
    sim.initialize();
    
    sim.run(outx, 0);
    
    sim.write_track("test.txt");

    auto T1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> DT = (T1 - T0);    
    std::cout << "this took: " << DT.count() << " seconds\n";
    
    return 0;
}
