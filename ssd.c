/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName�� ssd.c
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

 

#include "ssd.h"

/********************************************************************************************************************************
1��main������initiatio()����������ʼ��ssd,��2��make_aged()����ʹSSD��Ϊaged��aged��ssd�൱��ʹ�ù�һ��ʱ���ssd��������ʧЧҳ��
non_aged��ssd���µ�ssd����ʧЧҳ��ʧЧҳ�ı��������ڳ�ʼ�����������ã�3��pre_process_page()������ǰɨһ�������?�Ѷ�����
��lpn<--->ppnӳ���ϵ���Ƚ����ã�д�����lpn<--->ppnӳ���ϵ��д��ʱ���ٽ�����Ԥ����trace��ֹ�������Ƕ��������ݣ�4��simulate()��
���Ĵ���������trace�ļ��Ӷ�������������ɶ��������������ɣ�?5��statistic_output()������ssd�ṹ�е���Ϣ���������ļ����������?
ͳ�����ݺ�ƽ�����ݣ�����ļ���С��trace_output�ļ���ܴ����ϸ��6��free_all_node()�����ͷ�����main����������Ľڵ�?
*********************************************************************************************************************************/
int  main()
{
	unsigned  int i,ii,j,k;
	struct ssd_info *ssd;

	#ifdef DEBUG
	printf("enter main\n"); 
	#endif

	ssd=(struct ssd_info*)malloc(sizeof(struct ssd_info));
	alloc_assert(ssd,"ssd");
	memset(ssd,0, sizeof(struct ssd_info));

	ssd=initiation(ssd);
	make_aged(ssd);
	pre_process_page(ssd);

	//KXC:allocate memory for vector
	vector=(int*)malloc(sizeof(int)*ssd->parameter->channel_number);
	alloc_assert(vector,"vector");
	memset(vector,0,sizeof(int)*ssd->parameter->channel_number);

/************KXC:�޸����ʹ������߼� 2019.8.13**************/
/* 	for (i=0;i<ssd->parameter->channel_number;i++)//����Ļ�����ʼ��о�?�Ŀհ�ҳ��Ϣ
	{
      for (ii=0;ii<ssd->parameter->chip_channel[i];ii++)
       {
         for (j=0;j<ssd->parameter->die_chip;j++)
		   {
			 for (k=0;k<ssd->parameter->plane_die;k++)
			 {
			  	printf("%d channel,%d chip,%d die,%d plane has free page num:  %5d\n",i,ii,j,k,ssd->channel_head[i].chip_head[ii].die_head[j].plane_head[k].free_page);
			 }
            }
  	    }
	} */
	fprintf(ssd->outputfile,"\t\t\t\t\t\t\t\t\tOUTPUT\n");
	fprintf(ssd->outputfile,"****************** TRACE INFO ******************\n");

	ssd=simulate(ssd);                         //ִ�з���
	statistic_output(ssd);                     //���ؾ��������������ļ�
/*	free_all_node(ssd);*/

	printf("\n");
	printf("the simulation is completed!\n");
	
	return 1;
/* 	_CrtDumpMemoryLeaks(); */
}


/******************simulate() *********************************************************************
*simulate()�Ǻ��Ĵ�����������Ҫʵ�ֵĹ��ܰ���
*1,��trace�ļ��л�ȡһ�����󣬹ҵ�ssd->request
*2������ssd�Ƿ���dram�ֱ��������������󣬰���Щ��������Ϊ��д�����󣬹ҵ�ssd->channel����ssd��
*3�������¼����Ⱥ���������Щ��д������
*4�����ÿ������������󶼴�������������?��outputfile�ļ���
**************************************************************************************************/
struct ssd_info *simulate(struct ssd_info *ssd)
{
	int flag=1,flag1=0;
	double output_step=0;
	unsigned int a=0,b=0;
	//errno_t err;
	int i=0,pflag=0;
	printf("\n");
	printf("begin simulating.......................\n");
	printf("\n");
	printf("\n");
	//printf("   ^o^    OK, please wait a moment, and enjoy music and coffee   ^o^    \n");

	ssd->tracefile = fopen(ssd->tracefilename,"r");
	if(ssd->tracefile == NULL)
	{  
		printf("the trace file can't open\n");
		return NULL;
	}

	fprintf(ssd->outputfile,"      arrive           lsn     size ope     begin time    response time    process time    wait time\n");	
	fflush(ssd->outputfile);

	while(flag!=100)      
	{
		while(ssd->request_queue_length<ssd->parameter->queue_length)
		{
			flag=get_requests(ssd);

			if(flag==0)
			{
				break;
			}
			if(ssd->next_request_time>ssd->current_time)
			{
				break;
			}
		}
		
		if(ssd->parameter->scheduling_algorithm==1)
		{
			if(ssd->parameter->allocation_scheme==1&&ssd->parameter->static_allocation==1)
			{
				schedule_PIQ(ssd);
			}
			
			if(ssd->parameter->allocation_scheme==0&&ssd->parameter->dynamic_allocation==0)
			{
				schedule_ours(ssd);
			}
		}
		
		dependency(ssd);
		//KXC:here just modify the function no_buffer_distribute so there is no buffer
		if(ssd->parameter->dram_capacity==0)
		{
			if(ssd->parameter->scheduling_algorithm==1)
			{
				no_buffer_distribute_sch(ssd);
			}
			else
			{
				no_buffer_distribute_sch(ssd);
			}
			
		}
		process(ssd);                                      //ִ�д�������
		trace_output(ssd);
		if(flag == 0 && ssd->request_queue == NULL)        //����ִ����ɣ�����?
			flag = 100;
	}

	fclose(ssd->tracefile);
	return ssd;
}



/********    get_request    ******************************************************
*	1.get requests that arrived already
*	2.add those request node to ssd->reuqest_queue
*	return	0: reach the end of the trace
*			-1: no request has been added
*			1: add one request to list
*SSDģ����������������ʽ:ʱ������(��ȷ��̫��) �¼�����(���������?) trace����()��
*���ַ�ʽ�ƽ��¼���channel/chip״̬�ı䡢trace�ļ�����ﵽ��?
*channel/chip״̬�ı��trace�ļ����󵽴���ɢ����ʱ�����ϵĵ㣬ÿ�δӵ�ǰ״̬����
*��һ��״̬��Ҫ���������һ��״�?��ÿ����һ����ִ��һ��process
********************************************************************************/
int get_requests(struct ssd_info *ssd)  
{  
	char buffer[200];
	unsigned int lsn=0;
	int device,  size, ope,large_lsn, i = 0,j=0;
	struct request *request1;
	int flag = 1;
	long filepoint; 
	int64_t time_t = 0;
	//int64_t nearest_event_time;    

	#ifdef DEBUG
	printf("enter get_requests,  current time:%lld\n",ssd->current_time);
	#endif

	if(feof(ssd->tracefile))
	{
		//ssd->current_time=find_nearest_event(ssd);
		return 0;
	}

	//KXC:if current time less than next request's arriving time, it should not be gotten
	if(ssd->empty==0)
	{
		if(ssd->next_request_time>ssd->current_time)
		{
			return -1;
		}
	}
	
	if (ssd->request_queue_length>=ssd->parameter->queue_length)
	{
		return -1;
	}
	
	filepoint = ftell(ssd->tracefile);	                  //ftell�����ļ���ǰָ�룬Ҳ�����ļ�λ��
	fgets(buffer, 200, ssd->tracefile);                   //�Ӹ����ļ��ж�ȡ
	sscanf(buffer,"%lld %d %d %d %d",&time_t,&device,&lsn,&size,&ope);   //����������
    
	if ((device<0)&&(lsn<0)&&(size<0)&&(ope<0))
	{
		return 100;
	}
	if (lsn<ssd->min_lsn) 
		ssd->min_lsn=lsn;
	if (lsn>ssd->max_lsn)
		ssd->max_lsn=lsn;
	/******************************************************************************************************
	*�ϲ��ļ�ϵͳ���͸�SSD���κζ�д��������������֣�LSN��size�� LSN���߼������ţ������ļ�ϵͳ���ԣ����������Ĵ�
	*���ռ���һ�����Ե������ռ䡣���磬������260��6����ʾ������Ҫ��ȡ��������Ϊ260���߼�������ʼ���ܹ�6��������
	*large_lsn: channel�����ж��ٸ�subpage�������ٸ�sector��overprovideϵ����SSD�в��������еĿռ䶼���Ը��û�ʹ�ã�
	*����32G��SSD������10%�Ŀռ䱣�������������ã����Գ���1-provide
	***********************************************************************************************************/
	large_lsn=(int)((ssd->parameter->subpage_page*ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*ssd->parameter->chip_num)*(1-ssd->parameter->overprovide));
	lsn = lsn%large_lsn;



	if(time_t < 0)
	{
		printf("error!\n");
		while(1){}
	}

	if(feof(ssd->tracefile))
	{
		request1=NULL;
		return 0;
	}

	request1 = (struct request*)malloc(sizeof(struct request));
	alloc_assert(request1,"request");
	memset(request1,0, sizeof(struct request));

	ssd->current_request_time=time_t;
	request1->time = time_t;
	request1->lsn = lsn;
	request1->size = size;
	request1->operation = ope;	
	request1->begin_time = time_t;
	request1->response_time = 0;	
	request1->energy_consumption = 0;	
	request1->next_node = NULL;
	request1->distri_flag = 0;              // indicate whether this request has been distributed already
	request1->subs = NULL;
	request1->need_distr_flag = NULL;
	request1->complete_lsn_count=0;         //record the count of lsn served by buffer
	filepoint = ftell(ssd->tracefile);		// set the file point

	if(ssd->request_queue == NULL)          //The queue is empty
	{
		ssd->request_queue = request1;
		ssd->request_tail = request1;
		ssd->request_queue_length++;
	}
	else
	{			
		(ssd->request_tail)->next_node = request1;	
		ssd->request_tail = request1;			
		ssd->request_queue_length++;
	}

	if (request1->operation==1)             //����ƽ��������? 1Ϊ�� 0Ϊд
	{
		ssd->ave_read_size=(ssd->ave_read_size*ssd->read_request_count+request1->size)/(ssd->read_request_count+1);
	} 
	else
	{
		ssd->ave_write_size=(ssd->ave_write_size*ssd->write_request_count+request1->size)/(ssd->write_request_count+1);
	}

	
	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile);    //Ѱ����һ������ĵ���ʱ��?
	sscanf(buffer,"%lld %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
	ssd->next_request_time=time_t;
	fseek(ssd->tracefile,filepoint,0);
	//ssd->empty=0;                     //KXC:the request queue is not empty
	if(ssd->parameter->scheduling_algorithm==1)
	{
		if(ssd->current_request_time==ssd->next_request_time)
		{
			ssd->empty=1;
		}
		else
		{
			ssd->empty=0;
		}
	}
	else
	{
		ssd->empty=0;
	}
	
	return 1;
}


