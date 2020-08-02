/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName�� pagemap.h
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

#define _CRTDBG_MAP_ALLOC
#include <math.h>
#include "pagemap.h"


/************************************************
*����,�����ļ�ʧ��ʱ�������open �ļ��� error��
*************************************************/
void file_assert(int error,char *s)
{
	if(error == 0) return;
	printf("open %s error\n",s);
	getchar();
	exit(-1);
}

/*****************************************************
*����,�������ڴ�ռ�ʧ��ʱ�������malloc ������ error��
******************************************************/
void alloc_assert(void *p,char *s)//����
{
	if(p!=NULL) return;
	printf("malloc %s error\n",s);
	getchar();
	exit(-1);
}

/*********************************************************************************
*����
*A��������time_t��device��lsn��size��ope��<0ʱ�������trace error:.....��
*B��������time_t��device��lsn��size��ope��=0ʱ�������probable read a blank line��
**********************************************************************************/
void trace_assert(int64_t time_t,int device,unsigned int lsn,int size,int ope)//����
{
	if(time_t <0 || device < 0 || lsn < 0 || size < 0 || ope < 0)
	{
		printf("trace error:%lld %d %d %d %d\n",time_t,device,lsn,size,ope);
		getchar();
		exit(-1);
	}
	if(time_t == 0 && device == 0 && lsn == 0 && size == 0 && ope == 0)
	{
		printf("probable read a blank line\n");
		getchar();
	}
}


/************************************************************************************
*�����Ĺ����Ǹ�������ҳ��ppn���Ҹ�����ҳ���ڵ�channel��chip��die��plane��block��page
*�õ���channel��chip��die��plane��block��page���ڽṹlocation�в���Ϊ����ֵ
//�����ҵ�PPN���ڵڼ���channel�ڼ���chip�ڼ���die�ڼ���plane�ڼ���block�ĵڼ���page��������
*************************************************************************************/
struct local *find_location(struct ssd_info *ssd,unsigned int ppn)
{
	struct local *location=NULL;
	unsigned int i=0;
	int pn,ppn_value=ppn;
	int page_plane=0,page_die=0,page_chip=0,page_channel=0;

	pn = ppn;

#ifdef DEBUG
	printf("enter find_location\n");
#endif

	location=(struct local *)malloc(sizeof(struct local));
    alloc_assert(location,"location");
	memset(location,0, sizeof(struct local));

	page_plane=ssd->parameter->page_block*ssd->parameter->block_plane;
	page_die=page_plane*ssd->parameter->plane_die;
	page_chip=page_die*ssd->parameter->die_chip;
	page_channel=page_chip*ssd->parameter->chip_channel[0];
	
	/*******************************************************************************
	*page_channel��һ��channel��page����Ŀ�� ppn/page_channel�͵õ������ĸ�channel��
	*��ͬ���İ취���Եõ�chip��die��plane��block��page
	********************************************************************************/
	location->channel = ppn/page_channel;
	location->chip = (ppn%page_channel)/page_chip;
	location->die = ((ppn%page_channel)%page_chip)/page_die;
	location->plane = (((ppn%page_channel)%page_chip)%page_die)/page_plane;
	location->block = ((((ppn%page_channel)%page_chip)%page_die)%page_plane)/ssd->parameter->page_block;
	location->page = (((((ppn%page_channel)%page_chip)%page_die)%page_plane)%ssd->parameter->page_block)%ssd->parameter->page_block;

	return location;
}


/*****************************************************************************
*��������Ĺ����Ǹ��ݲ���channel��chip��die��plane��block��page���ҵ�������ҳ��
*�����ķ���ֵ�����������ҳ�ţ����ڷ���ʱ������channel��chip��die��plane��block��page
 �ҵ������PPN
******************************************************************************/
unsigned int find_ppn(struct ssd_info * ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,unsigned int block,unsigned int page)
{
	unsigned int ppn=0;
	unsigned int i=0;
	int page_plane=0,page_die=0,page_chip=0;
	int page_channel[100];                  /*��������ŵ���ÿ��channel��page��Ŀ*/

#ifdef DEBUG
	printf("enter find_psn,channel:%d, chip:%d, die:%d, plane:%d, block:%d, page:%d\n",channel,chip,die,plane,block,page);
#endif
    
	/*********************************************
	*�����plane��die��chip��channel�е�page����Ŀ
	**********************************************/
	page_plane=ssd->parameter->page_block*ssd->parameter->block_plane;
	page_die=page_plane*ssd->parameter->plane_die;
	page_chip=page_die*ssd->parameter->die_chip;
	while(i<ssd->parameter->channel_number)//����ÿ��ͨ���µ�ҳ������
	{
		page_channel[i]=ssd->parameter->chip_channel[i]*page_chip;
		i++;
	}

    /****************************************************************************
	*��������ҳ��ppn��ppn��channel��chip��die��plane��block��page��page�������ܺ�
	*****************************************************************************/
	i=0;
	while(i<channel)//���������channel֮ǰ����channel��ҳ����channel��0��ʼ���
	{
		ppn=ppn+page_channel[i];
		i++;
	}
	ppn=ppn+page_chip*chip+page_die*die+page_plane*plane+block*ssd->parameter->page_block+page;
	
	return ppn;
}

/********************************
*���������ǻ��һ�����������״̬
*********************************/
int set_entry_state(struct ssd_info *ssd,unsigned int lsn,unsigned int size)
{
	int temp,state,move;

	temp=~(0xffffffff<<size);
	move=lsn%ssd->parameter->subpage_page;
	state=temp<<move;

	return state;
}

/**************************************************
*������Ԥ����������������������ҳ����û������ʱ��
*��ҪԤ��������ҳ����д���ݣ��Ա�֤�ܶ�������
*�ο�https://blog.csdn.net/JewelCCL/article/details/52739013
***************************************************/
struct ssd_info *pre_process_page(struct ssd_info *ssd)
{
	int fl=0;
	unsigned int device,lsn,size,ope,lpn,full_page;
	unsigned int largest_lsn,sub_size,ppn,add_size=0;
	unsigned int i=0,ii,j,k;
	int map_entry_new,map_entry_old,modify;
	int flag=0;
	char buffer_request[200];
	struct local *location;
	int64_t time;

	printf("\n");
	printf("begin pre_process_page.................\n");

	ssd->tracefile=fopen(ssd->tracefilename,"r");
	if(ssd->tracefile == NULL )      /*��trace�ļ����ж�ȡ����*/
	{
		printf("the trace file can't open\n");
		return NULL;
	}

	full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
	/*��������ssd������߼�������*/
	largest_lsn=(unsigned int )((ssd->parameter->chip_num*ssd->parameter->die_chip*ssd->parameter->plane_die*ssd->parameter->block_plane*ssd->parameter->page_block*ssd->parameter->subpage_page)*(1-ssd->parameter->overprovide));

	while(fgets(buffer_request,200,ssd->tracefile))
	{
		sscanf(buffer_request,"%lld %d %d %d %d",&time,&device,&lsn,&size,&ope);
		fl++;
		trace_assert(time,device,lsn,size,ope);                         /*���ԣ���������time��device��lsn��size��ope���Ϸ�ʱ�ͻᴦ��*/

		add_size=0;                                                     /*add_size����������Ѿ�Ԥ����Ĵ�С*/

		if(ope==1)                                                      /*����ֻ�Ƕ������Ԥ������Ҫ��ǰ����Ӧλ�õ���Ϣ������Ӧ�޸�*/
		{
			while(add_size<size)
			{				
				lsn=lsn%largest_lsn;                                    /*��ֹ��õ�lsn������lsn����*/		
				sub_size=ssd->parameter->subpage_page-(lsn%ssd->parameter->subpage_page);	
				/*�����sub_size��Ҫ��Ϊ�˶�λ�����������λ�õģ����λ���������ĳһ���ض���page���Եģ������page�ĵ�һ��sub_page��ʼ���㵽����ض��Ĳ���λ��
                 * ��ˣ�sub_size��ʵ���Ǵ�lsn����λ����ʼ����pageĩ�˵��ⲿ�����ݣ��ⲿ�������������page����Ҫ����ȡ�ġ�*/
               	
				if(add_size+sub_size>=size)                             /*ֻ�е�һ������Ĵ�СС��һ��page�Ĵ�Сʱ�����Ǵ���һ����������һ��pageʱ������������*/
				{		
					sub_size=size-add_size;		//�����ǿ�������������Ĳ�����sizeΪ���������С��add_sizeΪ�����������ɵĴ�С
					add_size+=sub_size;		
				}

				if((sub_size>ssd->parameter->subpage_page)||(add_size>size))/*��Ԥ����һ���Ӵ�Сʱ�������С����һ��page�����Ѿ�����Ĵ�С����size�ͱ���*/		
				{		
					printf("pre_process sub_size:%d\n",sub_size);		
				}

                /*******************************************************************************************************
				*�����߼�������lsn������߼�ҳ��lpn
				*�ж����dram��ӳ���map����lpnλ�õ�״̬
				*A�����״̬==0����ʾ��ǰû��д����������Ҫֱ�ӽ�sub_size��С����ҳд��ȥд��ȥ
				*B�����״̬>0����ʾ����ǰ��д��������Ҫ��һ���Ƚ�״̬����Ϊ��д��״̬��������ǰ��״̬���ص��������ĵط�
				*��ˣ�״̬==0������£������ڴ���ӳ��״̬��Ч�����Ա���Ҫ���·����״̬��Ч������ҳ������������󣬵��ú���get_ppn_for_pre_process�õ���Ч������ҳ��Ϣ��
                *��ͨ��find_location()������ҳ���ַ��Ϣ�洢���Ҹ��¸�lpn��Ӧ��ӳ�����Ϣ��
                *ͬ���ģ���״̬>0ʱ��֤����lpnλ�ô����ڴ�ӳ����е�ӳ��״̬��Ч����ֱ�ӿ���ʹ�õ���Ч����ҳ������ppnֱ�ӿ���ʹ��ӳ����е�pn
				********************************************************************************************************/
				lpn=lsn/ssd->parameter->subpage_page;
				if(ssd->dram->map->map_entry[lpn].state==0)                 /*״̬Ϊ0�����,���е�ӳ�������Ч״̬*/
				{
					/**************************************************************
					*�������get_ppn_for_pre_process�������ppn���ٵõ�location
					*�޸�ssd����ز�����dram��ӳ���map���Լ�location�µ�page��״̬
					***************************************************************/
					ppn=get_ppn_for_pre_process(ssd,lsn);                  
					location=find_location(ssd,ppn);
					ssd->program_count++;	                               //�������Ҫд��������ݣ�����д������1
					ssd->channel_head[location->channel].program_count++;
					ssd->channel_head[location->channel].chip_head[location->chip].program_count++;		
					ssd->dram->map->map_entry[lpn].pn=ppn;	
					ssd->dram->map->map_entry[lpn].state=set_entry_state(ssd,lsn,sub_size);   //0001
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=lpn;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=ssd->dram->map->map_entry[lpn].state;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=((~ssd->dram->map->map_entry[lpn].state)&full_page);
					
					free(location);
					location=NULL;
				}//if(ssd->dram->map->map_entry[lpn].state==0)
				else if(ssd->dram->map->map_entry[lpn].state>0)           /*״̬��Ϊ0�����*/
				{
					map_entry_new=set_entry_state(ssd,lsn,sub_size);      /*�õ��µ�״̬������ԭ����״̬���ĵ�һ��״̬*/
					map_entry_old=ssd->dram->map->map_entry[lpn].state;
                    modify=map_entry_new|map_entry_old;                  //ȡ��ԭ��ҳ������Ч���������µ�״̬��Ч�������Ĳ���
					ppn=ssd->dram->map->map_entry[lpn].pn;
					location=find_location(ssd,ppn);

					ssd->program_count++;	                             //�������Ҫд��������ݣ�����д������1
					ssd->channel_head[location->channel].program_count++;
					ssd->channel_head[location->channel].chip_head[location->chip].program_count++;		
					ssd->dram->map->map_entry[lsn/ssd->parameter->subpage_page].state=modify; 
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=modify;
					ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=((~modify)&full_page);
					
					free(location);
					location=NULL;
				}//else if(ssd->dram->map->map_entry[lpn].state>0)
				lsn=lsn+sub_size;                                         /*�¸����������ʼλ��*/
				add_size+=sub_size;                                       /*�Ѿ������˵�add_size��С�仯*/
			}//while(add_size<size)                                       //����state<0�����˵��������������Ч��ֱ�ӽ��и���
		}//if(ope==1) 
	}	

