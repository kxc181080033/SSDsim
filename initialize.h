/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName�� initialize.h
Author: Hu Yang		Version: 2.1	Date:2011/12/02
Description: 

History:
<contributor>     <time>        <version>       <desc>                   <e-mail>
Yang Hu	        2009/09/25	      1.0		    Creat SSDsim       yanghu@foxmail.com
                2010/05/01        2.x           Change 
Zhiming Zhu     2011/07/01        2.0           Change               812839842@qq.com
Shuangwu Zhang  2011/11/01        2.1           Change               820876427@qq.com
Chao Ren        2011/07/01        2.0           Change               529517386@qq.com
Hao Luo         2011/01/01        2.0           Change               luohao135680@gmail.com
*****************************************************************************************************************************/
#ifndef INITIALIZE_H
#define INITIALIZE_H 10000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include "avlTree.h"

#define SECTOR 512
#define BUFSIZE 200

#define DYNAMIC_ALLOCATION 0
#define STATIC_ALLOCATION 1

#define INTERLEAVE 0
#define TWO_PLANE 1

#define NORMAL    2
#define INTERLEAVE_TWO_PLANE 3
#define COPY_BACK	4

#define AD_RANDOM 1
#define AD_COPYBACK 2
#define AD_TWOPLANE 4
#define AD_INTERLEAVE 8
#define AD_TWOPLANE_READ 16

#define READ 1
#define WRITE 0

/*********************************all states of each objects************************************************
*һ�¶�����channel�Ŀ��У������ַ���䣬���ݴ��䣬���䣬������״̬
*����chip�Ŀ��У�дæ����æ�������ַ���䣬���ݴ��䣬����æ��copybackæ��������״̬
*���ж�д������sub���ĵȴ����������ַ���䣬���������ݴ��䣬д�����ַ���䣬д���ݴ��䣬д���䣬��ɵ�״̬
************************************************************************************************************/

#define CHANNEL_IDLE 000
#define CHANNEL_C_A_TRANSFER 3
#define CHANNEL_GC 4           
#define CHANNEL_DATA_TRANSFER 7
#define CHANNEL_TRANSFER 8
#define CHANNEL_UNKNOWN 9

#define CHIP_IDLE 100
#define CHIP_WRITE_BUSY 101
#define CHIP_READ_BUSY 102
#define CHIP_C_A_TRANSFER 103
#define CHIP_DATA_TRANSFER 107
#define CHIP_WAIT 108
#define CHIP_ERASE_BUSY 109
#define CHIP_COPYBACK_BUSY 110
#define UNKNOWN 111

#define SR_WAIT 200                 
#define SR_R_C_A_TRANSFER 201
#define SR_R_READ 202
#define SR_R_DATA_TRANSFER 203
#define SR_W_C_A_TRANSFER 204
#define SR_W_DATA_TRANSFER 205
#define SR_W_TRANSFER 206
#define SR_COMPLETE 299

#define REQUEST_IN 300         //��һ�����󵽴��ʱ��
#define OUTPUT 301             //��һ�����������ʱ��

#define GC_WAIT 400
#define GC_ERASE_C_A 401
#define GC_COPY_BACK 402
#define GC_COMPLETE 403
#define GC_INTERRUPT 0
#define GC_UNINTERRUPT 1

#define CHANNEL(lsn) (lsn&0x0000)>>16      
#define chip(lsn) (lsn&0x0000)>>16 
#define die(lsn) (lsn&0x0000)>>16 
#define PLANE(lsn) (lsn&0x0000)>>16 
#define BLOKC(lsn) (lsn&0x0000)>>16 
#define PAGE(lsn) (lsn&0x0000)>>16 
#define SUBPAGE(lsn) (lsn&0x0000)>>16  

#define PG_SUB 0xffffffff		

int *count;   //KXC:add a tabel to record the frequency

/*****************************************
*�������״̬����
*Status �Ǻ������ͣ���ֵ�Ǻ������״̬����
******************************************/
#define TRUE		1
#define FALSE		0
#define SUCCESS		1
#define FAILURE		0
#define ERROR		-1
#define INFEASIBLE	-2
#define OVERFLOW	-3
typedef int Status;     

