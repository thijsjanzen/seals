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

TEST_CASE("test initialization")
{
    parameters params;
    simulation sim(params);
    
    sim.initialize();
    
    CHECK(sim.mothers.size() == params.init_population_size);
    CHECK(sim.pups.size() == params.init_population_size);
    
    size_t id_counter = 0;
    auto test_indiv = individual(params.init_energy,
                                 life_stage::mother,
                                 ++id_counter);
    CHECK(test_indiv.current_location == location::colony);
    CHECK(test_indiv.milk == 0.0);
    CHECK(test_indiv.age == 0);
    CHECK(test_indiv.ID == id_counter);
    CHECK(test_indiv.time_since_pup_death == -1);
}

TEST_CASE("test birth")
{
    parameters params;
    simulation sim(params);
    
    sim.initialize();
    size_t id_counter = 0;
    auto test_indiv = individual(params.init_energy,
                                 life_stage::mother,
                                 ++id_counter);
    
    auto offspring = test_indiv.reproduce(++id_counter, params.init_offspring_energy);
    
    CHECK(offspring.age == 0);
    CHECK(offspring.mother_ID == test_indiv.ID);
    CHECK(offspring.ID == id_counter);
}

TEST_CASE("test nursing")
{
    parameters params;
    simulation sim(params);
    
    size_t id_counter = 0;
    auto mother = individual(params.init_energy,
                             life_stage::mother,
                                 ++id_counter);
    
    auto pup = mother.reproduce(++id_counter, params.init_offspring_energy);
    
    mother.produce_milk(params);
    
    double pup_energy_before = pup.energy;
    double mother_milk_before = mother.milk;
    
    sim.nurse(&pup, &mother, params);
    
    double mother_milk_after = mother.milk;
    double pup_energy_after = pup.energy;
    
    CHECK(pup_energy_after > pup_energy_before);
    CHECK(mother_milk_after < mother_milk_before);
    CHECK(mother_milk_before - mother_milk_after == params.nurse_amount * params.milk_consumption);
}

TEST_CASE("test update mothers")
{
    parameters params;
    params.c_survival_mother = 10000;
    simulation sim(params);
    sim.initialize();
    
    // with survival switched off, all mothers survive.
    // also, in the initial time step, mothers have full energy and no incentive to forage
    sim.update_mothers();
    
    CHECK(sim.mothers.size() == params.init_population_size);
    CHECK(sim.available_mothers.size() <= params.init_population_size);
    
    params.c_survival_mother = 0; // we kill all mothers
    simulation sim2(params);
    sim2.initialize();
    sim2.update_mothers();
    CHECK(sim2.mothers.size() == 0);
    CHECK(sim2.available_mothers.size() == 0);
}

TEST_CASE("test update pups")
{
    parameters params;
    params.c_survival_pup = 10000;
    simulation sim(params);
    sim.initialize();
    sim.update_mothers();
    
    sim.update_pups();
    CHECK(sim.pups.size() == params.init_population_size);
    
    params.c_survival_pup = 0;
    simulation sim2(params);
    sim2.initialize();
    sim2.update_mothers();
    
    sim2.update_pups();
    CHECK(sim2.pups.size() == 0);
}

TEST_CASE("test find mother") {
    parameters params;
    params.c_survival_pup = 10000;
    params.c_survival_mother = 10000;
    params.maintenance_cost = 0.0;
    params.milk_production = 0.0;
    
    simulation sim(params);
    sim.initialize();
    sim.update_mothers();
    
    // all mothers and pups should be alive now
    for (const auto& i : sim.pups) {
        auto mother_index = sim.find_mother(i.mother_ID);
        CHECK(mother_index >= 0);
    }
    
    sim.mothers[0].milk = 2.0;
    sim.available_mothers.clear();
    sim.available_mothers.insert({sim.mothers[0].ID, 0});
    auto mother_index = sim.find_mother(0);
    auto allo_mother_index = sim.find_mother(1);
    CHECK(mother_index == allo_mother_index);
    
    // now with multiple mothers
    sim.available_mothers.clear();
    for (size_t i = 0; i < 5; ++i) {
        sim.mothers[i].milk = 2.0;
        sim.available_mothers.insert({sim.mothers[i].ID, i});
    }
    sim.mothers[5].milk = 0.9;
    sim.available_mothers.insert({sim.mothers[5].ID, 5}); // add non receptive mother
    
    std::vector<int> picks(1000);
    for (size_t i = 0; i < 1000; ++i) {
        picks[i] = sim.find_mother(10);
    }
    
    // there are only 5 mothers with > 1 milk, so out of 1000 draws, we only should have those 5:
    
    std::sort(picks.begin(), picks.end());
    picks.erase(unique(picks.begin(),picks.end()),picks.end());
    CHECK(picks.size() == 5);
}

TEST_CASE("start foraging") {
    parameters params;
    simulation sim(params);
    
    size_t id_counter = 0;
    auto mother = individual(params.init_energy,
                             life_stage::mother,
                             ++id_counter);
    
    
    mother.energy = 0.0;
    mother.start_stop_foraging(sim.rndgen, params);
    CHECK(mother.current_location == foraging);
    CHECK(mother.foraging_duration > 0);
    while(mother.current_location == foraging) {
        mother.start_stop_foraging(sim.rndgen, params);
    }
    CHECK(mother.current_location == colony);
    CHECK(mother.foraging_duration < 1);
    CHECK(mother.milk == 1.0);
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