	printf("\n");
	printf("pre_process is complete!\n");

	fclose(ssd->tracefile);

	//KXC:�޸����
	for(i=0;i<ssd->parameter->channel_number;i++)
	for(ii=0;ii<ssd->parameter->chip_channel[i];ii++)
    for(j=0;j<ssd->parameter->die_chip;j++)
	for(k=0;k<ssd->parameter->plane_die;k++)
	{
		
		//fprintf(ssd->outputfile,"chip:%d,die:%d,plane:%d have free page: %d\n",i,j,k,ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);				
		fprintf(ssd->outputfile,"channel:%d,chip:%d,die:%d,plane:%d have free page: %d\n",i,ii,j,k,ssd->channel_head[i].chip_head[ii].die_head[j].plane_head[k].free_page);
		fflush(ssd->outputfile);
	}
	
	return ssd;
}

/**************************************
*����������ΪԤ��������ȡ����ҳ��ppn
*��ȡҳ�ŷ�Ϊ��̬��ȡ�;�̬��ȡ
**************************************/
unsigned int get_ppn_for_pre_process(struct ssd_info *ssd,unsigned int lsn)     
{
	unsigned int channel=0,chip=0,die=0,plane=0; 
	unsigned int ppn,lpn;
	unsigned int active_block;
	unsigned int channel_num=0,chip_num=0,die_num=0,plane_num=0;

#ifdef DEBUG
	printf("enter get_psn_for_pre_process\n");
#endif

	channel_num=ssd->parameter->channel_number;
	chip_num=ssd->parameter->chip_channel[0];               //����chip���������𣿶Եģ����±ߵĴ����֪���ˣ�ÿ��ͨ���µ�оƬ��Ӧ��һ��
	//chip_num=ssd->parameter->chip_num;                        
	die_num=ssd->parameter->die_chip;
	plane_num=ssd->parameter->plane_die;
	lpn=lsn/ssd->parameter->subpage_page;

	if (ssd->parameter->allocation_scheme==0)                           /*��̬��ʽ�»�ȡppn*/
	{
		if (ssd->parameter->dynamic_allocation==0)                      /*��ʾȫ��̬��ʽ�£�Ҳ����channel��chip��die��plane��block�ȶ��Ƕ�̬����*/
		{
			channel=ssd->token;
			ssd->token=(ssd->token+1)%ssd->parameter->channel_number;
			chip=ssd->channel_head[channel].token;
			ssd->channel_head[channel].token=(chip+1)%ssd->parameter->chip_channel[0];
			die=ssd->channel_head[channel].chip_head[chip].token;
			ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
		} 
		else if (ssd->parameter->dynamic_allocation==1)                 /*��ʾ�붯̬��ʽ��channel��̬������package��die��plane��̬����*/                 
		{
			channel=lpn%ssd->parameter->channel_number;
			chip=ssd->channel_head[channel].token;
			ssd->channel_head[channel].token=(chip+1)%ssd->parameter->chip_channel[0];
			die=ssd->channel_head[channel].chip_head[chip].token;
			ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
		}
	} 
	else if (ssd->parameter->allocation_scheme==1)                       /*��ʾ��̬���䣬ͬʱҲ��0,1,2,3,4,5��6�в�ͬ��̬���䷽ʽ*/
	{
		switch (ssd->parameter->static_allocation)
		{
			
			case 0:         
			{
				channel=(lpn/(plane_num*die_num*chip_num))%channel_num;
				chip=lpn%chip_num;
				die=(lpn/chip_num)%die_num;
				plane=(lpn/(die_num*chip_num))%plane_num;
				break;
			}
		case 1:
			{
				channel=lpn%channel_num;
				chip=(lpn/channel_num)%chip_num;
				die=(lpn/(chip_num*channel_num))%die_num;
				plane=(lpn/(die_num*chip_num*channel_num))%plane_num;
							
				break;
			}
		case 2:
			{
				channel=lpn%channel_num;
				chip=(lpn/(plane_num*channel_num))%chip_num;
				die=(lpn/(plane_num*chip_num*channel_num))%die_num;
				plane=(lpn/channel_num)%plane_num;
				break;
			}
		case 3:
			{
				channel=lpn%channel_num;
				chip=(lpn/(die_num*channel_num))%chip_num;
				die=(lpn/channel_num)%die_num;
				plane=(lpn/(die_num*chip_num*channel_num))%plane_num;
				break;
			}
		case 4:  
			{
				channel=lpn%channel_num;
				chip=(lpn/(plane_num*die_num*channel_num))%chip_num;
				die=(lpn/(plane_num*channel_num))%die_num;
				plane=(lpn/channel_num)%plane_num;
							
				break;
			}
		case 5:   
			{
				channel=lpn%channel_num;
				chip=(lpn/(plane_num*die_num*channel_num))%chip_num;
				die=(lpn/channel_num)%die_num;
				plane=(lpn/(die_num*channel_num))%plane_num;
							
				break;
			}
		default : return 0;
		}
	}
    
	/******************************************************************************
	*�����������䷽���ҵ�channel��chip��die��plane��������������ҵ�active_block
	*���Ż��ppn
	******************************************************************************/
	if(find_active_block(ssd,channel,chip,die,plane)==FAILURE)
	{
		printf("the read operation is expand the capacity of SSD\n");	
		return 0;
	}
	if(ssd->parameter->interruptible == 3)
	{
		if(ssd->hot_cold_flag == 3)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot_block;
		}
		else if(ssd->hot_cold_flag == 2)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot2_block;
		}
		else if(ssd->hot_cold_flag == 1)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold2_block;
		}
		else if(ssd->hot_cold_flag == 0)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold_block;
		}
		else
		{
			printf("error in hot and clod identify \n");
		}
	}
	else
	{
		active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	}
	//active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	if(write_page(ssd,channel,chip,die,plane,active_block,&ppn)==ERROR)
	{
		return 0;
	}

	return ppn;
}


/***************************************************************************************************
*������������������channel��chip��die��plane�����ҵ�һ��active_blockȻ���������block�����ҵ�һ��ҳ��
*������find_ppn�ҵ�ppn��
*��������������Ϊ�˽���Ԥ�����������ڽ��ж�д����ʱ����ȡPPN
****************************************************************************************************/
struct ssd_info *get_ppn(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,struct sub_request *sub)
{
	int old_ppn=-1;
	unsigned int ppn,lpn,full_page,lpn_page;
	unsigned int active_block;
	unsigned int block,pagenum=0;
	unsigned int page,flag=0,flag1=0;
	unsigned int old_state=0,state=0,copy_subpage=0;
	struct local *location;
	struct direct_erase *direct_erase_node,*new_direct_erase;
	struct gc_operation *gc_node;
	int victim_block = -1;
	int free_page = 0, invalid_page = 0, valid_page = 0,hot_flag;