struct ac_time_characteristics{
	int tPROG;     //program time
	int tDBSY;     //bummy busy time for two-plane program
	int tBERS;     //block erase time
	int tCLS;      //CLE setup time
	int tCLH;      //CLE hold time
	int tCS;       //CE setup time
	int tCH;       //CE hold time
	int tWP;       //WE pulse width
	int tALS;      //ALE setup time
	int tALH;      //ALE hold time
	int tDS;       //data setup time
	int tDH;       //data hold time
	int tWC;       //write cycle time
	int tWH;       //WE high hold time
	int tADL;      //address to data loading time
	int tR;        //data transfer from cell to register
	int tAR;       //ALE to RE delay
	int tCLR;      //CLE to RE delay
	int tRR;       //ready to RE low
	int tRP;       //RE pulse width
	int tWB;       //WE high to busy
	int tRC;       //read cycle time
	int tREA;      //RE access time
	int tCEA;      //CE access time
	int tRHZ;      //RE high to output hi-z
	int tCHZ;      //CE high to output hi-z
	int tRHOH;     //RE high to output hold
	int tRLOH;     //RE low to output hold
	int tCOH;      //CE high to output hold
	int tREH;      //RE high to output time
	int tIR;       //output hi-z to RE low
	int tRHW;      //RE high to WE low
	int tWHR;      //WE high to RE low
	int tRST;      //device resetting time
}ac_timing;


struct ssd_info{ 
	double ssd_energy;                   //SSD���ܺģ���ʱ���оƬ���ĺ���,�ܺ�����
	int64_t current_time;                //��¼ϵͳʱ��
	int64_t next_request_time;
	unsigned int real_time_subreq;       //��¼ʵʱ��д�������������ȫ��̬����ʱ��channel���ȵ����
	int flag;
	int active_flag;                     //��¼����д�Ƿ����������������������Ҫ��ʱ����ǰ�ƽ�,0��ʾû��������1��ʾ����������Ҫ��ǰ�ƽ�ʱ��
	unsigned int page;
	int64_t current_req_time;            //KXC:add a variable to record the time of current request
	unsigned int token;                  //�ڶ�̬�����У�Ϊ��ֹÿ�η����ڵ�һ��channel��Ҫά��һ�����ƣ�ÿ�δ�������ָ��λ�ÿ�ʼ����
	unsigned int gc_request;             //��¼��SSD�У���ǰʱ���ж���gc����������

	unsigned int write_request_count;    //��¼д�����Ĵ���
	unsigned int read_request_count;     //��¼�������Ĵ���
	int64_t write_avg;                   //��¼���ڼ���д����ƽ����Ӧʱ���ʱ��
	int64_t read_avg;                    //��¼���ڼ��������ƽ����Ӧʱ���ʱ��
	float av_write_count;               //KXC:add to judge whether hot of cold
	int write_no;
	unsigned int min_lsn;
	unsigned int max_lsn;
	unsigned long read_count;
	unsigned long program_count;
	unsigned long erase_count;
	unsigned long direct_erase_count;
	unsigned long copy_back_count;
	unsigned long m_plane_read_count;
	unsigned long m_plane_prog_count;
	unsigned long interleave_count;
	unsigned long interleave_read_count;
	unsigned long inter_mplane_count;
	unsigned long inter_mplane_prog_count;
	unsigned long interleave_erase_count;
	unsigned long mplane_erase_conut;
	unsigned long interleave_mplane_erase_count;
	unsigned long gc_copy_back;
	unsigned long write_flash_count;     //ʵ�ʲ����Ķ�flash��д����
	unsigned long waste_page_count;      //��¼��Ϊ�߼���������Ƶ��µ�ҳ�˷�
	float ave_read_size;
	float ave_write_size;
	unsigned int request_queue_length;
	unsigned int update_read_count;      //��¼��Ϊ���²������µĶ����������

	char parameterfilename[30];
	char tracefilename[30];
	char outputfilename[30];
	char statisticfilename[30];
	char statisticfilename2[30];

	FILE * outputfile;
	FILE * tracefile;
	FILE * statisticfile;
	FILE * statisticfile2;

    struct parameter_value *parameter;   //SSD��������
	struct dram_info *dram;
	struct request *request_queue;       //dynamic request queue
	struct request *request_tail;	     // the tail of the request queue
	struct sub_request *subs_w_head;     //������ȫ��̬����ʱ�������ǲ�֪��Ӧ�ù����ĸ�channel�ϣ������ȹ���ssd�ϣ��Ƚ���process����ʱ�Źҵ���Ӧ��channel�Ķ����������
	struct sub_request *subs_w_tail;
	struct event_node *event;            //�¼����У�ÿ����һ���µ��¼�������ʱ��˳��ӵ�������У���simulate������󣬸���������ж��׵�ʱ�䣬ȷ��ʱ��
	struct channel_info *channel_head;   //ָ��channel�ṹ��������׵�ַ
};


