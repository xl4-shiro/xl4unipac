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
#include "xl4unibase/unibase.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "_CONFPREFIX__configs.h"

static ub_esarray_cstd_t *item_extends;

#define MAX_CHARS_PER_LINE 1024
static int _CONFPREFIX_conf_readline(FILE *inf, char *line)
{
	char a;
	int res;
	int rp=0;

	if(feof(inf)) return 0;
	while((res=fread(&a, 1, 1, inf))){
		if(res<0){
			if(feof(inf)) return 0;
			UB_LOG(UBL_WARN,"%s:error to read config file\n", __func__);
			return -1;
		}
		if(a=='\n') break;
		line[rp++]=a;
		if(rp>=MAX_CHARS_PER_LINE-1){
			UB_LOG(UBL_WARN,"%s:too long line\n", __func__);
			return -1;
		}
	}
	line[rp++]=0;
	return rp;
}

#define MAX_ITEM_EXTENDS 256
static void init_item_extends(void)
{
	item_extends=ub_esarray_init(4, sizeof(_CONFPREFIX_item_extend_t), MAX_ITEM_EXTENDS);
}

int _CONFPREFIX_register_extend_item(_CONFPREFIX_item_extend_t *eid)
{
	int i,en;
	_CONFPREFIX_item_extend_t *reid;
	if(!item_extends) init_item_extends();
	en=ub_esarray_ele_nums(item_extends);
	for(i=0;i<en;i++) {
		reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, i);
		if(!strcmp(reid->name, eid->name)) {
			UB_LOG(UBL_ERROR, "%s:name %s is already registered\n",
			       __func__, eid->name);
			return -1;
		}
	}
	return ub_esarray_add_ele(item_extends, (ub_esarray_element_t *)eid);
}

int _CONFPREFIX_remove_extend_item(char *vname)
{
	int i,en;
	_CONFPREFIX_item_extend_t *reid;
	if(!item_extends) init_item_extends();
	en=ub_esarray_ele_nums(item_extends);
	for(i=0;i<en;i++) {
		reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, i);
		if(!strcmp(reid->name, vname))
			return ub_esarray_del_index(item_extends, i);
	}
	UB_LOG(UBL_ERROR, "%s:name %s is not registered\n", __func__, vname);
	return -1;
}

void _CONFPREFIX_removeall_extend_item(void)
{
	if(!item_extends) return;
	ub_esarray_close(item_extends);
	item_extends=NULL;
}

int _CONFPREFIX_get_extend_itemlist(void **data)
{
	int i,en,esize;
	void *p;
	if(!item_extends) return 0;
	en=ub_esarray_ele_nums(item_extends);
	if(!en) return 0;
	esize=sizeof(_CONFPREFIX_item_extend_t);
	*data=ub_malloc_or_die(__func__, esize*en);
	for(i=0;i<en;i++){
		p=ub_esarray_get_ele(item_extends, i);
		memcpy(*data+esize*i, p, esize);
	}
	return en;
}

static char *get_extend_item_vname(int eitemn)
{
	_CONFPREFIX_item_extend_t *reid;
	if(!item_extends) return NULL;
	reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, eitemn);
	if(!reid) return NULL;
	return reid->name;
}

static void *get_extend_item_vp(int eitemn)
{
	_CONFPREFIX_item_extend_t *reid;
	if(!item_extends) return NULL;
	reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, eitemn);
	if(!reid) return NULL;
	return reid->valuep;
}

static int *get_extend_item_vsizes(int eitemn)
{
	_CONFPREFIX_item_extend_t *reid;
	if(!item_extends) return NULL;
	reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, eitemn);
	if(!reid) return NULL;
	return reid->vsizes;
}

static uint8_t get_extend_item_ipcon(int eitemn)
{
	_CONFPREFIX_item_extend_t *reid;
	if(!item_extends) return UPIPC_N;
	reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, eitemn);
	if(!reid) return UPIPC_N;
	return reid->ipcon;
}

static bool check_first_char(char **svalues, int *svalsize, char a)
{
	if(*svalsize>0 && (*svalues)[0]==a){
		(*svalues)++;
		(*svalsize)--;
		return true;
	};
	return false;
}

