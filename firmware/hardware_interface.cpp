#include "hardware_interface.h"

const std::unordered_map<HWI::Pin,std::string,EnumHash> HWI::pinNames = {
    { Pin::MOTOR0_PIN0,     "Motor0 Pin0" },
    { Pin::MOTOR0_PIN1,     "Motor0 Pin1" },
    { Pin::MOTOR1_PIN0,     "Motor1 Pin0" },
    { Pin::MOTOR1_PIN1,     "Motor1 Pin1" },
    { Pin::ENCODER0_PIN0,   "Encoder0 Pin0" },
    { Pin::ENCODER0_PIN1,   "Encoder0 Pin1" },
    { Pin::ENCODER1_PIN0,   "Encoder1 Pin0" },
    { Pin::ENCODER1_PIN1,   "Encoder1 Pin1" },
};

const std::unordered_map<HWI::PinState,std::string,EnumHash> HWI::pinStateNames = {
  { PinState::MOTOR_POS,            "Voltage On"      },
  { PinState::MOTOR_NEG,            "Voltage Off"     },
  { PinState::INPUT_LOW,            "Input GND"       },
  { PinState::INPUT_HIGH,           "Input VCC"       }
};

const std::unordered_map<HWI::PinIOMode,std::string,EnumHash> HWI::pinIOModeNames = {
    { PinIOMode::M_INPUT,        "Input" },
    { PinIOMode::M_OUTPUT,       "Output" }
};