//KXC: the schedule function to schedule the request in the queue
struct ssd_info *schedule_PIQ(struct ssd_info *ssd)
{
	struct request *temp=NULL;
	struct request *temp_tail=NULL;
	struct request *temp1=NULL;
	struct request *temp1_tail=NULL;
	struct request *temp2=NULL;
	struct request *temp2_tail=NULL;
	struct request *write=NULL;
	struct request *write_tail=NULL;
	struct request *read=NULL;
	struct request *read_tail=NULL;
	struct request *overtime=NULL;
	struct request *overtime_tail=NULL;
	struct request *conflict=NULL;
	struct request *conflict_tail=NULL;

	unsigned int first_lpn,last_lpn;
	int i,j,flag;
	int channel,chip;
	int conflict_flag=1;
	int schetwo;       //1-read schedule and write schedule;0-only one type request schedule
	int no_sche;       //KXC:to indicate the request can not shcedule 

	//int *channel;
	//int *chip;

	if(ssd->request_queue==NULL)    //no need to schedule
		return 0;

	//to find the tail of the distributed requests
	temp1=ssd->request_queue;
	while(temp1!=NULL)
	{
		if(temp1->dis==1)
		{
			if(temp1->next_node!=NULL)
			{
				//temp1=temp1->next_node;
				if(temp1->next_node->dis==1)
				{
					temp1=temp1->next_node;
				}
				else
				{
					temp1_tail=temp1;
					//temp1_tail->next_node=NULL;
					ssd->request_tail=temp1_tail;
					break;
				}
			}
			else
			{
				return 0;    //all the requests have been distributed, no need to schedule
			}
		}
		else
		{
			temp1_tail=NULL;
			ssd->request_tail=ssd->request_queue;
			break;
		}

	}
	//seperate the read and write requests first, divided into 3parts
	//1.scheduled requests;2.overtime requests;3.can be scheduled read and write request
	temp=ssd->request_tail;
	while(temp!=NULL)
	{
		if(temp->dis==1)
		{
			temp=temp->next_node;
		}
		else
		{		
			if(temp->operation==WRITE)
			{
				if(write==NULL)
				{
					write=temp;
					write_tail=temp;
					//write_tail->next_node=NULL;
				}
				else
				{
					write_tail->next_node=temp;
					write_tail=temp;
					//write_tail->next_node=NULL;
				}	
			}
			else
			{
				if(read==NULL)
				{
					read=temp;
					read_tail=temp;
					//read_tail->next_node=NULL;
				}
				else
				{
					read_tail->next_node=temp;
					read_tail=temp;
					//read_tail->next_node=NULL;
				}
			}
			
			temp=temp->next_node;
		}	
	}
	
	if(write!=NULL)
	{
		write_tail->next_node=NULL;
	}
	if(read!=NULL)
	{
		read_tail->next_node=NULL;
	}

	

	if(read==NULL&&write==NULL)
	{
		return 0;
	}

	//the read and write requests have been seperated
	
	//read_tail->next_node=write;
	//overtime_tail->next_node=read;
	//temp=overtime;

	
	/* if(read!=NULL)
	{
		temp2=read;
		temp2_tail=read_tail;
		if(write!=NULL)
		{
			temp2_tail->next_node=write;
			//temp2_tail=write_tail;
		}
	}
	else
	{
		if(write!=NULL)
		{
			temp2=write;
			temp2_tail=write_tail;
		}
	} */

	/* 	if(temp2==NULL)
		{
			return 0;
		}
 	*/
	
	//here the allocaton of PIQ is CWDP, allocation_scheme=1,static_allocation=1
	//
	//if(ssd->parameter->allocation_scheme==1&&ssd->parameter->static_allocation==1)    //here the allocaton of PIQ is CWDP, allocation_scheme=1,static_allocation=1
	//{
	if(read!=NULL)
	{
		temp=read;
		if(write!=NULL)
		{
			schetwo=1;
		}
	}
	else
	{
		temp=write;
		schetwo=0;
	}
	temp2=NULL;               //to recorded the no conflict request
	temp2_tail=NULL;
	
	while(1)
	{
		//conflict_flag=0;
		while(temp!=NULL)
		{
			no_sche=0;
			if(temp->dis==0)
			{
				//temp->sch=0;
				last_lpn=(temp->lsn+temp->size-1)/ssd->parameter->subpage_page;
				first_lpn=temp->lsn/ssd->parameter->subpage_page;
				flag=0;
				for(i=first_lpn;i<=last_lpn; i++)
				{
					channel=i%ssd->parameter->channel_number;
					chip=(i/ssd->parameter->channel_number)%ssd->parameter->chip_channel[0];
					if((int)(vector[channel]&chip)==0)                //KXC:no conflict
					{
						continue;
					}
					else
					{
						flag=1;
						break;
					}	
				}
				if(flag==0)       //no conflict
				{
					//to update the vector
					//conflict_flag=0;
					for(i=first_lpn;i<=last_lpn; i++)
					{
						channel=i%ssd->parameter->channel_number;
						chip=(i/ssd->parameter->channel_number)%ssd->parameter->chip_channel[0];
						vector[channel]=vector[channel]|chip;
					}

					//record temp2
					if(temp2==NULL)
					{
						temp2=temp;
						temp2_tail=temp;
						//temp2_tail->next_node=NULL;
					}
					else
					{
						temp2_tail->next_node=temp;
						temp2_tail=temp;
						//temp2_tail->next_node=NULL;
					}
					
				}
				else
				{
					//conflict_flag=1;
					if(conflict==NULL)
					{
						conflict=temp;
						conflict_tail=temp;
						//conflict_tail->next_node=NULL;
					}
					else
					{
						conflict_tail->next_node=temp;
						conflict_tail=temp;
						//conflict_tail->next_node=NULL;
					}

				}
				temp->sch=1;            //the request has been scheduled
				temp=temp->next_node;
				
			}
			else
			{
				printf("the distributed request is in the schedule process!\n");
				//no_sche=1;
				break;
			}
		}
		
		memset(vector,0,sizeof(int)*ssd->parameter->channel_number);
		
		if(conflict!=NULL)
		{
			conflict_tail->next_node=NULL;
			temp=conflict;
			conflict=NULL;
		}
		else
		{
			if(schetwo==1)
			{
				temp=write;
				schetwo=0;
			}
			else
			{
				break;
			}
			
		}
				
	}
	//}

	if(temp1_tail!=NULL)
	{
		ssd->request_tail->next_node=temp2;
		ssd->request_tail=temp2_tail;
		ssd->request_tail->next_node=NULL;
	}
	else
	{
		ssd->request_queue=temp2;
		ssd->request_tail=temp2_tail;
		ssd->request_tail->next_node=NULL;
	}	
}

//KXC: the schedule function to schedule the request in the queue
struct ssd_info *schedule_ours(struct ssd_info *ssd)
{
	struct request *temp=NULL;
	struct request *temp_tail=NULL;
	struct request *temp1=NULL;
	struct request *temp1_tail=NULL;
	struct request *temp2=NULL;
	struct request *temp2_tail=NULL;
	struct request *write=NULL;
	struct request *write_tail=NULL;
	struct request *read=NULL;
	struct request *read_tail=NULL;
	struct request *overtime=NULL;
	struct request *overtime_tail=NULL;
	struct request *conflict=NULL;
	struct request *conflict_tail=NULL;

	struct local * loc=NULL;

