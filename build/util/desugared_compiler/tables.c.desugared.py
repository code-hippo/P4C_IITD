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
from utils.hlir import getTypeAndLength
import p4_hlir.hlir.p4_stateful as p4_stateful

generated_code += " #include \"dataplane.h\"// sugar@17\n"
generated_code += " #include \"actions.h\"// sugar@18\n"
generated_code += " #include \"data_plane_data.h\"// sugar@19\n"
generated_code += "\n"
generated_code += " lookup_table_t table_config[NB_TABLES] = {// sugar@21\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " {// sugar@24\n"
    generated_code += "  .name= \"" + str(table.name) + "\",// sugar@25\n"
    generated_code += "  .type = " + str(table_type) + ",// sugar@26\n"
    generated_code += "  .key_size = " + str(key_length) + ",// sugar@27\n"
    generated_code += "  .val_size = sizeof(struct " + str(table.name) + "_action),// sugar@28\n"
    generated_code += "  .min_size = 0, //" + str(table.min_size) + ",// sugar@29\n"
    generated_code += "  .max_size = 255 //" + str(table.max_size) + "// sugar@30\n"
    generated_code += " },// sugar@31\n"
generated_code += " };// sugar@32\n"

generated_code += " counter_t counter_config[NB_COUNTERS] = {// sugar@34\n"
for counter in hlir.p4_counters.values():
    generated_code += " {// sugar@36\n"
    generated_code += "  .name= \"" + str(counter.name) + "\",// sugar@37\n"
    if counter.instance_count is not None:
        generated_code += " .size = " + str(counter.instance_count) + ",// sugar@39\n"
    elif counter.binding is not None:
        btype, table = counter.binding
        if btype is p4_stateful.P4_DIRECT:
            generated_code += " .size = " + str(table.max_size) + ",// sugar@43\n"
    else:
        generated_code += " .size = 1,// sugar@45\n"
    generated_code += "  .min_width = " + str(32 if counter.min_width is None else counter.min_width) + ",// sugar@46\n"
    generated_code += "  .saturating = " + str(1 if counter.saturating else 0) + "// sugar@47\n"
    generated_code += " },// sugar@48\n"
generated_code += " };// sugar@49\n"