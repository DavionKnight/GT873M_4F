#ifndef _OAM_H_
#define _OAM_H_
#include "timer.h"
#include "gpio.h"


#define OK	                    0
#define ERROR		-1
#define FLASH_USER_DATA_MAX_SIZE     	(1024*20)
#define SLOW_PATH_CFG_OFFSET            (1024*19+512)
#define GWD_PRODUCT_CFG_OFFSET          (SLOW_PATH_CFG_OFFSET+64) 
#define GWD_PRODUCT_CFG_OFFSET_W		(1024*20)
#define GWD_PRODUCT_CFG_OFFSET_INBAND	(1024*20)


/* This struct should be moved to a header file */
typedef struct standard_oam_header
{
	cs_uint8 dmac[6];				/* Destination MAC: 01-80-c2-00-00-02 */
	cs_uint8 smac[6];				/* Source MAC */
	cs_uint16 type;					/* 88 09 */
	cs_uint8 subtype;				/* 0x03 - OAM */
	cs_uint16 flags;
	cs_uint8 code;					/* 0xFE Vender Externsion */
} __attribute__ ((packed)) OAM_HEADER;

typedef struct gwtt_oam_header
{
	struct standard_oam_header std_hdr;
	cs_uint8 oui[3];				/* GWTT's OUI is 00-0f-e9 */
	cs_uint8 opCode;				/* Operation Code */
	cs_ulong32 senderSerNo;		/* Sender's serial no */
	cs_uint16 wholePktLen;		/* The whole packet length, including the fragments */
	cs_uint16 payloadOffset;		/* Offset in the entire packet */
	cs_uint16 payLoadLength;		/* Payload length in this packet */
	cs_uint8 sessionId[8];		/* sesion id used by EMS */
} __attribute__ ((packed)) GWTT_OAM_HEADER;

/* �й����Ŷ��� */
typedef struct ctc_oam_header
{
	cs_uint8 oui[3];				/* CTC's OUI is waiting define */
	cs_uint8 opCode;				/* Operation Code */
	cs_uint8 payload[4];			/* Payload length is larger then this, this is just used for pointer */
} __attribute__ ((packed)) CTC_OAM_HEADER;

typedef struct ctc_oam_message_list_node
{
	struct ctc_oam_message_list_node *next;
	cs_uint8 CtcOpcode;
	cs_uint8 Reserved;
	cs_uint16 PayloadLen;
	cs_uint8 *pPayLoad;
} __attribute__ ((packed)) CTC_OAM_MESSAGE_NODE;

typedef struct TLV_Varialbe_Container
{
	cs_uint8 branch;
	cs_uint16 leaf;
	cs_uint8 width;
	cs_uint8 value[1];				/* Its length is defined in width field, here is just for reference */
} __attribute__ ((packed)) TLV_VARIALBE_CONTAINER;

typedef struct TLV_Varialbe_Descriptor
{
	cs_uint8 branch;
	cs_uint16 leaf;
} __attribute__ ((packed)) TLV_VARIALBE_DESCRIPTOR;

typedef struct TLV_Varialbe_Indication
{
	cs_uint8 branch;
	cs_uint16 leaf;
	cs_uint8 var;
} __attribute__ ((packed)) TLV_VARIALBE_INDICATION;

typedef struct TLV_Classification_Marking_Entry
{
	cs_uint8 field_sel;				/* ��1��������Ӧ����field����
										 * 0x00������DA MAC���ࣻ
										 * 0x01������SA MAC���ࣻ
										 * 0x02��������̫�����ȼ�Pri��IEEE 802.1D�����ࣻ
										 * 0x03������VLAN ID���ࣻ
										 * 0x04��������̫�����ͣ�0x8808��0x8809��0x88A8�ȡ���Ҫָ��̫��
										 * ֡�е�ԭʼ��Length/EtherType��������VLAN tag�е�TPID�򣩣�
										 * 0x05������Ŀ��IP��ַ���ࣻ
										 * 0x06������ԴIP��ַ���ࣻ
										 * 0x07������IPЭ�����ͣ�IP��ICMP��IGMP�ȣ�
										 * 0x08������IP TOS/DSCP��IP V4�����ࣻ
										 * 0x09������IP Precedence��IP V6�����ࣻ
										 * 0x0A������L4 ԴPORT���ࣻ
										 * 0x0B������L4 Ŀ��PORT���ࣻ
										 * ������ʽ�����塣 */
	cs_uint8 value[6];				/* ��1��������ƥ��ֵ���������Ӧ����С��6�ֽڣ�����VLAN Pri=1
										 * ��Ϊƥ���򣩣��������λ���뽫��ƥ��ֵ���ڱ�6�ֽڵ�
										 * ���λ����Ӧ��ƥ��ֵΪ0x00 00 00 00 00 01���� */
	cs_uint8 match_op;				/*  0x00 F Never match
										 * 0x01 == Field Equal to value
										 * 0x02 != Field Not equal to value
										 * 0x03 <= Field Less than or equal to value(��ѡ)
										 * 0x04 >= Field Greater than or equal to value����ѡ��
										 * 0x05 exists True if field exists (value ignored)
										 * 0x06 !exist True if field does not exist (value ignored)
										 * 0x07 T Always match */
} __attribute__ ((packed)) TLV_CLASSIFICATION_MARKING_ENTRY;

typedef struct TLV_Classification_Marking
{
	cs_uint8 precedence;			/* �����ࡢ�Ŷ�&��ǡ���������ȼ����� */
	cs_uint8 rule_len;				/* ����ĳ��ȣ���λΪ�ֽ� */
	cs_uint8 queue_mapped;			/* ���ϱ��������̫��֡��Ҫӳ��Ķ��б�� */
	cs_uint8 eth_pri_mark;			/* �Է��ϱ��������̫��֡�������ȼ���ǣ�IEEE 802.1D����
										 * ��ֵΪ0x00��0x07�����ֽ�ȱʡֵΪ0x00���籾�ֽڵ�ֵ
										 * Ϊ0xFF������ζ�ŶԷ��ϸ�������֡���������ȼ���ǡ� */
	cs_uint8 entry_num;			/* �������������������entries������������ж��������������
										 * Ϊ���field-value-operator�򣬼���ζ�ű���ͬʱ���������������
										 * ����ִ����������action�� */
	TLV_CLASSIFICATION_MARKING_ENTRY entry[1];
} __attribute__ ((packed)) TLV_CLASSIFICATION_MARKING;

/*Add bye QJF 2006.12.26 */
typedef struct gwtt_oam_message_list_node
{
	struct gwtt_oam_message_list_node *next;
	cs_uint8 GwOpcode;
	cs_uint32 SendSerNo;
	cs_uint16 WholePktLen;
	cs_uint16 RevPktLen;
	cs_uint8 SessionID[8];
	struct epon_timer_t TimerID;
	cs_uint8 *pPayLoad;
} GWTT_OAM_MESSAGE_NODE;

typedef struct gwtt_oam_session_info
{
	cs_uint32 SendSerNo;
	cs_uint8 SessionID[8];
} GWTT_OAM_SESSION_INFO;