bool _CONFPREFIX_skip_chars(char **svalues, int *svalsize, char a, char b)
{
	while(*svalsize>0 && **svalues){
		if(**svalues==a || **svalues==b) {
			(*svalues)++;
			(*svalsize)--;
			continue;
		}
		return true;
	}
	return false;
}

bool _CONFPREFIX_skip_to_chars(char **svalues, int *svalsize, char a, char b)
{
	bool inquote=false;
	while(*svalsize>0 && **svalues){
		if(**svalues=='"') {
			inquote=inquote?false:true;
		}else if(!inquote && (**svalues==a || **svalues==b)) {
			return true;
		}
		(*svalues)++;
		(*svalsize)--;
	}
	return false;
}

static int get_bracket_num(char **svalues, int *svalsize, bool nopup)
{
	char *p, *q;
	int i,s;
	p=*svalues;
	s=*svalsize;
	if(!check_first_char(&p, &s, '[')) return -1;
	i=strtol(p, &q, 10);
	p=q;
	s-=q-p;
	if(!check_first_char(&p, &s, ']')) return -1;
	if(nopup) return i;
	*svalues=p;
	*svalsize=s;
	return i;
}

static int check_item(int n)
{
	if(n<0 || n>=_CONFPREFIX_CONF_ENUM_LAST_ITEM) return -1;
	return 0;
}

/*_CONF_GENERATED_*/

void *_CONFPREFIX_conf_get_item(_CONFPREFIX__config_item_t item)
{
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE)
		return get_extend_item_vp(item-_CONFPREFIX_ITEM_EXTEND_BASE);
	if(check_item(item)) return NULL;
	return config_value_pointers[item];
}

uint8_t _CONFPREFIX_conf_get_ipcon(_CONFPREFIX__config_item_t item)
{
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE)
		return get_extend_item_ipcon(item-_CONFPREFIX_ITEM_EXTEND_BASE);
	if(check_item(item)) return UPIPC_N;
	return config_item_ipcon[item];
}

int _CONFPREFIX_conf_get_item_value_size(_CONFPREFIX__config_item_t item)
{
	int *vsizes;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		vsizes=get_extend_item_vsizes(item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!vsizes) return -1;
		return vsizes[0];
	}
	if(check_item(item)) return -1;
	return config_value_sizes[item][0];
}

int _CONFPREFIX_conf_get_item_value_num(_CONFPREFIX__config_item_t item)
{
	int *vsizes;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		vsizes=get_extend_item_vsizes(item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!vsizes) return -1;
		return vsizes[1];
	}
	if(check_item(item)) return -1;
	return config_value_sizes[item][1];
}

int _CONFPREFIX_conf_get_item_vtype(_CONFPREFIX__config_item_t item)
{
	int *vsizes;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		vsizes=get_extend_item_vsizes(item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!vsizes) return -1;
		return vsizes[3];
	}
	if(check_item(item)) return -1;
	return config_value_sizes[item][3];
}

int _CONFPREFIX_conf_get_item_element_size(_CONFPREFIX__config_item_t item)
{
	int *vsizes;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		vsizes=get_extend_item_vsizes(item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!vsizes) return -1;
		return vsizes[0]*vsizes[1];
	}
	if(check_item(item)) return -1;
	return config_value_sizes[item][0]*config_value_sizes[item][1];
}

int _CONFPREFIX_conf_get_item_element_num(_CONFPREFIX__config_item_t item)
{
	int *vsizes;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		vsizes=get_extend_item_vsizes(item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!vsizes) return -1;
		return vsizes[2];
	}
	if(check_item(item)) return -1;
	return config_value_sizes[item][2];
}

void *_CONFPREFIX_conf_get_item_index(_CONFPREFIX__config_item_t item, int index)
{
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		_CONFPREFIX_item_extend_t *reid;
		if(!item_extends) return NULL;
		reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(
			item_extends, item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!reid) return NULL;
		if(index<0 || index>=reid->vsizes[2]) return NULL;
		return reid->valuep+reid->vsizes[0]*reid->vsizes[1]*index;
	}
	if(check_item(item)) return NULL;
	if(index<0 || index>=config_value_sizes[item][2]) return NULL;
	return config_value_pointers[item]+config_value_sizes[item][0]*
		config_value_sizes[item][1]*index;
}

