 #include <stdlib.h>// sugar@17
 #include <string.h>// sugar@18
 #include "dpdk_lib.h"// sugar@19
 #include "actions.h"// sugar@20
 
 extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@22

 extern void parse_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@24 

 extern void increase_counter (int counterid, int index);// sugar@24

 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@27
 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@27
 void apply_table_smacs(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@29 
 void apply_table_dmacs(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables);// sugar@29
 uint64_t prev_tsc, diff_tsc, cur_tsc; // part of timing profiles       

 uint8_t reverse_buffer[6];// sugar@30
 void table_smac_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@39
 EXTRACT_BYTEBUF(pd, field_instance_ethernet_srcAddr, key)// sugar@47
 key += 6;// sugar@48
 }// sugar@56

 void table_dmac_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@39
 EXTRACT_BYTEBUF(pd, field_instance_ethernet_dstAddr, key)// sugar@47
 key += 6;// sugar@48
 }// sugar@56

 void table_smacs_key(packet_descriptor_t* pd, int batch_size, uint8_t* key[][6]) {// sugar@41
 for(int i=0;i<batch_size;i++){
        EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet_srcAddr, (uint8_t *)key[i])// sugar@49
 //key += 6;// sugar@50
 }
 }// sugar@58

  void table_dmacs_key(packet_descriptor_t* pd, int batch_size, uint8_t* key[][6]) {// sugar@41
 for(int i=0;i<batch_size;i++){
        EXTRACT_BYTEBUF(&pd[i], field_instance_ethernet_dstAddr, (uint8_t *)key[i])// sugar@49
 //key += 6;// sugar@50
 }
 }// sugar@58

 void apply_table_smacs(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@64
 {// sugar@65
     /* Check if number of packets lookup are more than zero */ 
     if(batch_size==0){
         return;
     }
     uint8_t* key[batch_size][6];// sugar@67
     table_smacs_key(pd, batch_size, key);// sugar@68
     uint8_t* values[batch_size];

     prev_tsc = rte_rdtsc();
     exact_lookups(tables[TABLE_smac], batch_size, key, values);// sugar@69
 
     cur_tsc = rte_rdtsc();
     debug(" %llu , \n", cur_tsc- prev_tsc); 
     int index[batch_size];
     struct smac_action* res[batch_size];
     //debug("  :::: EXECUTING TABLE smac\n");// sugar@66
     for(int i=0;i<batch_size;i++){
         //index[i] = *(int*)(values[i]+sizeof(struct smac_action));
         //(void)index[i];// sugar@70
         res[i] = (struct smac_action*)((values[i]));// sugar@71
         if(res[i] == NULL) {// sugar@74
 	     //debug("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@75
             continue;// sugar@76
         }// sugar@77
	 //debug("src read action : %d\n", res[i]->action_id);
         switch (res[i]->action_id) {// sugar@78
         case action_mac_learn:// sugar@80
         //debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@81
             action_code_mac_learn(&pd[i], tables);// sugar@85
             break;// sugar@86
         case action__nop:// sugar@80
         //debug("    :: EXECUTING ACTION _nop...\n");// sugar@81
             action_code__nop(&pd[i], tables);// sugar@85
             break;// sugar@86
             }// sugar@87
         switch (res[i]->action_id) {// sugar@96
         case action_mac_learn:// sugar@98
             default :
             break;// sugar@100
         case action__nop:// sugar@98
 //          return apply_table_dmac(&pd[i], tables);// sugar@99
             break;// sugar@100
         }// sugar@101
     }
     
     return apply_table_dmacs(pd, batch_size, tables);// sugar@99
     //debug(" :::: End of apply_table_smacs \n");
 }// sugar@102


 void apply_table_dmacs(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@64
 {// sugar@65
     // debug("  :::: EXECUTING TABLE dmac\n");// sugar@66
     uint8_t* key[batch_size][6];// sugar@67
     table_dmacs_key(pd, batch_size, key);// sugar@68
     uint8_t* values[batch_size];
  
//   prev_tsc = rte_rdtsc();
     exact_lookups(tables[TABLE_dmac], batch_size, key, values);// sugar@69

//   cur_tsc = rte_rdtsc();
//   int index = *(int*)(value+sizeof(struct dmac_action)); (void)index;// sugar@70
     struct dmac_action *res[batch_size];
//   printf(" %llu\n", cur_tsc -prev_tsc); 
     for(int i=0; i<batch_size; i++){
   
/*     
         uint8_t *p1;
	 uint8_t *p;
 	 p = (uint8_t *)(pd[i].headers[header_instance_ethernet].pointer + 0);
 	 p1 =  (uint8_t *)(pd[i].headers[header_instance_ethernet].pointer + 6);
	 debug("src_addr %x, %x, %x, %x, %x, %x\n",p[0], p[1], p[2], p[3], p[4], p[5]);
	 debug("dst_addr %x, %x, %x, %x,%x, %x\n",p1[0], p1[1], p1[2], p1[3], p1[4], p1[5]);
*/

     res[i] = (struct dmac_action*)(values[i]);// sugar@71
     if(res[i] == NULL) {// sugar@74
//       debug("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@75
         continue;
//       return;// sugar@76
     }// sugar@77

//   debug("dst read action : %d\n", res[i]->action_id);
     switch (res[i]->action_id) {// sugar@78
     case action_forward:// sugar@80
//   debug("    :: EXECUTING ACTION forward...\n");// sugar@81
         action_code_forward(&pd[i], tables, res[i]->forward_params);// sugar@83
         break;// sugar@86
     case action_bcast:// sugar@80
//   debug("    :: EXECUTING ACTION bcast...\n");// sugar@81
         action_code_bcast(&pd[i], tables);// sugar@85
         break;// sugar@86
     }// sugar@87
     switch (res[i]->action_id) {// sugar@96
     case action_forward:// sugar@98
     // sugar@99
         break;// sugar@100
     case action_bcast:// sugar@98
     // sugar@99
         break;// sugar@100
     }// sugar@101
     }

 }// sugar@102




 void apply_table_smac(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@62
 {// sugar@63
     //debug("  :::: EXECUTING TABLE smac\n");// sugar@64
     uint8_t* key[6];// sugar@65
     table_smac_key(pd, (uint8_t*)key);// sugar@66
     uint8_t* value = exact_lookup(tables[TABLE_smac], (uint8_t*)key);// sugar@67
     int index = *(int*)(value+sizeof(struct smac_action)); (void)index;// sugar@68
     debug("print index : %d, %d\n", index, *((int*)value));
     struct smac_action* res = (struct smac_action*)value;// sugar@69
     if(res == NULL) {// sugar@72
         //debug("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@73
         return;// sugar@74
     }// sugar@75
     switch (res->action_id) {// sugar@76
 case action_mac_learn:// sugar@78
   //debug("    :: EXECUTING ACTION mac_learn...\n");// sugar@79
 action_code_mac_learn(pd, tables);// sugar@83
     break;// sugar@84
 case action__nop:// sugar@78
   //debug("    :: EXECUTING ACTION _nop...\n");// sugar@79
 action_code__nop(pd, tables);// sugar@83
     break;// sugar@84
     }// sugar@85
 switch (res->action_id) {// sugar@94
 case action_mac_learn:// sugar@96
     return apply_table_dmac(pd, tables);// sugar@97
     break;// sugar@98
 case action__nop:// sugar@96
     return apply_table_dmac(pd, tables);// sugar@97
     break;// sugar@98
 }// sugar@99
 }// sugar@100

 void apply_table_dmac(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@62
 {// sugar@63
     //debug("  :::: EXECUTING TABLE dmac\n");// sugar@64
     uint8_t* key[6];// sugar@65
     table_dmac_key(pd, (uint8_t*)key);// sugar@66
     uint8_t* value = exact_lookup(tables[TABLE_dmac], (uint8_t*)key);// sugar@67
     int index = *(int*)(value+sizeof(struct dmac_action)); (void)index;// sugar@68
     struct dmac_action* res = (struct dmac_action*)value;// sugar@69
     if(res == NULL) {// sugar@72
         //debug("    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET.\n");// sugar@73
         return;// sugar@74
     }// sugar@75

debug("read action dest : %d\n", res->action_id);
     
	    uint8_t *p1;
	    uint8_t *p;
 	    p = (uint8_t *)(pd->headers[header_instance_ethernet].pointer + 0);
 	    p1 =  (uint8_t *)(pd->headers[header_instance_ethernet].pointer + 6);
	    debug("src_addr %x, %x, %x, %x, %x, %x\n",p[0], p[1], p[2], p[3], p[4], p[5]);
	    debug("dst_addr %x, %x, %x, %x,%x, %x\n",p1[0], p1[1], p1[2], p1[3], p1[4], p1[5]);
switch (res->action_id) {// sugar@76
 case action_forward:// sugar@78
    debug("    :: EXECUTING ACTION forward...%u\n", res->forward_params.port[0]);// sugar@79
 action_code_forward(pd, tables, res->forward_params);// sugar@81
     break;// sugar@84
 case action_bcast:// sugar@78
   debug("    :: EXECUTING ACTION bcast...\n");// sugar@79
 action_code_bcast(pd, tables);// sugar@83
     break;// sugar@84
     }// sugar@85
 switch (res->action_id) {// sugar@94
 case action_forward:// sugar@96
     // sugar@97
     break;// sugar@98
 case action_bcast:// sugar@96
     // sugar@97
     break;// sugar@98
 }// sugar@99
 }// sugar@100

 void init_headers(packet_descriptor_t* packet_desc) {// sugar@103
 packet_desc->headers[header_instance_standard_metadata] = (header_descriptor_t) { .type = header_instance_standard_metadata, .length = header_instance_byte_width[header_instance_standard_metadata],// sugar@107
                               .pointer = calloc(header_instance_byte_width[header_instance_standard_metadata], sizeof(uint8_t)) };// sugar@108
 packet_desc->headers[header_instance_ethernet] = (header_descriptor_t) { .type = header_instance_ethernet, .length = header_instance_byte_width[header_instance_ethernet], .pointer = NULL };// sugar@110
 }// sugar@111


 void init_keyless_tables() {// sugar@119
 }// sugar@127

 void init_dataplane(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@129
     init_headers(pd);// sugar@130
     init_keyless_tables();// sugar@131
 }// sugar@132
 
 void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@135
 {// sugar@136
     int value32;// sugar@137
     EXTRACT_INT32_BITS(pd, field_instance_standard_metadata_ingress_port, value32)// sugar@138
     //debug("### HANDLING PACKET ARRIVING AT PORT %" PRIu32 "...\n", value32);// sugar@139
     parse_packet(pd, tables);// sugar@140
 }// sugar@141

void handle_packets(packet_descriptor_t* pd, int batch_size, lookup_table_t** tables)// sugar@137
 {// sugar@138
     for( int i=0;i<batch_size;i ++){// sugar@139
        int value32;// sugar@140
        EXTRACT_INT32_BITS(&pd[i], field_instance_standard_metadata_ingress_port, value32)// sugar@141

     }// sugar@143
     parse_packets(pd, batch_size, tables);// sugar@144
 }// sugar@145

