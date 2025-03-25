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

    simulation sim(params);
    auto T0 = std::chrono::high_resolution_clock::now();


    //indiv data
    /*std::ofstream outf("ind_data_PupSur10_b1_ResetTimeSincePupDeath_FeedReset10.csv", std::ios::app);
    outf << "Season" << "," << "Day" << "," << "ID" << "," << "Location" << "," << "Stage" << "," << "Energy" << "," << "Milk" << "," << "Age" << "," << "MotherID";
    outf << '\n';*/

    //avg data
    //std::ofstream outf("avgSeals_"
    //    //+ "_bPSur" + std::to_string(params.base_surv_pup)
    //    //+ "_ForageM" + std::to_string(static_cast<int>(params.foraging_duration))
    //    //+ "_ForageSt" + std::to_string(params.foraging_stdev)
    //    //+ "_PopSize" + std::to_string(static_cast<int>(params.init_population_size))
    //    +"_MilkLag" + std::to_string(static_cast<int>(params.milk_prod_cutoff))
    //    + ".csv", std::ios::app);
    std::ofstream outf("avgSeals_LoopData_ParaCombis.csv", std::ios::app);
    outf << "Season" << "," << "Day" << "," << "Rep" << "," << "PopSize" << "," << "PopSize_Mother" << "," << "PopSize_Pup" << "," << "NursingCounter" << "," << "AllonursingCounter"
        << "," << "PropNurseAllo" << "," << "PropPupAllo" << "," << "NrMotherForage" << "," << "Forage_Mean" << "," << "Forage_StDev" << "," << "PupSurvB" << "," << "PopSizeIni"
        << "," << "AvgE_mother" << "," << "AvgM" << "," << "AvgE_pup" << "," << "Avg_ColonyStayDuration";
    outf << '\n';

    for (double FM_count = 0; FM_count < 6; FM_count++) {
        sim.params.foraging_duration = 4.0 + FM_count * 2;
        std::cout << "Foraging mean: " << sim.params.foraging_duration << "\n";

        for (double FS_count = 0; FS_count < 6; FS_count++) {
            sim.params.foraging_stdev = 1.0 + FS_count;
            std::cout << "Foraging stdev: " << sim.params.foraging_stdev << "\n";

            for (double PSurB_Count = 5; PSurB_Count < 6; PSurB_Count++) {
                sim.params.base_surv_pup = 0.99 + PSurB_Count * 0.002;
                std::cout << "PupSurB: " << sim.params.base_surv_pup << "\n";

                for (double PopSizeIni_Count = 0; PopSizeIni_Count < 6; PopSizeIni_Count++) {
                    sim.params.init_population_size = 50 + PopSizeIni_Count * 50;
                    std::cout << "PopSizeIni: " << sim.params.init_population_size << "\n";

                    for (size_t r = 0; r < params.num_replicates; ++r) {
                        sim.initialize();
                        sim.run(outf, r);
                        //std::cout << "repl: " << r << " of " << params.num_replicates << "\n";
                    }

                }
            }
        }
    }


    outf.close();
    auto T1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> DT = (T1 - T0);    
    std::cout << "this took: " << DT.count() << " seconds\n";
    
    return 0;
}
