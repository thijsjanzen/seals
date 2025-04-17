//
//  main.cpp
//  Seals
//
//  Created by Thijs Janzen on 09/12/2024.
//

#include <iostream>
#include <chrono>
#include "parameters.h"
#include "simulation.h"

int main(int argc, const char * argv[]) {
    std::cout << "This is running!" << std::endl;
    parameters params;
    params.read_from_config("config.ini");
    auto T0 = std::chrono::high_resolution_clock::now();
   
    std::ofstream outf("summary_maintenance_fullsweep_allonursing.txt");
    std::ofstream outx("x.txt");
    
  /*  for (double mc = 0.0; mc < 0.2; mc += 0.01) {
        params.maintenance_cost_pup = mc;
        for (size_t r = 0; r < 100; ++r) {
            simulation sim(params);
            sim.initialize();
            sim.run(outx, r);
            
            outf << mc << "\t" <<
            r << "\t" <<
            sim.mothers.size() << "\t" <<
            sim.pups.size() << "\t" <<
            sim.dead_mothers.size() << "\t" <<
            sim.dead_pups.size() << "\n";
            
            std::cout << mc << "\t" <<
            r << "\t" <<
            sim.mothers.size() << "\t" <<
            sim.pups.size() << "\t" <<
            sim.dead_mothers.size() << "\t" <<
            sim.dead_pups.size() << "\n";
        }
    }
    */
    
  //  std::ofstream outf("summary.txt");
   // size_t r = 0;
  //  std::ofstream outx("x.txt");
    
    params.maintenance_cost_pup = 0.05;
    for (int sdev = 0; sdev < 20; ++sdev) {
        params.foraging_stdev = sdev;
        for (int duration = 1; duration < 20; ++duration) {
            params.foraging_duration = duration;
            for (size_t r = 0; r < 100; ++r) {
                params.seed = duration * r + 13 * sdev;
                simulation sim(params);
                
                sim.initialize();
                
                
                sim.run(outx, r);
                
                double avg_foraging_duration = 0.0;
                double avg_colony_duration = 0.0;
                size_t foraging_cnt = 0;
                size_t colony_cnt = 0;
                for (const auto& i : sim.mothers) {
                    for (const auto& j : i.past_foraging_duration) {
                        avg_foraging_duration += j;
                        foraging_cnt++;
                    }
                    for (const auto& j : i.past_colony_duration) {
                        avg_colony_duration += j;
                        colony_cnt++;
                    }
                }
                
                
                
                outf << duration << "\t" <<
                r << "\t" <<
                sim.mothers.size() << "\t" <<
                sim.pups.size() << "\t" <<
                sim.dead_mothers.size() << "\t" <<
                sim.dead_pups.size() << "\t" <<
                sdev << "\t" <<
                1.0 * sim.allonurse_counter / (sim.allonurse_counter + sim.nurse_counter) << "\t" <<
                avg_foraging_duration / foraging_cnt << "\t" <<
                avg_colony_duration / colony_cnt << "\n";
                
                
                std::cout << duration << "\t" <<
                r << "\t" <<
                sim.mothers.size() << "\t" <<
                sim.pups.size() << "\t" <<
                sim.dead_mothers.size() << "\t" <<
                sim.dead_pups.size() << "\t" <<
                sdev << "\t" <<
                1.0 * sim.allonurse_counter / (sim.allonurse_counter + sim.nurse_counter) << "\t" <<
                avg_foraging_duration / foraging_cnt << "\t" <<
                avg_colony_duration / colony_cnt << "\n";
            }
            // sim.write_track("track.txt");
        }
    }
    outf.close();

    auto T1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> DT = (T1 - T0);    
    std::cout << "this took: " << DT.count() << " seconds\n";
    
    return 0;
}
