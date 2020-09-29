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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sample_configs.h"

int simple_value_update(void)
{
	int32_t v1,v2;
	int64_t lv1,lv2;
	double fv1,fv2;
	char cv1[10], cv2[10];
	bool bv1,bv2;
	char *a;
	int ssize;

	v1=sampleconf_get_intitem(VALUE_A_01);
	a="5678";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_A_01", 0, 0, &a, &ssize);
	v2=sampleconf_get_intitem(VALUE_A_01);
	printf("v1=%d, v2=%d\n", v1, v2);
	if(v2!=5678) return -1;

	lv1=sampleconf_get_lintitem(VALUE_A_04);
	a="5678912345678";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_A_04", 0, 0, &a, &ssize);
	lv2=sampleconf_get_lintitem(VALUE_A_04);
	printf("lv1=%"PRIi64", lv2=%"PRIi64"\n", lv1, lv2);
	if(lv2!=5678912345678) return -1;

	fv1=*((double*)sampleconf_get_item(VALUE_C_01));
	a="3.1415";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_C_01", 0, 0, &a, &ssize);
	fv2=*((double*)sampleconf_get_item(VALUE_C_01));
	fv1=3.1415;
	if(fv2!=fv1) return -1;

	snprintf(cv1, 10, "%s", (char*)sampleconf_get_item(VALUE_B_01));
	a="'X','Z','\0'";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_B_01", 0, 0, &a, &ssize);
	snprintf(cv2, 10, "%s", (char*)sampleconf_get_item(VALUE_B_01));
	printf("cv1=%s, cv2=%s\n", cv1, cv2);
	if(strcmp(cv2, "XZ")) return -1;

	a="\"PQR\"";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_B_02", 0, 0, &a, &ssize);
	snprintf(cv2, 10, "%s", (char*)sampleconf_get_item(VALUE_B_02));
	printf("cv1=%s, cv2=%s\n", cv1, cv2);
	if(strcmp(cv2, "PQR")) return -1;

	bv1=*((bool*)sampleconf_get_item(VALUE_G_02));
	a="true";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_G_02", 0, 0, &a, &ssize);
	bv2=*((bool*)sampleconf_get_item(VALUE_G_02));
	printf("bv1=%d, bv2=%d\n", bv1, bv2);
	if(!bv2) return -1;
	a="false";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_G_02", 0, 0, &a, &ssize);
	bv2=*((bool*)sampleconf_get_item(VALUE_G_02));
	printf("bv1=%d, bv2=%d\n", bv1, bv2);
	if(bv2) return -1;

	printf("%s:PASS\n",__func__);
	return 0;
}

int array_update(void)
{
	char *a;
	int ssize;
	int v1, v2;
	int *pv1;
	uint8_t *pv2;

	a="{{A0:A1},{b0:b1},{c0:C1}}";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_E_04", 0, 0, &a, &ssize);
	pv2=((uint8_t *)sampleconf_get_item_index(VALUE_E_04, 0));
	printf("v1=0x%x,0x%x\n", pv2[0],pv2[1]);
	if(pv2[0]!=0xa0 || pv2[1]!=0xa1) return -1;
	pv2=((uint8_t *)sampleconf_get_item_index(VALUE_E_04, 1));
	printf("v1=0x%x,0x%x\n", pv2[0],pv2[1]);
	if(pv2[0]!=0xb0 || pv2[1]!=0xb1) return -1;
	pv2=((uint8_t *)sampleconf_get_item_index(VALUE_E_04, 2));
	printf("v1=0x%x,0x%x\n", pv2[0],pv2[1]);
	if(pv2[0]!=0xc0 || pv2[1]!=0xc1) return -1;


	v1=*((int32_t *)sampleconf_get_item_index(VALUE_E_05, 1));
	a="100";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_E_05", 1, 0, &a, &ssize);
	v2=*((int32_t *)sampleconf_get_item_index(VALUE_E_05, 1));
	printf("v1=%d, v2=%d\n", v1, v2);
	if(v2!=100) return -1;
	a="200";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_E_05", 2, 0, &a, &ssize);
	v1=*((int32_t *)sampleconf_get_item_index(VALUE_E_05, 1));
	v2=*((int32_t *)sampleconf_get_item_index(VALUE_E_05, 2));
	printf("v1=%d, v2=%d\n", v1, v2);
	if(v1!=100) return -1;
	if(v2!=200) return -1;

	a="{11,22},{33,44},{55,66}";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_E_05", 0, 0, &a, &ssize);
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 0));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=11 || pv1[1]!=22) return -1;
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 1));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=33 || pv1[1]!=44) return -1;
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 2));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=55 || pv1[1]!=66) return -1;

	a="[3]{88,99}";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_E_05", 0, 0, &a, &ssize);
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 0));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=88 || pv1[1]!=99) return -1;
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 1));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=88 || pv1[1]!=99) return -1;
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 2));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=88 || pv1[1]!=99) return -1;

	a="{111,222},{333,444},{555,666}";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_E_05", 1, 0, &a, &ssize); // from index=1, {555,666} is ignored
	v1=*((int32_t *)sampleconf_get_item_index(VALUE_E_05, 0));
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 0));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=88 || pv1[1]!=99) return -1;
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 1));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=111 || pv1[1]!=222) return -1;
	pv1=((int32_t *)sampleconf_get_item_index(VALUE_E_05, 2));
	printf("v1=%d,%d\n", pv1[0],pv1[1]);
	if(pv1[0]!=333 || pv1[1]!=444) return -1;

	printf("%s:PASS\n",__func__);
	return 0;
}

