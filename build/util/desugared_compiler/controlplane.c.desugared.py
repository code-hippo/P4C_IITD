# Copyright 2016 Eotvos Lorand University, Budapest, Hungary
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import p4_hlir.hlir.p4 as p4
from utils.hlir import fld_prefix, fld_id, getTypeAndLength 

def match_type_order(t):
    if t is p4.p4_match_type.P4_MATCH_EXACT:   return 0
    if t is p4.p4_match_type.P4_MATCH_LPM:     return 1
    if t is p4.p4_match_type.P4_MATCH_TERNARY: return 2

generated_code += " #include \"dpdk_lib.h\"// sugar@22\n"
generated_code += " #include \"actions.h\"// sugar@23\n"
generated_code += " \n"
generated_code += " extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@25\n"
generated_code += " extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@26\n"
generated_code += " extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@27\n"
generated_code += " extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@28\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    generated_code += " extern void table_" + str(table.name) + "_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@31\n"
generated_code += "\n"

generated_code += " uint8_t reverse_buffer[" + str(max([t[1] for t in map(getTypeAndLength, hlir.p4_tables.values())])) + "];// sugar@34\n"

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " void// sugar@38\n"
    generated_code += " " + str(table.name) + "_add(// sugar@39\n"
    for match_field, match_type, match_mask in table.match_fields:
        byte_width = (match_field.width+7)/8
        generated_code += " uint8_t " + str(fld_id(match_field)) + "[" + str(byte_width) + "],// sugar@42\n"
        ###if match_type is p4_match_type.P4_MATCH_EXACT:
        if match_type is p4.p4_match_type.P4_MATCH_TERNARY:
            generated_code += " uint8_t " + str(fld_id(match_field)) + "_mask[" + str(byte_width) + "],// sugar@45\n"
        if match_type is p4.p4_match_type.P4_MATCH_LPM:
            generated_code += " uint8_t " + str(fld_id(match_field)) + "_prefix_length,// sugar@47\n"
    generated_code += " struct " + str(table.name) + "_action action)// sugar@48\n"
    generated_code += " {// sugar@49\n"
    generated_code += "     uint8_t key[" + str(key_length) + "];// sugar@50\n"
    sortedfields = sorted(table.match_fields, key=lambda field: match_type_order(field[1]))
    k = 0
    for match_field, match_type, match_mask in sortedfields:
        byte_width = (match_field.width+7)/8
        generated_code += " memcpy(key+" + str(k) + ", " + str(fld_id(match_field)) + ", " + str(byte_width) + ");// sugar@55\n"
        k += byte_width;
    if table_type == "LOOKUP_LPM":
        generated_code += " uint8_t prefix_length = 0;// sugar@58\n"
        for match_field, match_type, match_mask in table.match_fields:
            byte_width = (match_field.width+7)/8
            if match_type is p4.p4_match_type.P4_MATCH_EXACT:
                generated_code += " prefix_length += " + str(match_field.width) + ";// sugar@62\n"
            if match_type is p4.p4_match_type.P4_MATCH_LPM:
                generated_code += " prefix_length += " + str(fld_id(match_field)) + "_prefix_length;// sugar@64\n"
        generated_code += " int c, d;// sugar@65\n"
        generated_code += " for(c = " + str(k-1) + ", d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);// sugar@66\n"
        generated_code += " for(c = 0; c < " + str(k) + "; c++) *(key+c) = *(reverse_buffer+c);// sugar@67\n"
        generated_code += " lpm_add_promote(TABLE_" + str(table.name) + ", (uint8_t*)key, prefix_length, (uint8_t*)&action);// sugar@68\n"
    if table_type == "LOOKUP_EXACT":
        for match_field, match_type, match_mask in table.match_fields:
            byte_width = (match_field.width+7)/8
        generated_code += " exact_add_promote(TABLE_" + str(table.name) + ", (uint8_t*)key, (uint8_t*)&action);// sugar@72\n"
    generated_code += " }// sugar@73\n"
    generated_code += "\n"
    generated_code += " void// sugar@75\n"
    generated_code += " " + str(table.name) + "_setdefault(struct " + str(table.name) + "_action action)// sugar@76\n"
    generated_code += " {// sugar@77\n"
    generated_code += "     table_setdefault_promote(TABLE_" + str(table.name) + ", (uint8_t*)&action);// sugar@78\n"
    generated_code += " }// sugar@79\n"


