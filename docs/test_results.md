# Test Results

| Scenario | How to run | Expected | Result (Pass/Fail) | Notes |
|----------|-----------|----------|--------------------|-------|
| Normal rainfall | Option 1, choose 1, then option 2 and 3 | All places NORMAL | | |
| Rapid water-level rise | Option 1, choose 2, then option 2 and 3 | All places WARNING, sudden rise noted | | |
| Invalid reading | Load the flood file | "Invalid readings skipped: 1" | | |
| Blocked road | Option 8 (Riverside), then option 7 with roads 5 and 6 | Route changes from 8 km to 11 km | | |
| No reachable shelter | Also block roads 4 and 5 (the bridge) | "No safe route found" | | |
| Duplicate reports | Option 6 twice for the same place | Second report marked duplicate | | |
| Multiple alerts | Critical file plus 5 reports for Riverside, option 2, then option 4 | CRITICAL alert first | | |
| Wrong input | Type letters at the menu | Asks again, no crash | | |
| Missing file | Rename a data file and start the program | "System error: Missing file" | | |
| Save and load state | Options 10 and 11 | State restored | | |