int struct_update(void)
{
	char *a;
	int i,ssize;
	ABC_01_t *v1;
	double fv;

	v1=(ABC_01_t *)sampleconf_get_item(VALUE_P);
	a="{10,\"xyz\",9.9,true,{3,2,1},9,8,7,{\"a\",\"b\",\"c\"}}";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_P", 0, -1, &a, &ssize);
	printf("ABC_01 VALUE_P f0=%d,f1=%s,f2=%f,f3=%d\n", v1->f0,v1->f1,v1->f2,v1->f3);
	if(v1->f0!=10) return -1;
	printf("ABC_01 VALUE_P f4=%d,%d,%d\n", v1->f4[0],v1->f4[1],v1->f4[2]);
	if(v1->f4[0]!=3) return -1;
	printf("ABC_01 VALUE_P f5=%d,f6=%d,f7=%d\n", v1->f5,v1->f6,v1->f7);
	if(v1->f6!=8) return -1;
	printf("ABC_01 VALUE_P f8=%s,%s,%s\n", v1->f8[0],v1->f8[1],v1->f8[2]);
	if(strcmp(v1->f8[2],"c")) return -1;

	v1=(ABC_01_t *)sampleconf_get_item(VALUE_P);
	a="111,222,333";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_P", 0, 4, &a, &ssize);
	printf("ABC_01 VALUE_P f4=%d,%d,%d\n", v1->f4[0],v1->f4[1],v1->f4[2]);
	if(v1->f4[0]!=111 || v1->f4[1]!=222 || v1->f4[2]!=333) return -1;

	//ABC_01-VALUE_R[10] {10,"x",1.0,true,{1,2,3},{10,20,30}}
	a="[10]{99,\"pq\",1.1,false,{10,20,30},90,80,70,{\"X\",\"Y\",\"Z\"}}";
	ssize=strlen(a)+1;
	samplestrvname_update("VALUE_R", 0, -1, &a, &ssize);
	fv=1.1;
	for(i=0;i<10;i++){
		v1=(ABC_01_t *)sampleconf_get_item_index(VALUE_R, i);
		if(!v1) return -1;
		printf("ABC_01-VALUE_R[10] i=%d f0=%d,f1=%s,f2=%f,f3=%d\n",
		       i, v1->f0,v1->f1,v1->f2,v1->f3);
		printf("f4=%d,%d,%d\n", v1->f4[0],v1->f4[1],v1->f4[2]);
		printf("f5=%d,f6=%d,f7=%d\n", v1->f5,v1->f6,v1->f7);
		printf("f8=%s,%s,%s\n", v1->f8[0],v1->f8[1],v1->f8[2]);
		if(v1->f0!=99) return -1;
		if(strcmp(v1->f1,"pq")) return -1;
		if(v1->f2!=fv) return -1;
		if(v1->f3!=false) return -1;
		if(v1->f4[0]!=10) return -1;
		if(v1->f4[1]!=20) return -1;
		if(v1->f4[2]!=30) return -1;
		if(v1->f5!=90) return -1;
		if(v1->f6!=80) return -1;
		if(v1->f7!=70) return -1;
		if(strcmp(v1->f8[0],"X")) return -1;
		if(strcmp(v1->f8[1],"Y")) return -1;
		if(strcmp(v1->f8[2],"Z")) return -1;
	}
	// return back to the original values
	a="[10]{10,\"x\",1.0,true,{1,2,3},10,20,30}}";
	ssize=strlen(a)+1;
	if(samplestrvname_update("VALUE_R", 0, -1, &a, &ssize)) return -1;
	v1=(ABC_01_t *)sampleconf_get_item_index(VALUE_R, 0);
	if(memcmp(v1->f8,"\x00\x00\x00\x00\x00\x00\x00\x00\x00",9)){
		printf("undefined area should be initialized to zero\n");
		return -1;
	}

	printf("%s:PASS\n",__func__);
	return 0;
}

