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
#include <getopt.h>
#include <xl4combase/combase.h>
#include <xl4unibase/unibase_binding.h>
#include "_CONFPREFIX__configs.h"
#include "_CONFPREFIX__ipcconfigs.h"

/*_CONF_GENERATED_*/

#define DEFAULT_IPCSERVER_NODE "/tmp/_CONFPREFIX__ipcconf"
#define DEFAULT_IPCCLIENT_NODE "/tmp/_CONFPREFIX__ipcclient"
static int print_usage(char *pname)
{
	char *s;
	s=strrchr(pname,'/');
	if(s) s+=1; else s=pname;
	printf("%s [options]\n", s);
	printf("-h|--help: this help\n");
	printf("-p|--port udp port_number: use UDP mode connection with port_number\n");
	printf("-a|--addr Domain socket server inode or IP address, "
	       "default=%s\n", DEFAULT_IPCSERVER_NODE);
	printf("-l|--laddr Domain socket client inode or IP address, "
	       "default=%s\n", DEFAULT_IPCCLIENT_NODE);
	return -1;
}

typedef struct ipcoptions {
	uint16_t port;
	char *addr;
	char *laddr;
}ipcoptions_t;

static int set_options(ipcoptions_t *ipcopt, int argc, char *argv[])
{
	int oc;
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"port", required_argument, 0, 'p'},
		{"addr", required_argument, 0, 'a'},
		{"laddr", required_argument, 0, 'l'},
	};
	while((oc=getopt_long(argc, argv, "hp:a:l:", long_options, NULL))!=-1){
		switch(oc){
		case 'p':
			ipcopt->port=strtol(optarg, NULL, 0);
			break;
		case 'a':
			ipcopt->addr=optarg;
			break;
		case 'l':
			ipcopt->laddr=optarg;
			break;
		case 'h':
		default:
			return print_usage(argv[0]);
		}
	}
	return 0;
}

static void print_variables(void)
{
	int i=0, c=0;
	char *vstr;
	uint8_t ipcon;
	char *onstr="";
	while(true){
		vstr=_CONFPREFIX_config_item_strings(i);
		if(!vstr && i<_CONFPREFIX_ITEM_EXTEND_BASE) {
			i=_CONFPREFIX_ITEM_EXTEND_BASE;
			continue;
		}
		if(!vstr) break;
		ipcon=_CONFPREFIX_conf_get_ipcon(i);
		if(ipcon==UPIPC_R) onstr="R";
		else if(ipcon==UPIPC_W) onstr="W";
		else if(ipcon==UPIPC_RW) onstr="RW";
		if(ipcon!=UPIPC_N){
			printf("%d:%s(%s)  ", i, vstr, onstr);
			c++;
		}
		i++;
		if(c%4==0) printf("\n");
	}
	if(c%4!=0) printf("\n");
	printf("\n");
	printf("q:quit, b:binary mode, t:text mode,\n"
	       "l:read dynamically registered variables, Return:print variable list\n");
	printf("ITEM_NUMBER [-i index] [-f field_index] [-u update_value]\n");
	printf("----------\n");
}

static int textmode_cmd_send(int fd, char *sval, char *ud, int index, int findex)
{
	char istr[6]={0,};
	char fstr[6]={0,};
	char sdata[1500];
	int res;

	if(!ud) ud="";
	if(index>=0 && index<1000) snprintf(istr, 6, "[%d]", index);
	if(findex>=0 && findex<1000) snprintf(fstr, 6, ".f%d", findex);
	snprintf(sdata, 1500, "TTTT %c %s%s%s %s", ud[0]?'W':'R', sval, istr, fstr, ud);
	UB_LOG(UBL_DEBUG, "send cmd=%s\n", sdata);
	res=write(fd, sdata, strlen(sdata));
	if(res!=strlen(sdata)) {
		UB_LOG(UBL_ERROR, "%s:send res=%d\n", __func__, res);
		return -1;
	}
	return 0;
}

static int binarymode_cmd_send(int fd, int item, char *ud, int index, int findex)
{
	char sdata[1500];
	_CONFPREFIX_ipcdata_t *ipcd=(_CONFPREFIX_ipcdata_t *)sdata;
	int offset, rsize, res;
	void *values=NULL;
	int udsize;
	ipcd->magic=_CONFPREFIX_IPCDATA_BMAGIC;
	ipcd->cmd=(ud!=NULL)?_CONFPREFIX_IPCCMD_WRITE:_CONFPREFIX_IPCCMD_READ;
	ipcd->item=item;
	ipcd->index=index;
	ipcd->findex=findex;
	rsize=0;
	if(ipcd->cmd==_CONFPREFIX_IPCCMD_WRITE){
		udsize=strlen(ud);
		rsize=_CONFPREFIX_stritem_values(item, index, findex,
						 &values, &offset, &ud, &udsize);
		if(rsize<0){
			UB_LOG(UBL_ERROR, "%s:data format error, item=%d, index=%d, findex=%d\n",
			       __func__, ipcd->item, ipcd->index, ipcd->findex);
			return -1;
		}
		if(rsize>sizeof(sdata)-sizeof(_CONFPREFIX_ipcdata_t)+4){
			UB_LOG(UBL_ERROR, "%s:data too big\n", __func__);
			return -1;
		}
		memcpy(ipcd->data, values, rsize);
		if(values) free(values);
	}
	ipcd->size=rsize;
	res=write(fd, sdata, sizeof(_CONFPREFIX_ipcdata_t)+ipcd->size-4);
	if(res!=sizeof(_CONFPREFIX_ipcdata_t)+ipcd->size-4) {
		UB_LOG(UBL_ERROR, "%s:send res=%d\n", __func__, res);
		return -1;
	}
	return 0;
}

