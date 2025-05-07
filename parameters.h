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
  
    size_t season_length = 100;
    double arrival_prob = 0.3;
    
    unsigned int seed = 42;
    double init_energy = 1.0;
    double init_offspring_energy = 1.0;
    size_t init_population_size = 200;
    
    size_t num_seasons = 1;
    size_t num_replicates = 100;
    
    double winter_survival_prob = 1.0;
    
    double foraging_duration = 5;
    double foraging_stdev = 0.3;
    
    double milk_production = 0.2;
    
    double maintenance_cost_mother = 0.1;
    double maintenance_cost_pup = 0.05;
    
    size_t max_num_tries = 5;
    
    double milk_consumption = 0.1;
    size_t max_pop_size = 200;
    double milk_prod_cutoff = 10;
    double c_survival_mother = 20.0;
    double c_survival_pup = 10.0;
    size_t nurse_amount = 2;
    double base_surv_pup = 1;
    double base_surv_mother = 1;

    
    void read_from_config(const std::string file_name) {
        ConfigFile from_config(file_name);
        
        init_population_size = from_config.getValueOfKey<size_t>("init_population_size");
        arrival_prob = from_config.getValueOfKey<double>("arrival_prob");
        
        seed = from_config.getValueOfKey<unsigned int>("seed");
        season_length = from_config.getValueOfKey<size_t>("season_length");
        init_energy = from_config.getValueOfKey<double>("init_energy");
        init_offspring_energy = from_config.getValueOfKey<double>("init_offspring_energy");
        
        num_seasons = from_config.getValueOfKey<size_t>("num_seasons");
        num_replicates = from_config.getValueOfKey<size_t>("num_replicates");
        
        winter_survival_prob = from_config.getValueOfKey<double>("winter_survival_prob");
        
        foraging_duration = from_config.getValueOfKey<double>("foraging_duration");
        foraging_stdev = from_config.getValueOfKey<double>("foraging_stdev");
        
        milk_production = from_config.getValueOfKey<double>("milk_production");//make sure this is equal to milk consumption * nurse amount
        milk_consumption = from_config.getValueOfKey<double>("milk_consumption");
        
        maintenance_cost_mother = from_config.getValueOfKey<double>("maintenance_cost_mother");
        maintenance_cost_pup = from_config.getValueOfKey<double>("maintenance_cost_pup");
        
        max_num_tries = from_config.getValueOfKey<size_t>("max_num_tries");
        max_pop_size = from_config.getValueOfKey<size_t>("max_pop_size");
        milk_prod_cutoff = from_config.getValueOfKey<size_t>("milk_prod_cutoff");
        c_survival_mother = from_config.getValueOfKey<size_t>("c_survival_mother");
        c_survival_pup = from_config.getValueOfKey<size_t>("c_survival_pup");
        nurse_amount = from_config.getValueOfKey<size_t>("nurse_amount"); //This parameter is currently useless, but we can use it to make nursing dependent on energy levels later on
        base_surv_pup = from_config.getValueOfKey<double>("base_surv_pup");
        base_surv_mother = from_config.getValueOfKey<double>("base_surv_mother");
    }
};
