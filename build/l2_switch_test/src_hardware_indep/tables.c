 #include "dataplane.h"// sugar@17
 #include "actions.h"// sugar@18
 #include "data_plane_data.h"// sugar@19

 lookup_table_t table_config[NB_TABLES] = {// sugar@21
 {// sugar@24
  .name= "smac",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 6,// sugar@27
  .val_size = sizeof(struct smac_action),// sugar@28
  .min_size = 0, //512,// sugar@29
  .max_size = 255 //512// sugar@30
 },// sugar@31
 {// sugar@24
  .name= "dmac",// sugar@25
  .type = LOOKUP_EXACT,// sugar@26
  .key_size = 6,// sugar@27
  .val_size = sizeof(struct dmac_action),// sugar@28
  .min_size = 0, //512,// sugar@29
  .max_size = 255 //512// sugar@30
 },// sugar@31
 };// sugar@32
 counter_t counter_config[NB_COUNTERS] = {// sugar@34
 };// sugar@49
