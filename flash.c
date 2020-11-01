/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName�� flash.c
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

#include "flash.h"

/**********************
*�������ֻ������д����
***********************/
Status allocate_location(struct ssd_info * ssd ,struct sub_request *sub_req)
{
	struct sub_request * update=NULL;
	unsigned int channel_num=0,chip_num=0,die_num=0,plane_num=0;
	struct local *location=NULL;

	channel_num=ssd->parameter->channel_number;
	chip_num=ssd->parameter->chip_channel[0];
	die_num=ssd->parameter->die_chip;
	plane_num=ssd->parameter->plane_die;
    
	
	if (ssd->parameter->allocation_scheme==0)                                          /*��̬��������*/
	{
		/******************************************************************
		* �ڶ�̬�����У���Ϊҳ�ĸ��²���ʹ�ò���copyback������
		*��Ҫ����һ�������󣬲���ֻ�������������ɺ���ܽ������ҳ��д����
		*******************************************************************/

		if (ssd->dram->map->map_entry[sub_req->lpn].state!=0)    //ӳ�����д�����Ч������
		{
			if ((sub_req->state&ssd->dram->map->map_entry[sub_req->lpn].state)!=ssd->dram->map->map_entry[sub_req->lpn].state)
			{
				ssd->read_count++;             //��������ԭ������Ҫ�������ݲ����ж������Ƿ�Ϊд����
				ssd->update_read_count++;

				update=(struct sub_request *)malloc(sizeof(struct sub_request));
				alloc_assert(update,"update");
				memset(update,0, sizeof(struct sub_request));

				if(update==NULL)
				{
					return ERROR;
				}
				update->location=NULL;
				update->next_node=NULL;
				update->next_subs=NULL;
				update->update=NULL;						
				location = find_location(ssd,ssd->dram->map->map_entry[sub_req->lpn].pn);
				update->location=location;
				update->begin_time = ssd->current_time;
				update->current_state = SR_WAIT;
				update->current_time=MAX_INT64;
				update->next_state = SR_R_C_A_TRANSFER;
				update->next_state_predict_time=MAX_INT64;
				update->lpn = sub_req->lpn;
				update->state=((ssd->dram->map->map_entry[sub_req->lpn].state^sub_req->state)&0x7fffffff);
				update->size=size(update->state);
				update->ppn = ssd->dram->map->map_entry[sub_req->lpn].pn;
				update->operation = READ;
				
				if (ssd->channel_head[location->channel].subs_r_tail!=NULL)            /*�����µĶ����󣬲��ҹҵ�channel��subs_r_tail����β*/
				{
						ssd->channel_head[location->channel].subs_r_tail->next_node=update;
						ssd->channel_head[location->channel].subs_r_tail=update;
				} 
				else
				{
					ssd->channel_head[location->channel].subs_r_tail=update;
					ssd->channel_head[location->channel].subs_r_head=update;
				}
			}
		}
		
		/***************************************
		*һ���Ƕ�̬����ļ������
		*0��ȫ��̬����
		*1����ʾchannel��package��die��plane��̬
		****************************************/
		switch(ssd->parameter->dynamic_allocation)
		{
			case 0:
			{
				sub_req->location->channel=-1;
				sub_req->location->chip=-1;
				sub_req->location->die=-1;
				sub_req->location->plane=-1;
				sub_req->location->block=-1;
				sub_req->location->page=-1;

				if (ssd->subs_w_tail!=NULL)
				{
					ssd->subs_w_tail->next_node=sub_req;
					ssd->subs_w_tail=sub_req;
				} 
				else
				{
					ssd->subs_w_tail=sub_req;
					ssd->subs_w_head=sub_req;
				}

				if (update!=NULL)
				{
					sub_req->update=update;
				}

				break;
			}
			case 1:
			{ 
				//sub_req->location->channel=sub_req->lpn%ssd->parameter->channel_number;
				sub_req->location->channel = ssd->ch_token;
				ssd->ch_token = (ssd->ch_token + 1) % ssd->parameter->channel_number;
				//if(ssd->parameter->interruptible == 0 && ssd->program_count % 1000 == 0)
				/*if(ssd->parameter->interruptible == 0 && ssd->parameter->dram_capacity == 0)
				{
					if(ssd->write_request_count > 0 && ssd->write_request_count < 1000000)
					ssd->ch_token = (ssd->ch_token + 1) % ssd->parameter->channel_number;
				}*/
				sub_req->location->chip=-1;
				sub_req->location->die=-1;
				sub_req->location->plane=-1;
				sub_req->location->block=-1;
				sub_req->location->page=-1;

				if (update!=NULL)
				{
					sub_req->update=update;
				}

				break;
			}
			case 2:
			{
				break;
			}
			case 3:
			{
				break;
			}
		}

	}
	else                                                                          
	{   /***************************************************************************
		*�Ǿ�̬���䷽ʽ�����Կ��Խ���������������channel��chip��die��planeȫ���ó�
		*�ܹ���0,1,2,3,4,5,�����־�̬���䷽ʽ��
		****************************************************************************/
		switch (ssd->parameter->static_allocation)
		{
			case 0:         //no striping static allocation
			{
				sub_req->location->channel=(sub_req->lpn/(plane_num*die_num*chip_num))%channel_num;
				sub_req->location->chip=sub_req->lpn%chip_num;
				sub_req->location->die=(sub_req->lpn/chip_num)%die_num;
				sub_req->location->plane=(sub_req->lpn/(die_num*chip_num))%plane_num;
				break;
			}
			case 1:
			{
				sub_req->location->channel=sub_req->lpn%channel_num;
				sub_req->location->chip=(sub_req->lpn/channel_num)%chip_num;
				sub_req->location->die=(sub_req->lpn/(chip_num*channel_num))%die_num;
				sub_req->location->plane=(sub_req->lpn/(die_num*chip_num*channel_num))%plane_num;
							
				break;
			}
			case 2:
			{
				sub_req->location->channel=sub_req->lpn%channel_num;
				sub_req->location->chip=(sub_req->lpn/(plane_num*channel_num))%chip_num;
				sub_req->location->die=(sub_req->lpn/(plane_num*chip_num*channel_num))%die_num;
				sub_req->location->plane=(sub_req->lpn/channel_num)%plane_num;
				break;
			}
			case 3:
			{
				sub_req->location->channel=sub_req->lpn%channel_num;
				sub_req->location->chip=(sub_req->lpn/(die_num*channel_num))%chip_num;
				sub_req->location->die=(sub_req->lpn/channel_num)%die_num;
				sub_req->location->plane=(sub_req->lpn/(die_num*chip_num*channel_num))%plane_num;
				break;
			}
			case 4:  
			{
				sub_req->location->channel=sub_req->lpn%channel_num;
				sub_req->location->chip=(sub_req->lpn/(plane_num*die_num*channel_num))%chip_num;
				sub_req->location->die=(sub_req->lpn/(plane_num*channel_num))%die_num;
				sub_req->location->plane=(sub_req->lpn/channel_num)%plane_num;
							
				break;
			}
			case 5:   
			{
				sub_req->location->channel=sub_req->lpn%channel_num;
				sub_req->location->chip=(sub_req->lpn/(plane_num*die_num*channel_num))%chip_num;
				sub_req->location->die=(sub_req->lpn/channel_num)%die_num;
				sub_req->location->plane=(sub_req->lpn/(die_num*channel_num))%plane_num;
							
				break;
			}
			default : return ERROR;
		
		}

		if (ssd->dram->map->map_entry[sub_req->lpn].state!=0)
		{                                                                              /*���д�ص���������߼�ҳ�����Ը���֮ǰ��д�ص����� ��Ҫ����������*/ 
			if ((sub_req->state&ssd->dram->map->map_entry[sub_req->lpn].state)!=ssd->dram->map->map_entry[sub_req->lpn].state)  
			{
				ssd->read_count++;
				ssd->update_read_count++;
				update=(struct sub_request *)malloc(sizeof(struct sub_request));
				alloc_assert(update,"update");
				memset(update,0, sizeof(struct sub_request));
				
				if(update==NULL)
				{
					return ERROR;
				}
				update->location=NULL;
				update->next_node=NULL;
				update->next_subs=NULL;
				update->update=NULL;						
				location = find_location(ssd,ssd->dram->map->map_entry[sub_req->lpn].pn);
				update->location=location;
				update->begin_time = ssd->current_time;
				update->current_state = SR_WAIT;
				update->current_time=MAX_INT64;
				update->next_state = SR_R_C_A_TRANSFER;
				update->next_state_predict_time=MAX_INT64;
				update->lpn = sub_req->lpn;
				update->state=((ssd->dram->map->map_entry[sub_req->lpn].state^sub_req->state)&0x7fffffff);
				update->size=size(update->state);
				update->ppn = ssd->dram->map->map_entry[sub_req->lpn].pn;
				update->operation = READ;
				
				if (ssd->channel_head[location->channel].subs_r_tail!=NULL)
				{
					ssd->channel_head[location->channel].subs_r_tail->next_node=update;
					ssd->channel_head[location->channel].subs_r_tail=update;
				} 
				else
				{
					ssd->channel_head[location->channel].subs_r_tail=update;
					ssd->channel_head[location->channel].subs_r_head=update;
				}
			}

			if (update!=NULL)
			{
				sub_req->update=update;

				sub_req->state=(sub_req->state|update->state);
				sub_req->size=size(sub_req->state);
			}

		}
		
	}
	if ((ssd->parameter->allocation_scheme!=0)||(ssd->parameter->dynamic_allocation!=0))
	{
		if (ssd->channel_head[sub_req->location->channel].subs_w_tail!=NULL)
		{
			ssd->channel_head[sub_req->location->channel].subs_w_tail->next_node=sub_req;
			ssd->channel_head[sub_req->location->channel].subs_w_tail=sub_req;
		} 
		else
		{
			ssd->channel_head[sub_req->location->channel].subs_w_tail=sub_req;
			ssd->channel_head[sub_req->location->channel].subs_w_head=sub_req;
		}
	}
	return SUCCESS;					
}	


/*******************************************************************************
*insert2buffer���������ר��Ϊд�������������������buffer_management�б����á�
********************************************************************************/
struct ssd_info * insert2buffer(struct ssd_info *ssd,unsigned int lpn,int state,struct sub_request *sub,struct request *req)      
{
	int write_back_count,flag=0;                                                             /*flag��ʾΪд���������ڿռ��Ƿ���ɣ�0��ʾ��Ҫ��һ���ڣ�1��ʾ�Ѿ��ڿ�*/
	unsigned int i,lsn,hit_flag,add_flag,sector_count,active_region_flag=0,free_sector=0;
	struct buffer_group *buffer_node=NULL,*pt,*new_node=NULL,key;
	struct sub_request *sub_req=NULL,*update=NULL;
	
	
	unsigned int sub_req_state=0, sub_req_size=0,sub_req_lpn=0;

	#ifdef DEBUG
	printf("enter insert2buffer,  current time:%lld, lpn:%d, state:%d,\n",ssd->current_time,lpn,state);
	#endif

	sector_count=size(state);                                                                /*��Ҫд��buffer��sector����*/
	key.group=lpn;
	buffer_node= (struct buffer_group*)avlTreeFind(ssd->dram->buffer, (TREE_NODE *)&key);    /*��ƽ���������Ѱ��buffer node*/ 
    
	/************************************************************************************************
	*û�����С�
	*��һ���������lpn�ж�����ҳ��Ҫд��buffer��ȥ����д�ص�lsn��Ϊ��lpn�ڳ�λ�ã�
	*���ȼ�Ҫ�����free sector����ʾ���ж��ٿ���ֱ��д��buffer�ڵ㣩��
	*���free_sector>=sector_count�����ж���Ŀռ乻lpn������д������Ҫ����д������
	*����û�ж���Ŀռ乩lpn������д����ʱ��Ҫ�ͷ�һ���ֿռ䣬����д�����󡣾�Ҫcreat_sub_request()
	*************************************************************************************************/
	if(buffer_node==NULL)
	{
		free_sector=ssd->dram->buffer->max_buffer_sector-ssd->dram->buffer->buffer_sector_count;   
		if(free_sector>=sector_count)
		{
			flag=1;    
		}
		if(flag==0)     
		{
			write_back_count=sector_count-free_sector;                //����Ҫ�ӻ������޳���������
			ssd->dram->buffer->write_miss_hit=ssd->dram->buffer->write_miss_hit+write_back_count;//�����пռ�д���ʾ���У��޿ռ�д����Ϊδ����
			while(write_back_count>0)
			{
				sub_req=NULL;
				sub_req_state=ssd->dram->buffer->buffer_tail->stored; 
				sub_req_size=size(ssd->dram->buffer->buffer_tail->stored);
				sub_req_lpn=ssd->dram->buffer->buffer_tail->group;
				sub_req=creat_sub_request(ssd,sub_req_lpn,sub_req_size,sub_req_state,req,WRITE);
				
				/**********************************************************************************
				*req��Ϊ�գ���ʾ���insert2buffer��������buffer_management�е��ã�������request����
				*reqΪ�գ���ʾ�����������process�����д���һ�Զ�ӳ���ϵ�Ķ���ʱ����Ҫ���������
				*�����ݼӵ�buffer�У�����ܲ���ʵʱ��д�ز�������Ҫ�����ʵʱ��д�ز��������������
				*������������������
				***********************************************************************************/
				if(req!=NULL)                                             
				{
				}
				else    
				{
					sub_req->next_subs=sub->next_subs;
					sub->next_subs=sub_req;
				}
                
				/*********************************************************************
				*д������뵽��ƽ�����������ʱ��Ҫ�޸�dram��buffer_sector_count��
				*ά��ƽ�����������avlTreeDel()��AVL_TREENODE_FREE()������ά��LRU�㷨��
				**********************************************************************/
				ssd->dram->buffer->buffer_sector_count=ssd->dram->buffer->buffer_sector_count-sub_req->size;
				pt = ssd->dram->buffer->buffer_tail;
				avlTreeDel(ssd->dram->buffer, (TREE_NODE *) pt);
				if(ssd->dram->buffer->buffer_head->LRU_link_next == NULL){
					ssd->dram->buffer->buffer_head = NULL;
					ssd->dram->buffer->buffer_tail = NULL;
				}else{
					ssd->dram->buffer->buffer_tail=ssd->dram->buffer->buffer_tail->LRU_link_pre;
					ssd->dram->buffer->buffer_tail->LRU_link_next=NULL;
				}
				pt->LRU_link_next=NULL;
				pt->LRU_link_pre=NULL;
				AVL_TREENODE_FREE(ssd->dram->buffer, (TREE_NODE *) pt);
				pt = NULL;
				
				write_back_count=write_back_count-sub_req->size;                            /*��Ϊ������ʵʱд�ز�������Ҫ������д�ز�����������*/
			}
		}
		
		/******************************************************************************
		*����һ��buffer node���������ҳ������ֱ�ֵ��������Ա����ӵ����׺Ͷ�������
		*******************************************************************************/
		new_node=NULL;
		new_node=(struct buffer_group *)malloc(sizeof(struct buffer_group));
		alloc_assert(new_node,"buffer_group_node");
		memset(new_node,0, sizeof(struct buffer_group));
		
		new_node->group=lpn;
		new_node->stored=state;
		new_node->dirty_clean=state;
		new_node->LRU_link_pre = NULL;
		new_node->LRU_link_next=ssd->dram->buffer->buffer_head;
		if(ssd->dram->buffer->buffer_head != NULL){
			ssd->dram->buffer->buffer_head->LRU_link_pre=new_node;      //LRU
		}else{
			ssd->dram->buffer->buffer_tail = new_node;
		}
		ssd->dram->buffer->buffer_head=new_node;
		new_node->LRU_link_pre=NULL;
		avlTreeAdd(ssd->dram->buffer, (TREE_NODE *) new_node);
		ssd->dram->buffer->buffer_sector_count += sector_count;
	}
	/****************************************************************************************
	*��buffer�����е����
	*��Ȼ�����ˣ��������е�ֻ��lpn���п���������д����ֻ����Ҫдlpn��һpage��ĳ����sub_page
	*��ʱ����Ҫ��һ�����ж�
	*****************************************************************************************/
	else
	{
		for(i=0;i<ssd->parameter->subpage_page;i++)
		{
			/*************************************************************
			*�ж�state��iλ�ǲ���1
			*�����жϵ�i��sector�Ƿ����buffer�У�1��ʾ���ڣ�0��ʾ�����ڡ�
			**************************************************************/
			if((state>>i)%2!=0)                                                         
			{
				lsn=lpn*ssd->parameter->subpage_page+i;
				hit_flag=0;
				hit_flag=(buffer_node->stored)&(0x00000001<<i);
				
				if(hit_flag!=0)				                                          /*�����ˣ���Ҫ���ýڵ��Ƶ�buffer�Ķ��ף����ҽ����е�lsn���б��*/
				{	
					active_region_flag=1;                                             /*������¼�����buffer node�е�lsn�Ƿ����У����ں������ֵ���ж�*/

					if(req!=NULL)
					{
						if(ssd->dram->buffer->buffer_head!=buffer_node)     
						{				
							if(ssd->dram->buffer->buffer_tail==buffer_node)
							{				
								ssd->dram->buffer->buffer_tail=buffer_node->LRU_link_pre;
								buffer_node->LRU_link_pre->LRU_link_next=NULL;					
							}				
							else if(buffer_node != ssd->dram->buffer->buffer_head)
							{					
								buffer_node->LRU_link_pre->LRU_link_next=buffer_node->LRU_link_next;				
								buffer_node->LRU_link_next->LRU_link_pre=buffer_node->LRU_link_pre;
							}				
							buffer_node->LRU_link_next=ssd->dram->buffer->buffer_head;	
							ssd->dram->buffer->buffer_head->LRU_link_pre=buffer_node;
							buffer_node->LRU_link_pre=NULL;				
							ssd->dram->buffer->buffer_head=buffer_node;					
						}					
						ssd->dram->buffer->write_hit++;
						req->complete_lsn_count++;                                        /*�ؼ� ����buffer������ʱ ����req->complete_lsn_count++��ʾ��buffer��д�����ݡ�*/					
					}
					else
					{
					}				
				}			
				else                 			
				{
					/************************************************************************************************************
					*��lsnû�����У����ǽڵ���buffer�У���Ҫ�����lsn�ӵ�buffer�Ķ�Ӧ�ڵ���
					*��buffer��ĩ����һ���ڵ㣬��һ���Ѿ�д�ص�lsn�ӽڵ���ɾ��(����ҵ��Ļ�)����������ڵ��״̬��ͬʱ������µ�
					*lsn�ӵ���Ӧ��buffer�ڵ��У��ýڵ������bufferͷ�����ڵĻ��������Ƶ�ͷ�������û���ҵ��Ѿ�д�ص�lsn����buffer
					*�ڵ���һ��group����д�أ�����������������������ϡ�������ǰ����һ��channel�ϡ�
					*��һ��:��buffer��β���Ѿ�д�صĽڵ�ɾ��һ����Ϊ�µ�lsn�ڳ��ռ䣬������Ҫ�޸Ķ�βĳ�ڵ��stored״̬���ﻹ��Ҫ
					*       ���ӣ���û�п���֮��ɾ����lsnʱ����Ҫ�����µ�д������д��LRU���Ľڵ㡣
					*�ڶ���:���µ�lsn�ӵ�������buffer�ڵ��С�
					*************************************************************************************************************/	
					ssd->dram->buffer->write_miss_hit++;
					
					if(ssd->dram->buffer->buffer_sector_count>=ssd->dram->buffer->max_buffer_sector)
					{
						if (buffer_node==ssd->dram->buffer->buffer_tail)                  /*������еĽڵ���buffer�����һ���ڵ㣬������������ڵ�*/
						{
							pt = ssd->dram->buffer->buffer_tail->LRU_link_pre;
							ssd->dram->buffer->buffer_tail->LRU_link_pre=pt->LRU_link_pre;
							ssd->dram->buffer->buffer_tail->LRU_link_pre->LRU_link_next=ssd->dram->buffer->buffer_tail;
							ssd->dram->buffer->buffer_tail->LRU_link_next=pt;
							pt->LRU_link_next=NULL;
							pt->LRU_link_pre=ssd->dram->buffer->buffer_tail;
							ssd->dram->buffer->buffer_tail=pt;
							
						}
						sub_req=NULL;
						sub_req_state=ssd->dram->buffer->buffer_tail->stored; 
						sub_req_size=size(ssd->dram->buffer->buffer_tail->stored);
						sub_req_lpn=ssd->dram->buffer->buffer_tail->group;
						sub_req=creat_sub_request(ssd,sub_req_lpn,sub_req_size,sub_req_state,req,WRITE);

						if(req!=NULL)           
						{
							
						}
						else if(req==NULL)   
						{
							sub_req->next_subs=sub->next_subs;
							sub->next_subs=sub_req;
						}

						ssd->dram->buffer->buffer_sector_count=ssd->dram->buffer->buffer_sector_count-sub_req->size;
						pt = ssd->dram->buffer->buffer_tail;	
						avlTreeDel(ssd->dram->buffer, (TREE_NODE *) pt);
							
						/************************************************************************/
						/* ��:  ������������buffer�Ľڵ㲻Ӧ����ɾ����						*/
						/*			��ȵ�д����֮�����ɾ��									*/
						/************************************************************************/
						if(ssd->dram->buffer->buffer_head->LRU_link_next == NULL)
						{
							ssd->dram->buffer->buffer_head = NULL;
							ssd->dram->buffer->buffer_tail = NULL;
						}else{
							ssd->dram->buffer->buffer_tail=ssd->dram->buffer->buffer_tail->LRU_link_pre;
							ssd->dram->buffer->buffer_tail->LRU_link_next=NULL;
						}
						pt->LRU_link_next=NULL;
						pt->LRU_link_pre=NULL;
						AVL_TREENODE_FREE(ssd->dram->buffer, (TREE_NODE *) pt);
						pt = NULL;	
					}

					                                                                     /*�ڶ���:���µ�lsn�ӵ�������buffer�ڵ���*/	
					add_flag=0x00000001<<(lsn%ssd->parameter->subpage_page);
					
					if(ssd->dram->buffer->buffer_head!=buffer_node)                      /*�����buffer�ڵ㲻��buffer�Ķ��ף���Ҫ������ڵ��ᵽ����*/
					{				
						if(ssd->dram->buffer->buffer_tail==buffer_node)
						{					
							buffer_node->LRU_link_pre->LRU_link_next=NULL;					
							ssd->dram->buffer->buffer_tail=buffer_node->LRU_link_pre;
						}			
						else						
						{			
							buffer_node->LRU_link_pre->LRU_link_next=buffer_node->LRU_link_next;						
							buffer_node->LRU_link_next->LRU_link_pre=buffer_node->LRU_link_pre;								
						}								
						buffer_node->LRU_link_next=ssd->dram->buffer->buffer_head;			
						ssd->dram->buffer->buffer_head->LRU_link_pre=buffer_node;
						buffer_node->LRU_link_pre=NULL;	
						ssd->dram->buffer->buffer_head=buffer_node;							
					}					
					buffer_node->stored=buffer_node->stored|add_flag;		
					buffer_node->dirty_clean=buffer_node->dirty_clean|add_flag;	
					ssd->dram->buffer->buffer_sector_count++;
				}			

			}
		}
	}