struct channel_info{
	int chip;                            //��ʾ�ڸ��������ж��ٿ���
	unsigned long read_count;
	unsigned long program_count;
	unsigned long erase_count;
	unsigned int token;                  //�ڶ�̬�����У�Ϊ��ֹÿ�η����ڵ�һ��chip��Ҫά��һ�����ƣ�ÿ�δ�������ָ��λ�ÿ�ʼ����

	int current_state;                   //channel has serveral states, including idle, command/address transfer,data transfer,unknown
	int next_state;
	int64_t current_time;                //��¼��ͨ���ĵ�ǰʱ��
	int64_t next_state_predict_time;     //the predict time of next state, used to decide the sate at the moment

	struct event_node *event;
	struct sub_request *subs_r_head;     //channel�ϵĶ��������ͷ���ȷ����ڶ���ͷ��������
	struct sub_request *subs_r_tail;     //channel�ϵĶ��������β���¼ӽ�����������ӵ���β
	struct sub_request *subs_w_head;     //channel�ϵ�д�������ͷ���ȷ����ڶ���ͷ��������
	struct sub_request *subs_w_tail;     //channel�ϵ�д������У��¼ӽ�����������ӵ���β
	struct gc_operation *gc_command;     //��¼��Ҫ����gc��λ��
	struct chip_info *chip_head;        
};


struct chip_info{
	unsigned int die_num;               //��ʾһ���������ж��ٸ�die
	unsigned int plane_num_die;         //indicate how many planes in a die
	unsigned int block_num_plane;       //indicate how many blocks in a plane
	unsigned int page_num_block;        //indicate how many pages in a block
	unsigned int subpage_num_page;      //indicate how many subpage in a page
	unsigned int ers_limit;             //��chip��ÿ���ܹ��������Ĵ���
	unsigned int token;                 //�ڶ�̬�����У�Ϊ��ֹÿ�η����ڵ�һ��die��Ҫά��һ�����ƣ�ÿ�δ�������ָ��λ�ÿ�ʼ����
	
	int current_state;                  //channel has serveral states, including idle, command/address transfer,data transfer,unknown
	int next_state;
	int64_t current_time;               //��¼��ͨ���ĵ�ǰʱ��
	int64_t next_state_predict_time;    //the predict time of next state, used to decide the sate at the moment
 
	unsigned long read_count;           //how many read count in the process of workload
	unsigned long program_count;
	unsigned long erase_count;

    struct ac_time_characteristics ac_timing;  
	struct die_info *die_head;
};


struct die_info{

	unsigned int token;                 //�ڶ�̬�����У�Ϊ��ֹÿ�η����ڵ�һ��plane��Ҫά��һ�����ƣ�ÿ�δ�������ָ��λ�ÿ�ʼ����
	struct plane_info *plane_head;
	
};


struct plane_info{
	int add_reg_ppn;                    //read��writeʱ�ѵ�ַ���͵��ñ������ñ���������ַ�Ĵ�����die��busy��Ϊidleʱ�������ַ //�п�����Ϊһ�Զ��ӳ�䣬��һ��������ʱ���ж����ͬ��lpn��������Ҫ��ppn������  
	unsigned int free_page;             //��plane���ж���free page
	unsigned int ers_invalid;           //��¼��plane�в���ʧЧ�Ŀ���
	unsigned int active_block[2];          //if a die has a active block, �����ʾ���������
										//KXC:a array to record the active boloc 0 for cold and 1 for hot
	int can_erase_block;                //��¼��һ��plane��׼����gc�����б����������Ŀ�,-1��ʾ��û���ҵ����ʵĿ�
	struct direct_erase *erase_node;    //������¼����ֱ��ɾ���Ŀ��,�ڻ�ȡ�µ�ppnʱ��ÿ������invalid_page_num==64ʱ���������ӵ����ָ���ϣ���GC����ʱֱ��ɾ��
	struct blk_info *blk_head;
};


struct blk_info{
	unsigned int erase_count;          //��Ĳ��������������¼��ram�У�����GC
	unsigned int free_page_num;        //��¼�ÿ��е�freeҳ������ͬ��
	unsigned int invalid_page_num;     //��¼�ÿ���ʧЧҳ�ĸ�����ͬ��
	int last_write_page;               //��¼���һ��д����ִ�е�ҳ��,-1��ʾ�ÿ�û��һҳ��д��
	struct page_info *page_head;       //��¼ÿһ��ҳ��״̬
};