int get_variable(void)
{
	int item, index, findex;
	char *a;
	char *p;
	int asize;

	a="VALUE_C_01 9.99";
	asize=strlen(a)+1;
	p=a;
	if(samplevariable_from_str(&item, NULL, NULL, &a, &asize)) {
		printf("samplevariable_from_str error for '%s, %s'\n", p, a);
		return -1;
	}
	printf("item=%d, left str='%s', left size=%d\n", item, a, asize);
	if(item!=VALUE_C_01){
		printf("item mismatch: %d should be %d\n", item, VALUE_C_01);
		return -1;
	}
	if(strcmp(a, " 9.99")){
		printf("left chars '%s' should be ' 9.99'\n", a);
		return -1;
	}
	if(asize!=6){
		printf("left string size %d should be 6\n", asize);
		return -1;
	}

	a="VALUE_C_01";
	asize=strlen(a)+1;
	p=a;
	if(samplevariable_from_str(&item, NULL, NULL, &a, &asize)) {
		printf("samplevariable_from_str error for '%s, %s'\n", p, a);
		return -1;
	}
	printf("item=%d, left str='%s', left size=%d\n", item, a, asize);
	if(item!=VALUE_C_01){
		printf("item mismatch: %d should be %d\n", item, VALUE_C_01);
		return -1;
	}
	if(asize!=1){
		printf("left string size %d should be 1\n", asize);
		return -1;
	}

	a="VALUE_R[5].f3";
	asize=strlen(a)+1;
	p=a;
	if(samplevariable_from_str(&item, &index, &findex, &a, &asize)){
		printf("samplevariable_from_str error for '%s, %s'\n", p, a);
		return -1;
	}
	printf("item=%d, left str='%s', left size=%d\n", item, a, asize);
	if(item!=VALUE_R){
		printf("item mismatch: %d should be %d\n", item, VALUE_R);
		return -1;
	}
	if(index!=5){
		printf("index: %d should be 5\n", index);
		return -1;
	}
	if(findex!=3){
		printf("findex: %d should be 3\n", findex);
		return -1;
	}

	printf("%s:PASS\n",__func__);
	return 0;
}

int update_from_conffile(void)
{
	ABC_01_t *sv;
	double fv;
	if(sampleread_config_file("sample_defaults.conf")){
		printf("failed to update from 'sample_defaults.conf'\n");
		return -1;
	}
	if(sampleconf_get_intitem(VALUE_A_01)!=1324) return -1;
	if(sampleconf_get_intitem(VALUE_A_02)!=-1324) return -1;
	if(sampleconf_get_intitem(VALUE_A_03)!=2147483647) return -1;
	printf("VALUE_A_01/02(int) was updated properly\n");
	fv=1.324;
	if(*((double*)sampleconf_get_item(VALUE_C_01))!=fv) return -1;
	printf("VALUE_C_01(double) was updated properly\n");

	printf("%s\n", (char*)sampleconf_get_item(VALUE_F_04));
	if(strcmp((char*)sampleconf_get_item(VALUE_F_04),"abcde fghij")) return -1;

	sv=(ABC_01_t*)sampleconf_get_item_index(VALUE_R, 3);
	if(strcmp(sv->f1,"x")) return -1;
	sv=(ABC_01_t*)sampleconf_get_item_index(VALUE_R, 4);
	if(sv->f0!=2) return -1;
	if(strcmp(sv->f1,"1324")) return -1;
	sv=(ABC_01_t*)sampleconf_get_item_index(VALUE_R, 5);
	if(strcmp(sv->f1,"x")) return -1;
	printf("VALUE_R(ABC_01_t) was updated properly\n");

	printf("%s:PASS\n",__func__);
	return 0;
}

int extend_item(void)
{
	int res;
	int eitem;
	int32_t v=1111;
	char *svalues;
	int svalsize;
	sampleitem_extend_t eid={"EXTVAR1", &v,
				 {sizeof(int32_t), 1, 1, VT_INT32_T}, UPIPC_RW};
	res=sampleregister_extend_item(&eid);
	if(res) return -1;
	printf("registered EXTVAR1\n");

	eitem=sampleconf_get_item_num("EXTVAR1");
	if(eitem<0) return -1;
	if(sampleconf_get_intitem(eitem)!=v) return -1;
	printf("got a value of the registered variable\n");

	svalues="2222";
	svalsize=strlen(svalues)+1;
	res=samplestrvname_update("EXTVAR1", 0, -1, &svalues, &svalsize);
	if(res<0) return -1;
	if(sampleconf_get_intitem(eitem)!=2222) return -1;
	printf("the registered variable was updated\n");

	res=sampleremove_extend_item("EXTVAR1");
	if(res) return -1;
	printf("removed EXTVAR1\n");

	sampleremoveall_extend_item();
	return 0;
}

int main(int argc, char* argv[])
{
	if(simple_value_update()) return -1;
	if(array_update()) return -1;
	if(struct_update()) return -1;
	if(get_variable()) return -1;
	if(update_from_conffile()) return -1;
	if(extend_item()) return -1;
	return 0;
}