enum GW_OAM_RESULT
{
	OAM_MESSAGE_RECEV_OK,
	OAM_MESSAGE_RECEV_ERR,
	OAM_MESSAGE_RECEV_NOT_COMPLETE,
	OAM_MESSAGE_RECEV_TOO_LONG,
	OAM_MESSAGE_RECEV_NO_MEM,
	OAM_MESSAGE_RECEV_OPCODE_OK,
	OAM_MESSAGE_RECEV_OPCODE_ERR,
	OAM_MESSAGE_SEND_OPCODE_OK,
	OAM_MESSAGE_SEND_OPCODE_ERR,
	OAM_MESSAGE_RECEV_TIMER_ERR,
	OAM_MESSAGE_SEND_ERROR,
	OAM_MESSAGE_SEND_OK
};
typedef enum _gwd_olt_type
{
	GWD_OLT_NONE,
	GWD_OLT_GFA6100,
	GWD_OLT_GFA6700,
	GWD_OLT_GFA6900,
	GWD_OLT_NOMATCH
}GWD_OLT_TYPE;
typedef struct Igmp_oam_auth_session
{
	struct Igmp_oam_auth_session *next;
	cs_uint32	sendSerNo;		/* send serial no */
	cs_ulong32	ulIfIndex;
	cs_ulong32	ulGroupIp;
	cs_ulong32	ulSrcIp;
	cs_ulong32	ulVid;
	cs_uint8	smac[6];
	cs_uint16	usLLID;
	cs_uint8	sessionId[8];
	cs_long32	timerId;
	cs_long32	retrans;
}IGMP_OAM_AUTH_SESSION;

typedef struct Igmp_oam_resp
{
	cs_uint16 	msg_type;
	cs_uint16	onu_id;
	cs_uint16	vid;
	cs_ulong32	gda;
	cs_ulong32	sport;
	cs_ulong32	sip;
	cs_uint8	smac[6];
	cs_uint16	result;
}IGMP_OAM_RESP;

typedef struct Igmp_oam_req
{
	cs_uint16 	msg_type;
	cs_uint16	onu_id;
	cs_uint16	vid;
	cs_ulong32	gda;
	cs_ulong32	sport;
	cs_ulong32	sip;
	cs_uint8	smac[6];
}IGMP_OAM_REQ;

#define GWD_RETURN_OK 0
#define GWD_RETURN_ERR -1

#define GWD_OAM_THREAD_PRIORITY 22

extern cs_ulong32   gulDebugOamRx;
extern cs_ulong32   gulDebugOamTx;
extern cs_ulong32   gulDebugOamRxCount;
extern cs_ulong32   gulDebugOamTxCount;
extern cs_ulong32   gulDebugOamFileOp;

#define OAM_RX_PACKET_DEBUG(str) if( gulDebugOamRx ){ Debug_Print_Rx_OAM str ;}
#define OAM_TX_PACKET_DEBUG(str) if( gulDebugOamTx ){ Debug_Print_Tx_OAM str ;}
#define OAM_FILE_OP_DEBUG(str) if( gulDebugOamFileOp ){ cl_vty_all_out str ;}
#define UNKOWN_OAM_RX_PACKET_DEBUG(str) if( gulDebugOamRx ){ Debug_Print_Rx_OAM_Unkown str ;}

#define OAM_DATA_LEN				65535
#define OAM_OVERHEAD_LEN_STD		22	/* DA/SA/Len/Sub/Flag/Code/FCS */
#define OAM_OVERHEAD_LEN_GW			22	/* OUI/Op/Ser/WLen/POff/PLen/SnID/ */
#define OAM_MAX_FRAM_SIZE 			(106-22)	/*GW˽��֡���ɵ���󳤶� */
#define OAM_MIN_FRAM_SIZE			20	/*GW˽��֡���ɵ���С���� */

/* OAM opCode definations */
#define EQU_DEVICE_INFO_REQ			0x01	/* Device information REQUEST */
#define EQU_DEVICE_INFO_RESP		0x02	/* Device information RESPONSE */
/*#define SET_REQ                       0x03
 * #define SET_RESP                 0x04 */
#define ALARM_REQ					0x03
#define ALARM_RESP					0x04
#define FILE_READ_WRITE_REQ			0x05
#define FILE_RESERVER				0x06
#define FILE_TRANSFER_DATA			0x07
#define FILE_TRANSFER_ACK			0x08
#define CHURNING					0x09
#define DBA							0x0A

#define SNMP_TRAN_REQ 				0xB
#define SNMP_TRAN_RESP				0xC
#define SNMP_TRAN_TRAP				0xD

#define CLI_REQ_TRANSMIT			0x10
#define CLI_RESP_TRANSMIT			0x11

#define IGMP_AUTH_TRAN_REQ			0x12
#define IGMP_AUTH_TRAN_RESP 		0x13

#define CLI_PTY_TRANSMIT            0x14

#if (RPU_MODULE_IGMP_TVM == RPU_YES)
#define  IGMP_TVM_REQ                      0x16
#define  IGMP_TVM_RESP                     0x17
#endif

#define DEVICE_TYPE_GT821			0x0005	/* GT821 */
#define DEVICE_TYPE_GT831			0x0006	/* GT831 */
#define DEVICE_TYPE_GT813			0x0008	/* GT813 */
#define DEVICE_TYPE_GT812			0x0007/*7*/	/* GT812 */
#define DEVICE_TYPE_GT811			0x0004/*11 - a*/ /*4-*/	/* GT811 */
#define DEVICE_TYPE_GT810			0x000c	/* GT810 */
#define DEVICE_TYPE_GT816			0x0010	/* GT816 */
#define DEVICE_TYPE_GT821_A			0x0013	/* GT821A */
#define DEVICE_TYPE_GT831_A			0x0014	/* GT831A */
#define DEVICE_TYPE_GT812_A			0x0012/*7*/	/* GT812 */
#define DEVICE_TYPE_GT811_A			0x0011/*11 - a*/ /*4-*/	/* GT811 */
#define DEVICE_TYPE_GT865			0x000F	/* GT865 */
#define DEVICE_TYPE_GT861			0x000A	/* GT861 */
#define DEVICE_TYPE_GT815			0x0015	/* GT815 */
#define DEVICE_TYPE_UNKNOWN			0x0001	/* unknown */
#define DEVICE_TYPE_GT812PB			0x0016	/* GT812PB */
#define DEVICE_TYPE_GT831_B			0x0017	/* GT831B */
#define DEVICE_TYPE_GT866			0x0018	/* GT866 */
#define DEVICE_TYPE_GT811_B			0x0019	/* GT811_B */
#define DEVICE_TYPE_GT851			0x001A	/* GT851 */
#define DEVICE_TYPE_GT813_B			0x001B	/* GT813_B */
#define DEVICE_TYPE_GT862			0x001C	/* GT862 */
#define DEVICE_TYPE_GT892			0x001D	/* GT892 */
#define DEVICE_TYPE_GT835			0x001E	/* GT835 */
#define DEVICE_TYPE_GT831_B_CATV	0x001F	/* GT831_B_CATV */
#define DEVICE_TYPE_GT815_B			0x0020	/* GT815_B */