struct page_info{                      //lpn��¼������ҳ�洢���߼�ҳ�������߼�ҳ��Чʱ��valid_state����0��free_state����0��
	int valid_state;                   //indicate the page is valid or invalid
	int free_state;                    //each bit indicates the subpage is free or occupted. 1 indicates that the bit is free and 0 indicates that the bit is used
	unsigned int lpn;                 
	unsigned int written_count;        //��¼��ҳ��д�Ĵ���
};


struct dram_info{
	unsigned int dram_capacity;     
	int64_t current_time;

	struct dram_parameter *dram_paramters;      
	struct map_info *map;
	struct buffer_info *buffer; 

};


/*********************************************************************************************
*buffer�е���д�ص�ҳ�Ĺ�������:��buffer_info��ά��һ������:written����������ж��ף���β��
*ÿ��buffer management�У���������ʱ�����groupҪ�Ƶ�LRU�Ķ��ף�ͬʱ�����group���Ƿ�����
*д�ص�lsn���еĻ�����Ҫ�����groupͬʱ�ƶ���written���еĶ�β��������е������ͼ��ٵķ���
*����:����Ҫͨ��ɾ����д�ص�lsnΪ�µ�д�����ڳ��ռ�ʱ����written�������ҳ�����ɾ����lsn��
*����Ҫ�����µ�д��lsnʱ���ҵ�����д�ص�ҳ�������group�ӵ�ָ��written_insert��ָ����written
*�ڵ�ǰ��������Ҫ��ά��һ��ָ�룬��buffer��LRU������ָ�����ϵ�һ��д���˵�ҳ���´�Ҫ��д��ʱ��
*ֻ�������ָ����˵�ǰһ��groupд�ؼ��ɡ�
**********************************************************************************************/
typedef struct buffer_group{
	TREE_NODE node;                     //���ڵ�Ľṹһ��Ҫ�����û��Զ���ṹ����ǰ�棬ע��!
	struct buffer_group *LRU_link_next;	// next node in LRU list
	struct buffer_group *LRU_link_pre;	// previous node in LRU list

	unsigned int group;                 //the first data logic sector number of a group stored in buffer 
	unsigned int stored;                //indicate the sector is stored in buffer or not. 1 indicates the sector is stored and 0 indicate the sector isn't stored.EX.  00110011 indicates the first, second, fifth, sixth sector is stored in buffer.
	unsigned int dirty_clean;           //it is flag of the data has been modified, one bit indicates one subpage. EX. 0001 indicates the first subpage is dirty
	int flag;			                //indicates if this node is the last 20% of the LRU list	
}buf_node;


struct dram_parameter{
	float active_current;
	float sleep_current;
	float voltage;
	int clock_time;
};


struct map_info{
	struct entry *map_entry;            //������ӳ����ṹ��ָ��,each entry indicate a mapping information
	struct buffer_info *attach_info;	// info about attach map
};


struct controller_info{
	unsigned int frequency;             //��ʾ�ÿ������Ĺ���Ƶ��
	int64_t clock_time;                 //��ʾһ��ʱ�����ڵ�ʱ��
	float power;                        //��ʾ��������λʱ����ܺ�
};


struct request{
	int64_t time;                      //���󵽴��ʱ�䣬��λΪus,�����ͨ����ϰ�߲�һ����ͨ������msΪ��λ��������Ҫ�и���λ�任����
	unsigned int lsn;                  //�������ʼ��ַ���߼���ַ
	unsigned int size;                 //����Ĵ�С���ȶ��ٸ�����
	unsigned int operation;            //��������࣬1Ϊ����0Ϊд

	unsigned int* need_distr_flag;
	unsigned int complete_lsn_count;   //record the count of lsn served by buffer

	int distri_flag;		           // indicate whether this request has been distributed already

	int64_t begin_time;
	int64_t response_time;
	double energy_consumption;         //��¼��������������ģ���λΪuJ

	struct sub_request *subs;          //���ӵ����ڸ����������������
	struct request *next_node;         //ָ����һ������ṹ��
};


struct sub_request{
	unsigned int lpn;                  //�����ʾ����������߼�ҳ��
	unsigned int ppn;                  //�����Ǹ�������ҳ�������������multi_chip_page_mapping�У�������ҳ����ʱ���ܾ�֪��psn��ֵ������ʱ��psn��ֵ��page_map_read,page_map_write��FTL��ײ㺯�������� 
	unsigned int operation;            //��ʾ������������ͣ����˶�1 д0�����в�����two plane�Ȳ��� 
	int size;
	int hot;                          //KXC:add a hot-cold flag

