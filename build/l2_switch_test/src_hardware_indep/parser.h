 #ifndef __HEADER_INFO_H__// sugar@92
 #define __HEADER_INFO_H__// sugar@93
 
 #define HEADER_INSTANCE_COUNT 2// sugar@95
 #define HEADER_STACK_COUNT 0// sugar@96
 #define FIELD_INSTANCE_COUNT 11// sugar@97
 
 enum header_stack_e {
  header_stack_
};

// sugar@99
 enum header_instance_e {
  header_instance_standard_metadata,
  header_instance_ethernet
};

// sugar@100
 enum field_instance_e {
  field_instance_standard_metadata_ingress_port,
  field_instance_standard_metadata_packet_length,
  field_instance_standard_metadata_egress_spec,
  field_instance_standard_metadata_egress_port,
  field_instance_standard_metadata_egress_instance,
  field_instance_standard_metadata_instance_type,
  field_instance_standard_metadata_clone_spec,
  field_instance_standard_metadata__padding,
  field_instance_ethernet_dstAddr,
  field_instance_ethernet_srcAddr,
  field_instance_ethernet_etherType
};

// sugar@101
 typedef enum header_instance_e header_instance_t;// sugar@102
 typedef enum field_instance_e field_instance_t;// sugar@103
 static const int header_instance_byte_width[HEADER_INSTANCE_COUNT] = {
  20 /* header_instance_standard_metadata */,
  14 /* header_instance_ethernet */
};

// sugar@104
 static const int header_instance_is_metadata[HEADER_INSTANCE_COUNT] = {
  1 /* header_instance_standard_metadata */,
  0 /* header_instance_ethernet */
};

// sugar@105
 static const int field_instance_bit_width[FIELD_INSTANCE_COUNT] = {
  9 /* field_instance_standard_metadata_ingress_port */,
  32 /* field_instance_standard_metadata_packet_length */,
  9 /* field_instance_standard_metadata_egress_spec */,
  9 /* field_instance_standard_metadata_egress_port */,
  32 /* field_instance_standard_metadata_egress_instance */,
  32 /* field_instance_standard_metadata_instance_type */,
  32 /* field_instance_standard_metadata_clone_spec */,
  5 /* field_instance_standard_metadata__padding */,
  48 /* field_instance_ethernet_dstAddr */,
  48 /* field_instance_ethernet_srcAddr */,
  16 /* field_instance_ethernet_etherType */
};

// sugar@106
 static const int field_instance_bit_offset[FIELD_INSTANCE_COUNT] = {
  0 /* field_instance_standard_metadata_ingress_port */,
  1 /* field_instance_standard_metadata_packet_length */,
  1 /* field_instance_standard_metadata_egress_spec */,
  2 /* field_instance_standard_metadata_egress_port */,
  3 /* field_instance_standard_metadata_egress_instance */,
  3 /* field_instance_standard_metadata_instance_type */,
  3 /* field_instance_standard_metadata_clone_spec */,
  3 /* field_instance_standard_metadata__padding */,
  0 /* field_instance_ethernet_dstAddr */,
  0 /* field_instance_ethernet_srcAddr */,
  0 /* field_instance_ethernet_etherType */
};

// sugar@107
 static const int field_instance_byte_offset_hdr[FIELD_INSTANCE_COUNT] = {
  0 /* field_instance_standard_metadata_ingress_port */,
  1 /* field_instance_standard_metadata_packet_length */,
  5 /* field_instance_standard_metadata_egress_spec */,
  6 /* field_instance_standard_metadata_egress_port */,
  7 /* field_instance_standard_metadata_egress_instance */,
  11 /* field_instance_standard_metadata_instance_type */,
  15 /* field_instance_standard_metadata_clone_spec */,
  19 /* field_instance_standard_metadata__padding */,
  0 /* field_instance_ethernet_dstAddr */,
  6 /* field_instance_ethernet_srcAddr */,
  12 /* field_instance_ethernet_etherType */
};

// sugar@108
 static const uint32_t field_instance_mask[FIELD_INSTANCE_COUNT] = {
  0x1ff /* field_instance_standard_metadata_ingress_port */,
  0x0 /* field_instance_standard_metadata_packet_length */,
  0x3fe /* field_instance_standard_metadata_egress_spec */,
  0x7fc /* field_instance_standard_metadata_egress_port */,
  0x0 /* field_instance_standard_metadata_egress_instance */,
  0x0 /* field_instance_standard_metadata_instance_type */,
  0x0 /* field_instance_standard_metadata_clone_spec */,
  0xf8 /* field_instance_standard_metadata__padding */,
  0x0 /* field_instance_ethernet_dstAddr */,
  0x0 /* field_instance_ethernet_srcAddr */,
  0xffff /* field_instance_ethernet_etherType */
};

// sugar@109
 static const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT] = {
  header_instance_standard_metadata /* field_instance_standard_metadata_ingress_port */,
  header_instance_standard_metadata /* field_instance_standard_metadata_packet_length */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_spec */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_port */,
  header_instance_standard_metadata /* field_instance_standard_metadata_egress_instance */,
  header_instance_standard_metadata /* field_instance_standard_metadata_instance_type */,
  header_instance_standard_metadata /* field_instance_standard_metadata_clone_spec */,
  header_instance_standard_metadata /* field_instance_standard_metadata__padding */,
  header_instance_ethernet /* field_instance_ethernet_dstAddr */,
  header_instance_ethernet /* field_instance_ethernet_srcAddr */,
  header_instance_ethernet /* field_instance_ethernet_etherType */
};

// sugar@110
 
 static const header_instance_t header_stack_elements[HEADER_STACK_COUNT][10] = {
  
};

// sugar@112
 static const unsigned header_stack_size[HEADER_STACK_COUNT] = {
  
};

// sugar@113
 typedef enum header_stack_e header_stack_t;// sugar@114
 
 #endif // __HEADER_INFO_H__// sugar@116
