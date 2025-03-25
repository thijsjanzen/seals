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

    
    simulation(const parameters& p) : params(p) {
        // set specific parameters
        rndgen.set_seed(params.seed);
    }
    
    void run(std::ofstream& outf, size_t r) {
        id_counter = 0;
        initialize();
        int season = 0;
        simulate_season(season, r, outf);
        //for (size_t season = 0; season < params.num_seasons; ++season) {
        //   // std::cout << "Season: " << season << std::endl;
        //    simulate_season(season, outf);
        //    update_population_end_of_season();
        //    //std::cout << season << " " << mothers.size() << " " << pups.size() << "\n";
        //    if (mothers.size() + pups.size() < 1) break;
        //}
        //outf.close();
    }
    
    void initialize() {
        mothers.clear();
        pups.clear();
        nurse_counter = 0;
        allonurse_counter = 0;

        for (size_t i = 0; i < params.init_population_size; ++i) {
            //double NewInitEnergy = rndgen.uniform_real(0.0, 1.0);
            double NewInitEnergy = params.init_energy;
            mothers.push_back( individual(NewInitEnergy,
                                       life_stage::mother,
                                       ++id_counter)); // this increments the counter after use!
            auto new_pup = mothers.back().reproduce(++id_counter,
                                                    params.init_offspring_energy);
            mothers.back().milk = 1.0;
            pups.push_back(new_pup);
        }
    }
    
    void simulate_season(size_t season, size_t r, std::ofstream& outf) {
        for (size_t days = 0; days < params.season_length; ++days) {
            //std::cout << "Day: " << days << std::endl;
            nurse_counter = 0; //for now, calculate nurse counter per day
            allonurse_counter = 0;
            update_mothers();
            update_pups();
            if (days == 99) {
                write_to_file(season, days, r, outf);
            }
         //   if (days == params.season_length - 1) {
          //      std::cout <<"At the end of season we have: " << season << " " << days << " " << mothers.size() << " " << pups.size() << "\n";
         //   }
        }
    }
    
    void update_mothers() {
        available_mothers.clear();
        for (size_t i = 0; i < mothers.size(); ) {
            mothers[i].age++;
            if (mothers[i].milk < 0) { std::cout << "ERROR! Negative milk..." << std::endl; }
            
            if (mothers[i].current_location == location::colony) {
                mothers[i].pay_maintenance(params.maintenance_cost);
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

            if (!mothers[i].survive(rndgen, params.c_survival_mother, 1.0)) {// here, b_survival is manually set to 1.0
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
    
    void update_pups() {
        for (size_t i = 0; i < pups.size(); ) {
            pups[i].age++;
            pups[i].pay_maintenance(params.maintenance_cost);
            if (!pups[i].survive(rndgen,params.c_survival_pup,params.base_surv_pup)) {//used to be set manually//now via config file
                //a little for loop to update whether this mother has a dead pup or not... there must be an easier way to do this?
                for (int j = 0; j < mothers.size(); j++) {
                    if (mothers[j].ID == pups[i].mother_ID) {
                        mothers[j].live_offspring = false;
                    }
                }
                pups[i] = pups.back();
                pups.pop_back();
            } else {
                auto mother_index = find_mother(pups[i].mother_ID);
                if (mother_index >= 0) {
                    nurse(&pups[i], &mothers[mother_index], params);
                } else {
                    // no (allo) mother found to nurse from!
                }
                if (pups[i].energy > 1) { pups[i].energy = 1; } //@Thijs: New! I think it makes sense to bound energy between 0 and 1. You can't just keep eating and get more and more energy from that. What do you think?
                if (pups[i].energy < 0) { pups[i].energy = 0; }

                ++i;
            }
        }
        for (size_t i = 0; i < pups.size(); i++) {
            if (pups[i].energy > 1) { std::cout << "Pup has high energy! " << pups[i].energy << std::endl; }
        }
    }
    
    int find_mother(size_t pup_id) {
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
        
        
        bool PICK_NOT_REPEATING = true;
                
        if (index < 0) {
            size_t max_num_tries = std::min(params.max_num_tries,
                                            available_mothers.size());
            
            if (PICK_NOT_REPEATING) {
                
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
                
            } else {
                 // random tries
                for (size_t num_tries = 0; num_tries < max_num_tries; ++num_tries) {
                    
                    auto x = available_mothers.begin();
                    auto r = rndgen.random_number(available_mothers.size());
                    std::advance(x, r);
                    index = static_cast<int>(x->second);
                    
                    // index = rndgen.random_number(mothers.size());
                    //while (mothers[index].current_location != location::colony) {
                    //    index = rndgen.random_number(mothers.size());
                    //}
                    if (mothers[index].allow_allo_nursing()) {
                        mother_found = true;
                        if (mothers[index].milk > 0) {
                            allonurse_counter++;
                        }
                        break;
                    }
                }
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
            //<< "," << season * params.season_length + day
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
    
    
    std::vector<individual> mothers;
    
    std::vector<individual> pups;
    size_t id_counter;
    
    std::map<size_t, size_t> available_mothers;
};