	unsigned int first_lpn,last_lpn;
	int i,j,flag;
	int channel,chip;
	int conflict_flag=1;
	int schetwo;       //1-read schedule and write schedule;0-only one type request schedule
	int no_sche;       //KXC:to indicate the request can not shcedule 

	//int *channel;
	//int *chip;

	if(ssd->request_queue==NULL)    //no need to schedule
		return 0;

	//to find the tail of the distributed requests
	temp1=ssd->request_queue;
	while(temp1!=NULL)
	{
		if(temp1->dis==1)
		{
			if(temp1->next_node!=NULL)
			{
				//temp1=temp1->next_node;
				if(temp1->next_node->dis==1)
				{
					temp1=temp1->next_node;
				}
				else
				{
					temp1_tail=temp1;
					//temp1_tail->next_node=NULL;
					ssd->request_tail=temp1_tail;
					break;
				}
			}
			else
			{
				return 0;    //all the requests have been distributed, no need to schedule
			}
		}
		else
		{
			temp1_tail=NULL;
			ssd->request_tail=ssd->request_queue;
			break;
		}

	}
	//seperate the read and write requests first, divided into 3parts
	//1.scheduled requests;2.overtime requests;3.can be scheduled read and write request
	temp=ssd->request_tail;
	while(temp!=NULL)
	{
		if(temp->dis==1)
		{
			temp=temp->next_node;
		}
		else
		{		
			if(temp->operation==WRITE)
			{
				if(write==NULL)
				{
					write=temp;
					write_tail=temp;
					//write_tail->next_node=NULL;
				}
				else
				{
					write_tail->next_node=temp;
					write_tail=temp;
					//write_tail->next_node=NULL;
				}	
			}
			else
			{
				if(read==NULL)
				{
					read=temp;
					read_tail=temp;
					//read_tail->next_node=NULL;
				}
				else
				{
					read_tail->next_node=temp;
					read_tail=temp;
					//read_tail->next_node=NULL;
				}
			}
			
			temp=temp->next_node;
		}	
	}
	
	if(write!=NULL)
	{
		write_tail->next_node=NULL;
	}
	if(read!=NULL)
	{
		read_tail->next_node=NULL;
	}

	

	if(read==NULL&&write==NULL)
	{
		return 0;
	}

	//the read and write requests have been seperated
	
	//read_tail->next_node=write;
	//overtime_tail->next_node=read;
	//temp=overtime;

	
	/* if(read!=NULL)
	{
		temp2=read;
		temp2_tail=read_tail;
		if(write!=NULL)
		{
			temp2_tail->next_node=write;
			//temp2_tail=write_tail;
		}
	}
	else
	{
		if(write!=NULL)
		{
			temp2=write;
			temp2_tail=write_tail;
		}
	} */

	/* 	if(temp2==NULL)
		{
			return 0;
		}
 	*/
	
	//here the allocaton of PIQ is CWDP, allocation_scheme=1,static_allocation=1
	//
	temp=NULL;
	if(read!=NULL)
	{
		temp=read;
	}

	temp2=NULL;               //to recorded the no conflict request
	temp2_tail=NULL;
	
	while(1)
	{
		if(read==NULL)
		{
			break;
		}
		//conflict_flag=0;
		while(temp!=NULL)
		{
			no_sche=0;
			if(temp->dis==0)
			{
				//temp->sch=0;
				last_lpn=(temp->lsn+temp->size-1)/ssd->parameter->subpage_page;
				first_lpn=temp->lsn/ssd->parameter->subpage_page;
				flag=0;
				for(i=first_lpn;i<=last_lpn; i++)
				{
					loc = find_location(ssd,ssd->dram->map->map_entry[i].pn);
					channel=loc->channel;
					chip=loc->chip;
					if((int)(vector[channel]&chip)==0)                //KXC:no conflict
					{
						continue;
					}
					else
					{
						flag=1;
						break;
					}	
				}
				if(flag==0)       //no conflict
				{
					//to update the vector
					//conflict_flag=0;
					for(i=first_lpn;i<=last_lpn; i++)
					{
						loc = find_location(ssd,ssd->dram->map->map_entry[i].pn);
						channel=loc->channel;
						chip=loc->chip;
						vector[channel]=vector[channel]|chip;
					}

					//record temp2
					if(temp2==NULL)
					{
						temp2=temp;
						temp2_tail=temp;
						//temp2_tail->next_node=NULL;
					}
					else
					{
						temp2_tail->next_node=temp;
						temp2_tail=temp;
						//temp2_tail->next_node=NULL;
					}
					
				}
				else
				{
					//conflict_flag=1;
					if(conflict==NULL)
					{
						conflict=temp;
						conflict_tail=temp;
						//conflict_tail->next_node=NULL;
					}
					else
					{
						conflict_tail->next_node=temp;
						conflict_tail=temp;
						//conflict_tail->next_node=NULL;
					}

				}
				temp->sch=1;            //the request has been scheduled
				temp=temp->next_node;
				
			}
			else
			{
				printf("the distributed request is in the schedule process!\n");
				//no_sche=1;
				break;
			}
		}
		
		memset(vector,0,sizeof(int)*ssd->parameter->channel_number);
		
		if(conflict!=NULL)
		{
			conflict_tail->next_node=NULL;
			temp=conflict;
			conflict=NULL;
		}
		else
		{
			break;
		}
				
	}

	if(write!=NULL)
	{
		if(temp2_tail!=NULL)
		{
			temp2_tail->next_node=write;
			temp2_tail=write_tail;
			write_tail->next_node=NULL;
		}
		else
		{
			temp2=write;
			temp2_tail=write_tail;
			temp2_tail->next_node=NULL;
		}
	}

	if(temp1_tail!=NULL)
	{
		ssd->request_tail->next_node=temp2;
		ssd->request_tail=temp2_tail;
		ssd->request_tail->next_node=NULL;
	}
	else
	{
		ssd->request_queue=temp2;
		ssd->request_tail=temp2_tail;
		ssd->request_tail->next_node=NULL;
	}	
}

struct ssd_info *dependency(struct ssd_info *ssd)
{
	struct request *temp=NULL;
	struct request *temp_tail=NULL;
	struct request *temp1=NULL;
	struct request *temp1_tail=NULL;
	//int64_t time1 = 0;


	temp=ssd->request_queue;
	//temp_tail=ssd->request_tail;
	//to find the first of the non-distributed requests--temp_tail
	while(temp!=NULL)
	{
		if(temp->dis==1)
		{
			if(temp->next_node!=NULL)
			{
				if(temp->next_node->dis==1)
				{
					temp=temp->next_node;
				}
				else
				{
					temp_tail=temp->next_node;
					break;
				}
			}
			else
			{
				return 0;   
			}
		}
		else
		{
			temp_tail=temp;
			break;
		}

	}

	if(temp_tail==NULL)
	{
		return 0;
	}
	
	temp1=temp_tail;
	//KXC:to fine the waw and raw requsts
	while(temp1!=NULL)
	{
		if(temp1->cnt==1)
		{
			temp1=temp1->next_node;
		}
		else
		{	
			temp1_tail=temp1->next_node;
			while(temp1_tail!=NULL)
			{
				if(temp1_tail->operation==READ)
				{
					temp1_tail=temp1_tail->next_node;
				}
				else
				{
					if(temp1->lsn>temp1_tail->lsn)
					{
						if(temp1->lsn-temp1_tail->lsn<=temp1_tail->size&&temp1->time>temp1_tail->time)
						{
							if(temp1->operation==WRITE)
							{
								ssd->waw=ssd->waw+1;
							}
							else
							{
								ssd->raw=ssd->raw+1;
							}
							temp1->cnt=1;
							temp1_tail=temp1_tail->next_node;
						}
						else
						{
							temp1_tail=temp1_tail->next_node;
						}
					}
					else
					{
						if(temp1_tail->lsn-temp1->lsn<=temp1->size&&temp1->time>temp1_tail->time)
						{
							if(temp1->operation==WRITE)
							{
								ssd->waw=ssd->waw+1;
							}
							else
							{
								ssd->raw=ssd->raw+1;
							}
							temp1->cnt=1;
							temp1_tail=temp1_tail->next_node;
						}
						else
						{
							temp1_tail=temp1_tail->next_node;
						}
					}
					//temp1->cnt=1;
				}
			}
			temp1=temp1->next_node;
		}
	}
}	

//KXC:to find the waw requests
/* 	while(temp1!=NULL)
	{
		if(temp1->operation==READ)
		{
			temp1=temp1->next_node;
		}
		else
		{
			temp1_tail=temp1->next_node;
			while(temp1_tail!=NULL)
			{
				if(temp1_tail->operation==READ)
				{
					temp1_tail=temp1_tail->next_node;
				}
				else
				{
					if(temp1->lsn>temp1_tail->lsn)
					{
						if(temp1->lsn-temp1_tail->lsn<=temp1_tail->size&&temp1->time>temp1_tail->time)
						{
							ssd->waw=ssd->waw+1;
							temp1_tail=temp1_tail->next_node;
						}
						else
						{
							temp1_tail=temp1_tail->next_node;
						}
					}
					else
					{
						if(temp1_tail->lsn-temp1->lsn<=temp1->size&&temp1->time>temp1_tail->time)
						{
							ssd->waw=ssd->waw+1;
							temp1_tail=temp1_tail->next_node;
						}
						else
						{
							temp1_tail=temp1_tail->next_node;
						}
					}
					
				}
				

			}
			temp1=temp1->next_node;
		}
		
	}
} */