int32_t _CONFPREFIX_conf_get_intitem(_CONFPREFIX__config_item_t item)
{
	if(check_item(item) && item<_CONFPREFIX_ITEM_EXTEND_BASE) return (int32_t)0x80000000;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE){
		if(!item_extends) return (int32_t)0x80000000;
		if(item>=_CONFPREFIX_ITEM_EXTEND_BASE+ub_esarray_ele_nums(item_extends))
			return (int32_t)0x80000000;
	}
	return *((int32_t *)_CONFPREFIX_conf_get_item(item));
}

int32_t _CONFPREFIX_conf_get_intitem_index(_CONFPREFIX__config_item_t item, int index)
{
	int32_t *vp;
	vp=_CONFPREFIX_conf_get_item_index(item, index);
	if(!vp) return (int32_t)0x80000000;
	return *((int32_t*)vp);
}

bool _CONFPREFIX_conf_get_boolitem(_CONFPREFIX__config_item_t item)
{
	if(check_item(item) && item<_CONFPREFIX_ITEM_EXTEND_BASE) return false;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE){
		if(!item_extends) return false;
		if(item>=_CONFPREFIX_ITEM_EXTEND_BASE+ub_esarray_ele_nums(item_extends))
			return false;
	}
	return *((bool *)_CONFPREFIX_conf_get_item(item));
}

int64_t _CONFPREFIX_conf_get_lintitem(_CONFPREFIX__config_item_t item)
{
	if(check_item(item) && item<_CONFPREFIX_ITEM_EXTEND_BASE)
		return (int64_t)0x8000000000000000;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE){
		if(!item_extends) return (int64_t)0x8000000000000000;
		if(item>=_CONFPREFIX_ITEM_EXTEND_BASE+ub_esarray_ele_nums(item_extends))
			return (int64_t)0x8000000000000000;
	}
	return *((int64_t *)_CONFPREFIX_conf_get_item(item));
}

int _CONFPREFIX_conf_set_item(_CONFPREFIX__config_item_t item, void *v)
{
	int s;
	void *p;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		_CONFPREFIX_item_extend_t *reid;
		if(!item_extends) return -1;
		reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(
			item_extends, item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!reid) return -1;
		s=reid->vsizes[0]*reid->vsizes[1]*reid->vsizes[2];
		p=reid->valuep;
		memcpy(p, v, s);
		return 0;
	}
	if(check_item(item)) return -1;
	s=config_value_sizes[item][0]*config_value_sizes[item][1]*config_value_sizes[item][2];
	p=config_value_pointers[item];
	memcpy(p, v, s);
	return 0;
}

int _CONFPREFIX_conf_set_item_index(_CONFPREFIX__config_item_t item, void *v, int index)
{
	int s;
	void *p;
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE) {
		_CONFPREFIX_item_extend_t *reid;
		if(!item_extends) return -1;
		reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(
			item_extends, item-_CONFPREFIX_ITEM_EXTEND_BASE);
		if(!reid) return -1;
		if(index<0 || index>=reid->vsizes[2]) return -1;
		s=reid->vsizes[0]*reid->vsizes[1];
		p=reid->valuep+s*index;
		memcpy(p, v, s);
		return 0;
	}
	if(check_item(item)) return -1;
	if(index<0 || index>=config_value_sizes[item][2]) return -1;
	s=config_value_sizes[item][0]*config_value_sizes[item][1];
	p=config_value_pointers[item]+s*index;
	memcpy(p, v, s);
	return 0;
}

_CONFPREFIX__config_item_t _CONFPREFIX_conf_get_item_num(char *istr)
{
	int i,en;
	if(!istr) return _CONFPREFIX_CONF_ENUM_NON_ITEM;
	for(i=0;i<_CONFPREFIX_CONF_ENUM_LAST_ITEM;i++)
		if(!strcmp(istr, config_item_strings[i])) return (_CONFPREFIX__config_item_t)i;
	if(!item_extends) return _CONFPREFIX_CONF_ENUM_NON_ITEM;
	en=ub_esarray_ele_nums(item_extends);
	for(i=0;i<en;i++) {
		_CONFPREFIX_item_extend_t *reid;
		reid=(_CONFPREFIX_item_extend_t *)ub_esarray_get_ele(item_extends, i);
		if(!strcmp(reid->name, istr)) return ((_CONFPREFIX__config_item_t)(i+_CONFPREFIX_ITEM_EXTEND_BASE));
	}
	return _CONFPREFIX_CONF_ENUM_NON_ITEM;
}

