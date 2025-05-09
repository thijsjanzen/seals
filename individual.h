//
//  individual.h
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include "rand_t.h"

enum life_stage {pup, mother}; //pup=0, mother=1
enum location {colony, foraging}; //colony=0, foraging=1

enum class activity {foraging, colony, drinking, nothing};

struct track {
    int t_;
    double energy_;
    double milk_;
    
    activity current_activity_;
    life_stage seal_type;
    
    track(int t, double e, double m, activity ca, life_stage ls) :
    t_(t), energy_(e), milk_(m), current_activity_(ca), seal_type(ls) {}
};

struct individual {
    double energy;
    double milk;
    size_t ID;
    size_t offspring_ID;
    size_t mother_ID;
    bool live_offspring;
    int time_since_pup_death;
    int age;

    int foraging_duration;
    int colonystay_duration;
    double E_at_ForageStart;
    
    location current_location;
    life_stage stage;
    
    std::vector<track> history;
    std::vector<int> past_foraging_duration;
    std::vector<int> past_colony_duration;
    
    individual(double init_energy,
               life_stage init_stage,
               size_t member_id) :
    energy(init_energy), stage(init_stage), ID(member_id) {
        milk = 0.0;
        age = 0;
        current_location = location::colony;
        time_since_pup_death = -1;
        colonystay_duration = 0;
        E_at_ForageStart = init_energy;//only average across indivs that have already left for foraging obv
    }
    
    bool survive(rnd_t& rndgen, double c_survival, double b_survival) {
        double prob = calc_survival_prob(c_survival, b_survival);
        if (!rndgen.bernouilli(prob)) return false;
        
        return true; // yay, survived!
    }
    
    double calc_survival_prob(double c_survival, double b_survival) {//here, I added b_survival, as a way to add a baseline
        if (energy < 0) energy = 0.0;
        double prob = (1.0 - exp(-c_survival * energy)) * b_survival;
        return prob;
    }
    
    individual reproduce(size_t new_id,
                         double offspring_energy) {
        individual offspring(offspring_energy,
                             life_stage::pup,
                             new_id);
        offspring.mother_ID = ID;
        offspring_ID = new_id;
        live_offspring = true;
        return offspring;
    }
    
    double calc_foraging_prob() {
        
        double foraging_prob = 1 - energy;
        if (foraging_prob > 1 || foraging_prob < 0) {
            std::cout << foraging_prob<<" Error! Foraging_prob not a probability..." << std::endl;
        }
        return foraging_prob;
        //previous version:
        //if (energy < 0.3) return 1.0;
        //return 0.0; // this can also be done smarter of course.
    }
    
    void start_stop_foraging(rnd_t& rndgen,
                             const parameters& params) {
        if (current_location == location::colony) {
            double prob = calc_foraging_prob();
            if (rndgen.bernouilli(prob)) {
                E_at_ForageStart = energy;
                start_foraging(rndgen, params);
                past_foraging_duration.push_back(foraging_duration);
            }
            else {
                colonystay_duration = colonystay_duration + 1;
            }
        } else {
            foraging_duration--;
            if (foraging_duration < 1) {
                // return to colony
                current_location = colony;
                //energy = 1.0; //is now set at the beginning of foraging, no energy is used up whilst foraging
                milk = 1.0;
                past_colony_duration.push_back(colonystay_duration);
                colonystay_duration = 0;
            }
        }
    }
    
    void start_foraging(rnd_t& rndgen,
                        const parameters& params) {
        current_location = foraging;
        energy = 1.0;
        foraging_duration = rndgen.normal_positive(params.foraging_duration,
                                                   params.foraging_stdev);
        
    }
    
    void produce_milk(const parameters& params) {
        if (current_location == foraging) {
            // update milk?
        }
        if (current_location == colony) {
            if (energy >= 0.1 * params.milk_production) {
                milk += params.milk_production;
                energy -= 0.1 * params.milk_production;
            }
        }
    }
    
    void pay_maintenance(double maint_cost) {
        energy -= maint_cost;
        if (milk > 1.0) energy -= maint_cost;
        
        if (energy < 0) energy = 0.0;
    }
    
    bool allow_allo_nursing() {
        if (milk >= 1.0) return true;
        
        return false;
    }
    
    void update_track(size_t t) {
        activity ca = activity::foraging;
        if (stage == mother) {
            if (current_location == colony) {
                ca = activity::colony;
            } else {
                ca = activity::foraging;
            }
        }
        if (stage == pup) {
            ca = activity::nothing;
        }
        
        
        //   foraging, colony, drinking, nothing
        history.push_back(track(t, energy, milk, ca, stage));
    }
    
};
