/*
 * Excelfore Universal IPC and Configuration code generator
 * Copyright (C) 2020 Excelfore Corporation (https://excelfore.com)
 *
 * This file is part of Excelfore-unipac.
 *
 * Excelfore-unipac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Excelfore-unipac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Excelfore-unipac.  If not, see
 * <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <xl4combase/combase.h>
#include <xl4unibase/unibase_binding.h>
#include "sample_configs.h"
#include "sample_ipcconfigs.h"

static int values_print(void)
{
	char *sval;
	char sv[128];
	void *value;
	int vsize;
	int item;

	item=VALUE_A_01;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item);
	sprintf(sv, "%d", *(int32_t*)value);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_A_01 %s\n", sval);
	if(strcmp(sval, sv)) goto erexit;
	free(sval);

	item=VALUE_C_01;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item);
	sprintf(sv, "%f", *(double*)value);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_C_01 %s\n", sval);
	if(strcmp(sval, sv)) goto erexit;
	free(sval);

	item=VALUE_E_04;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_element_size(item)*sampleconf_get_item_element_num(item);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_E_04 %s\n", sval);
	free(sval);

	item=VALUE_B_01;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item)*sampleconf_get_item_element_num(item);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_B_01 %s\n", sval);
	free(sval);

	item=VALUE_B_02;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item);
	sprintf(sv, "\"%s\"", (char *)value);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_B_02 %s\n", sval);
	if(strcmp(sval, sv)) goto erexit;
	free(sval);

	item=VALUE_F_03;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item)*sampleconf_get_item_element_num(item);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_F_03 %s\n", sval);
	free(sval);

	item=VALUE_P;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_P %s\n", sval);
	free(sval);

	item=VALUE_R;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_R index=0 %s\n", sval);
	free(sval);
	// index strats from 2 and vsize is up to 5, so it shows index=2,3,4
	sval=sampleitem_values_to_string(item, 2, -1, value, vsize*5, true);
	if(!sval) return -1;
	printf("VALUE_R index=2..4 %s\n", sval);
	free(sval);

	// index=1, field .f1
	vsize=sampleconf_get_item_value_size(item)*sampleconf_get_item_element_num(item);
	sval=sampleitem_values_to_string(item, 1, 1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_R[1].f1 %s\n", sval);
	free(sval);
	sval=sampleitem_values_to_string(item, 1, 2, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_R[1].f2 %s\n", sval);
	free(sval);

	item=VALUE_S;
	value=sampleconf_get_item(item);
	vsize=sampleconf_get_item_value_size(item)*sampleconf_get_item_element_num(item);
	sval=sampleitem_values_to_string(item, 0, -1, value, vsize, true);
	if(!sval) return -1;
	printf("VALUE_S all indices %s\n", sval);

	if(samplewrite_config_file("sample_updated.conf")){
		printf("failed to write the data into 'sample_updated.conf'\n");
		goto erexit;
	}
	free(sval);
	return 0;
erexit:
	if(sval) free(sval);
	return -1;
}

// text mode client
static void *client_thread1(void *arg)
{
	int fd;
	int i,res;
	int *thread_stop=(int *)arg;
	char *scmd;
	char rdata[1024];
	char *ipcclnode="/tmp/sample_ipcconfcl";

	for(i=0;i<10;i++){
		res=cb_ipcsocket_init(&fd, ipcclnode, NULL, "/tmp/sample_ipcconf");
		if(!res) break;
		usleep(10000);
	}
	if(res) return NULL;

	printf("----- Text mode client -----\n");
	scmd="TTTT R VALUE_A_01";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) {
		printf("%s:res=%d, read errr,%s\n", __func__, res, strerror(errno));
		goto erexit;
	}
	rdata[res]=0;
	if(strcmp(rdata, "TTTT R VALUE_A_01 1234"))  goto erexit;
	printf("%s\n-----\n", rdata);

	scmd="TTTT R VALUE_A_02";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res!=0) goto erexit; //VALUE_A_02 is not IPC readable

	scmd="TTTT R VALUE_E_02";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) goto erexit;
	rdata[res]=0;
	printf("%s\n-----\n", rdata);

	scmd="TTTT R VALUE_E_04";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) goto erexit;
	rdata[res]=0;
	printf("%s\n-----\n", rdata);

	scmd="TTTT W VALUE_E_01 {0x1000,0x2000}";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	scmd="TTTT R VALUE_E_01";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) goto erexit;
	rdata[res]=0;
	printf("%s\n-----\n", rdata);

	scmd="TTTT W VALUE_E_01[1] 0x3000";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	scmd="TTTT R VALUE_E_01";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) goto erexit;
	rdata[res]=0;
	printf("%s\n-----\n", rdata);

	scmd="TTTT D";
	printf("send cmd=%s\n", scmd);
	res=write(fd, scmd, strlen(scmd));

	cb_ipcsocket_close(fd, ipcclnode, NULL);
	*thread_stop=1;
	return NULL;
erexit:
	cb_ipcsocket_close(fd, ipcclnode, NULL);
	*thread_stop=2;
	return NULL;
}

// binary mode client
static void *client_thread2(void *arg)
{
	int fd;
	int i,res;
	char *ipcclnode="/tmp/sample_ipcconfc2";
	int *thread_stop=(int *)arg;
	uint8_t sdata[1024];
	uint8_t rdata[1024];
	sampleipcdata_t *ripcd=(sampleipcdata_t *)rdata;
	sampleipcdata_t *ipcd=(sampleipcdata_t *)sdata;

	for(i=0;i<10;i++){
		res=cb_ipcsocket_init(&fd, ipcclnode, NULL, "/tmp/sample_ipcconf");
		if(!res) break;
		usleep(10000);
	}
	if(res) return NULL;

	printf("----- Binary mode client -----\n");
	memset(sdata, 0, sizeof(sdata));
	ipcd->magic=sampleIPCDATA_BMAGIC;
	ipcd->cmd=sampleIPCCMD_READ;

	ipcd->item=VALUE_A_01;
	ipcd->index=-1;
	ipcd->findex=-1;
	res=write(fd, sdata, sizeof(sampleipcdata_t));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) {printf("cmd error, res=%d\n", res);goto erexit;}
	if(*((int32_t *)ripcd->data)!=1234) goto erexit;
	printf("%d\n-----\n", *((int32_t *)ripcd->data));

	ipcd->item=VALUE_S;
	ipcd->index=-1;
	ipcd->findex=-1;
	res=write(fd, sdata, sizeof(sampleipcdata_t));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) {printf("cmd error, res=%d\n", res);goto erexit;}
	if(ripcd->size!=sizeof(ABC_02_t)*5) goto erexit;
	{
		ABC_02_t *v=(ABC_02_t *)ripcd->data;
		for(i=0;i<5;i++){
			if(v[i].NumX!=3) goto erexit;
			if(v[i].FlagY!=true) goto erexit;
			if(strcmp(v[i].StringZ,"pq")) goto erexit;
		}
	}
	printf("%s\n-----\n", "VALUE_S");

	ipcd->item=VALUE_R;
	ipcd->index=2;
	ipcd->findex=-1;
	res=write(fd, sdata, sizeof(sampleipcdata_t));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) {printf("cmd error, res=%d\n", res);goto erexit;}
	if(ripcd->size!=sizeof(ABC_01_t)) goto erexit;
	{
		ABC_01_t *v=(ABC_01_t *)ripcd->data;
		if(v->f0!=0) goto erexit;
		if(strcmp(v->f1,"y")) goto erexit;
		if(v->f2!=2.0) goto erexit;
		if(v->f3!=true) goto erexit;
		if(v->f4[0]!=1) goto erexit;
		if(v->f4[1]!=2) goto erexit;
		if(v->f4[2]!=3) goto erexit;
		if(v->f5!=10) goto erexit;
		if(v->f6!=20) goto erexit;
		if(v->f7!=30) goto erexit;
	}
	printf("%s\n-----\n", "VALUE_R[2]");

	ipcd->item=VALUE_R;
	ipcd->cmd=sampleIPCCMD_WRITE;
	ipcd->index=2;
	ipcd->findex=1;
	ipcd->size=4;
	memcpy(ipcd->data, "XYZ", 4);
	res=write(fd, sdata, sizeof(sampleipcdata_t));
	ipcd->cmd=sampleIPCCMD_READ;
	res=write(fd, sdata, sizeof(sampleipcdata_t));
	res=cb_fdread_timeout(fd, rdata, 1024, 100);
	if(res<=0) {printf("cmd error, res=%d\n", res);goto erexit;}
	if(strcmp((char*)ripcd->data,"XYZ")) goto erexit;
	printf("%s\n-----\n", "VALUE_R[2].f1 update");

	ipcd->cmd=sampleIPCCMD_DISCONNECT;
	res=write(fd, sdata, sizeof(sampleipcdata_t));

	cb_ipcsocket_close(fd, ipcclnode, NULL);
	*thread_stop=1;
	return NULL;
erexit:
	cb_ipcsocket_close(fd, ipcclnode, NULL);
	*thread_stop=2;
	return NULL;
}

int ipc_update_cb(void *cbdata, int item, int index, int findex)
{
	char istr[6]={0,};
	char fstr[6]={0,};
	char *vname;
	vname=sampleconfig_item_strings(item);
	if(!vname){
		UB_LOG(UBL_ERROR, "%s:wrong item=%d\n", __func__, item);
		return -1;
	}
	if(index>=0 && index<1000) snprintf(istr, 6, "[%d]", index);
	if(findex>=0 && findex<1000) snprintf(fstr, 6, ".f%d", findex);
	printf("Update:%s%s%s\n", vname, istr, fstr);
	return 0;
}

int ipc_client(int mode)
{
	CB_THREAD_T clthread;
	int thread_stop=0;
	char c;
	int res;
	sampleipcserver_init("/tmp/sample_ipcconf", 0, true);
	if(mode==0){
		CB_THREAD_CREATE(&clthread, NULL, client_thread1, &thread_stop);
	}else if(mode==1){
		CB_THREAD_CREATE(&clthread, NULL, client_thread2, &thread_stop);
	}else{
		int32_t v=1111;
		upstring9 v2;
		sampleitem_extend_t eid={"EXTVAR1", &v,
					 {sizeof(int32_t), 1, 1, VT_INT32_T}, UPIPC_RW};
		strcpy(v2, "hello");
		sampleitem_extend_t eid2={"EXTVAR2", v2,
					 {sizeof(upstring9), 1, 1, UPSTRING_TYPE_BASE+9},
					  UPIPC_RW};
		res=sampleregister_extend_item(&eid);
		res=sampleregister_extend_item(&eid2);
		sampleipcserver_set_update_cb(ipc_update_cb, NULL);

		res=read(0, &c, 1);
		sampleipcserver_close();
		if(res<0) return -1;
		return 0;
	}
	while(!thread_stop) usleep(100000);
	CB_THREAD_JOIN(clthread, NULL);
	sampleipcserver_close();
	if(thread_stop>1) return -1;
	return 0;
}

int main(int argc, char *argv[])
{
	unibase_init_para_t init_para;
	int res=0;
	ubb_default_initpara(&init_para);
	init_para.ub_log_initstr=UBL_OVERRIDE_ISTR("4,ubase:45,combase:45,unip:66", "UBL_UNIP");
	unibase_init(&init_para);
	ubb_memory_out_init(NULL, 64*1024);
	while(true){
		if(argc>1 && argv[1][0]=='r'){
			ipc_client(2);
			break;
		}
		if((res=values_print())) break;
		if((res=ipc_client(0))) break;
		if((res=ipc_client(1))) break;
		break;
	}
	ubb_memory_out_close();
	unibase_close();
	return 0;
}
