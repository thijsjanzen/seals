//
//  parameters.h
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#pragma once

#include "config_parser.h"
#include <string>


struct parameters {
  
    size_t season_length;
    size_t seed;
    double init_energy;
    double init_offspring_energy;
    size_t init_population_size;
    size_t num_seasons;
    
    double winter_survival_prob;
    
    double foraging_duration;
    double foraging_stdev;
    
    double milk_production;
    
    double maintenance_cost;
    
    size_t max_num_tries;
    
    double milk_consumption;
    
    void read_from_config(const std::string file_name) {
        ConfigFile from_config(file_name);
        
        init_population_size = from_config.getValueOfKey<size_t>("init_population_size");
        seed = from_config.getValueOfKey<size_t>("seed");
        season_length = from_config.getValueOfKey<size_t>("season_length");
        init_energy = from_config.getValueOfKey<double>("init_energy");
        init_offspring_energy = from_config.getValueOfKey<double>("init_offspring_energy");
        
        num_seasons = from_config.getValueOfKey<size_t>("num_seasons");
        
        winter_survival_prob = from_config.getValueOfKey<double>("winter_survival_prob");
        
        foraging_duration = from_config.getValueOfKey<double>("foraging_duration");
        foraging_stdev = from_config.getValueOfKey<double>("foraging_stdev");
        
        milk_production = from_config.getValueOfKey<double>("milk_production");
        milk_consumption = from_config.getValueOfKey<double>("milk_consumption");
        
        maintenance_cost = from_config.getValueOfKey<double>("maintenance_cost");
        
        max_num_tries = from_config.getValueOfKey<size_t>("max_num_tries");
    }
};
