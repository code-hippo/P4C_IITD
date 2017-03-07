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
generated_code += " #ifndef __ACTION_INFO_GENERATED_H__// sugar@14\n"
generated_code += " #define __ACTION_INFO_GENERATED_H__// sugar@15\n"
generated_code += " \n"
generated_code += "\n"
generated_code += " #define FIELD(name, length) uint8_t name[(length + 7) / 8];// sugar@18\n"
generated_code += "\n"

generated_code += " enum actions {// sugar@21\n"
a = []
for table in hlir.p4_tables.values():
    for action in table.actions:
        if a.count(action.name) == 0:
            a.append(action.name)
            generated_code += " action_" + str(action.name) + ",// sugar@27\n"
generated_code += " };// sugar@28\n"

for table in hlir.p4_tables.values():
    for action in table.actions:
        if action.signature:
            generated_code += " struct action_" + str(action.name) + "_params {// sugar@33\n"
            for name, length in zip(action.signature, action.signature_widths):
                generated_code += " FIELD(" + str(name) + ", " + str(length) + ");// sugar@35\n"
            generated_code += " };// sugar@36\n"

for table in hlir.p4_tables.values():
    generated_code += " struct " + str(table.name) + "_action {// sugar@39\n"
    generated_code += "     int action_id;// sugar@40\n"
    generated_code += "     union {// sugar@41\n"
    for action in table.actions:
        if action.signature:
            generated_code += " struct action_" + str(action.name) + "_params " + str(action.name) + "_params;// sugar@44\n"
    generated_code += "     };// sugar@45\n"
    generated_code += " };// sugar@46\n"

for table in hlir.p4_tables.values():
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t *pd, lookup_table_t** tables);// sugar@49\n"
    for action in table.actions:
        if action.signature:
            generated_code += " void action_code_" + str(action.name) + "(packet_descriptor_t *pd, lookup_table_t **tables, struct action_" + str(action.name) + "_params);// sugar@52\n"
        else:
            generated_code += " void action_code_" + str(action.name) + "(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@54\n"

generated_code += " #endif// sugar@56\n"