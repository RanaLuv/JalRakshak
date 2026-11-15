// main.cpp 

#include <iostream>
#include "controller_class.h"

using namespace std;

int main() {
    try {
        SystemController controller("JalRakshak");
        controller.start();
    }
    catch (const SystemException &e) {
        cout << "System error: " << e.what() << endl;
        return 1;
    }

    cout << "Program finished." << endl;
    return 0;
}
