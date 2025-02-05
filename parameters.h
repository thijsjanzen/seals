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
    int max_pop_size;
    double milk_prod_cutoff;
    double c_survival_mother;
    double c_survival_pup;
    int nurse_amount;
    double base_surv_pup;
    
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
        
        milk_production = from_config.getValueOfKey<double>("milk_production");//make sure this is equal to milk consumption * nurse amount
        milk_consumption = from_config.getValueOfKey<double>("milk_consumption");
        maintenance_cost = from_config.getValueOfKey<double>("maintenance_cost");
        
        max_num_tries = from_config.getValueOfKey<size_t>("max_num_tries");
        max_pop_size = from_config.getValueOfKey<size_t>("max_pop_size");
        milk_prod_cutoff = from_config.getValueOfKey<size_t>("milk_prod_cutoff");
        c_survival_mother = from_config.getValueOfKey<size_t>("c_survival_mother");
        c_survival_pup = from_config.getValueOfKey<size_t>("c_survival_pup");
        nurse_amount = from_config.getValueOfKey<size_t>("nurse_amount"); //This parameter is currently useless, but we can use it to make nursing dependent on energy levels later on
        base_surv_pup = from_config.getValueOfKey<size_t >("base_surv_pup"); //@THIJS: For some reason, this keeps rounding to integers, idk why... so now I don't use this variable (just set it manually in the main code). I am not sure what went wrong...
    }
};
