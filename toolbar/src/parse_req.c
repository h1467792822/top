
#line 1 "parse_req.rl"

#include "parse.h"
#include <stdio.h>


#line 9 "parse_req.c"
static const char _req_machine_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4
};

static const unsigned char _req_machine_key_offsets[] = {
	0, 0, 4, 7, 10, 14, 16, 17, 
	18, 19, 20, 22, 23, 24, 26, 27, 
	30, 32, 33, 36, 37, 40, 43, 46, 
	49, 52, 55, 58, 61, 64, 67, 70, 
	73, 76, 80, 85, 89, 90, 92, 100, 
	106, 112, 117, 119, 123, 127, 135, 141, 
	146, 149, 153, 157, 160, 163
};

static const char _req_machine_trans_keys[] = {
	10, 13, 32, 71, 10, 13, 32, 10, 
	13, 32, 10, 13, 32, 46, 32, 72, 
	84, 84, 80, 47, 48, 49, 46, 57, 
	13, 32, 10, 10, 13, 67, 10, 13, 
	10, 10, 13, 67, 10, 10, 13, 111, 
	10, 13, 110, 10, 13, 116, 10, 13, 
	101, 10, 13, 110, 10, 13, 116, 10, 
	13, 45, 10, 13, 76, 10, 13, 101, 
	10, 13, 110, 10, 13, 103, 10, 13, 
	116, 10, 13, 104, 10, 13, 32, 58, 
	10, 13, 32, 48, 57, 10, 13, 48, 
	57, 46, 48, 49, 10, 13, 32, 46, 
	67, 74, 99, 106, 10, 13, 32, 46, 
	83, 115, 10, 13, 32, 46, 83, 115, 
	10, 13, 32, 46, 63, 32, 72, 10, 
	13, 32, 46, 10, 13, 32, 46, 10, 
	13, 32, 46, 67, 74, 99, 106, 10, 
	13, 32, 46, 83, 115, 10, 13, 32, 
	46, 63, 10, 13, 32, 10, 13, 32, 
	69, 10, 13, 32, 84, 10, 13, 32, 
	10, 13, 32, 0
};

static const char _req_machine_single_lengths[] = {
	0, 4, 3, 3, 4, 2, 1, 1, 
	1, 1, 2, 1, 1, 2, 1, 3, 
	2, 1, 3, 1, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 4, 3, 2, 1, 0, 8, 6, 
	6, 5, 2, 4, 4, 8, 6, 5, 
	3, 4, 4, 3, 3, 0
};

static const char _req_machine_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 1, 0, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const short _req_machine_index_offsets[] = {
	0, 0, 5, 9, 13, 18, 21, 23, 
	25, 27, 29, 32, 34, 36, 39, 41, 
	45, 48, 50, 54, 56, 60, 64, 68, 
	72, 76, 80, 84, 88, 92, 96, 100, 
	104, 108, 113, 118, 122, 124, 126, 135, 
	142, 149, 155, 158, 163, 168, 177, 184, 
	190, 194, 199, 204, 208, 212
};