	unsigned int i=0,j=0,k=0,l=0,m=0,n=0;
	//pagenum = ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*ssd->parameter->chip_num;
    pagenum = ssd->page;


#ifdef DEBUG
	printf("enter get_ppn,channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);
#endif

	full_page=~(0xffffffff<<(ssd->parameter->subpage_page));
	lpn=sub->lpn;
    
	/*************************************************************************************
	*���ú���find_active_block��channel��chip��die��plane�ҵ���Ծblock
	*�����޸����channel��chip��die��plane��active_block�µ�last_write_page��free_page_num
	**************************************************************************************/
	if(ssd->current_time == 98811897900 && channel == 2)
	{
		i = 0;
	}
	if(ssd->parameter->interruptible == 3)
	{
		ssd->dram->map->psn_entry1[lpn].psn = ssd->dram->map->psn_entry2[lpn].psn;
		ssd->dram->map->psn_entry2[lpn].psn = ssd->fagc_count[channel];
		ssd->fagc_count[channel]++;
		hot_cold_identify(ssd,lpn);
		//thessd->dram->map->psn_entry1[lpn].psn hit count of lpn
		ssd->dram->map->psn_entry1[lpn].pn++;
		if(ssd->write_request_count % 1000 == 0)
		{
			for(i = 0; i < pagenum; i++)
			{
				ssd->dram->map->psn_entry1[i].pn = 0.6 * ssd->dram->map->psn_entry1[i].pn;
			}
		}
	}
	/*if(ssd->dram->map->update_fre2[lpn] > 12)
	{
		hot_flag = 3;
	}
	else if(ssd->dram->map->update_fre2[lpn] > 8 && ssd->dram->map->update_fre2[lpn] <= 12)
	{
		hot_flag = 2;
	}
	else if(ssd->dram->map->update_fre2[lpn] > 4 && ssd->dram->map->update_fre2[lpn] <= 8)
	{
		hot_flag = 1;
	}
	else 
	{
		hot_flag = 0;
	}*/
	if(ssd->dram->map->psn_entry1[lpn].pn > 4)
	{
		hot_flag = 3;
	}
	else
	{
		hot_flag = 0;
	}
	ssd->hot_cold_flag = hot_flag;
	if(find_active_block(ssd,channel,chip,die,plane)==FAILURE)                      
	{
		printf("ERROR :there is no free page in channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);	
	}
    if(ssd->parameter->interruptible == 3)
	{
		if(ssd->hot_cold_flag == 3)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot_block;
		}
		else if(ssd->hot_cold_flag == 2)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot2_block;
		}
		else if(ssd->hot_cold_flag == 1)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold2_block;
		}
		else if(ssd->hot_cold_flag == 0)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold_block;
		}
		else
		{
			printf("error in hot and clod identify \n");
		}
	}
	else
	{
		active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	}
	//active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page++;	
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
    //KXC:�޸�page per block��ֵ��ʹ��������������ı�
	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page>(ssd->parameter->page_block-1))
	{
		printf("error! the last write page larger than a block!!\n");
		while(1){}
	}

	block=active_block;	
	page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page;	

	if(ssd->dram->map->map_entry[lpn].state==0)                                       /*this is the first logical page*/
	{
		if(ssd->dram->map->map_entry[lpn].pn!=0)
		{
			printf("Error in get_ppn()\n");
		}
		ssd->dram->map->map_entry[lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
		ssd->dram->map->map_entry[lpn].state=sub->state;
	}
	else                                                                            /*����߼�ҳ�����˸��£���Ҫ��ԭ����ҳ��ΪʧЧ*/
	{
		ppn=ssd->dram->map->map_entry[lpn].pn;
		location=find_location(ssd,ppn);
		lpn_page = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn;
		//KXC_2:to update the last update time of the block
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].last_update_time = ssd->current_time;
		if(	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn!=lpn && ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn != -1)
		{
			printf("\nError in get_ppn()22222\n");
		}

		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=0;             /*��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0*/
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=0;              /*��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0*/
		if(ssd->parameter->interruptible != 3)
		{
			ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=0;
		}
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].invalid_page++;
		/*******************************************************************************************
		*��block��ȫ��invalid��ҳ������ֱ��ɾ�������ڴ���һ���ɲ����Ľڵ㣬����location�µ�plane����(����)
		********************************************************************************************/
		if ( 0&& ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num==ssd->parameter->page_block)    
		{
			new_direct_erase=(struct direct_erase *)malloc(sizeof(struct direct_erase));
            alloc_assert(new_direct_erase,"new_direct_erase");
			memset(new_direct_erase,0, sizeof(struct direct_erase));

			new_direct_erase->block=location->block;
			new_direct_erase->next_node=NULL;
			direct_erase_node=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
			if (direct_erase_node==NULL)
			{
				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node=new_direct_erase;
			} 
			else
			{
				new_direct_erase->next_node=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node=new_direct_erase;
			}
		}
		/*******************************************************************************************
		*��block��ȫ��invalid��ҳ����ӵ�soft gc�У��ڿ���ʱ����ɲ���
		********************************************************************************************/		
		if ( ssd->parameter->interruptible == 1 && ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num==ssd->parameter->page_block)    
		{
			if(ssd->channel_head[location->channel].gc_soft == NULL && ssd->channel_head[location->channel].gc_sub_queue == NULL)
			{
				//find block
				//victim_block = find_victim_interrupt_gc(ssd,channel,chip,die,plane);
				victim_block = location->block;
				valid_page = 0;
				for(i = 0; i < ssd->parameter->page_block; i++)
				{
					if(ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[victim_block].page_head[i].valid_state > 0)
						valid_page++;
				}

				free_page = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[victim_block].free_page_num;
				//invalid_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[victim_block].invalid_page_num;
				//valid_page = ssd->parameter->page_block - free_page - invalid_page;
				if(free_page > 0) printf("error in victim block selection in soft gc");
				//KXC_2: gc buffer is enough to put the valid pages
				if(valid_page == 0 && valid_page <= ssd->parameter->gc_buffer_size - ssd->gc_buf_count)
				{
					gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
					alloc_assert(gc_node,"gc_node");
					memset(gc_node,0, sizeof(struct gc_operation));
					gc_node->next_node=NULL;
					gc_node->chip=location->chip;
					gc_node->die=location->die;
					gc_node->plane=location->plane;
					gc_node->block= victim_block;
					gc_node->page=0;
					gc_node->state=GC_WAIT;
					gc_node->priority=GC_INTERRUPT;
					gc_node->next_node=ssd->channel_head[location->channel].gc_soft;
					ssd->channel_head[location->channel].gc_soft=gc_node;
					ssd->gc_request++;
					ssd->gc_soft_count++;
					ssd->direct_erase_count++;
					soft_gc_distribute(ssd,location->channel,location->chip,location->die,location->plane);
				}
			}
		}

		free(location);
		location=NULL;
		ssd->dram->map->map_entry[lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
		ssd->dram->map->map_entry[lpn].state=(ssd->dram->map->map_entry[lpn].state|sub->state);
	}

	
	sub->ppn=ssd->dram->map->map_entry[lpn].pn;                                      /*�޸�sub�������ppn��location�ȱ���*/
	sub->location->channel=channel;
	sub->location->chip=chip;
	sub->location->die=die;
	sub->location->plane=plane;
	sub->location->block=active_block;
	sub->location->page=page;

	ssd->program_count++;                                                           /*�޸�ssd��program_count,free_page�ȱ���*/
	ssd->channel_head[channel].program_count++;
	ssd->channel_head[channel].chip_head[chip].program_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].lpn=lpn;	
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].valid_state=sub->state;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].free_state=((~(sub->state))&full_page);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].written_count++;
	ssd->write_flash_count++;


	//KXC_2:the algorithm of 14's meeting DT-GC
	if(ssd->parameter->interruptible == 2)
	{
		dtgc_judge(ssd,channel,chip,die,plane);
		return ssd;
	}

    //��ʵ����������ʱ����û�п���������������
	if (ssd->parameter->active_write==0)                                            /*���û���������ԣ�ֻ����gc_hard_threshold�������޷��ж�GC����*/
	{                                                                               /*���plane�е�free_page����Ŀ����gc_hard_threshold���趨����ֵ�Ͳ���gc����*/
		if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page<(ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->gc_hard_threshold))
		{
			gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
			alloc_assert(gc_node,"gc_node");
			memset(gc_node,0, sizeof(struct gc_operation));

			gc_node->next_node=NULL;
			gc_node->chip=chip;
			gc_node->die=die;
			gc_node->plane=plane;
			gc_node->block=0xffffffff;
			gc_node->page=0;
			gc_node->state=GC_WAIT;
			gc_node->priority=GC_UNINTERRUPT;
			gc_node->next_node=ssd->channel_head[channel].gc_command;
			ssd->channel_head[channel].gc_command=gc_node;
			ssd->gc_request++;
			ssd->gc_hard_count++;
		}//KXC_2: to produce interruptible gc suing soft gc threshold
		else if(ssd->parameter->interruptible == 1 &&  ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page<(ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->gc_soft_threshold))
		{
			if(1)	
			{
				if(ssd->channel_head[channel].gc_soft == NULL && ssd->channel_head[channel].gc_sub_queue == NULL)
				{
					//find block
					victim_block = find_victim_interrupt_gc(ssd,channel,chip,die,plane);
					valid_page = 0;
					for(i = 0; i < ssd->parameter->page_block; i++)
					{
						if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[victim_block].page_head[i].valid_state > 0)
							valid_page++;
					}
					free_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[victim_block].free_page_num;
					//invalid_page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[victim_block].invalid_page_num;
					//valid_page = ssd->parameter->page_block - free_page - invalid_page;
					if(free_page > 0) printf("error in victim block selection in soft gc");
					//KXC_2: gc buffer is enough to put the valid pages
					if(valid_page>= 0 && valid_page <= ssd->parameter->gc_buffer_size - ssd->gc_buf_count)
					{
						gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
						alloc_assert(gc_node,"gc_node");
						memset(gc_node,0, sizeof(struct gc_operation));

						gc_node->next_node=NULL;
						gc_node->chip=chip;
						gc_node->die=die;
						gc_node->plane=plane;
						gc_node->block= victim_block;
						gc_node->page=0;
						gc_node->state=GC_WAIT;
						gc_node->priority=GC_INTERRUPT;
						gc_node->next_node=ssd->channel_head[channel].gc_soft;
						ssd->channel_head[channel].gc_soft=gc_node;
						ssd->gc_request++;
						ssd->gc_soft_count++;

						soft_gc_distribute(ssd,channel,chip,die,plane);

					}
					else
					{
						return ssd;
					}

				}
			}
		}
	} 
	return ssd;
}
struct ssd_info *dtgc_judge(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)
{
	double alpha = 1.1, beta = 0.05, gama = 0.15, sigma = 1.03, kesai = 0.07;
	double IGC_TH = 0.0, NGC_TH = 0.0;
	unsigned int page_plane = ssd->parameter->block_plane * ssd->parameter->page_block;
	double FPP = (double)ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page / (double)page_plane;
	double IPP = (double)ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].invalid_page / (double)page_plane;
	double VPP = 1.0 - FPP - IPP;
	struct gc_operation *gc_node;

	//if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter >= ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter_th)
	if(1)
	{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter = 0;
		IGC_TH = alpha * ssd->parameter->overprovide + beta * (1 - VPP) + gama * IPP;
		NGC_TH = sigma * ssd->parameter->overprovide + kesai * (1- VPP);
		IGC_TH = IGC_TH < ssd->parameter->gc_soft_threshold ? IGC_TH : ssd->parameter->gc_soft_threshold;
		NGC_TH = NGC_TH < ssd->parameter->gc_hard_threshold ? NGC_TH : ssd->parameter->gc_hard_threshold;
		if(FPP < IGC_TH)
		{
			if(FPP < NGC_TH)
			{
				gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
				alloc_assert(gc_node,"gc_node");
				memset(gc_node,0, sizeof(struct gc_operation));

				gc_node->next_node=NULL;
				gc_node->chip=chip;
				gc_node->die=die;
				gc_node->plane=plane;
				gc_node->block=0xffffffff;
				gc_node->page=0;
				gc_node->state=GC_WAIT;
				gc_node->priority=GC_UNINTERRUPT;
				gc_node->next_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_hard_head;
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_hard_head = gc_node;
				ssd->gc_request++;
				ssd->gc_hard_count++;
			}
			else
			{
				//if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_soft_head == NULL)
				if(1)
				{
					gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
					alloc_assert(gc_node,"gc_node");
					memset(gc_node,0, sizeof(struct gc_operation));

					gc_node->next_node=NULL;
					gc_node->chip=chip;
					gc_node->die=die;
					gc_node->plane=plane;
					gc_node->block=0xffffffff;
					gc_node->page=0;
					gc_node->state=GC_WAIT;
					gc_node->priority=GC_INTERRUPT;
					gc_node->next_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_soft_head;
					ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_soft_head = gc_node;
					ssd->gc_request++;
					ssd->gc_soft_count++;
				}
			}
		}
	}

}
struct ssd_info *soft_gc_distribute(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)
{
	unsigned int i,block,active_block,transfer_size,invalid_page=0;
	struct local *location;
	struct gc_operation *gc_node = ssd->channel_head[channel].gc_soft;
	//struct  *gc_sub;

	for (i=0;i<ssd->parameter->page_block;i++)
	{
		if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].page_head[i].valid_state>0)
		{
			creat_sub_gc(ssd,gc_node,channel,i,READ);
		}
	}
	creat_sub_gc(ssd,gc_node,channel,-1,11);  //KXC_2: erase operation
	for (i=0;i<ssd->parameter->page_block;i++)
	{
		if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].page_head[i].valid_state>0)
		{
			creat_sub_gc(ssd,gc_node,channel,i,WRITE);
		}
	}

}

struct ssd_info * creat_sub_gc(struct ssd_info *ssd,struct gc_operation *gc_node,unsigned int channel, int page, int type)
{
	struct sub_request* sub=NULL;
	struct sub_request* sub_r=NULL, *sub_w=NULL, *test=NULL;
	struct local *location=NULL;
	
	struct channel_info * p_ch=NULL;
	struct local * loc=NULL;
	unsigned int flag=0;
	unsigned int sub_size=0;
	unsigned int sub_state=0;
	unsigned int lpn;
	int read_hit = 0;
	int write_hit = 0;

