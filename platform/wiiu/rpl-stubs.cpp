#include <coreinit/dynload.h>
#include <coreinit/atomic64.h>
#include <stdbool.h>
#include <stdint.h>

#include "wiiu-stderr.hpp"
#include "wiiu-stdout.hpp"

static bool moduleLogInit = 0;
static bool cafeLogInit;
static bool udpLogInit;

extern "C" uint64_t __atomic_fetch_add_8(volatile void* ptr, uint64_t val, int memorder)
{
    return OSAddAtomic64((volatile int64_t*)ptr, val);
}

extern "C" int
rpl_entry(OSDynLoad_Module module, OSDynLoad_EntryReason reason) {
   if (reason == OS_DYNLOAD_LOADED) {
      wiiu_init_stdout();
      wiiu_init_stderr();
   } else if (reason == OS_DYNLOAD_UNLOADED) {
      wiiu_fini_whb_log();
   }

   return 0;
}