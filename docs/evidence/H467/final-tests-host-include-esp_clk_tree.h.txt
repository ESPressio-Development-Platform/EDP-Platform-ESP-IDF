#pragma once
#include <stdint.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
typedef enum { SOC_MOD_CLK_XTAL=1, SOC_MOD_CLK_APB=2 } soc_module_clk_t;
typedef enum { ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED=0 } esp_clk_tree_src_freq_precision_t;
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t esp_clk_tree_src_get_freq_hz(soc_module_clk_t,esp_clk_tree_src_freq_precision_t,uint32_t*);
#ifdef __cplusplus
}
#endif