	sub = (struct sub_request*)malloc(sizeof(struct sub_request));                        /*����һ��������Ľṹ*/
	alloc_assert(sub,"sub_request");
	memset(sub,0, sizeof(struct sub_request));

	if(sub==NULL)
	{
		return NULL;
	}
	sub->location=NULL;
	//sub->mom=gc_node;
	sub->next_subs=NULL;
	
	//p_ch = &ssd->channel_head[loc->channel];
	//sub_r=p_ch->subs_r_head;

	if(ssd->channel_head[channel].gc_sub_queue ==NULL)
	{
		sub->next_subs = ssd->channel_head[channel].gc_sub_queue;
		ssd->channel_head[channel].gc_sub_queue = sub;
		//ssd->channel_head[channel].gc_sub_tail = sub;
	}

	//KXC_2: ��ͨ������ӦLPN�Ķ����󣬽������gc_buffer�м��ɣ���ʱ���ò���gc_sub_read
	lpn = ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].blk_head[gc_node->block].page_head[page].lpn;
	sub_r = ssd->channel_head[channel].subs_r_head;
	if(ssd->parameter->allocation_scheme == 0 && ssd->parameter->dynamic_allocation == 0)
	{
		sub_w = ssd->subs_w_head;
	}
	else
	{
		sub_w = ssd->channel_head[channel].subs_w_head;
	}
	
	
	while (sub_r)
	{
		if(lpn == sub_r->lpn)
		{
			read_hit = 1;
			break;
		}
		sub_r = sub_r->next_node;
	}
	while (sub_w)
	{
		if(lpn == sub_w->lpn)
		{
			write_hit = 1;
			break;
		}
		sub_w = sub_w->next_node;
	}
	if(read_hit == 1 && write_hit == 1)
	{
		ssd->gc_write_hit_count++;
		ssd->gc_read_hit_count++;
		free(sub);
		sub = NULL;	
		return;    //if io read and write are both in io queue, the gc sub no need for read and write
	}
	else if(read_hit ==1 && write_hit == 0)
	{
		if(type == READ)    //gc write if also need to write back
		{	
			ssd->gc_read_hit_count++;
			sub_r->buf_flag = 1;
			free(sub);
			sub = NULL;
			return;
		}
	}
	else if(read_hit == 0 && write_hit ==1)
	{
		ssd->gc_write_hit_count++;
		free(sub);
		sub = NULL;
		return; 	
	}
	
	/*************************************************************************************
	*�ڶ�����������£���һ��ǳ���Ҫ����ҪԤ���ж϶�������������Ƿ����������������ͬ�ģ�
	*�еĻ�����������Ͳ�����ִ���ˣ����µ�������ֱ�Ӹ�Ϊ���
	**************************************************************************************/
	if (type == READ)
	{	
		ssd->gc_buf_count++; //KXC_2:the number of gc buffer that should use
		//loc = find_location(ssd,ssd->dram->map->map_entry[lpn].pn);
		//sub->location=loc;
		location=(struct local *)malloc(sizeof(struct local));
		alloc_assert(location,"location");
		memset(location,0, sizeof(struct local));
		location->channel = channel;
		location->chip = gc_node->chip;
		location->die = gc_node->die;
		location->plane = gc_node->plane;
		location->block = gc_node->block;
		location->page = page;
		sub->location = location;



		sub->begin_time = ssd->current_time;
		sub->current_state = SR_WAIT;
		sub->current_time=MAX_INT64;
		sub->next_state = SR_R_C_A_TRANSFER;
		sub->next_state_predict_time=MAX_INT64;
		sub->lpn = ssd->channel_head[channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[page].lpn;
		sub_state=(ssd->dram->map->map_entry[sub->lpn].state&0x7fffffff);
		sub_size=size(sub_state);                                                             /*��Ҫ�������������������С*/

		p_ch = &ssd->channel_head[location->channel];	
		sub->ppn = ssd->dram->map->map_entry[sub->lpn].pn;
		sub->operation = READ;
		sub_r=p_ch->subs_r_head;                                                      /*һ�¼��а���flag�����жϸö�������������Ƿ����������������ͬ�ģ��еĻ������µ�������ֱ�Ӹ�Ϊ���*/
		flag=0;
		while (sub_r!=NULL)
		{
			if (sub_r->ppn==sub->ppn)
			{
				flag=1;
				break;
			}
			sub_r=sub_r->next_node;
		}
		if (flag==0)
		{
			if (p_ch->gc_sub_tail!=NULL)
			{
				p_ch->gc_sub_tail->next_node=sub;
				p_ch->gc_sub_tail=sub;
			} 
			else
			{
				p_ch->gc_sub_queue=sub;
				p_ch->gc_sub_tail=sub;
			}
		}
		else
		{
			
			sub->current_state = SR_R_DATA_TRANSFER;
			sub->current_time=ssd->current_time;
			sub->next_state = SR_COMPLETE;
			sub->next_state_predict_time=ssd->current_time+1000;
			sub->complete_time=ssd->current_time+1000;
		}
	}
	/*************************************************************************************
	*д���������£�ֻ�ö�̬д
	**************************************************************************************/
	else if(type == WRITE)
	{                                
		sub->ppn=0;
		sub->operation = WRITE;
		sub->location=(struct local *)malloc(sizeof(struct local));
		alloc_assert(sub->location,"sub->location");
		memset(sub->location,0, sizeof(struct local));

		sub->current_state=SR_WAIT;
		sub->current_time=ssd->current_time;
		sub->lpn = ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].blk_head[gc_node->block].page_head[page].lpn;
		
		sub->state=(ssd->dram->map->map_entry[sub->lpn].state&0x7fffffff);
		sub->size=size(sub->state); 
		sub->begin_time=ssd->current_time;

		sub->location->channel=-1;
		sub->location->chip=-1;
		sub->location->die=-1;
		sub->location->plane=-1;
		sub->location->block=-1;
		sub->location->page=-1;

		if (ssd->channel_head[channel].gc_sub_tail!=NULL)
		{
			ssd->channel_head[channel].gc_sub_tail->next_node=sub;
			ssd->channel_head[channel].gc_sub_tail=sub;
		} 
		else
		{
			ssd->channel_head[channel].gc_sub_queue=sub;
			ssd->channel_head[channel].gc_sub_tail=sub;
		}
      
			
	}
	else if(type == 11)
	{
		sub->ppn=0;
		sub->operation = 11;
		sub->location=(struct local *)malloc(sizeof(struct local));
		alloc_assert(sub->location,"sub->location");
		memset(sub->location,0, sizeof(struct local));

		sub->current_state=GC_WAIT;
		sub->current_time=ssd->current_time;
		sub->lpn = -1;
		
		//sub_state=(ssd->dram->map->map_entry[sub->lpn].state&0x7fffffff);
		//sub_size=size(sub_state); 

		sub->begin_time=ssd->current_time;

		sub->location->channel = channel;
		sub->location->chip = gc_node->chip;
		sub->location->die = gc_node->die;
		sub->location->plane = gc_node->plane;
		sub->location->block = gc_node->block;
		sub->location->page = page;

		if (ssd->channel_head[channel].gc_sub_tail!=NULL)
		{
			ssd->channel_head[channel].gc_sub_tail->next_node=sub;
			ssd->channel_head[channel].gc_sub_tail=sub;
		} 
		else
		{
			ssd->channel_head[channel].gc_sub_queue=sub;
			ssd->channel_head[channel].gc_sub_tail=sub;
		}
	}
	else
	{
		free(sub->location);
		sub->location=NULL;
		free(sub);
		sub=NULL;
		printf("\nERROR ! Unexpected command.\n");
		return NULL;
	}
	
	return ssd;
}
/*****************************************************************************************
*�������������Ϊgc����Ѱ���µ�ppn����Ϊ��gc��������Ҫ�ҵ��µ��������ԭ��������ϵ�����
*��gc��Ѱ���������ĺ�������������ѭ����gc����
******************************************************************************************/
 unsigned int get_ppn_for_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)     
{
	unsigned int ppn;
	unsigned int active_block,block,page;

#ifdef DEBUG
	printf("enter get_ppn_for_gc,channel:%d, chip:%d, die:%d, plane:%d\n",channel,chip,die,plane);
#endif
	ssd->find_what = 1;
	if(find_active_block(ssd,channel,chip,die,plane)!=SUCCESS)
	{
		printf("\n\n Error int get_ppn_for_gc().\n");
		return 0xffffffff;
	}
    if(ssd->parameter->interruptible == 3)
	{
		if(ssd->hot_cold_flag == 3)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot_block;
		}
		else if(ssd->hot_cold_flag == 2)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot2_block;
		}
		else if(ssd->hot_cold_flag == 1)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold2_block;
		}
		else if(ssd->hot_cold_flag == 0)
		{
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold_block;
		}
		else
		{
			printf("error in hot and clod identify \n");
		}
	}
	else
	{
		active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	}
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page++;	
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
    //KSC:�޸�63
	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page>(ssd->parameter->page_block-1))
	{
		printf("error! the last write page larger than a block in get ppn for gc!!\n");
		while(1){}
	}

	block=active_block;	
	page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page;	

	ppn=find_ppn(ssd,channel,chip,die,plane,block,page);

	ssd->program_count++;
	ssd->channel_head[channel].program_count++;
	ssd->channel_head[channel].chip_head[chip].program_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[page].written_count++;
	ssd->write_flash_count++;

	return ppn;

}

/*********************************************************************************************************************
* ��־�� ��2011��7��28���޸�   
*�����Ĺ��ܾ���erase_operation������������channel��chip��die��plane�µ�block������
*Ҳ���ǳ�ʼ�����block����ز�����eg��free_page_num=page_block��invalid_page_num=0��last_write_page=-1��erase_count++
*�������block�����ÿ��page����ز���ҲҪ�޸ġ�
*********************************************************************************************************************/

Status erase_operation(struct ssd_info * ssd,unsigned int channel ,unsigned int chip ,unsigned int die ,unsigned int plane ,unsigned int block)
{
	unsigned int i=0;
	unsigned int erase_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num=ssd->parameter->page_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num=0;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page=-1;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].erase_count++;
	erase_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].erase_count;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase > erase_block? ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase : erase_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].min_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].min_erase < erase_block? ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].min_erase : erase_block;
	for (i=0;i<ssd->parameter->page_block;i++)
	{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state=PG_SUB;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state=0;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].lpn=-1;
	}
	ssd->erase_count++;
	ssd->channel_head[channel].erase_count++;			
	ssd->channel_head[channel].chip_head[chip].erase_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page+=ssd->parameter->page_block;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].invalid_page-=ssd->parameter->page_block;
	if(ssd->iops_time == 0) ssd->iops_time =400000000000000;
	/*if(ssd->parameter->interruptible == 0)
	{
		if(ssd->iops_time == 0) 
		{
			ssd->iops_time = ssd->current_time;
			FILE *fpWriter = fopen("time","w");
			if(fpWriter == NULL)
			{
				printf("Error in the file open to write the time\n");
			}
			fprintf(fpWriter,"%d",ssd->iops_time);
			fclose(fpWriter);
		}
	}
	else
	{
		if(ssd->iops_time == 0) 
		{
			FILE *fpReader = fopen("time","r");
			if(fpReader == NULL)
			{
				printf("Error in the file open to write the time\n");
			}
			fscanf(fpReader,"%d",&ssd->iops_time);
			fclose(fpReader);
		}
	}*/		
	return SUCCESS;

}


