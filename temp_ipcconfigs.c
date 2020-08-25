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
#include "xl4combase/combase.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "_CONFPREFIX__configs.h"
#include "_CONFPREFIX__ipcconfigs.h"

static int realloc_vstr(char **vstr, int *vstrsize)
{
	char *np;
	np=realloc(*vstr, *vstrsize+128);
	if(!np) return -1;
	np[*vstrsize]=0;
	*vstr=np;
	*vstrsize+=128;
	return 0;
}

static int append_char(char **vstr, int *vstrsize, int *vpi, char a)
{
	if(*vstrsize-*vpi<1){
		if(realloc_vstr(vstr, vstrsize)) return -1;
	}
	*(*vstr+*vpi)=a;
	(*vpi)++;
	return 0;
}

// append staring with null terminate, *vpi points the null terminator
static int append_string(char **vstr, int *vstrsize, int *vpi, char *astr)
{
	do{
		if(append_char(vstr, vstrsize, vpi, *astr)) return -1;
	}while(*astr++);
	(*vpi)--;
	return 0;
}

/*_CONF_GENERATED_*/

static int single_hex_value_to_string(int bsize, char **vstr, int *vstrsize, int *vpi,
				      void **values, int *valsize, bool addx)
{
	int res;
	char *sa="";
	if(addx) sa="0x";
	if(*vstrsize-*vpi<16){
		if(realloc_vstr(vstr, vstrsize)) return -1;
	}
	if(*valsize<bsize) return -1;
	if(bsize==1)
		res=snprintf(*vstr+*vpi, *vstrsize, "%s%02X", sa, *((uint8_t*)*values));
	else if(bsize==2)
		res=snprintf(*vstr+*vpi, *vstrsize, "%s%04X", sa, *((uint16_t*)*values));
	else return -1;
	if(res<0) return -1;
	*vpi+=res;
	*valsize-=bsize;
	*values+=bsize;
	return 0;
}

static int field_values_to_string(int vtype, int elen, int vnum,
				  char **vstr, int *vstrsize, int *vpi,
				  void **values, int *valsize)
{
	int i, j;
	char delimi=',';
	if(elen>1) append_char(vstr, vstrsize, vpi, '{');
	for(i=0;i<elen && *valsize>0;i++){
		if(vnum>1) append_char(vstr, vstrsize, vpi, '{');
		for(j=0;j<vnum;j++){
			if(vtype==VT_UINT8_T){
				if(single_hex_value_to_string(1, vstr, vstrsize, vpi,
							      values, valsize,
							      (vnum==1 && elen==1))) return -1;
				delimi=':';
			}else{
				if(single_value_to_string(vtype, vstr, vstrsize, vpi,
							  values, valsize)) return -1;
				delimi=',';
			}
			*vpi+=strlen(*vstr+*vpi);
			if(j<vnum-1){
				append_char(vstr, vstrsize, vpi, delimi);
			}
		}
		if(vnum>1) {
			append_char(vstr, vstrsize, vpi, '}');
			delimi=',';
		}
		if(i<elen-1) append_char(vstr, vstrsize, vpi, delimi);
	}
	if(elen>1) append_char(vstr, vstrsize, vpi, '}');
	return 0;
}