/**********************************************************************************************************************************************
*����buffer�Ǹ�дbuffer������Ϊд�������ģ���Ϊ��flash��ʱ��tRΪ20us��дflash��ʱ��tprogΪ200us������Ϊд������ܽ�ʡʱ��?
*  �����������������buffer����buffer������ռ��channel��I/O���ߣ�û������buffer����flash����ռ��channel��I/O���ߣ����ǲ���buffer��
*  д����������request�ֳ�sub_request����������Ƕ��?���䣬sub_request�ҵ�ssd->sub_request�ϣ���Ϊ��֪��Ҫ�ȹҵ��ĸ�channel��sub_request��
*          ����Ǿ��?������sub_request�ҵ�channel��sub_request����,ͬʱ���ܶ�̬���仹�Ǿ�̬����sub_request��Ҫ�ҵ�request��sub_request����
*		   ��Ϊÿ������һ��request����Ҫ��traceoutput�ļ�������������request����Ϣ��������һ��sub_request,�ͽ����channel��sub_request��
*		   ��ssd��sub_request����ժ����������traceoutput�ļ����һ���������request��sub_request����
*		   sub_request����buffer����buffer����д�����ˣ����ҽ���sub_page�ᵽbuffer��ͷ(LRU)����û��������buffer�������Ƚ�buffer��β��sub_request
*		   д��flash(������һ��sub_requestд���󣬹ҵ��������request��sub_request���ϣ�ͬʱ�Ӷ�̬���仹�Ǿ�̬����ҵ�channel��ssd��
*		   sub_request����),�ڽ�Ҫд��sub_pageд��buffer��ͷ
***********************************************************************************************************************************************/
struct ssd_info *buffer_management(struct ssd_info *ssd)
{   
	unsigned int j,lsn,lpn,last_lpn,first_lpn,index,complete_flag=0, state,full_page;
	unsigned int flag=0,need_distb_flag,lsn_flag,flag1=1,active_region_flag=0;           
	struct request *new_request;
	struct buffer_group *buffer_node,key;
	unsigned int mask=0,offset1=0,offset2=0;

	#ifdef DEBUG
	printf("enter buffer_management,  current time:%lld\n",ssd->current_time);
	#endif
	ssd->dram->current_time=ssd->current_time;
	full_page=~(0xffffffff<<ssd->parameter->subpage_page);
	
	new_request=ssd->request_tail;
	lsn=new_request->lsn;
	lpn=new_request->lsn/ssd->parameter->subpage_page;
	last_lpn=(new_request->lsn+new_request->size-1)/ssd->parameter->subpage_page;
	first_lpn=new_request->lsn/ssd->parameter->subpage_page;

	new_request->need_distr_flag=(unsigned int*)malloc(sizeof(unsigned int)*((last_lpn-first_lpn+1)*ssd->parameter->subpage_page/32+1));
	alloc_assert(new_request->need_distr_flag,"new_request->need_distr_flag");
	memset(new_request->need_distr_flag, 0, sizeof(unsigned int)*((last_lpn-first_lpn+1)*ssd->parameter->subpage_page/32+1));
	
	if(new_request->operation==READ) 
	{		
		while(lpn<=last_lpn)      		
		{
			/************************************************************************************************
			 *need_distb_flag��ʾ�Ƿ���Ҫִ��distribution������1��ʾ��Ҫִ�У�buffer��û�У�0��ʾ����Ҫִ��
             *��1��ʾ��Ҫ�ַ���0��ʾ����Ҫ�ַ�����Ӧ���ʼ�?����Ϊ1
			*************************************************************************************************/
			need_distb_flag=full_page;   
			key.group=lpn;
			buffer_node= (struct buffer_group*)avlTreeFind(ssd->dram->buffer, (TREE_NODE *)&key);		// buffer node 

			while((buffer_node!=NULL)&&(lsn<(lpn+1)*ssd->parameter->subpage_page)&&(lsn<=(new_request->lsn+new_request->size-1)))             			
			{             	
				lsn_flag=full_page;
				mask=1 << (lsn%ssd->parameter->subpage_page);
				if(mask>31)
				{
					printf("the subpage number is larger than 32!add some cases");
					getchar(); 		   
				}
				else if((buffer_node->stored & mask)==mask)  //stored����Ƕ�Ӧ�������Ƿ���buffer�У������ڣ���flag=1����ʾ��ȡ�ɹ�
				{
					flag=1;
					lsn_flag=lsn_flag&(~mask);
				}

				if(flag==1)				
				{	//�����buffer�ڵ㲻��buffer�Ķ��ף���Ҫ������ڵ��ᵽ���ף�ʵ����LRU�㷨�������һ���?����С�?		       		
					if(ssd->dram->buffer->buffer_head!=buffer_node)     
					{		
						if(ssd->dram->buffer->buffer_tail==buffer_node)								
						{			
							buffer_node->LRU_link_pre->LRU_link_next=NULL;		//β�ڵ�Ͽ�?			
							ssd->dram->buffer->buffer_tail=buffer_node->LRU_link_pre;							
						}				
						else								
						{				
							buffer_node->LRU_link_pre->LRU_link_next=buffer_node->LRU_link_next;		//�м�ڵ�Ͽ�		
							buffer_node->LRU_link_next->LRU_link_pre=buffer_node->LRU_link_pre;								
						}								
						buffer_node->LRU_link_next=ssd->dram->buffer->buffer_head;
						ssd->dram->buffer->buffer_head->LRU_link_pre=buffer_node;
						buffer_node->LRU_link_pre=NULL;			
						ssd->dram->buffer->buffer_head=buffer_node;													
					}						
					ssd->dram->buffer->read_hit++;					
					new_request->complete_lsn_count++;											
				}		
				else if(flag==0)
					{
						ssd->dram->buffer->read_miss_hit++;
					}

				need_distb_flag=need_distb_flag&lsn_flag;
				
				flag=0;		
				lsn++;						
			}	
				
			index=(lpn-first_lpn)/(32/ssd->parameter->subpage_page); 			
			new_request->need_distr_flag[index]=new_request->need_distr_flag[index]|(need_distb_flag<<(((lpn-first_lpn)%(32/ssd->parameter->subpage_page))*ssd->parameter->subpage_page));	
			lpn++;
			
		}
	}  
	else if(new_request->operation==WRITE)
	{
		while(lpn<=last_lpn)           	
		{	
			need_distb_flag=full_page;
			mask=~(0xffffffff<<(ssd->parameter->subpage_page));
			state=mask;

			if(lpn==first_lpn)
			{
				offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-new_request->lsn);//������ĵ�һ��LPN����Ҫ��������ĵ�һ��lsn�Ƿ���LPN�ĵ�һ��lsn���������?
				state=state&(0xffffffff<<offset1);
			}
			if(lpn==last_lpn)
			{
				offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(new_request->lsn+new_request->size));
				state=state&(~(0xffffffff<<offset2));
			}
			
			ssd=insert2buffer(ssd, lpn, state,NULL,new_request);
			lpn++;
		}
	}
	complete_flag = 1;
	for(j=0;j<=(last_lpn-first_lpn+1)*ssd->parameter->subpage_page/32;j++)
	{
		if(new_request->need_distr_flag[j] != 0)
		{
			complete_flag = 0;
		}
	}

	/*************************************************************
	*��������Ѿ����?����buffer���񣬸�������Ա�ֱ����Ӧ��������?
	*�������dram�ķ���ʱ��Ϊ1000ns
	**************************************************************/
	if((complete_flag == 1)&&(new_request->subs==NULL))               
	{
		new_request->begin_time=ssd->current_time;
		new_request->response_time=ssd->current_time+1000;            
	}

	return ssd;
}

/*****************************
*lpn��ppn��ת��
******************************/
unsigned int lpn2ppn(struct ssd_info *ssd,unsigned int lsn)
{
	int lpn, ppn;	
	struct entry *p_map = ssd->dram->map->map_entry;
#ifdef DEBUG
	printf("enter lpn2ppn,  current time:%lld\n",ssd->current_time);
#endif
	lpn = lsn/ssd->parameter->subpage_page;			//lpn
	ppn = (p_map[lpn]).pn;
	return ppn;
}

/**********************************************************************************
*�����������������������ֻ����������д�����Ѿ���buffer_management()�����д�����
*����������к�buffer���еļ��?��ÿ������ֽ�������󣬽���������й���channel�ϣ�
*��ͬ��channel���Լ������������?
**********************************************************************************/

struct ssd_info *distribute(struct ssd_info *ssd) 
{
	unsigned int start, end, first_lsn,last_lsn,lpn,flag=0,flag_attached=0,full_page;
	unsigned int j, k, sub_size;
	int i=0;
	struct request *req;
	struct sub_request *sub;
	int* complt;

	#ifdef DEBUG
	printf("enter distribute,  current time:%lld\n",ssd->current_time);
	#endif
	full_page=~(0xffffffff<<ssd->parameter->subpage_page);

