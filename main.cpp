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
   
    
    std::ofstream outx("Seals_ForagingMean10_ForagingStDev_ColonySize200_PupSurv1_ArrivalProb.csv");
    outx << "Season" << "," << "Day" << "," << "Rep" << "," << "Popsize" << "," << "PopSize_Mother" << "," << "PopSize_Pup" << "," << "NursingCounter" << "," << "AllonursingCounter"
        << "," << "PropNurseAllo" << "," << "PropPupAllo" << "," << "NrMotherForage" << "," << "Forage_Mean" << "," << "Forage_StDev" << "," << "PupSurvB" << "," << "PopSizeIni"
        << "," << "AvgE_mother" << "," << "AvgM_mother" << "," << "AvgE_pup" << "," << "Avg_ColonyStayDuration" << "," << "FemaleArrivalProb";
    outx << '\n';
    
    //params.foraging_stdev = 2;
    params.foraging_duration = 10;
    params.init_population_size = 200;
    params.base_surv_pup = 1.0;



    for (double FS_nr = 0; FS_nr < 17; FS_nr++) {
        
        double FS = FS_nr * 0.25;
        //double FS = FS_nr;

        /*double PSB = 1 - PSB_nr * 0.05;
        std::cout << "season survival probability: " << PSB << std::endl;
        double PSB_daily = std::pow(PSB,1.0/100);
        std::cout << "daily survival probability: " << PSB_daily << std::endl;
        params.base_surv_pup = PSB_daily;*/

        /*double CS2 = CS * 100;
        if (CS == 0) { CS2 = 50; }*/

        params.foraging_stdev = FS;
        std::cout << "FS: " << params.foraging_stdev << std::endl;
        //std::cout << "PSB daily: " << params.base_surv_pup << std::endl;
        //std::cout << "PSB season: " << std::pow(params.base_surv_pup,100) << std::endl;

        for (int AP = 0; AP < 1; AP++) {
            params.arrival_prob = 0.3 - AP * 0.05;
            std::cout << "FM: " << params.arrival_prob << std::endl;

            // create simulation
            simulation sim(params);
            for (int seed = 0; seed < 300; seed++) {
                // for every replicate, reset the simulation and reset the seed.
                sim.initialize(seed);
                sim.run(outx,seed);
            }
            sim.write_track("test.txt");
        }
    }

    auto T1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> DT = (T1 - T0);    
    std::cout << "this took: " << DT.count() << " seconds\n";
    
    return 0;
}