/**************************************************************************************
*��������Ĺ����Ǵ���INTERLEAVE_TWO_PLANE��INTERLEAVE��TWO_PLANE��NORMAL�µĲ����Ĳ�����
***************************************************************************************/
Status erase_planes(struct ssd_info * ssd, unsigned int channel, unsigned int chip, unsigned int die1, unsigned int plane1,unsigned int command)
{
	unsigned int die=0;
	unsigned int plane=0;
	unsigned int block=0;
	struct direct_erase *direct_erase_node=NULL;
	unsigned int block0=0xffffffff;
	unsigned int block1=0;

	if((ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node==NULL)||               
		((command!=INTERLEAVE_TWO_PLANE)&&(command!=INTERLEAVE)&&(command!=TWO_PLANE)&&(command!=NORMAL)))     /*���û�в�������������command���ԣ����ش���*/           
	{
		return ERROR;
	}

	/************************************************************************************************************
	*�����������ʱ������Ҫ���Ͳ����������channel��chip���ڴ��������״̬����CHANNEL_TRANSFER��CHIP_ERASE_BUSY
	*��һ״̬��CHANNEL_IDLE��CHIP_IDLE��
	*************************************************************************************************************/
	block1=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node->block;
	
	ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	

	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;										
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;

	if(command==INTERLEAVE_TWO_PLANE)                                       /*�߼�����INTERLEAVE_TWO_PLANE�Ĵ���*/
	{
		for(die=0;die<ssd->parameter->die_chip;die++)
		{
			block0=0xffffffff;
			if(die==die1)
			{
				block0=block1;
			}
			for (plane=0;plane<ssd->parameter->plane_die;plane++)
			{
				direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
				if(direct_erase_node!=NULL)
				{
					
					block=direct_erase_node->block; 
					
					if(block0==0xffffffff)
					{
						block0=block;
					}
					else
					{
						if(block!=block0)
						{
							continue;
						}

					}
					ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node=direct_erase_node->next_node;
					erase_operation(ssd,channel,chip,die,plane,block);     /*��ʵ�Ĳ��������Ĵ���*/
					free(direct_erase_node);                               
					direct_erase_node=NULL;
					ssd->direct_erase_count++;
				}

			}
		}

		ssd->interleave_mplane_erase_count++;                             /*������һ��interleave two plane erase����,��������������ʱ�䣬�Լ���һ��״̬��ʱ��*/
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+18*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tWB;       
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time-9*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tBERS;

	}
	else if(command==INTERLEAVE)                                          /*�߼�����INTERLEAVE�Ĵ���*/
	{
		for(die=0;die<ssd->parameter->die_chip;die++)
		{
			for (plane=0;plane<ssd->parameter->plane_die;plane++)
			{
				if(die==die1)
				{
					plane=plane1;
				}
				direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
				if(direct_erase_node!=NULL)
				{
					block=direct_erase_node->block;
					ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node=direct_erase_node->next_node;
					erase_operation(ssd,channel,chip,die,plane,block);
					free(direct_erase_node);
					direct_erase_node=NULL;
					ssd->direct_erase_count++;
					break;
				}	
			}
		}
		ssd->interleave_erase_count++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;       
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
	}
	else if(command==TWO_PLANE)                                          /*�߼�����TWO_PLANE�Ĵ���*/
	{

		for(plane=0;plane<ssd->parameter->plane_die;plane++)
		{
			direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane].erase_node;
			if((direct_erase_node!=NULL))
			{
				block=direct_erase_node->block;
				if(block==block1)
				{
					ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane].erase_node=direct_erase_node->next_node;
					erase_operation(ssd,channel,chip,die1,plane,block);
					free(direct_erase_node);
					direct_erase_node=NULL;
					ssd->direct_erase_count++;
				}
			}
		}

		ssd->mplane_erase_conut++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;      
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
	}
	else if(command==NORMAL)                                             /*��ͨ����NORMAL�Ĵ���*/
	{
		direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node;
		block=direct_erase_node->block;
		ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node=direct_erase_node->next_node;
		free(direct_erase_node);
		direct_erase_node=NULL;
		erase_operation(ssd,channel,chip,die1,plane1,block);

		ssd->direct_erase_count++;
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+5*ssd->parameter->time_characteristics.tWC;       								
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tWB+ssd->parameter->time_characteristics.tBERS;	
	}
	else
	{
		return ERROR;
	}

	direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die1].plane_head[plane1].erase_node;

	if(((direct_erase_node)!=NULL)&&(direct_erase_node->block==block1))
	{
		return FAILURE; 
	}
	else
	{
		return SUCCESS;
	}
}


/*******************************************************************************************************************
*GC������ĳ��plane��free��������ֵ���д�������ĳ��plane������ʱ��GC����ռ�����plane���ڵ�die����Ϊdie��һ��������Ԫ��
*��һ��die��GC���������������ĸ�planeͬʱerase������interleave erase������GC����Ӧ������������ʱֹͣ���ƶ����ݺͲ���
*ʱ���У����Ǽ�϶ʱ�����ֹͣGC���������Է����µ�������󣬵��������������������϶ʱ�䣬����GC������������������
*GC��ֵ��һ������ֵ��һ��Ӳ��ֵ������ֵ��ʾ�������ֵ�󣬿��Կ�ʼ������GC���������ü�Ъʱ�䣬GC���Ա��µ��������жϣ�
*������Ӳ��ֵ��ǿ����ִ��GC�������Ҵ�GC�������ܱ��жϣ�ֱ���ص�Ӳ��ֵ���ϡ�
*������������棬�ҳ����die���е�plane�У���û�п���ֱ��ɾ����block��Ҫ���еĻ�������interleave two plane���ɾ��
*��Щblock�������ж���plane������ֱ��ɾ����block��ͬʱɾ�������еĻ��������ǵ������plane����ɾ��������Ҳ������Ļ���
*ֱ����������gc_parallelism�������н�һ��GC�������ú���Ѱ��ȫ��Ϊinvalid�Ŀ飬ֱ��ɾ�����ҵ���ֱ��ɾ���ķ���1��û����
*������-1��
*********************************************************************************************************************/
int gc_direct_erase(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)     
{
	unsigned int lv_die=0,lv_plane=0;                                                           /*Ϊ����������ʹ�õľֲ����� Local variables*/
	unsigned int interleaver_flag=FALSE,muilt_plane_flag=FALSE;
	unsigned int normal_erase_flag=TRUE;

	struct direct_erase * direct_erase_node1=NULL;
	struct direct_erase * direct_erase_node2=NULL;

	direct_erase_node1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
	if (direct_erase_node1==NULL)
	{
		return FAILURE;
	}
    
	/********************************************************************************************************
	*���ܴ���TWOPLANE�߼�����ʱ��������Ӧ��channel��chip��die��������ͬ��plane�ҵ�����ִ��TWOPLANE������block
	*����muilt_plane_flagΪTRUE
	*********************************************************************************************************/
	if((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)
	{	
		for(lv_plane=0;lv_plane<ssd->parameter->plane_die;lv_plane++)
		{
			direct_erase_node2=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
			if((lv_plane!=plane)&&(direct_erase_node2!=NULL))
			{
				if((direct_erase_node1->block)==(direct_erase_node2->block))
				{
					muilt_plane_flag=TRUE;
					break;
				}
			}
		}
	}

	/***************************************************************************************
	*���ܴ���INTERLEAVE�߼�����ʱ����ͬ��die�µ�ͬ����plane��ͬ����block������Ӧ��channel��chip�ҵ�����ִ��INTERLEAVE������block
	*����interleaver_flagΪTRUE
	****************************************************************************************/
	if((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)
	{
		for(lv_die=0;lv_die<ssd->parameter->die_chip;lv_die++)
		{
			if(lv_die!=die)
			{
				for(lv_plane=0;lv_plane<ssd->parameter->plane_die;lv_plane++)
				{
					if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node!=NULL)
					{
						interleaver_flag=TRUE;
						break;
					}
				}
			}
			if(interleaver_flag==TRUE)
			{
				break;
			}
		}
	}
    
	/************************************************************************************************************************
	*A������ȿ���ִ��twoplane������block���п���ִ��interleaver������block����ô��ִ��INTERLEAVE_TWO_PLANE�ĸ߼������������
	*B�����ֻ����ִ��interleaver������block����ô��ִ��INTERLEAVE�߼�����Ĳ�������
	*C�����ֻ����ִ��TWO_PLANE������block����ô��ִ��TWO_PLANE�߼�����Ĳ�������
	*D��û��������Щ�������ô��ֻ�ܹ�ִ����ͨ�Ĳ���������
	*************************************************************************************************************************/
	if ((muilt_plane_flag==TRUE)&&(interleaver_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))     
	{
		if(erase_planes(ssd,channel,chip,die,plane,INTERLEAVE_TWO_PLANE)==SUCCESS)
		{
			return SUCCESS;
		}
	} 
	else if ((interleaver_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))
	{
		if(erase_planes(ssd,channel,chip,die,plane,INTERLEAVE)==SUCCESS)
		{
			return SUCCESS;
		}
	}
	else if ((muilt_plane_flag==TRUE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
	{
		if(erase_planes(ssd,channel,chip,die,plane,TWO_PLANE)==SUCCESS)
		{
			return SUCCESS;
		}
	}

	if ((normal_erase_flag==TRUE))                              /*����ÿ��plane���п���ֱ��ɾ����block��ֻ�Ե�ǰplane������ͨ��erase����������ֻ��ִ����ͨ����*/
	{
		if (erase_planes(ssd,channel,chip,die,plane,NORMAL)==SUCCESS)
		{
			return SUCCESS;
		} 
		else
		{
			return FAILURE;                                     /*Ŀ���planeû�п���ֱ��ɾ����block����ҪѰ��Ŀ����������ʵʩ��������*/
		}
	}
	return SUCCESS;
}


Status move_page(struct ssd_info * ssd, struct local *location, unsigned int * transfer_size)
{
	struct local *new_location=NULL;
	unsigned int free_state=0,valid_state=0;
	unsigned int lpn=0,old_ppn=0,ppn=0;

	lpn=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn;
	valid_state=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state;
	free_state=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state;
	old_ppn=find_ppn(ssd,location->channel,location->chip,location->die,location->plane,location->block,location->page);      /*��¼�����Ч�ƶ�ҳ��ppn���Ա�map���߶���ӳ���ϵ�е�ppn������ɾ������Ӳ���*/
	ppn=get_ppn_for_gc(ssd,location->channel,location->chip,location->die,location->plane);                /*�ҳ�����ppnһ�����ڷ���gc������plane��,����ʹ��copyback������Ϊgc������ȡppn*/

	new_location=find_location(ssd,ppn);                                                                   /*�����»�õ�ppn��ȡnew_location*/

	if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
	{
		if (ssd->parameter->greed_CB_ad==1)                                                                /*̰����ʹ�ø߼�����*/
		{
			ssd->copy_back_count++;
			ssd->gc_copy_back++;
			while (old_ppn%2!=ppn%2)
			{
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=0;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=0;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=0;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].invalid_page_num++;
				ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].invalid_page++;
				
				free(new_location);
				new_location=NULL;

				ppn=get_ppn_for_gc(ssd,location->channel,location->chip,location->die,location->plane);    /*�ҳ�����ppnһ�����ڷ���gc������plane�У�����������ż��ַ����,����ʹ��copyback����*/
				ssd->program_count--;
				ssd->write_flash_count--;
				ssd->waste_page_count++;
			}
			if(new_location==NULL)
			{
				new_location=find_location(ssd,ppn);
			}
			
			ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=free_state;
			ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=lpn;
			ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=valid_state;
		} 
		else
		{
			if (old_ppn%2!=ppn%2)
			{
				(* transfer_size)+=size(valid_state);    //��ʹ��copyback����
			}
			else
			{

				ssd->copy_back_count++;
				ssd->gc_copy_back++;
			}
		}	
	} 
	else
	{
		(* transfer_size)+=size(valid_state);
	}
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=free_state;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=lpn;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=valid_state;


	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].invalid_page++;

	if (old_ppn==ssd->dram->map->map_entry[lpn].pn)                                                     /*�޸�ӳ���*/
	{
		ssd->dram->map->map_entry[lpn].pn=ppn;
	}
	free(new_location);
	new_location=NULL;

	return SUCCESS;
}

