#include <iostream>
using namespace std;
#include "Logger.h"
using namespace yazi::utility;

int main(){
    Logger::instance()->open("test.log");

    debug("This is a test");
    info("INFO TEST");
    warn("WARN TEST");
    error("ERROR");
    return 0;
}