int _CONFPREFIX_conf_get_struct_num(char *istr)
{
	int i;
	if(!istr) return -1;
	for(i=0;i<_CONFPREFIX_CONF_ENUM_LAST_STRUCT;i++)
		if(!strcmp(istr, config_struct_strings[i]))
			return i;
	return -1;
}

int _CONFPREFIX_conf_set_stritem(char *istr, void *v)
{
	int item;
	item=_CONFPREFIX_conf_get_item_num(istr);
	if(item<0) return -1;
	return _CONFPREFIX_conf_set_item(((_CONFPREFIX__config_item_t)item), v);
}

int _CONFPREFIX_item_index_update(int item, int index, int findex, void *values, int sizev)
{
	void *p;
	int vtype;
	int elnum=1;
	if(index>_CONFPREFIX_conf_get_item_element_num(((_CONFPREFIX__config_item_t)item))) return -1;
	if(index<0) index=0;
	p=_CONFPREFIX_conf_get_item_index(((_CONFPREFIX__config_item_t)item), index);
	if(!p) return -1;
	vtype=_CONFPREFIX_conf_get_item_vtype(((_CONFPREFIX__config_item_t)item));
	if(findex==-1 || vtype<0 || vtype>=UPSTRING_TYPE_BASE){
		elnum=_CONFPREFIX_conf_get_item_element_num(((_CONFPREFIX__config_item_t)item));
		elnum-=index;
		memcpy(p, values, UB_MIN(_CONFPREFIX_conf_get_item_element_size(((_CONFPREFIX__config_item_t)item))*elnum,sizev));
		return 0;
	}
	return _CONFPREFIX_struct_field_update(vtype, findex, p, values, sizev);
}

int _CONFPREFIX_item_update(int item, void *values, int sizev)
{
	return _CONFPREFIX_item_index_update(item, 0, -1, values, sizev);
}

int _CONFPREFIX_strvname_update(char *vname, int index, int findex, char **svalues, int *svalsize)
{
	int item;
	item=_CONFPREFIX_conf_get_item_num(vname);
	if(item<0) return -1;
	return _CONFPREFIX_stritem_update(item, index, findex, svalues, svalsize);
}

static int get_strvtype_values(int vtype, int vsize, int elen, int vnum, int rsize,
			       void **values, char **svalues, int *svalsize)
{
	int esize, rn, i;
	void *fvalues;
	if(vsize<0) return -1;
	esize=vsize*elen*vnum;

	rn=get_bracket_num(svalues, svalsize, false);
	if(rn==-1) rn=1;
	rn=UB_MIN(rn, elen);
	fvalues=get_value(vsize, elen, vnum, vtype, svalues, svalsize, &esize);
	if(!fvalues) return -1;
	if(esize){
		*values=realloc(*values, rsize+esize*rn);
		if(!*values) return -1;
		for(i=0;i<rn;i++){
			memcpy(*values+rsize, fvalues, esize);
			rsize+=esize;
		}
	}
	free(fvalues);
	return rsize;
}