	return ssd;
}

/**************************************************************************************
*�����Ĺ�����Ѱ�һ�Ծ�죬ӦΪÿ��plane�ж�ֻ��һ����Ծ�飬ֻ�������Ծ���в��ܽ��в���
***************************************************************************************/
Status  find_active_block(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane)
{
	unsigned int active_block, write_block = -1, hot_block, hot2_block, cold_block, cold2_block;
	unsigned int free_page_num=0;
	unsigned int count=0;
	int i;
	unsigned int last = 0;
	
	hot_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot_block;
	hot2_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot2_block;
	cold_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold_block;
	cold2_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold2_block;
    if(ssd->parameter->interruptible == 3)
	{
		if(ssd->find_what == 1)
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
			write_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
		}
		else
		{
			write_block = -1;
			active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
		}
	}
	else
	{
		write_block = -1;
		active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	}
	
	//active_block=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
	free_page_num = 0;
	if(active_block!= -1)
	{
		free_page_num=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
	}
	//last_write_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
	if(active_block == -1)
	{
		while((free_page_num!=ssd->parameter->page_block)&&(count<ssd->parameter->block_plane))
		{
			active_block=(active_block+1)%ssd->parameter->block_plane;
			if(active_block == write_block)	continue;
			free_page_num=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
			count++;
		}
	}
	while((free_page_num==0)&&(count<ssd->parameter->block_plane))
	{
		active_block=(active_block+1)%ssd->parameter->block_plane;
		if(active_block == write_block | active_block == hot_block | active_block == hot2_block | active_block == cold_block | active_block == cold2_block)
			continue;	
		free_page_num=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
		count++;
	}

	//ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block=active_block;
	if(ssd->parameter->interruptible == 3)
	{
		if(ssd->find_what == 1)
		{
			if(ssd->hot_cold_flag == 3)
			{
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot_block=active_block;
			}
			else if(ssd->hot_cold_flag == 2)
			{
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].hot2_block=active_block;
			}
			else if(ssd->hot_cold_flag == 1)
			{
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold2_block=active_block;
			}
			else if(ssd->hot_cold_flag == 0)
			{
				ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].cold_block=active_block;
			}
			else
			{
				printf("error in hot and clod identify \n");
			}
		}
		else
		{
			ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block=active_block;
		}
	}
	else
	{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block=active_block;
	}
	//ssd->find_what = 0;
	if(count<ssd->parameter->block_plane)
	{
		return SUCCESS;
	}
	else
	{
		return FAILURE;
	}
}

/*************************************************
*��������Ĺ��ܾ���һ��ģ��һ��ʵʵ���ڵ�д����
*���Ǹ������page����ز������Լ�����ssd��ͳ�Ʋ���
**************************************************/
Status write_page(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,unsigned int active_block,unsigned int *ppn)
{
	int last_write_page=0;
	last_write_page=++(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page);	
	if(last_write_page>=(int)(ssd->parameter->page_block))
	{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page=0;
		printf("error! the last write page larger than 64!!\n");
		return ERROR;
	}
		
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--; 
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].counter++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[last_write_page].written_count++;
	ssd->write_flash_count++;    
	*ppn=find_ppn(ssd,channel,chip,die,plane,active_block,last_write_page);

	return SUCCESS;
}

/**********************************************
*��������Ĺ����Ǹ���lpn��size��state����������
**********************************************/
struct sub_request * creat_sub_request(struct ssd_info * ssd,unsigned int lpn,int size,unsigned int state,struct request * req,unsigned int operation)
{
	struct sub_request* sub=NULL,* sub_r=NULL, *sub_gc=NULL, *test = NULL;
	struct channel_info * p_ch=NULL;
	struct local * loc=NULL;
	unsigned int flag=0;
	int i,channel;
	int buf_flag = 0;

	sub = (struct sub_request*)malloc(sizeof(struct sub_request));                        /*����һ��������Ľṹ*/
	alloc_assert(sub,"sub_request");
	memset(sub,0, sizeof(struct sub_request));

	if(sub==NULL)
	{
		return NULL;
	}
	sub->location=NULL;
	sub->next_node=NULL;
	sub->next_subs=NULL;
	sub->update=NULL;
	
	if(req!=NULL)
	{
		sub->next_subs = req->subs;
		req->subs = sub;
	}
	
	/*************************************************************************************
	*�ڶ�����������£���һ��ǳ���Ҫ����ҪԤ���ж϶�������������Ƿ����������������ͬ�ģ�
	*�еĻ�����������Ͳ�����ִ���ˣ����µ�������ֱ�Ӹ�Ϊ���
	**************************************************************************************/
	if (operation == READ)
	{	
		loc = find_location(ssd,ssd->dram->map->map_entry[lpn].pn);
		sub->location=loc;
		sub->begin_time = ssd->current_time;
		sub->current_state = SR_WAIT;
		sub->current_time=MAX_INT64;
		sub->next_state = SR_R_C_A_TRANSFER;
		sub->next_state_predict_time=MAX_INT64;
		sub->lpn = lpn;
		sub->size=size;
		sub->buf_flag = buf_flag;                                                               /*��Ҫ�������������������С*/

		p_ch = &ssd->channel_head[loc->channel];	
		sub->ppn = ssd->dram->map->map_entry[lpn].pn;
		sub->operation = READ;
		sub->state=(ssd->dram->map->map_entry[lpn].state&0x7fffffff);
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
		//to check whether the LPN is in the buffer
		for(i = 0; i < ssd->parameter->gc_buffer_size; i++)
		{
			if(sub->lpn == ssd->gc_buffer[i])
			{
				flag = 1;
				ssd->gc_read_hit_count++;
				break;
			}
		}

		if (flag==0)
		{
			//to judge whether the gc sub queue has the same lpn operation
			for(channel = 0; channel < ssd->parameter->channel_number; channel++)
			{
				if(ssd->channel_head[channel].gc_sub_queue == NULL)
				{
					continue;
				}
				else
				{
					sub_gc = ssd->channel_head[channel].gc_sub_queue;
					while (sub_gc)
					{
						if(sub->lpn == sub_gc->lpn && sub_gc->operation == READ)
						{
							buf_flag = 1;
							sub->buf_flag = 1;
							ssd->gc_read_hit_count++;
						}
						sub_gc = sub_gc->next_node;
					}
				}
				
			}

			if (p_ch->subs_r_tail!=NULL)
			{
				p_ch->subs_r_tail->next_node=sub;
				p_ch->subs_r_tail=sub;
			} 
			else
			{
				p_ch->subs_r_head=sub;
				p_ch->subs_r_tail=sub;
			}
		}
		else
		{
			//ssd->gc_read_hit_count++;
			sub->current_state = SR_R_DATA_TRANSFER;
			sub->current_time=ssd->current_time;
			sub->next_state = SR_COMPLETE;
			sub->next_state_predict_time=ssd->current_time+1000;
			sub->complete_time=ssd->current_time+1000;
		}
	}
	/*************************************************************************************
	*д���������£�����Ҫ���õ�����allocate_location(ssd ,sub)������̬����Ͷ�̬������
	**************************************************************************************/
	else if(operation == WRITE)
	{                                
		sub->ppn=0;
		sub->operation = WRITE;
		sub->location=(struct local *)malloc(sizeof(struct local));
		alloc_assert(sub->location,"sub->location");
		memset(sub->location,0, sizeof(struct local));

		sub->current_state=SR_WAIT;
		sub->current_time=ssd->current_time;
		sub->lpn=lpn;
		sub->size=size;
		sub->state=state;
		sub->begin_time=ssd->current_time;

		flag = 0;

		//KXC_2:���gc_sub_queue���ж�ӦLPN�Ķ�д������Ӧ����gc_sub��ͬһ��LPN�Ķ�д������ɾ��
		delete_gc_write_when_hit(ssd,lpn);
		if (allocate_location(ssd ,sub)==ERROR)
		{
			free(sub->location);
			sub->location=NULL;
			free(sub);
			sub=NULL;
			return NULL;
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
	
	return sub;
}


/******************************************************
*�����Ĺ��������gc_sub_queue��ͬ��LPN�Ķ�д�����󣬽���ɾ��
*���IO��д�����󣬴�ʱgc���ý���ҳ��������Ҳ������ͨ��gcд��
*******************************************************/
struct ssd_info *delete_gc_write_when_hit(struct ssd_info *ssd,unsigned int lpn)
{
	int channel = 0;
	struct sub_request* sub=NULL,* p=NULL;
	struct channel_info * p_ch=NULL;
	struct local * loc=NULL;
	unsigned int flag=0;
	int i = 0;

	for(channel = 0; channel < ssd->parameter->channel_number; channel++)
	{
		if(ssd->channel_head[channel].gc_sub_queue == NULL)
		{
			continue;
		}
		else
		{
			sub = ssd->channel_head[channel].gc_sub_queue;
			while (sub)
			{
				if(sub->lpn == lpn)
				{
					ssd->gc_write_hit_count++;
					if(sub->operation == READ) ssd->gc_buf_count--;  //no need use gc buffer to store the page
					else if(sub->operation == WRITE)
					{
						for(i = 0; i < ssd->parameter->gc_buffer_size; i++)
						{
							if(lpn == ssd->gc_buffer[i])
							{
								ssd->gc_buffer[i] = -1;
								ssd->gc_buf_count--;
							}
						}
					}
					
					if(sub!=ssd->channel_head[channel].gc_sub_queue)                             /*if the request is completed, we delete it from gc queue */							
					{		
						//p = ssd->channel_head[channel].gc_sub_queue;
						if(sub == ssd->channel_head[channel].gc_sub_tail)
						{
							ssd->channel_head[channel].gc_sub_tail = p;
						}
						p->next_node = sub->next_node;						
					}			
					else					
					{	
						if (ssd->channel_head[channel].gc_sub_queue!=ssd->channel_head[channel].gc_sub_tail)
						{
							ssd->channel_head[channel].gc_sub_queue=sub->next_node;
						} 
						else
						{
							ssd->channel_head[channel].gc_sub_queue=NULL;
							ssd->channel_head[channel].gc_sub_tail=NULL;
						}							
					}
					//if(sub == NULL)
				}
				p = sub;
				sub = sub->next_node;
			}
		}
		
	}

	return ssd;
}
/******************************************************
*�����Ĺ������ڸ�����channel��chip��die����Ѱ�Ҷ�������
*����������ppnҪ����Ӧ��plane�ļĴ��������ppn���
*******************************************************/
struct sub_request * find_read_sub_request(struct ssd_info * ssd, unsigned int channel, unsigned int chip, unsigned int die)
{
	unsigned int plane=0;
	unsigned int address_ppn=0;
	struct sub_request *sub=NULL,* p=NULL;

	for(plane=0;plane<ssd->parameter->plane_die;plane++)
	{
		address_ppn=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].add_reg_ppn;
		if(address_ppn!=-1)
		{
			sub=ssd->channel_head[channel].subs_r_head;
			if(sub->ppn==address_ppn)
			{
				if(sub->next_node==NULL)
				{
					ssd->channel_head[channel].subs_r_head=NULL;
					ssd->channel_head[channel].subs_r_tail=NULL;
				}
				ssd->channel_head[channel].subs_r_head=sub->next_node;
			}
			while((sub->ppn!=address_ppn)&&(sub->next_node!=NULL))
			{
				if(sub->next_node->ppn==address_ppn)
				{
					p=sub->next_node;
					if(p->next_node==NULL)
					{
						sub->next_node=NULL;
						ssd->channel_head[channel].subs_r_tail=sub;
					}
					else
					{
						sub->next_node=p->next_node;
					}
					sub=p;
					break;
				}
				sub=sub->next_node;
			}
			if(sub->ppn==address_ppn)
			{
				sub->next_node=NULL;
				return sub;
			}
			else 
			{
				printf("Error! Can't find the sub request.");
			}
		}
	}
	return NULL;
}

/*******************************************************************************
*�����Ĺ�����Ѱ��д������
*���������1��Ҫ������ȫ��̬�������ssd->subs_w_head��������
*2��Ҫ�ǲ�����ȫ��̬������ô����ssd->channel_head[channel].subs_w_head�����ϲ���
********************************************************************************/
struct sub_request * find_write_sub_request(struct ssd_info * ssd, unsigned int channel)
{
	struct sub_request * sub=NULL,* p=NULL;
	if ((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==0))    /*����ȫ�Ķ�̬����*/
	{
		sub=ssd->subs_w_head;
		while(sub!=NULL)        							
		{
			if(sub->current_state==SR_WAIT)								
			{
				if (sub->update!=NULL)                                                      /*�������Ҫ��ǰ������ҳ*/
				{
					if ((sub->update->current_state==SR_COMPLETE)||((sub->update->next_state==SR_COMPLETE)&&(sub->update->next_state_predict_time<=ssd->current_time)))   //�����µ�ҳ�Ѿ�������
					{
						break;
					}
				} 
				else
				{
					break;
				}						
			}
			p=sub;
			sub=sub->next_node;							
		}

		if (sub==NULL)                                                                      /*���û���ҵ����Է�����������������forѭ��*/
		{
			return NULL;
		}

		if (sub!=ssd->subs_w_head)
		{
			if (sub!=ssd->subs_w_tail)
			{
				p->next_node=sub->next_node;
			}
			else
			{
				ssd->subs_w_tail=p;
				ssd->subs_w_tail->next_node=NULL;
			}
		} 
		else
		{
			if (sub->next_node!=NULL)
			{
				ssd->subs_w_head=sub->next_node;
			} 
			else
			{
				ssd->subs_w_head=NULL;
				ssd->subs_w_tail=NULL;
			}
		}
		sub->next_node=NULL;
		if (ssd->channel_head[channel].subs_w_tail!=NULL)
		{
			ssd->channel_head[channel].subs_w_tail->next_node=sub;
			ssd->channel_head[channel].subs_w_tail=sub;
		} 
		else
		{
			ssd->channel_head[channel].subs_w_tail=sub;
			ssd->channel_head[channel].subs_w_head=sub;
		}
	}
	/**********************************************************
	*����ȫ��̬���䷽ʽ��������ʽ�������Ѿ����䵽�ض���channel��
	*��ֻ��Ҫ��channel���ҳ�׼�������������
	***********************************************************/
	else            
	{
		sub=ssd->channel_head[channel].subs_w_head;
		while(sub!=NULL)        						
		{
			if(sub->current_state==SR_WAIT)								
			{
				if (sub->update!=NULL)    
				{
					if ((sub->update->current_state==SR_COMPLETE)||((sub->update->next_state==SR_COMPLETE)&&(sub->update->next_state_predict_time<=ssd->current_time)))   //�����µ�ҳ�Ѿ�������
					{
						break;
					}
				} 
				else
				{
					break;
				}						
			}
			p=sub;
			sub=sub->next_node;							
		}

		if (sub==NULL)
		{
			return NULL;
		}
	}
	
