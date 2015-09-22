//
// Created by Dusan Klinec on 16.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_COMMON_H
#define OPENSIPS_1_11_2_TLS_COMMON_H

#define SET_STR_VAL(_str, _res, _r, _c)	\
	if (RES_ROWS(_res)[_r].values[_c].nul == 0) \
	{ \
		switch(RES_ROWS(_res)[_r].values[_c].type) \
		{ \
		case DB_STRING: \
			(_str).s=(char*)RES_ROWS(_res)[_r].values[_c].val.string_val; \
			(_str).len=strlen((_str).s); \
			break; \
		case DB_STR: \
			(_str).len=RES_ROWS(_res)[_r].values[_c].val.str_val.len; \
			(_str).s=(char*)RES_ROWS(_res)[_r].values[_c].val.str_val.s; \
			break; \
		case DB_BLOB: \
			(_str).len=RES_ROWS(_res)[_r].values[_c].val.blob_val.len; \
			(_str).s=(char*)RES_ROWS(_res)[_r].values[_c].val.blob_val.s; \
			break; \
		default: \
			(_str).len=0; \
			(_str).s=NULL; \
		} \
	}



#define S_TABLE_VERSION 6

#endif //OPENSIPS_1_11_2_TLS_COMMON_H