#define DEVICE_TYPE_GT863			0x000D	/* GT863 */ //added by wangxiaoyu 2009-05-25
#define DEVICE_TYPE_GT871B			0x0021	/* GT871 */ //added by dushaobo 2009-07-13
#define DEVICE_TYPE_GT871R                    0x0022
#define DEVICE_TYPE_GT872                       0x0025
#define DEVICE_TYPE_GT873                       0x0028
#define DEVICE_TYPE_GT870                       0x002C
#define DEVICE_TYPE_GT811D						0x002D
#define DEVICE_TYPE_GT811G                      0x002E
#define DEVICE_TYPE_GT812C						0x002F
#define DEVICE_TYPE_VALID_MAX		DEVICE_TYPE_GT812C
#define DEVICE_TYPE_VALID_MIN		DEVICE_TYPE_GT811


#define DeviceTypeIsValid( _device_type ) \
    ((_device_type)>=DEVICE_TYPE_VALID_MIN && (_device_type)<=DEVICE_TYPE_VALID_MAX)

#define MODULE_TYPE_GT861_NULL		0	/* empty */
#define MODULE_TYPE_GT861_EPON_B	1	/* EPON_B */
#define MODULE_TYPE_GT861_8POTS_A	2	/* RJ11 */
#define MODULE_TYPE_GT861_6FE		3	
#define MODULE_TYPE_GT861_8FE		4	
#define MODULE_TYPE_GT861_16FE		5	
#define MODULE_TYPE_GT861_8FXS_A	6	/* RJ11 */	
#define MODULE_TYPE_GT861_8POTS_B	7	/* RJ21 */
#define MODULE_TYPE_GT861_8FXS_B	8	/* RJ21 */
#define MODULE_TYPE_GT861_8POTSO_A	9	/* RJ11 */
#define MODULE_TYPE_GT861_8POTSO_B	10	/* RJ21 */
#define MODULE_TYPE_GT861_4E1_120	11	/* RJ21 */
#define MODULE_TYPE_GT861_4E1_75	12	/* RJ21 */


#define DEVICE_CHIP_6301			0x6301
#define DEVICE_CHIP_6201			0x6201

#define ONU_TEMPRATURE_ALARM			2	/*ONU�¶ȸ澯 */
#define ONU_ETH_PORT_STATE				10	/*ONU��̫���˿�״̬�澯 */
#define ONU_ETH_PORT_ABILITY			20	/*ONU��̫���˿����ܸ澯 */
#define ONU_ETH_WORK_STOP				21	/*ONU��̫���˿�ҵ���жϸ澯 */
#define ONU_STP_EVENT					30	/*STP�¼� */
#define ONU_DEVICE_INFO_CHANGE			100	/*ONU�豸��Ϣ�޸��¼� */
#define ONU_FILE_DOWNLOAD				50	/*ONU���ݼ����¼� */
#define ONU_DATAFILE_CHG				60	/*ONU�����ļ��޸��¼� */
#define ONU_PORT_LOOP_ALARM      		11  /*ONU or Switch port loop alarm*/
#define ONU_SWITCH_STATUS_CHANGE_ALARM  80  /*ONU�¹ҽ�������ע�����߸澯*/
#define ONU_SWITCH_STATUS_CHANGE_ALARM_LEN  14  /*ONU�¹ҽ�������ע�����߸澯��Ϣ����*/

#define ONU_INFOR_GET				1	/*ONU�豸��Ϣ��ѯ */
#define ONU_INFOR_SET				2	/*ONU�豸��Ϣ���� */
#define ONU_REALTIME_SYNC			3	/*ONUϵͳʱ��ͬ�� */
/*begin:
added by wangxiaoyu 2008-05-05
*/
#define ONU_LPB_DETECT					4	/*ONU���ؼ��*/
/*end*/
#define ONU_BOARD_GET					5	/*ONU board info get*/

#define ACCESS_IDENTIFIER     8/*ONU�û�������·��ʶ*/

#define ONU_BOARD_GET_RESP_SUCCESS		1
#define ONU_BOARD_GET_RESP_FAIL			2

#define ONU_BOARD_GET_STATUS_NULL		1
#define ONU_BOARD_GET_STATUS_INITIALIZING	2
#define ONU_BOARD_GET_STATUS_UPGRADING	3
#define ONU_BOARD_GET_STATUS_RUNNING	4
#define ONU_BOARD_GET_STATUS_ALARM		5

#define ONU_SLOT_RST_SET				6	/*Reset ONU Slot*/

#define ONU_PON_PARAMETERS_GET			7	/*ONU PON transceiver info get*/
#define ONU_GET_RESP_SUCCESS			1
#define ONU_GET_RESP_FAIL				2
/*begin:
added by wangxiaoyu 2008-05-21
modified by wangxiaoyu 2008-12-25 IP_RESOURCE_ALLOC value 10-->12
*/
#define IP_RESOURCE_ALLOC				12
#define IP_RESOURCE_FREE				11
//#endif

#define ONU_IGMP_REGISTER			1	/*ONUע�ᱨ�� */
#define ONU_IGMP_UNREGISTER			2	/*ONUע������ */
#define ONU_IGMP_LEAVE_REQ			3	/*ONUǿ���뿪���� */
#define ONU_IGMP_REGISTER_ACK		11	/*ONUע��Ӧ���� */
#define ONU_IGMP_UNREGISTER_ACK		12	/*ONUע��Ӧ���� */
#define ONU_IGMP_LEAVE_ACK			13	/*ONUǿ���뿪Ӧ���� */

#if(RPU_MODULE_VOICE == RPU_YES)		/* VM = Voice Module */
#define ONU_VM_BASIC_SET			100	/*ONU Voice module chip-enable, DA, SA set*/
#define ONU_VM_BASIC_GET			101	/*ONU Voice module chip-enable, DA, SA get*/
#define ONU_VM_VLAN_SET				102	/*ONU Voice module vlan-enable, Tag set*/
#define ONU_VM_VLAN_GET				103	/*ONU Voice module vlan-enable, Tag get*/
#define ONU_VM_PORT_EN_SET			104	/*ONU Voice module port-enable set*/
#define ONU_VM_PORT_EN_GET			105	/*ONU Voice module port-enable get*/
#define ONU_VM_PORT_STATUS_GET		106	/*ONU Voice module port status get*/
#define ONU_VM_PORT_LOOP_SET		107	/*ONU Voice module port loop set*/
#define ONU_VM_PORT_LOOP_GET		108	/*ONU Voice module port loop get*/
#define ONU_INFOR_EXT_GET			109	/*ONU extended info set*/
#define ONU_VM_TOTAL_SET			110	/*ONU Voice module all feature set*/
#define ONU_VM_TOTAL_GET			111	/*ONU Voice module all feature get*/
#endif

#define ONU_E1_LINK_SET				120	/*ONU E1 module port, port_en, DA, SA, Vlan_en, Vlan_Tag set*/
#define ONU_E1_VLAN_SET				121	/*ONU E1 module port, Vlan_en, Vlan_Tag set*/
#define ONU_E1_LOOP_SET				122	/*ONU E1 module port, loop-mode*/
#define ONU_E1_CLK_SET				123	/*ONU E1 module port, clk source*/
#define ONU_E1_ALM_MSK_SET			124	/*ONU E1 module port, alarm mask*/
#define ONU_E1_SERVICE_SET			125	/*ONU E1 module port, all para*/
#define ONU_E1_SERVICE_GET			126	/*ONU E1 module port, all para*/

#define ONU_E1_LINK_SET_LEN			20	/*Length of each link para set*/
#define ONU_E1_SERVICE_GET_LEN		26	/*Length of each port para set*/
#define ONU_MAX_E1_PORTS			16

