//
//  simulation.h
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include "rand_t.h"
#include "parameters.h"
#include "individual.h"
#include <numeric>

struct simulation {
    
    rnd_t rndgen;
    parameters params;

    //keeping track of some counters
    int nurse_counter = 0;
    int allonurse_counter = 0;
    
    int num_arrived_mothers = 0;

    
    simulation(const parameters& p) : params(p) {
        // set specific parameters
        rndgen.set_seed(params.seed);
    }
    
    void run(std::ofstream& outf, size_t r) {
        id_counter = 0;
        initialize();
        int season = 0;
        simulate_season(season, r, outf);
    }
    
    void initialize() {
        mothers.clear();
        pups.clear();
        nurse_counter = 0;
        allonurse_counter = 0;
        num_arrived_mothers = 0;

        for (size_t i = 0; i < params.init_population_size; ++i) {
            //double NewInitEnergy = rndgen.uniform_real(0.0, 1.0);
            
            if (rndgen.bernouilli(params.arrival_prob)) {
                
                double NewInitEnergy = params.init_energy;
                mothers.push_back( individual(NewInitEnergy,
                                              life_stage::mother,
                                              ++id_counter)); // this increments the counter after use!
                auto new_pup = mothers.back().reproduce(++id_counter,
                                                        params.init_offspring_energy);
                mothers.back().milk = 1.0;
                pups.push_back(new_pup);
                
                num_arrived_mothers++;
            }
        }
        
        update_tracks(-1);
    }
    
    void simulate_season(size_t season, size_t r, std::ofstream& outf) {
        for (size_t days = 0; days < params.season_length; ++days) {
            //std::cout << "Day: " << days << std::endl;
            nurse_counter = 0; //for now, calculate nurse counter per day
            allonurse_counter = 0;
            
            if (num_arrived_mothers < params.init_population_size) {
                add_mothers();
            }
            
            update_mothers(days);
            update_pups(days);
            
            update_tracks(days);
            
            if (days == 99) {
                write_to_file(season, days, r, outf);
            }
        }
    }
    
    void add_mothers() {
        size_t remaining = params.init_population_size - num_arrived_mothers;
        for (size_t i = 0; i < remaining; ++i) {
            if (rndgen.bernouilli(params.arrival_prob)) {
                
                double NewInitEnergy = params.init_energy;
                mothers.push_back( individual(NewInitEnergy,
                                              life_stage::mother,
                                              ++id_counter)); // this increments the counter after use!
                auto new_pup = mothers.back().reproduce(++id_counter,
                                                        params.init_offspring_energy);
                mothers.back().milk = 1.0;
                pups.push_back(new_pup);
                
                num_arrived_mothers++;
            }
        }
    }
    
    void update_mothers(size_t t) {
        available_mothers.clear();
        for (size_t i = 0; i < mothers.size(); ) {
            mothers[i].age++;
            if (mothers[i].milk < 0) { std::cout << "ERROR! Negative milk..." << std::endl; }
            
            if (mothers[i].current_location == location::colony) {
                mothers[i].pay_maintenance(params.maintenance_cost_mother);
                if (mothers[i].live_offspring) {
                    mothers[i].time_since_pup_death = -1;
                } else {
                    mothers[i].time_since_pup_death++;
                }
                
                if (mothers[i].time_since_pup_death < params.milk_prod_cutoff) {
                    mothers[i].produce_milk(params);
                }

                if (mothers[i].time_since_pup_death > (params.milk_prod_cutoff+10)) {//NEW: Yitzchak pointed out that milk eventually gets re-absorbed!
                    mothers[i].milk = 0;
                }
            }

            if (!mothers[i].survive(rndgen, params.c_survival_mother, params.base_surv_mother)) {// here, b_survival is manually set to 1.0
                dead_mothers.push_back(mothers[i]);
                dead_mothers.back().update_track(t);
                mothers[i] = mothers.back();
                mothers.pop_back();
            } 
            else {
                mothers[i].start_stop_foraging(rndgen, params);
                
                if (mothers[i].current_location == location::colony) {
                    available_mothers.insert({mothers[i].ID, i});
                }
                    
                ++i;
            }
        }
    }
    