char *_CONFPREFIX_item_values_to_string(int item, int index, int findex,
					void *values, int valsize, bool dnonidxed)
{
	int svsize, vsize, vnum, elen, vtype, fvtype, foffset;
	int res=0;
	char *vstr=NULL;
	int vstrsize=128;
	int vpi;
	if(!values) return NULL;
	if(valsize<=0) return NULL;
	svsize=_CONFPREFIX_conf_get_item_value_size(item);
	if(svsize<0) return NULL;
	elen=_CONFPREFIX_conf_get_item_element_num(item);
	if(index>=0){
		if(dnonidxed){
			values+=index*svsize;
			valsize-=index*svsize;
			elen-=index;
		}else{
			elen=UB_MIN(valsize/svsize, elen-index);
		}
	}
	if(elen<=0){
		UB_LOG(UBL_ERROR, "%s:index=%d may be wrong\n", __func__, index);
		return NULL;
	}

	vnum=_CONFPREFIX_conf_get_item_value_num(item);
	vtype=_CONFPREFIX_conf_get_item_vtype(item);
	vstr=ub_malloc_or_die(__func__, vstrsize);
	vpi=0;
	while(valsize>=svsize && res==0){
		if(vtype<0 || vtype>=UPSTRING_TYPE_BASE){
			res=field_values_to_string(vtype, elen, vnum, &vstr, &vstrsize, &vpi,
						   &values, &valsize);
		}else if(findex>=0){
			fvtype=_CONFPREFIX_struct_field_vtype(vtype, findex, &vsize, &elen,
							      &vnum, &foffset);
			if(fvtype==VT_INVALID){
				res=-1;
			}else{
				values+=foffset;
				valsize-=foffset;
				res=field_values_to_string(fvtype, elen, vnum, &vstr, &vstrsize,
							   &vpi, &values, &valsize);
				break;
			}
		}else{
			void *svalues;
			int svalsize=valsize;
			findex=0;
			if(vpi) append_char(&vstr, &vstrsize, &vpi, ',');
			append_char(&vstr, &vstrsize, &vpi, '{');
			while(svalsize>0){
				fvtype=_CONFPREFIX_struct_field_vtype(vtype, findex, &vsize,
								      &elen, &vnum, &foffset);
				if(fvtype==VT_INVALID) break;
				svalues=values+foffset;
				if(findex>0) append_char(&vstr, &vstrsize, &vpi, ',');
				res=field_values_to_string(fvtype, elen, vnum, &vstr, &vstrsize,
							   &vpi, &svalues, &svalsize);
				if(res) break;
				findex++;
			}
			append_char(&vstr, &vstrsize, &vpi, '}');
			findex=-1;
			values+=svsize;
			valsize-=svsize;
		}
	}
	if(res){
		free(vstr);
		return NULL;
	}
	append_char(&vstr, &vstrsize, &vpi, 0);
	if((vtype<0 || vtype>=UPSTRING_TYPE_BASE) && vstr[0]=='{' && vstr[vpi-2]=='}'){
		// non-struct type variable case, the outmost '{','}' are not needed
		memmove(vstr, vstr+1, vpi-3);
		vstr[vpi-3]=0;
	}
	return vstr;
}

#define IPCSERVER_MUTEX_TOUT_USEC 10000
typedef struct _CONFPREFIX_ipcserver _CONFPREFIX_ipcserver_t;
struct _CONFPREFIX_ipcserver {
	cb_ipcserverd_t *ipcsd;
	CB_THREAD_T thread;
	CB_THREAD_MUTEX_T mutex;
	bool threadrun;
	_CONFPREFIX_ipcserver_update_cb update_cb;
	void *update_cbdata;
};


static void *ipc_receiver_proc(void *ptr) __attribute__((unused));
static void *ipc_receiver_proc(void *ptr)
{
	_CONFPREFIX_ipcserver_t *isvd=(_CONFPREFIX_ipcserver_t *)ptr;
	fd_set rfds;
	struct timeval tvtout;
	int ipcfd=cb_ipcsocket_getfd(isvd->ipcsd);
	int res;
	while(isvd->threadrun){
		FD_ZERO(&rfds);
		FD_SET(ipcfd, &rfds);
		tvtout.tv_sec=0;
		tvtout.tv_usec=100000; // 100msec timeout
		res=select(ipcfd+1, &rfds, NULL, NULL, &tvtout);
		if(res == -1){
			UB_LOG(UBL_ERROR,"%s:select error %s\n", __func__, strerror(errno));
			break;
		}
		if(res==0) continue;
		if(FD_ISSET(ipcfd, &rfds)){
			_CONFPREFIX_ipcserver_receive();
			continue;
		}
		UB_LOG(UBL_ERROR,"%s:unknown event\n", __func__);
	}
	return NULL;
}

static uint8_t *ipc_get_variable_data_b(int item, int index, int findex, int *size,
					struct sockaddr *addr, char read_notice)
{
	int esize, tsize;
	void *values;
	_CONFPREFIX_ipcdata_t *ipd=NULL;
	int foffset=0;
	values=_CONFPREFIX_conf_get_item(item);
	if(!values) return NULL;

	esize=_CONFPREFIX_conf_get_item_element_size(item);
	tsize=esize;
	if(index==-1) tsize*=_CONFPREFIX_conf_get_item_element_num(item);

