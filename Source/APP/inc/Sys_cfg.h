#ifndef _SYS_CFG_H_
#define _SYS_CFG_H_

/* REMAP宏是作为是否支持IAP功能的开关，如果支持，则该宏要大于0。
另外，如果REMAP大于0，那么需要在编译器link选型哪里设置
vector table 地址为8003000，memory region 中的rom地址从8003000开始*/
#define REMAP    1

/* 看门狗开关 */
#define IWDG_EN  0

/*ID长度选择，旧的设备ID是12位的，新的设备ID是11位的
则使用11位ID。*/
#define DEFAULT_DEV_ID_LEN		11

/*倾角传感器开关*/
#define MMA8652 0

#endif