	req = ssd->request_tail;
	if(req->response_time != 0){
		return ssd;
	}
	if (req->operation==WRITE)
	{
		return ssd;
	}

	if(req != NULL)
	{
		if(req->distri_flag == 0)
		{
			//�������һЩ���������?����
			if(req->complete_lsn_count != ssd->request_tail->size)
			{		
				first_lsn = req->lsn;				
				last_lsn = first_lsn + req->size;
				complt = req->need_distr_flag;
				start = first_lsn - first_lsn % ssd->parameter->subpage_page;
				end = (last_lsn/ssd->parameter->subpage_page + 1) * ssd->parameter->subpage_page;
				i = (end - start)/32;	

				while(i >= 0)
				{	
					/*************************************************************************************
					*һ��32λ���������ݵ�ÿһλ����һ����ҳ��32/ssd->parameter->subpage_page�ͱ�ʾ�ж���ҳ��
					*�����ÿһҳ��״�?���������? req->need_distr_flag�У�Ҳ����complt�У�ͨ���Ƚ�complt��
					*ÿһ����full_page���Ϳ���֪������һҳ�Ƿ�����ɡ����û����������?��creat_sub_request
					��������������
					*************************************************************************************/
					for(j=0; j<32/ssd->parameter->subpage_page; j++)
					{	
						k = (complt[((end-start)/32-i)] >>(ssd->parameter->subpage_page*j)) & full_page;	
						if (k !=0)
						{
							lpn = start/ssd->parameter->subpage_page+ ((end-start)/32-i)*32/ssd->parameter->subpage_page + j;
							sub_size=transfer_size(ssd,k,lpn,req);    
							if (sub_size==0) 
							{
								continue;
							}
							else
							{
								sub=creat_sub_request(ssd,lpn,sub_size,0,req,req->operation);
							}	
						}
					}
					i = i-1;
				}

			}
			else                                          //distri_flag == 1��������buffer�еõ�����ֱ�ӷ��ط���ʱ�䡣
			{
				req->begin_time=ssd->current_time;
				req->response_time=ssd->current_time+1000;   
			}

		}
	}
	return ssd;
}


/**********************************************************************
*trace_output()��������ÿһ�����������������?��process()�����������?
*��ӡ�����ص����н����outputfile�ļ��У�����Ľ����Ҫ�����е�ʱ��
**********************************************************************/
void trace_output(struct ssd_info* ssd){
	int flag = 1;	
	int64_t start_time, end_time,wait_time;
	struct request *req, *pre_node;
	struct sub_request *sub, *tmp;
	int i,channel,chip;
	unsigned int first_lpn,last_lpn;

#ifdef DEBUG
	printf("enter trace_output,  current time:%lld\n",ssd->current_time);
#endif

	pre_node=NULL;
	req = ssd->request_queue;
	start_time = 0;
	end_time = 0;

	if(req == NULL)
		return;

	while(req != NULL)	
	{
		sub = req->subs;
		flag = 1;
		start_time = 0;
		end_time = 0;
		if(req->response_time != 0)
		{
			fprintf(ssd->outputfile,"%16lld %10d %6d %2d %16lld %16lld %10lld\n",req->time,req->lsn, req->size, req->operation, req->begin_time, req->response_time, req->response_time-req->time);
			fflush(ssd->outputfile);

			if(req->response_time-req->begin_time==0)
			{
				printf("the response time is 0?? \n");
				getchar();
			}

			if (req->operation==READ)
			{
				ssd->read_request_count++;
				ssd->read_avg=ssd->read_avg+(req->response_time-req->time);
			} 
			else
			{
				ssd->write_request_count++;
				ssd->write_avg=ssd->write_avg+(req->response_time-req->time);
			}

			if(pre_node == NULL)
			{
				if(req->next_node == NULL)
				{
					free(req->need_distr_flag);
					req->need_distr_flag=NULL;
					free(req);
					req = NULL;
					ssd->request_queue = NULL;
					ssd->request_tail = NULL;
					ssd->request_queue_length--;
				}
				else
				{
					ssd->request_queue = req->next_node;
					pre_node = req;
					req = req->next_node;
					free(pre_node->need_distr_flag);
					pre_node->need_distr_flag=NULL;
					free((void *)pre_node);
					pre_node = NULL;
					ssd->request_queue_length--;
				}
			}
			else
			{
				if(req->next_node == NULL)
				{
					pre_node->next_node = NULL;
					free(req->need_distr_flag);
					req->need_distr_flag=NULL;
					free(req);
					req = NULL;
					ssd->request_tail = pre_node;
					ssd->request_queue_length--;
				}
				else
				{
					pre_node->next_node = req->next_node;
					free(req->need_distr_flag);
					req->need_distr_flag=NULL;
					free((void *)req);
					req = pre_node->next_node;
					ssd->request_queue_length--;
				}
			}
		}
		else
		{
			flag=1;
			while(sub != NULL)
			{
				if(start_time == 0)
					start_time = sub->begin_time;
				if(start_time > sub->begin_time)
					start_time = sub->begin_time;
				if(end_time < sub->complete_time)
					end_time = sub->complete_time;
				if((sub->current_state == SR_COMPLETE)||((sub->next_state==SR_COMPLETE)&&(sub->next_state_predict_time<=ssd->current_time)))	// if any sub-request is not completed, the request is not completed
				{
					sub = sub->next_subs;
				}
				else
				{
					flag=0;
					break;
				}
				
			}

			if (flag == 1&&req->dis==1)
			{		
				//KXC:to calcute the wait time
				if (start_time>=req->time)
				{
					wait_time=start_time-req->time;
				}
				else
				{
					wait_time=0;
				}
							
				//fprintf(ssd->outputfile,"%10I64u %10u %6u %2u %16I64u %16I64u %10I64u\n",req->time,req->lsn, req->size, req->operation, start_time, end_time, end_time-req->time);
				fprintf(ssd->outputfile,"%16lld %10d %6d %2d %16lld %16lld %10lld %16lld\n",req->time,req->lsn, req->size, req->operation, start_time, end_time, end_time-start_time,wait_time);
				fflush(ssd->outputfile);

				if(end_time-start_time==0)
				{
					printf("the response time is 0?? \n");
					getchar();
				}
				//KXC:to update the value
				ssd->previous_response_time=end_time;
				ssd->wait_avg=ssd->wait_avg+wait_time;
				ssd->total_avg=ssd->total_avg+(end_time-start_time);
				ssd->total_avg_wait=ssd->total_avg_wait+(end_time-req->time);
				if (req->operation==READ)
				{
					ssd->read_request_count++;
					ssd->read_avg=ssd->read_avg+(end_time-start_time);
					ssd->read_wait_avg=ssd->read_wait_avg+wait_time;
					ssd->read_avg_wait=ssd->read_avg_wait+(end_time-req->time);
				} 
				else
				{
					ssd->write_request_count++;
					ssd->write_avg=ssd->write_avg+(end_time-start_time);
					ssd->write_wait_avg=ssd->write_wait_avg+wait_time;
					ssd->write_avg_wait=ssd->write_avg_wait+(end_time-req->time);
				}
				
				while(req->subs!=NULL)
				{
					tmp = req->subs;
					req->subs = tmp->next_subs;
					if (tmp->update!=NULL)
					{
						free(tmp->update->location);
						tmp->update->location=NULL;
						free(tmp->update);
						tmp->update=NULL;
					}
					free(tmp->location);
					tmp->location=NULL;
					free(tmp);
					tmp=NULL;
					
				}
				
				if(pre_node == NULL)
				{
					if(req->next_node == NULL)
					{
						free(req->need_distr_flag);
						req->need_distr_flag=NULL;
						free(req);
						req = NULL;
						ssd->request_queue = NULL;
						ssd->request_tail = NULL;
						ssd->request_queue_length--;
					}
					else
					{
						ssd->request_queue = req->next_node;
						pre_node = req;
						req = req->next_node;
						free(pre_node->need_distr_flag);
						pre_node->need_distr_flag=NULL;
						free(pre_node);
						pre_node = NULL;
						ssd->request_queue_length--;
					}
				}
				else
				{
					if(req->next_node == NULL)
					{
						pre_node->next_node = NULL;
						free(req->need_distr_flag);
						req->need_distr_flag=NULL;
						free(req);
						req = NULL;
						ssd->request_tail = pre_node;	
						ssd->request_queue_length--;
					}
					else
					{
						pre_node->next_node = req->next_node;
						free(req->need_distr_flag);
						req->need_distr_flag=NULL;
						free(req);
						req = pre_node->next_node;
						ssd->request_queue_length--;
					}

				}
			}
			else
			{	
				pre_node = req;
				req = req->next_node;
			}
		}		
	}
}