	ipd=ub_malloc_or_die(__func__, sizeof(_CONFPREFIX_ipcdata_t)+tsize-4);
	if(read_notice=='R')
		ipd->cmd=_CONFPREFIX_IPCCMD_READ;
	else
		ipd->cmd=_CONFPREFIX_IPCCMD_NOTICE;

	if(index>0){
		values+=index*esize;
	}
	if(findex>=0){
		int vtype, fvtype, vsize, elen, vnum;
		vtype=_CONFPREFIX_conf_get_item_vtype(item);
		fvtype=_CONFPREFIX_struct_field_vtype(vtype, findex, &vsize, &elen,
						&vnum, &foffset);
		if(fvtype==VT_INVALID) goto erexit;
		ipd->size=vsize*elen*vnum;
	}else{
		ipd->size=tsize;
	}
	UB_LOG(UBL_DEBUGV, "%s:index=%d, findex=%d, size=%d\n",
	       __func__, index, findex, ipd->size);
	memcpy(ipd->data, values+foffset, ipd->size);
	ipd->item=item;
	ipd->index=index;
	ipd->findex=findex;
	ipd->magic=_CONFPREFIX_IPCDATA_BMAGIC;
	*size=sizeof(_CONFPREFIX_ipcdata_t)+ipd->size-4;
	return (uint8_t *)ipd;
erexit:
	if(ipd) free(ipd);
	return NULL;
}

static uint8_t *ipc_get_variable_data(_CONFPREFIX_ipcserver_t *isvd,
				      int item, int index, int findex, int *size,
				      struct sockaddr *addr, char read_notice)
{
	cb_ipcclient_commode_t commode;
	void *values;
	char *sval, *ssval;
	char istr[6]={0,};
	char fstr[6]={0,};
	char *vname;
	int tl, tsize;

	commode=cb_ipcsocket_get_commode(isvd->ipcsd, addr);
	if(commode==CB_IPCCLIENT_BINARY)
		return ipc_get_variable_data_b(item, index, findex, size, addr, read_notice);
	vname=_CONFPREFIX_config_item_strings(item);
	if(!vname) return NULL;
	values=_CONFPREFIX_conf_get_item(item);
	if(!values) return NULL;
	tsize=_CONFPREFIX_conf_get_item_element_size(item);
	if(index<0){
		tsize*=_CONFPREFIX_conf_get_item_element_num(item);
	}else{
		if(index>_CONFPREFIX_conf_get_item_element_num(item)) return NULL;
		values+=tsize*index;
	}
	UB_LOG(UBL_DEBUGV, "%s:vname=%s index=%d findex=%d tsize=%d\n",
	       __func__, vname, index, findex, tsize);
	sval=_CONFPREFIX_item_values_to_string(item, index, findex, values, tsize, false);
	if(!sval) return NULL;
	tl=strlen(sval)+strlen(vname)+20;
	ssval=ub_malloc_or_die(__func__, tl);
	if(index>=0 && index<1000) snprintf(istr, 6, "[%d]", index);
	if(findex>=0 && findex<1000) snprintf(fstr, 6, ".f%d", findex);
	snprintf(ssval, tl, "TTTT %c %s%s%s %s", read_notice,
		 vname, istr, fstr, sval);
	free(sval);
	*size=strlen(ssval);
	return (uint8_t *)ssval;
}

static int ipc_send_variable_values(_CONFPREFIX_ipcserver_t *isvd,
				    int item, int index, int findex,
				    struct sockaddr *addr, char read_notice)
{
	uint8_t *sdata;
	int size;
	int res;
	sdata=ipc_get_variable_data(isvd, item, index, findex, &size, addr, read_notice);
	if(!sdata) return -1;
	res=cb_ipcsocket_server_write(isvd->ipcsd, sdata, size, addr);
	free(sdata);
	return res;
}

static int ipc_sendback_to_query_evlist(_CONFPREFIX_ipcserver_t *isvd, bool textmode,
					struct sockaddr *addr)
{

	int i, en, esize, res;
	uint8_t sdata[1500]={0,};
	_CONFPREFIX_ipcdata_t *ipd=(_CONFPREFIX_ipcdata_t *)sdata;
	int sp=0;
	_CONFPREFIX_item_extend_t *reid;
	void *edata;