    void update_pups(size_t t) {
        
        std::vector<size_t> orphans; // orphans, or pups with a foraging mother
        
        for (size_t i = 0; i < pups.size(); ) {
            pups[i].age++;
            pups[i].pay_maintenance(params.maintenance_cost_pup);
            if (!pups[i].survive(rndgen, params.c_survival_pup, params.base_surv_pup)) {//used to be set manually//now via config file
                //a little for loop to update whether this mother has a dead pup or not... there must be an easier way to do this?
                for (int j = 0; j < mothers.size(); j++) {
                    if (mothers[j].ID == pups[i].mother_ID) {
                        mothers[j].live_offspring = false;
                    }
                }
                dead_pups.push_back(pups[i]);
                dead_pups.back().update_track(t);
                pups[i] = pups.back();
                pups.pop_back();
            } else {
                auto mother_index = find_own_mother(pups[i].mother_ID);
                if (mother_index >= 0) {
                    nurse(&pups[i], &mothers[mother_index], params);
                } else {
                  // no own mother available
                  orphans.push_back(i);
                }
                
                if (pups[i].energy > 1) { pups[i].energy = 1; } //@Thijs: New! I think it makes sense to bound energy between 0 and 1. You can't just keep eating and get more and more energy from that. What do you think?
                if (pups[i].energy < 0) { pups[i].energy = 0; }

                ++i;
            }
        }
        
        // go over orphans
        for (const auto& i : orphans) {
            auto mother_index = find_allo_mother(pups[i].mother_ID);
            if (mother_index >= 0) {
                nurse(&pups[i], &mothers[mother_index], params);
            }
            // else: no mother found, no nursing
            if (pups[i].energy > 1) { pups[i].energy = 1; } //@Thijs: New! I think it makes sense to bound energy between 0 and 1. You can't just keep eating and get more and more energy from that. What do you think?
            if (pups[i].energy < 0) { pups[i].energy = 0; }
        }
        
        
        for (size_t i = 0; i < pups.size(); i++) {
            if (pups[i].energy > 1) { std::cout << "Pup has high energy! " << pups[i].energy << std::endl; }
        }
    }
    
    int find_own_mother(size_t pup_id) {
        if (available_mothers.empty()) return -1;
        int index = -1;
        bool mother_found = false;
        
        auto x = available_mothers.find(pup_id);
        if (x != available_mothers.end()) {
            index = static_cast<int>(x->second);
            mother_found = true;
            if (mothers[x->second].milk>0) { //@Thijs: is this correctly done?
                nurse_counter++;  //If statement is to ensure that the nursing counter only increases if the mother has enough milk for a nursing event
            }
        }
        return index;
    }
    
    int find_allo_mother(size_t pup_id) {
        if (available_mothers.empty()) return -1;

        int index = -1;
        bool mother_found = false;
        
        size_t max_num_tries = std::min(params.max_num_tries,
                                        available_mothers.size());
            
        std::vector<size_t> potential_mothers(available_mothers.size());
        std::iota(potential_mothers.begin(), potential_mothers.end(), 0);
        
        for (size_t i = 0; i < max_num_tries; ++i) {
            if (potential_mothers.size() > 1) {
                size_t j = i + rndgen.random_number(static_cast<int>(potential_mothers.size() - i));
                if (i != j) {
                    std::swap(potential_mothers[i], potential_mothers[j]);
                }
            }
        }
        
        for (size_t num_tries = 0; num_tries < max_num_tries; ++num_tries) {
            auto x = available_mothers.begin();
            auto r = potential_mothers[num_tries];
            std::advance(x, r);
            index = static_cast<int>(x->second);
            
            if (mothers[index].allow_allo_nursing()) {
                mother_found = true;
                if (mothers[index].milk > 0) {
                    allonurse_counter++;
                }
                break;
            }
        }
        
        if (!mother_found) index = -1;
        
        return index;
    }
    
