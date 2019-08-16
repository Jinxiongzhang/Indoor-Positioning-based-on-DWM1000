#ifndef __MAC_H__
#define __MAC_H__

#include <stdint.h>

#define LEN_UWB_MSG_PAYLOAD 1000

// Packet format with compressed PAN and 64Bit addresses
// Maximum 64 bytes payload
#pragma anon_unions
typedef struct packet_s {
    union {
      uint16_t fcf; //frame control field������dw1000 User Manual V2.17��P221��11.2�� -- add by WangPengGuy
      struct {      // ���struct��union�У���˺�fcf����һ��16bit�ĵ�ַ��Ҳ��������octets
        uint16_t type:3;          // Frame Type field, 3 bits : bit0 - bit2
        uint16_t security:1;      // Security Enabled field, 1bit : bit3
        uint16_t framePending:1;  // Frame Pending field
        uint16_t ack:1;           // ACK Request
        uint16_t ipan:1;          // PAN ID Compress
        uint16_t reserved:3;      // Reserved
        uint16_t destAddrMode:2;  // Dest Address Mode
        uint16_t version:2;       // Frame Version
        uint16_t srcAddrMode:2;   // Source Address Mode
      }  __attribute__ ((packed))fcf_s;
    } __attribute__ ((packed));

    uint8_t seq;                   // Sequeence Number field
    uint16_t destPANId;            // Destination PAN Identifier
    uint8_t destAddress[8];        // Destination Address
		//uint16_t sourcePANId;        // Source PAN Identifier,Ӧ������������ǲ�֪��Ϊʲôû�ж��� -- add by WangPengGuy
		                               // ֪��Ϊʲô�ˣ�����dw1000 User Manual V2.17��P222��11.2.5�ڣ�fcf_s.ipan�����Ϊ1��Ŀ��PAN_ID��ԴPAN_ID��ͬ������ʡ��
		                               // �����Ϊ0������ö��壬����ʡ��
    uint8_t sourceAddress[8];      // Source Address
		//uint8_t Aux[14]              // Aux Security Header��ҲӦ���������Ҳû�ж��� -- add by WangPengGuy

    uint8_t payload[LEN_UWB_MSG_PAYLOAD];
}__attribute__ ((packed)) packet_t;

// ���涨�壺ack = 0 ˵�� Auto-Ack δ����
#define MAC80215_PACKET_INIT(packet, TYPE) packet.fcf_s.type = (TYPE); \
  packet.fcf_s.security = 0; \
  packet.fcf_s.framePending = 0; \
  packet.fcf_s.ack = 0; \
  packet.fcf_s.ipan = 1; \
  packet.fcf_s.destAddrMode = 3; \
  packet.fcf_s.version = 1; \
  packet.fcf_s.srcAddrMode = 3;

// Frame Type, ��packet_t�е�type���壬��ϸ����dw1000 User Manual V2.17��P221��11.2.1�� -- add by WangPengGuy
#define MAC802154_TYPE_BEACON 0
#define MAC802154_TYPE_DATA 1
#define MAC802154_TYPE_ACK 2
#define MAC802154_TYPE_CMD 3

#define MAC802154_HEADER_LENGTH 21  // ����packet_t�б����ε�����Field��ȷʵ21���ֽ�

#endif