	unsigned int current_state;        //��ʾ��������������״̬�����궨��sub request
	int64_t current_time;
	unsigned int next_state;
	int64_t next_state_predict_time;
	 unsigned int state;              //ʹ��state�����λ��ʾ���������Ƿ���һ�Զ�ӳ���ϵ�е�һ�����ǵĻ�����Ҫ����buffer�С�1��ʾ��һ�Զ࣬0��ʾ����д��buffer
	                                  //��������Ҫ�����Ա��lsn��size�Ϳ��Էֱ����ҳ��״̬;����д������Ҫ�����Ա���󲿷�д������������bufferд�ز�����������������ҳ�������������������Ҫ����ά�ָó�Ա

	int64_t begin_time;               //������ʼʱ��
	int64_t complete_time;            //��¼��������Ĵ���ʱ��,������д����߶������ݵ�ʱ��

	struct local *location;           //�ھ�̬����ͻ�Ϸ��䷽ʽ�У���֪lpn��֪����lpn�÷��䵽�Ǹ�channel��chip��die��plane������ṹ�������������õ��ĵ�ַ
	struct sub_request *next_subs;    //ָ������ͬһ��request��������
	struct sub_request *next_node;    //ָ��ͬһ��channel����һ��������ṹ��
	struct sub_request *update;       //��Ϊ��д�����д��ڸ��²�������Ϊ�ڶ�̬���䷽ʽ���޷�ʹ��copyback��������Ҫ��ԭ����ҳ��������ܽ���д���������ԣ�������²����Ķ������������ָ����
};


/***********************************************************************
*�¼��ڵ����ʱ���������ÿ��ʱ��������Ǹ���ʱ�������һ���¼���ȷ����
************************************************************************/
struct event_node{
	int type;                        //��¼���¼������ͣ�1��ʾ�������ͣ�2��ʾ���ݴ�������
	int64_t predict_time;            //��¼���ʱ�俪ʼ��Ԥ��ʱ�䣬��ֹ��ǰִ�����ʱ��
	struct event_node *next_node;
	struct event_node *pre_node;
};

struct parameter_value{
	unsigned int chip_num;          //��¼һ��SSD���ж��ٸ�����
	unsigned int dram_capacity;     //��¼SSD��DRAM capacity
	unsigned int cpu_sdram;         //��¼Ƭ���ж���

	unsigned int channel_number;    //��¼SSD���ж��ٸ�ͨ����ÿ��ͨ���ǵ�����bus
	unsigned int chip_channel[100]; //����SSD��channel����ÿchannel�Ͽ���������

	unsigned int die_chip;    
	unsigned int plane_die;
	unsigned int block_plane;
	unsigned int page_block;
	unsigned int subpage_page;

	unsigned int page_capacity;
	unsigned int subpage_capacity;


	unsigned int ers_limit;         //��¼ÿ����ɲ����Ĵ���
	int address_mapping;            //��¼ӳ������ͣ�1��page��2��block��3��fast
	int wear_leveling;              // WL�㷨
	int gc;                         //��¼gc����
	int clean_in_background;        //��������Ƿ���ǰ̨���
	int alloc_pool;                 //allocation pool ��С(plane��die��chip��channel),Ҳ����ӵ��active_block�ĵ�λ
	float overprovide;
	float gc_threshold;             //���ﵽ�����ֵʱ����ʼGC������������д�����У���ʼGC�����������ʱ�ж�GC�����������µ�����������ͨ�����У�GC�����ж�

	double operating_current;       //NAND FLASH�Ĺ���������λ��uA
	double supply_voltage;	
	double dram_active_current;     //cpu sdram work current   uA
	double dram_standby_current;    //cpu sdram work current   uA
	double dram_refresh_current;    //cpu sdram work current   uA
	double dram_voltage;            //cpu sdram work voltage  V

	int buffer_management;          //indicates that there are buffer management or not
	int scheduling_algorithm;       //��¼ʹ�����ֵ����㷨��1:FCFS
	float quick_radio;
	int related_mapping;

	unsigned int time_step;
	unsigned int small_large_write; //the threshould of large write, large write do not occupt buffer, which is written back to flash directly