/*******************************************************************************
*statistic_output()������Ҫ�����������һ����������ش�����Ϣ��
*1�������ÿ��plane�Ĳ���������plane_erase���ܵĲ���������erase
*2����ӡmin_lsn��max_lsn��read_count��program_count��ͳ����Ϣ���ļ�outputfile�С�
*3����ӡ��ͬ����Ϣ���ļ�statisticfile��
*******************************************************************************/
void statistic_output(struct ssd_info *ssd)
{
	unsigned int lpn_count=0,i,ii,j,k,m,erase=0,plane_erase=0;
	double gc_energy=0.0;
#ifdef DEBUG
	printf("enter statistic_output,  current time:%lld\n",ssd->current_time);
#endif

/************KXC:�޸����ʹ������߼�  2019.8.12*********/
	for(i=0;i<ssd->parameter->channel_number;i++)
	{
		for (ii=0;ii<ssd->parameter->chip_channel[i];ii++)
        { 
			for(j=0;j<ssd->parameter->die_chip;j++)
		   {
			  for(k=0;k<ssd->parameter->plane_die;k++)
			  {
				plane_erase=0;
				for(m=0;m<ssd->parameter->block_plane;m++)
				{
					if(ssd->channel_head[i].chip_head[ii].die_head[j].plane_head[k].blk_head[m].erase_count>0)
					{
						erase=erase+ssd->channel_head[i].chip_head[ii].die_head[j].plane_head[k].blk_head[m].erase_count;
						plane_erase+=ssd->channel_head[i].chip_head[ii].die_head[j].plane_head[k].blk_head[m].erase_count;
					}
				}
				fprintf(ssd->outputfile,"the %d channel, %d chip, %d die, %d plane has : %13d erase operations\n",i,ii,j,k,plane_erase);
				fprintf(ssd->statisticfile,"the %d channel, %d chip, %d die, %d plane has : %13d erase operations\n",i,ii,j,k,plane_erase);
			  }
		    }
	    }
	}

	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"---------------------------statistic data---------------------------\n");
	fprintf(ssd->outputfile,"buffer read hits: %13d\n",ssd->dram->buffer->read_hit);
	fprintf(ssd->outputfile,"buffer read miss: %13d\n",ssd->dram->buffer->read_miss_hit);
	fprintf(ssd->outputfile,"buffer write hits: %13d\n",ssd->dram->buffer->write_hit);
	fprintf(ssd->outputfile,"buffer write miss: %13d\n",ssd->dram->buffer->write_miss_hit);
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"min lsn: %13d\n",ssd->min_lsn);	
	fprintf(ssd->outputfile,"max lsn: %13d\n",ssd->max_lsn);
	fprintf(ssd->outputfile,"read count: %13d\n",ssd->read_count);	  
	fprintf(ssd->outputfile,"program count: %13d",ssd->program_count);	
	fprintf(ssd->outputfile,"                        include the flash write count leaded by read requests\n");
	fprintf(ssd->outputfile,"the read operation leaded by un-covered update count: %13d\n",ssd->update_read_count);
	fprintf(ssd->outputfile,"erase count: %13d\n",ssd->erase_count);
	fprintf(ssd->outputfile,"direct erase count: %13d\n",ssd->direct_erase_count);
	fprintf(ssd->outputfile,"copy back count: %13d\n",ssd->copy_back_count);
	fprintf(ssd->outputfile,"multi-plane program count: %13d\n",ssd->m_plane_prog_count);
	fprintf(ssd->outputfile,"multi-plane read count: %13d\n",ssd->m_plane_read_count);
	fprintf(ssd->outputfile,"interleave write count: %13d\n",ssd->interleave_count);
	fprintf(ssd->outputfile,"interleave read count: %13d\n",ssd->interleave_read_count);
	fprintf(ssd->outputfile,"interleave two plane and one program count: %13d\n",ssd->inter_mplane_prog_count);
	fprintf(ssd->outputfile,"interleave two plane count: %13d\n",ssd->inter_mplane_count);
	fprintf(ssd->outputfile,"gc copy back count: %13d\n",ssd->gc_copy_back);
	fprintf(ssd->outputfile,"write flash count: %13d\n",ssd->write_flash_count);
	fprintf(ssd->outputfile,"interleave erase count: %13d\n",ssd->interleave_erase_count);
	fprintf(ssd->outputfile,"multiple plane erase count: %13d\n",ssd->mplane_erase_conut);
	fprintf(ssd->outputfile,"interleave multiple plane erase count: %13d\n",ssd->interleave_mplane_erase_count);
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"erase: %13d\n",erase);
	fprintf(ssd->outputfile,"raw count: %13d\n",ssd->raw);
	fprintf(ssd->outputfile,"waw count: %13d\n",ssd->waw);
	fprintf(ssd->outputfile,"read request count: %13d\n",ssd->read_request_count);
	fprintf(ssd->outputfile,"write request count: %13d\n",ssd->write_request_count);
	fprintf(ssd->outputfile,"read request average size: %13f\n",ssd->ave_read_size);
	fprintf(ssd->outputfile,"write request average size: %13f\n",ssd->ave_write_size);
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"read request average response time: %lld\n",ssd->read_avg/ssd->read_request_count);
	fprintf(ssd->outputfile,"write request average response time: %lld\n",ssd->write_avg/ssd->write_request_count);
	fprintf(ssd->outputfile,"total average response time: %lld\n",ssd->total_avg/(ssd->write_request_count+ssd->read_request_count));
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"read request average response time including wait time: %lld\n",ssd->read_avg_wait/ssd->read_request_count);
	fprintf(ssd->outputfile,"write request average response time including wait time: %lld\n",ssd->write_avg_wait/ssd->write_request_count);
	fprintf(ssd->outputfile,"total average response time including wait time: %lld\n",ssd->total_avg_wait/(ssd->write_request_count+ssd->read_request_count));
	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"read average wait time: %lld\n",ssd->read_wait_avg/ssd->read_request_count);
	fprintf(ssd->outputfile,"write average wait time: %lld\n",ssd->write_wait_avg/ssd->write_request_count);
	fprintf(ssd->outputfile,"total average wait time: %lld\n",ssd->wait_avg/(ssd->write_request_count+ssd->read_request_count));
	fprintf(ssd->outputfile,"\n");

	fflush(ssd->outputfile);

	fclose(ssd->outputfile);


	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"---------------------------statistic data---------------------------\n");
	fprintf(ssd->statisticfile,"buffer read hits: %13d\n",ssd->dram->buffer->read_hit);
	fprintf(ssd->statisticfile,"buffer read miss: %13d\n",ssd->dram->buffer->read_miss_hit);
	fprintf(ssd->statisticfile,"buffer write hits: %13d\n",ssd->dram->buffer->write_hit);
	fprintf(ssd->statisticfile,"buffer write miss: %13d\n",ssd->dram->buffer->write_miss_hit);
	fprintf(ssd->statisticfile,"\n");	
	fprintf(ssd->statisticfile,"min lsn: %13d\n",ssd->min_lsn);	
	fprintf(ssd->statisticfile,"max lsn: %13d\n",ssd->max_lsn);
	fprintf(ssd->statisticfile,"read count: %13d\n",ssd->read_count);	  
	fprintf(ssd->statisticfile,"program count: %13d",ssd->program_count);	  
	fprintf(ssd->statisticfile,"                        include the flash write count leaded by read requests\n");
	fprintf(ssd->statisticfile,"the read operation leaded by un-covered update count: %13d\n",ssd->update_read_count);
	fprintf(ssd->statisticfile,"erase count: %13d\n",ssd->erase_count);	  
	fprintf(ssd->statisticfile,"direct erase count: %13d\n",ssd->direct_erase_count);
	fprintf(ssd->statisticfile,"copy back count: %13d\n",ssd->copy_back_count);
	fprintf(ssd->statisticfile,"multi-plane program count: %13d\n",ssd->m_plane_prog_count);
	fprintf(ssd->statisticfile,"multi-plane read count: %13d\n",ssd->m_plane_read_count);
	fprintf(ssd->statisticfile,"interleave count: %13d\n",ssd->interleave_count);
	fprintf(ssd->statisticfile,"interleave read count: %13d\n",ssd->interleave_read_count);
	fprintf(ssd->statisticfile,"interleave two plane and one program count: %13d\n",ssd->inter_mplane_prog_count);
	fprintf(ssd->statisticfile,"interleave two plane count: %13d\n",ssd->inter_mplane_count);
	fprintf(ssd->statisticfile,"gc copy back count: %13d\n",ssd->gc_copy_back);
	fprintf(ssd->statisticfile,"write flash count: %13d\n",ssd->write_flash_count);
	fprintf(ssd->statisticfile,"waste page count: %13d\n",ssd->waste_page_count);
	fprintf(ssd->statisticfile,"interleave erase count: %13d\n",ssd->interleave_erase_count);
	fprintf(ssd->statisticfile,"multiple plane erase count: %13d\n",ssd->mplane_erase_conut);
	fprintf(ssd->statisticfile,"interleave multiple plane erase count: %13d\n",ssd->interleave_mplane_erase_count);
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"erase: %13d\n",erase);
	fprintf(ssd->statisticfile,"raw count: %13d\n",ssd->raw);
	fprintf(ssd->statisticfile,"waw count: %13d\n",ssd->waw);		
	fprintf(ssd->statisticfile,"read request count: %13d\n",ssd->read_request_count);
	fprintf(ssd->statisticfile,"write request count: %13d\n",ssd->write_request_count);
	fprintf(ssd->statisticfile,"read request average size: %13f\n",ssd->ave_read_size);
	fprintf(ssd->statisticfile,"write request average size: %13f\n",ssd->ave_write_size);
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"read request average response time: %lld\n",ssd->read_avg/ssd->read_request_count);
	fprintf(ssd->statisticfile,"write request average response time: %lld\n",ssd->write_avg/ssd->write_request_count);
	fprintf(ssd->statisticfile,"total average response time: %lld\n",ssd->total_avg/(ssd->write_request_count+ssd->read_request_count));
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"read request average response time including wait time: %lld\n",ssd->read_avg_wait/ssd->read_request_count);
	fprintf(ssd->statisticfile,"write request average response time including wait time: %lld\n",ssd->write_avg_wait/ssd->write_request_count);
	fprintf(ssd->statisticfile,"total average response time including wait time: %lld\n",ssd->total_avg_wait/(ssd->write_request_count+ssd->read_request_count));
	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"read average wait time: %lld\n",ssd->read_wait_avg/ssd->read_request_count);
	fprintf(ssd->statisticfile,"write average wait time: %lld\n",ssd->write_wait_avg/ssd->write_request_count);
	fprintf(ssd->statisticfile,"total average wait time: %lld\n",ssd->wait_avg/(ssd->write_request_count+ssd->read_request_count));
	fprintf(ssd->statisticfile,"\n");

	fflush(ssd->statisticfile);

	fclose(ssd->statisticfile);
}


