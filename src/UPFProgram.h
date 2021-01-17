#ifndef __UPFPROGRAMMANAGER_H__
#define __UPFPROGRAMMANAGER_H__

#include <atomic>
#include <linux/bpf.h> // manage maps (e.g. bpf_update*)
#include <memory>
#include <mutex>
#include <signal.h> // signals
#include <upf_xdp_bpf_skel.h>
#include <utils/LogDefines.h>
#include <wrappers/BPFMap.hpp>
#include <ProgramLifeCycle.hpp>

class BPFMaps;
class SessionManager;
class RulesUtilities;

using UPFProgramLifeCycle = ProgramLifeCycle<upf_xdp_bpf_c>;

/**
 * @brief Singleton class to abrastract the UPF bpf program.
 */
class UPFProgram
{
public:
  /**
   * @brief Construct a new UPFProgram object.
   *
   */
  explicit UPFProgram();
  /**
   * @brief Destroy the UPFProgram object
   */
  virtual ~UPFProgram();
  /**
   * @brief Setup the BPF program.
   *
   */
  void setup();
  /**
   * @brief Get the BPFMaps object.
   *
   * @return std::shared_ptr<BPFMaps> The reference of the BPFMaps.
   */
  std::shared_ptr<BPFMaps> getMaps();
  /**
   * @brief Tear downs the BPF program.
   *
   */
  void tearDown();

  typedef struct {
    unsigned int programId;          //!< The program identifier.
    unsigned int programFd;          //!< The program file descriptor.
    unsigned int xdpFlag;            //!< The XDP flag used in link call.
    std::atomic<ProgramState> state; //!< The program state.
    struct bpf_prog_info info = {};  //!< The BPF program information.
    __u32 info_len = sizeof(info);   //!< The size of the information struct.
    int ifIndex;                     //!< The interface index.
  } XDPProgramInfo;

  /**
   * @brief Update program int map.
   * 
   * @param key The key which will be inserted the program file descriptor.
   * @param fd The file descriptor.
   */
  void updateProgramMap(uint32_t key, uint32_t fd);
  /**
   * @brief Remove program in map.
   * 
   * @param key The key which will be remove in the program map.
   */
  void removeProgramMap(uint32_t key);
  /**
   * @brief Get the Sessions Map object.
   * 
   * @return std::shared_ptr<BPFMap> The seid to session map.
   */
  std::shared_ptr<BPFMap> getSessionsMap() const;
  /**
   * @brief Get the Uplink PDRs Map object.
   * 
   * @return std::shared_ptr<BPFMap> The uplink to PDR map.
   */
  std::shared_ptr<BPFMap> getUplinkPDRsMap() const;
  /**
   * @brief Get the Programs Map object.
   * 
   * @return std::shared_ptr<BPFMap> The seid to fd map.
   */
  std::shared_ptr<BPFMap> getProgramsMap() const;

private:
  /**
   * @brief Initialize BPF wrappers maps.
   * 
   */
  void initializeMaps();

  // The reference of the bpf maps.
  std::shared_ptr<BPFMaps> mpMaps;

  // The skeleton of the UPF program generated by bpftool.
  // ProgramLifeCycle is the owner of the pointer.
  upf_xdp_bpf_c* spSkeleton;

  // XDP program information.
  XDPProgramInfo sXDPInfo[2];

  // Mutex for tearDown (async).
  std::mutex sTearDownMutex;

  // The PFCP session eBPF map.
  std::shared_ptr<BPFMap> mpSessionsMap;

  // The uplink PDR eBPF map.
  std::shared_ptr<BPFMap> mpUplinkPDRsMap;

  // The program eBPF map.
  std::shared_ptr<BPFMap> mpProgramsMap;

  // The BPF lifecycle program. 
  std::unique_ptr<UPFProgramLifeCycle> mpLifeCycle;
};

#endif // __BPFPROGRAMMANAGER_H__