Status move_page_hot_cold(struct ssd_info * ssd, struct local *location, unsigned int * transfer_size)
{
	struct local *new_location=NULL;
	unsigned int free_state=0,valid_state=0;
	unsigned int lpn=0,old_ppn=0,ppn=0;
	int hot_flag = -1;

	lpn=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn;
	valid_state=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state;
	free_state=ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state;
	old_ppn=find_ppn(ssd,location->channel,location->chip,location->die,location->plane,location->block,location->page);      /*��¼�����Ч�ƶ�ҳ��ppn���Ա�map���߶���ӳ���ϵ�е�ppn������ɾ������Ӳ���*/
	/*if(ssd->dram->map->update_fre2[lpn] > 12)
	{
		hot_flag = 0;
	}
	else if(ssd->dram->map->update_fre2[lpn] > 8 && ssd->dram->map->update_fre2[lpn] <= 12)
	{
		hot_flag = 1;
	}
	else if(ssd->dram->map->update_fre2[lpn] > 4 && ssd->dram->map->update_fre2[lpn] <= 8)
	{
		hot_flag = 2;
	}
	else 
	{
		hot_flag = 3;
	}*/
	if(ssd->dram->map->psn_entry1[lpn].pn > 4)
	{
		hot_flag = 3;
	}
	else
	{
		hot_flag = 0;
	}
	ssd->hot_cold_flag = hot_flag;
	ppn=get_ppn_for_gc(ssd,location->channel,location->chip,location->die,location->plane);                /*�ҳ�����ppnһ�����ڷ���gc������plane��,����ʹ��copyback������Ϊgc������ȡppn*/

	new_location=find_location(ssd,ppn);                                                                   /*�����»�õ�ppn��ȡnew_location*/
	(* transfer_size)+=size(valid_state);
	
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].free_state=free_state;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].lpn=lpn;
	ssd->channel_head[new_location->channel].chip_head[new_location->chip].die_head[new_location->die].plane_head[new_location->plane].blk_head[new_location->block].page_head[new_location->page].valid_state=valid_state;


	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=0;
	//ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=0;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
	ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].invalid_page++;

	if (old_ppn==ssd->dram->map->map_entry[lpn].pn)                                                     /*�޸�ӳ���*/
	{
		ssd->dram->map->map_entry[lpn].pn=ppn;
	}
	free(new_location);
	new_location=NULL;

	return SUCCESS;
}
Status hot_cold_identify(struct ssd_info * ssd, unsigned int lpn)
{
	unsigned int C = 1;
	//unsigned psn = 0;
	unsigned int Nblock = ssd->parameter->block_plane, Npage = ssd->parameter->page_block * ssd->parameter->block_plane;
	unsigned psn_dif;
	double psn;
	ssd->dram->map->update_fre1[lpn] = ssd->dram->map->update_fre2[lpn];
	psn_dif = abs(ssd->dram->map->psn_entry1[lpn].psn - ssd->dram->map->psn_entry2[lpn].psn);
	if(ssd->dram->map->psn_entry1[lpn].psn == 0) return 0;
	psn = (double)psn_dif / (double)Nblock;
	if(psn > Npage / 2)
	{
		ssd->dram->map->update_fre2[lpn] = C;
	}
	else if(psn <= Npage / 2 && psn >= Npage / 8)
	{
		ssd->dram->map->update_fre2[lpn] = ssd->dram->map->update_fre1[lpn];
	}
	else
	{
		ssd->dram->map->update_fre2[lpn] = ssd->dram->map->update_fre2[lpn] > 16 ? 16 : (ssd->dram->map->update_fre1[lpn] + C);
	}
	
	return 1;
}

/*******************************************************************************************************************************************
*Ŀ���planeû�п���ֱ��ɾ����block����ҪѰ��Ŀ����������ʵʩ�������������ڲ����жϵ�gc�����У��ɹ�ɾ��һ���飬����1��û��ɾ��һ���鷵��-1
*����������У����ÿ���Ŀ��channel,die�Ƿ��ǿ��е�,����invalid_page_num����block
********************************************************************************************************************************************/
int uninterrupt_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)       
{
	unsigned int i=0,j=0,invalid_page=0;
	unsigned int block,block_tmp,active_block,transfer_size,free_page,page_move_count=0;                           /*��¼ʧЧҳ���Ŀ��*/
	struct local *  location=NULL;
	unsigned int total_invalid_page_num=0;
	int block_soft_gc;
	int score = 0, score_tmp = 0;
	int psn = 0;
	unsigned int erase_max, erase_min, erase_mintmp, valid_block;
	unsigned int lpn;
	double te = 0.0;
	if(ssd->parameter->interruptible == 2)
	{
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_soft_head == NULL)
		{
			block_soft_gc = -1;
		}
		else
		{
			block_soft_gc = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].gc_soft_head->block;
		}
	}
	else if(ssd->parameter->interruptible == 1)
	{
		if(ssd->channel_head[channel].gc_soft == NULL)
		{
			block_soft_gc = -1;
		}
		else
		{
			block_soft_gc = ssd->channel_head[channel].gc_soft->block;
		}
	}

	if(find_active_block(ssd,channel,chip,die,plane)!=SUCCESS)                                           /*��ȡ��Ծ��*/
	{
		printf("\n\n Error in uninterrupt_gc().\n");
		return ERROR;
	}
	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;

	invalid_page=0;
	transfer_size=0;
	block=-1;
	if(ssd->parameter->interruptible != 3)
	{
		for(i=0;i<ssd->parameter->block_plane;i++)                                                           /*�������invalid_page�Ŀ�ţ��Լ�����invalid_page_num*/
		{	
			total_invalid_page_num+=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
			if((i!=block_soft_gc)&&(active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
			{				
				invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
				block=i;						
			}
		}
	}
	else
	{
		valid_block = 0;
		/*for(i=0;i<ssd->parameter->block_plane;i++)                                            
		{
			if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num == 0 )						
			{				
				valid_block++;
			}
		}*/
		erase_max = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase;
		erase_min = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].min_erase;
		te = (1.0 - (double)valid_block/ssd->parameter->block_plane) * 100.0 / 2.0;
		//if((double)(erase_max - erase_min) > te)
		if(1)
		{	
			erase_mintmp = erase_max;
			for(i=0;i<ssd->parameter->block_plane;i++)                                                           /*�������invalid_page�Ŀ�ţ��Լ�����invalid_page_num*/
			{	
				//if((i!=block_soft_gc)&&(active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>=invalid_page) && (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count <= erase_mintmp))						
				if((i!=block_soft_gc)&&(active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
				{				
					invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
					erase_mintmp = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count;					
					block=i;						
				}
			}
		}
		else
		{
			score = 0;
			for(i=0;i<ssd->parameter->block_plane;i++)                                                           /*�������invalid_page�Ŀ�ţ��Լ�����invalid_page_num*/
			{	
				score_tmp = 0;
				if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num < ssd->parameter->page_block)
				{
					continue;
				}
				else
				{
					for(j=0;j<ssd->parameter->page_block;j++)
					{
						if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].page_head[j].valid_state == 0 && ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].page_head[j].free_state == 0)
						{
							lpn = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].page_head[j].lpn;
							psn = abs(ssd->dram->map->psn_entry1[lpn].psn - ssd->dram->map->psn_entry2[lpn].psn);
							score_tmp += psn;
						}
					}
					if((i!=block_soft_gc)&&(active_block!=i)&&(score_tmp > score))						
					{				
						score = score_tmp;
						block=i;						
					}
				}
			}
			if(block == -1)
			{
				for(i=0;i<ssd->parameter->block_plane;i++)                                                           /*�������invalid_page�Ŀ�ţ��Լ�����invalid_page_num*/
				{	
					if((i!=block_soft_gc)&&(active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>=invalid_page))						
					{				
						invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
						erase_mintmp = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count;					
						block=i;						
					}
				}
			}
		}
	}
	if(ssd->parameter->interruptible == 0 && ssd->parameter->dram_capacity == 0)
	{
		block_tmp = block;
		invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num;
		erase_mintmp = 0;
		for(i=0;i<ssd->parameter->block_plane;i++)                                                           
		{	
			if((i!=block_soft_gc)&&(active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>=invalid_page) && (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count >= erase_mintmp))						
			//if((i!=block_soft_gc)&&(active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>=invalid_page))						
			{				
				erase_mintmp = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count;					
				block=i;						
			}
		}
	}
	if (block==-1)
	{
		return 1;
	}

	//if(invalid_page<5)
	//{
		//printf("\ntoo less invalid page. \t %d\t %d\t%d\t%d\t%d\t%d\t\n",invalid_page,channel,chip,die,plane,block);
	//}

	free_page=0;
	//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter_th = 0;
	for(i=0;i<ssd->parameter->page_block;i++)		                                                     /*������ÿ��page���������Ч���ݵ�page��Ҫ�ƶ��������ط��洢*/		
	{		
		if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].free_state&PG_SUB)==0x0000000f)
		{
			free_page++;
		}
		if(free_page!=0)
		{
			//printf("\ntoo much free page. \t %d\t .%d\t%d\t%d\t%d\t\n",free_page,channel,chip,die,plane);
		}
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].valid_state>0) /*��ҳ����Чҳ����Ҫcopyback����*/		
		{	
			//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter_th++;
			location=(struct local * )malloc(sizeof(struct local ));
			alloc_assert(location,"location");
			memset(location,0, sizeof(struct local));

			location->channel=channel;
			location->chip=chip;
			location->die=die;
			location->plane=plane;
			location->block=block;
			location->page=i;
			//lpn = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[i].lpn;
			if(ssd->parameter->interruptible == 3)
			{
				move_page_hot_cold(ssd, location, &transfer_size);
			}
			else
			{
				move_page(ssd, location, &transfer_size);                                                   /*��ʵ��move_page����*/
			}
			page_move_count++;

			free(location);	
			location=NULL;

		}				
	}
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter_th = 0.2* page_move_count;
	erase_operation(ssd,channel ,chip , die,plane ,block);	                                              /*ִ����move_page�����󣬾�����ִ��block�Ĳ�������*/
	ssd->page_move_count += page_move_count;
	ssd->channel_head[channel].current_state=CHANNEL_GC;									
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;								
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;			
	
	/***************************************************************
	*�ڿ�ִ��COPYBACK�߼������벻��ִ��COPYBACK�߼���������������£�
	*channel�¸�״̬ʱ��ļ��㣬�Լ�chip�¸�״̬ʱ��ļ���
	***************************************************************/
	if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
	{
		if (ssd->parameter->greed_CB_ad==1)
		{

			ssd->channel_head[channel].next_state_predict_time=ssd->current_time+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG);			
			ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
		} 
	} 
	else if(ssd->parameter->interruptible == 3)
	{

		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC;
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS*0.8;

	}
	else
	{
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+page_move_count*(7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tPROG)+transfer_size*SECTOR*(ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tRC);					
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
	}
	

	return 1;
}