for table in hlir.p4_tables.values():
    generated_code += " void// sugar@83\n"
    generated_code += " " + str(table.name) + "_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@84\n"
    for i, m in enumerate(table.match_fields):
        match_field, match_type, match_mask = m
        if match_type is p4.p4_match_type.P4_MATCH_EXACT:
            generated_code += " uint8_t* " + str(fld_id(match_field)) + " = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[" + str(i) + "])->bitmap);// sugar@88\n"
        if match_type is p4.p4_match_type.P4_MATCH_LPM:
            generated_code += " uint8_t* " + str(fld_id(match_field)) + " = (uint8_t*)(((struct p4_field_match_lpm*)ctrl_m->field_matches[" + str(i) + "])->bitmap);// sugar@90\n"
            generated_code += " uint16_t " + str(fld_id(match_field)) + "_prefix_length = ((struct p4_field_match_lpm*)ctrl_m->field_matches[" + str(i) + "])->prefix_length;// sugar@91\n"
    for action in table.actions:
        generated_code += " if(strcmp(\"" + str(action.name) + "\", ctrl_m->action_name)==0) {// sugar@93\n"
        generated_code += "     struct " + str(table.name) + "_action action;// sugar@94\n"
        generated_code += "     action.action_id = action_" + str(action.name) + ";// sugar@95\n"
        for j, (name, length) in enumerate(zip(action.signature, action.signature_widths)):
            generated_code += " uint8_t* " + str(name) + " = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[" + str(j) + "])->bitmap;// sugar@97\n"
            generated_code += " memcpy(action." + str(action.name) + "_params." + str(name) + ", " + str(name) + ", " + str((length+7)/8) + ");// sugar@98\n"
        generated_code += "     debug(\"Reply from the control plane arrived.\\n\");// sugar@99\n"
        generated_code += "     debug(\"Addig new entry to " + str(table.name) + " with action " + str(action.name) + "\\n\");// sugar@100\n"
        generated_code += "     " + str(table.name) + "_add(// sugar@101\n"
        for m in table.match_fields:
            match_field, match_type, match_mask = m
            generated_code += " " + str(fld_id(match_field)) + ",// sugar@104\n"
            if match_type is p4.p4_match_type.P4_MATCH_LPM:
                generated_code += " " + str(fld_id(match_field)) + "_prefix_length,// sugar@106\n"
        generated_code += "     action);// sugar@107\n"
        generated_code += " } else// sugar@108\n"
    generated_code += " debug(\"Table add entry: action name mismatch (%s).\\n\", ctrl_m->action_name);// sugar@109\n"
    generated_code += " }// sugar@110\n"

for table in hlir.p4_tables.values():
    generated_code += " void// sugar@113\n"
    generated_code += " " + str(table.name) + "_set_default_table_action(struct p4_ctrl_msg* ctrl_m) {// sugar@114\n"
    generated_code += " debug(\"Action name: %s\\n\", ctrl_m->action_name);// sugar@115\n"
    for action in table.actions:
        generated_code += " if(strcmp(\"" + str(action.name) + "\", ctrl_m->action_name)==0) {// sugar@117\n"
        generated_code += "     struct " + str(table.name) + "_action action;// sugar@118\n"
        generated_code += "     action.action_id = action_" + str(action.name) + ";// sugar@119\n"
        for j, (name, length) in enumerate(zip(action.signature, action.signature_widths)):
            generated_code += " uint8_t* " + str(name) + " = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[" + str(j) + "])->bitmap;// sugar@121\n"
            generated_code += " memcpy(action." + str(action.name) + "_params." + str(name) + ", " + str(name) + ", " + str((length+7)/8) + ");// sugar@122\n"
        generated_code += "     debug(\"Message from the control plane arrived.\\n\");// sugar@123\n"
        generated_code += "     debug(\"Set default action for " + str(table.name) + " with action " + str(action.name) + "\\n\");// sugar@124\n"
        generated_code += "     " + str(table.name) + "_setdefault( action );// sugar@125\n"
        generated_code += " } else// sugar@126\n"
    generated_code += " debug(\"Table setdefault: action name mismatch (%s).\\n\", ctrl_m->action_name);// sugar@127\n"
    generated_code += " }// sugar@128\n"


generated_code += " void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@131\n"
generated_code += "     debug(\"MSG from controller %d %s\\n\", ctrl_m->type, ctrl_m->table_name);// sugar@132\n"
generated_code += "     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@133\n"
for table in hlir.p4_tables.values():
    generated_code += " if (strcmp(\"" + str(table.name) + "\", ctrl_m->table_name) == 0)// sugar@135\n"
    generated_code += "     " + str(table.name) + "_add_table_entry(ctrl_m);// sugar@136\n"
    generated_code += " else// sugar@137\n"
generated_code += " debug(\"Table add entry: table name mismatch (%s).\\n\", ctrl_m->table_name);// sugar@138\n"
generated_code += "     }// sugar@139\n"
generated_code += "     else if (ctrl_m->type == P4T_SET_DEFAULT_ACTION) {// sugar@140\n"
for table in hlir.p4_tables.values():
    generated_code += " if (strcmp(\"" + str(table.name) + "\", ctrl_m->table_name) == 0)// sugar@142\n"
    generated_code += "     " + str(table.name) + "_set_default_table_action(ctrl_m);// sugar@143\n"
    generated_code += " else// sugar@144\n"
generated_code += " debug(\"Table setdefault: table name mismatch (%s).\\n\", ctrl_m->table_name);// sugar@145\n"
generated_code += "     }// sugar@146\n"
generated_code += " }// sugar@147\n"



generated_code += " backend bg;// sugar@151\n"
generated_code += " void init_control_plane()// sugar@152\n"
generated_code += " {// sugar@153\n"
generated_code += "     debug(\"Creating control plane connection...\\n\");// sugar@154\n"
generated_code += "     bg = create_backend(3, 1000, \"localhost\", 11111, recv_from_controller);// sugar@155\n"
generated_code += "     launch_backend(bg);// sugar@156\n"
generated_code += " /*// sugar@157\n"
if "smac" in hlir.p4_tables:
    generated_code += " struct smac_action action;// sugar@159\n"
    generated_code += " action.action_id = action_mac_learn;// sugar@160\n"
    generated_code += " smac_setdefault(action);// sugar@161\n"
    generated_code += " debug(\"smac setdefault\\n\");// sugar@162\n"
if "dmac" in hlir.p4_tables:
    generated_code += " struct dmac_action action2;// sugar@164\n"
    generated_code += " action2.action_id = action_bcast;// sugar@165\n"
    generated_code += " dmac_setdefault(action2);// sugar@166\n"
    generated_code += " debug(\"dmac setdefault\\n\");// sugar@167\n"
generated_code += " */// sugar@168\n"
generated_code += " }// sugar@169\n"