/***********************************************************************************
*����ÿһҳ��״̬�����ÿһ���?��������ҳ����Ŀ��Ҳ����һ����������Ҫ��������ҳ��ҳ��
************************************************************************************/
unsigned int size(unsigned int stored)
{
	unsigned int i,total=0,mask=0x80000000;

	#ifdef DEBUG
	printf("enter size\n");
	#endif
	for(i=1;i<=32;i++)
	{
		if(stored & mask) total++;
		stored<<=1;
	}
	#ifdef DEBUG
	    printf("leave size\n");
    #endif
    return total;
}


/*********************************************************
*transfer_size()���������þ��Ǽ�������������Ҫ������size
*�����е���������first_lpn��last_lpn�������ر���������?��
*��������º��п��ܲ��Ǵ���һ��ҳ���Ǵ���һҳ��һ���֣���?
*Ϊlsn�п��ܲ���һҳ�ĵ�һ����ҳ��
*********************************************************/
unsigned int transfer_size(struct ssd_info *ssd,int need_distribute,unsigned int lpn,struct request *req)
{
	unsigned int first_lpn,last_lpn,state,trans_size;
	unsigned int mask=0,offset1=0,offset2=0;

	first_lpn=req->lsn/ssd->parameter->subpage_page;
	last_lpn=(req->lsn+req->size-1)/ssd->parameter->subpage_page;

	mask=~(0xffffffff<<(ssd->parameter->subpage_page));
	state=mask;
	if(lpn==first_lpn)
	{
		offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-req->lsn);
		state=state&(0xffffffff<<offset1);
	}
	if(lpn==last_lpn)
	{
		offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(req->lsn+req->size));
		state=state&(~(0xffffffff<<offset2));
	}

	trans_size=size(state&need_distribute);

	return trans_size;
}


/**********************************************************************************************************  
*int64_t find_nearest_event(struct ssd_info *ssd)       
*Ѱ����������������絽����¸�״̬ʱ��,���ȿ��������һ��״�?ʱ�䣬���������¸�״̬ʱ��С�ڵ��ڵ�ǰʱ�䣬
*˵��������������Ҫ�鿴channel���߶�Ӧdie����һ״̬ʱ�䡣Int64���з��� 64 λ�����������ͣ�ֵ���ͱ�ʾֵ����
*-2^63 ( -9,223,372,036,854,775,808)��2^63-1(+9,223,372,036,854,775,807 )֮�����������?�ռ�ռ 8 �ֽڡ�
*channel,die���¼���ǰ�ƽ��Ĺؼ����أ������������ʹ�¼�������ǰ�ƽ���channel��die�ֱ�ص�idle״̬��die�е�
*������׼������
***********************************************************************************************************/
int64_t find_nearest_event(struct ssd_info *ssd) 
{
	unsigned int i,j;
	int64_t time=MAX_INT64;
	int64_t time1=MAX_INT64;
	int64_t time2=MAX_INT64;

	for (i=0;i<ssd->parameter->channel_number;i++)
	{
		if (ssd->channel_head[i].next_state==CHANNEL_IDLE)
			if(time1>ssd->channel_head[i].next_state_predict_time)
				if (ssd->channel_head[i].next_state_predict_time>ssd->current_time)    
					time1=ssd->channel_head[i].next_state_predict_time;
		for (j=0;j<ssd->parameter->chip_channel[i];j++)
		{
			if ((ssd->channel_head[i].chip_head[j].next_state==CHIP_IDLE)||(ssd->channel_head[i].chip_head[j].next_state==CHIP_DATA_TRANSFER))
				if(time2>ssd->channel_head[i].chip_head[j].next_state_predict_time)
					if (ssd->channel_head[i].chip_head[j].next_state_predict_time>ssd->current_time)    
						time2=ssd->channel_head[i].chip_head[j].next_state_predict_time;	
		}   
	} 

	/*****************************************************************************************************
	 *timeΪ���� A.��һ״̬ΪCHANNEL_IDLE����һ״̬Ԥ��ʱ�����ssd��ǰʱ���CHANNEL����һ״̬Ԥ��ʱ��
	 *           B.��һ״̬ΪCHIP_IDLE����һ״̬Ԥ��ʱ�����ssd��ǰʱ���DIE����һ״̬Ԥ��ʱ��
	 *		     C.��һ״̬ΪCHIP_DATA_TRANSFER����һ״̬Ԥ��ʱ�����ssd��ǰʱ���DIE����һ״̬Ԥ��ʱ��
	 *CHIP_DATA_TRANSFER��׼����״̬�������Ѵӽ��ʴ�����register����һ״̬�Ǵ�register����buffer�е���Сֵ 
	 *ע����ܶ�û�������?���time����ʱtime����0x7fffffffffffffff ��
	*****************************************************************************************************/
	time=(time1>time2)?time2:time1;
	return time;
}

/***********************************************
*free_all_node()���������þ����ͷ���������Ľڵ�?
************************************************/
void free_all_node(struct ssd_info *ssd)
{
	unsigned int i,j,k,l,n;
	struct buffer_group *pt=NULL;
	struct direct_erase * erase_node=NULL;
	for (i=0;i<ssd->parameter->channel_number;i++)
	{
		for (j=0;j<ssd->parameter->chip_channel[0];j++)
		{
			for (k=0;k<ssd->parameter->die_chip;k++)
			{
				for (l=0;l<ssd->parameter->plane_die;l++)
				{
					for (n=0;n<ssd->parameter->block_plane;n++)
					{
						free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[n].page_head);
						ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[n].page_head=NULL;
					}
					free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head);
					ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head=NULL;
					while(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node!=NULL)
					{
						erase_node=ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node;
						ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node=erase_node->next_node;
						free(erase_node);
						erase_node=NULL;
					}
				}
				
				free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head);
				ssd->channel_head[i].chip_head[j].die_head[k].plane_head=NULL;
			}
			free(ssd->channel_head[i].chip_head[j].die_head);
			ssd->channel_head[i].chip_head[j].die_head=NULL;
		}
		free(ssd->channel_head[i].chip_head);
		ssd->channel_head[i].chip_head=NULL;
	}
	free(ssd->channel_head);
	ssd->channel_head=NULL;

	avlTreeDestroy( ssd->dram->buffer);
	ssd->dram->buffer=NULL;
	
	free(ssd->dram->map->map_entry);
	ssd->dram->map->map_entry=NULL;
	free(ssd->dram->map);
	ssd->dram->map=NULL;
	free(ssd->dram);
	ssd->dram=NULL;
	free(ssd->parameter);
	ssd->parameter=NULL;

	free(ssd);
	ssd=NULL;
}


/*****************************************************************************
*make_aged()���������þ���ģ����ʵ���ù�һ��ʱ���ssd��
*��ô���ssd����Ӧ�Ĳ�����Ҫ�ı䣬�����������ʵ���Ͼ��Ƕ�ssd�и��������ĸ�ֵ��
******************************************************************************/
struct ssd_info *make_aged(struct ssd_info *ssd)
{
	unsigned int i,j,k,l,m,n,ppn;
	int threshould,flag=0;
    
	if (ssd->parameter->aged==1)
	{
		//threshold��ʾһ��plane���ж���ҳ��Ҫ��ǰ��ΪʧЧ
		threshould=(int)(ssd->parameter->block_plane*ssd->parameter->page_block*ssd->parameter->aged_ratio);  
		for (i=0;i<ssd->parameter->channel_number;i++)
			for (j=0;j<ssd->parameter->chip_channel[i];j++)
				for (k=0;k<ssd->parameter->die_chip;k++)
					for (l=0;l<ssd->parameter->plane_die;l++)
					{  
						flag=0;
						for (m=0;m<ssd->parameter->block_plane;m++)
						{  
							if (flag>=threshould)
							{
								break;
							}
							for (n=0;n<(ssd->parameter->page_block*ssd->parameter->aged_ratio+1);n++)
							{  
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].valid_state=0;        //��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].free_state=0;         //��ʾĳһҳʧЧ��ͬʱ���valid��free״̬��Ϊ0
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].lpn=0;  //��valid_state free_state lpn����Ϊ0��ʾҳʧЧ������ʱ��������?����lpn=0��������Чҳ
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_page_num--;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].invalid_page_num++;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_page++;
								ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].free_page--;
								flag++;

