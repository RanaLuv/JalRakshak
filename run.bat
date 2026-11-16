@echo off
gcc -Wall -Iinclude -c src/integration/system_controller.c src/integration/glue.c src/data_acquisition/data_acquisition.c src/risk/risk.c src/map/map.c src/routing/routing.c src/routing/route_planner.c src/alerts/alerts.c
if errorlevel 1 exit /b 1
g++ -Wall -Iinclude src/integration/main.cpp src/integration/controller_class.cpp src/integration/report_classes.cpp system_controller.o glue.o data_acquisition.o risk.o map.o routing.o route_planner.o alerts.o -o jalrakshak
if errorlevel 1 exit /b 1
jalrakshak.exe