	return sub;
}

/*********************************************************************************************
*ר��Ϊgc_sub_request �е����������
**********************************************************************************************/
Status services_2_gc_sub(struct ssd_info * ssd, int channel,unsigned int * channel_busy_flag, unsigned int * change_current_time_flag)
{
	unsigned int i=0;
	struct sub_request * sub=NULL, * p=NULL;
	sub = ssd->channel_head[channel].gc_sub_queue;
	struct  gc_operation *gc_node;
	int64_t next_state_time = 0;
	double random_num=rand()/(RAND_MAX+1.0);
	//double erase_p=(double)ssd->interval[3]/(ssd->interval[3]+ssd->interval[2]);
	double erase_p=(double)((ssd->recent[0]+ssd->recent[1]+ssd->recent[2])+ssd->interval[3]/(ssd->interval[3]+ssd->interval[2]));
	
	if(sub->operation == READ)
	{
		/*if there are read requests in queue, send one of them to target chip*/
		//next_state_time = ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tR;
		/*if(ssd->parameter->time_control == 1 && ssd->next_request_time < next_state_time)
		{
			return;
		}*/	


		if(sub->current_state==SR_WAIT && sub->operation == READ)									
		{	                                                                       
			if((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state==CHIP_IDLE)&&
				(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time<=ssd->current_time)))												
			{	

				go_one_step(ssd, sub,NULL, SR_R_GC_SUB,NORMAL);
						
				*change_current_time_flag=0;
				*channel_busy_flag=1;                                              /*�Ѿ�ռ����������ڵ����ߣ�����ִ��die�����ݵĻش�*/
													
			}	
			else
			{
																					/*��Ϊdie��busy���µ�����*/
			}
		}						
	}
	else if(sub->operation == 11)     //KXC_2: ERASE
	{
		//next_state_time = ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;
		
		/*if(ssd->next_request_time >= ssd->current_time + ssd->parameter->time_characteristics.tBERS)		
		{
			ssd->recent[ssd->recent_num] = 1;
			ssd->recent_num = (ssd->recent_num + 1) % 3;
		}*/
		if((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state==CHIP_IDLE)&&
				(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time<=ssd->current_time)))
		{	
			/*if(ssd->parameter->time_control == 1 && ssd->next_request_time < next_state_time)
			{
				return;
			}*/
			
			if(ssd->parameter->time_control == 1 && random_num > erase_p)
			{
				return;
			}
			ssd->predict_count++;
			if(ssd->next_request_time >= ssd->current_time + ssd->parameter->time_characteristics.tBERS)		
			{
				ssd->predict_yes++;
			}

			erase_operation(ssd,channel,sub->location->chip, sub->location->die,sub->location->plane,sub->location->block);	                                              /*ִ����move_page�����󣬾�����ִ��block�Ĳ�������*/

			ssd->channel_head[channel].current_state=CHANNEL_GC;									
			ssd->channel_head[channel].current_time=ssd->current_time;										
			ssd->channel_head[channel].next_state=CHANNEL_IDLE;	
			ssd->channel_head[channel].chip_head[sub->location->chip].current_state=CHIP_ERASE_BUSY;								
			ssd->channel_head[channel].chip_head[sub->location->chip].current_time=ssd->current_time;						
			ssd->channel_head[channel].chip_head[sub->location->chip].next_state=CHIP_IDLE;			
		

			ssd->channel_head[channel].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC;;					
			ssd->channel_head[channel].chip_head[sub->location->chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tBERS;


			//KXC_2: delete erase sub-request while read and write requests of gc if deleted in  services_2_r_cmd_trans_and_complete  and  delete_from_channel
			ssd->channel_head[channel].gc_sub_queue = sub->next_node;
			if(sub == ssd->channel_head[channel].gc_sub_tail) ssd->channel_head[channel].gc_sub_tail = NULL;
			free(sub);
			sub = NULL;
			
			
			free(ssd->channel_head[channel].gc_soft);
			ssd->channel_head[channel].gc_soft = NULL;
			
		}

	}
	
	
}

/*********************************************************************************************
*ר��Ϊ�����������ĺ���
*1��ֻ�е���������ĵ�ǰ״̬��SR_R_C_A_TRANSFER
*2����������ĵ�ǰ״̬��SR_COMPLETE������һ״̬��SR_COMPLETE������һ״̬�����ʱ��ȵ�ǰʱ��С
**********************************************************************************************/
Status services_2_r_cmd_trans_and_complete(struct ssd_info * ssd)
{
	unsigned int i=0, j = 0, flag = 0;
	struct sub_request * sub=NULL, * p=NULL;
	for(i=0;i<ssd->parameter->channel_number;i++)                                       /*���ѭ��������Ҫchannel��ʱ��(�������Ѿ�����chip��chip��ready��Ϊbusy)�������������ʱ�������channel�Ķ�����ȡ��*/
	{
		sub=ssd->channel_head[i].subs_r_head;

		while(sub!=NULL)
		{
			if(sub->current_state==SR_R_C_A_TRANSFER)                                  /*���������ϣ�����Ӧ��die��Ϊbusy��ͬʱ�޸�sub��״̬; �������ר�Ŵ���������ɵ�ǰ״̬Ϊ�������Ϊdie��ʼbusy��die��ʼbusy����ҪchannelΪ�գ����Ե����г�*/
			{
				if(sub->next_state_predict_time<=ssd->current_time)
				{
					go_one_step(ssd, sub,NULL, SR_R_READ,NORMAL);                      /*״̬���䴦����*/

				}
			}
			else if((sub->current_state==SR_COMPLETE)||((sub->next_state==SR_COMPLETE)&&(sub->next_state_predict_time<=ssd->current_time)))					
			{			
				if(sub->buf_flag == 1)
				{
					for(j = 0; j < ssd->parameter->gc_buffer_size; j++)
					{
						if(ssd->gc_buffer[j] == -1)
						{
							ssd->gc_buffer[j] = sub->lpn;
							//ssd->gc_read_hit_count++;
							//ssd->gc_buf_count++;
							break;
						}
					}
				}
				
				if(sub!=ssd->channel_head[i].subs_r_head)                             /*if the request is completed, we delete it from read queue */							
				{		
					p->next_node=sub->next_node;						
				}			
				else					
				{	
					if (ssd->channel_head[i].subs_r_head!=ssd->channel_head[i].subs_r_tail)
					{
						ssd->channel_head[i].subs_r_head=sub->next_node;
					} 
					else
					{
						ssd->channel_head[i].subs_r_head=NULL;
						ssd->channel_head[i].subs_r_tail=NULL;
					}							
				}			
			}
			p=sub;
			sub=sub->next_node;
		}
	}

	//KXC_2: to finish the gc read subrequests
	for(i=0;i<ssd->parameter->channel_number;i++)                                       /*���ѭ��������Ҫchannel��ʱ��(�������Ѿ�����chip��chip��ready��Ϊbusy)�������������ʱ�������channel�Ķ�����ȡ��*/
	{
		sub=ssd->channel_head[i].gc_sub_queue;
		p = NULL;
		while(sub!=NULL)
		{

			if(sub->operation == READ)
			{
				if((sub->current_state==SR_COMPLETE)||((sub->next_state==SR_COMPLETE)&&(sub->next_state_predict_time<=ssd->current_time)))					
				{			
					//KXC_2:put the page into gc butter
					for(j = 0; j < ssd->parameter->gc_buffer_size; j++)
					{
						if(ssd->gc_buffer[j] == -1)
						{
							ssd->gc_buffer[j] = sub->lpn;
							ssd->soft_gc_read_count++;
							flag = 1;
							break;
						}
					}

					if(flag == 0)
					{
						printf("Error in gc_valid page put in gc buffer \n");
					}
					else
					{
						//ssd->gc_buf_count++;
						flag = 0;
					}					
					
					if(sub!=ssd->channel_head[i].gc_sub_queue)                             /*if the request is completed, we delete it from gc queue */							
					{		
						//p = ssd->channel_head[i].gc_sub_queue;
						p->next_node = sub->next_node;
						if(sub == ssd->channel_head[i].gc_sub_tail)	
						{
							ssd->channel_head[i].gc_sub_tail = p;
						}					
					}			
					else					
					{	
						if (ssd->channel_head[i].gc_sub_queue!=ssd->channel_head[i].gc_sub_tail)
						{
							ssd->channel_head[i].gc_sub_queue=sub->next_node;
						} 
						else
						{
							ssd->channel_head[i].gc_sub_queue=NULL;
							ssd->channel_head[i].gc_sub_tail=NULL;
						}							
					}
							
				}
			}
			p=sub;
			sub=sub->next_node;
		}
	}
	
	return SUCCESS;
}

/**************************************************************************
*�������Ҳ��ֻ����������󣬴���chip��ǰ״̬��CHIP_WAIT��
*������һ��״̬��CHIP_DATA_TRANSFER������һ״̬��Ԥ��ʱ��С�ڵ�ǰʱ���chip
***************************************************************************/
Status services_2_r_data_trans(struct ssd_info * ssd,unsigned int channel,unsigned int * channel_busy_flag, unsigned int * change_current_time_flag)
{
	int chip=0;
	unsigned int die=0,plane=0,address_ppn=0,die1=0;
	struct sub_request * sub=NULL, * p=NULL,*sub1=NULL;
	struct sub_request * sub_twoplane_one=NULL, * sub_twoplane_two=NULL;
	struct sub_request * sub_interleave_one=NULL, * sub_interleave_two=NULL;
	for(chip=0;chip<ssd->channel_head[channel].chip;chip++)           			    
	{				       		      
			if((ssd->channel_head[channel].chip_head[chip].current_state==CHIP_WAIT)||((ssd->channel_head[channel].chip_head[chip].next_state==CHIP_DATA_TRANSFER)&&
				(ssd->channel_head[channel].chip_head[chip].next_state_predict_time<=ssd->current_time)))					       					
			{
				for(die=0;die<ssd->parameter->die_chip;die++)
				{
					sub=find_read_sub_request(ssd,channel,chip,die);                   /*��channel,chip,die���ҵ��������������Ѿ������Ĵ����еĶ�����*/
					if(sub!=NULL)
					{
						break;
					}
				}

				if(sub==NULL)
				{
					continue;
				}
				
				/**************************************************************************************
				*���ssd֧�ָ߼������û���ǿ���һ����֧��AD_TWOPLANE_READ��AD_INTERLEAVE�Ķ�������
				*1���п��ܲ�����two plane����������������£���ͬһ��die�ϵ�����plane���������δ���
				*2���п��ܲ�����interleave����������������£�����ͬdie�ϵ�����plane���������δ���
				***************************************************************************************/
				if(((ssd->parameter->advanced_commands&AD_TWOPLANE_READ)==AD_TWOPLANE_READ)||((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))
				{
					if ((ssd->parameter->advanced_commands&AD_TWOPLANE_READ)==AD_TWOPLANE_READ)     /*�п��ܲ�����two plane����������������£���ͬһ��die�ϵ�����plane���������δ���*/
					{
						sub_twoplane_one=sub;
						sub_twoplane_two=NULL;                                                      
						                                                                            /*Ϊ�˱�֤�ҵ���sub_twoplane_two��sub_twoplane_one��ͬ����add_reg_ppn=-1*/
						ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[sub->location->plane].add_reg_ppn=-1;
						sub_twoplane_two=find_read_sub_request(ssd,channel,chip,die);               /*����ͬ��channel,chip,die��Ѱ������һ����������*/
						
						/******************************************************
						*����ҵ�����ô��ִ��TWO_PLANE��״̬ת������go_one_step
						*���û�ҵ���ô��ִ����ͨ�����״̬ת������go_one_step
						******************************************************/
						if (sub_twoplane_two==NULL)
						{
							go_one_step(ssd, sub_twoplane_one,NULL, SR_R_DATA_TRANSFER,NORMAL);   
							*change_current_time_flag=0;   
							*channel_busy_flag=1;

						}
						else
						{
							go_one_step(ssd, sub_twoplane_one,sub_twoplane_two, SR_R_DATA_TRANSFER,TWO_PLANE);
							*change_current_time_flag=0;  
							*channel_busy_flag=1;

						}
					} 
					else if ((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)      /*�п��ܲ�����interleave����������������£�����ͬdie�ϵ�����plane���������δ���*/
					{
						sub_interleave_one=sub;
						sub_interleave_two=NULL;
						ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[sub->location->plane].add_reg_ppn=-1;
						
						for(die1=0;die1<ssd->parameter->die_chip;die1++)
						{	
							if(die1!=die)
							{
								sub_interleave_two=find_read_sub_request(ssd,channel,chip,die1);    /*����ͬ��channel��chhip��ͬ��die����������һ����������*/
								if(sub_interleave_two!=NULL)
								{
									break;
								}
							}
						}	
						if (sub_interleave_two==NULL)
						{
							go_one_step(ssd, sub_interleave_one,NULL, SR_R_DATA_TRANSFER,NORMAL);

							*change_current_time_flag=0;  
							*channel_busy_flag=1;

						}
						else
						{
							go_one_step(ssd, sub_twoplane_one,sub_interleave_two, SR_R_DATA_TRANSFER,INTERLEAVE);
												
							*change_current_time_flag=0;   
							*channel_busy_flag=1;
							
						}
					}
				}
				else                                                                                 /*���ssd��֧�ָ߼�������ô��ִ��һ��һ����ִ�ж�������*/
				{
											
					go_one_step(ssd, sub,NULL, SR_R_DATA_TRANSFER,NORMAL);
					*change_current_time_flag=0;  
					*channel_busy_flag=1;
					
				}
				break;
			}		
			
		if(*channel_busy_flag==1)
		{
			break;
		}
	}		
	return SUCCESS;
}


/******************************************************
*�������Ҳ��ֻ����������󣬲��Ҵ��ڵȴ�״̬�Ķ�������
KXC���о������߼��д��������TWOPLANE_READ��ִ�У�����Ӧ���ж��Ƿ���INTERLEAVING��������ִ��NORMAL
*******************************************************/
int services_2_r_wait(struct ssd_info * ssd,unsigned int channel,unsigned int * channel_busy_flag, unsigned int * change_current_time_flag)
{
	unsigned int plane=0,address_ppn=0;
	struct sub_request * sub=NULL, * p=NULL;
	struct sub_request * sub_twoplane_one=NULL, * sub_twoplane_two=NULL;
	struct sub_request * sub_interleave_one=NULL, * sub_interleave_two=NULL;
	
	
	sub=ssd->channel_head[channel].subs_r_head;

	
	if ((ssd->parameter->advanced_commands&AD_TWOPLANE_READ)==AD_TWOPLANE_READ)         /*to find whether there are two sub request can be served by two plane operation*/
	{
		sub_twoplane_one=NULL;
		sub_twoplane_two=NULL;                                                         
		                                                                                /*Ѱ����ִ��two_plane��������������*/
		find_interleave_twoplane_sub_request(ssd,channel,sub_twoplane_one,sub_twoplane_two,TWO_PLANE);

		if (sub_twoplane_two!=NULL)                                                     /*����ִ��two plane read ����*/
		{
			go_one_step(ssd, sub_twoplane_one,sub_twoplane_two, SR_R_C_A_TRANSFER,TWO_PLANE);
						
			*change_current_time_flag=0;
			*channel_busy_flag=1;                                                       /*�Ѿ�ռ����������ڵ����ߣ�����ִ��die�����ݵĻش�*/
		} 
		else if((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)       /*û����������������page��������û��interleave read����ʱ��ֻ��ִ�е���page�Ķ�*/
		{
			while(sub!=NULL)                                                            /*if there are read requests in queue, send one of them to target die*/			
			{		
				if(sub->current_state==SR_WAIT)									
				{	                                                                    /*ע���¸�����ж�������services_2_r_data_trans���ж������Ĳ�ͬ
																						*/
					if((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state==CHIP_IDLE)&&
						(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time<=ssd->current_time)))												
					{	
						go_one_step(ssd, sub,NULL, SR_R_C_A_TRANSFER,NORMAL);
									
						*change_current_time_flag=0;
						*channel_busy_flag=1;                                           /*�Ѿ�ռ����������ڵ����ߣ�����ִ��die�����ݵĻش�*/
						break;										
					}	
					else
					{
						                                                                /*��Ϊdie��busy���µ�����*/
					}
				}						
				sub=sub->next_node;								
			}
		}
	} 
	if ((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)               /*to find whether there are two sub request can be served by INTERLEAVE operation*/
	{
		sub_interleave_one=NULL;
		sub_interleave_two=NULL;
		find_interleave_twoplane_sub_request(ssd,channel,sub_interleave_one,sub_interleave_two,INTERLEAVE);
		
		if (sub_interleave_two!=NULL)                                                  /*����ִ��interleave read ����*/
		{

			go_one_step(ssd, sub_interleave_one,sub_interleave_two, SR_R_C_A_TRANSFER,INTERLEAVE);
						
			*change_current_time_flag=0;
			*channel_busy_flag=1;                                                      /*�Ѿ�ռ����������ڵ����ߣ�����ִ��die�����ݵĻش�*/
		} 
		else                                                                           /*û����������������page��ֻ��ִ�е���page�Ķ�*/
		{
			while(sub!=NULL)                                                           /*if there are read requests in queue, send one of them to target die*/			
			{		
				if(sub->current_state==SR_WAIT)									
				{	
					if((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state==CHIP_IDLE)&&
						(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time<=ssd->current_time)))												
					{	

						go_one_step(ssd, sub,NULL, SR_R_C_A_TRANSFER,NORMAL);
									
						*change_current_time_flag=0;
						*channel_busy_flag=1;                                          /*�Ѿ�ռ����������ڵ����ߣ�����ִ��die�����ݵĻش�*/
						break;										
					}	
					else
					{
						                                                               /*��Ϊdie��busy���µ�����*/
					}
				}						
				sub=sub->next_node;								
			}
		}
	}

	/*******************************
	*ssd����ִ��ִ�и߼�����������
	*******************************/
	if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE_READ)!=AD_TWOPLANE_READ))
	{
		while(sub!=NULL)                                                               /*if there are read requests in queue, send one of them to target chip*/			
		{		
			if(sub->current_state==SR_WAIT)									
			{	                                                                       
				
				if((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state==CHIP_IDLE)&&
					(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time<=ssd->current_time)))												
				{	

					go_one_step(ssd, sub,NULL, SR_R_C_A_TRANSFER,NORMAL);
							
					*change_current_time_flag=0;
					*channel_busy_flag=1;                                              /*�Ѿ�ռ����������ڵ����ߣ�����ִ��die�����ݵĻش�*/
					break;										
				}	
				else
				{
					                                                                   /*��Ϊdie��busy���µ�����*/
				}
			}						
			sub=sub->next_node;								
		}
	}

	return SUCCESS;
}

/*********************************************************************
*��һ��д�����������Ҫ�����������ɾ���������������ִ��������ܡ�
**********************************************************************/
int delete_w_sub_request(struct ssd_info * ssd, unsigned int channel, struct sub_request * sub )
{
	struct sub_request * p=NULL;
	if (sub==ssd->channel_head[channel].subs_w_head)                                   /*������������channel������ɾ��*/
	{
		if (ssd->channel_head[channel].subs_w_head!=ssd->channel_head[channel].subs_w_tail)
		{
			ssd->channel_head[channel].subs_w_head=sub->next_node;
		} 
		else
		{
			ssd->channel_head[channel].subs_w_head=NULL;
			ssd->channel_head[channel].subs_w_tail=NULL;
		}
	}
	else
	{
		p=ssd->channel_head[channel].subs_w_head;
		while(p->next_node !=sub)
		{
			p=p->next_node;
		}

		if (sub->next_node!=NULL)
		{
			p->next_node=sub->next_node;
		} 
		else
		{
			p->next_node=NULL;
			ssd->channel_head[channel].subs_w_tail=p;
		}
	}
	
	return SUCCESS;	
}