static int strstructitem_values(int item, int vtype, void **values,
				int elen, char **svalues, int *svalsize)
{
	int vsize, vnum, fvtype, i, rn, rnn, foffset, update_size;
	int findex=0;
	int ssize=0;
	bool verr=false;
	bool fb;
	// struct variable without an indexed field, get the whole struct
	rn=get_bracket_num(svalues, svalsize, false);
	rn=UB_MIN(rn, elen);
	fb=check_first_char(svalues, svalsize, '{');
	while(*svalsize>0){
		rnn=get_bracket_num(svalues, svalsize, true);
		if(rnn<=0) rnn=1;
		fvtype=_CONFPREFIX_struct_field_vtype(vtype, findex, &vsize, &elen,
						      &vnum, &foffset);
		if(fvtype==VT_INVALID) break;
		ssize=foffset;
		if(!verr){
			i=get_strvtype_values(fvtype, vsize, elen, vnum, ssize,
					      values, svalues, svalsize);
			if(i<0) break;
			verr=(i==ssize);
			ssize=i;
		}
		if(verr){
			if(!findex) return -1;
		}
		findex+=rnn;
		check_first_char(svalues, svalsize, ',');
	}
	update_size=ssize;
	ssize=_CONFPREFIX_conf_get_item_element_size(((_CONFPREFIX__config_item_t)item));
	if(rn>1){
		*values=realloc(*values, ssize*rn);
		memset(*values+update_size, 0x0, ssize-update_size);
		for(i=1;i<rn;i++){
			memcpy(*values+i*ssize, *values, update_size);
			memset(*values+i*ssize+update_size, 0x0, ssize-update_size);
		}
		ssize=ssize*rn;
	}else{
		*values=realloc(*values, ssize);
		memset(*values+update_size, 0x0, ssize-update_size);
	}

	if(fb && !check_first_char(svalues, svalsize, '}')) return -1;
	return ssize;
}

int _CONFPREFIX_stritem_values(int item, int index, int findex, void **values,
			       int *foffset, char **svalues, int *svalsize)
{
	int vtype, vsize=0, elen=0, vnum=0, fvtype, rsize;
	char *sv=*svalues;
	if(index<0) index=0;
	*values=NULL;
	vtype=_CONFPREFIX_conf_get_item_vtype(((_CONFPREFIX__config_item_t)item));
	if(vtype<0 || vtype>=UPSTRING_TYPE_BASE){
		// not struct variable
		vsize=_CONFPREFIX_conf_get_item_value_size(((_CONFPREFIX__config_item_t)item));
		elen=_CONFPREFIX_conf_get_item_element_num(((_CONFPREFIX__config_item_t)item))-index;
		vnum=_CONFPREFIX_conf_get_item_value_num(((_CONFPREFIX__config_item_t)item));
		rsize=get_strvtype_values(vtype, vsize, elen, vnum, 0,
					  values, svalues, svalsize);
	}else if(findex>=0){
		// struct variable with an indexed field
		fvtype=_CONFPREFIX_struct_field_vtype(vtype, findex, &vsize, &elen,
						      &vnum, foffset);
		rsize=get_strvtype_values(fvtype, vsize, elen, vnum, 0,
					  values, svalues, svalsize);
	}else{
		elen=_CONFPREFIX_conf_get_item_element_num(((_CONFPREFIX__config_item_t)item))-index;
		rsize=0;
		*foffset=0;
		while(elen > 0 && *svalsize>0){
			void *vval;
			int vvsize;
			vval=NULL;
			vvsize=strstructitem_values(item, vtype, &vval, elen--,
						    svalues, svalsize);
			if(vvsize>0){
				*values=realloc(*values, rsize+vvsize);
				memcpy(*values+rsize, vval, vvsize);
				rsize+=vvsize;
			}
			free(vval);
			if(vvsize<0) goto erexit;;
			if(!check_first_char(svalues, svalsize, ',')) break;
		}
	}
	if(*values) return rsize;
erexit:
	UB_LOG(UBL_ERROR, "%s:bad format values, item=%d, svalues=%s\n",
	       __func__, item, sv);
	if(*values) free(*values);
	*values=NULL;
	return -1;
}

int _CONFPREFIX_stritem_update(int item, int index, int findex, char **svalues, int *svalsize)
{
	void *p;
	int rsize;
	int foffset=0;
	void *values;
	if(index>_CONFPREFIX_conf_get_item_element_num(((_CONFPREFIX__config_item_t)item))) {
		UB_LOG(UBL_ERROR, "%s:item=%d must be wrong: %s\n", __func__, item, *svalues);
		return -1;
	}
	if(index<0) index=0;
	p=_CONFPREFIX_conf_get_item_index(((_CONFPREFIX__config_item_t)item), index);
	if(!p) {
		UB_LOG(UBL_ERROR, "%s:item=%d, index=%d must be wrong: %s\n",
		       __func__, item, index, *svalues);
		return -1;
	}
	rsize=_CONFPREFIX_stritem_values(item, index, findex, &values,
					 &foffset, svalues, svalsize);
	if(rsize<0) {
		UB_LOG(UBL_ERROR, "%s:item=%d, index=%d, findex=%d, failed to get values: %s\n",
		       __func__, item, index, findex, *svalues);
		return -1;
	}
	memcpy(p+foffset, values, rsize);
	free(values);
	return 0;
}

