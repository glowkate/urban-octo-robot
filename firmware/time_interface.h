#ifndef __TIME_INTERFACE_H__
#define __TIME_INTERFACE_H__

#include "basic_types.h"

namespace Time
{
/// @brief A unit of time measured in ms since the device started.

extern const char deviceTimeMSTag[];
using DeviceTimeMS = UrbanRobot::TypeSafeNumber< unsigned int, deviceTimeMSTag >;

extern const char timeMSTag[];
using TimeMS = UrbanRobot::TypeSafeNumber< unsigned int, timeMSTag >;

extern const char realTimeSTag[];
using RealTimeS = UrbanRobot::TypeSafeNumber< unsigned int, realTimeSTag >;

extern const char deviceTimeUSTag[];
using DeviceTimeUS = UrbanRobot::TypeSafeNumber< unsigned long long, deviceTimeUSTag>;

extern const char timeUSTag[];
using TimeUS = UrbanRobot::TypeSafeNumber< unsigned long long, timeUSTag>;

class Interface {
  public:

  virtual RealTimeS     secondsSince1970() = 0;
  virtual DeviceTimeMS  msSinceDeviceStart() = 0;
};

inline TimeMS UStoMS( TimeUS in ) { return TimeMS( in.get() / 1000 ); }
inline TimeUS MStoUS( TimeMS in ) { return TimeUS( in.get() * 1000 ); }

constexpr unsigned int USPerMs = 1000;
constexpr unsigned int USPerS = 1000000;

} // end Time namespace

inline Time::DeviceTimeUS operator+( Time::DeviceTimeUS lhs, Time::TimeUS rhs ) {
  return lhs + rhs.get();
}

inline Time::DeviceTimeUS operator+( Time::TimeUS lhs, Time::DeviceTimeUS rhs ) {
  return rhs + lhs.get();
}



#endif