#define E1_PORTS_PER_SLOT 4			
#define E1_SLOT_MIN 2
#define E1_SLOT_MAX 5

#define IGMP_AUTH_REQ			0x1
#define IGMP_LEAVE_REQ			0x2
#define IGMP_FRC_LEAVE			0x3
#define IGMP_AUTH_RESP			11
#define IGMP_LEAVE_RESP			12
#define IGMP_FRC_LEAVE_RESP		13

#define IGMP_AUTH_SESSION_TIMEOUT_TIME	1000	/* 1 s */

#define SOFTWARE_UPGRADE		0x01
#define FIRMWARE_UPGRADE		0x02
#define BOOT_UPGRADE			0x03
#define CONFIG_UPGRADE			0x04
#define VOICE_UPGRADE			0x05
#define FPGA_UPGRADE			0x06	/* Combination file include Voice and E1 fpga */

#define ONU_SWITCH_STATUS_CHANGE_ALM_REG    	0x01  /*type of switch status change alarm : new switch register */
#define ONU_SWITCH_STATUS_CHANGE_ALM_REREG  	0x02  /*type of switch status change alarm : switch register again */
#define ONU_SWITCH_STATUS_CHANGE_ALM_OFFLINE  	0x03  /*type of switch status change alarm : switch offline */

#define UPGRADE_RESULT_OK		0x01
#define UPGRADE_RESULT_ERROR	0x02

#define APP_IMAGE_NAME			"appimage.bin"
#define BOOT_IAMGE_NAME			"boot.bin"
#define CONFIG_FILE_NAME		"config.txt"
#define SYSLOG_FILE_NAME		"log.txt"
#define FIRMWARE_FILE_NAME		"firmware.bin"
#define VOICEAPP_IMAGE_NAME		"appvoice.bin"
#define FPGA_IMAGE_NAME			"appfpga.bin"

#define IROS_TICK_IN_MILLISECOND	10
#define IROS_TICK_PER_SECOND		100
#define WAIT_TIME_FOR_OAM_MESSAGE  	(2*IROS_TICK_PER_SECOND)	/* Ticks */

/*
 * File operation state machine
 *  Manage the file operation
 */
/* States */
#define FSM(state, event) (event + (state<<8))
#define FMST_IDLE			0			/* idle */
#define FMST_WAITDATA		1			/* process receive */
#define FMST_WAITACK		2			/* process transmit */
#define FMST_TERMINATE		3			/* terminated state */
#define FMST_FILE_TRAN_COMPLETE 4

/* Events */
#define FCMD_WRITE			0			/* receive an WRITE CMD */
#define FCMD_READ			1			/* receive an READ CMD */
#define FCMD_TO0			2			/* process 2S TIMERS' TimeOut */
#define FCMD_TO1			3			/* process 15S TIMERS' TimeOut */
#define FCMD_ACKOK			4			/* receive a SUCCESS ACK */
#define FCMD_ACKERROR		5			/* receive a ERROR INDIACT ACK */
#define FCMD_ACK303		6				/* receive a FINISH ACK */
#define FCMD_TRANSFER		7			/* receive a DATA-TRANSFER PACKET */
#define FCMD_TERMINATE		8			/* receive a TERMINATE CMD, clear the machine */
#define FCMD_UST_ERROR		9			/* retransmit software update error */
#define FCMD_UST_OK		10			/* retransmit software update ok */
#define FCMD_UVST_ERROR	11			/* retransmit voip software update error */
#define FCMD_UVST_OK		12			/* retransmit voip software update ok */
#define FCMD_RECVALAMRESP	13			/* receive alarm response */
#define FCMD_UFPGA_ERROR	14			/* retransmit FPGA update error */
#define FCMD_UFPGA_OK		15			/* retransmit FPGA update ok */

#define E1_FPGA_UPDATE_RESULT_SUCCESS 	0
#define E1_FPGA_UPDATE_RESULT_NOMEM 	1
#define E1_FPGA_UPDATE_RESULT_SEM_FAIL 	2
#define E1_FPGA_UPDATE_RESULT_NOE1 		3
#define E1_FPGA_UPDATE_RESULT_SLOT_ERR	4
#define E1_FPGA_UPDATE_RESULT_MAN_MAC	5
#define E1_FPGA_UPDATE_RESULT_NOQUEUE	6
#define E1_FPGA_UPDATE_RESULT_HW_FAIL	7
#define E1_FPGA_UPDATE_RESULT_MSG_FAIL	8
#define E1_FPGA_UPDATE_RESULT_TIMEOUT	9
#define E1_FPGA_UPDATE_RESULT_QUEUE_FAIL	10
#define E1_FPGA_UPDATE_RESULT_STATE_ERR	11
#define E1_FPGA_UPDATE_RESULT_MSG_ERR	12
#define E1_FPGA_UPDATE_RESULT_NOTINIT 	13
#define E1_FPGA_UPDATE_RESULT_READ_FILE	14
#define E1_FPGA_UPDATE_RESULT_TASK		15
#define E1_FPGA_UPDATE_RESULT_FILE_ERR	16
#define E1_FPGA_UPDATE_RESULT_PORT_DOWN	17
#define E1_FPGA_UPDATE_RESULT_PORT_ERR	18

/* Defines */
#ifdef _OAM_FILE_LENGTH_EXCEED_2M_
#define MAX_FILE_WRITE_LEN	0x400000 /*0x2C0000*/	/* 2.0 M */
#else
#define MAX_FILE_WRITE_LEN	0x200000 /*0x2C0000*/	/* 2.0 M */
#endif
#define OAM_DATA_MTU	OAM_MAX_FRAM_SIZE
#define OAM_FILE_OP_BASE_TIME		1000 	/* MS */
/* ACK Codes */
#define READ_DENY           0x100		/*  ���ܾ� */
#define READ_ACCEPT	    	0x101		/*  ������ */
#define WRITE_DENY         0x200		/*  д�ܾ� */
#define WRITE_ACCEPT     	0x201		/*  д���� */
#define TRANS_ERROR       	0x300		/*  ���ʹ��� */
#define TRANS_START      	0x301		/*  ���Ϳ�ʼ�����͹��̿���Ӧ��Ҳ��ʾ���������� */
#define TRANS_DOING      	0x302		/*  �����У����͹��̿���Ӧ��Ҳ��ʾ���������� */
#define TRANS_DONE        	0x303		/*  ���ͽ��������͹��̿���Ӧ��Ҳ��ʾ���������� */

/* ACK ERROR Codes */
#define SYS_NOERROR			0x00		/*  �޴��� */
#define SYS_BUSY			0x01		/*  ϵͳæ */
#define SYS_NORESOURCE 	0x02			/*  ϵͳ��Դ���� */
#define SYS_PROCESSERR		0x03		/*  ϵͳ������� */
#define SYS_PROTOERR		0x04		/*  ���̴��� */
#define SYS_NOSUCHFILE		0x05		/*  �ļ������� */
#define SYS_FILETOOLONG	0x06			/*  �ļ�̫�� */
#define SYS_FILETOOSHORT	0x07		/*  �ļ�̫�� */
#define SYS_FILEOPERR		0x08		/*  ���Ȼ�ƫ��ƥ����� */
#define SYS_FILECKERR		0x09		/*  ����У����� */
#define SYS_FILESAVEERR		0x0A		/*  �ļ�������� */