    void update_population_end_of_season() {
        //only add pups to the mother population if there is still space
        size_t OpenSpaces = params.max_pop_size - mothers.size();
        //std::cout << "Nr mothers: " << mothers.size() << ", open spaces: " << OpenSpaces << std::endl;

        for (int i = 0; i < OpenSpaces; i++) {
            if (pups.size() < 1) { break; }
            int j = rndgen.random_number(pups.size());
            pups[j].stage = mother; //milk is automatically 0 when pups are born - is that good or bad? Does it matter? Probably not?
            pups[j].current_location = colony;
            mothers.push_back(pups[j]);
            pups[j] = pups.back();
            pups.pop_back();
        }
        //std::cout << "Nr new mothers: " << mothers.size() << std::endl;

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
            
            //produce milk here maybe? Or not...
            pups.push_back(new_pup);
        }
    }
    
    void nurse(individual* pup, individual* nurse,
               const parameters& p) {
        auto nurse_amount = params.nurse_amount; //new!!
                                                //In later versions: might make this dependent on energy of nurse & pup, for example
                                                //so then could for example write nurse_amount=params.nurse_amount*(1-pups[i].Energy), or something roughly like that
        for (size_t nurse_iter = 0; nurse_iter < nurse_amount; nurse_iter++) {
            if (nurse->milk >= p.milk_consumption) {  // can't feed if there is no milk!! //@THIJS: is this correctly done?: yes.
                nurse->milk -= p.milk_consumption;
                pup->energy += p.milk_consumption;
                //nurse->time_since_pup_death = -1;///CHANGE!! toggle this to see impact //this is to "reset" after allonursing or not; default: no reset
            }
        }
    }
    
    void write_to_file(size_t season,
                       size_t day,
                       size_t r,
                       std::ofstream& outf) {
        //std::ofstream outf("results.csv", std::ios::app);


        //averaged data
        int FemaleForageCounter = 0;
        double avgE_mother = 0.0;
        double avgMilk_mother = 0.0;
        double avgE_pup = 0.0;
        double avg_colonystay_duration = 0.0;
        double avg_E_at_ForageStart = 0.0;

        for (int i = 0; i < mothers.size(); i++) {
            if (mothers[i].current_location == foraging) {
                FemaleForageCounter = FemaleForageCounter + 1; 
                avg_colonystay_duration = avg_colonystay_duration + mothers[i].colonystay_duration;//only do this for females who are currently foraging. This way, u don't count ongoinig colony stays (where u don't know yet how long the stay will have been)
                avg_E_at_ForageStart = avg_E_at_ForageStart + mothers[i].E_at_ForageStart;
            }
            avgE_mother = avgE_mother + mothers[i].energy;
            avgMilk_mother = avgMilk_mother + mothers[i].milk;
        }
        for (int i = 0; i < pups.size(); i++) {
            avgE_pup = avgE_pup + pups[i].energy;
        }
        avgE_mother = avgE_mother / static_cast<double>(mothers.size());
        avgMilk_mother = avgMilk_mother / static_cast<double>(mothers.size());
        avgE_pup = avgE_pup / static_cast<double>(pups.size());
        avg_colonystay_duration = avg_colonystay_duration / static_cast<double>(FemaleForageCounter);
        avg_E_at_ForageStart = avg_E_at_ForageStart / static_cast<double>(FemaleForageCounter);
        //std::cout << "AvgE at FStart" << avg_E_at_ForageStart << std::endl;

        outf << season << "," << day
            // d<< "," << season * params.season_length + day
            << "," << r
            << "," << mothers.size() + pups.size()
            << "," << mothers.size()
            << "," << pups.size()
            << "," << nurse_counter
            << "," << allonurse_counter
            << "," << static_cast<double>(allonurse_counter) / (static_cast<double>(allonurse_counter) + static_cast<double>(nurse_counter))
            << "," << static_cast<double>(allonurse_counter) / static_cast<double>(pups.size())
            << "," << FemaleForageCounter
            << "," << params.foraging_duration
            << "," << params.foraging_stdev
            << "," << params.base_surv_pup
            << "," << params.init_population_size
            << "," << avgE_mother
            << "," << avgMilk_mother
            << "," << avgE_pup
            << "," << avg_colonystay_duration
            << "\n";

        //Individual data
        /*for (const auto& i : mothers) {
            outf << season   << "," << day
                            << "," << i.ID
                            << "," << i.current_location
                            << "," << i.stage
                            << "," << i.energy
                            << "," << i.milk
                            << "," << i.age
                            << "," << i.mother_ID
                            << "\n";
        }
        for (const auto& i : pups) {
            outf << season   << "," << day
                            << "," << i.ID
                            << "," << i.current_location
                            << "," << i.stage
                            << "," << i.energy
                            << "," << i.milk
                            << "," << i.age
                            << "," << i.mother_ID
                            << "\n";
        }*/

        //out.close();
    }
    
    void update_state(std::vector<individual> *v, size_t t) {
        for (auto& i : *v) {
            i.update_track(t);
        }
    }
    
    void update_tracks(size_t t) {
        update_state(&mothers, t);
        update_state(&pups, t);
    }
    
    void write_state(const std::vector<individual>& v, std::ofstream& out,
                     bool alive) {
        for (const auto& i : v) {
            for (const auto& j : i.history) {
                out << i.ID << "\t" 
                    << j.t_ << "\t"
                    << j.energy_ << "\t"
                    << j.milk_ << "\t"
                    << static_cast<size_t>(j.current_activity_) << "\t"
                    << j.seal_type << "\t"
                    << alive << "\n";
            }
        }
    }
    
    void write_track(const std::string& file_name) {
        std::ofstream out_file(file_name.c_str());
        
        write_state(mothers, out_file, true);
        write_state(pups, out_file, true);
        write_state(dead_mothers, out_file, false);
        write_state(dead_pups, out_file, false);
        out_file.close();
    }
    
    
    std::vector<individual> mothers;
    
    std::vector<individual> pups;
    size_t id_counter;
    
    std::map<size_t, size_t> available_mothers;
    
    
    std::vector<individual> dead_mothers;
    std::vector<individual> dead_pups;
};