/*******************************************************************************************************************************************
*Ŀ���planeû�п���ֱ��ɾ����block����ҪѰ��Ŀ����������ʵʩ�������������ڿ����жϵ�gc�����У��ɹ�ɾ��һ���飬����1��û��ɾ��һ���鷵��-1
*����������У����ÿ���Ŀ��channel,die�Ƿ��ǿ��е�
********************************************************************************************************************************************/
int interrupt_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,struct gc_operation *gc_node)        
{
	unsigned int i,block=-1,active_block,gc_hard_block,transfer_size,invalid_page=0;
	struct local *location;

	//test
	if(ssd->current_time == 199091540533100)
	{
		//printf("time is coming");
	}
	if(find_active_block(ssd,channel,chip,die,plane)==FAILURE)
	{
		printf("error in DTGC soft gc\n");	
		return 0;
	}

	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	transfer_size=0;
	if (gc_node->block>=ssd->parameter->block_plane)
	{
		for(i=0;i<ssd->parameter->block_plane;i++)
		{			
			if((active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
			{				
				invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
				block=i;						
			}
		}
		gc_node->block=block;
	}
	if(block == -1) return;
	//if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].invalid_page_num < ssd->parameter->page_block)     /*����Ҫִ��copyback����*/
	//{
		for (i=0;i<ssd->parameter->page_block;i++)
		{
			if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].page_head[i].valid_state>0)
			{
				location=(struct local * )malloc(sizeof(struct local ));
				alloc_assert(location,"location");
				memset(location,0, sizeof(struct local));

				location->channel=channel;
				location->chip=chip;
				location->die=die;
				location->plane=plane;
				location->block=block;
				location->page=i;
				transfer_size=0;

				move_page(ssd, location, &transfer_size);

				free(location);
				location=NULL;

				gc_node->page=i+1;
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[gc_node->block].invalid_page_num++;
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].invalid_page++;
				ssd->channel_head[channel].current_state=CHANNEL_C_A_TRANSFER;									
				ssd->channel_head[channel].current_time=ssd->current_time;										
				ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
				ssd->channel_head[channel].chip_head[chip].current_state=CHIP_COPYBACK_BUSY;								
				ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
				ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;		

				if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
				{					
					ssd->channel_head[channel].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+7*ssd->parameter->time_characteristics.tWC;		
					ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
				} 
				else
				{	
					//ssd->channel_head[channel].next_state_predict_time=ssd->current_time+(7+transfer_size*SECTOR)*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(7+transfer_size*SECTOR)*ssd->parameter->time_characteristics.tWC;					
					//ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
				}
				return 0;    
			}
		}
	//}
	//else
	//{
		erase_operation(ssd,channel ,chip, die,plane,gc_node->block);	

		ssd->channel_head[channel].current_state=CHANNEL_C_A_TRANSFER;									
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;								
		ssd->channel_head[channel].next_state_predict_time=ssd->current_time+5*ssd->parameter->time_characteristics.tWC;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_ERASE_BUSY;								
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;						
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;							
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;

		return 1;                                                                      /*��gc������ɣ�����1�����Խ�channel�ϵ�gc����ڵ�ɾ��*/
	//}

	printf("there is a problem in interrupt_gc\n");
	return 1;
}

//KXC_2: find victim block when soft gc
unsigned int find_victim_interrupt_gc(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)        
{
	unsigned int i,j,block = 500000,active_block,transfer_size,invalid_page=0;
	struct local *location;
	unsigned int victim_block = 500000,erase = 0;
	unsigned long max_erase = 0, min_erase = 0;
	double lamda = 0.0;
	unsigned int invalid_block = 0, free_block = 0, valid_block = 0;
	double score = 10000000.0, score_tmp = 0.0, benifit_future = 0.0;
	int flag = 0;

	if(find_active_block(ssd,channel,chip,die,plane)!=SUCCESS)                                           /*��ȡ��Ծ��*/
	{
		printf("\n\n Error in find_victim_interrupt_gc.\n");
		return ERROR;
	}
	active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	transfer_size=0;
	
	//to search the block in the plane
	if(ssd->parameter->wear_leveling == 0)
	{
		for(i=0;i<ssd->parameter->block_plane;i++)
		{			
			if((active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
			{				
				invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
				block=i;						
			}
		}
	}
	
	//KXC:the 2012 meeting's WL  
	if (ssd->parameter->wear_leveling==1)
	{
		max_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase;
		min_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].min_erase;


		if ((max_erase - min_erase) ==0)
		{
			lamda = 0.0;
		}
		else
		{
			lamda = 2.0/(1+exp(10/(max_erase - min_erase)));
		}

		//score = (double)((1-lamda)*(valid_block/(valid_block+invalid_block+1))+lamda*(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count/(1+max_erase)));	
		for (i=0;i<ssd->parameter->block_plane;i++)
		{
			invalid_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
			free_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].free_page_num;
			if(free_block != 0) continue;
			valid_block = ssd->parameter->page_block - invalid_block - free_block;
			score_tmp = (double)((1-lamda)*((double)valid_block/((double)valid_block+invalid_block+1))+lamda*((double)ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count/(1+(double)max_erase)));
			if((active_block!=i)&&(score > score_tmp))
			{
				score = score_tmp;
				block = i;

			}
		}
	}
		
	//KXC:the WL of 2017 paper
	if (ssd->parameter->wear_leveling==2)
	{

		max_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase;
		min_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].min_erase;
		if (max_erase - min_erase >2)   //wl
		{
			flag = 0;
		}
		else
		{
			flag = 1;                   //ferformace
		}

		if (flag == 0)
		{	
			for (i=0;i<ssd->parameter->block_plane;i++)
			{
				free_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].free_page_num;
				if(free_block != 0) continue;
				score_tmp =(double)((double)ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count/((double)max_erase+1)+ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].last_update_time/(double)ssd->current_time);
				if((active_block!=i)&&(score > score_tmp))
				{
					score =score_tmp;
					block = i;
				}
			}
						
		}
		else 
		{
			for(i=0;i<ssd->parameter->block_plane;i++)
			{			
				free_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].free_page_num;
				if(free_block != 0) continue;
				if((active_block!=i)&&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num>invalid_page))						
				{				
					invalid_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
					block=i;						
				}
			}
		}	
		
	}

	if(ssd->parameter->wear_leveling == 3)
	{
		score = -500000.0;
		max_erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].max_erase;
		for (i=0;i<ssd->parameter->block_plane;i++)
		{
			invalid_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].invalid_page_num;
			free_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].free_page_num;
			if(free_block != 0) continue;
			valid_block = ssd->parameter->page_block - invalid_block - free_block;
			erase = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[i].erase_count;

			/*for(j = 1; j < ssd->parameter->ers_limit - erase +1;j++)
			{
				benifit_future += (double)ssd->parameter->page_block/(pow(1+0.5,j));
			}*/
			benifit_future = ssd->mi[max_erase - erase];
			//benifit_future = ssd->mi[ssd->parameter->ers_limit - erase];
			//score_tmp = (double) invalid_block/ssd->parameter->page_block + benifit_future/(max_erase+10)/10 - (double) valid_block/ssd->parameter->page_block;
			//score_tmp = (double) invalid_block/ssd->parameter->page_block + benifit_future/ssd->parameter->ers_limit - (double) valid_block/ssd->parameter->page_block;
			score_tmp = (double) invalid_block/ssd->parameter->page_block + benifit_future/ssd->parameter->ers_limit;
			if((active_block!=i)&&(score < score_tmp))
			{
				score = score_tmp;
				block = i;

			}
		}
	}

	victim_block=block;

	return victim_block;
	
}

/*************************************************************
*�����Ĺ����ǵ�������һ��gc����ʱ����Ҫ��gc���ϵ�gc_nodeɾ����
**************************************************************/
int delete_gc_node(struct ssd_info *ssd, unsigned int channel,struct gc_operation *gc_node)
{
	struct gc_operation *gc_pre=NULL;
	if(gc_node==NULL)                                                                  
	{
		return ERROR;
	}
	if(ssd->parameter->interruptible == 2)
	{
		if(gc_node->priority == GC_UNINTERRUPT)
		{
			if (gc_node==ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].gc_hard_head)
			{
				ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].gc_hard_head=gc_node->next_node;
			}
			else
			{
				gc_pre=ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].gc_hard_head;
				while (gc_pre->next_node!=NULL)
				{
					if (gc_pre->next_node==gc_node)
					{
						gc_pre->next_node=gc_node->next_node;
						break;
					}
					gc_pre=gc_pre->next_node;
				}
			}
		}
		else
		{
			if (gc_node==ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].gc_soft_head)
			{
				ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].gc_soft_head=gc_node->next_node;
			}
			else
			{
				gc_pre=ssd->channel_head[channel].chip_head[gc_node->chip].die_head[gc_node->die].plane_head[gc_node->plane].gc_soft_head;
				while (gc_pre->next_node!=NULL)
				{
					if (gc_pre->next_node==gc_node)
					{
						gc_pre->next_node=gc_node->next_node;
						break;
					}
					gc_pre=gc_pre->next_node;
				}
			}
		}
		
	}
	else
	{
		if (gc_node==ssd->channel_head[channel].gc_command)
		{
			ssd->channel_head[channel].gc_command=gc_node->next_node;
		}
		else
		{
			gc_pre=ssd->channel_head[channel].gc_command;
			while (gc_pre->next_node!=NULL)
			{
				if (gc_pre->next_node==gc_node)
				{
					gc_pre->next_node=gc_node->next_node;
					break;
				}
				gc_pre=gc_pre->next_node;
			}
		}
	}
	free(gc_node);
	gc_node=NULL;
	ssd->gc_request--;
	return SUCCESS;
}