#define FILE_OP_PACKET		0x01
#define FILE_OP_COMMAND		0x02
#define CLI_PACKET			0x03
#define SNMP_PACKET			0x04
#define FILE_OP_TIMERCHECK	0x05


#define PTY_PACKET       	0x06
#define PTY_NOTI_MSG        0x07
#define PTY_TIMER_MSG       0x08
#define PTY_ONU_LOSE        0x09

#if (RPU_MODULE_IGMP_TVM == RPU_YES)
enum IGMP_TVM_OAM_ENABLE
{
    IGMP_TVM_ENABLE = 1,
    IGMP_TVM_DISABLE = 2
};

enum IMGP_TVM_OAM_TYPE
{
    IGMP_TVM_ADD = 1,
    IGMP_TVM_DEL = 2,
    IGMP_TVM_UPDATE = 3,
    IGMP_TVM_DEL_BY_VLAN = 4,
    IMGP_TVM_MAP_REQ = 5
};

#endif

typedef struct _file_op_session_ctl_block
{
	struct _file_op_session_ctl_block *next;	/* all session blocks are linked */
	cs_int8    session[8];				/* the session id */
	cs_int8    filename[128];				/* the file to operate */
	cs_int32     filelen;						/* the file length in last packet */
	cs_int8   *buffer;					/* the buffer that contains the file */
	cs_int32     buflen;						/* the buffer length */
	cs_int32     datalen;					/* data length in last packet */
	cs_int32     pktlen;						/* current packet len */
	cs_int32     cur_offset;					/* offset into the buffer in last packet */
	cs_int32     exp_offset;					/* the offset expired */
	cs_int32     operation;					/* 0 for read, 1 for write */
	cs_int32     state;						/* current state of the state machine */
	cs_int32     nextstate;					/* next state of the state machine */
	cs_int32     timer0;						/* 2s timer */
	cs_int32     timer1;						/* 15s timer */
	cs_int32     AlarmRetran;				/*�����ļ�������Alarm�ش� ���� */
	cs_ulong32 senderSerNo;		/* Sender's serial no, we used when we response */
	cs_int8   *rcv_pkt;					/* received packet */
	cs_int8   *retrans;					/* the packet waitting for retransmit */
	cs_long32  timerId;					/* protocol timer base */
	cs_long32  loadStatus;
} FILE_OP_SESSION_CTL_BLOCK;

#define GET_SHORT( _BUF )    ((((cs_uint16)(((cs_uint8 *)(_BUF))[0])) << 8) | (((cs_uint16)(((cs_uint8 *)(_BUF))[1])) << 0))
#define GET_LONG( _BUF )    ((((cs_ulong32)(((cs_uint8 *)(_BUF))[0])) << 24) | (((cs_ulong32)(((cs_uint8 *)(_BUF))[1])) << 16) | (((cs_ulong32)(((cs_uint8 *)(_BUF))[2])) << 8) | (((cs_ulong32)(((cs_uint8 *)(_BUF))[3])) << 0))
#define SET_LONG( _BUF, value)	\
{ \
    cs_ulong32 _ulValue;\
    _ulValue = (cs_ulong32)(htonl(value));\
    memcpy(_BUF,&_ulValue,4);\
}
#define GET_LONG_OP( _BUF )    ((((cs_ulong32)(((cs_uint8 *)(_BUF))[3])) << 24) | (((cs_ulong32)(((cs_uint8 *)(_BUF))[2])) << 16) | (((cs_ulong32)(((cs_uint8 *)(_BUF))[1])) << 8) | (((cs_ulong32)(((cs_uint8 *)(_BUF))[0])) << 0))

#define SET_SHORT( _BUF, value)	\
{ \
    cs_uint16 _ulValue;\
    _ulValue = (cs_uint16)(htons(value));\
    memcpy(_BUF,&_ulValue,2);\
}

/* Op-Code for CTC extend OAM */
#define Extended_Variable_Request		0x1	/* ����OLT ��ONU ��ѯ��չ���� */
#define Extended_Variable_Response		0x2	/* ����ONU��OLT ������չ���� */
#define Extended_Variable_Set_Request	0x3	/* ����OLT ��ONU ������չ����/���� */
#define Extended_Variable_Set_Response	0x4	/* ����ONU��OLT���ض���չ����/�������õ�ȷ�� */
#define Extended_Variable_Churning		0x5	/* ��Triply-Churning ��ص���Կ���� */
#define Extended_Variable_DBA			0x6	/* DBA �����������ѯ */

/* Branch ID for CTC */
#define Branch_Standard_Attribute1		0x07	/* IEEE 802.3 Clause 30�涨�ı�׼���� */
#define Branch_Standard_Attribute2		0x09	/* IEEE 802.3 Clause 30�涨�Ĳ������� */
#define Branch_Ctc_Extended_Attribute1	0xc7	/* CTC��չ�����ԣ�����ִ��Get��(��)Set���� */
#define Branch_Ctc_Extended_Attribute2	0xc9	/* CTC��չ�Ĳ��� */
#define Branch_Instance_Index			0x36	/* ʵ�����������Ϊʵ������ */

/* Leaf */
#define Leaf_Index_Port			0x0001	/* �˿�ʵ��������leafֵ */
#define Leaf_ONU_SN				0x0001	/* ONU�ı�ʶ�� */
#define Leaf_FirmwareVer		0x0002	/* ONU�Ĺ̼��汾 */
#define Leaf_ChipsetID			0x0003	/* ONU��PONоƬ���̺Ͱ汾 */
#define Leaf_ONU_Capabilities		0x0004	/* ONU�Ķ˿ڡ����� */

#define Leaf_EthLinkState			0x0011	/* ��̫���û��˿ڵ���·״̬ */
#define Leaf_EthPortPause		0x0012	/* ��̫���˿ڵ����ع��ܼ����� */
#define Leaf_EthPortPolicing		0x0013	/* ��̫���˿ڵ����ٹ��ܣ����У� */
#define Leaf_VoIP_Port			0x0014	/* VoIP�˿ڹ��� */

#define Leaf_VLAN				0x0021	/* ONU��VLAN���� */

#define Leaf_ClassMarking		0x0031	/* ҵ������������ */

#define Leaf_Add_Del_Multicast_VLAN	0x0041	/* ONU����̫���˿ڵ��鲥VLAN���� */
#define Leaf_MulticastTagStripe	0x0042	/* ONU������Multicast���ݱ��ĵ�VLAN TAG���� */
#define Leaf_MulticastSwitch		0x0043	/* �鲥Э�鿪�� */
#define Leaf_MulticastControl		0x0044	/* ����Ƶ�����鲥ҵ����� */
#define Leaf_Group_Num_Max		0x0045	/* ONU��˿�ͬʱ֧�ֵ��鲥������ */