	if(textmode){
		ipd->magic=_CONFPREFIX_IPCDATA_TMAGIC;
		sp+=4;
		sdata[sp++]=' ';
		sdata[sp++]='Q';
		sdata[sp++]=' ';
	}else{
		ipd->magic=_CONFPREFIX_IPCDATA_BMAGIC;
		ipd->cmd=_CONFPREFIX_IPCCMD_QEVLIST;
		sp=sizeof(_CONFPREFIX_ipcdata_t)-4;
		ipd->size=0;
	}

	esize=sizeof(_CONFPREFIX_item_extend_t);
	en=_CONFPREFIX_get_extend_itemlist(&edata);
	if(en<0) return -1;
	for(i=0;i<en && sp<sizeof(sdata);i++) {
		reid=(_CONFPREFIX_item_extend_t *)(edata+esize*i);
		if(textmode){
			if(i>0) sdata[sp++]=',';
			res=snprintf((char*)sdata+sp, sizeof(sdata)-sp, "{%s 0 {%d %d %d %d} %d}",
				     reid->name, reid->vsizes[0],reid->vsizes[1],
				     reid->vsizes[2],reid->vsizes[3],reid->ipcon);
			if(res<=0) break;
			sp+=res;
		}else{
			if(sizeof(sdata)-sp<esize) break;
			memcpy(sdata+sp, reid, esize);
			sp+=esize;
			ipd->size+=esize;
		}
	}
	return cb_ipcsocket_server_write(isvd->ipcsd, sdata, sp, addr);
}

static int ipc_disconnect(_CONFPREFIX_ipcserver_t *isvd, struct sockaddr *addr)
{
	UB_LOG(UBL_INFO, "%s:one client is closing\n", __func__);
	return cb_ipcsocket_remove_client(isvd->ipcsd, addr);
}

static int ipc_receive_textmode(_CONFPREFIX_ipcserver_t *isvd, struct sockaddr *addr,
				uint8_t *rdata, int size)
{
	_CONFPREFIX_ipccmd_t cmd;
	int index, findex, item;
	int res;
	uint8_t ipcon;

	if(*((uint32_t*)rdata)!=_CONFPREFIX_IPCDATA_TMAGIC) return -1;
	rdata+=4;
	size-=4;
	rdata[size]=0;
	cb_ipcsocket_set_commode(isvd->ipcsd, addr, CB_IPCCLIENT_TEXT);
	_CONFPREFIX_skip_chars((char**)&rdata, &size, ' ', '\t');
	if(rdata[0]=='R') cmd=_CONFPREFIX_IPCCMD_READ;
	else if(rdata[0]=='W') cmd=_CONFPREFIX_IPCCMD_WRITE;
	else if(rdata[0]=='Q') return ipc_sendback_to_query_evlist(isvd, true, addr);
	else if(rdata[0]=='D') return ipc_disconnect(isvd, addr);
	else return -1;
	rdata+=1;
	size-=1;
	if(_CONFPREFIX_variable_from_str(&item, &index, &findex,
					 (char**)&rdata, &size)) return -1;
	ipcon=_CONFPREFIX_conf_get_ipcon(item);
	if(cmd==_CONFPREFIX_IPCCMD_WRITE){
		if(!(ipcon & UPIPC_W)){
			UB_LOG(UBL_DEBUG, "item=%d is not writable through IPC\n", item);
			return -1;
		}
		_CONFPREFIX_skip_chars((char**)&rdata, &size, ' ', '\t');
		UB_LOG(UBL_DEBUGV, "%s:update item=%d, index=%d, findex=%d, %s\n",
		       __func__, item, index, findex, (char*)rdata);
		_CONFPREFIX_ipcserver_lock(IPCSERVER_MUTEX_TOUT_USEC);
		res=_CONFPREFIX_stritem_update(item, index, findex, (char**)&rdata, &size);
		_CONFPREFIX_ipcserver_unlock();
		if(!res && isvd->update_cb)
			res=isvd->update_cb(isvd->update_cbdata, item, index, findex);
		return res;
	}
	if(!(ipcon & UPIPC_R)) {
		UB_LOG(UBL_DEBUG, "item=%d is not readable through IPC\n", item);
		return -1;
	}
	return ipc_send_variable_values(isvd, item, index, findex, addr, 'R');
}

