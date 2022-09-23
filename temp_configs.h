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
#ifndef ___CONFPREFIX__CONFING_H_
#define ___CONFPREFIX__CONFING_H_
#include <inttypes.h>
#include <stdbool.h>

/*_CONF_GENERATED_*/

#define _CONFPREFIX_ITEM_EXTEND_MAX_NAME 16
#define _CONFPREFIX_ITEM_EXTEND_BASE 10000
typedef struct _CONFPREFIX_item_extend {
	char name[_CONFPREFIX_ITEM_EXTEND_MAX_NAME];
	void *valuep;
	int vsizes[4];
	uint8_t ipcon;
} _CONFPREFIX_item_extend_t;

/**
 * @brief skip first appeared char a or b
 * @param svalues	pointer of string data of values
 * @param svalsize	pointer of size of the svalues
 * @param a	skip character
 * @param b	skip character
 * @return true:found skip character, false:no skip
 * @note *svalues and *svalsize are update to index the next to the skipped character
 */
bool _CONFPREFIX_skip_chars(char **svalues, int *svalsize, char a, char b);

/**
 * @brief skip to char a or b
 * @param svalues	pointer of string data of values
 * @param svalsize	pointer of size of the svalues
 * @param a	skip character
 * @param b	skip character
 * @return true:found skip character, false:no skip
 * @note *svalues and *svalsize are update to index the next to the skipped character
 */
bool _CONFPREFIX_skip_to_chars(char **svalues, int *svalsize, char a, char b);

/**
 * @brief a field vtype in struct
 * @param sindex	struct index number
 * @param findex	field index number
 * @param vsize	_CONFPREFIX_conf_get_item_value_size of the field
 * @param elen	_CONFPREFIX_conf_get_item_element_num of the field
 * @param vnum	_CONFPREFIX_conf_get_item_value_num of the field
 * @param foffset	the position of the field by byte offset from the struct data top.
 * @return vtype, -1:on error
 */
int _CONFPREFIX_struct_field_vtype(int sindex, int findex,
				   int *vsize, int *elen, int *vnum, int *foffset);

/**
 * @brief return variable pointer
 * @param item	variable
 * @return NULL:on error, pointer of variable 'item'
 */
void *_CONFPREFIX_conf_get_item(_CONFPREFIX__config_item_t item);

/**
 * @brief return IPC flags
 * @param item	variable
 * @return UPIPC_N(no)/UPIPC_R(readable)/UPIPC_W(writable)/UPIPC_RW(readbale and writable)
 */
uint8_t _CONFPREFIX_conf_get_ipcon(_CONFPREFIX__config_item_t item);

/**
 * @brief size of variable
 * @param item	variable
 * @return size of the variable; for an array, size of an element of the array;
 *	for a struct, size of the struct.
 */
int _CONFPREFIX_conf_get_item_value_size(_CONFPREFIX__config_item_t item);

/**
 * @brief number of 2nd dimension elements
 * @param item	variable
 * @return number of 2nd dimension elements
 */
int _CONFPREFIX_conf_get_item_value_num(_CONFPREFIX__config_item_t item);

/**
 * @brief size of one 1st dimension element
 * @param item	variable
 * @return size of one 1st dimension element
 */
int _CONFPREFIX_conf_get_item_element_size(_CONFPREFIX__config_item_t item);

/**
 * @brief number of 1st dimension elements
 * @param item	variable
 * @return number of 1st dimension elements
 */
int _CONFPREFIX_conf_get_item_element_num(_CONFPREFIX__config_item_t item);

/**
 * @brief variable type
 * @param item	variable
 * @return variable type number, which is defined in 'up_config_struct_t',
 *	'_CONFPREFIX__config_struct_t' or string type:UPSTRING_TYPE_BASE+strlen.
 */
int _CONFPREFIX_conf_get_item_vtype(_CONFPREFIX__config_item_t item);

/**
 * @brief return indexed variable pointer
 * @param item	variable
 * @param index	index
 * @return NULL:on error, pointer of indexed variable
 */
void *_CONFPREFIX_conf_get_item_index(_CONFPREFIX__config_item_t item, int index);

/**
 * @brief return int32_t type variable value
 * @param item	variable
 * @return 0x80000000:on error, integer number
 */
int32_t _CONFPREFIX_conf_get_intitem(_CONFPREFIX__config_item_t item);

/**
 * @brief return int32_t type variable value
 * @param item	variable
 * @param index	index
 * @return 0x80000000:on error, integer number
 */
int32_t _CONFPREFIX_conf_get_intitem_index(_CONFPREFIX__config_item_t item, int index);

/**
 * @brief return bool type variable value
 * @param item	variable
 * @return error is returned as false
 */
