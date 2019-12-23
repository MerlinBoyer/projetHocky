#ifndef HOCKY_SENSOR
#define HOCKY_SENSOR

#include <string>

using namespace std;


class HockySensor
{
  public:
    // Constructeurs & destructeurs
    HockySensor();
    ~HockySensor();

    void init();
    long getData();
};

#endif