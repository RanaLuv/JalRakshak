# Test Results

| Scenario | How to run | Expected | Result (Pass/Fail) | Notes |
|----------|-----------|----------|--------------------|-------|
| Normal rainfall | Option 1, choose 1, then option 2 and 3 | All places NORMAL | | |
| Rapid water-level rise | Option 1, choose 2 or 3, then option 2 and 3 | Risk reaches WARNING or CRITICAL | | |
| Invalid reading | Load the flood file | "Invalid readings skipped: 1" | | |
| Blocked bridge | Option 8 (route), then option 7 (block bridge) | Route changes, new distance shown | | |
| No reachable shelter | Block both roads, then option 8 | "No safe route found" | | |
| Duplicate reports | Option 6 twice for the same place | Second report marked duplicate | | |
| Multiple alerts | Run simulation on critical file, option 4 | Highest priority alert first | | |
| Wrong input | Type letters at the menu | Asks again, no crash | | |
| Missing file | Rename a data file and start the program | "System error: Missing file" | | |
| Save and load state | Options 10 and 11 | State restored | | |
