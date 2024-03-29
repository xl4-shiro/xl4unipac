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
#ifndef ___CONFPREFIX__IPCCONFING_H_
#define ___CONFPREFIX__IPCCONFING_H_
#include <inttypes.h>
#include <stdbool.h>

/*_CONF_GENERATED_*/

char *_CONFPREFIX_item_values_to_string(int item, int index, int findex,
					void *values, int valsize, bool dnonidxed);

#ifndef __XL4UNIPACK_IPCDATA_
#define __XL4UNIPACK_IPCDATA_

#define XL4UNIPAC_IPCDATA_BMAGIC 0x00112233
#define XL4UNIPAC_IPCDATA_TMAGIC 0x54545454 //"TTTT"
typedef enum {
	XL4UNIPAC_IPCCMD_READ = 0,
	XL4UNIPAC_IPCCMD_WRITE,
	XL4UNIPAC_IPCCMD_NOTICE,
	XL4UNIPAC_IPCCMD_QEVLIST,
	XL4UNIPAC_IPCCMD_DISCONNECT,
} xl4unipac_ipccmd_t;

/* this is a format in the binary mode
   in text mode, "magic R/W/N/Q vname data_in_text"

   BTMODE, item=-1,
   This mode uses the binary mode, but the variable is passed not by
   item but by variable name string.
   the variable string size in one byte comes data[0]
   data[1:] is the variable string without null terminator.
   data[strlen(VARIABLE)+1:] is the binary data.
   'size' field includes this size, i.e. the binary data size + strlen(VARIABLE)+1
*/
typedef struct xl4unipac_ipcdata {
	uint32_t magic;
	int32_t cmd;
	int32_t item;
	int32_t index;
	int32_t findex;
	int32_t size;
	uint8_t data[4];
}__attribute__((packed)) xl4unipac_ipcdata_t;
#endif

/* TO BE UPPER COMPATIBLE */
/* The following 'XL4UNIPAC_*' -> '_CONFPREFIX_' definitions are to be compatible
   the old version. new programs should use  XL4UNIPAC_* */
#define _CONFPREFIX_IPCDATA_BMAGIC XL4UNIPAC_IPCDATA_BMAGIC
#define _CONFPREFIX_IPCDATA_TMAGIC XL4UNIPAC_IPCDATA_TMAGIC

#define _CONFPREFIX_IPCCMD_READ        XL4UNIPAC_IPCCMD_READ
#define _CONFPREFIX_IPCCMD_WRITE       XL4UNIPAC_IPCCMD_WRITE
#define _CONFPREFIX_IPCCMD_NOTICE      XL4UNIPAC_IPCCMD_NOTICE
#define _CONFPREFIX_IPCCMD_QEVLIST     XL4UNIPAC_IPCCMD_QEVLIST
#define _CONFPREFIX_IPCCMD_DISCONNECT  XL4UNIPAC_IPCCMD_DISCONNECT

#define _CONFPREFIX_ipccmd_t xl4unipac_ipccmd_t
#define _CONFPREFIX_ipcdata_t xl4unipac_ipcdata_t
/* END TO BE UPPER COMPATIBLE */

typedef int (* _CONFPREFIX_ipcserver_update_cb)(void *cbdata, int item, int index, int findex);

/**
 * @brief initialize ipcserver
 * @param node_ip	Unix domain socket inode when port=0, local IP port when port>0
 * @param port	0 to use Unix domain socket, local IP port number to use UDP port
 * @param thread	true:run in threading mode, false: run in non-threading mode
 * @return -1:on error, 0:normal
 */
int _CONFPREFIX_ipcserver_init(char *node_ip, uint16_t port, bool thread);

/**
 * @brief close ipcserver
 */
void _CONFPREFIX_ipcserver_close(void);

/**
 * @brief set up data update callback function
 * @param update_cb	callback function
 * @param update_cbdata	data on the callback function
 * @return -1:on error, 0:normal
 */
int _CONFPREFIX_ipcserver_set_update_cb(_CONFPREFIX_ipcserver_update_cb update_cb,
					void *update_cbdata);

/**
 * @brief ipcserver receive process, in non-threading mode this should be called by
 *	receiving fd event.  In threading mode, it is internally called from the thread.
 */
int _CONFPREFIX_ipcserver_receive(void);

/**
 * @brief send notice data
 * @param ipd	notice data parameter. when ipd->item is not registered ipd->data is sent.
 */
int _CONFPREFIX_ipcserver_send_notice(_CONFPREFIX_ipcdata_t *ipd);

/**
 * @brief lock ipcserver, when it is locked no update by IPC can happen.
 * @param tout_usec	lock timeout in usec
 * @note In non-threading mode, locking is not needed.
 */
int _CONFPREFIX_ipcserver_lock(int tout_usec);

/**
 * @brief unlock ipcserver
 */
void _CONFPREFIX_ipcserver_unlock(void);

/**
 * @brief get ipc socket file descriptor
 * @return file descriptor number, -1:on error
 */
int _CONFPREFIX_get_ipcfd(void);

/**
 * @brief write current data into a runtime configuration file
 * @param fname	configuration file name
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_write_config_file(char *fname);

#endif
