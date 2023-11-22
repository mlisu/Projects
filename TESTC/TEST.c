/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        15       /* Number of identical tasks                          */
#define  N_TASKS_QM						10

#define SIZE 63
#define QPRIOR 5
#define WYSW_MSG_QUEUE_SIZE 3
#define BUFSIZE 20
#define TEMP65535 65535

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
INT8U         TaskNr[N_TASKS];                      /* Parameters to pass to each task               */
OS_EVENT     *VSem;

OS_EVENT     *TaskMbox[5];
OS_EVENT     *SendMbox;

OS_EVENT       *BufMsgQueue;
void           *BufMsgQueueTbl[2];

OS_EVENT       *WyswMsgQueue;
void           *WyswMsgQueueTbl[WYSW_MSG_QUEUE_SIZE];

OS_EVENT       *ObcMsgQueue[5];
void           *ObcMsgQueueTbl[5][1];

OS_EVENT       *SendMsgQueue;
void           *SendMsgQueueTbl[2];

OS_STK       KtaskStk[TASK_STK_SIZE];
OS_STK       WtaskStk[TASK_STK_SIZE];
OS_STK       BtaskStk[TASK_STK_SIZE];
OS_STK		 SendtaskStk[TASK_STK_SIZE];

typedef struct {
	INT8U	nr;
	INT32U	obciazenie;
	INT32U	licznik;
	char	*buffer;
	INT8U	nrob;
} TASK_DATA;
typedef struct {
	INT32U	obc[5];
	INT8U	nrob[5];
} OBCG;
typedef struct {// do SendMsgQ
	INT32U *ob;
	INT8U nrob;
	INT8U nrz;
	INT16U iluWysl;
} OB;
typedef struct {//do BufMsgqQ
	INT8U nrob;
	INT8U nrz;
	INT16S key;
} CTR;
OBCG obcg = {{0},{0}};

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task(void *data);                       /* Function prototypes of tasks                  */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);
		
		void  Ktask(void *pdata);
		void  Wtask(void *pdata); 
		void  Btask(void *pdata);
		void  Sendtask(void *pdata);
		
		void  Qtask(void *pdata);
		void  Mtask(void *pdata);
		void  Stask(void *pdata);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    VSem   = OSSemCreate(1);                          /* Random number semaphore                  */

    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[100];
    INT16S     key;
	INT8U  err;
	INT8U i;
	TASK_DATA Data;

    pdata = pdata;
	
	Data.nr = 0;

    TaskStartDispInit();                                   /* Initialize the display                   */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
	
	SendMbox = OSMboxCreate((void *)0);
	
	for(i = 0; i < 5; i++){
		TaskMbox[i] = OSMboxCreate((void *)0);
		ObcMsgQueue[i] = OSQCreate(&ObcMsgQueueTbl[i][0], 1);
	}
	
	WyswMsgQueue = OSQCreate(&WyswMsgQueueTbl[0], WYSW_MSG_QUEUE_SIZE);
	SendMsgQueue = OSQCreate(&SendMsgQueueTbl[0], 2);
	BufMsgQueue = OSQCreate(&BufMsgQueueTbl[0], 2);

    TaskStartCreateTasks();                                /* Create all the application tasks         */

    for (;;) {
        TaskStartDisp();                                  /* Update the display                       */

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
		OSQPost(WyswMsgQueue, (void *)&Data);
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void)
{
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
    PC_DispStr( 0,  0, "01234567890123456789012345678901234567890123456789012345678901234567890123456789", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr( 0,  1, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  5, "Typ  Nr        Obciazenie      Licznik      Delta      Stan                     ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 10, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 12, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 13, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 14, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 15, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 16, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 17, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 18, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 21, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp (void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

#if OS_TASK_STAT_EN > 0
    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
#endif

    sprintf(s, "%5lu", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%1d.%02d", OSVersion() / 100, OSVersion() % 100); /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
    INT8U  i;
	INT8U prior;

    for (i = 0; i < N_TASKS; i++) {
        prior = QPRIOR + i;
		TaskNr[i] = i+1;
        if(i < 5)
			OSTaskCreate(Qtask, (void *)&TaskNr[i], &TaskStk[i][TASK_STK_SIZE - 1], prior);
		else if(i < 10)
			OSTaskCreate(Mtask, (void *)&TaskNr[i], &TaskStk[i][TASK_STK_SIZE - 1], prior);
		else
			OSTaskCreate(Stask, (void *)&TaskNr[i], &TaskStk[i][TASK_STK_SIZE - 1], prior);
    }

	OSTaskCreate(Ktask, (void *)0, &KtaskStk[TASK_STK_SIZE - 1], 1);
	OSTaskCreate(Wtask, (void *)0, &WtaskStk[TASK_STK_SIZE - 1], 3);
	OSTaskCreate(Btask, (void *)0, &BtaskStk[TASK_STK_SIZE - 1], 2);
	OSTaskCreate(Sendtask, (void *)0, &SendtaskStk[TASK_STK_SIZE - 1], 4);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/
void  Qtask (void *pdata){

	TASK_DATA	Data;
	INT32U i = 0, j = 0;
	OB *OBdata;

	Data.licznik = 0;
	Data.nr = *(INT8U*)pdata;
	Data.obciazenie = 0;

	for(;;){
		
		if((OBdata = OSQAccept(ObcMsgQueue[Data.nr - 1])) != NULL){
			Data.obciazenie = *OBdata->ob;
			Data.nrob = OBdata->nrob;
		}
	
		OSQPost(WyswMsgQueue, &Data);

		for (i = 0; i < Data.obciazenie; i++) j++;
		Data.licznik++;		
		
		OSTimeDly(1);
	}
	
}

void  Mtask (void *pdata){

	TASK_DATA	Data;
	INT32U i = 0, j = 0;
	OB *OBdata;

	Data.licznik = 0;
	Data.nr = *(INT8U*)pdata;
	Data.obciazenie = 0;

	for(;;){
		if((OBdata = OSMboxAccept(TaskMbox[Data.nr-6])) != NULL){
			Data.obciazenie = *OBdata->ob;
			Data.nrob = OBdata->nrob;	
		}
		
		OSQPost(WyswMsgQueue, (void *)&Data);

		for (i = 0; i < Data.obciazenie; i++) j++;
		Data.licznik++;		
		OSTimeDly(1);
	}
}

void  Stask (void *pdata){

	TASK_DATA	Data;
	INT32U i = 0, j = 0;
	INT8U  err;

	Data.licznik = 0;
	Data.nr = *(INT8U*)pdata;
	Data.obciazenie = 0;

	for(;;){	
		OSSemPend(VSem, 0, &err);
			Data.obciazenie = obcg.obc[Data.nr-11];
			Data.nrob = obcg.nrob[Data.nr-11];
		OSSemPost(VSem);	
		
		OSQPost(WyswMsgQueue, (void *)&Data);

		for (i = 0; i < Data.obciazenie; i++) j++;
		Data.licznik++;
		
		OSTimeDly(1);
	}
}

void Ktask (void *pdata){
	
	CTR K;
	K.nrz = 20;
	
	pdata = pdata;
	
	for (;;){

			if (PC_GetKey(&K.key) == TRUE) {
				if (K.key == 0x1B) PC_DOSReturn();
				else OSQPost(BufMsgQueue, (void *)&K);;
			}
		
		OSTimeDly(20);
	}	
}

void Wtask (void *pdata){

	INT8U err;
	char  *rxmsg;
	TASK_DATA *Data;
	char s[80];
	char taskType = 'Q';
	INT8U i = 0;
	INT32U Prev[15] = {0};
	INT32U Act[15] = {0};
	CTR Ctr;

	pdata = pdata;

	for (;;) {
		Data = OSQPend(WyswMsgQueue, 0, &err);
		if (!Data->nr){
			for (i = 0; i < N_TASKS; i++){
				PC_DispStr(46, i + 6, "    ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				PC_DispStr(46, i + 6, ltoa(Act[i] - Prev[i], s, 10), DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				Prev[i] = Act[i];
			}
		}
		else if(Data->nr < 16){
		
			if(Data->nr < 6) taskType = 'Q';
			else if (Data->nr < 11) taskType = 'M';
			else taskType = 'S';
			
			PC_DispChar(2, Data->nr + 5, taskType, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispStr(5, Data->nr + 5, itoa(Data->nr, s, 10), DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispStr(15, Data->nr + 5, "          ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispStr(15, Data->nr + 5, ultoa(Data->obciazenie, s, 10), DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispStr(32, Data->nr + 5, "      ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			PC_DispStr(32, Data->nr + 5, ultoa(Data->licznik, s, 10), DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);

			Act[Data->nr - 1] = Data->licznik;
			
			Ctr.nrob = Data->nrob;
			Ctr.nrz = Data->nr;	
			OSQPost(BufMsgQueue, (void *)&Ctr);
		}else{
			rxmsg = Data->buffer;
			if (rxmsg != 0){
				if (rxmsg[(SIZE-1)] == 13) {
					PC_DispStr(0, 2, "                                                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(0, 4, "                                                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(0, 4, rxmsg, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(0, 2, "            ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);//kasowanie kokmunikatu bufor pelny
				} else if (rxmsg[(SIZE-1)] == 97) {
					PC_DispStr(0, 2, "                                                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				} else if (rxmsg[(SIZE-1)] == 98) {
					PC_DispStr(0, 2, "                                                                  ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(0, 2, rxmsg, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				}else if(rxmsg[(SIZE-1)] == 14){
					PC_DispStr(0, 0, "bufor pelny", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				} else PC_DispStr(0, 2, rxmsg, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			}
		}
	}	
}

void Btask (void *pdata){
	
	char buf[SIZE];
    INT8U	err;
	INT8U	i = 0;
	BOOLEAN control = 0;
	INT16U	j;
	char s[80];
	TASK_DATA Data;
	INT32U obcBuf[BUFSIZE];
	INT8S pocz = 0, kon = 0;
	CTR *Ctr;
	OB ObcSend[N_TASKS];
	INT8U ileBuf = 0;
	INT8U ostObc = 0;
	INT8U ostObcTemp = 0;
	INT16U mask[BUFSIZE];
	INT16U nrzMask = 0;
	BOOLEAN flag = 0;
	
	pdata = pdata;
	Data.nr = 16;
	Data.buffer = buf;
	for(i = 0; i < N_TASKS; i++){
		ObcSend[i].nrz = i + 1;
		ObcSend[i].iluWysl = 0;
	}
	for(i = 0; i < BUFSIZE; i++) mask[i] = 1;
	mask[0] = TEMP65535;
	i = 0;
	
	for (;;) {

		Ctr = OSQPend(BufMsgQueue, 0, &err);
			if(Ctr->nrz == 20){
				if (i < (SIZE - 3) || Ctr->key == 13 || control || Ctr->key == 8) {
					if (Ctr->key == 13) {
						if(ileBuf < BUFSIZE){

							ileBuf++;
							
							PC_DispStr(60, 1, "   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
							PC_DispStr(60, 1, itoa(BUFSIZE - ileBuf, s, 10), DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
							
							ostObcTemp = ostObc;
							obcBuf[kon] = atol(buf);
							kon++;
							if(kon == BUFSIZE) kon = 0;
							if(kon == 0) ostObc = BUFSIZE - 1;
							else ostObc = kon - 1;
							if(mask[ostObcTemp] > 1){
								for(j = 1; j <= N_TASKS; j++){
									if(((1 << j) & mask[ostObcTemp]) != 0){				
										ObcSend[j-1].ob = obcBuf + ostObc;
										ObcSend[j-1].nrob = ostObc;
										if(!flag){
											flag = 1;
											ObcSend[j-1].iluWysl = mask[ostObcTemp];
											OSMboxPost(SendMbox, (void *)&ObcSend[j-1]);
										}
									}
								}
								flag = 0;
								if(ileBuf == 1) mask[ostObcTemp] = 1;
							}
							i = 0;
							buf[(SIZE-1)] = 13;
						} else buf[(SIZE-1)] = 14;				
					} else if (control) {
						control = 0;
						if (Ctr->key == 83){
							i = 0;
							buf[(SIZE-1)] = 97;
						}
					} else if (Ctr->key == 8) {
						if (i) {
							buf[--i] = '\0';
							buf[(SIZE-1)] = 98;
						}
					} else {
						if(isdigit(Ctr->key)){
							buf[i] = Ctr->key;
							i++;
							buf[i] = '\0';
							buf[(SIZE-1)] = '\0';
						}
					}
					OSQPost(WyswMsgQueue, (void *)&Data);
				}
			
				if (Ctr->key == 0) control = 1;
			} else if(ileBuf){
				
				nrzMask = 1 << Ctr->nrz;
				mask[Ctr->nrob] |= nrzMask; //= nrzMask | mask[Ctr->nrob];
				
				if(Ctr->nrob != ostObc){
					ObcSend[Ctr->nrz - 1].nrob = Ctr->nrob + 1;
					if(ObcSend[Ctr->nrz - 1].nrob == BUFSIZE) ObcSend[Ctr->nrz - 1].nrob = 0;
					ObcSend[Ctr->nrz - 1].iluWysl = 0;
					ObcSend[Ctr->nrz - 1].ob = obcBuf + ObcSend[Ctr->nrz - 1].nrob;
					OSMboxPost(SendMbox, (void *)&ObcSend[Ctr->nrz - 1]);
				}
				if(mask[Ctr->nrob] == TEMP65535){
					if(++pocz == BUFSIZE) pocz = 0;
					ileBuf--;
					mask[Ctr->nrob] = 1;
					if(!ileBuf) mask[ostObc] = TEMP65535;
					PC_DispStr(60, 1, "   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
					PC_DispStr(60, 1, itoa(BUFSIZE - ileBuf, s, 10), DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
				}
			}
	}
}

void Sendtask (void *pdata){
	

	INT8U	err;
	INT8U	err2;
	INT8U	i;
	OB *ObcRec;
	INT8U flag = 0;
	INT8U j = 0;
	
	pdata = pdata;

	for (;;) {
		
		ObcRec = OSMboxPend(SendMbox, 0, &err);
		
		if(ObcRec->iluWysl){
			for(i = 1; i <= N_TASKS; i++){
				if(1 << i & ObcRec->iluWysl){
					if(!flag){
						flag = 1;
						j = i;
					}
					if(i < 6)
						OSQPost(ObcMsgQueue[i-1], (void *)(ObcRec+i-j));
					else if(i < 11) OSMboxPost(TaskMbox[i-6], (void *)(ObcRec+i-j));
					else{
						OSSemPend(VSem, 0, &err2);
							obcg.nrob[i-11] = (ObcRec+i-j)->nrob;
							obcg.obc[i-11] = *(ObcRec+i-j)->ob;
						OSSemPost(VSem);	
					}
				}
			}
			flag = 0;
		}else if(ObcRec->nrz <6) OSQPost(ObcMsgQueue[ObcRec->nrz - 1], (void *)ObcRec);
		else if(ObcRec->nrz <11) OSMboxPost(TaskMbox[ObcRec->nrz - 6], (void *)ObcRec);
		else{
			OSSemPend(VSem, 0, &err);
				obcg.nrob[ObcRec->nrz - 11] = ObcRec->nrob;
				obcg.obc[ObcRec->nrz - 11] = *ObcRec->ob;
			OSSemPost(VSem);
		}
	}
}
