#include "UPFProgram.h"
#include <SessionManager.h>
#include <bpf/bpf.h>       // bpf calls
#include <bpf/libbpf.h>    // bpf wrappers
#include <iostream>        // cout
#include <stdexcept>       // exception
#include <utils/LogDefines.h> 
#include <wrappers/BPFMap.hpp>
#include <wrappers/BPFMaps.h>

UPFProgram::UPFProgram()
{
  LOG_FUNC();
  mpLifeCycle = std::make_unique<UPFProgramLifeCycle>(upf_xdp_bpf_c__open, upf_xdp_bpf_c__load, upf_xdp_bpf_c__attach, upf_xdp_bpf_c__destroy);
}

UPFProgram::~UPFProgram()
{
  LOG_FUNC();
}

void UPFProgram::setup()
{
  LOG_FUNC();

  spSkeleton = mpLifeCycle->open();
  initializeMaps();
  mpLifeCycle->load();
  mpLifeCycle->attach();
  // Uplink and Downlink interface
  mpLifeCycle->link("xdp_entry_point","enp0s20f0u4u2u4");

  // TODO navarrothiago - split to a different downlink interface.
  // TODO navarrothiago - Remove hardcoded - https://github.com/navarrothiago/upf-bpf/issues/24
}

std::shared_ptr<BPFMaps> UPFProgram::getMaps()
{
  LOG_FUNC();
  return mpMaps;
}

// TODO navarrothiago - check when kill when running.
// It was noted the infinity loop.
void UPFProgram::tearDown()
{
  LOG_FUNC();
  mpLifeCycle->tearDown();
}

void UPFProgram::updateProgramMap(uint32_t key, uint32_t fd)
{
  LOG_FUNC();
  mpTeidSessionMap->update(key, fd, BPF_ANY);
}

void UPFProgram::removeProgramMap(uint32_t key)
{
  LOG_FUNC();
  s32 fd;
  // Remove only if exists.
  if(mpTeidSessionMap->lookup(key, &fd) == 0) {
    mpTeidSessionMap->remove(key);
  }
}

std::shared_ptr<BPFMap> UPFProgram::getTeidSessionMap() const
{
  LOG_FUNC();
  return mpTeidSessionMap;
}

std::shared_ptr<BPFMap> UPFProgram::getUeIpSessionMap() const
{
  LOG_FUNC();
  return mpUeIpSessionMap;
}

void UPFProgram::initializeMaps()
{
  LOG_FUNC();
  // Store all maps available in the program.
  mpMaps = std::make_unique<BPFMaps>(mpLifeCycle->getBPFSkeleton()->skeleton);

  // Warning - The name of the map must be the same of the BPF program.
  mpTeidSessionMap = std::make_shared<BPFMap>(mpMaps->getMap("m_teid_session"));
  mpUeIpSessionMap = std::make_shared<BPFMap>(mpMaps->getMap("m_ueip_session"));
}