/***************************************
*��������Ĺ����Ǵ���channel��ÿ��gc����
****************************************/
Status gc_for_channel(struct ssd_info *ssd, unsigned int channel)
{
	int flag_direct_erase=1,flag_gc=1,flag_invoke_gc=1,i,j,m,n;
	unsigned int chip,die,plane,flag_priority=0;
	unsigned int current_state=0, next_state=0;
	long long next_state_predict_time=0;
	struct gc_operation *gc_node=NULL,*gc_p=NULL;

	/*******************************************************************************************
	*����ÿһ��gc_node����ȡgc_node���ڵ�chip�ĵ�ǰ״̬���¸�״̬���¸�״̬��Ԥ��ʱ��
	*�����ǰ״̬�ǿ��У������¸�״̬�ǿ��ж��¸�״̬��Ԥ��ʱ��С�ڵ�ǰʱ�䣬�����ǲ����жϵ�gc
	*��ô��flag_priority��Ϊ1������Ϊ0
	*������ȫ���ǲ����ж�GC�ˣ�������ж϶��࣬����û��ɾ��
	********************************************************************************************/
	if(ssd->parameter->interruptible == 2 && ssd->gc_type == 1)
	{
		for(j=0;j<ssd->parameter->chip_channel[0];j++)
		{
			current_state=ssd->channel_head[channel].chip_head[j].current_state;
			next_state=ssd->channel_head[channel].chip_head[j].next_state;
			next_state_predict_time=ssd->channel_head[channel].chip_head[j].next_state_predict_time;
			if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))
			{
				for(m=0;m<ssd->parameter->die_chip;m++)
				{
					for (n=0;n<ssd->parameter->plane_die; n++)
					{
						gc_node=ssd->channel_head[channel].chip_head[j].die_head[m].plane_head[n].gc_hard_head;
						if(gc_node != NULL)
						{
							break;
						}
					}
					if(gc_node != NULL)
					{
						break;
					}
				}
				if(gc_node != NULL)
				{
					break;
				}
			}
		}	
	}
	else if(ssd->parameter->interruptible == 2 && ssd->gc_type == 0)
	{
		for(j=0;j<ssd->parameter->chip_channel[0];j++)
		{
			current_state=ssd->channel_head[channel].chip_head[j].current_state;
			next_state=ssd->channel_head[channel].chip_head[j].next_state;
			next_state_predict_time=ssd->channel_head[channel].chip_head[j].next_state_predict_time;
			if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))
			{
				for(m=0;m<ssd->parameter->die_chip;m++)
				{
					for (n=0;n<ssd->parameter->plane_die; n++)
					{
						gc_node=ssd->channel_head[channel].chip_head[j].die_head[m].plane_head[n].gc_soft_head;
						if(gc_node != NULL)
						{
							break;
						}
					}
					if(gc_node != NULL)
					{
						break;
					}
				}
				if(gc_node != NULL)
				{
					break;
				}
			}
		}
	}
	else
	{
		gc_node=ssd->channel_head[channel].gc_command;
	}
	
	if(ssd->parameter->interruptible == 2)
	{

	}
	else
	{
		while (gc_node!=NULL)
		{
			current_state=ssd->channel_head[channel].chip_head[gc_node->chip].current_state;
			next_state=ssd->channel_head[channel].chip_head[gc_node->chip].next_state;
			next_state_predict_time=ssd->channel_head[channel].chip_head[gc_node->chip].next_state_predict_time;
			if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))
			{
				if (gc_node->priority==GC_UNINTERRUPT)                                     /*���gc�����ǲ����жϵģ����ȷ������gc����*/
				{
					flag_priority=1;
					break;
				}
			}
			gc_node=gc_node->next_node;
		}
		if (flag_priority!=1)                                                              /*û���ҵ������жϵ�gc��������ִ�ж��׵�gc����*/
		{
			gc_node=ssd->channel_head[channel].gc_command;
			while (gc_node!=NULL)
			{
				current_state=ssd->channel_head[channel].chip_head[gc_node->chip].current_state;
				next_state=ssd->channel_head[channel].chip_head[gc_node->chip].next_state;
				next_state_predict_time=ssd->channel_head[channel].chip_head[gc_node->chip].next_state_predict_time;
				/**********************************************
				 *��Ҫgc������Ŀ��chip�ǿ��еģ��ſ��Խ���gc����
				***********************************************/
				if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))   
				{
					break;
				}
				gc_node=gc_node->next_node;
			}
		}
	}
	if(gc_node==NULL)
	{
		return FAILURE;
	}
	chip = gc_node->chip;
	die = gc_node->die;
	plane = gc_node->plane;

	if (gc_node->priority==GC_UNINTERRUPT)
	{
		/*flag_direct_erase=gc_direct_erase(ssd,channel,chip,die,plane);
		if (flag_direct_erase!=SUCCESS)
		{
			flag_gc=uninterrupt_gc(ssd,channel,chip,die,plane);                         //*��һ��������gc�������ʱ���Ѿ�����һ���飬������һ��������flash�ռ䣩������1����channel����Ӧ��gc��������ڵ�ɾ��
			if (flag_gc==1)
			{
				delete_gc_node(ssd,channel,gc_node);
			}
		}
		else
		{
			delete_gc_node(ssd,channel,gc_node);
		}*/
		flag_gc=uninterrupt_gc(ssd,channel,chip,die,plane);                         //*��һ��������gc�������ʱ���Ѿ�����һ���飬������һ��������flash�ռ䣩������1����channel����Ӧ��gc��������ڵ�ɾ��
		if (flag_gc==1)
		{
			delete_gc_node(ssd,channel,gc_node);
		}
		return SUCCESS;
	}
	/*******************************************************************************
	*���жϵ�gc������Ҫ����ȷ�ϸ�channel��û�������������ʱ����Ҫʹ�����channel��
	*û�еĻ�����ִ��gc�������еĻ�����ִ��gc����
	********************************************************************************/
	//KXC_2: the interruptible gc of out is not process here
	else        
	{
		flag_invoke_gc=decide_gc_invoke(ssd,channel);                                  //�ж��Ƿ�����������Ҫchannel���������������Ҫ���channel����ô���gc�����ͱ��ж���

		/*if (flag_invoke_gc==1)
		{
			flag_direct_erase=gc_direct_erase(ssd,channel,chip,die,plane);
			if (flag_direct_erase==-1)
			{
				flag_gc=interrupt_gc(ssd,channel,chip,die,plane,gc_node);             //��һ��������gc�������ʱ���Ѿ�����һ���飬������һ��������flash�ռ䣩������1����channel����Ӧ��gc��������ڵ�ɾ��
				if (flag_gc==1)
				{
					delete_gc_node(ssd,channel,gc_node);
				}
			}
			else if (flag_direct_erase==1)
			{
				delete_gc_node(ssd,channel,gc_node);
			}
			return SUCCESS;
		}*/ 
		if(flag_invoke_gc==1)
		{
			flag_gc=interrupt_gc(ssd,channel,chip,die,plane,gc_node);                         //*��һ��������gc�������ʱ���Ѿ�����һ���飬������һ��������flash�ռ䣩������1����channel����Ӧ��gc��������ڵ�ɾ��
			if (flag_gc==1)
			{
				delete_gc_node(ssd,channel,gc_node);
			}
		return SUCCESS;
		}
		else
		{
			return FAILURE;
		}		
	}
}



/************************************************************************************************************
*flag�������gc��������ssd��������idle������±����õģ�1��������ȷ����channel��chip��die��plane�����ã�0��
*����gc��������Ҫ�ж��Ƿ��ǲ����жϵ�gc����������ǣ���Ҫ��һ����Ŀ��block��ȫ�����������ɣ�����ǿ��жϵģ�
*�ڽ���GC����ǰ����Ҫ�жϸ�channel��die�Ƿ����������ڵȴ����������û����ʼһ��һ���Ĳ������ҵ�Ŀ��
*���һ��ִ��һ��copyback����������gc��������ʱ����ǰ�ƽ���������һ��copyback����erase����
*����gc������һ����Ҫ����gc��������Ҫ����һ�����жϣ�������Ӳ��ֵ����ʱ���������gc����������������ֵ����ʱ��
*��Ҫ�жϣ������channel���Ƿ����������ڵȴ�(��д������ȴ��Ͳ��У�gc��Ŀ��die����busy״̬Ҳ����)�����
*�оͲ�ִ��gc���������������ִ��һ������
************************************************************************************************************/
unsigned int gc(struct ssd_info *ssd,unsigned int channel, unsigned int flag)
{
	unsigned int i;
	int flag_direct_erase=1,flag_gc=1,flag_invoke_gc=1;
	unsigned int flag_priority=0;
	struct gc_operation *gc_node=NULL,*gc_p=NULL;

	//KXC_2: ����û�õ�falg == 1��ÿ��ͨ�������ж�

	if (flag==1)                                                                       /*����ssd����IDEL�����*/
	{
		for (i=0;i<ssd->parameter->channel_number;i++)
		{
			flag_priority=0;
			flag_direct_erase=1;
			flag_gc=1;
			flag_invoke_gc=1;
			gc_node=NULL;
			gc_p=NULL;
			if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time))
			{
				channel=i;
				if (ssd->channel_head[channel].gc_command!=NULL)
				{
					gc_for_channel(ssd, channel);
				}
			}
		}
		return SUCCESS;

	} 
	else                                                                               /*ֻ�����ĳ���ض���channel��chip��die����gc����Ĳ���(ֻ���Ŀ��die�����ж������ǲ���idle��*/
	{
		//���ﲻ���ж�GC�������ж�����������ֱ�ӽ���GC
		/*if ((ssd->parameter->allocation_scheme==1)||((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==1)))
		{
			if ((ssd->channel_head[channel].subs_r_head!=NULL)||(ssd->channel_head[channel].subs_w_head!=NULL))    //�������������ȷ�������
			{
				return 0;
			}
		}*/

		gc_for_channel(ssd,channel);
		return SUCCESS;
	}
}



/**********************************************************
*�ж��Ƿ�����������Ҫchannel������û�з���1�Ϳ��Է���gc����
*����з���0���Ͳ���ִ��gc������gc�������ж�
***********************************************************/
int decide_gc_invoke(struct ssd_info *ssd, unsigned int channel)      
{
	struct sub_request *sub;
	struct local *location;

	if ((ssd->channel_head[channel].subs_r_head==NULL)&&(ssd->channel_head[channel].subs_w_head==NULL))    /*������Ҷ�д�������Ƿ���Ҫռ�����channel�����õĻ�����ִ��GC����*/
	{
		return 1;                                                                        /*��ʾ��ǰʱ�����channelû����������Ҫռ��channel*/
	}
	else
	{
		if (ssd->channel_head[channel].subs_w_head!=NULL)
		{
			return 0;
		}
		else if (ssd->channel_head[channel].subs_r_head!=NULL)
		{
			sub=ssd->channel_head[channel].subs_r_head;
			while (sub!=NULL)
			{
				if (sub->current_state==SR_WAIT)                                         /*����������Ǵ��ڵȴ�״̬���������Ŀ��die����idle������ִ��gc����������0*/
				{
					location=find_location(ssd,sub->ppn);
					if ((ssd->channel_head[location->channel].chip_head[location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[location->channel].chip_head[location->chip].next_state==CHIP_IDLE)&&
						(ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time<=ssd->current_time)))
					{
						free(location);
						location=NULL;
						return 0;
					}
					free(location);
					location=NULL;
				}
				else if (sub->next_state==SR_R_DATA_TRANSFER)
				{
					location=find_location(ssd,sub->ppn);
					if (ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time<=ssd->current_time)
					{
						free(location);
						location=NULL;
						return 0;
					}
					free(location);
					location=NULL;
				}
				sub=sub->next_node;
			}
		}
		return 1;
	}
}