static int ipc_receive_binarymode(_CONFPREFIX_ipcserver_t *isvd, struct sockaddr *addr,
				  uint8_t *rdata, int size)
{
	_CONFPREFIX_ipcdata_t *ipd=(_CONFPREFIX_ipcdata_t *)rdata;
	int res=0;
	uint8_t ipcon;
	if(ipd->magic!=_CONFPREFIX_IPCDATA_BMAGIC) return -1;
	cb_ipcsocket_set_commode(isvd->ipcsd, addr, CB_IPCCLIENT_BINARY);
	ipcon=_CONFPREFIX_conf_get_ipcon(ipd->item);
	if(ipd->cmd==_CONFPREFIX_IPCCMD_READ){
		if(!(ipcon & UPIPC_R)) {
			UB_LOG(UBL_DEBUG, "item=%d is not readable through IPC\n", ipd->item);
			return -1;
		}
		return ipc_send_variable_values(
			isvd, ipd->item, ipd->index, ipd->findex, addr, 'R');
	}else if (ipd->cmd==_CONFPREFIX_IPCCMD_WRITE){
		if(!(ipcon & UPIPC_W)) {
			UB_LOG(UBL_DEBUG, "item=%d is not writable through IPC\n", ipd->item);
			return -1;
		}
		UB_LOG(UBL_DEBUGV, "%s:update item=%d, index=%d, findex=%d\n",
		       __func__, ipd->item, ipd->index, ipd->findex);
		_CONFPREFIX_ipcserver_lock(IPCSERVER_MUTEX_TOUT_USEC);
		res=_CONFPREFIX_item_index_update(ipd->item, ipd->index, ipd->findex,
						  ipd->data, ipd->size);
		_CONFPREFIX_ipcserver_unlock();
		if(!res && isvd->update_cb)
			res=isvd->update_cb(isvd->update_cbdata, ipd->item,
					    ipd->index, ipd->findex);
	}else if (ipd->cmd==_CONFPREFIX_IPCCMD_QEVLIST){
		return ipc_sendback_to_query_evlist(isvd, false, addr);
	}else if (ipd->cmd==_CONFPREFIX_IPCCMD_DISCONNECT){
		return ipc_disconnect(isvd, addr);
	}else{
		return -1;
	}
	return res;
}

static int ipc_receive_cb(void *cbdata, uint8_t *rdata, int size, struct sockaddr *addr)
{
	_CONFPREFIX_ipcserver_t *isvd=(_CONFPREFIX_ipcserver_t *)cbdata;
	if(size<6) return -1;
	UB_LOG(UBL_DEBUG, "%s:size=%d\n", __func__, size);
	if(!ipc_receive_textmode(isvd, addr, rdata, size)) return 0;
	if(!ipc_receive_binarymode(isvd, addr, rdata, size)) return 0;
	return -1;
}

static _CONFPREFIX_ipcserver_t *isvd;
int _CONFPREFIX_ipcserver_init(char *node_ip, uint16_t port, bool thread)
{
	isvd=(_CONFPREFIX_ipcserver_t *)ub_malloc_or_die(
		__func__, sizeof(_CONFPREFIX_ipcserver_t));
	memset(isvd, 0, sizeof(_CONFPREFIX_ipcserver_t));
	isvd->ipcsd=cb_ipcsocket_server_init(node_ip, NULL, port);
	if(!isvd->ipcsd) {
		UB_LOG(UBL_ERROR, "%s:can't open ipc socket\n", __func__);
		goto erexit;
	}
	if(!thread) return cb_ipcsocket_getfd(isvd->ipcsd);
	isvd->threadrun=true;
	if(CB_THREAD_MUTEX_INIT(&isvd->mutex, NULL)) goto erexit;
	if(CB_THREAD_CREATE(&isvd->thread, NULL, ipc_receiver_proc, isvd)) goto erexit;
	return cb_ipcsocket_getfd(isvd->ipcsd);
erexit:
	_CONFPREFIX_ipcserver_close();
	return -1;
}

static int check_isvd(const char *fname)
{
	if(!isvd){
		UB_LOG(UBL_ERROR, "%s:ipcserver is not opened\n", fname);
		return -1;
	}
	return 0;
}

