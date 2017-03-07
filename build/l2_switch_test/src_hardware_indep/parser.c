 #include "dpdk_lib.h"// sugar@72
 #include "actions.h" // apply_table_* and action_code_*// sugar@73

 void print_mac(uint8_t* v) { printf("%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\n", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@75
 void print_ip(uint8_t* v) { printf("%d.%d.%d.%d\n",v[0],v[1],v[2],v[3]); }// sugar@76
 
 static void// sugar@78
 extract_header(uint8_t* buf, packet_descriptor_t* pd, header_instance_t h) {// sugar@79
     pd->headers[h] =// sugar@80
       (header_descriptor_t) {// sugar@81
         .type = h,// sugar@82
         .pointer = buf,// sugar@83
         .length = header_instance_byte_width[h]// sugar@84
       };// sugar@85
 }// sugar@86
 
 static inline void p4_pe_header_too_short(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_default(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_checksum(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_unhandled_select(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_index_out_of_bounds(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_header_too_long(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static inline void p4_pe_out_of_packet(packet_descriptor_t *pd) {// sugar@90
 drop(pd);// sugar@92
 }// sugar@95
 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@98
 static void parse_states_start(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@99
 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@99
 static void parse_states_parse_ethernet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@99

 static void parse_state_start(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@123
 {// sugar@124
  return parse_state_parse_ethernet(pd, buf, tables);// sugar@21
// sugar@147
 }// sugar@182
 
 static void parse_states_start(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
  return parse_states_parse_ethernet(pd, batch_size, tables);// sugar@22
// sugar@148
 }// sugar@183

 static void parse_state_parse_ethernet(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@123
 {// sugar@124
     extract_header(buf, pd, header_instance_ethernet);// sugar@129
     buf += header_instance_byte_width[header_instance_ethernet];// sugar@130
 return apply_table_smac(pd, tables);// sugar@147
 }// sugar@182
 
 static void parse_states_parse_ethernet(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@124
 {// sugar@125
     uint8_t* buf;
     for(int i=0;i<batch_size; i++){
        buf = (uint8_t*) pd[i].data;
        extract_header(buf, &pd[i], header_instance_ethernet);// sugar@130
        buf += header_instance_byte_width[header_instance_ethernet];// sugar@131
     }
   return apply_table_smacs(pd, batch_size, tables);// sugar@148
 }// sugar@183


 void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@185
     parse_state_start(pd, pd->data, tables);// sugar@186
 }// sugar@187

 void parse_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables) {// sugar@190
        parse_states_start(pd, batch_size, tables); // sugar@191
 }// sugar@192