int _CONFPREFIX_variable_from_str(int *item, int *index, int *findex, char **svalues, int *svalsize)
{
	int i;
	char *sp, *q;
	char *ve=NULL;
	char *vn=NULL;
	if(index) *index=-1;
	if(findex) *findex=-1;
	if(!_CONFPREFIX_skip_chars(svalues, svalsize, ' ', '\t')) return -1;
	sp=*svalues;
	while(*svalsize>0 && **svalues){
		if(**svalues==' ' || **svalues=='\t'){
			if(!ve) ve=*svalues;
			break;
		}
		if(**svalues=='['){
			if(!ve) ve=*svalues;
			i=get_bracket_num(svalues, svalsize, false);
			if(i<0) i=0;
			if(index) *index=i;
			continue;
		}
		if(strstr(*svalues,".f")==*svalues){
			if(!ve) ve=*svalues;
			*svalues+=2;
			*svalsize-=2;
			i=strtol(*svalues, &q, 10);
			if(findex) *findex=i;
			*svalsize-=q-*svalues;
			*svalues=q;
			continue;
		}
		*svalues+=1;
		*svalsize-=1;
	}
	if(!ve) ve=*svalues;
	vn=malloc(ve-sp+1);
	memcpy(vn, sp, ve-sp);
	vn[ve-sp]=0;
	*item=_CONFPREFIX_conf_get_item_num(vn);
	free(vn);
	if(*item<0){
		UB_LOG(UBL_ERROR, "%s:failed to get variable: %s\n", __func__, sp);
		return -1;
	}
	return 0;
}

int _CONFPREFIX_read_config_file(char *fname)
{
	FILE *inf;
	char line[MAX_CHARS_PER_LINE];
	char *pline;
	int nr, index, findex, item;
	int res=0;

	inf=fopen(fname, "r");
	if(!inf) return -1;
	while((nr=_CONFPREFIX_conf_readline(inf, line))>0){
		pline=line;
		nr++; // add the last null code
		_CONFPREFIX_skip_chars(&pline, &nr, ' ', '\t');
		if(nr<=2) continue;
		if(check_first_char(&pline, &nr, '#')) continue;
		if(_CONFPREFIX_variable_from_str(&item, &index, &findex, &pline, &nr)){
			res=-1;
			break;
		}
		_CONFPREFIX_skip_chars(&pline, &nr, ' ', '\t');
		if(_CONFPREFIX_stritem_update(item, index, findex, &pline, &nr)){
			res=-1;
			break;
		}
	}
	fclose(inf);
	return res;
}

int _CONFPREFIX_read_config_buffer(int conf_num, char *conf_array[])
{
	char *pline;
	int nr, index, findex, item;
	int res=0;

	for(int idx = 0; idx < conf_num && conf_array[idx]; idx++){
		pline=conf_array[idx];
		nr = strlen(pline);
		nr++; // add the last null code
		_CONFPREFIX_skip_chars(&pline, &nr, ' ', '\t');
		if(nr<=1) continue;
		if(check_first_char(&pline, &nr, '#')) continue;
		if(_CONFPREFIX_variable_from_str(&item, &index, &findex, &pline, &nr)){
			res=-1;
			break;
		}
		_CONFPREFIX_skip_chars(&pline, &nr, ' ', '\t');
		if(_CONFPREFIX_stritem_update(item, index, findex, &pline, &nr)){
			res=-1;
			break;
		}
	}
	return res;
}

char *_CONFPREFIX_config_item_strings(int item)
{
	if(item>=_CONFPREFIX_ITEM_EXTEND_BASE)
		return get_extend_item_vname(item-_CONFPREFIX_ITEM_EXTEND_BASE);
	if(check_item(item)) return NULL;
	return config_item_strings[item];
}
