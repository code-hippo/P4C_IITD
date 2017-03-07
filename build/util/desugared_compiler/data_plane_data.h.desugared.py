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
generated_code += " #ifndef __DATA_PLANE_DATA_H__// sugar@14\n"
generated_code += " #define __DATA_PLANE_DATA_H__// sugar@15\n"
generated_code += "\n"
generated_code += " #include \"parser.h\"// sugar@17\n"
generated_code += "\n"
generated_code += " #define NB_TABLES " + str(len(hlir.p4_tables)) + "// sugar@19\n"
generated_code += "\n"
generated_code += " enum table_names {// sugar@21\n"
for table in hlir.p4_tables.values():
    generated_code += " TABLE_" + str(table.name) + ",// sugar@23\n"
generated_code += " TABLE_// sugar@24\n"
generated_code += " };// sugar@25\n"
generated_code += "\n"
generated_code += " #define NB_COUNTERS " + str(len(hlir.p4_counters)) + "// sugar@27\n"
generated_code += "\n"
generated_code += " enum counter_names {// sugar@29\n"
for counter in hlir.p4_counters.values():
    generated_code += " COUNTER_" + str(counter.name) + ",// sugar@31\n"
generated_code += " COUNTER_// sugar@32\n"
generated_code += " };// sugar@33\n"
generated_code += " \n"
generated_code += " #endif // __DATA_PLANE_DATA_H__// sugar@35\n"