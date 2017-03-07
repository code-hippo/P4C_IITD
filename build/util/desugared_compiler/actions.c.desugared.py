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
from p4_hlir.hlir.p4_headers import p4_field, p4_field_list
from p4_hlir.hlir.p4_imperatives import p4_signature_ref
from utils.misc import addError, addWarning 
from utils.hlir import hdr_prefix, fld_prefix, fld_id, userActions 

generated_code += " #include \"dpdk_lib.h\"// sugar@19\n"
generated_code += " #include \"actions.h\"// sugar@20\n"
generated_code += " #include <unistd.h>// sugar@21\n"
generated_code += " #include <arpa/inet.h>// sugar@22\n"
generated_code += " extern void increase_counter (int counterid, int index);// sugar@23\n"
generated_code += "\n"
generated_code += " extern backend bg;// sugar@25\n"
generated_code += "\n"

actions = hlir.p4_actions
useractions = userActions(actions)
useraction_objs = [(actions[act_key]) for act_key in useractions ]

# =============================================================================
# MODIFY_FIELD

def modify_field(fun, call):
    generated_code = ""
    args = call[1]
    dst = args[0]
    src = args[1]
    # mask = args[2]
    if not isinstance(dst, p4_field):
        addError("generating modify_field", "We do not allow changing an R-REF yet")
    if isinstance(src, int):
        generated_code += " value32 = " + str(src) + ";// sugar@44\n"
        if dst.width <= 32:
            generated_code += " MODIFY_INT32_INT32_AUTO(pd, " + str(fld_id(dst)) + ", value32)// sugar@46\n"
        else:
            if dst.width % 8 == 0 and dst.offset % 8 == 0:
                generated_code += " MODIFY_BYTEBUF_INT32(pd, " + str(fld_id(dst)) + ", value32) //TODO: This macro is not implemented// sugar@49\n"
            else:
                addError("generating modify_field", "Improper bytebufs cannot be modified yet.")
    elif isinstance(src, p4_field):
        if dst.width <= 32 and src.width <= 32:
            if src.instance.metadata == dst.instance.metadata:
                generated_code += " EXTRACT_INT32_BITS(pd, " + str(fld_id(src)) + ", value32)// sugar@55\n"
                generated_code += " MODIFY_INT32_INT32_BITS(pd, " + str(fld_id(dst)) + ", value32)// sugar@56\n"
            else:
                generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(src)) + ", value32)// sugar@58\n"
                generated_code += " MODIFY_INT32_INT32_AUTO(pd, " + str(fld_id(dst)) + ", value32)// sugar@59\n"
        elif src.width != dst.width:
            addError("generating modify_field", "bytebuf field-to-field of different widths is not supported yet")
        else:
            if dst.width % 8 == 0 and dst.offset % 8 == 0 and src.width % 8 == 0 and src.offset % 8 == 0 and src.instance.metadata == dst.instance.metadata:
                generated_code += " MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", FIELD_BYTE_ADDR(pd, field_desc(" + str(fld_id(src)) + ")), (field_desc(" + str(fld_id(dst)) + ")).bytewidth)// sugar@64\n"
            else:
                addError("generating modify_field", "Improper bytebufs cannot be modified yet.")
    elif isinstance(src, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[src.idx])
        l = fun.signature_widths[src.idx]
        if dst.width <= 32 and l <= 32:
            generated_code += " MODIFY_INT32_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(p) + ", " + str((l+7)/8) + ")// sugar@71\n"
        else:
            if dst.width % 8 == 0 and dst.offset % 8 == 0 and l % 8 == 0: #and dst.instance.metadata:
                generated_code += " MODIFY_BYTEBUF_BYTEBUF(pd, " + str(fld_id(dst)) + ", " + str(p) + ", (field_desc(" + str(fld_id(dst)) + ")).bytewidth)// sugar@74\n"
            else:
                addError("generating modify_field", "Improper bytebufs cannot be modified yet.")        
    return generated_code

# =============================================================================
# ADD_TO_FIELD