static const char _req_machine_indicies[] = {
	1, 1, 1, 2, 0, 1, 1, 3, 
	0, 1, 1, 3, 4, 1, 1, 5, 
	6, 4, 5, 7, 1, 8, 1, 9, 
	1, 10, 1, 11, 1, 12, 13, 1, 
	14, 1, 15, 1, 16, 15, 1, 17, 
	1, 1, 1, 19, 18, 1, 20, 18, 
	21, 1, 1, 22, 19, 18, 23, 1, 
	1, 20, 24, 18, 1, 20, 25, 18, 
	1, 20, 26, 18, 1, 20, 27, 18, 
	1, 20, 28, 18, 1, 20, 29, 18, 
	1, 20, 30, 18, 1, 20, 31, 18, 
	1, 20, 32, 18, 1, 20, 33, 18, 
	1, 20, 34, 18, 1, 20, 35, 18, 
	1, 20, 36, 18, 1, 20, 36, 37, 
	18, 1, 20, 37, 38, 18, 1, 20, 
	38, 18, 39, 1, 15, 1, 1, 1, 
	5, 6, 40, 41, 40, 41, 4, 1, 
	1, 5, 6, 42, 42, 4, 1, 1, 
	5, 6, 43, 43, 4, 1, 1, 44, 
	6, 45, 4, 44, 7, 1, 1, 1, 
	5, 47, 46, 1, 1, 44, 6, 4, 
	1, 1, 44, 6, 40, 41, 40, 41, 
	4, 1, 1, 5, 6, 48, 48, 4, 
	1, 1, 44, 6, 49, 4, 1, 1, 
	44, 49, 1, 1, 3, 50, 0, 1, 
	1, 3, 51, 0, 1, 1, 52, 0, 
	1, 1, 52, 4, 1, 0
};

static const char _req_machine_trans_targs[] = {
	2, 0, 49, 3, 4, 5, 38, 6, 
	7, 8, 9, 10, 11, 36, 12, 13, 
	14, 15, 16, 20, 17, 18, 19, 53, 
	21, 22, 23, 24, 25, 26, 27, 28, 
	29, 30, 31, 32, 33, 34, 35, 37, 
	39, 46, 40, 41, 42, 43, 44, 45, 
	47, 48, 50, 51, 52
};

static const char _req_machine_trans_actions[] = {
	0, 7, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 9, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 5, 0, 
	0, 0, 0, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 1
};

static const char _req_machine_eof_actions[] = {
	0, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 0
};

static const int req_machine_start = 1;
static const int req_machine_first_final = 53;
static const int req_machine_error = 0;

static const int req_machine_en_main = 1;


#line 44 "parse_req.rl"


void parse_req_beg(struct parse_data* data)
{
	
#line 152 "parse_req.c"
	{
	 data->cs = req_machine_start;
	}

#line 50 "parse_req.rl"

}

void parse_req(struct parse_data* data)
{
	const char* eof = 0;
	
#line 165 "parse_req.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( ( (data->p)) == ( (data->pe)) )
		goto _test_eof;
	if (  data->cs == 0 )
		goto _out;
_resume:
	_keys = _req_machine_trans_keys + _req_machine_key_offsets[ data->cs];
	_trans = _req_machine_index_offsets[ data->cs];

	_klen = _req_machine_single_lengths[ data->cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*( (data->p))) < *_mid )
				_upper = _mid - 1;
			else if ( (*( (data->p))) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _req_machine_range_lengths[ data->cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*( (data->p))) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*( (data->p))) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _req_machine_indicies[_trans];
	 data->cs = _req_machine_trans_targs[_trans];

	if ( _req_machine_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _req_machine_actions + _req_machine_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 12 "parse_req.rl"
	{
	data->true_flag = 1;
}
	break;
	case 1:
#line 16 "parse_req.rl"
	{
	data->false_flag = 1;
}
	break;
	case 2:
#line 20 "parse_req.rl"
	{
	data->length *= 10;
	data->length += *data->p - '0';	
}
	break;
	case 3:
#line 25 "parse_req.rl"
	{
	data->err = 1;
}
	break;
	case 4:
#line 41 "parse_req.rl"
	{{( (data->p))++; goto _out; }}
	break;
#line 268 "parse_req.c"
		}
	}

_again:
	if (  data->cs == 0 )
		goto _out;
	if ( ++( (data->p)) != ( (data->pe)) )
		goto _resume;
	_test_eof: {}
	if ( ( (data->p)) == eof )
	{
	const char *__acts = _req_machine_actions + _req_machine_eof_actions[ data->cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
#line 25 "parse_req.rl"
	{
	data->err = 1;
}
	break;
#line 290 "parse_req.c"
		}
	}
	}

	_out: {}
	}

#line 59 "parse_req.rl"

}

