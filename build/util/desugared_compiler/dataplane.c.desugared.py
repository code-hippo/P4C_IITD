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
from utils.hlir import *  

generated_code += " #include <stdlib.h>// sugar@17\n"
generated_code += " #include <string.h>// sugar@18\n"
generated_code += " #include \"dpdk_lib.h\"// sugar@19\n"
generated_code += " #include \"actions.h\"// sugar@20\n"
generated_code += " \n"
generated_code += " extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@22\n"
generated_code += "\n"
generated_code += " extern void increase_counter (int counterid, int index);// sugar@24\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@27\n"
generated_code += "\n"

generated_code += " uint8_t reverse_buffer[" + str(max([t[1] for t in map(getTypeAndLength, hlir.p4_tables.values())])) + "];// sugar@30\n"

def match_type_order(t):
    if t is p4.p4_match_type.P4_MATCH_EXACT:   return 0
    if t is p4.p4_match_type.P4_MATCH_LPM:     return 1
    if t is p4.p4_match_type.P4_MATCH_TERNARY: return 2

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " void table_" + str(table.name) + "_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@39\n"
    sortedfields = sorted(table.match_fields, key=lambda field: match_type_order(field[1]))
    for match_field, match_type, match_mask in sortedfields:
        if match_field.width <= 32:
            generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(match_field)) + ", *(uint32_t*)key)// sugar@43\n"
            generated_code += " key += sizeof(uint32_t);// sugar@44\n"
        elif match_field.width > 32 and match_field.width % 8 == 0:
            byte_width = (match_field.width+7)/8
            generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(match_field)) + ", key)// sugar@47\n"
            generated_code += " key += " + str(byte_width) + ";// sugar@48\n"
        else:
            print "Unsupported field %s ignored in key calculation." % fld_id(match_field)
    if table_type == "LOOKUP_LPM":
        generated_code += " key -= " + str(key_length) + ";// sugar@52\n"
        generated_code += " int c, d;// sugar@53\n"
        generated_code += " for(c = " + str(key_length-1) + ", d = 0; c >= 0; c--, d++) *(reverse_buffer+d) = *(key+c);// sugar@54\n"
        generated_code += " for(c = 0; c < " + str(key_length) + "; c++) *(key+c) = *(reverse_buffer+c);// sugar@55\n"
    generated_code += " }// sugar@56\n"
    generated_code += "\n"

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    lookupfun = {'LOOKUP_LPM':'lpm_lookup', 'LOOKUP_EXACT':'exact_lookup', 'LOOKUP_TERNARY':'ternary_lookup'}
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@62\n"
    generated_code += " {// sugar@63\n"
    generated_code += "     debug(\"  :::: EXECUTING TABLE " + str(table.name) + "\\n\");// sugar@64\n"
    generated_code += "     uint8_t* key[" + str(key_length) + "];// sugar@65\n"
    generated_code += "     table_" + str(table.name) + "_key(pd, (uint8_t*)key);// sugar@66\n"
    generated_code += "     uint8_t* value = " + str(lookupfun[table_type]) + "(tables[TABLE_" + str(table.name) + "], (uint8_t*)key);// sugar@67\n"
    generated_code += "     int index = *(int*)(value+sizeof(struct " + str(table.name) + "_action)); (void)index;// sugar@68\n"
    generated_code += "     struct " + str(table.name) + "_action* res = (struct " + str(table.name) + "_action*)value;// sugar@69\n"
    for counter in table.attached_counters:
        generated_code += " increase_counter(COUNTER_" + str(counter.name) + ", index);// sugar@71\n"
    generated_code += "     if(res == NULL) {// sugar@72\n"
    generated_code += "         debug(\"    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\\n\");// sugar@73\n"
    generated_code += "         return;// sugar@74\n"
    generated_code += "     }// sugar@75\n"
    generated_code += "     switch (res->action_id) {// sugar@76\n"
    for action in table.actions:
        generated_code += " case action_" + str(action.name) + ":// sugar@78\n"
        generated_code += "   debug(\"    :: EXECUTING ACTION " + str(action.name) + "...\\n\");// sugar@79\n"
        if action.signature:
            generated_code += " action_code_" + str(action.name) + "(pd, tables, res->" + str(action.name) + "_params);// sugar@81\n"
        else:
            generated_code += " action_code_" + str(action.name) + "(pd, tables);// sugar@83\n"
        generated_code += "     break;// sugar@84\n"
    generated_code += "     }// sugar@85\n"
    if 'hit' in table.next_:
        if table.next_['hit'] is not None:
            generated_code += " if(res != NULL)// sugar@88\n"
            generated_code += "     apply_table_" + str(table.next_['hit'].name) + "(pd, tables);// sugar@89\n"
        if table.next_['miss'] is not None:
            generated_code += " if(res == NULL)// sugar@91\n"
            generated_code += "     apply_table_" + str(table.next_['miss'].name) + "(pd, tables);// sugar@92\n"
    else:
        generated_code += " switch (res->action_id) {// sugar@94\n"
        for action, nextnode in table.next_.items():
            generated_code += " case action_" + str(action.name) + ":// sugar@96\n"
            generated_code += "     " + str(format_p4_node(nextnode)) + "// sugar@97\n"
            generated_code += "     break;// sugar@98\n"
        generated_code += " }// sugar@99\n"
    generated_code += " }// sugar@100\n"
    generated_code += "\n"

generated_code += " void init_headers(packet_descriptor_t* packet_desc) {// sugar@103\n"
for hi in header_instances(hlir):
    n = hdr_prefix(hi.name)
    if hi.metadata:
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_instance_byte_width[" + str(n) + "],// sugar@107\n"
        generated_code += "                               .pointer = calloc(header_instance_byte_width[" + str(n) + "], sizeof(uint8_t)) };// sugar@108\n"
    else:
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_instance_byte_width[" + str(n) + "], .pointer = NULL };// sugar@110\n"
generated_code += " }// sugar@111\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    if key_length == 0 and len(table.actions) == 1:
        action = table.actions[0]
        generated_code += " extern void " + str(table.name) + "_setdefault(struct " + str(table.name) + "_action);// sugar@117\n"
generated_code += "\n"
generated_code += " void init_keyless_tables() {// sugar@119\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    if key_length == 0 and len(table.actions) == 1:
        action = table.actions[0]
        generated_code += " struct " + str(table.name) + "_action " + str(table.name) + "_a;// sugar@124\n"
        generated_code += " " + str(table.name) + "_a.action_id = action_" + str(action.name) + ";// sugar@125\n"
        generated_code += " " + str(table.name) + "_setdefault(" + str(table.name) + "_a);// sugar@126\n"
generated_code += " }// sugar@127\n"
generated_code += "\n"
generated_code += " void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@129\n"
generated_code += "     init_headers(pd);// sugar@130\n"
generated_code += "     init_keyless_tables();// sugar@131\n"
generated_code += " }// sugar@132\n"

generated_code += " \n"
generated_code += " void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@135\n"
generated_code += " {// sugar@136\n"
generated_code += "     int value32;// sugar@137\n"
generated_code += "     EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@138\n"
generated_code += "     debug(\"### HANDLING PACKET ARRIVING AT PORT %\" PRIu32 \"...\\n\", value32);// sugar@139\n"
generated_code += "     parse_packet(pd, tables);// sugar@140\n"
generated_code += " }// sugar@141\n"