def add_to_field(fun, call):
    generated_code = ""
    args = call[1]
    dst = args[0]
    val = args[1]
    if not isinstance(dst, p4_field):
        addError("generating add_to_field", "We do not allow changing an R-REF yet")
    if isinstance(val, int):
        generated_code += " value32 = " + str(val) + ";// sugar@90\n"
        if dst.width <= 32:
            generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(dst)) + ", res32)// sugar@92\n"
            generated_code += " value32 += res32;// sugar@93\n"
            generated_code += " MODIFY_INT32_INT32_AUTO(pd, " + str(fld_id(dst)) + ", value32)// sugar@94\n"
        else:
            addError("generating modify_field", "Bytebufs cannot be modified yet.")
    elif isinstance(val, p4_field):
        if dst.width <= 32 and val.length <= 32:
            generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(val)) + ", value32)// sugar@99\n"
            generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(dst)) + ", res32)// sugar@100\n"
            generated_code += " value32 += res32;// sugar@101\n"
            generated_code += " MODIFY_INT32_INT32_AUTO(pd, " + str(fld_id(dst)) + ", value32)// sugar@102\n"
        else:
            addError("generating add_to_field", "bytebufs cannot be modified yet.")
    elif isinstance(val, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[val.idx])
        l = fun.signature_widths[val.idx]
        if dst.width <= 32 and l <= 32:
            generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(dst)) + ", res32)// sugar@109\n"
            generated_code += " TODO// sugar@110\n"
        else:
            addError("generating add_to_field", "bytebufs cannot be modified yet.")
    return generated_code

# =============================================================================
# COUNT

def count(fun, call):
    generated_code = ""
    args = call[1]
    counter = args[0]
    index = args[1]
    if isinstance(index, int): # TODO
        generated_code += " value32 = " + str(val) + ";// sugar@124\n"
    elif isinstance(index, p4_field): # TODO
        generated_code += " EXTRACT_INT32_AUTO(pd, " + str(fld_id(index)) + ", value32)// sugar@126\n"
    elif isinstance(val, p4_signature_ref):
        generated_code += " value32 = TODO;// sugar@128\n"
    generated_code += " increase_counter(COUNTER_" + str(counter.name) + ", value32);// sugar@129\n"
    return generated_code

# =============================================================================
# GENERATE_DIGEST

def generate_digest(fun, call):
    generated_code = ""
    
    ## TODO make this proper
    fun_params = ["bg", "\"mac_learn_digest\""]
    for p in call[1]:
        if isinstance(p, int):
            fun_params += "0" #[str(p)]
        elif isinstance(p, p4_field_list):
            field_list = p
            fun_params += ["&fields"]
        else:
            addError("generating actions.c", "Unhandled parameter type in generate_digest: " + str(p))
 
    generated_code += "  struct type_field_list fields;// sugar@149\n"
    quan = str(len(field_list.fields))
    generated_code += "    fields.fields_quantity = " + str(quan) + ";// sugar@151\n"
    generated_code += "    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@152\n"
    generated_code += "    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@153\n"
    for i,field in enumerate(field_list.fields):
        j = str(i)
        if isinstance(field, p4_field):
            generated_code += "    fields.field_offsets[" + str(j) + "] = (uint8_t*) (pd->headers[header_instance_" + str(field.instance) + "].pointer + field_instance_byte_offset_hdr[field_instance_" + str(field.instance) + "_" + str(field.name) + "]);// sugar@157\n"
            generated_code += "    fields.field_widths[" + str(j) + "] = field_instance_bit_width[field_instance_" + str(field.instance) + "_" + str(field.name) + "]*8;// sugar@158\n"
        else:
            addError("generating actions.c", "Unhandled parameter type in field_list: " + name + ", " + str(field))

    params = ",".join(fun_params)
    generated_code += "\n"
    generated_code += "    generate_digest(" + str(params) + "); sleep(1);// sugar@164\n"
    return generated_code

# =============================================================================
# DROP

def drop(fun, call):
    return "drop(pd);"

# =============================================================================
# PUSH

def push(fun, call):
    generated_code = ""
    args = call[1]
    i = args[0]
    generated_code += " push(pd, header_stack_" + str(i.base_name) + ");// sugar@180\n"
    return generated_code

# =============================================================================
# POP

def pop(fun, call):
    generated_code = ""
    args = call[1]
    i = args[0]
    generated_code += " pop(pd, header_stack_" + str(i.base_name) + ");// sugar@190\n"
    return generated_code

# =============================================================================

for fun in useraction_objs:
    hasParam = fun.signature
    modifiers = ""
    ret_val_type = "void"
    name = fun.name
    params = ", struct action_%s_params parameters" % (name) if hasParam else ""
    generated_code += " " + str(modifiers) + " " + str(ret_val_type) + " action_code_" + str(name) + "(packet_descriptor_t* pd, lookup_table_t** tables " + str(params) + ") {// sugar@201\n"
    generated_code += "     uint32_t value32, res32;// sugar@202\n"
    generated_code += "     (void)value32; (void)res32;// sugar@203\n"
    for i,call in enumerate(fun.call_sequence):
        name = call[0].name 

        # Generates a primitive action call to `name'
        if name in locals().keys():
            generated_code += " " + str(locals()[name](fun, call)) + "// sugar@209\n"
        else:
            addWarning("generating actions.c", "Unhandled primitive function: " +  name)

    generated_code += " }// sugar@213\n"
    generated_code += "\n"
