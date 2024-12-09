//
//  simulation.h
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include "rand_t.h"
#include "parameters.h"
#include "individual.h"

struct simulation {
    
    rnd_t rndgen;
    const parameters params;
    
    simulation(const parameters& p) : params(p) {
        // set specific parameters
        rndgen.set_seed(params.seed);
        
    }
    
    void run() {
        id_counter = 0;
        initialize();
        
        for (size_t season = 0; season < params.num_seasons; ++season) {
            simulate_season(season);
            update_population_end_of_season();
            std::cout << season << " " << mothers.size() << " " << pups.size() << "\n";
            if (mothers.size() + pups.size() < 1) break;
        }
    }
    
    void initialize() {
        mothers.clear();
        pups.clear();
        for (size_t i = 0; i < params.init_population_size; ++i) {
            mothers.push_back( individual(params.init_energy,
                                       life_stage::mother,
                                       ++id_counter)); // this increments the counter after use!
            auto new_pup = mothers.back().reproduce(++id_counter,
                                                 params.init_offspring_energy);
            pups.push_back(new_pup);
        }
    }
    
    void simulate_season(size_t season) {
        for (size_t days = 0; days < params.season_length; ++days) {
            update_mothers();
            update_pups();
            write_to_file(season, days);
            std::cout << season << " " << days << " " << mothers.size() << " " << pups.size() << "\n";
        }
    }
    
    void update_mothers() {
        available_mothers.clear();
        for (size_t i = 0; i < mothers.size(); ) {
            mothers[i].pay_maintenance(params);
            mothers[i].start_stop_foraging(rndgen,
                                  params);
            mothers[i].produce_milk(params);
            
            
            
            if (!mothers[i].survive(rndgen)) {
                mothers[i] = mothers.back();
                mothers.pop_back();
            } else {
                if (mothers[i].current_location == location::colony) {
                    available_mothers.insert({mothers[i].ID, i});
                }
                    
                ++i;
            }
        }
    }
    
    void update_pups() {
        for (size_t i = 0; i < pups.size(); ) {
            pups[i].pay_maintenance(params);
            if (!pups[i].survive(rndgen)) {
                pups[i] = pups.back();
                pups.pop_back();
            } else {
                auto mother_index = find_mother(pups[i].mother_ID);
                if (mother_index >= 0) {
                    nurse(&pups[i], &mothers[mother_index], params);
                } else {
                    // no (allo) mother found to nurse from!
                }
                
                ++i;
            }
        }
    }
    
    int find_mother(size_t pup_id) {
        if (available_mothers.empty()) return -1;
        
        
        
        int index = -1;
        bool mother_found = false;
        
        auto x = available_mothers.find(pup_id);
        if (x != available_mothers.end()) {
            index = x->second;
            mother_found = true;
        }
        
        
        if (index < 0) {
            for (size_t num_tries = 0; num_tries < params.max_num_tries; ++num_tries) {
                index = rndgen.random_number(mothers.size());
                while (mothers[index].current_location != location::colony) {
                    index = rndgen.random_number(mothers.size());
                }
                if (mothers[index].allow_allo_nursing()) {
                    mother_found = true;
                    break;
                }
            }
        }
        if (!mother_found) index = -1;
        
        return index;
    }
    
    
    void update_population_end_of_season() {
        for (auto& i : pups) {
            i.stage = mother;
            i.current_location = colony;
            mothers.push_back(i);
        }
        pups.clear();
        
        for (size_t i = 0; i < mothers.size();) {
            if (!rndgen.bernouilli(params.winter_survival_prob)) {
                mothers[i] = mothers.back();
                mothers.pop_back();
            } else {
                ++i;
            }
        }
        
        // we reproduce again!
        // it is a bit inefficient to do this in two goes (one could do survival and
        // reproduction in one loop, but the loss is minimal compared to the rest of the code
        for (auto& i : mothers) {
            i.milk = 0.0;
            auto new_pup = i.reproduce(++id_counter,
                                       params.init_offspring_energy);
            pups.push_back(new_pup);
        }
    }
    
    void nurse(individual* pup, individual* nurse,
               const parameters& p) {
        nurse->milk -= p.milk_consumption;
        pup->energy += p.milk_consumption;
    }
    
    void write_to_file(size_t season,
                       size_t day) {
        std::ofstream out("results.txt", std::ios::app);
        for (const auto& i : mothers) {
            out << season   << "\t" << day
                            << "\t" << i.ID
                            << "\t" << i.current_location
                            << "\t" << i.stage
                            << "\t" << i.energy
                            << "\t" << i.milk
                            << "\n";
        }
        for (const auto& i : pups) {
            out << season   << "\t" << day
                            << "\t" << i.ID
                            << "\t" << i.current_location
                            << "\t" << i.stage
                            << "\t" << i.energy
                            << "\t" << i.milk
                            << "\n";
        }
        out.close();
    }
    
    
    std::vector<individual> mothers;
    
    std::vector<individual> pups;
    size_t id_counter;
    
    std::map<size_t, size_t> available_mothers;
};