/* Leaf,  branch 0x07/0x09 */
#define Leaf_aPhyAdminState		0x0025	/* ��ѯ��̫���˿ڵ�״̬, 0x07 */
#define Leaf_acPhyAdminControl	0x0005	/* ���û������̫������˿ڵ�״̬, 0x09 */
#define Leaf_aAutoNegAdminState	0x004f	/* ��̫���˿ڵ�״̬����Э�̣�, 0x07 */
#define Leaf_aAutoNegLocalTechnologyAbility	0x0052	/* actual port capabilities, 0x07 */
#define Leaf_aAutoNegAdvertisedTechnologyAbility	0x0053	/* �˿���Э������ͨ��, 0x07 */
#define Leaf_acAutoNegRestartAutoConfig	0x000b	/* ǿ����·����Э��, 0x09 */
#define Leaf_acAutoNegAdminControl	0x000c	/* �򿪻��߹ر�PHY�˿ڵ���Э�̹���, 0x09 */
#define Leaf_aFECAbility			0x0139	/* FEC������ѯ��IEEE 802.3-2005 Clause 30.5.1.1.13��, 0x07 */
#define Leaf_aFECmode			0x013a	/* ˫��FEC���ܵĴ�/�رգ�IEEE 802.3-2005 Clause30.5.1.1.14��, 0x07 */

#define TLV_SET_OK				0x80	/* ��set variable request�������Action����ȷ�� */
#define TLV_SET_PARAM_ERR		0x86	/* ��������Set Request���������Action���Ĳ�����Ч */
#define TLV_SET_ERR				0x87	/* ��������Set Request���������Action���Ĳ�����Ч����ONU�ĵ�ǰ״̬ʹ�ò����޷���� */

#define CLASS_MARK_DEL			0x00	/* ɾ��������Classification��Queuing&Marking���ƹ�������Set Variable Request��Ϣ��*/
#define CLASS_MARK_ADD			0x01	/* ����������Classification��Queuing&Marking���ƹ�������Set Variable Request��Ϣ��*/
#define CLASS_MARK_CLR			0x02	/* ���ONU��Classification��Queuing&Marking���Ʊ���ɾ����ONU����
										    �ķ��ࡢ�ŶӺͱ�ǹ��򣩣��ò������ͽ�����Set Variable
										    Request��Ϣ������containerΪ�˲�������ʱ�����ֽں���û����������*/
#define CLASS_MARK_GET			0x03	/* �г���ONU���е�Classification��Queuing&Marking������Ŀ����
										     ��Get Variable Request/Response��Ϣ)������container����Get Variable Requestʱ��
										     ���ֽں���û���������ݣ�����container����Get Variable Responseʱ��
										     ���ֽں���Ϊ�ö˿ڵ����з��ࡢ�ŶӺͱ�ǹ���*/
#define CTC_OAM_RESPONSE_COPY(dest, src, len, proclen)  \
{ \
	CTC_OAM_MESSAGE_NODE *_resp ;  \
	_resp = (CTC_OAM_MESSAGE_NODE *)(dest); \
	VOS_MemCpy((_resp->pPayLoad+_resp->PayloadLen), (src), (len)); \
	_resp->PayloadLen += (len); \
	(proclen) += len; \
}

#define CTC_OAM_SETMSG_RESP(msg, tlv, err) \
{ \
	CTC_OAM_MESSAGE_NODE *_resp; \
	TLV_VARIALBE_INDICATION *_tlv_ind; \
	_resp = (CTC_OAM_MESSAGE_NODE *)msg; \
	_tlv_ind = (TLV_VARIALBE_INDICATION *)(_resp->pPayLoad+_resp->PayloadLen); \
	_tlv_ind->branch = (tlv)->branch; \
	_tlv_ind->leaf = (tlv)->leaf; \
	_tlv_ind->var = (err); \
	_resp->PayloadLen += sizeof(TLV_VARIALBE_INDICATION); \
}

/* The return value is only one cs_int8 */
#define CTC_OAM_REQMSG_UCHAR_RESP(msg, tlv, val) \
{ \
	CTC_OAM_MESSAGE_NODE *_resp; \
	TLV_VARIALBE_CONTAINER *_tlv; \
	_resp = (CTC_OAM_MESSAGE_NODE *)msg; \
	_tlv = (TLV_VARIALBE_CONTAINER *)(_resp->pPayLoad+_resp->PayloadLen); \
	_tlv->branch = (tlv)->branch; \
	_tlv->leaf = (tlv)->leaf; \
	_tlv->width = 0x01; \
	_tlv->value[0] = (cs_uint8)(val); \
	_resp->PayloadLen += sizeof(TLV_VARIALBE_CONTAINER); \
}

#define GET_TRIPLE_CHAR_LONG(addr, value) \
{ \
	cs_ulong32 _var = 0; \
	((cs_uint8*)&(_var))[0] = (addr)[0]; \
	((cs_uint8*)&(_var))[1] = (addr)[1]; \
	((cs_uint8*)&(_var))[2] = (addr)[2]; \
	value = VOS_NTOHL(_var); \
}

#define SET_TRIPLE_CHAR_LONG(addr, value) \
{ \
	cs_ulong32 _var = VOS_HTONL(value); \
	(addr)[0] = ((cs_uint8 *) & (_var))[0]; \
	(addr)[1] = ((cs_uint8 *) & (_var))[1]; \
	(addr)[2] = ((cs_uint8 *) & (_var))[2]; \
}

#define CTC_OAM_REQMSG_UCHAR_ULONG_RESP(msg, tlv, val, data) \
{ \
	CTC_OAM_MESSAGE_NODE *_resp = (CTC_OAM_MESSAGE_NODE *)msg; \
	TLV_VARIALBE_CONTAINER *_tlv = (TLV_VARIALBE_CONTAINER *)(_resp->pPayLoad+_resp->PayloadLen); \
	_tlv->branch = (tlv)->branch; \
	_tlv->leaf = (tlv)->leaf; \
	_tlv->width = 0x05; \
	_tlv->value[0] = (cs_uint8)(val); \
	_resp->PayloadLen += sizeof(TLV_VARIALBE_CONTAINER); \
	*((cs_ulong32 *)(_resp->pPayLoad+_resp->PayloadLen)) = VOS_HTONL(data); \
	_resp->PayloadLen += sizeof(cs_ulong32); \
}

#define CTC_OAM_REQMSG_UCHAR_XULONG_RESP(msg, tlv, val, pdata, len) \
{ \
	cs_uint32 iTmp; \
	CTC_OAM_MESSAGE_NODE *_resp = (CTC_OAM_MESSAGE_NODE *)msg; \
	TLV_VARIALBE_CONTAINER *_tlv = (TLV_VARIALBE_CONTAINER *)(_resp->pPayLoad+_resp->PayloadLen); \
	_tlv->branch = (tlv)->branch; \
	_tlv->leaf = (tlv)->leaf; \
	_tlv->width = 0x01+len*sizeof(cs_ulong32); \
	_tlv->value[0] = (cs_uint8)(val); \
	_resp->PayloadLen += sizeof(TLV_VARIALBE_CONTAINER); \
	for (iTmp=0; iTmp<len; iTmp++) \
	{ \
		*((cs_ulong32 *)(_resp->pPayLoad+_resp->PayloadLen)) = VOS_HTONL(pdata[iTmp]); \
		_resp->PayloadLen += sizeof(cs_ulong32); \
	} \
}

/*
** Macros for Multicast Group Control
*/
#define MGROUP_DELETE	0x00
#define MGROUP_ADD		0x01
#define MGROUP_DELALL	0x03
#define MGROUP_GET		0x04

typedef struct Mulicast_Control_Entry
{
	cs_uint16	usCVid;
	cs_uint16	usMVid;
	cs_uint8	ucGMAC[6];
}__attribute__((packed))MULICAST_CONTROL_ENTRY;

