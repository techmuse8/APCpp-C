#include <coreinit/dynload.h>
#include <coreinit/atomic64.h>
#include <stdbool.h>
#include <stdint.h>

#include <whb/log.h>
#include <whb/log_module.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>

static bool moduleLogInit = 0;
static bool cafeLogInit;
static bool udpLogInit;

extern "C" uint64_t __atomic_fetch_add_8(volatile void* ptr, uint64_t val, int memorder)
{
    return OSAddAtomic64((volatile int64_t*)ptr, val);
}

void initLogging()
{
   if (!(moduleLogInit = WHBLogModuleInit()))
   {
      cafeLogInit = WHBLogCafeInit();
      udpLogInit = WHBLogUdpInit();
   }
}

extern "C" int
rpl_entry(OSDynLoad_Module module, OSDynLoad_EntryReason reason) {
   if (reason == OS_DYNLOAD_LOADED) {
      initLogging();
   } else if (reason == OS_DYNLOAD_UNLOADED) {
      // Do stuff on unload
   }

   return 0;
}