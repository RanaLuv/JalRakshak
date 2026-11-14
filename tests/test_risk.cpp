#include "../src/risk/risk_calculator.hpp"
#include <iostream>

void run_scenario(const char* scenario_name, double* rain, int nr, double* water, int nw, int reports) {
    std::cout << "=== SCENARIO: " << scenario_name << " ===\n";

    // 1. Calculator object banana
    RiskCalculator calc;
    
    // 2. Polymorphic Detectors add karna
    calc.add_detector(new SpikeDetector());
    calc.add_detector(new TrendDetector());

    // 3. Risk calculate karna
    RiskResult res = calc.calculate(1, rain, nr, water, nw, reports);

    // 4. Result print karna
    explain_risk(&res);
    std::cout << "\n";
}

int main() {
    std::cout << "--- STARTING RISK ENGINE TESTS ---\n\n";

    // Step 1: Config load karna
    RiskWeights w;
    Thresholds t;
    if (load_config("data/config.txt", &w, &t) != STATUS_OK) {
        std::cout << "Failed to load config.txt!\n";
        return 1;
    }
    std::cout << "Config loaded successfully.\n\n";

    // Step 2: Normal Scenario (Paani aur baarish kam hai)
    double normal_rain[] = {10.0, 15.0, 12.0};
    double normal_water[] = {1.0, 1.1, 1.15};
    run_scenario("Normal Rainfall", normal_rain, 3, normal_water, 3, 1);

    // Step 3: Flood Scenario (Achanak paani ka level badha)
    // 1.5 se seedha 4.5 matlab spike limit (1.5) cross ho gayi
    double flood_rain[] = {50.0, 90.0, 160.0};
    double flood_water[] = {1.2, 1.5, 4.5}; 
    run_scenario("Rapid Water-Level Rise", flood_rain, 3, flood_water, 3, 8);

    // Step 4: Custom Sorting Demo
    std::cout << "=== SCENARIO: Custom Sorting ===\n";
    LocationRisk locs[3];
    locs[0].location_id = 101; locs[0].result.score = 45.0;
    locs[1].location_id = 102; locs[1].result.score = 95.0; // Sabse zyada risk
    locs[2].location_id = 103; locs[2].result.score = 15.0; // Sabse kam risk

    sort_by_risk(locs, 3); // Tumhara banaya hua Bubble Sort

    for(int i = 0; i < 3; i++) {
        std::cout << "Rank " << i+1 << ": Location ID " << locs[i].location_id 
                  << " -> Score: " << locs[i].result.score << "%\n";
    }

    return 0;
}