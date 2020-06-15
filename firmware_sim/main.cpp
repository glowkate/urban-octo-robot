
#include <iostream>
#include <memory>
#include <unistd.h>
#include <time.h>
#include <math.h>   // for adding variation to simulated temperature.

#include "command_scheduler.h"
#include "command_motor.h"
#include "command_process_input.h"
#include "hardware_interface.h"
#include "time_interface.h"
#include "time_manager.h"
#include "time_hst.h"

std::shared_ptr<Command::Scheduler> scheduler;

class TimeInterfaceSim: public Time::Interface {
  public:
 
  Time::RealTimeS secondsSince1970() override {
    return Time::RealTimeS(time(nullptr));
  } 

};

class SimTimeHST: public Time::HST 
{
  public:
  SimTimeHST() 
  {
    startTime = Time::DeviceTimeMS(0);
    startTime = msSinceDeviceStart();    // side effects.
    startTimeUs = Time::DeviceTimeUS(0);
    startTimeUs = usSinceDeviceStart();    // side effects.
  }

  Time::DeviceTimeMS msSinceDeviceStart() override 
  {
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t );
    const unsigned int msFromS  = t.tv_sec * 1000;
    const unsigned int msFromNs = t.tv_nsec / 1000000;
    return Time::DeviceTimeMS( msFromS + msFromNs - startTime.get() ); 
  }

  Time::DeviceTimeUS usSinceDeviceStart() override 
  {
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t );
    const unsigned long int usFromS  = t.tv_sec * 1000000;
    const unsigned long int usFromNs = t.tv_nsec / 1000;
    return Time::DeviceTimeUS( usFromS + usFromNs - startTimeUs.get() ); 
  }

  virtual Time::TimeUS execute() override final
  {
    return Time::TimeUS( 1000000 );
  }

  const char* debugName() override final {
    return "Sim High Speed Timer";
  }

  private:

  Time::DeviceTimeMS startTime;
  Time::DeviceTimeUS startTimeUs;
};

class NetConnectionSim: public NetConnection {
  bool getString( std::string& string ) {
    string = "";
    return true;
  }
  operator bool(void) {
    return true;
  }
  void reset(void )
  {
  }
  std::streamsize write( const char_type* s, std::streamsize n )
  {
    for ( std::streamsize i = 0; i < n; ++i ) {
      std::cout << s[i];
    }
    return n;
  }
  void flush()
  {
  }
};

class NetInterfaceSim: public NetInterface {
  public:

  struct category: virtual beefocus_tag {};
  using char_type = char;

  NetInterfaceSim( std::shared_ptr<DebugInterface> debugLog )
  {
    (*debugLog) << "Simulator Net Interface Init\n";
  }
  bool getString( std::string& input ) override
  {
    fd_set readfds;
    FD_ZERO(&readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_SET(STDIN_FILENO, &readfds );
    if ( select(1, &readfds, nullptr, nullptr, &timeout ))
    {
      std::cin >> input;
      return true;
    }
    input = "";
    return false;
  }
  std::streamsize write( const char_type* s, std::streamsize n ) override
  {
    for ( std::streamsize i = 0; i < n; ++i ) {
      std::cout << s[i];
    }
    return n;
  }
  void flush() override
  {
  }
  const char* debugName() override { return "NetInterfaceSim"; }
  Time::TimeUS execute() override {
    return Time::TimeUS( 5 * Time::USPerS );
  }
  std::unique_ptr<NetConnection> connect( const std::string& location, unsigned int port ) override
  {
    return std::unique_ptr<NetConnection>(new NetConnectionSim());  
  }
};

class HWISim: public HWI
{
  public: 

  void PinMode( Pin pin, PinIOMode mode ) override
  {
    std::cout << "PM (" << HWI::pinNames.at(pin) << ") = " << HWI::pinIOModeNames.at(mode) << "\n";
  }
  void DigitalWrite( Pin pin, PinState state ) override
  {
    const std::string name = HWI::pinNames.at(pin);
    std::cout << "DW (" << HWI::pinNames.at(pin) 
              << ") = " << HWI::pinStateNames.at( state ) 
              << "\n";
  }
  PinState DigitalRead( Pin pin ) override
  {
    std::cout << "DR " << HWI::pinNames.at(pin) << " returning INPUT_LOW";
    return HWI::PinState::INPUT_LOW;
  }
  unsigned AnalogRead( Pin pin ) override
  {
    static unsigned int count = 0;
    int count_pos = ((count / 2 ) & 0xfff )/256;   // Range 0 - 0xff
    int count_amp = (count & 1) ? count_pos : -count_pos ;
    count++;

    return 200 + count_amp;
  }
};

class DebugInterfaceSim: public DebugInterface
{
  struct category: virtual beefocus_tag {};
  using char_type = char;

  std::streamsize write( const char_type* s, std::streamsize n ) override
  {
    // Ignore for now.
    return n;
  }
  void disable() override 
  {
    // Can't disable what we're ignoring.
  }
};

Time::TimeUS loop() {
  return scheduler->execute();
}

void setup() {
  auto debug      = std::make_shared<DebugInterfaceSim>();
  auto wifi       = std::make_shared<NetInterfaceSim>( debug );
  auto hardware   = std::make_shared<HWISim>();
  auto hst        = std::make_shared<SimTimeHST>();

  scheduler = std::make_shared<Command::Scheduler>( 
                          wifi, hardware, debug, hst );

  auto timeSim    = std::make_shared<TimeInterfaceSim>();
  auto time       = std::make_shared<Time::Manager>( timeSim, hst );
  auto motorSim   = std::make_shared<Command::Motor>( 
                          hardware, debug, wifi, 
                          HWI::Pin::MOTOR0_PIN0, HWI::Pin::MOTOR0_PIN1 );
  auto encoderSim = std::make_shared<Command::Encoder>(
                          hardware, debug, wifi, 
                          HWI::Pin::ENCODER0_PIN0, HWI::Pin::ENCODER0_PIN1);
 
  auto commandProcessor= std::make_shared<Command::ProcessCommand>( 
                          wifi, hardware, debug, 
                          time, motorSim , encoderSim,
                          hst,
                          scheduler );

  scheduler->addCommand( commandProcessor );
  scheduler->addCommand( time );
  scheduler->addCommand( hst );
  scheduler->addCommand( motorSim );
  scheduler->addCommand( wifi );
}

int main(int argc, char* argv[])
{
  setup();
  for ( ;; ) 
  {
    Time::TimeUS delay = loop();
    usleep( delay.get() );
  }
}

