# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\diskio.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\ff.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\ffconf.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\sleep.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xilffs.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xilffs_config.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xilrsa.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xiltimer.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xtimer_config.h"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\lib\\libxilffs.a"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\lib\\libxilrsa.a"
  "D:\\Homework_10\\LED_AXI_Zynq\\platformV1_2\\zynq_fsbl\\zynq_fsbl_bsp\\lib\\libxiltimer.a"
  )
endif()