bool _CONFPREFIX_conf_get_boolitem(_CONFPREFIX__config_item_t item);

/**
 * @brief return int64_t type variable value
 * @param item	variable
 * @return 0x8000000000000000:on error, integer number
 */
int64_t _CONFPREFIX_conf_get_lintitem(_CONFPREFIX__config_item_t item);

/**
 * @brief Update variable 'item'
 * @param item	variable
 * @param v	new value data, MUST have the variable size
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_conf_set_item(_CONFPREFIX__config_item_t item, void *v);

/**
 * @brief Update indexed variable 'item'
 * @param item	variable
 * @param v	new value data, MUST have one index of the variable size
 * @param index	index
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_conf_set_item_index(_CONFPREFIX__config_item_t item, void *v, int index);

/**
 * @brief item number from a string
 * @param istr	variable name string
 * @return item number, -1:on error
 */
_CONFPREFIX__config_item_t _CONFPREFIX_conf_get_item_num(char *istr);

/**
 * @brief read from a runtime configuration file
 * @param fname	configuration file name
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_read_config_file(char *fname);

/**
 * @brief read from array of strings
 * @param conf_num	number of configuration in the conf array
 * @param conf_array	array of config strings
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_read_config_buffer(int conf_num, char *conf_array[]);

/**
 * @brief register a new variable dynamically
 * @param eid	a new variable data
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_register_extend_item(_CONFPREFIX_item_extend_t *eid);

/**
 * @brief de-register a dynamically registered variable
 * @param vname	dynamically registered variable name
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_remove_extend_item(char *vname);

/**
 * @brief de-register all dynamically registered variables
 * @return 0:success, -1:on error
 */
void _CONFPREFIX_removeall_extend_item(void);

/**
 * @brief a list of dynamically registered variables
 * @param data	a list of helloitem_extend_t in *data. *data must be freed by a caller.
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_get_extend_itemlist(void **data);

/**
 * @brief variable name from item number
 * @param item	variable
 * @return variable name, NULL:on error
 */
const char *_CONFPREFIX_config_item_strings(int item);

/**
 * @brief _CONFPREFIX_conf_get_item_num + _CONFPREFIX_conf_set_item
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_conf_set_stritem(char *istr, void *v);

/**
 * @brief update item, the update ends by sizev
 * @param item	variable
 * @param values	pointer of the update data
 * @param sizev	size of the values
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_item_update(int item, void *values, int sizev);

/**
 * @brief update item from index and findex, the update ends by sizev
 * @param item	variable
 * @param index	index
 * @param findex	field index
 * @param values	pointer of the update data
 * @param sizev	size of the values
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_item_index_update(int item, int index, int findex, void *values, int sizev);

/**
 * @brief update variable from index and findex by strindata, the update ends svalsize
 * @param vname	variable
 * @param index	index
 * @param findex	field index
 * @param svalues	string data of values
 * @param svalsize	size of the svalues
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_strvname_update(char *vname, int index, int findex, char **svalues, int *svalsize);

/**
 * @brief retrun values in string
 * @param item	variable
 * @param index	index
 * @param findex	field index
 * @param values	handle of the update data
 * @param foffset	the position of the field by byte offset from the struct data top.
 * @param svalues	pointer of string data of values
 * @param svalsize	pointer of size of the svalues
 * @return 0:success, -1:on error
 * @note *svalues is reallocated by the call and can expand to fit to the result string.
 *	*values MUST have enough data.  index=-1 or findex=-1 indicates the entire data.
 */
int _CONFPREFIX_stritem_values(int item, int index, int findex, void **values,
			       int *foffset, char **svalues, int *svalsize);

/**
 * @brief update item from index and findex by strindata, the update ends svalsize
 * @param item	variable
 * @param index	index
 * @param findex	field index
 * @param svalues	string data of values
 * @param svalsize	size of the svalues
 * @return 0:success, -1:on error
 */
int _CONFPREFIX_stritem_update(int item, int index, int findex, char **svalues, int *svalsize);

/**
 * @brief get item, index, findex from a string
 * @param *item	variable
 * @param *index	index
 * @param *findex	field index
 * @param svalues	pointer of string data of values
 * @param svalsize	pointer of size of the svalues
 * @return 0:success, -1:on error
 * @note index=N, findex=M is set by 'VARIABLE[N].fM'
 */
int _CONFPREFIX_variable_from_str(int *item, int *index, int *findex,
				  char **svalues, int *svalsize);

/**
 * @brief save persistent flagged data into the file
 */
void _CONFPREFIX_persistent_save(void);

/**
 * @brief restore persistent flagged data from the file
 */
void _CONFPREFIX_persistent_restore(void);

#endif