static int send_qurey_vlist(int fd, bool textmode)
{
	int ssize, res;
	if(textmode){
		ssize=6;
		res=write(fd, "TTTT Q", ssize);
	}else{
		_CONFPREFIX_ipcdata_t ipcd;
		ssize=sizeof(ipcd);
		memset(&ipcd, 0, ssize);
		ipcd.magic=_CONFPREFIX_IPCDATA_BMAGIC;
		ipcd.cmd=_CONFPREFIX_IPCCMD_QEVLIST;
		res=write(fd, &ipcd, ssize);
	}
	if(res!=ssize){
		UB_LOG(UBL_ERROR, "%s:send res=%d\n", __func__, res);
		return -1;
	}
	return 0;
}

static int console_cmd_proc(int fd, bool *textmode)
{
	int i, lsize, item, index, findex;
	char line[1500];
	char *lp=line;
	char *ep, *sval;
	char *ud=NULL;
	char *litems[7]={NULL,};
	lsize=read(0, line, sizeof(line));
	if(lsize<=0) {
		UB_LOG(UBL_ERROR, "%s:stdin error: %s\n", __func__, strerror(errno));
		return -1;
	}
	line[lsize-1]=0; // replace \n to null
	if(line[0]=='q'){
		return 1;
	}else if(*lp=='t') {
		*textmode=true;
		return 0;
	}else if(*lp=='b') {
		*textmode=false;
		return 0;
	}else if(*lp=='l') {
		send_qurey_vlist(fd, *textmode);
		return 0;
	}else if(*lp==0) {
		print_variables();
		return 0;
	}
	for(i=0;i<7;i++){
		_CONFPREFIX_skip_chars(&lp, &lsize, ' ', '\t');
		litems[i]=lp;
		if(!_CONFPREFIX_skip_to_chars(&lp, &lsize, ' ', '\t')) break;
		*lp=0;
		lp+=1;
		lsize-=1;
	}
	item=strtol(litems[0], &ep, 0);
	index=-1;
	findex=-1;
	if(ep>litems[0])
		sval=_CONFPREFIX_config_item_strings(item);
	else
		sval=litems[0];
	if(!sval) {
		UB_LOG(UBL_WARN,"%s:bad item number=%d\n", __func__, item);
		return 0;
	}
	for(i=1;i<7;i++){
		if(!litems[i]) break;
		if(!strcmp(litems[i],"-i")){
			index=strtol(litems[++i], &ep, 0);
			continue;
		}
		if(!strcmp(litems[i],"-f")){
			findex=strtol(litems[++i], &ep, 0);
			continue;
		}
		if(!strcmp(litems[i],"-u")){
			ud=litems[++i];
			continue;
		}
		break;
	}
	if(*textmode)
		return textmode_cmd_send(fd, sval, ud, index, findex);
	return binarymode_cmd_send(fd, item, ud, index, findex);
}

static int qevlist_proc(uint8_t *data, int size, bool textmode)
{
	_CONFPREFIX_item_extend_t eid;
	int esize=sizeof(_CONFPREFIX_item_extend_t);
	int count=0;
	int res;
	while(size>0){
		if(textmode){
			data[size]=0;
			memset(&eid, 0, sizeof(eid));
			res=sscanf((char*)data, "{%s %p {%d %d %d %d} %hhu}",
				   eid.name, &eid.valuep, &eid.vsizes[0],
				   &eid.vsizes[1], &eid.vsizes[2], &eid.vsizes[3],
				   &eid.ipcon);
			if(res!=7) break;
			if(!_CONFPREFIX_skip_to_chars((char**)&data, &size, '}', 0)) break;
			data++;
			size--;
			if(!_CONFPREFIX_skip_to_chars((char**)&data, &size, '}', 0)) break;
			data++;
			size--;
			_CONFPREFIX_skip_chars((char**)&data, &size, ',', 0);
		}else{
			if(size<esize) break;
			memcpy(&eid, data+count*esize, esize);
			size-=esize;
		}
		eid.valuep=NULL; // no data in the client
		_CONFPREFIX_register_extend_item(&eid);
		count++;
	}
	printf("update %d dynamically registered variables\n", count);
	return 0;
}

