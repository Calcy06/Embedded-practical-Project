#ifndef __MODBUS_RTU_DEBUG_H__
#define __MODBUS_RTU_DEBUG_H__

#include <libdebug/libdebug.h>

extern dbg_md_t g_main_id;

/* debug宏定�?*/
#define RTU_LOG(fmt, arg...) do { \
    dbg_logfile(g_main_id, "[%s %d]: " fmt,__FUNCTION__, __LINE__,##arg);\
} while (0)

#define RTU_DEBUG(fmt, arg...) do { \
    dbg_printf(g_main_id, DBG_LV_DEBUG, fmt, ##arg);\
} while (0)

#define RTU_WARNING(fmt, arg...) do { \
    dbg_printf(g_main_id, DBG_LV_WARNING, "WARNING in %s [%d]: "fmt, __FILE__, __LINE__, ##arg);\
} while (0)

#define RTU_ERROR(fmt, arg...) do { \
    dbg_printf(g_main_id, DBG_LV_ERROR, "ERROR in %s [%d]: "fmt, __FILE__, __LINE__, ##arg);\
} while (0)


#define DEBUG_PRINT(fmt, arg...)                \
    do                                      \
    {                                       \
        dbg_logfile(g_md_id, "[%s %d]: " fmt,__FUNCTION__, __LINE__,##arg); \
    } while (0)

#endif  /* __MODBUS_RTU_DEBUG_H__ */   