int _CONFPREFIX_ipcserver_set_update_cb(_CONFPREFIX_ipcserver_update_cb update_cb,
					void *update_cbdata)
{
	if(check_isvd(__func__)) return -1;
	isvd->update_cb=update_cb;
	isvd->update_cbdata=update_cbdata;
	return 0;
}

void _CONFPREFIX_ipcserver_close(void)
{
	if(check_isvd(__func__)) return;
	if(isvd->thread){
		isvd->threadrun=false;
		CB_THREAD_JOIN(isvd->thread, NULL);
		CB_THREAD_MUTEX_DESTROY(&isvd->mutex);
	}
	cb_ipcsocket_server_close(isvd->ipcsd);
	free(isvd);
	isvd=NULL;
}

int _CONFPREFIX_ipcserver_receive(void)
{
	if(check_isvd(__func__)) return -1;
	return cb_ipcsocket_server_read(isvd->ipcsd, ipc_receive_cb, isvd);
}

static int ipcserver_send_notice_ddata(void *cbdata, uint8_t **sdata, int *size,
				       struct sockaddr *addr)
{
	_CONFPREFIX_ipcdata_t *ipd=(_CONFPREFIX_ipcdata_t *)cbdata;
	*sdata=ipc_get_variable_data(isvd, ipd->item, ipd->index, ipd->findex, size, addr, 'N');
	if(!sdata) return -1;
	return 0;
}

int _CONFPREFIX_ipcserver_send_notice(_CONFPREFIX_ipcdata_t *ipd)
{
	if(check_isvd(__func__)) return -1;
	if(!ipd) return -1;
	if(!_CONFPREFIX_config_item_strings(ipd->item)){
		UB_LOG(UBL_INFO, "%s:not registered item, send data as in ipd\n", __func__);
		return cb_ipcsocket_server_write(
			isvd->ipcsd, (uint8_t*)ipd,
			sizeof(_CONFPREFIX_ipcdata_t)+ipd->size-4, NULL);
	}
	return cb_ipcsocket_server_write_ddata(isvd->ipcsd, ipd, ipcserver_send_notice_ddata);
}

int _CONFPREFIX_ipcserver_lock(int tout_usec)
{
	struct timespec mtots __attribute__((unused));
	uint64_t mtout;
	if(check_isvd(__func__)) return -1;
	if(!isvd->threadrun) return 0;
	if(tout_usec<0) return CB_THREAD_MUTEX_LOCK(&isvd->mutex);
	mtout=ub_rt_gettime64()+tout_usec*UB_USEC_NS;
	UB_NSEC2TS(mtout, mtots);
	if(CB_THREAD_MUTEX_TIMEDLOCK(&isvd->mutex, &mtots)){
		UB_LOG(UBL_ERROR, "%s:can't lock in timeout time\n", __func__);
		return -1;
	}
	return 0;
}

void _CONFPREFIX_ipcserver_unlock(void)
{
	if(check_isvd(__func__)) return;
	if(!isvd->threadrun) return;
	CB_THREAD_MUTEX_UNLOCK(&isvd->mutex);
}

int _CONFPREFIX_get_ipcfd(void)
{
	if(check_isvd(__func__)) return -1;
	return cb_ipcsocket_getfd(isvd->ipcsd);
}

int _CONFPREFIX_write_config_file(char *fname)
{
	int i;
	FILE *outf;
	int res=-1;
	char *vname;
	void *values=NULL;
	char *sval=NULL;
	int tsize;
	outf=fopen(fname, "w");
	if(!outf) return -1;
	for(i=0;i<_CONFPREFIX_CONF_ENUM_LAST_ITEM;i++){
		vname=_CONFPREFIX_config_item_strings(i);
		if(!vname) goto erexit;
		values=_CONFPREFIX_conf_get_item(i);
		if(!values) goto erexit;
		tsize=_CONFPREFIX_conf_get_item_element_size(i) *
			_CONFPREFIX_conf_get_item_element_num(i);
		sval=_CONFPREFIX_item_values_to_string(i, -1, -1, values, tsize, false);
		if(!sval) goto erexit;
		fwrite(vname, 1, strlen(vname), outf);
		fwrite(" ", 1, 1, outf);
		fwrite(sval, 1, strlen(sval), outf);
		fwrite("\n", 1, 1, outf);
		free(sval);
		sval=NULL;
	}
	res=0;
erexit:
	fclose(outf);
	if(sval) free(sval);
	return res;
}