static int read_proc_textmode(uint8_t *rdata, int size)
{
	char *cmdt=NULL;
	_CONFPREFIX_skip_chars((char**)&rdata, &size, ' ', '\t');
	if(rdata[0]=='R'){
		cmdt="Read";
	}else if(rdata[0]=='N'){
		cmdt="Notice";
	}else if(rdata[0]=='Q'){
		rdata+=2; // skip 'Q' command
		size-=2;
		return qevlist_proc(rdata, size, true);
	}else{
		UB_LOG(UBL_ERROR, "%s:'%c' is not supported\n", __func__, rdata[0]);
		return -1;
	}
	printf("%s: %s\n", cmdt, (char*)(rdata+2));
	return 0;
}

static int read_proc_binarymode(_CONFPREFIX_ipcdata_t *ipd, int size)
{
	char istr[6]={0,};
	char fstr[6]={0,};
	char *vname;
	char *vstr;
	char *cmdt=NULL;

	if(ipd->cmd==_CONFPREFIX_IPCCMD_READ) cmdt="Read";
	else if(ipd->cmd==_CONFPREFIX_IPCCMD_NOTICE) cmdt="Notice";
	else if(ipd->cmd==_CONFPREFIX_IPCCMD_QEVLIST)
		return qevlist_proc(ipd->data, ipd->size, false);
	else return -1;

	vname=_CONFPREFIX_config_item_strings(ipd->item);
	if(!vname){
		printf("no variable for item=%d\n", ipd->item);
		return -1;
	}
	vstr=_CONFPREFIX_item_values_to_string(ipd->item, ipd->index, ipd->findex,
					       ipd->data, ipd->size, false);
	if(!vstr){
		printf("no value string for item=%d\n", ipd->item);
		return -1;
	}
	if(ipd->index>=0 && ipd->index<1000) snprintf(istr, 6, "[%d]", ipd->index);
	if(ipd->findex>=0 && ipd->findex<1000) snprintf(fstr, 6, ".f%d", ipd->findex);
	printf("%s: %s%s%s %s\n", cmdt, vname, istr, fstr, vstr);
	free(vstr);
	return 0;
}

static int read_from_server(int fd)
{
	int res;
	uint8_t rdata[1500]={0,};
	_CONFPREFIX_ipcdata_t *ipd=(_CONFPREFIX_ipcdata_t *)rdata;
	res=read(fd, rdata, sizeof(rdata));
	if(res<=0) {
		UB_LOG(UBL_ERROR, "%s:error: %s\n", __func__, strerror(errno));
		return -1;
	}
	if(ipd->magic==_CONFPREFIX_IPCDATA_TMAGIC){
		return read_proc_textmode(rdata+4, res-4);
	}else if(ipd->magic==_CONFPREFIX_IPCDATA_BMAGIC){
		return read_proc_binarymode(ipd, res);
	}
	UB_LOG(UBL_ERROR, "%s:received wrong magic\n", __func__);
	return -1;
}


static int client_proc(ipcoptions_t *ipcopt)
{
	int fd;
	int i, res;
	char *scmd;
	fd_set rfds;
	bool textmode=true;

	for(i=0;i<10;i++){
		res=cb_ipcsocket_init(&fd, ipcopt->laddr, NULL, ipcopt->addr);
		if(!res) break;
		usleep(10000);
	}
	if(res) return -1;

	while(true){
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		FD_SET(0, &rfds); // add stdion
		res=select(fd+1, &rfds, NULL, NULL, NULL);
		if(res == -1){
			UB_LOG(UBL_ERROR,"%s:select error %s\n", __func__, strerror(errno));
			break;
		}
		if(FD_ISSET(fd, &rfds)){
			res=read_from_server(fd);
			if(res) {
				UB_LOG(UBL_ERROR,"%s:errorn in read_from_server\n", __func__);
				break;
			}
		}
		if(FD_ISSET(0, &rfds)){
			res=console_cmd_proc(fd, &textmode);
			if(res==1) {
				res=0;
				break;
			}
			if(res) {
				UB_LOG(UBL_ERROR,"%s:errorn in console_cmd_proc\n", __func__);
				break;
			}
		}
	}

	scmd="TTTT D";
	res=write(fd, scmd, strlen(scmd));
	cb_ipcsocket_close(fd, ipcopt->laddr, NULL);
	return 0;
}

int main(int argc, char *argv[])
{
	unibase_init_para_t init_para;
	ipcoptions_t ipcopt={0, DEFAULT_IPCSERVER_NODE, DEFAULT_IPCCLIENT_NODE};
	int res;

	if(set_options(&ipcopt, argc, argv)) return -1;
	ubb_default_initpara(&init_para);
	init_para.ub_log_initstr=UBL_OVERRIDE_ISTR("4,ubase:45,combase:45,unip:45", "UBL_UNIP");
	unibase_init(&init_para);
	res=client_proc(&ipcopt);
	unibase_close();
	return res;
}