								ppn=find_ppn(ssd,i,j,k,l,m,n);
							
							}
						} 
					}	 
	}  
	else
	{
		return ssd;
	}

	return ssd;
}


/*********************************************************************************************
*no_buffer_distribute()�����Ǵ�����ssdû��dram��ʱ��
*���Ƕ�д����Ͳ��������?��buffer����Ѱ�ң�ֱ������creat_sub_request()���������������ٴ�����
*********************************************************************************************/
struct ssd_info *no_buffer_distribute_sch(struct ssd_info *ssd)
{
	unsigned int lsn,lpn,last_lpn,first_lpn,complete_flag=0, state;
	unsigned int flag=0,flag1=1,active_region_flag=0;           //to indicate the lsn is hitted or not
	struct request *req=NULL,*reqtemp;
	struct sub_request *sub=NULL,*sub_r=NULL,*update=NULL;
	struct local *loc=NULL;
	struct channel_info *p_ch=NULL;

	int64_t nearest_event_time;
	int64_t next_time;
	unsigned int mask=0; 
	unsigned int offset1=0, offset2=0;
	unsigned int sub_size=0;
	unsigned int sub_state=0;

	int i=0,pflag=0;

	//KXC:the request is empty,exit and get next request
	if(ssd->request_queue==NULL)
	{
		ssd->empty=1;
		return 0;
	}
	else
	{
		next_time=ssd->request_queue->time;
	}
		
	//to find the next not distributed request
	req=ssd->request_queue;
	while(req!=NULL)
	{
		if (req->dis==1)
		{
			if(req->next_node!=NULL)
			{
				if(req->next_node->dis==1)
				{
					req=req->next_node;
				}
				else
				{
					req=req->next_node;
					break;
				}
				
			}
			else
			{
				//req=ssd->request_tail;
				break;
			}
			
		}	
		else
		{
			//req=ssd->request_tail;
			break;
		}
	} 

	//to update the current time of ssd
	nearest_event_time=find_nearest_event(ssd);
	if (nearest_event_time==MAX_INT64)
	{
		ssd->current_time=ssd->request_queue->time;           
	}
	else
	{   
		next_time=req->time;
		
		if(nearest_event_time<next_time)
		{
			if(req->subs==NULL)     //the request is in the queue but not distribute
				ssd->current_time=req->time;
			//fseek(ssd->tracefile,filepoint,0); 
			else
			{
				if(ssd->current_time<=nearest_event_time)  //the request has been distributed but not finish
				{
					ssd->current_time=nearest_event_time;
					//return -1;
				}	
			}
			
		}
		else
		{

			if(req->subs!=NULL)   //the request has ben distributed 
			{
				ssd->current_time=nearest_event_time;
			}
			else
			{
				ssd->current_time=next_time;
			}
			
		}
	}
	
	

	while(req!=NULL)
	{
		if(req->time>ssd->current_time)
		{
			break;
		}

		for(i=0;i<ssd->parameter->channel_number;i++)
		{          
			if((ssd->channel_head[i].current_state==CHANNEL_IDLE)||((ssd->channel_head[i].next_state==CHANNEL_IDLE)&&(ssd->channel_head[i].next_state_predict_time<=ssd->current_time)))
			{
				pflag=1;                       //所有通道均无请求处理。上边一行，对于全动态分配策略的写请求，不挂在通道上，需要再分配
			}
			else
			{
				pflag=0;
				break;
			}
		}

		if(ssd->parameter->allocation_scheme==1&&pflag==0)
		{
			break;
		}

		ssd->dram->current_time=ssd->current_time;
		//req=ssd->request_tail;       
		lsn=req->lsn;
		lpn=req->lsn/ssd->parameter->subpage_page;
		last_lpn=(req->lsn+req->size-1)/ssd->parameter->subpage_page;
		first_lpn=req->lsn/ssd->parameter->subpage_page;

		if(req->subs!=NULL)
		{
			req=req->next_node;
			continue;
		}
			

		if(req->operation==READ)        
		{		
			while(lpn<=last_lpn) 		
			{
				sub_state=(ssd->dram->map->map_entry[lpn].state&0x7fffffff);
				sub_size=size(sub_state);
				sub=creat_sub_request(ssd,lpn,sub_size,sub_state,req,req->operation);
				lpn++;
			}
		}
		else if(req->operation==WRITE)
		{
			while(lpn<=last_lpn)     	
			{	
				mask=~(0xffffffff<<(ssd->parameter->subpage_page));
				state=mask;
				if(lpn==first_lpn)
				{
					offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-req->lsn);
					state=state&(0xffffffff<<offset1);
				}
				if(lpn==last_lpn)
				{
					offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(req->lsn+req->size));
					state=state&(~(0xffffffff<<offset2));
				}
				sub_size=size(state);

				sub=creat_sub_request(ssd,lpn,sub_size,state,req,req->operation);
				lpn++;
			}
		}
		req->dis=1;
		req=req->next_node;
		if(ssd->parameter->scheduling_algorithm==0)
		{
			break;
		}
	}
	return(ssd);	
}

struct ssd_info *no_buffer_distribute_nosch(struct ssd_info *ssd)
{
	unsigned int lsn,lpn,last_lpn,first_lpn,complete_flag=0, state;
	unsigned int flag=0,flag1=1,active_region_flag=0;           //to indicate the lsn is hitted or not
	struct request *req=NULL,*reqtemp;
	struct sub_request *sub=NULL,*sub_r=NULL,*update=NULL;
	struct local *loc=NULL;
	struct channel_info *p_ch=NULL;

	int64_t nearest_event_time;
	int64_t next_time;
	unsigned int mask=0; 
	unsigned int offset1=0, offset2=0;
	unsigned int sub_size=0;
	unsigned int sub_state=0;
	//KXC:the request is empty,exit and get next request
	if(ssd->request_queue==NULL)
	{
		ssd->empty=1;
		return 0;
	}
	else
	{
		next_time=ssd->request_queue->time;
	}
		
	req=ssd->request_tail; 

	//to update the current time of ssd
	nearest_event_time=find_nearest_event(ssd);
	if (nearest_event_time==MAX_INT64)
	{
		ssd->current_time=ssd->request_queue->time;           
	}
	else
	{   
		//KXC:request is processing to find the next request's arriving time
		reqtemp=ssd->request_queue->next_node;
		while (reqtemp!=NULL)
		{
			if(reqtemp->time==ssd->request_queue->time)
			{
				next_time=reqtemp->time;				
				reqtemp=reqtemp->next_node;
			}
			else
			{
				next_time=reqtemp->time;
				break;
			}

		}
		
		if(nearest_event_time<next_time)
		{
			if(req->subs==NULL)     //the request is in the queue but not distribute
				ssd->current_time=req->time;
			//fseek(ssd->tracefile,filepoint,0); 
			else
			{
				if(ssd->current_time<=nearest_event_time)  //the request has been distributed but not finish
				{
					ssd->current_time=nearest_event_time;
					return -1;
				}	
			}
			
		}
		else
		{
			if (ssd->request_queue_length>=ssd->parameter->queue_length)// the request queue is full
			{
				//fseek(ssd->tracefile,filepoint,0);
				ssd->current_time=nearest_event_time;
				return -1;
			} 
			else
			{
				if(req->subs!=NULL)   //the request has ben distributed 
				{
					ssd->current_time=nearest_event_time;
				}
				else
				{
					//ssd->current_time=next_time;
					ssd->current_time=ssd->current_time>next_time?ssd->current_time:next_time;
				}
			}
		}
	}	
	
	ssd->dram->current_time=ssd->current_time;
	//req=ssd->request_tail;       
	lsn=req->lsn;
	lpn=req->lsn/ssd->parameter->subpage_page;
	last_lpn=(req->lsn+req->size-1)/ssd->parameter->subpage_page;
	first_lpn=req->lsn/ssd->parameter->subpage_page;

	if(req->subs!=NULL)
		return 0;

	if(req->operation==READ)        
	{		
		while(lpn<=last_lpn) 		
		{
			sub_state=(ssd->dram->map->map_entry[lpn].state&0x7fffffff);
			sub_size=size(sub_state);
			sub=creat_sub_request(ssd,lpn,sub_size,sub_state,req,req->operation);
			lpn++;
		}
	}
	else if(req->operation==WRITE)
	{
		while(lpn<=last_lpn)     	
		{	
			mask=~(0xffffffff<<(ssd->parameter->subpage_page));
			state=mask;
			if(lpn==first_lpn)
			{
				offset1=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-req->lsn);
				state=state&(0xffffffff<<offset1);
			}
			if(lpn==last_lpn)
			{
				offset2=ssd->parameter->subpage_page-((lpn+1)*ssd->parameter->subpage_page-(req->lsn+req->size));
				state=state&(~(0xffffffff<<offset2));
			}
			sub_size=size(state);

			sub=creat_sub_request(ssd,lpn,sub_size,state,req,req->operation);
			lpn++;
		}
	}
	req->dis=1;	
	return ssd;
}