/*
*�����Ĺ��ܾ���ִ��copyback����Ĺ��ܣ�
*/
Status copy_back(struct ssd_info * ssd, unsigned int channel, unsigned int chip, unsigned int die,struct sub_request * sub)
{
	int old_ppn=-1, new_ppn=-1;
	long long time=0;
	if (ssd->parameter->greed_CB_ad==1)                                               /*����̰��ʹ��copyback�߼�����*/
	{
		old_ppn=-1;
		if (ssd->dram->map->map_entry[sub->lpn].state!=0)                             /*˵������߼�ҳ֮ǰ��д������Ҫʹ��copyback+random input�������ֱ��д��ȥ����*/
		{
			if ((sub->state&ssd->dram->map->map_entry[sub->lpn].state)==ssd->dram->map->map_entry[sub->lpn].state)       
			{
				sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;	
			} 
			else
			{
				sub->next_state_predict_time=ssd->current_time+19*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				ssd->copy_back_count++;
				ssd->read_count++;
				ssd->update_read_count++;
				old_ppn=ssd->dram->map->map_entry[sub->lpn].pn;                       /*��¼ԭ��������ҳ��������copybackʱ���ж��Ƿ�����ͬΪ���ַ����ż��ַ*/
			}															
		} 
		else
		{
			sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		}
		sub->complete_time=sub->next_state_predict_time;		
		time=sub->complete_time;

		get_ppn(ssd,sub->location->channel,sub->location->chip,sub->location->die,sub->location->plane,sub);

		if (old_ppn!=-1)                                                              /*������copyback��������Ҫ�ж��Ƿ���������ż��ַ������*/
		{
			new_ppn=ssd->dram->map->map_entry[sub->lpn].pn;
			while (old_ppn%2!=new_ppn%2)                                              /*û��������ż��ַ���ƣ���Ҫ��������һҳ*/
			{
				get_ppn(ssd,sub->location->channel,sub->location->chip,sub->location->die,sub->location->plane,sub);
				ssd->program_count--;
				ssd->write_flash_count--;
				ssd->waste_page_count++;
				new_ppn=ssd->dram->map->map_entry[sub->lpn].pn;
			}
		}
	} 
	else                                                                              /*����̰����ʹ��copyback�߼�����*/
	{
		if (ssd->dram->map->map_entry[sub->lpn].state!=0)
		{
			if ((sub->state&ssd->dram->map->map_entry[sub->lpn].state)==ssd->dram->map->map_entry[sub->lpn].state)        
			{
				sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				get_ppn(ssd,sub->location->channel,sub->location->chip,sub->location->die,sub->location->plane,sub);
			} 
			else
			{
				old_ppn=ssd->dram->map->map_entry[sub->lpn].pn;                       /*��¼ԭ��������ҳ��������copybackʱ���ж��Ƿ�����ͬΪ���ַ����ż��ַ*/
				get_ppn(ssd,sub->location->channel,sub->location->chip,sub->location->die,sub->location->plane,sub);
				new_ppn=ssd->dram->map->map_entry[sub->lpn].pn;
				if (old_ppn%2==new_ppn%2)
				{
					ssd->copy_back_count++;
					sub->next_state_predict_time=ssd->current_time+19*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				} 
				else
				{
					sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(size(ssd->dram->map->map_entry[sub->lpn].state))*ssd->parameter->time_characteristics.tRC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				}
				ssd->read_count++;
				ssd->update_read_count++;
			}
		} 
		else
		{
			sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
			get_ppn(ssd,sub->location->channel,sub->location->chip,sub->location->die,sub->location->plane,sub);
		}
		sub->complete_time=sub->next_state_predict_time;		
		time=sub->complete_time;
	}
    
	/****************************************************************
	*ִ��copyback�߼�����ʱ����Ҫ�޸�channel��chip��״̬���Լ�ʱ���
	*****************************************************************/
	ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
	ssd->channel_head[channel].next_state_predict_time=time;

	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
	ssd->channel_head[channel].chip_head[chip].next_state_predict_time=time+ssd->parameter->time_characteristics.tPROG;
	
	return SUCCESS;
}

/*****************
*��̬д������ʵ��
******************/
Status static_write(struct ssd_info * ssd, unsigned int channel,unsigned int chip, unsigned int die,struct sub_request * sub)
{
	long long time=0;
	if (ssd->dram->map->map_entry[sub->lpn].state!=0)                                    /*˵������߼�ҳ֮ǰ��д������Ҫʹ���ȶ���������д��ȥ������ֱ��д��ȥ����*/
	{
		if ((sub->state&ssd->dram->map->map_entry[sub->lpn].state)==ssd->dram->map->map_entry[sub->lpn].state)   /*���Ը��ǣ������е������ྻ��һ�µ�*/
		{
			sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		} 
		else
		{
			sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(size((ssd->dram->map->map_entry[sub->lpn].state^sub->state)))*ssd->parameter->time_characteristics.tRC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
			ssd->read_count++;
			ssd->update_read_count++;   //��������ϲ�ʱ�����Ķ�����
		}
	} 
	else
	{
		sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
	}
	sub->complete_time=sub->next_state_predict_time;		
	time=sub->complete_time;

	get_ppn(ssd,sub->location->channel,sub->location->chip,sub->location->die,sub->location->plane,sub);
    //get_ppn����������������д������ɣ���Ҫ����activeblock�͸���token��������Ҫ�ж��Ƿ���Ҫ�������յȵ�
    /****************************************************************
	*ִ��copyback�߼�����ʱ����Ҫ�޸�channel��chip��״̬���Լ�ʱ���
	*****************************************************************/
	ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
	ssd->channel_head[channel].current_time=ssd->current_time;										
	ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
	ssd->channel_head[channel].next_state_predict_time=time;

	ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
	ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
	ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
	ssd->channel_head[channel].chip_head[chip].next_state_predict_time=time+ssd->parameter->time_characteristics.tPROG;
	
	return SUCCESS;
}

/********************
д������Ĵ�����
*********************/
Status services_2_write(struct ssd_info * ssd,unsigned int channel,unsigned int * channel_busy_flag, unsigned int * change_current_time_flag)
{
	int j=0,chip=0,m=0;
	unsigned int k=0, random_num = 0;
	unsigned int  old_ppn=0,new_ppn=0;
	unsigned int chip_token=0,die_token=0,plane_token=0,address_ppn=0;
	unsigned int  die=0,plane=0;
	long long time=0;
	struct sub_request * sub=NULL, * p=NULL, * sub1=NULL, *sub_gc = NULL;
	struct sub_request * sub_twoplane_one=NULL, * sub_twoplane_two=NULL;
	struct sub_request * sub_interleave_one=NULL, * sub_interleave_two=NULL;
	unsigned int lpn;
	int w_flag = 0;
	int flag , i;
	int channel_gc = 0, channel_gc_num = 0, find_flag = 0;
    int64_t next_state_time = 0;
	/************************************************************************************************************************
	*д��������������ط�һ����channel_head[channel].subs_w_head������һ����ssd->subs_w_head������Ҫ��֤������һ�����в�Ϊ��
	*ͬʱ������Ĵ�����Ϊ��̬����;�̬���䡣
	*************************************************************************************************************************/
	if((ssd->channel_head[channel].subs_w_head!=NULL)||(ssd->subs_w_head!=NULL))      
	{
		if (ssd->parameter->allocation_scheme==0)                                       /*��̬����*/
		{
			for(j=0;j<ssd->channel_head[channel].chip;j++)					
			{		
				/*if((ssd->channel_head[channel].subs_w_head==NULL)&&(ssd->subs_w_head==NULL)) 
				{
					break;
				}*/
				
				chip_token=ssd->channel_head[channel].token;                            /*����*/
				if (*channel_busy_flag==0)
				{
					if((ssd->channel_head[channel].chip_head[chip_token].current_state==CHIP_IDLE)||((ssd->channel_head[channel].chip_head[chip_token].next_state==CHIP_IDLE)&&(ssd->channel_head[channel].chip_head[chip_token].next_state_predict_time<=ssd->current_time)))				
					{
							if((ssd->channel_head[channel].subs_w_head==NULL)&&(ssd->subs_w_head==NULL)) 
							{
								break;
							}
							die_token=ssd->channel_head[channel].chip_head[chip_token].token;	
							if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)!=AD_TWOPLANE))       //can't use advanced commands
							{
								sub=find_write_sub_request(ssd,channel);
								if(sub==NULL)
								{
									break;
								}

								if(sub->current_state==SR_WAIT)
								{
									plane_token=ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token;

									get_ppn(ssd,channel,chip_token,die_token,plane_token,sub);

									ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token=(ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token+1)%ssd->parameter->plane_die;

									*change_current_time_flag=0;

									if(ssd->parameter->ad_priority2==0)
									{
										ssd->real_time_subreq--;
									}

									go_one_step(ssd,sub,NULL,SR_W_TRANSFER,NORMAL);       /*ִ����ͨ��״̬��ת�䡣*/
									delete_w_sub_request(ssd,channel,sub);                /*ɾ����������д������*/
						
									*channel_busy_flag=1;
									/**************************************************************************
									*����forѭ��ǰ���޸�����
									*�����token�ı仯��ȫȡ���������channel chip die plane��д�Ƿ�ɹ� 
									*�ɹ��˾�break û�ɹ�token��Ҫ�仯ֱ���ҵ���д�ɹ���channel chip die plane
									***************************************************************************/
									ssd->channel_head[channel].chip_head[chip_token].token=(ssd->channel_head[channel].chip_head[chip_token].token+1)%ssd->parameter->die_chip;
									ssd->channel_head[channel].token=(ssd->channel_head[channel].token+1)%ssd->parameter->chip_channel[channel];
									
									break;
								}
							} 
							else                                                          /*use advanced commands*/
							{
								if (dynamic_advanced_process(ssd,channel,chip_token)==NULL)
								{
									*channel_busy_flag=0;
								}
								else
								{
									*channel_busy_flag=1;                                 /*ִ����һ�����󣬴��������ݣ�ռ�������ߣ���Ҫ��������һ��channel*/
                                    ssd->channel_head[channel].chip_head[chip_token].token=(ssd->channel_head[channel].chip_head[chip_token].token+1)%ssd->parameter->die_chip;
                                    ssd->channel_head[channel].token=(ssd->channel_head[channel].token+1)%ssd->parameter->chip_channel[channel];
									break;
								}
							}	
								
						ssd->channel_head[channel].chip_head[chip_token].token=(ssd->channel_head[channel].chip_head[chip_token].token+1)%ssd->parameter->die_chip;
					}
				}
								
				ssd->channel_head[channel].token=(ssd->channel_head[channel].token+1)%ssd->parameter->chip_channel[channel];
			}
		} 
		else if(ssd->parameter->allocation_scheme==1)                                     /*��̬����*/
		{
			for(chip=0;chip<ssd->channel_head[channel].chip;chip++)					
			{	
				if((ssd->channel_head[channel].chip_head[chip].current_state==CHIP_IDLE)||((ssd->channel_head[channel].chip_head[chip].next_state==CHIP_IDLE)&&(ssd->channel_head[channel].chip_head[chip].next_state_predict_time<=ssd->current_time)))				
				{		
					if(ssd->channel_head[channel].subs_w_head==NULL)
					{
						break;
					}
					if (*channel_busy_flag==0)
					{
							                                                            
							if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)!=AD_TWOPLANE))     /*��ִ�и߼�����*/
							{
								for(die=0;die<ssd->channel_head[channel].chip_head[chip].die_num;die++)				
								{	
									if(ssd->channel_head[channel].subs_w_head==NULL)
									{
										break;
									}
									sub=ssd->channel_head[channel].subs_w_head;
									while (sub!=NULL)
									{
										if ((sub->current_state==SR_WAIT)&&(sub->location->channel==channel)&&(sub->location->chip==chip)&&(sub->location->die==die))      /*����������ǵ�ǰdie������*/
										{
											if (sub->update!=NULL)                                                      /*�������Ҫ��ǰ������ҳ*/
											{
												if ((sub->update->current_state==SR_COMPLETE)||((sub->update->next_state==SR_COMPLETE)&&(sub->update->next_state_predict_time<=ssd->current_time)))   //�����µ�ҳ�Ѿ�������
												{
													break;
												}
											}
											else
											{
												break;
											}	 
										}
										sub=sub->next_node;
									}
									if (sub==NULL)
									{
										continue;
									}

									if(sub->current_state==SR_WAIT)
									{
										sub->current_time=ssd->current_time;
										sub->current_state=SR_W_TRANSFER;
										sub->next_state=SR_COMPLETE;

										if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
										{
											copy_back(ssd, channel,chip, die,sub);      /*�������ִ��copyback�߼������ô���ú���copy_back(ssd, channel,chip, die,sub)����д������*/
											*change_current_time_flag=0;
										} 
										else
										{
											static_write(ssd, channel,chip, die,sub);   /*����ִ��copyback�߼������ô����static_write(ssd, channel,chip, die,sub)����������д������*/ 
											*change_current_time_flag=0;
										}
										
										delete_w_sub_request(ssd,channel,sub);
										*channel_busy_flag=1;
										break;
									}
								}
							} 
							else                                                        /*���ܴ���߼�����*/
							{
								if (dynamic_advanced_process(ssd,channel,chip)==NULL)
								{
									*channel_busy_flag=0;
								}
								else
								{
									*channel_busy_flag=1;                               /*ִ����һ�����󣬴��������ݣ�ռ�������ߣ���Ҫ��������һ��channel*/
									break;
								}
							}	
						
					}
				}		
			}
		}			
	}
	else      //no sub-write-request so to process gc_write_sub
	{
		sub = NULL;
		for(j=0;j<ssd->channel_head[channel].chip;j++)					
		{		
			chip_token=ssd->channel_head[channel].token;                            /*����*/
			if (*channel_busy_flag==0)
			{
				next_state_time = ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
				/*if(ssd->parameter->time_control == 1 && ssd->next_request_time < next_state_time)
				{
					return;
				}*/
				if((ssd->channel_head[channel].chip_head[chip_token].current_state==CHIP_IDLE)||((ssd->channel_head[channel].chip_head[chip_token].next_state==CHIP_IDLE)&&(ssd->channel_head[channel].chip_head[chip_token].next_state_predict_time<=ssd->current_time)))				
				{
					random_num=ssd->program_count%ssd->parameter->channel_number; 
					for(channel_gc = 0; channel_gc < ssd->parameter->channel_number; channel_gc++)
					{	
						channel_gc_num=(random_num+channel_gc)%ssd->parameter->channel_number;

						if(ssd->channel_head[channel_gc_num].gc_sub_queue==NULL)
						{
							continue;
						}


						sub=ssd->channel_head[channel_gc_num].gc_sub_queue;
						while (sub != NULL)
						{
							if(sub->operation == WRITE)
							{
								lpn = sub->lpn;
								for(m = 0; m < ssd->parameter->gc_buffer_size; m++)
								{
									if(lpn == ssd->gc_buffer[m])
									{
										w_flag = 1;
										break;
									}
								}

								if(w_flag == 1) 
								{
									w_flag = 0;
									find_flag = 1;
									break;
								}
								else
								{
									p = sub;
									sub = sub->next_node;
								}
								
							}
							else
							{
								p = sub;
								sub = sub->next_node;
							}
							
						}
						if(find_flag == 1)
						{
							find_flag = 0;
							break;
						}
				
					}
					if(sub==NULL)
					{
						break;
					}

					die_token=ssd->channel_head[channel].chip_head[chip_token].token;	
					if(sub->current_state==SR_WAIT)
					{
						plane_token=ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token;

						get_ppn(ssd,channel,chip_token,die_token,plane_token,sub);  

						ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token=(ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token+1)%ssd->parameter->plane_die;

						*change_current_time_flag=0;

						if(ssd->parameter->ad_priority2==0)
						{
							ssd->real_time_subreq--;
						}
						go_one_step(ssd,sub,NULL,SR_W_TRANSFER,NORMAL);       /*ִ����ͨ��״̬��ת�䡣*/
						//delete_w_sub_request(ssd,channel,sub);                /*ɾ����������д������*/
						for(m = 0; m < ssd->parameter->gc_buffer_size; m++)
						{
							if(lpn == ssd->gc_buffer[m])
							{
								ssd->gc_buffer[m] = -1;
							}
						}
						ssd->gc_buf_count--;
						
						if(sub!=ssd->channel_head[channel_gc_num].gc_sub_queue)                             /*if the request is completed, we delete it from read queue */							
						{		
							p->next_node=sub->next_node;
							if(sub == ssd->channel_head[channel_gc_num].gc_sub_tail)
							{
								ssd->channel_head[channel_gc_num].gc_sub_tail = p;
							}	
						}			
						else					
						{	
							if (ssd->channel_head[channel_gc_num].gc_sub_queue!=ssd->channel_head[channel_gc_num].gc_sub_tail)
							{
								ssd->channel_head[channel_gc_num].gc_sub_queue=sub->next_node;
							} 
							else
							{
								ssd->channel_head[channel_gc_num].gc_sub_queue=NULL;
								ssd->channel_head[channel_gc_num].gc_sub_tail=NULL;
							}							
						}

						free(sub);
						sub = NULL;	
			
						*channel_busy_flag=1;
						/**************************************************************************
						*����forѭ��ǰ���޸�����
						*�����token�ı仯��ȫȡ���������channel chip die plane��д�Ƿ�ɹ� 
						*�ɹ��˾�break û�ɹ�token��Ҫ�仯ֱ���ҵ���д�ɹ���channel chip die plane
						***************************************************************************/
						ssd->channel_head[channel].chip_head[chip_token].token=(ssd->channel_head[channel].chip_head[chip_token].token+1)%ssd->parameter->die_chip;
						ssd->channel_head[channel].token=(ssd->channel_head[channel].token+1)%ssd->parameter->chip_channel[channel];
						break;
					}							
					ssd->channel_head[channel].chip_head[chip_token].token=(ssd->channel_head[channel].chip_head[chip_token].token+1)%ssd->parameter->die_chip;
				}
			}			
			ssd->channel_head[channel].token=(ssd->channel_head[channel].token+1)%ssd->parameter->chip_channel[channel];
		}
	}
	
	return SUCCESS;	
}


/********************************************************
*�����������Ҫ���������ض��������д�������״̬�仯����
*********************************************************/

struct ssd_info *process(struct ssd_info *ssd)   
{

	/*********************************************************************************************************
	*flag_die��ʾ�Ƿ���Ϊdie��busy��������ʱ��ǰ����-1��ʾû�У���-1��ʾ��������
	*flag_die��ֵ��ʾdie��,old ppn��¼��copyback֮ǰ������ҳ�ţ������ж�copyback�Ƿ���������ż��ַ�����ƣ�
	*two_plane_bit[8],two_plane_place[8]�����Ա��ʾͬһ��channel��ÿ��die��������������
	*chg_cur_time_flag��Ϊ�Ƿ���Ҫ������ǰʱ��ı�־λ������Ϊchannel����busy������������ʱ����Ҫ������ǰʱ�䣻
	*��ʼ��Ϊ��Ҫ��������Ϊ1�����κ�һ��channel�����˴��������������ʱ�����ֵ��Ϊ0����ʾ����Ҫ������
	**********************************************************************************************************/
	int old_ppn=-1,flag_die=-1; 
	unsigned int i,j,m,n,chan,chip,die,plane,random_num;     
	unsigned int flag=0,new_write=0,chg_cur_time_flag=1,flag2=0,flag_gc=0;       
	int64_t time, channel_time=MAX_INT64;
	struct sub_request *sub;
	unsigned int current_state=0, next_state=0; 
	long long next_state_predict_time=0;         

#ifdef DEBUG
	printf("enter process,  current time:%lld\n",ssd->current_time);
#endif

	/*********************************************************
	*�ж��Ƿ��ж�д�������������ôflag��Ϊ0��û��flag��Ϊ1
	*��flagΪ1ʱ����ssd����gc������ʱ�Ϳ���ִ��gc����
	*����û�����⣬������û��GC�����л���Ӱ��ͨ��״̬�Ķ������󡣶���GC��ÿ��ͨ�����У������ж�ҳǨ�Ƶ�ʱ��û��ͳ�����ڣ�����֣������޸�
	**********************************************************/


	/*for(i=0;i<ssd->parameter->channel_number;i++)
	{          
		if((ssd->channel_head[i].subs_r_head==NULL)&&(ssd->channel_head[i].subs_w_head==NULL)&&(ssd->subs_w_head==NULL))
		{
			flag=1;                       //����ͨ�������������ϱ�һ�У�����ȫ��̬������Ե�д���󣬲�����ͨ���ϣ���Ҫ�ٷ���
		}
		else
		{
			flag=0;
			break;
		}
	}
	if(flag==1)
	{
		ssd->flag=1;                                                                
		if (ssd->gc_request>0)      //�޶�д���󣬿�ʼ������ж�  soft_gc ��ʵӦ���Ƕ�ÿ��ͨ�������жϣ������޸�Ϊ�����ж�GC��д��ʱ������ִ��
		{
			gc(ssd,0,1);            //���gcҪ������channel�����������
		}
		return ssd;
	}
	else
	{
		ssd->flag=0;
	}*/
		
	time = ssd->current_time;
	services_2_r_cmd_trans_and_complete(ssd);                                            /*����ǰ״̬��SR_R_C_A_TRANSFER���ߵ�ǰ״̬��SR_COMPLETE��������һ״̬��SR_COMPLETE������һ״̬Ԥ��ʱ��С�ڵ�ǰ״̬ʱ��*/
	
