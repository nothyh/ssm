#ifndef SSM_INCLUDE_STD_LIB_H
#define SSM_INCLUDE_STD_LIB_H
#include "mcu.h"
#include "pack.h"
class StdLib : public Pack {
  const AllConfig &all_config;
  StdLib(const AllConfig &all_config_ref);
  void unzip() override;
};
#endif