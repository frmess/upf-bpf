#ifndef __UPFPROGRAMMANAGER_H__
#define __UPFPROGRAMMANAGER_H__

#include <atomic>
#include <linux/bpf.h> // manage maps (e.g. bpf_update*)
#include <memory>
#include <mutex>
#include <signal.h> // signals
#include <upf_xdp_bpf_skel.h>
#include <utils/LogDefines.h>

class BPFMaps;
/**
 * @brief Singleton class to abrastract the UPF bpf program.
 */
class UPFProgramManager
{
public:
  /**
   * @brief Program states.
   */
  enum ProgramState {
    IDLE,
    OPENED,
    LOADED,
    ATTACHED,
    LINKED,
    DESTROYED,
  };

  /**
   * @brief Destroy the UPFProgramManager object
   */
  virtual ~UPFProgramManager();
  /**
   * @brief Setup the BPF program.
   *
   */
  void setup();
  /**
   * @brief Get the Skeleton object.
   *
   * @return upf_xdp_bpf_c*  The skeleton generated by bpftoo.
   */
  upf_xdp_bpf_c *getSkeleton();
  /**
   * @brief Get the Instance object.
   *
   * @return UPFProgramManager& The singleton instance.
   */
  static UPFProgramManager &getInstance();
  /**
   * @brief Get the BPFMaps object.
   *
   * @return std::shared_ptr<BPFMaps> The reference of the BPFMaps.
   */
  std::shared_ptr<BPFMaps> getMaps();
  /**
   * @brief Tear downs the BPF program.
   * It is also used as signal callback.
   *
   * @param signal  The received signal
   */
  static void tearDown(int signal = SIGTERM);

  typedef struct {
    unsigned int programId;          //!< The program identifier.
    unsigned int programFd;          //!< The program file descriptor.
    unsigned int xdpFlag;            //!< The XDP flag used in link call.
    std::atomic<ProgramState> state; //!< The program state.
    struct bpf_prog_info info = {};  //!< The BPF program information.
    __u32 info_len = sizeof(info);   //!< The size of the information struct.
    int ifIndex;                     //!< The interface index.
  } XDPProgramInfo;

private:
  /**
   * @brief Construct a new UPFProgramManager object.
   *
   */
  explicit UPFProgramManager();
  /**
   * @brief Open the BPF program.
   *
   */
  void open();
  /**
   * @brief Load the BPF program.
   *
   */
  void load();
  /**
   * @brief Attach the BPF program.
   *
   */
  void attach();
  /**
   * @brief Destroy the BPF program.
   *
   */
  static void destroy();

  // The reference of the bpf maps.
  std::shared_ptr<BPFMaps> mpMaps;

  // The skeleton of the UPF program generated by bpftool.
  static upf_xdp_bpf_c *spSkeleton;

  static XDPProgramInfo sXDPInfo[2];

  static std::mutex sTearDownMutex;
  static std::atomic<ProgramState> sState;
};

#endif // __BPFPROGRAMMANAGER_H__