	//KXC_2:�������ж�GC hard GC
	if(ssd->parameter->interruptible == 2)
	{
		for(i=0;i<ssd->parameter->channel_number;i++)
		{
			for(j=0;j<ssd->parameter->chip_channel[0];j++)
			{
				for(m=0;m<ssd->parameter->die_chip;m++)
				{
					for (n=0;n<ssd->parameter->plane_die; n++)
					{
						if(ssd->channel_head[i].chip_head[j].die_head[m].plane_head[n].gc_hard_head!=NULL)
						{
							if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time))
							{
								ssd->gc_type = 1;
								gc(ssd,i,0);
							}
						}
					}
					
				}
			}
			
		}
	}
	else
	{
		for(i=0;i<ssd->parameter->channel_number;i++)
		{          
			if((ssd->channel_head[i].gc_command!=NULL)&&((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time)))
			{
				gc(ssd,i,0);			
			}
		}
	}
	



	random_num=ssd->program_count%ssd->parameter->channel_number;                        /*����һ�����������֤ÿ�δӲ�ͬ��channel��ʼ��ѯ*/

	/*****************************************
	*ѭ����������channel�ϵĶ�д������
	*���������������д���ݣ�����Ҫռ�����ߣ�
	******************************************/
	for(chan=0;chan<ssd->parameter->channel_number;chan++)	     
	{
		i=(random_num+chan)%ssd->parameter->channel_number;
		flag=0;
		flag_gc=0;                                                                       /*ÿ�ν���channelʱ����gc�ı�־λ��Ϊ0��Ĭ����Ϊû�н���gc����*/
		if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time))		
		{   
			/*if (ssd->gc_request>0)                                                       //��gc��������Ҫ����һ�����ж�
			{
				if (ssd->channel_head[i].gc_command!=NULL)
				{
					flag_gc=gc(ssd,i,0);                                                 //gc��������һ��ֵ����ʾ�Ƿ�ִ����gc���������ִ����gc����flag_gc=1�����channel�����ʱ�̲��ܷ�������������
				}
				if (flag_gc==1)                                                          //ִ�й�gc��������Ҫ�����˴�ѭ��
				{
					continue;
				}
			}*/

			sub=ssd->channel_head[i].subs_r_head;                                        /*�ȴ��������*/
			services_2_r_wait(ssd,i,&flag,&chg_cur_time_flag);                           /*�����ڵȴ�״̬�Ķ�������*/
		
			if((flag==0)&&(ssd->channel_head[i].subs_r_head!=NULL))                      /*if there are no new read request and data is ready in some dies, send these data to controller and response this request*/		
			{		     
				services_2_r_data_trans(ssd,i,&flag,&chg_cur_time_flag);                    
						
			}
			if(flag==0)                                                                  /*if there are no read request to take channel, we can serve write requests*/ 		
			{	
				services_2_write(ssd,i,&flag,&chg_cur_time_flag);
				
			}	
		}

		//KXC_2:ִ��������д���������ͨ����Ȼ���ڿ���״̬����ʼִ��soft gc
		if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time))		
		{   
			if(ssd->parameter->interruptible == 2)
			{
				for(j=0;j<ssd->parameter->chip_channel[0];j++)
				{
					for(m=0;m<ssd->parameter->die_chip;m++)
					{
						for (n=0;n<ssd->parameter->plane_die; n++)
						{
							if(((ssd->channel_head[i].chip_head[j].current_state==CHIP_IDLE)||(ssd->channel_head[i].chip_head[j].next_state==CHIP_IDLE&&ssd->channel_head[i].chip_head[j].next_state_predict_time<=ssd->current_time)))
							{
								if(ssd->channel_head[i].chip_head[j].die_head[m].plane_head[n].gc_soft_head!=NULL)
								{
									ssd->gc_type = 0;
									gc(ssd,i,0);
								}
							}
						}
						
					}
				}	
			}
			else if(ssd->channel_head[i].gc_sub_queue != NULL)
			{
				services_2_gc_sub(ssd,i,&flag,&chg_cur_time_flag); 
			}
		}	

		//KXC_2:to erase blocks that all pages are invalid when the plane is idel   not for use
		/*if(ssd->parameter->active_erase == 1)
		{
			if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||(ssd->channel_head[i].next_state==CHANNEL_IDLE&&ssd->channel_head[i].next_state_predict_time<=ssd->current_time))
			{
				for(chip = 0; chip < ssd->parameter->chip_channel[0];chip++)
				{
					
					current_state=ssd->channel_head[chan].chip_head[chip].current_state;
					next_state=ssd->channel_head[chan].chip_head[chip].next_state;
					next_state_predict_time=ssd->channel_head[chan].chip_head[chip].next_state_predict_time;
					if((current_state==CHIP_IDLE)||((next_state==CHIP_IDLE)&&(next_state_predict_time<=ssd->current_time)))
					{
						for(die = 0; die < ssd->parameter->die_chip; die++)
						{
							for(plane = 0; plane < ssd->parameter->plane_die; plane++)
							{
								if (ssd->channel_head[chan].chip_head[chip].die_head[die].plane_head[plane].free_page<(ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->gc_hard_threshold))
								{
									gc_direct_erase(ssd,chan,chip,die,plane);
								}
							}
						}
					}
				}
			}
			
		}*/
	}

	return ssd;
}

/****************************************************************************************************************************
*��ssd֧�ָ߼�����ʱ��������������þ��Ǵ���߼������д������
*��������ĸ���������ѡ�����ָ߼�����������ֻ����д���󣬶������Ѿ����䵽ÿ��channel��������ִ��ʱ֮�����ѡȡ��Ӧ�����
*****************************************************************************************************************************/
struct ssd_info *dynamic_advanced_process(struct ssd_info *ssd,unsigned int channel,unsigned int chip)         
{
	unsigned int die=0,plane=0;
	unsigned int subs_count=0;
	int flag;
	unsigned int gate;                                                                    /*record the max subrequest that can be executed in the same channel. it will be used when channel-level priority order is highest and allocation scheme is full dynamic allocation*/
	unsigned int plane_place;                                                             /*record which plane has sub request in static allocation*/
	struct sub_request *sub=NULL,*p=NULL,*sub0=NULL,*sub1=NULL,*sub2=NULL,*sub3=NULL,*sub0_rw=NULL,*sub1_rw=NULL,*sub2_rw=NULL,*sub3_rw=NULL;
	struct sub_request ** subs=NULL;
	unsigned int max_sub_num=0;
	unsigned int die_token=0,plane_token=0;
	unsigned int * plane_bits=NULL;
	unsigned int interleaver_count=0;
	
	unsigned int mask=0x00000001;
	unsigned int i=0,j=0;
	
	max_sub_num=(ssd->parameter->die_chip)*(ssd->parameter->plane_die);
	gate=max_sub_num;
	subs=(struct sub_request **)malloc(max_sub_num*sizeof(struct sub_request *));
	alloc_assert(subs,"sub_request");
	
	for(i=0;i<max_sub_num;i++)
	{
		subs[i]=NULL;
	}
	
	if((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==0)&&(ssd->parameter->ad_priority2==0))
	{
		gate=ssd->real_time_subreq/ssd->parameter->channel_number;

		if(gate==0)
		{
			gate=1;
		}
		else
		{
			if(ssd->real_time_subreq%ssd->parameter->channel_number!=0)
			{
				gate++;
			}
		}
	}

	if ((ssd->parameter->allocation_scheme==0))                                           /*ȫ��̬���䣬��Ҫ��ssd->subs_w_head��ѡȡ�ȴ������������*/
	{
		if(ssd->parameter->dynamic_allocation==0)
		{
			sub=ssd->subs_w_head;
		}
		else
		{
			sub=ssd->channel_head[channel].subs_w_head;
		}
		
		subs_count=0;
		
		while ((sub!=NULL)&&(subs_count<max_sub_num)&&(subs_count<gate))
		{
			if(sub->current_state==SR_WAIT)								
			{
				if ((sub->update==NULL)||((sub->update!=NULL)&&((sub->update->current_state==SR_COMPLETE)||((sub->update->next_state==SR_COMPLETE)&&(sub->update->next_state_predict_time<=ssd->current_time)))))    //û����Ҫ��ǰ������ҳ
				{
					subs[subs_count]=sub;
					subs_count++;
				}						
			}
			
			p=sub;
			sub=sub->next_node;	
		}

		if (subs_count==0)                                                               /*û��������Է��񣬷���NULL*/
		{
			for(i=0;i<max_sub_num;i++)
			{
				subs[i]=NULL;
			}
			free(subs);

			subs=NULL;
			free(plane_bits);
			return NULL;
		}
		if(subs_count>=2)
		{
		    /*********************************************
			*two plane,interleave������ʹ��
			*�����channel�ϣ�ѡ��interleave_two_planeִ��
			**********************************************/
			if (((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))     
			{                                                                        
				get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,INTERLEAVE_TWO_PLANE); 
			}
			else if (((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE))
			{
				if(subs_count>ssd->parameter->plane_die)
				{	
					for(i=ssd->parameter->plane_die;i<subs_count;i++)
					{
						subs[i]=NULL;
					}
					subs_count=ssd->parameter->plane_die;
				}
				get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,TWO_PLANE);
			}
			else if (((ssd->parameter->advanced_commands&AD_TWOPLANE)!=AD_TWOPLANE)&&((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE))
			{
				
				if(subs_count>ssd->parameter->die_chip)
				{	
					for(i=ssd->parameter->die_chip;i<subs_count;i++)
					{
						subs[i]=NULL;
					}
					subs_count=ssd->parameter->die_chip;
				}
				get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,INTERLEAVE);
			}
			else
			{
				for(i=1;i<subs_count;i++)
				{
					subs[i]=NULL;
				}
				subs_count=1;
				get_ppn_for_normal_command(ssd,channel,chip,subs[0]);
			}
			
		}//if(subs_count>=2)
		else if(subs_count==1)     //only one request
		{
			get_ppn_for_normal_command(ssd,channel,chip,subs[0]);
		}
		
	}//if ((ssd->parameter->allocation_scheme==0)) 
	else                                                                                  /*��̬���䷽ʽ��ֻ�������ض���channel��ѡȡ�ȴ������������*/
	{
		                                                                                  /*�ھ�̬���䷽ʽ�У�����channel�ϵ���������ͬһ��die�ϵ���Щplane��ȷ��ʹ��ʲô����*/
		
			sub=ssd->channel_head[channel].subs_w_head;
			plane_bits=(unsigned int * )malloc((ssd->parameter->die_chip)*sizeof(unsigned int));
			alloc_assert(plane_bits,"plane_bits");
			memset(plane_bits,0, (ssd->parameter->die_chip)*sizeof(unsigned int));

			for(i=0;i<ssd->parameter->die_chip;i++)
			{
				plane_bits[i]=0x00000000;
			}
			subs_count=0;
			
			while ((sub!=NULL)&&(subs_count<max_sub_num))
			{
				if(sub->current_state==SR_WAIT)								
				{
					if ((sub->update==NULL)||((sub->update!=NULL)&&((sub->update->current_state==SR_COMPLETE)||((sub->update->next_state==SR_COMPLETE)&&(sub->update->next_state_predict_time<=ssd->current_time)))))
					{
						if (sub->location->chip==chip)
						{
							plane_place=0x00000001<<(sub->location->plane);
	
							if ((plane_bits[sub->location->die]&plane_place)!=plane_place)      //we have not add sub request to this plane
							{
								subs[sub->location->die*ssd->parameter->plane_die+sub->location->plane]=sub;
								subs_count++;
								plane_bits[sub->location->die]=(plane_bits[sub->location->die]|plane_place);
							}
						}
					}						
				}
				sub=sub->next_node;	
			}//while ((sub!=NULL)&&(subs_count<max_sub_num))

			if (subs_count==0)                                                            /*û��������Է��񣬷���NULL*/
			{
				for(i=0;i<max_sub_num;i++)
				{
					subs[i]=NULL;
				}
				free(subs);
				subs=NULL;
				free(plane_bits);
				return NULL;
			}
			
			flag=0;
			if (ssd->parameter->advanced_commands!=0)
			{
				if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)        /*ȫ���߼��������ʹ��*/
				{
					if (subs_count>1)                                                    /*��1�����Ͽ���ֱ�ӷ����д����*/
					{
						get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,COPY_BACK);
					} 
					else
					{
						for(i=0;i<max_sub_num;i++)
						{
							if(subs[i]!=NULL)
							{
								break;
							}
						}
						get_ppn_for_normal_command(ssd,channel,chip,subs[i]);
					}
				
				}// if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
				else                                                                     /*����ִ��copyback*/
				{
					if (subs_count>1)                                                    /*��1�����Ͽ���ֱ�ӷ����д����*/
					{
						if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
						{
							get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,INTERLEAVE_TWO_PLANE);
						} 
						else if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)!=AD_TWOPLANE))
						{
							for(die=0;die<ssd->parameter->die_chip;die++)
							{
								if(plane_bits[die]!=0x00000000)
								{
									for(i=0;i<ssd->parameter->plane_die;i++)
									{
										plane_token=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
										ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane_token+1)%ssd->parameter->plane_die;
										mask=0x00000001<<plane_token;
										if((plane_bits[die]&mask)==mask)
										{
											plane_bits[die]=mask;
											break;
										}
									}
									for(i=i+1;i<ssd->parameter->plane_die;i++)
									{
										plane=(plane_token+1)%ssd->parameter->plane_die;
										subs[die*ssd->parameter->plane_die+plane]=NULL;
										subs_count--;
									}
									interleaver_count++;
								}//if(plane_bits[die]!=0x00000000)
							}//for(die=0;die<ssd->parameter->die_chip;die++)
							if(interleaver_count>=2)
							{
								get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,INTERLEAVE);
							}
							else
							{
								for(i=0;i<max_sub_num;i++)
								{
									if(subs[i]!=NULL)
									{
										break;
									}
								}
								get_ppn_for_normal_command(ssd,channel,chip,subs[i]);	
							}
						}//else if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)!=AD_TWOPLANE))
						else if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
						{
							for(i=0;i<ssd->parameter->die_chip;i++)
							{
								die_token=ssd->channel_head[channel].chip_head[chip].token;
								ssd->channel_head[channel].chip_head[chip].token=(die_token+1)%ssd->parameter->die_chip;
								if(size(plane_bits[die_token])>1)
								{
									break;
								}
								
							}
							
							if(i<ssd->parameter->die_chip)
							{
								for(die=0;die<ssd->parameter->die_chip;die++)
								{
									if(die!=die_token)
									{
										for(plane=0;plane<ssd->parameter->plane_die;plane++)
										{
											if(subs[die*ssd->parameter->plane_die+plane]!=NULL)
											{
												subs[die*ssd->parameter->plane_die+plane]=NULL;
												subs_count--;
											}
										}
									}
								}
								get_ppn_for_advanced_commands(ssd,channel,chip,subs,subs_count,TWO_PLANE);
							}//if(i<ssd->parameter->die_chip)
							else
							{
								for(i=0;i<ssd->parameter->die_chip;i++)
								{
									die_token=ssd->channel_head[channel].chip_head[chip].token;
									ssd->channel_head[channel].chip_head[chip].token=(die_token+1)%ssd->parameter->die_chip;
									if(plane_bits[die_token]!=0x00000000)
									{
										for(j=0;j<ssd->parameter->plane_die;j++)
										{
											plane_token=ssd->channel_head[channel].chip_head[chip].die_head[die_token].token;
											ssd->channel_head[channel].chip_head[chip].die_head[die_token].token=(plane_token+1)%ssd->parameter->plane_die;
											if(((plane_bits[die_token])&(0x00000001<<plane_token))!=0x00000000)
											{
												sub=subs[die_token*ssd->parameter->plane_die+plane_token];
												break;
											}
										}
									}
								}//for(i=0;i<ssd->parameter->die_chip;i++)
								get_ppn_for_normal_command(ssd,channel,chip,sub);
							}//else
						}//else if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
					}//if (subs_count>1)  
					else
					{
						for(i=0;i<ssd->parameter->die_chip;i++)
						{
							die_token=ssd->channel_head[channel].chip_head[chip].token;
							ssd->channel_head[channel].chip_head[chip].token=(die_token+1)%ssd->parameter->die_chip;
							if(plane_bits[die_token]!=0x00000000)
							{
								for(j=0;j<ssd->parameter->plane_die;j++)
								{
									plane_token=ssd->channel_head[channel].chip_head[chip].die_head[die_token].token;
									ssd->channel_head[channel].chip_head[chip].die_head[die_token].token=(plane_token+1)%ssd->parameter->plane_die;
									if(((plane_bits[die_token])&(0x00000001<<plane_token))!=0x00000000)
									{
										sub=subs[die_token*ssd->parameter->plane_die+plane_token];
										break;
									}
								}
								if(sub!=NULL)
								{
									break;
								}
							}
						}//for(i=0;i<ssd->parameter->die_chip;i++)
						get_ppn_for_normal_command(ssd,channel,chip,sub);
					}//else
				}
			}//if (ssd->parameter->advanced_commands!=0)
			else
			{
				for(i=0;i<ssd->parameter->die_chip;i++)
				{
					die_token=ssd->channel_head[channel].chip_head[chip].token;
					ssd->channel_head[channel].chip_head[chip].token=(die_token+1)%ssd->parameter->die_chip;
					if(plane_bits[die_token]!=0x00000000)
					{
						for(j=0;j<ssd->parameter->plane_die;j++)
						{
							plane_token=ssd->channel_head[channel].chip_head[chip].die_head[die_token].token;
							ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane_token+1)%ssd->parameter->plane_die;
							if(((plane_bits[die_token])&(0x00000001<<plane_token))!=0x00000000)
							{
								sub=subs[die_token*ssd->parameter->plane_die+plane_token];
								break;
							}
						}
						if(sub!=NULL)
						{
							break;
						}
					}
				}//for(i=0;i<ssd->parameter->die_chip;i++)
				get_ppn_for_normal_command(ssd,channel,chip,sub);
			}//else
		
	}//else

	for(i=0;i<max_sub_num;i++)
	{
		subs[i]=NULL;
	}
	free(subs);
	subs=NULL;
	free(plane_bits);
	return ssd;
}

/****************************************
*ִ��д������ʱ��Ϊ��ͨ��д�������ȡppn
*****************************************/
Status get_ppn_for_normal_command(struct ssd_info * ssd, unsigned int channel,unsigned int chip, struct sub_request * sub)
{
	unsigned int die=0;
	unsigned int plane=0;
	if(sub==NULL)
	{
		return ERROR;
	}
	
	if (ssd->parameter->allocation_scheme==DYNAMIC_ALLOCATION)
	{
		die=ssd->channel_head[channel].chip_head[chip].token;
		plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
		get_ppn(ssd,channel,chip,die,plane,sub);
		ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
		ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
		
		compute_serve_time(ssd,channel,chip,die,&sub,1,NORMAL);
		return SUCCESS;
	}
	else
	{
		die=sub->location->die;
		plane=sub->location->plane;
		get_ppn(ssd,channel,chip,die,plane,sub);   
		compute_serve_time(ssd,channel,chip,die,&sub,1,NORMAL);
		return SUCCESS;
	}

}



