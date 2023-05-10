#include <chrono>
#include <iostream>
#include <time.h>
#include <ctime>

using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

int main() 
{
    auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    auto sec_since_epoch = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

    cout << "seconds since epoch: " << sec_since_epoch << endl;
    cout << "milliseconds since epoch: " << millisec_since_epoch << endl;

    return EXIT_SUCCESS;
}