	int striping;                   //��ʾ�Ƿ�ʹ����striping��ʽ��0��ʾû�У�1��ʾ��
	int interleaving;
	int pipelining;
	int threshold_fixed_adjust;
	int threshold_value;
	int active_write;               //��ʾ�Ƿ�ִ������д����1,yes;0,no
	float gc_hard_threshold;        //��ͨ�������ò����ò�����ֻ��������д�����У������������ֵʱ��GC���������ж�
	int allocation_scheme;          //��¼���䷽ʽ��ѡ��0��ʾ��̬���䣬1��ʾ��̬����
	int static_allocation;          //��¼�����־�̬���䷽ʽ����ICS09��ƪ�������������о�̬���䷽ʽ
	int dynamic_allocation;         //��¼��̬����ķ�ʽ
	int advanced_commands;  
	int ad_priority;                //record the priority between two plane operation and interleave operation
	int ad_priority2;               //record the priority of channel-level, 0 indicates that the priority order of channel-level is highest; 1 indicates the contrary
	int greed_CB_ad;                //0 don't use copyback advanced commands greedily; 1 use copyback advanced commands greedily
	int greed_MPW_ad;               //0 don't use multi-plane write advanced commands greedily; 1 use multi-plane write advanced commands greedily
	int aged;                       //1��ʾ��Ҫ�����SSD���aged��0��ʾ��Ҫ�����SSD����non-aged
	float aged_ratio; 
	int queue_length;               //������еĳ�������

	struct ac_time_characteristics time_characteristics;
};

/********************************************************
*mapping information,state�����λ��ʾ�Ƿ��и���ӳ���ϵ
*********************************************************/
struct entry{                       
	unsigned int pn;                //�����ţ��ȿ��Ա�ʾ����ҳ�ţ�Ҳ���Ա�ʾ������ҳ�ţ�Ҳ���Ա�ʾ�������
	int state;                      //ʮ�����Ʊ�ʾ�Ļ���0000-FFFF��ÿλ��ʾ��Ӧ����ҳ�Ƿ���Ч��ҳӳ�䣩�����������ҳ�У�0��1����ҳ��Ч��2��3��Ч�����Ӧ����0x0003.
};


struct local{          
	unsigned int channel;
	unsigned int chip;
	unsigned int die;
	unsigned int plane;
	unsigned int block;
	unsigned int page;
	unsigned int sub_page;
};


struct gc_info{
	int64_t begin_time;            //��¼һ��planeʲôʱ��ʼgc������
	int copy_back_count;    
	int erase_count;
	int64_t process_time;          //��plane���˶���ʱ����gc������
	double energy_consumption;     //��plane���˶���������gc������
};


struct direct_erase{
	unsigned int block;
	struct direct_erase *next_node;
};


/**************************************************************************************
 *������һ��GC����ʱ��������ṹ������Ӧ��channel�ϣ��ȴ�channel����ʱ������GC��������
***************************************************************************************/
struct gc_operation{          
	unsigned int chip;
	unsigned int die;
	unsigned int plane;
	unsigned int block;           //�ò���ֻ�ڿ��жϵ�gc������ʹ�ã�gc_interrupt����������¼�ѽ��ҳ�����Ŀ����
	unsigned int page;            //�ò���ֻ�ڿ��жϵ�gc������ʹ�ã�gc_interrupt����������¼�Ѿ���ɵ�����Ǩ�Ƶ�ҳ��
	unsigned int state;           //��¼��ǰgc�����״̬
	unsigned int priority;        //��¼��gc���������ȼ���1��ʾ�����жϣ�0��ʾ���жϣ�����ֵ������gc����
	struct gc_operation *next_node;
};

/*
*add by ninja
*used for map_pre function
*/
typedef struct Dram_write_map
{
	unsigned int state; 
}Dram_write_map;


struct ssd_info *initiation(struct ssd_info *);
struct parameter_value *load_parameters(char parameter_file[30]);
struct page_info * initialize_page(struct page_info * p_page);
struct blk_info * initialize_block(struct blk_info * p_block,struct parameter_value *parameter);
struct plane_info * initialize_plane(struct plane_info * p_plane,struct parameter_value *parameter );
struct die_info * initialize_die(struct die_info * p_die,struct parameter_value *parameter,long long current_time );
struct chip_info * initialize_chip(struct chip_info * p_chip,struct parameter_value *parameter,long long current_time );
struct ssd_info * initialize_channels(struct ssd_info * ssd );
struct dram_info * initialize_dram(struct ssd_info * ssd);

#endif