/************************************************************************************************
*Ϊ�߼������ȡppn
*���ݲ�ͬ����������ͬһ��block��˳��д��Ҫ��ѡȡ���Խ���д������ppn��������ppnȫ����ΪʧЧ��
*��ʹ��two plane����ʱ��Ϊ��Ѱ����ͬˮƽλ�õ�ҳ��������Ҫֱ���ҵ�������ȫ�հ׵Ŀ飬���ʱ��ԭ��
*�Ŀ�û�����ֻ꣬�ܷ����⣬�ȴ��´�ʹ�ã�ͬʱ�޸Ĳ��ҿհ�page�ķ���������ǰ����Ѱ��free���Ϊ��ֻ
*Ҫinvalid block!=64���ɡ�
*except find aim page, we should modify token and decide gc operation
*************************************************************************************************/
Status get_ppn_for_advanced_commands(struct ssd_info *ssd,unsigned int channel,unsigned int chip,struct sub_request * * subs ,unsigned int subs_count,unsigned int command)      
{
	unsigned int die=0,plane=0;
	unsigned int die_token=0,plane_token=0;
	struct sub_request * sub=NULL;
	unsigned int i=0,j=0,k=0;
	unsigned int unvalid_subs_count=0;
	unsigned int valid_subs_count=0;
	unsigned int interleave_flag=FALSE;
	unsigned int multi_plane_falg=FALSE;
	unsigned int max_subs_num=0;
	struct sub_request * first_sub_in_chip=NULL;
	struct sub_request * first_sub_in_die=NULL;
	struct sub_request * second_sub_in_die=NULL;
	unsigned int state=SUCCESS;
	unsigned int multi_plane_flag=FALSE;

	max_subs_num=ssd->parameter->die_chip*ssd->parameter->plane_die;
	
	if (ssd->parameter->allocation_scheme==DYNAMIC_ALLOCATION)                         /*��̬�������*/ 
	{
		if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))                      /*INTERLEAVE_TWO_PLANE�Լ�COPY_BACK�����*/
		{
			for(i=0;i<subs_count;i++)
			{
				die=ssd->channel_head[channel].chip_head[chip].token;
				if(i<ssd->parameter->die_chip)                                         /*Ϊÿ��subs[i]��ȡppn��iС��die_chip*/
				{
					plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
					get_ppn(ssd,channel,chip,die,plane,subs[i]);
					ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
				}
				else                                                                  
				{   
					/*********************************************************************************************************************************
					*����die_chip��i��ָ���subs[i]��subs[i%ssd->parameter->die_chip]��ȡ��ͬλ�õ�ppn
					*����ɹ��Ļ�ȡ������multi_plane_flag=TRUE��ִ��compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE_TWO_PLANE);
					*����ִ��compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE);
					***********************************************************************************************************************************/
					state=make_level_page(ssd,subs[i%ssd->parameter->die_chip],subs[i]);
					if(state!=SUCCESS)                                                 
					{
						subs[i]=NULL;
						unvalid_subs_count++;
					}
					else
					{
						multi_plane_flag=TRUE;
					}
				}
				ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			}
			valid_subs_count=subs_count-unvalid_subs_count;
			ssd->interleave_count++;
			if(multi_plane_flag==TRUE)
			{
				ssd->inter_mplane_count++;
				compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE_TWO_PLANE);/*����д������Ĵ���ʱ�䣬��д�������״̬ת��*/		
			}
			else
			{
				compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE);
			}
			return SUCCESS;
		}//if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))
		else if(command==INTERLEAVE)
		{
			/***********************************************************************************************
			*INTERLEAVE�߼�����Ĵ�����������TWO_PLANE�߼�����Ĵ����
			*��Ϊtwo_plane��Ҫ����ͬһ��die���治ͬplane��ͬһλ�õ�page����interleaveҪ�����ǲ�ͬdie����ġ�
			************************************************************************************************/
			for(i=0;(i<subs_count)&&(i<ssd->parameter->die_chip);i++)
			{
				die=ssd->channel_head[channel].chip_head[chip].token;
				plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
				get_ppn(ssd,channel,chip,die,plane,subs[i]);
				ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane+1)%ssd->parameter->plane_die;
				ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
				valid_subs_count++;
			}
			ssd->interleave_count++;
			compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE);
			return SUCCESS;
		}//else if(command==INTERLEAVE)
		else if(command==TWO_PLANE)
		{
			if(subs_count<2)
			{
				return ERROR;
			}
			die=ssd->channel_head[channel].chip_head[chip].token;
			for(j=0;j<subs_count;j++)
			{
				if(j==1)
				{
					state=find_level_page(ssd,channel,chip,die,subs[0],subs[1]);        /*Ѱ����subs[0]��ppnλ����ͬ��subs[1]��ִ��TWO_PLANE�߼�����*/
					if(state!=SUCCESS)
					{
						get_ppn_for_normal_command(ssd,channel,chip,subs[0]);           /*û�ҵ�����ô�͵���ͨ����������*/
						return FAILURE;
					}
					else
					{
						valid_subs_count=2;
					}
				}
				else if(j>1)
				{
					state=make_level_page(ssd,subs[0],subs[j]);                         /*Ѱ����subs[0]��ppnλ����ͬ��subs[j]��ִ��TWO_PLANE�߼�����*/
					if(state!=SUCCESS)
					{
						for(k=j;k<subs_count;k++)
						{
							subs[k]=NULL;
						}
						subs_count=j;
						break;
					}
					else
					{
						valid_subs_count++;
					}
				}
			}//for(j=0;j<subs_count;j++)
			ssd->channel_head[channel].chip_head[chip].token=(die+1)%ssd->parameter->die_chip;
			ssd->m_plane_prog_count++;
			compute_serve_time(ssd,channel,chip,die,subs,valid_subs_count,TWO_PLANE);
			return SUCCESS;
		}//else if(command==TWO_PLANE)
		else 
		{
			return ERROR;
		}
	}//if (ssd->parameter->allocation_scheme==DYNAMIC_ALLOCATION)
	else                                                                              /*��̬��������*/
	{
		if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))
		{
			for(die=0;die<ssd->parameter->die_chip;die++)
			{
				first_sub_in_die=NULL;
				for(plane=0;plane<ssd->parameter->plane_die;plane++)
				{
					sub=subs[die*ssd->parameter->plane_die+plane];
					if(sub!=NULL)
					{
						if(first_sub_in_die==NULL)
						{
							first_sub_in_die=sub;
							get_ppn(ssd,channel,chip,die,plane,sub);
						}
						else
						{
							state=make_level_page(ssd,first_sub_in_die,sub);
							if(state!=SUCCESS)
							{
								subs[die*ssd->parameter->plane_die+plane]=NULL;
								subs_count--;
								sub=NULL;
							}
							else
							{
								multi_plane_flag=TRUE;
							}
						}
					}
				}
			}
			if(multi_plane_flag==TRUE)
			{
				ssd->inter_mplane_count++;
				compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE_TWO_PLANE);
				return SUCCESS;
			}
			else
			{
				compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE);
				return SUCCESS;
			}
		}//if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))
		else if(command==INTERLEAVE)
		{
			for(die=0;die<ssd->parameter->die_chip;die++)
			{	
				first_sub_in_die=NULL;
				for(plane=0;plane<ssd->parameter->plane_die;plane++)
				{
					sub=subs[die*ssd->parameter->plane_die+plane];
					if(sub!=NULL)
					{
						if(first_sub_in_die==NULL)
						{
							first_sub_in_die=sub;
							get_ppn(ssd,channel,chip,die,plane,sub);
							valid_subs_count++;
						}
						else
						{
							subs[die*ssd->parameter->plane_die+plane]=NULL;
							subs_count--;
							sub=NULL;
						}
					}
				}
			}
			if(valid_subs_count>1)
			{
				ssd->interleave_count++;
			}
			compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE);	
		}//else if(command==INTERLEAVE)
		else if(command==TWO_PLANE)
		{
			for(die=0;die<ssd->parameter->die_chip;die++)
			{	
				first_sub_in_die=NULL;
				second_sub_in_die=NULL;
				for(plane=0;plane<ssd->parameter->plane_die;plane++)
				{
					sub=subs[die*ssd->parameter->plane_die+plane];
					if(sub!=NULL)
					{	
						if(first_sub_in_die==NULL)
						{
							first_sub_in_die=sub;
						}
						else if(second_sub_in_die==NULL)
						{
							second_sub_in_die=sub;
							state=find_level_page(ssd,channel,chip,die,first_sub_in_die,second_sub_in_die);
							if(state!=SUCCESS)
							{
								subs[die*ssd->parameter->plane_die+plane]=NULL;
								subs_count--;
								second_sub_in_die=NULL;
								sub=NULL;
							}
							else
							{
								valid_subs_count=2;
							}
						}
						else
						{
							state=make_level_page(ssd,first_sub_in_die,sub);
							if(state!=SUCCESS)
							{
								subs[die*ssd->parameter->plane_die+plane]=NULL;
								subs_count--;
								sub=NULL;
							}
							else
							{
								valid_subs_count++;
							}
						}
					}//if(sub!=NULL)
				}//for(plane=0;plane<ssd->parameter->plane_die;plane++)
				if(second_sub_in_die!=NULL)
				{
					multi_plane_flag=TRUE;
					break;
				}
			}//for(die=0;die<ssd->parameter->die_chip;die++)
			if(multi_plane_flag==TRUE)
			{
				ssd->m_plane_prog_count++;
				compute_serve_time(ssd,channel,chip,die,subs,valid_subs_count,TWO_PLANE);
				return SUCCESS;
			}//if(multi_plane_flag==TRUE)
			else
			{
				i=0;
				sub=NULL;
				while((sub==NULL)&&(i<max_subs_num))
				{
					sub=subs[i];
					i++;
				}
				if(sub!=NULL)
				{
					get_ppn_for_normal_command(ssd,channel,chip,sub);
					return FAILURE;
				}
				else 
				{
					return ERROR;
				}
			}//else
		}//else if(command==TWO_PLANE)
		else
		{
			return ERROR;
		}
	}//elseb ��̬��������
}


/***********************************************
*��������������sub0��sub1��ppn���ڵ�pageλ����ͬ
*����ҳ����߼������Ҫ��
************************************************/
Status make_level_page(struct ssd_info * ssd, struct sub_request * sub0,struct sub_request * sub1)
{
	unsigned int i=0,j=0,k=0;
	unsigned int channel=0,chip=0,die=0,plane0=0,plane1=0,block0=0,block1=0,page0=0,page1=0;
	unsigned int active_block0=0,active_block1=0;
	unsigned int old_plane_token=0;
	
	if((sub0==NULL)||(sub1==NULL)||(sub0->location==NULL))
	{
		return ERROR;
	}
	channel=sub0->location->channel;
	chip=sub0->location->chip;
	die=sub0->location->die;
	plane0=sub0->location->plane;
	block0=sub0->location->block;
	page0=sub0->location->page;
	old_plane_token=ssd->channel_head[channel].chip_head[chip].die_head[die].token;

	/***********************************************************************************************
	*��̬����������
	*sub1��plane�Ǹ���sub0��ssd->channel_head[channel].chip_head[chip].die_head[die].token���ƻ�ȡ��
	*sub1��channel��chip��die��block��page����sub0����ͬ
	************************************************************************************************/
	if(ssd->parameter->allocation_scheme==DYNAMIC_ALLOCATION)                             
	{
		old_plane_token=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
		for(i=0;i<ssd->parameter->plane_die;i++)
		{
			plane1=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
			if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].add_reg_ppn==-1)
			{
				find_active_block(ssd,channel,chip,die,plane1);                               /*��plane1���ҵ���Ծ��*/
				block1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].active_block;

				/*********************************************************************************************
				*ֻ���ҵ���block1��block0��ͬ�����ܼ�������Ѱ����ͬ��page
				*��Ѱ��pageʱ�Ƚϼ򵥣�ֱ����last_write_page����һ��д��page��+1�Ϳ����ˡ�
				*����ҵ���page����ͬ����ô���ssd����̰����ʹ�ø߼���������Ϳ�����С��page �����page��£
				*********************************************************************************************/
				if(block1==block0)
				{
					page1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].blk_head[block1].last_write_page+1;
					if(page1==page0)
					{
						break;
					}
					else if(page1<page0)
					{
						if (ssd->parameter->greed_MPW_ad==1)                                  /*����̰����ʹ�ø߼�����*/
						{                                                                   
							//make_same_level(ssd,channel,chip,die,plane1,active_block1,page0); /*С��page��ַ�����page��ַ��*/
							make_same_level(ssd,channel,chip,die,plane1,block1,page0);
							break;
						}    
					}
				}//if(block1==block0)
			}
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane1+1)%ssd->parameter->plane_die;
		}//for(i=0;i<ssd->parameter->plane_die;i++)
		if(i<ssd->parameter->plane_die)
		{
			flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page0);          /*������������þ��Ǹ���page1����Ӧ������ҳ�Լ�location����map��*/
			//flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page1);
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(plane1+1)%ssd->parameter->plane_die;
			return SUCCESS;
		}
		else
		{
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane_token;
			return FAILURE;
		}
	}
	else                                                                                      /*��̬��������*/
	{
		if((sub1->location==NULL)||(sub1->location->channel!=channel)||(sub1->location->chip!=chip)||(sub1->location->die!=die))
		{
			return ERROR;
		}
		plane1=sub1->location->plane;
		if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].add_reg_ppn==-1)
		{
			find_active_block(ssd,channel,chip,die,plane1);
			block1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].active_block;
			if(block1==block0)
			{
				page1=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].blk_head[block1].last_write_page+1;
				if(page1>page0)
				{
					return FAILURE;
				}
				else if(page1<page0)
				{
					if (ssd->parameter->greed_MPW_ad==1)
					{ 
						//make_same_level(ssd,channel,chip,die,plane1,active_block1,page0);    /*С��page��ַ�����page��ַ��*/
                        make_same_level(ssd,channel,chip,die,plane1,block1,page0);
						flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page0);
						//flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page1);
						return SUCCESS;
					}
					else
					{
						return FAILURE;
					}					
				}
				else
				{
					flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page0);
					//flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page1);
					return SUCCESS;
				}
				
			}
			else
			{
				return FAILURE;
			}
			
		}
		else
		{
			return ERROR;
		}
	}
	
}

/******************************************************************************************************
*�����Ĺ�����Ϊtwo plane����Ѱ�ҳ�������ͬˮƽλ�õ�ҳ�������޸�ͳ��ֵ���޸�ҳ��״̬
*ע�������������һ������make_level_page����������make_level_page�����������sub1��sub0��pageλ����ͬ
*��find_level_page�������������ڸ�����channel��chip��die��������λ����ͬ��subA��subB��
*******************************************************************************************************/
Status find_level_page(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,struct sub_request *subA,struct sub_request *subB)       
{
	unsigned int i,planeA,planeB,active_blockA,active_blockB,pageA,pageB,aim_page,old_plane;
	struct gc_operation *gc_node;

	old_plane=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
    
	/************************************************************
	*�ڶ�̬����������
	*planeA����ֵΪdie�����ƣ����planeA��ż����ôplaneB=planeA+1
	*planeA����������ôplaneA+1��Ϊż��������planeB=planeA+1
	*************************************************************/
	if (ssd->parameter->allocation_scheme==0)                                                
	{
		planeA=ssd->channel_head[channel].chip_head[chip].die_head[die].token;
		if (planeA%2==0)
		{
			planeB=planeA+1;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(ssd->channel_head[channel].chip_head[chip].die_head[die].token+2)%ssd->parameter->plane_die;
		} 
		else
		{
			planeA=(planeA+1)%ssd->parameter->plane_die;
			planeB=planeA+1;
			ssd->channel_head[channel].chip_head[chip].die_head[die].token=(ssd->channel_head[channel].chip_head[chip].die_head[die].token+3)%ssd->parameter->plane_die;
		}
	} 
	else                                                                                     /*��̬������������ֱ�Ӹ�ֵ��planeA��planeB*/
	{
		planeA=subA->location->plane;
		planeB=subB->location->plane;
	}
	find_active_block(ssd,channel,chip,die,planeA);                                          /*Ѱ��active_block*/
	find_active_block(ssd,channel,chip,die,planeB);
	active_blockA=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].active_block;
	active_blockB=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].active_block;

	
    
	/*****************************************************
	*���active_block��ͬ����ô������������������ͬ��page
	*����ʹ��̰���ķ����ҵ�������ͬ��page
	******************************************************/
	if (active_blockA==active_blockB)
	{
		pageA=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockA].last_write_page+1;      
		pageB=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockB].last_write_page+1;
		if (pageA==pageB)                                                                    /*�������õ�ҳ������ͬһ��ˮƽλ����*/
		{
			flash_page_state_modify(ssd,subA,channel,chip,die,planeA,active_blockA,pageA);
			flash_page_state_modify(ssd,subB,channel,chip,die,planeB,active_blockB,pageB);
		} 
		else
		{
			if (ssd->parameter->greed_MPW_ad==1)                                             /*̰����ʹ�ø߼�����*/
			{
				if (pageA<pageB)                                                            
				{
					aim_page=pageB;
					make_same_level(ssd,channel,chip,die,planeA,active_blockA,aim_page);     /*С��page��ַ�����page��ַ��*/
				}
				else
				{
					aim_page=pageA;
					make_same_level(ssd,channel,chip,die,planeB,active_blockB,aim_page);    
				}
				flash_page_state_modify(ssd,subA,channel,chip,die,planeA,active_blockA,aim_page);
				flash_page_state_modify(ssd,subB,channel,chip,die,planeB,active_blockB,aim_page);
			} 
			else                                                                             /*����̰����ʹ�ø߼�����*/
			{
				subA=NULL;
				subB=NULL;
				ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane;
				return FAILURE;
			}
		}
	}
	/*********************************
	*����ҵ�������active_block����ͬ
	**********************************/
	else
	{   
		pageA=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockA].last_write_page+1;      
		pageB=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockB].last_write_page+1;
		if (pageA<pageB)
		{
			if (ssd->parameter->greed_MPW_ad==1)                                             /*̰����ʹ�ø߼�����*/
			{
				/*******************************************************************************
				*��planeA�У���active_blockB��ͬλ�õĵ�block�У���pageB��ͬλ�õ�page�ǿ��õġ�
				*Ҳ����palneA�е���Ӧˮƽλ���ǿ��õģ�������Ϊ��planeB�ж�Ӧ��ҳ��
				*��ô��Ҳ��planeA��active_blockB�е�page��pageB��£
				********************************************************************************/
				if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockB].page_head[pageB].free_state==PG_SUB)    
				{
					make_same_level(ssd,channel,chip,die,planeA,active_blockB,pageB);
					flash_page_state_modify(ssd,subA,channel,chip,die,planeA,active_blockB,pageB);
					flash_page_state_modify(ssd,subB,channel,chip,die,planeB,active_blockB,pageB);
				}
                /********************************************************************************
				*��planeA�У���active_blockB��ͬλ�õĵ�block�У���pageB��ͬλ�õ�page�ǿ��õġ�
				*��ô��Ҫ����Ѱ��block����Ҫ������ˮƽλ����ͬ��һ��ҳ
				*********************************************************************************/
				else    
				{
					for (i=0;i<ssd->parameter->block_plane;i++)
					{
						pageA=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].last_write_page+1;
						pageB=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].last_write_page+1;
						if ((pageA<ssd->parameter->page_block)&&(pageB<ssd->parameter->page_block))
						{
							if (pageA<pageB)
							{
								if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].page_head[pageB].free_state==PG_SUB)
								{
									aim_page=pageB;
									make_same_level(ssd,channel,chip,die,planeA,i,aim_page);
									break;
								}
							} 
							else
							{
								if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].page_head[pageA].free_state==PG_SUB)
								{
									aim_page=pageA;
									make_same_level(ssd,channel,chip,die,planeB,i,aim_page);
									break;
								}
							}
						}
					}//for (i=0;i<ssd->parameter->block_plane;i++)
					if (i<ssd->parameter->block_plane)
					{
						flash_page_state_modify(ssd,subA,channel,chip,die,planeA,i,aim_page);
						flash_page_state_modify(ssd,subB,channel,chip,die,planeB,i,aim_page);
					} 
					else
					{
						subA=NULL;
						subB=NULL;
						ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane;
						return FAILURE;
					}
				}
			}//if (ssd->parameter->greed_MPW_ad==1)  
			else
			{
				subA=NULL;
				subB=NULL;
				ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane;
				return FAILURE;
			}
		}//if (pageA<pageB)
		else
		{
			if (ssd->parameter->greed_MPW_ad==1)     
			{
				if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockA].page_head[pageA].free_state==PG_SUB)
				{
					make_same_level(ssd,channel,chip,die,planeB,active_blockA,pageA);
					flash_page_state_modify(ssd,subA,channel,chip,die,planeA,active_blockA,pageA);
					flash_page_state_modify(ssd,subB,channel,chip,die,planeB,active_blockA,pageA);
				}
				else    
				{
					for (i=0;i<ssd->parameter->block_plane;i++)
					{
						pageA=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].last_write_page+1;
						pageB=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].last_write_page+1;
						if ((pageA<ssd->parameter->page_block)&&(pageB<ssd->parameter->page_block))
						{
							if (pageA<pageB)
							{
								if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].page_head[pageB].free_state==PG_SUB)
								{
									aim_page=pageB;
									make_same_level(ssd,channel,chip,die,planeA,i,aim_page);
									break;
								}
							} 
							else
							{
								if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].page_head[pageA].free_state==PG_SUB)
								{
									aim_page=pageA;
									make_same_level(ssd,channel,chip,die,planeB,i,aim_page);
									break;
								}
							}
						}
					}//for (i=0;i<ssd->parameter->block_plane;i++)
					if (i<ssd->parameter->block_plane)
					{
						flash_page_state_modify(ssd,subA,channel,chip,die,planeA,i,aim_page);
						flash_page_state_modify(ssd,subB,channel,chip,die,planeB,i,aim_page);
					} 
					else
					{
						subA=NULL;
						subB=NULL;
						ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane;
						return FAILURE;
					}
				}
			} //if (ssd->parameter->greed_MPW_ad==1) 
			else
			{
				if ((pageA==pageB)&&(pageA==0))
				{
					/*******************************************************************************************
					*�������������
					*1��planeA��planeB�е�active_blockA��pageAλ�ö����ã���ô��ͬplane ����ͬλ�ã���blockAΪ׼
					*2��planeA��planeB�е�active_blockB��pageAλ�ö����ã���ô��ͬplane ����ͬλ�ã���blockBΪ׼
					********************************************************************************************/
					if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockA].page_head[pageA].free_state==PG_SUB)
					  &&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockA].page_head[pageA].free_state==PG_SUB))
					{
						flash_page_state_modify(ssd,subA,channel,chip,die,planeA,active_blockA,pageA);
						flash_page_state_modify(ssd,subB,channel,chip,die,planeB,active_blockA,pageA);
					}
					else if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockB].page_head[pageA].free_state==PG_SUB)
						   &&(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockB].page_head[pageA].free_state==PG_SUB))
					{
						flash_page_state_modify(ssd,subA,channel,chip,die,planeA,active_blockB,pageA);
						flash_page_state_modify(ssd,subB,channel,chip,die,planeB,active_blockB,pageA);
					}
					else
					{
						subA=NULL;
						subB=NULL;
						ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane;
						return FAILURE;
					}
				}
				else
				{
					subA=NULL;
					subB=NULL;
					ssd->channel_head[channel].chip_head[chip].die_head[die].token=old_plane;
					return ERROR;
				}
			}
		}
	}

	if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].free_page<(ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->gc_hard_threshold))
	{
		gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
		alloc_assert(gc_node,"gc_node");
		memset(gc_node,0, sizeof(struct gc_operation));

		gc_node->next_node=NULL;
		gc_node->chip=chip;
		gc_node->die=die;
		gc_node->plane=planeA;
		gc_node->block=0xffffffff;
		gc_node->page=0;
		gc_node->state=GC_WAIT;
		gc_node->priority=GC_UNINTERRUPT;
		gc_node->next_node=ssd->channel_head[channel].gc_command;
		ssd->channel_head[channel].gc_command=gc_node;
		ssd->gc_request++;
	}
	if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].free_page<(ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->gc_hard_threshold))
	{
		gc_node=(struct gc_operation *)malloc(sizeof(struct gc_operation));
		alloc_assert(gc_node,"gc_node");
		memset(gc_node,0, sizeof(struct gc_operation));

		gc_node->next_node=NULL;
		gc_node->chip=chip;
		gc_node->die=die;
		gc_node->plane=planeB;
		gc_node->block=0xffffffff;
		gc_node->page=0;
		gc_node->state=GC_WAIT;
		gc_node->priority=GC_UNINTERRUPT;
		gc_node->next_node=ssd->channel_head[channel].gc_command;
		ssd->channel_head[channel].gc_command=gc_node;
		ssd->gc_request++;
	}

	return SUCCESS;     
}

