//
//  main.cpp
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include <iostream>
#include "parameters.h"
#include "simulation.h"





int main(int argc, const char * argv[]) {
    
    parameters params;
    params.read_from_config("config.ini");
    
    simulation sim(params);
    
    // here, we choose to run once, or do multiple replicates
    sim.run();
    
    
    
    
    return 0;
}
