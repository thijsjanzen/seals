//
//  individual.h
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include "rand_t.h"

enum life_stage {pup, mother};
enum location {colony, foraging};

struct individual {
    double energy;
    double milk;
    size_t ID;
    size_t offspring_ID;
    size_t mother_ID;
    
    int foraging_duration;
    
    location current_location;
    life_stage stage;
    
    individual(double init_energy,
               life_stage init_stage,
               size_t member_id) :
    energy(init_energy), stage(init_stage), ID(member_id) {
        milk = 0.0;
        current_location = location::colony;
    }
    
    bool survive(rnd_t& rndgen) {
        double prob = calc_survival_prob();
        if (!rndgen.bernouilli(prob)) return false;
        
        return true; // yay, survived!
    }
    
    double calc_survival_prob() {
        if (energy < 0) energy = 0;
        
        double prob = energy; // this should be done smarter ofc.
        if (prob > 1.0) prob = 1.0;
        
        return prob;
    }
    
    individual reproduce(size_t new_id,
                         double offspring_energy) {
        individual offspring(offspring_energy,
                             life_stage::pup,
                             new_id);
        offspring.mother_ID = ID;
        offspring_ID = new_id;
        return offspring;
    }
    
    double calc_foraging_prob() {
        if (energy < 0.3) return 1.0;
        
        return 0.0; // this can also be done smarter of course.
    }
    
    void start_stop_foraging(rnd_t& rndgen,
                             const parameters& params) {
        if (current_location == location::colony) {
            double prob = calc_foraging_prob();
            if (rndgen.bernouilli(prob)) {
                start_foraging(rndgen, params);
            }
        } else {
            foraging_duration--;
            if (foraging_duration < 1) {
                // return to colony
                current_location = colony;
                energy = 1.0;
                milk = 1.0;
            }
        }
    }
    
    void start_foraging(rnd_t& rndgen,
                        const parameters& params) {
        current_location = foraging;
        foraging_duration = rndgen.normal_positive(params.foraging_duration,
                                                   params.foraging_stdev);
    }
    
    void produce_milk(const parameters& params) {
        if (current_location == foraging) {
            // update milk?
        }
        if (current_location == colony) {
            milk += params.milk_production;
            energy -= 0.1 * params.milk_production;
        }
    }
    
    void pay_maintenance(const parameters& params) {
        energy -= params.maintenance_cost;
        if (milk > 1.0) energy -= params.maintenance_cost;
    }
    
    bool allow_allo_nursing() {
        if (milk >= 1.0) return true;
        
        return false;
    }
    
};