/*
*�����Ĺ������޸��ҵ���pageҳ��״̬�Լ���Ӧ��dram��ӳ����ֵ
*/
struct ssd_info *flash_page_state_modify(struct ssd_info *ssd,struct sub_request *sub,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,unsigned int block,unsigned int page)
{
	unsigned int ppn,full_page;
	struct local *location;
	struct direct_erase *new_direct_erase,*direct_erase_node;
	
	full_page=~(0xffffffff<<ssd->parameter->subpage_page);
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page=page;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num--;

	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page>63)
	{
		printf("error! the last write page larger than 64!!\n");
		while(1){}
	}

	if(ssd->dram->map->map_entry[sub->lpn].state==0)                                          /*this is the first logical page*/
	{
		ssd->dram->map->map_entry[sub->lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
		ssd->dram->map->map_entry[sub->lpn].state=sub->state;
	}
	else                                                                                      /*����߼�ҳ�����˸��£���Ҫ��ԭ����ҳ��ΪʧЧ*/
	{
		ppn=ssd->dram->map->map_entry[sub->lpn].pn;
		location=find_location(ssd,ppn);
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state=0;        //��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state=0;         //��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn=0;
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
		ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].invalid_page++;
		if (ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num==ssd->parameter->page_block)    //��block��ȫ��invalid��ҳ������ֱ��ɾ��
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
		free(location);
		location=NULL;
		ssd->dram->map->map_entry[sub->lpn].pn=find_ppn(ssd,channel,chip,die,plane,block,page);
		ssd->dram->map->map_entry[sub->lpn].state=(ssd->dram->map->map_entry[sub->lpn].state|sub->state);
	}

	sub->ppn=ssd->dram->map->map_entry[sub->lpn].pn;
	sub->location->channel=channel;
	sub->location->chip=chip;
	sub->location->die=die;
	sub->location->plane=plane;
	sub->location->block=block;
	sub->location->page=page;
	
	ssd->program_count++;
	ssd->channel_head[channel].program_count++;
	ssd->channel_head[channel].chip_head[chip].program_count++;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].lpn=sub->lpn;	
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].valid_state=sub->state;
	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].free_state=((~(sub->state))&full_page);
	ssd->write_flash_count++;

	return ssd;
}


/********************************************
*�����Ĺ��ܾ���������λ�ò�ͬ��pageλ����ͬ
*********************************************/
struct ssd_info *make_same_level(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,unsigned int plane,unsigned int block,unsigned int aim_page)
{
	int i=0,step,page;
	struct direct_erase *new_direct_erase,*direct_erase_node;

	page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page+1;                  /*��Ҫ�����ĵ�ǰ��Ŀ�дҳ��*/
	step=aim_page-page;
	while (i<step)
	{
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page+i].valid_state=0;     /*��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0*/
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page+i].free_state=0;      /*��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0*/
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page+i].lpn=0;

		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num++;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].invalid_page++;
		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num--;

		ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;

		i++;
	}

	ssd->waste_page_count+=step;//֮ǰ��ҳ���˷ѵ���

	ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page=aim_page-1;

	if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num==ssd->parameter->page_block)    /*��block��ȫ��invalid��ҳ������ֱ��ɾ��*/
	{
		new_direct_erase=(struct direct_erase *)malloc(sizeof(struct direct_erase));
		alloc_assert(new_direct_erase,"new_direct_erase");
		memset(new_direct_erase,0, sizeof(struct direct_erase));

		direct_erase_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
		if (direct_erase_node==NULL)
		{
			ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node=new_direct_erase;
		} 
		else
		{
			new_direct_erase->next_node=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
			ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node=new_direct_erase;
		}
	}

	if(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page>63)
		{
		printf("error! the last write page larger than 64!!\n");
		while(1){}
		}

	return ssd;
}


/****************************************************************************
*�ڴ���߼������д������ʱ����������Ĺ��ܾ��Ǽ��㴦��ʱ���Լ������״̬ת��
*���ܻ����Ǻ����ƣ���Ҫ���ƣ��޸�ʱע��Ҫ��Ϊ��̬����Ͷ�̬�����������
KXC���ڴ���߼�����ʱ���ã����Ǻܶ�
*****************************************************************************/
struct ssd_info *compute_serve_time(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,struct sub_request **subs, unsigned int subs_count,unsigned int command)
{
	unsigned int i=0;
	unsigned int max_subs_num=0;
	struct sub_request *sub=NULL,*p=NULL;
	struct sub_request * last_sub=NULL;
	max_subs_num=ssd->parameter->die_chip*ssd->parameter->plane_die;