typedef struct Mulicast_Group_Entry
{
	cs_uint16	usCVid;
	cs_uint16	usMVid;
	cs_uint8	ucGMAC[6];
	cs_ulong32	ulPort;
	struct Mulicast_Group_Entry *next;
}__attribute__((packed))MULICAST_GROUP_ENTRY;

/*begin:
added by wangxiaoyu 2008-05-05
���ز���OAM ��
*/
typedef struct Oam_Onu_Lpb_Detect_Frame{
cs_uint8	type;		//�������ͣ�4��ʾΪ���ؼ��
cs_uint8	result;		//���Խ������ONU�����
cs_uint8	enable;		//�Ƿ�ʹ�ܹ��ܣ���OLT�����
cs_uint16	vid;			//���в��Ե�VLAN, 0:ONU��������VLAN
cs_uint8	smac[6];	//�����õ�ԴMAC
cs_uint16	interval;		//OLT������֡�ļ��ʱ��s
cs_uint16	policy;		//���򣬼��Ƿ�رն˿�
/*added by wangxiaoyu 2009-03-11*/
cs_uint16  waitforwakeup; //�ȴ����ѵ����ڣ�Ϊ��ѯ���ڵı���
cs_uint16  maxwakeup;		//����������Դ���
}__attribute__((packed))OAM_ONU_LPB_DETECT_FRAME;
/*end*/

typedef struct loop_detect_frame
{
	cs_uint8 Destmac[6];
        cs_uint8 Srcmac[6];
        cs_uint16 Tpid;
        cs_uint8 Vid[2];
        cs_uint16 Ethtype;
        cs_uint16 LoopFlag;
        cs_uint8 OltType;
        cs_uint8 OnuType;
        cs_uint8 OnuLocation[4];
        cs_uint8 Onumac[6];
        cs_uint16 OnuVid;
        cs_ulong32 Onuifindex;
} __attribute__ ((packed)) LOOP_DETECT_FRAME;

/*jiangxt 20111010.*/
typedef struct loop_detect_frame_data
{
        cs_uint16 Ethtype;
        cs_uint16 LoopFlag;
        cs_uint8 OltType;
        cs_uint8 OnuType;
        cs_uint8 OnuLocation[4];
        cs_uint8 Onumac[6];
        cs_uint16 OnuVid;
        cs_ulong32 Onuifindex;
}__attribute__ ((packed)) LOOP_DETECT_FRAME_DATA;
/*add end*/

typedef struct alarm_loop
{
	cs_uint8 alarmFlag;				
	cs_uint8 portNum[4];				
	cs_uint8 loopstate;					
	cs_uint16 vlanid;				
	cs_uint8 switchMac[6];
	cs_uint8 externFlag;	       
        cs_uint8 oltType;
        cs_uint8 oltMac[6];
        cs_uint8 onuLocation[4];
        cs_uint8 onuType;
        cs_uint8 onuMac[6];
        cs_uint8 onuPort[4];
} __attribute__ ((packed)) ALARM_LOOP;

#if(PRODUCT_CLASS == PRODUCTS_GT811D || PRODUCT_CLASS == PRODUCTS_GT811G ||	PRODUCT_CLASS == PRODUCTS_GT873_M_4F4S)
#define NUM_PORTS_PER_SYSTEM 5
#elif(PRODUCT_CLASS == PRODUCTS_GT812C)
#define NUM_PORTS_PER_SYSTEM 9
#endif

#define IFM_ETH_ALARM_STATUS_LOOP     0x08

/*added by wangxiaoyu 2009-03-11*/
#define LPB_OLD_VER_LEN	(sizeof(OAM_ONU_LPB_DETECT_FRAME)-4)

#define ETH_TYPE_LOOP_DETECT  0x0800
#define LOOP_DETECT_CHECK 0x0080

typedef struct Oam_Onu_Lpb_Detect_Ctrl{
cs_uint16	vid;				//��һ�λ��ؼ��vlan
cs_uint8	lpbnum;			//��һ�β鵽�Ļ��ض˿���
cs_uint8	lpbmask[NUM_PORTS_PER_SYSTEM+1];			//��һ�εĻ��ض˿�״̬��¼(1~15bit������15���˿ڵĻ���״̬
cs_uint8   lpbportdown[NUM_PORTS_PER_SYSTEM+1];	//�Ƿ�ر� 0:û�йر� 1:�Ѿ��ر�
cs_uint8	lpbStateChg[NUM_PORTS_PER_SYSTEM+1];	//�澯״̬�仯λ added by wangxiaoyu 2009-03-17
cs_uint8	lpbportwakeupcounter[NUM_PORTS_PER_SYSTEM+1];			//��һ�ε�OAM����ʹ��״̬
cs_uint8   lpbClearCnt[NUM_PORTS_PER_SYSTEM+1];
cs_int32				slpcounter[NUM_PORTS_PER_SYSTEM+1];
ALARM_LOOP		alarmInfo[NUM_PORTS_PER_SYSTEM+1];
}__attribute__((packed))OAM_ONU_LPB_DETECT_CTRL;

typedef struct lpb_ctrl_list{
	OAM_ONU_LPB_DETECT_CTRL *ctrlnode;
	struct lpb_ctrl_list *next;
}LPB_CTRL_LIST;

#define   VOS_LITTLE_ENDIAN			1

typedef union tagIFM_ETH_IfIndex
{
    struct tagPhy
    {
#if VOS_LITTLE_ENDIAN
        unsigned subif:12;
        unsigned port:6;
        unsigned slot:5;
        unsigned subcard:3;
        unsigned type:6;
#else
        unsigned type:6;
        unsigned subcard:3;
        unsigned slot:5;
        unsigned port:6;
        unsigned subif:12;
#endif		
    }phy_slot_port;
    cs_ulong32 ulPhyIfIndex;
} IFM_ETH_IF_INDEX_U;

#define MAX_GWD_OLT_SLOT 24
#define MAX_GWD_OLT_PORT 16

/* begin:added loop-detect onu config default value by dushb 2009-10-30 */
#define LOOP_DETECT_MODE_OLT               1
#define LOOP_DETECT_MODE_DISABLE        0
#define LOOP_DETECT_LOCAL_DFT              0
#define LOOP_DETECT_CONTROL_DFT        1
#define LOOP_DETECT_VLAN_DFT                 0
#define LOOP_DETECT_INTERVAL_DFT         30
#define LOOP_DETECT_WAKEUP_DFT           3
#define LOOP_DETECT_THRESHOLD_DFT     3
/* end */
#define OAM_ALM_SLOT_ID							3 
#define OAM_ALM_LNKCHG_ID						10 
#define OAM_ALM_PON_ID							25 
#define OAM_ALM_E1_ID							40 
#define OAM_ALM_SAMPLE_PKT_ID					4 

#define OAM_ALM_PON_TRANSMISSION_POWER_HIGH_ID	1 
#define OAM_ALM_PON_TRANSMISSION_POWER_LOW_ID	2 
#define OAM_ALM_PON_RECEIVER_POWER_HIGH_ID		3 
#define OAM_ALM_PON_RECEIVER_POWER_LOW_ID		4 
#define OAM_ALM_PON_APPLIED_VOLTAGE_HIGH_ID		5 
#define OAM_ALM_PON_APPLIED_VOLTAGE_LOW_ID		6 
#define OAM_ALM_PON_BIAS_CURRENT_HIGH_ID		7 
#define OAM_ALM_PON_BIAS_CURRENT_LOW_ID			8 
#define OAM_ALM_PON_MODULE_TEMPERATURE_HIGH_ID	9
#define OAM_ALM_PON_MODULE_TEMPERATURE_LOW_ID	10 
#define OAM_ALM_PON_LASER_ALWAYS_ON_ID			11 

