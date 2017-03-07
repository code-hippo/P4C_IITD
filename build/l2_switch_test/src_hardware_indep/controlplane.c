 #include "dpdk_lib.h"// sugar@22
 #include "actions.h"// sugar@23
 
 extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@25
 extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@26
 extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@27
 extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@28

 extern void table_smac_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31
 extern void table_dmac_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31

 uint8_t reverse_buffer[6];// sugar@34
 void// sugar@38
 smac_add(// sugar@39
 uint8_t field_instance_ethernet_srcAddr[6],// sugar@42
 struct smac_action action)// sugar@48
 {// sugar@49
     uint8_t key[6];// sugar@50
 memcpy(key+0, field_instance_ethernet_srcAddr, 6);// sugar@55
 exact_add_promote(TABLE_smac, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 smac_setdefault(struct smac_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_smac, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@38
 dmac_add(// sugar@39
 uint8_t field_instance_ethernet_dstAddr[6],// sugar@42
 struct dmac_action action)// sugar@48
 {// sugar@49
     uint8_t key[6];// sugar@50
 memcpy(key+0, field_instance_ethernet_dstAddr, 6);// sugar@55
 exact_add_promote(TABLE_dmac, (uint8_t*)key, (uint8_t*)&action);// sugar@72
 }// sugar@73

 void// sugar@75
 dmac_setdefault(struct dmac_action action)// sugar@76
 {// sugar@77
     table_setdefault_promote(TABLE_dmac, (uint8_t*)&action);// sugar@78
 }// sugar@79
 void// sugar@83
 smac_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_ethernet_srcAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@93
     struct smac_action action;// sugar@94
     action.action_id = action_mac_learn;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to smac with action mac_learn\n");// sugar@100
     smac_add(// sugar@101
 field_instance_ethernet_srcAddr,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@93
     struct smac_action action;// sugar@94
     action.action_id = action__nop;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to smac with action _nop\n");// sugar@100
     smac_add(// sugar@101
 field_instance_ethernet_srcAddr,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@83
 dmac_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84
 uint8_t* field_instance_ethernet_dstAddr = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[0])->bitmap);// sugar@88
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@93
     struct dmac_action action;// sugar@94
     action.action_id = action_forward;// sugar@95
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@97
 memcpy(action.forward_params.port, port, 2);// sugar@98
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to dmac with action forward\n");// sugar@100
     dmac_add(// sugar@101
 field_instance_ethernet_dstAddr,// sugar@104
     action);// sugar@107
 } else// sugar@108
 if(strcmp("bcast", ctrl_m->action_name)==0) {// sugar@93
     struct dmac_action action;// sugar@94
     action.action_id = action_bcast;// sugar@95
     debug("Reply from the control plane arrived.\n");// sugar@99
     debug("Addig new entry to dmac with action bcast\n");// sugar@100
     dmac_add(// sugar@101
 field_instance_ethernet_dstAddr,// sugar@104
     action);// sugar@107
 } else// sugar@108
 debug("Table add entry: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@109
 }// sugar@110
 void// sugar@113
 smac_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("mac_learn", ctrl_m->action_name)==0) {// sugar@117
     struct smac_action action;// sugar@118
     action.action_id = action_mac_learn;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for smac with action mac_learn\n");// sugar@124
     smac_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("_nop", ctrl_m->action_name)==0) {// sugar@117
     struct smac_action action;// sugar@118
     action.action_id = action__nop;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for smac with action _nop\n");// sugar@124
     smac_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void// sugar@113
 dmac_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114
 debug("Action name: %s\n", ctrl_m->action_name);// sugar@115
 if(strcmp("forward", ctrl_m->action_name)==0) {// sugar@117
     struct dmac_action action;// sugar@118
     action.action_id = action_forward;// sugar@119
 uint8_t* port = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;// sugar@121
 memcpy(action.forward_params.port, port, 2);// sugar@122
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for dmac with action forward\n");// sugar@124
     dmac_setdefault( action );// sugar@125
 } else// sugar@126
 if(strcmp("bcast", ctrl_m->action_name)==0) {// sugar@117
     struct dmac_action action;// sugar@118
     action.action_id = action_bcast;// sugar@119
     debug("Message from the control plane arrived.\n");// sugar@123
     debug("Set default action for dmac with action bcast\n");// sugar@124
     dmac_setdefault( action );// sugar@125
 } else// sugar@126
 debug("Table setdefault: action name mismatch (%s).\n", ctrl_m->action_name);// sugar@127
 }// sugar@128
 void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@131
     debug("MSG from controller %d %s\n", ctrl_m->type, ctrl_m->table_name);// sugar@132
     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@133
 if (strcmp("smac", ctrl_m->table_name) == 0)// sugar@135
     smac_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 if (strcmp("dmac", ctrl_m->table_name) == 0)// sugar@135
     dmac_add_table_entry(ctrl_m);// sugar@136
 else// sugar@137
 debug("Table add entry: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@138
     }// sugar@139
     else if (ctrl_m->type == P4T_SET_DEFAULT_ACTION) {// sugar@140
 if (strcmp("smac", ctrl_m->table_name) == 0)// sugar@142
     smac_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 if (strcmp("dmac", ctrl_m->table_name) == 0)// sugar@142
     dmac_set_default_table_action(ctrl_m);// sugar@143
 else// sugar@144
 debug("Table setdefault: table name mismatch (%s).\n", ctrl_m->table_name);// sugar@145
     }// sugar@146
 }// sugar@147
 backend bg;// sugar@151
 void init_control_plane()// sugar@152
 {// sugar@153
     debug("Creating control plane connection...\n");// sugar@154
     bg = create_backend(3, 1000, "localhost", 11111, recv_from_controller);// sugar@155
     launch_backend(bg);// sugar@156
 /*// sugar@157
 struct smac_action action;// sugar@159
 action.action_id = action_mac_learn;// sugar@160
 smac_setdefault(action);// sugar@161
 debug("smac setdefault\n");// sugar@162
 struct dmac_action action2;// sugar@164
 action2.action_id = action_bcast;// sugar@165
 dmac_setdefault(action2);// sugar@166
 debug("dmac setdefault\n");// sugar@167
 */// sugar@168
 }// sugar@169
