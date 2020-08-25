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
#include <unistd.h>
#include <xl4unibase/unibase.h>
#include <xl4unibase/unibase_binding.h>
#include "_CONFPREFIX__configs.h"
#include "_CONFPREFIX__ipcconfigs.h"

static int ipc_update_cb(void *cbdata, int item, int index, int findex)
{
	char istr[6]={0,};
	char fstr[6]={0,};
	char *vname;
	vname=_CONFPREFIX_config_item_strings(item);
	if(!vname){
		UB_LOG(UBL_ERROR, "%s:wrong item=%d\n", __func__, item);
		return -1;
	}
	if(index>=0 && index<1000) snprintf(istr, 6, "[%d]", index);
	if(findex>=0 && findex<1000) snprintf(fstr, 6, ".f%d", findex);
	printf("Update:%s%s%s\n", vname, istr, fstr);
	return 0;
}

int main(int argc, char *argv[])
{
	unibase_init_para_t init_para;
	char c;

	ubb_default_initpara(&init_para);
	init_para.ub_log_initstr=UBL_OVERRIDE_ISTR("4,ubase:45,combase:45,unip:45", "UBL_UNIP");
	unibase_init(&init_para);

	if(_CONFPREFIX_read_config_file("_CONFPREFIX_.conf")){
		printf("failed to update from '_CONFPREFIX_.conf'\n");
		goto erexit;
	}
	_CONFPREFIX_ipcserver_init("/tmp/_CONFPREFIX__ipcconf", 0, true);
	_CONFPREFIX_ipcserver_set_update_cb(ipc_update_cb, NULL);
	read(0, &c, 1);
	_CONFPREFIX_ipcserver_close();

erexit:
	unibase_close();
	return 0;
}