#define DFT_PONRECEIVERPOWERLOW			-30
#define DFT_PONRECEIVERPOWERHIGH		-6
#define DFT_PONTRANSMISSIONPOWERLOW		1
#define DFT_PONTRANSMISSIONPOWERHIGH	50
#define DFT_PONMODULETEMPERATURELOW		-40
#define DFT_PONMODULETEMPERATUREHIGH	50
#define DFT_PONAPPLIEDVOLTAGELOW		30
#define DFT_PONAPPLIEDVOLTAGEHIGH		50
#define DFT_PONBIASCURRENTLOW     		1
#define DFT_PONBIASCURRENTHIGH			500

#define	DHCP_OPTION82_RELAY_MODE_CTC	1
#define DHCP_OPTION82_RELAY_MODE_STD	2

#define LENGTH_OF_VERSION_STRING		32
#define LENGTH_OF_VENDOR_STRING			128
#define LENGTH_OF_SN_STRING				32
#define LENGTH_OF_DATE_STRING			32

/* Onu E1 alarm flag */
#define ONU_ALM_E1_OOS		0x8000
#define ONU_ALM_E1_LOS		0x0080
#define ONU_ALM_E1_LOF		0x0040
#define ONU_ALM_E1_AIS		0x0020
#define ONU_ALM_E1_RAI		0x0010
#define ONU_ALM_E1_SMF		0x0008
#define ONU_ALM_E1_LOFSMF	0x0004
#define ONU_ALM_E1_CRC3		0x0002
#define ONU_ALM_E1_CRC6		0x0001

#define NUM_UNITS_PER_SYSTEM    1
#define PHY_PORT_MAX 			11

#define PHY_PORT_FE0 0

typedef struct log_phy_map_s {
    cs_uint8 unit;
    cs_uint8 physical_port;
} log_phy_map_t;

/*begin: added by wangxiaoyu 2008-12-26*/
enum{
	NO_UPDATE_SESSION,
	FTP_UPDATE_SESSION,
	OAM_UPDATE_SESSION
};

/*end*/

#define TASK_PRIORITY_LOWEST                  26

#define IFM_ETH_TYPE      (0x1)

#if 0
#define FLASH_USER_DATA_MAX_SIZE     	(1024*20)
/*#define SLOW_PATH_CFG_OFFSET            (1024*19+512)*/
#define GWD_PRODUCT_CFG_OFFSET          (1024*19+512+64) /* follow slow path config */
#endif

/* Note: adjust reserved section , not change the TYPE size. If you change the TYPE size, not change 
   the valid_flag position for keep the data of previous version remain valid */ 
typedef struct onu_ablity_info{
	cs_uint8 type;
	cs_uint8 leng;
	cs_uint8 val;
}ONU_SYS_INFO_ABILITY;
typedef struct onu_system_information_total
{
	cs_uint16 	product_serial;					/* 2 */
	cs_uint16	product_type;					/* 2 */
	cs_uint8	device_name[127];				/* 16 */
	cs_uint8	sw_version[12];					/* 12 */
	cs_uint8	serial_no[18];					/* 18 */
	cs_uint8	hw_version[6];					/* 6 */
	cs_uint8	hw_manufature_date[12];			/* 12 */
	cs_uint8    auto_requ[2];					/*2*/    
														/* 68 */
	struct	_pon_transceiver_info_
	{
		cs_uint8 	available	: 1;
		cs_uint8	reach 		: 1;
		cs_uint8	package 	: 1;
		cs_uint8	protectEn 	: 1;
		cs_uint8	manufacturer 	: 4;
	}pon_transceiver_info;							/* 1 */
						
	cs_uint8	TDM_module_info;				/* 1 */
	cs_uint8 	VoIP_module_info;				/* 1 */
	cs_uint8	reserved_1;						/* 1 */
														/* 72 */
	cs_uint8	mac_addr_primary[6];			/* 6 */
	cs_uint8  	mac_addr_second[6];				/* 6 */
														/* 84 */
	cs_ulong32	ip_addr;						/* 4 */
	cs_ulong32  	net_mask;						/* 4 */
														/* 92 */
	cs_ulong32	vos_image_size;					/* 4 */
														/* 96 */
	cs_uint8	loadstartupconfig;				/* 1 */
	cs_uint8	hardwarediagnosis;				/* 1 */
	cs_uint8	reserved_2[2];					/* 2 */
														/* 100 */
	cs_ulong32	reserved_3[64];					/* 256 */
														/* 356 */
	cs_uint8 	reserved_4[2];					/* 2 */
	cs_uint8 	reboot;							/* 1 */
	cs_uint8	valid_flag;						/* 1 */ /* Total: 360 */
}ONU_SYS_INFO_TOTAL;

typedef enum {
        EPON_INDEPENDENT_VLAN_LEARNING = 1,
        EPON_SHARED_VLAN_LEARNING
} epon_vlan_learning_t;


cs_uint16 oam_sdl_get_llid();
typedef enum
{     
    FDB_ENTRY_INVALID,     /* the entry is no longer valid but not flushed yet */    
    FDB_ENTRY_LEARNED,    /* dynamically learned */
    FDB_ENTRY_STATIC,
    FDB_ENTRY_MGMT, 
    FDB_ENTRY_PRIOVERRIDE
} epon_sw_fdb_entry_status_t;

typedef struct
{
    cs_uint32       index;

    epon_macaddr_t    addr;
    cs_uint16     vlan_id;
    cs_uint32       egress_portmap;

    epon_sw_fdb_entry_status_t  status;
    cs_uint32       private_data;
} epon_sw_fdb_entry_t;

#if 0
typedef struct enet_format_s
{
	/* edge0 */
	cs_ulong32  en_mac_dhost_hi;
	/* edge1 */
	cs_uint16  en_mac_dhost_lo;
	cs_uint16  en_mac_shost_hi;
	/* edge2 */
	cs_ulong32  en_mac_shost_lo;
	/* edge3 */
	cs_uint16  _tpid;						/* Tag Prot ID (0x8100) */
	cs_uint16  en_tag_ctrl;				/* Tag control */
	/* edge4 */
	cs_uint16  en_pro_II;
	cs_uint16  en_ver;
	/* edge5 */
	cs_uint16  en_tot_len;
	cs_uint16  en_frag_id;
	/* edge6 */
	cs_uint16  en_flag;
	cs_uint8   en_ttl;
	cs_uint8   en_pro;						/* TCP/UDP */
	/* edge7 */
	cs_uint16  en_chk_sum;
	cs_uint16  en_ip_shost_hi;
	cs_uint16  en_ip_shost_lo;
	cs_uint16  en_ip_dhost_hi;
	cs_uint16  en_ip_dhost_lo;
	cs_uint16  en_l4_sport;
	cs_uint16  en_l4_dport;
} enet_format_t;
#endif


#if 1
typedef struct{
	cs_uint64 gulBcStormThreshold;
	cs_uint64 gulBcStormStat;
}broadcast_storm_s;

#endif


#endif
