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

    //keeping track of some counters
    int nurse_counter = 0;
    int allonurse_counter = 0;

    
    simulation(const parameters& p) : params(p) {
        // set specific parameters
        rndgen.set_seed(params.seed);
    }
    
    void run() {
        id_counter = 0;
        initialize();
        
        //indiv data
        /*std::ofstream outf("ind_data_PupSur10_b1_ResetTimeSincePupDeath_FeedReset10.csv", std::ios::app);
        outf << "Season" << "," << "Day" << "," << "ID" << "," << "Location" << "," << "Stage" << "," << "Energy" << "," << "Milk" << "," << "Age" << "," << "MotherID";
        outf << '\n';*/

        //avg data
        std::ofstream outf("avg_data_PupSur10_b1_FeedReset10_new.csv", std::ios::app);
        outf << "Season" << "," << "Day" << "," << "Time" << "," << "PopSize" << "," << "PopSize_Mother" << "," << "PopSize_Pup" << "," << "NursingCounter" << "," << "AllonursingCounter" << "," << "PropAllo" << "," << "a" << "," << "b";
        outf << '\n';


        for (size_t season = 0; season < params.num_seasons; ++season) {
           // std::cout << "Season: " << season << std::endl;
            simulate_season(season, outf);
            update_population_end_of_season();
            //std::cout << season << " " << mothers.size() << " " << pups.size() << "\n";
            if (mothers.size() + pups.size() < 1) break;
        }
        outf.close();
    }
    
    void initialize() {
        mothers.clear();
        pups.clear();
        nurse_counter = 0;
        allonurse_counter = 0;

        for (size_t i = 0; i < params.init_population_size; ++i) {
            mothers.push_back( individual(params.init_energy,
                                       life_stage::mother,
                                       ++id_counter)); // this increments the counter after use!
            auto new_pup = mothers.back().reproduce(++id_counter,
                                                    params.init_offspring_energy);
            pups.push_back(new_pup);
        }
    }
    
    void simulate_season(size_t season, std::ofstream& outf) {
        for (size_t days = 0; days < params.season_length; ++days) {
            //std::cout << "Day: " << days << std::endl;
            nurse_counter = 0; //for now, calculate nurse counter per day
            allonurse_counter = 0;
            update_mothers();
            update_pups();
            write_to_file(season, days, outf);
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
            mothers[i].start_stop_foraging(rndgen,
                                  params);

            if (mothers[i].current_location == location::colony) {
                mothers[i].pay_maintenance(params);
                if (mothers[i].live_offspring) {mothers[i].time_since_pup_death = -1;}
                else {mothers[i].time_since_pup_death++;}
                if (mothers[i].time_since_pup_death < params.milk_prod_cutoff) {
                    mothers[i].produce_milk(params);
                }
            }

            if (!mothers[i].survive(rndgen, params.c_survival_mother, 1.0)) {//here, b_survival is manually set to 1.0
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
            pups[i].age++;
            pups[i].pay_maintenance(params);
            if (!pups[i].survive(rndgen,params.c_survival_pup,1.0)) {//set baseline manually here //for some reason, it doesn't work otherwise
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
            index = x->second;
            mother_found = true;
            if (mothers[x->second].milk>0) { //@Thijs: is this correctly done?
                nurse_counter++;  //If statement is to ensure that the nursing counter only increases if the mother has enough milk for a nursing event
            }
        }
        
        
        if (index < 0) {
            for (size_t num_tries = 0; num_tries < params.max_num_tries; ++num_tries) {
                index = rndgen.random_number(mothers.size());
                while (mothers[index].current_location != location::colony) {
                    index = rndgen.random_number(mothers.size());
                }
                if (mothers[index].allow_allo_nursing()) {
                    mother_found = true;
                    if (mothers[index].milk > 0) {
                        allonurse_counter++;
                    }
                    break;
                }
            }
        }
        if (!mother_found) index = -1;
        
        return index;
    }
    
    
    void update_population_end_of_season() {
        //only add pups to the mother population if there is still space
        int OpenSpaces = params.max_pop_size - mothers.size();
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
        int nurse_amount = params.nurse_amount;//new!! 
                                               //In later versions: might make this dependent on energy of nurse & pup, for example
                                               //so then could for example write nurse_amount=params.nurse_amount*(1-pups[i].Energy), or something roughly like that
        for (int nurse_iter = 0; nurse_iter < nurse_amount; nurse_iter++) {
            if (nurse->milk > 0) {//can't feed if there is no milk!! //@THIJS: is this correctly done?
                nurse->milk -= p.milk_consumption; 
                pup->energy += p.milk_consumption;
                //nurse->time_since_pup_death = -1;///CHANGE!! toggle this to see impact
            }
        }
    }
    
    void write_to_file(size_t season,
                       size_t day,
                       std::ofstream& outf) {
        //std::ofstream outf("results.csv", std::ios::app);


        //averaged data
        outf << season << "," << day << "," << season * params.season_length + day
            << "," << mothers.size() + pups.size()
            << "," << mothers.size()
            << "," << pups.size()
            << "," << nurse_counter
            << "," << allonurse_counter
            << "," << static_cast<double>(allonurse_counter) / (static_cast<double>(allonurse_counter) + static_cast<double>(nurse_counter))
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