	if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))
	{
		for(i=0;i<max_subs_num;i++)
		{
			if(subs[i]!=NULL)
			{
				last_sub=subs[i];
				subs[i]->current_state=SR_W_TRANSFER;
				subs[i]->current_time=ssd->current_time;
				subs[i]->next_state=SR_COMPLETE;
				subs[i]->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(subs[i]->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				subs[i]->complete_time=subs[i]->next_state_predict_time;

				delete_from_channel(ssd,channel,subs[i]);
			}
		}
		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=last_sub->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;	
	}
	else if(command==TWO_PLANE)
	{
		for(i=0;i<max_subs_num;i++)
		{
			if(subs[i]!=NULL)
			{
				
				subs[i]->current_state=SR_W_TRANSFER;
				if(last_sub==NULL)
				{
					subs[i]->current_time=ssd->current_time;
				}
				else
				{
					subs[i]->current_time=last_sub->complete_time+ssd->parameter->time_characteristics.tDBSY;
				}
				
				subs[i]->next_state=SR_COMPLETE;
				subs[i]->next_state_predict_time=subs[i]->current_time+7*ssd->parameter->time_characteristics.tWC+(subs[i]->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				subs[i]->complete_time=subs[i]->next_state_predict_time;
				last_sub=subs[i];

				delete_from_channel(ssd,channel,subs[i]);
			}
		}
		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=last_sub->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
	}
	else if(command==INTERLEAVE)
	{
		for(i=0;i<max_subs_num;i++)
		{
			if(subs[i]!=NULL)
			{
				
				subs[i]->current_state=SR_W_TRANSFER;
				if(last_sub==NULL)
				{
					subs[i]->current_time=ssd->current_time;
				}
				else
				{
					subs[i]->current_time=last_sub->complete_time;
				}
				subs[i]->next_state=SR_COMPLETE;
				subs[i]->next_state_predict_time=subs[i]->current_time+7*ssd->parameter->time_characteristics.tWC+(subs[i]->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				subs[i]->complete_time=subs[i]->next_state_predict_time;
				last_sub=subs[i];

				delete_from_channel(ssd,channel,subs[i]);
			}
		}
		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=last_sub->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
	}
	else if(command==NORMAL)
	{
		subs[0]->current_state=SR_W_TRANSFER;
		subs[0]->current_time=ssd->current_time;
		subs[0]->next_state=SR_COMPLETE;
		subs[0]->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(subs[0]->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		subs[0]->complete_time=subs[0]->next_state_predict_time;

		delete_from_channel(ssd,channel,subs[0]);

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=subs[0]->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
	}
	else
	{
		return NULL;
	}
	
	return ssd;

}


/*****************************************************************************************
*�����Ĺ��ܾ��ǰ��������ssd->subs_w_head����ssd->channel_head[channel].subs_w_head��ɾ��
*ֻɾ��д������,�����������ʱ��ֱ��ɾ����
******************************************************************************************/
struct ssd_info *delete_from_channel(struct ssd_info *ssd,unsigned int channel,struct sub_request * sub_req)
{
	struct sub_request *sub,*p;
    
	/******************************************************************
	*��ȫ��̬�������������ssd->subs_w_head��
	*������ȫ��̬�������������ssd->channel_head[channel].subs_w_head��
	*******************************************************************/
	if ((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==0))    
	{
		sub=ssd->subs_w_head;
	} 
	else
	{
		sub=ssd->channel_head[channel].subs_w_head;
	}
	p=sub;

	while (sub!=NULL)
	{
		if (sub==sub_req)
		{
			if ((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==0))
			{
				if(ssd->parameter->ad_priority2==0)
				{
					ssd->real_time_subreq--;
				}
				
				if (sub==ssd->subs_w_head)                                                     /*������������sub request������ɾ��*/
				{
					if (ssd->subs_w_head!=ssd->subs_w_tail)
					{
						ssd->subs_w_head=sub->next_node;
						sub=ssd->subs_w_head;
						continue;
					} 
					else
					{
						ssd->subs_w_head=NULL;
						ssd->subs_w_tail=NULL;
						p=NULL;
						break;
					}
				}//if (sub==ssd->subs_w_head) 
				else
				{
					if (sub->next_node!=NULL)
					{
						p->next_node=sub->next_node;
						sub=p->next_node;
						continue;
					} 
					else
					{
						ssd->subs_w_tail=p;
						ssd->subs_w_tail->next_node=NULL;
						break;
					}
				}
			}//if ((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==0)) 
			else
			{
				if (sub==ssd->channel_head[channel].subs_w_head)                               /*������������channel������ɾ��*/
				{
					if (ssd->channel_head[channel].subs_w_head!=ssd->channel_head[channel].subs_w_tail)
					{
						ssd->channel_head[channel].subs_w_head=sub->next_node;
						sub=ssd->channel_head[channel].subs_w_head;
						continue;;
					} 
					else
					{
						ssd->channel_head[channel].subs_w_head=NULL;
						ssd->channel_head[channel].subs_w_tail=NULL;
						p=NULL;
						break;
					}
				}//if (sub==ssd->channel_head[channel].subs_w_head)
				else
				{
					if (sub->next_node!=NULL)
					{
						p->next_node=sub->next_node;
						sub=p->next_node;
						continue;
					} 
					else
					{
						ssd->channel_head[channel].subs_w_tail=p;
						ssd->channel_head[channel].subs_w_tail->next_node=NULL;
						break;
					}
				}//else
			}//else
		}//if (sub==sub_req)
		p=sub;
		sub=sub->next_node;
	}//while (sub!=NULL)

	return ssd;
}


struct ssd_info *un_greed_interleave_copyback(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,struct sub_request *sub1,struct sub_request *sub2)
{
	unsigned int old_ppn1,ppn1,old_ppn2,ppn2,greed_flag=0;

	old_ppn1=ssd->dram->map->map_entry[sub1->lpn].pn;
	get_ppn(ssd,channel,chip,die,sub1->location->plane,sub1);                                  /*�ҳ�����ppnһ���Ƿ���������������ͬ��plane��,����ʹ��copyback����*/
	ppn1=sub1->ppn;

	old_ppn2=ssd->dram->map->map_entry[sub2->lpn].pn;
	get_ppn(ssd,channel,chip,die,sub2->location->plane,sub2);                                  /*�ҳ�����ppnһ���Ƿ���������������ͬ��plane��,����ʹ��copyback����*/
	ppn2=sub2->ppn;

	if ((old_ppn1%2==ppn1%2)&&(old_ppn2%2==ppn2%2))
	{
		ssd->copy_back_count++;
		ssd->copy_back_count++;

		sub1->current_state=SR_W_TRANSFER;
		sub1->current_time=ssd->current_time;
		sub1->next_state=SR_COMPLETE;
		sub1->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub1->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub1->complete_time=sub1->next_state_predict_time;

		sub2->current_state=SR_W_TRANSFER;
		sub2->current_time=sub1->complete_time;
		sub2->next_state=SR_COMPLETE;
		sub2->next_state_predict_time=sub2->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub2->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub2->complete_time=sub2->next_state_predict_time;

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=sub2->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;

		delete_from_channel(ssd,channel,sub1);
		delete_from_channel(ssd,channel,sub2);
	} //if ((old_ppn1%2==ppn1%2)&&(old_ppn2%2==ppn2%2))
	else if ((old_ppn1%2==ppn1%2)&&(old_ppn2%2!=ppn2%2))
	{
		ssd->interleave_count--;
		ssd->copy_back_count++;

		sub1->current_state=SR_W_TRANSFER;
		sub1->current_time=ssd->current_time;
		sub1->next_state=SR_COMPLETE;
		sub1->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub1->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub1->complete_time=sub1->next_state_predict_time;

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=sub1->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;

		delete_from_channel(ssd,channel,sub1);
	}//else if ((old_ppn1%2==ppn1%2)&&(old_ppn2%2!=ppn2%2))
	else if ((old_ppn1%2!=ppn1%2)&&(old_ppn2%2==ppn2%2))
	{
		ssd->interleave_count--;
		ssd->copy_back_count++;

		sub2->current_state=SR_W_TRANSFER;
		sub2->current_time=ssd->current_time;
		sub2->next_state=SR_COMPLETE;
		sub2->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub2->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub2->complete_time=sub2->next_state_predict_time;

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=sub2->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;

		delete_from_channel(ssd,channel,sub2);
	}//else if ((old_ppn1%2!=ppn1%2)&&(old_ppn2%2==ppn2%2))
	else
	{
		ssd->interleave_count--;

		sub1->current_state=SR_W_TRANSFER;
		sub1->current_time=ssd->current_time;
		sub1->next_state=SR_COMPLETE;
		sub1->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+2*(ssd->parameter->subpage_page*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub1->complete_time=sub1->next_state_predict_time;

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=sub1->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;

		delete_from_channel(ssd,channel,sub1);
	}//else

	return ssd;
}

//�������û�б����ù�����صĹ�����gc()���������Ҳ��
struct ssd_info *un_greed_copyback(struct ssd_info *ssd,unsigned int channel,unsigned int chip,unsigned int die,struct sub_request *sub1)
{
	unsigned int old_ppn,ppn;

	old_ppn=ssd->dram->map->map_entry[sub1->lpn].pn;
	get_ppn(ssd,channel,chip,die,0,sub1);                                                     /*�ҳ�����ppnһ���Ƿ���������������ͬ��plane��,����ʹ��copyback����*/
	ppn=sub1->ppn;
	
	if (old_ppn%2==ppn%2)        //��żУ��һ��
	{
		ssd->copy_back_count++;
		sub1->current_state=SR_W_TRANSFER;
		sub1->current_time=ssd->current_time;
		sub1->next_state=SR_COMPLETE;  //�����ʱ����������������Ĵ�����ʱ�䣬��tWC������tPROG�����Ǻ�������ڼ���оƬ���¸�״̬��Ԥ��ʱ���õ���tPROG�������sub->sizeָ������������
		sub1->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+(sub1->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub1->complete_time=sub1->next_state_predict_time;

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=sub1->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
	}//if (old_ppn%2==ppn%2)
	else
	{
		sub1->current_state=SR_W_TRANSFER;
		sub1->current_time=ssd->current_time;
		sub1->next_state=SR_COMPLETE;   //�����̰�Ĳ��ԣ���żУ�鲻ͨ����Ǩ�Ƶ���һ��plane�����������ߴ���ʱ�䣨twc��
		sub1->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR+2*(ssd->parameter->subpage_page*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
		sub1->complete_time=sub1->next_state_predict_time;

		ssd->channel_head[channel].current_state=CHANNEL_TRANSFER;										
		ssd->channel_head[channel].current_time=ssd->current_time;										
		ssd->channel_head[channel].next_state=CHANNEL_IDLE;										
		ssd->channel_head[channel].next_state_predict_time=sub1->complete_time;

		ssd->channel_head[channel].chip_head[chip].current_state=CHIP_WRITE_BUSY;										
		ssd->channel_head[channel].chip_head[chip].current_time=ssd->current_time;									
		ssd->channel_head[channel].chip_head[chip].next_state=CHIP_IDLE;										
		ssd->channel_head[channel].chip_head[chip].next_state_predict_time=ssd->channel_head[channel].next_state_predict_time+ssd->parameter->time_characteristics.tPROG;
	}//else

	delete_from_channel(ssd,channel,sub1);

	return ssd;
}


/****************************************************************************************
*�����Ĺ������ڴ����������ĸ߼�����ʱ����Ҫ����one_page��ƥ�������һ��page��two_page
*û���ҵ����Ժ�one_pageִ��two plane����interleave������ҳ,��Ҫ��one_page�����һ���ڵ�
*****************************************************************************************/
struct sub_request *find_interleave_twoplane_page(struct ssd_info *ssd, struct sub_request *one_page,unsigned int command)
{
	struct sub_request *two_page;

	if (one_page->current_state!=SR_WAIT)
	{
		return NULL;                                                            
	}
	if (((ssd->channel_head[one_page->location->channel].chip_head[one_page->location->chip].current_state==CHIP_IDLE)||((ssd->channel_head[one_page->location->channel].chip_head[one_page->location->chip].next_state==CHIP_IDLE)&&
		(ssd->channel_head[one_page->location->channel].chip_head[one_page->location->chip].next_state_predict_time<=ssd->current_time))))
	{
		two_page=one_page->next_node;
		if(command==TWO_PLANE)
		{
			while (two_page!=NULL)
		    {
				if (two_page->current_state!=SR_WAIT)
				{
					two_page=two_page->next_node;
				}
				else if ((one_page->location->chip==two_page->location->chip)&&(one_page->location->die==two_page->location->die)&&(one_page->location->block==two_page->location->block)&&(one_page->location->page==two_page->location->page))
				{
					if (one_page->location->plane!=two_page->location->plane)
					{
						return two_page;                                                       /*�ҵ�����one_page����ִ��two plane������ҳ*/
					}
					else
					{
						two_page=two_page->next_node;
					}
				}
				else
				{
					two_page=two_page->next_node;
				}
		     }//while (two_page!=NULL)
		    if (two_page==NULL)                                                               /*û���ҵ����Ժ�one_pageִ��two_plane������ҳ,��Ҫ��one_page�����һ���ڵ�*/
		    {
				return NULL;
			}
		}//if(command==TWO_PLANE)
		else if(command==INTERLEAVE)
		{
			while (two_page!=NULL)
		    {
				if (two_page->current_state!=SR_WAIT)
				{
					two_page=two_page->next_node;
				}
				else if ((one_page->location->chip==two_page->location->chip)&&(one_page->location->die!=two_page->location->die))
				{
					return two_page;                                                           /*�ҵ�����one_page����ִ��interleave������ҳ*/
				}
				else
				{
					two_page=two_page->next_node;
				}
		     }
		    if (two_page==NULL)                                                                /*û���ҵ����Ժ�one_pageִ��interleave������ҳ,��Ҫ��one_page�����һ���ڵ�*/
		    {
				return NULL;
			}//while (two_page!=NULL)
		}//else if(command==INTERLEAVE)
		
	} 
	{
		return NULL;
	}
}


/*************************************************************************
*�ڴ����������߼�����ʱ������������ǲ��ҿ���ִ�и߼������sub_request
**************************************************************************/
int find_interleave_twoplane_sub_request(struct ssd_info * ssd, unsigned int channel,struct sub_request * sub_request_one,struct sub_request * sub_request_two,unsigned int command)
{
	sub_request_one=ssd->channel_head[channel].subs_r_head;
	while (sub_request_one!=NULL)
	{
		sub_request_two=find_interleave_twoplane_page(ssd,sub_request_one,command);                /*�ҳ�����������two_plane����interleave��read�����󣬰���λ��������ʱ������*/
		if (sub_request_two==NULL)
		{
			sub_request_one=sub_request_one->next_node;
		}
		else if (sub_request_two!=NULL)                                                            /*�ҵ�����������ִ��two plane������ҳ*/
		{
			break;
		}
	}

	if (sub_request_two!=NULL)
	{
		if (ssd->request_queue!=ssd->request_tail)      
		{                                                                                         /*ȷ��interleave read���������ǵ�һ�������������*/
			if ((ssd->request_queue->lsn-ssd->parameter->subpage_page)<(sub_request_one->lpn*ssd->parameter->subpage_page))  
			{
				if ((ssd->request_queue->lsn+ssd->request_queue->size+ssd->parameter->subpage_page)>(sub_request_one->lpn*ssd->parameter->subpage_page))
				{
				}
				else
				{
					sub_request_two=NULL;
				}
			}
			else
			{
				sub_request_two=NULL;
			}
		}//if (ssd->request_queue!=ssd->request_tail) 
	}//if (sub_request_two!=NULL)

	if(sub_request_two!=NULL)
	{
		return SUCCESS;
	}
	else
	{
		return FAILURE;
	}

}


/**************************************************************************
*��������ǳ���Ҫ�����������״̬ת�䣬�Լ�ʱ��ļ��㶼ͨ���������������
*����д�������ִ����ͨ����ʱ��״̬���Լ�ʱ��ļ���Ҳ��ͨ����������������
****************************************************************************/
Status go_one_step(struct ssd_info * ssd, struct sub_request * sub1,struct sub_request *sub2, unsigned int aim_state,unsigned int command)
{
	unsigned int i=0,j=0,k=0,m=0;
	long long time=0;
	struct sub_request * sub=NULL ; 
	struct sub_request * sub_twoplane_one=NULL, * sub_twoplane_two=NULL;
	struct sub_request * sub_interleave_one=NULL, * sub_interleave_two=NULL;
	struct local * location=NULL;
	if(sub1==NULL)
	{
		return ERROR;
	}
	
	/***************************************************************************************************
	*������ͨ����ʱ�����������Ŀ��״̬��Ϊ���¼������SR_R_READ��SR_R_C_A_TRANSFER��SR_R_DATA_TRANSFER
	*д�������Ŀ��״ֻ̬��SR_W_TRANSFER
	****************************************************************************************************/
	if(command==NORMAL)
	{
		sub=sub1;
		location=sub1->location;
		switch(aim_state)						
		{	
			case SR_R_READ:
			{   
				/*****************************************************************************************************
			    *���Ŀ��״̬��ָflash���ڶ����ݵ�״̬��sub����һ״̬��Ӧ���Ǵ�������SR_R_DATA_TRANSFER
			    *��ʱ��channel�޹أ�ֻ��chip�й�����Ҫ�޸�chip��״̬ΪCHIP_READ_BUSY����һ��״̬����CHIP_DATA_TRANSFER
			    ******************************************************************************************************/
				sub->current_time=ssd->current_time;
				sub->current_state=SR_R_READ;
				sub->next_state=SR_R_DATA_TRANSFER;
				sub->next_state_predict_time=ssd->current_time+ssd->parameter->time_characteristics.tR;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_READ_BUSY;
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_DATA_TRANSFER;
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=ssd->current_time+ssd->parameter->time_characteristics.tR;

				break;
			}
			case SR_R_C_A_TRANSFER:
			{   
				/*******************************************************************************************************
				*Ŀ��״̬�������ַ����ʱ��sub����һ��״̬����SR_R_READ
				*���״̬��channel��chip�йأ�����Ҫ�޸�channel��chip��״̬�ֱ�ΪCHANNEL_C_A_TRANSFER��CHIP_C_A_TRANSFER
				*��һ״̬�ֱ�ΪCHANNEL_IDLE��CHIP_READ_BUSY
				*******************************************************************************************************/
				sub->current_time=ssd->current_time;									
				sub->current_state=SR_R_C_A_TRANSFER;									
				sub->next_state=SR_R_READ;									
				sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC;									
				sub->begin_time=ssd->current_time;

				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn=sub->ppn;//��PPN������ַ�Ĵ���
				ssd->read_count++;

				ssd->channel_head[location->channel].current_state=CHANNEL_C_A_TRANSFER;									
				ssd->channel_head[location->channel].current_time=ssd->current_time;										
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;								
				ssd->channel_head[location->channel].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_C_A_TRANSFER;								
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;						
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_READ_BUSY;							
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC;
				
				break;
			
			}
			case SR_R_DATA_TRANSFER:
			{   
				/**************************************************************************************************************
				*Ŀ��״̬�����ݴ���ʱ��sub����һ��״̬�������״̬SR_COMPLETE
				*���״̬�Ĵ���Ҳ��channel��chip�йأ�����channel��chip�ĵ�ǰ״̬��ΪCHANNEL_DATA_TRANSFER��CHIP_DATA_TRANSFER
				*��һ��״̬�ֱ�ΪCHANNEL_IDLE��CHIP_IDLE��
				***************************************************************************************************************/
				sub->current_time=ssd->current_time;					
				sub->current_state=SR_R_DATA_TRANSFER;		
				sub->next_state=SR_COMPLETE;				
				sub->next_state_predict_time=ssd->current_time+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC;			
				sub->complete_time=sub->next_state_predict_time;

				ssd->channel_head[location->channel].current_state=CHANNEL_DATA_TRANSFER;		
				ssd->channel_head[location->channel].current_time=ssd->current_time;		
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;	
				ssd->channel_head[location->channel].next_state_predict_time=sub->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_DATA_TRANSFER;				
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_IDLE;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=sub->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn=-1;  //��ռĴ���

				break;
			}
			case SR_R_GC_SUB:
			{   
				/**************************************************************************************************************
				*Ŀ��״̬�����ݴ���ʱ��sub����һ��״̬�������״̬SR_COMPLETE
				*���ﴦ�� soft_gc�Ķ��������������� SR_R_DATA_TRANSFER һ�£�����ʱ�䲻ͬ
				*���״̬�Ĵ���Ҳ��channel��chip�йأ�����channel��chip�ĵ�ǰ״̬��ΪCHANNEL_DATA_TRANSFER��CHIP_DATA_TRANSFER
				*��һ��״̬�ֱ�ΪCHANNEL_IDLE��CHIP_IDLE��
				***************************************************************************************************************/
				sub->current_time=ssd->current_time;					
				sub->current_state=SR_R_DATA_TRANSFER;		
				sub->next_state=SR_COMPLETE;				
				//sub->next_state_predict_time=ssd->current_time+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC;
				sub->next_state_predict_time=ssd->current_time+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC+7*ssd->parameter->time_characteristics.tWC+ssd->parameter->time_characteristics.tR;			
				sub->complete_time=sub->next_state_predict_time;

				ssd->channel_head[location->channel].current_state=CHANNEL_DATA_TRANSFER;		
				ssd->channel_head[location->channel].current_time=ssd->current_time;		
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;	
				ssd->channel_head[location->channel].next_state_predict_time=sub->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_DATA_TRANSFER;				
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_IDLE;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=sub->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn=-1;  //��ռĴ���

				break;
			}
			case SR_W_TRANSFER:
			{
				/******************************************************************************************************
				*���Ǵ���д������ʱ��״̬��ת���Լ�ʱ��ļ���
				*��Ȼд������Ĵ���״̬Ҳ�����������ô�࣬����д�����Ǵ�����plane�д�������
				*�����Ϳ��԰Ѽ���״̬��һ��״̬�������͵���SR_W_TRANSFER���״̬������sub����һ��״̬�������״̬��
				*��ʱchannel��chip�ĵ�ǰ״̬��ΪCHANNEL_TRANSFER��CHIP_WRITE_BUSY
				*��һ��״̬��ΪCHANNEL_IDLE��CHIP_IDLE
				*******************************************************************************************************/
				sub->begin_time = ssd->current_time;
				sub->current_time=ssd->current_time;
				sub->current_state=SR_W_TRANSFER;
				sub->next_state=SR_COMPLETE;        //д���������������ݴ����ʱ��������һ����Ϊ������״̬�������Ҷ�����ͬ
				sub->next_state_predict_time=ssd->current_time+7*ssd->parameter->time_characteristics.tWC+(sub->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tWC;
				sub->complete_time=sub->next_state_predict_time;		
				time=sub->complete_time;
				
				ssd->channel_head[location->channel].current_state=CHANNEL_TRANSFER;										
				ssd->channel_head[location->channel].current_time=ssd->current_time;										
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;										
				ssd->channel_head[location->channel].next_state_predict_time=time;   //���ݴ�����ɺ�ͨ��״̬��Ϊ����

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_WRITE_BUSY;										
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;									
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_IDLE;										
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=time+ssd->parameter->time_characteristics.tPROG;//����д����ɺ�оƬ��Ϊ����
				
				break;
			}
			default :  return ERROR;
			
		}//switch(aim_state)	
	}//if(command==NORMAL)
	else if(command==TWO_PLANE)
	{   
		/**********************************************************************************************
		*�߼�����TWO_PLANE�Ĵ��������TWO_PLANE�߼������Ƕ�������ĸ߼�����
		*״̬ת������ͨ����һ������ͬ������SR_R_C_A_TRANSFERʱ����ʱ���Ǵ��еģ���Ϊ����һ��ͨ��channel
		*����SR_R_DATA_TRANSFERҲ�ǹ���һ��ͨ��
		**********************************************************************************************/
		if((sub1==NULL)||(sub2==NULL))
		{
			return ERROR;
		}
		sub_twoplane_one=sub1;
		sub_twoplane_two=sub2;
		location=sub1->location;
		
		switch(aim_state)						
		{	
			case SR_R_C_A_TRANSFER:
			{
				sub_twoplane_one->current_time=ssd->current_time;									
				sub_twoplane_one->current_state=SR_R_C_A_TRANSFER;									
				sub_twoplane_one->next_state=SR_R_READ;									
				sub_twoplane_one->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;									
				sub_twoplane_one->begin_time=ssd->current_time;

				ssd->channel_head[sub_twoplane_one->location->channel].chip_head[sub_twoplane_one->location->chip].die_head[sub_twoplane_one->location->die].plane_head[sub_twoplane_one->location->plane].add_reg_ppn=sub_twoplane_one->ppn;
				ssd->read_count++;

				sub_twoplane_two->current_time=ssd->current_time;									
				sub_twoplane_two->current_state=SR_R_C_A_TRANSFER;									
				sub_twoplane_two->next_state=SR_R_READ;									
				sub_twoplane_two->next_state_predict_time=sub_twoplane_one->next_state_predict_time;									
				sub_twoplane_two->begin_time=ssd->current_time;

				ssd->channel_head[sub_twoplane_two->location->channel].chip_head[sub_twoplane_two->location->chip].die_head[sub_twoplane_two->location->die].plane_head[sub_twoplane_two->location->plane].add_reg_ppn=sub_twoplane_two->ppn;
				ssd->read_count++;
				ssd->m_plane_read_count++;

				ssd->channel_head[location->channel].current_state=CHANNEL_C_A_TRANSFER;									
				ssd->channel_head[location->channel].current_time=ssd->current_time;										
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;								
				ssd->channel_head[location->channel].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_C_A_TRANSFER;								
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;						
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_READ_BUSY;							
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;

				
				break;
			}
			case SR_R_DATA_TRANSFER:
			{
				sub_twoplane_one->current_time=ssd->current_time;					
				sub_twoplane_one->current_state=SR_R_DATA_TRANSFER;		
				sub_twoplane_one->next_state=SR_COMPLETE;				
				sub_twoplane_one->next_state_predict_time=ssd->current_time+(sub_twoplane_one->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC;			
				sub_twoplane_one->complete_time=sub_twoplane_one->next_state_predict_time;
				
				sub_twoplane_two->current_time=sub_twoplane_one->next_state_predict_time;					
				sub_twoplane_two->current_state=SR_R_DATA_TRANSFER;		
				sub_twoplane_two->next_state=SR_COMPLETE;				
				sub_twoplane_two->next_state_predict_time=sub_twoplane_two->current_time+(sub_twoplane_two->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC;			
				sub_twoplane_two->complete_time=sub_twoplane_two->next_state_predict_time;
				
				ssd->channel_head[location->channel].current_state=CHANNEL_DATA_TRANSFER;		
				ssd->channel_head[location->channel].current_time=ssd->current_time;		
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;	
				ssd->channel_head[location->channel].next_state_predict_time=sub_twoplane_one->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_DATA_TRANSFER;				
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_IDLE;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=sub_twoplane_one->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn=-1;
			
				break;
			}
			default :  return ERROR;
		}//switch(aim_state)	
	}//else if(command==TWO_PLANE)
	else if(command==INTERLEAVE)
	{
		if((sub1==NULL)||(sub2==NULL))
		{
			return ERROR;
		}
		sub_interleave_one=sub1;
		sub_interleave_two=sub2;
		location=sub1->location;
		
		switch(aim_state)						
		{	
			case SR_R_C_A_TRANSFER:
			{
				sub_interleave_one->current_time=ssd->current_time;									
				sub_interleave_one->current_state=SR_R_C_A_TRANSFER;									
				sub_interleave_one->next_state=SR_R_READ;									
				sub_interleave_one->next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;									
				sub_interleave_one->begin_time=ssd->current_time;

				ssd->channel_head[sub_interleave_one->location->channel].chip_head[sub_interleave_one->location->chip].die_head[sub_interleave_one->location->die].plane_head[sub_interleave_one->location->plane].add_reg_ppn=sub_interleave_one->ppn;
				ssd->read_count++;

				sub_interleave_two->current_time=ssd->current_time;									
				sub_interleave_two->current_state=SR_R_C_A_TRANSFER;									
				sub_interleave_two->next_state=SR_R_READ;									
				sub_interleave_two->next_state_predict_time=sub_interleave_one->next_state_predict_time;									
				sub_interleave_two->begin_time=ssd->current_time;

				ssd->channel_head[sub_interleave_two->location->channel].chip_head[sub_interleave_two->location->chip].die_head[sub_interleave_two->location->die].plane_head[sub_interleave_two->location->plane].add_reg_ppn=sub_interleave_two->ppn;
				ssd->read_count++;
				ssd->interleave_read_count++;

				ssd->channel_head[location->channel].current_state=CHANNEL_C_A_TRANSFER;									
				ssd->channel_head[location->channel].current_time=ssd->current_time;										
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;								
				ssd->channel_head[location->channel].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_C_A_TRANSFER;								
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;						
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_READ_BUSY;							
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=ssd->current_time+14*ssd->parameter->time_characteristics.tWC;
				
				break;
						
			}
			case SR_R_DATA_TRANSFER:
			{
				sub_interleave_one->current_time=ssd->current_time;					
				sub_interleave_one->current_state=SR_R_DATA_TRANSFER;		
				sub_interleave_one->next_state=SR_COMPLETE;				
				sub_interleave_one->next_state_predict_time=ssd->current_time+(sub_interleave_one->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC;			
				sub_interleave_one->complete_time=sub_interleave_one->next_state_predict_time;
				
				sub_interleave_two->current_time=sub_interleave_one->next_state_predict_time;					
				sub_interleave_two->current_state=SR_R_DATA_TRANSFER;		
				sub_interleave_two->next_state=SR_COMPLETE;				
				sub_interleave_two->next_state_predict_time=sub_interleave_two->current_time+(sub_interleave_two->size*ssd->parameter->subpage_capacity)*ssd->parameter->time_characteristics.tRC;			
				sub_interleave_two->complete_time=sub_interleave_two->next_state_predict_time;

				ssd->channel_head[location->channel].current_state=CHANNEL_DATA_TRANSFER;		
				ssd->channel_head[location->channel].current_time=ssd->current_time;		
				ssd->channel_head[location->channel].next_state=CHANNEL_IDLE;	
				ssd->channel_head[location->channel].next_state_predict_time=sub_interleave_two->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].current_state=CHIP_DATA_TRANSFER;				
				ssd->channel_head[location->channel].chip_head[location->chip].current_time=ssd->current_time;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state=CHIP_IDLE;			
				ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time=sub_interleave_two->next_state_predict_time;

				ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn=-1;
				
				break;
			}
			default :  return ERROR;	
		}//switch(aim_state)				
	}//else if(command==INTERLEAVE)
	else
	{
		printf("\nERROR: Unexpected command !\n" );
		return ERROR;
	}

	return SUCCESS;
}

