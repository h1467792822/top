
#line 1 "parse_rsp.rl"

#include "parse.h"
#include <stdio.h>


#line 9 "parse_rsp.c"
static const char _req_machine_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 2, 0, 1
};

static const unsigned char _req_machine_key_offsets[] = {
	0, 0, 1, 2, 3, 4, 5, 7, 
	8, 9, 13, 16, 18, 20, 21, 25, 
	27, 28, 32, 33, 36, 39, 42, 45, 
	48, 51, 54, 59, 62, 65, 68, 71, 
	74, 77, 80, 84, 86, 89, 92, 95, 
	98, 101, 105, 110, 114, 117, 120, 123, 
	127, 131, 134, 137, 140, 143, 146, 149, 
	152, 155, 158, 161, 164, 167, 170, 173, 
	176, 179, 182, 185, 189, 193, 196, 197, 
	199
};

static const char _req_machine_trans_keys[] = {
	72, 84, 84, 80, 47, 48, 49, 46, 
	57, 32, 50, 48, 57, 32, 48, 57, 
	10, 13, 10, 13, 10, 10, 13, 67, 
	84, 10, 13, 10, 10, 13, 67, 84, 
	10, 10, 13, 111, 10, 13, 110, 10, 
	13, 116, 10, 13, 101, 10, 13, 110, 
	10, 13, 116, 10, 13, 45, 10, 13, 
	69, 76, 84, 10, 13, 110, 10, 13, 
	99, 10, 13, 111, 10, 13, 100, 10, 
	13, 105, 10, 13, 110, 10, 13, 103, 
	10, 13, 32, 58, 10, 13, 10, 13, 
	101, 10, 13, 110, 10, 13, 103, 10, 
	13, 116, 10, 13, 104, 10, 13, 32, 
	58, 10, 13, 32, 48, 57, 10, 13, 
	48, 57, 10, 13, 121, 10, 13, 112, 
	10, 13, 101, 10, 13, 32, 58, 10, 
	13, 32, 116, 10, 13, 101, 10, 13, 
	120, 10, 13, 116, 10, 13, 47, 10, 
	13, 104, 10, 13, 116, 10, 13, 109, 
	10, 13, 108, 10, 13, 32, 10, 13, 
	114, 10, 13, 97, 10, 13, 110, 10, 
	13, 115, 10, 13, 102, 10, 13, 101, 
	10, 13, 114, 10, 13, 45, 10, 13, 
	69, 32, 48, 49, 57, 32, 48, 49, 
	57, 32, 48, 57, 46, 48, 49, 0
};

static const char _req_machine_single_lengths[] = {
	0, 1, 1, 1, 1, 1, 2, 1, 
	1, 2, 1, 2, 2, 1, 4, 2, 
	1, 4, 1, 3, 3, 3, 3, 3, 
	3, 3, 5, 3, 3, 3, 3, 3, 
	3, 3, 4, 2, 3, 3, 3, 3, 
	3, 4, 3, 2, 3, 3, 3, 4, 
	4, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 2, 2, 1, 1, 0, 
	0
};

static const char _req_machine_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 1, 1, 1, 0, 1, 
	0
};

static const short _req_machine_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 10, 13, 
	15, 17, 21, 24, 27, 30, 32, 37, 
	40, 42, 47, 49, 53, 57, 61, 65, 
	69, 73, 77, 83, 87, 91, 95, 99, 
	103, 107, 111, 116, 119, 123, 127, 131, 
	135, 139, 144, 149, 153, 157, 161, 165, 
	170, 175, 179, 183, 187, 191, 195, 199, 
	203, 207, 211, 215, 219, 223, 227, 231, 
	235, 239, 243, 247, 251, 255, 258, 260, 
	262
};

static const char _req_machine_indicies[] = {
	1, 0, 2, 0, 3, 0, 4, 0, 
	5, 0, 6, 7, 0, 8, 0, 9, 
	0, 9, 11, 10, 0, 12, 10, 0, 
	0, 0, 13, 0, 14, 13, 15, 0, 
	0, 0, 17, 18, 16, 0, 19, 16, 
	20, 0, 0, 21, 17, 18, 16, 22, 
	0, 0, 19, 23, 16, 0, 19, 24, 
	16, 0, 19, 25, 16, 0, 19, 26, 
	16, 0, 19, 27, 16, 0, 19, 28, 
	16, 0, 19, 29, 16, 0, 19, 30, 
	31, 32, 16, 0, 19, 33, 16, 0, 
	19, 34, 16, 0, 19, 35, 16, 0, 
	19, 36, 16, 0, 19, 37, 16, 0, 
	19, 38, 16, 0, 19, 39, 16, 0, 
	19, 39, 40, 16, 0, 19, 41, 0, 
	19, 42, 16, 0, 19, 43, 16, 0, 
	19, 44, 16, 0, 19, 45, 16, 0, 
	19, 46, 16, 0, 19, 46, 47, 16, 
	0, 19, 47, 48, 16, 0, 19, 48, 
	16, 0, 19, 49, 16, 0, 19, 50, 
	16, 0, 19, 51, 16, 0, 19, 51, 
	52, 16, 0, 19, 52, 53, 16, 0, 
	19, 54, 16, 0, 19, 55, 16, 0, 
	19, 56, 16, 0, 19, 57, 16, 0, 
	19, 58, 16, 0, 19, 59, 16, 0, 
	19, 60, 16, 0, 19, 61, 16, 0, 
	19, 61, 16, 0, 19, 62, 16, 0, 
	19, 63, 16, 0, 19, 64, 16, 0, 
	19, 65, 16, 0, 19, 66, 16, 0, 
	19, 67, 16, 0, 19, 68, 16, 0, 
	19, 69, 16, 0, 19, 30, 16, 12, 
	70, 10, 0, 12, 71, 10, 0, 72, 
	10, 0, 73, 0, 9, 0, 0, 0
};

static const char _req_machine_trans_targs[] = {
	0, 2, 3, 4, 5, 6, 7, 70, 
	8, 9, 10, 67, 11, 12, 13, 14, 
	15, 19, 58, 16, 17, 18, 72, 20, 
	21, 22, 23, 24, 25, 26, 27, 36, 
	44, 28, 29, 30, 31, 32, 33, 34, 
	35, 35, 37, 38, 39, 40, 41, 42, 
	43, 45, 46, 47, 48, 49, 50, 51, 
	52, 53, 54, 55, 56, 57, 59, 60, 
	61, 62, 63, 64, 65, 66, 68, 69, 
	11, 71
};

static const char _req_machine_trans_actions[] = {
	7, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 3, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 9, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 3, 0, 0, 0, 0, 0, 0, 
	5, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	11, 0
};

static const char _req_machine_eof_actions[] = {
	0, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	7, 7, 7, 7, 7, 7, 7, 7, 
	0
};

static const int req_machine_start = 1;
static const int req_machine_first_final = 72;
static const int req_machine_error = 0;

static const int req_machine_en_main = 1;


#line 44 "parse_rsp.rl"


void parse_rsq_beg(struct parse_data* data)
{
	
#line 183 "parse_rsp.c"
	{
	( (data->cs)) = req_machine_start;
	}

#line 50 "parse_rsp.rl"

}

void parse_rsq(struct parse_data* data)
{
	const char* eof = 0;
	
#line 196 "parse_rsp.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( ( (data->p)) == ( (data->pe)) )
		goto _test_eof;
	if ( ( (data->cs)) == 0 )
		goto _out;
_resume:
	_keys = _req_machine_trans_keys + _req_machine_key_offsets[( (data->cs))];
	_trans = _req_machine_index_offsets[( (data->cs))];

	_klen = _req_machine_single_lengths[( (data->cs))];
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

	_klen = _req_machine_range_lengths[( (data->cs))];
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
	( (data->cs)) = _req_machine_trans_targs[_trans];

	if ( _req_machine_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _req_machine_actions + _req_machine_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 12 "parse_rsp.rl"
	{
	data->true_flag = 1;
}
	break;
	case 1:
#line 16 "parse_rsp.rl"
	{
	data->false_flag = 1;
}
	break;
	case 2:
#line 20 "parse_rsp.rl"
	{
	data->length *= 10;
	data->length += *data->p - '0';	
}
	break;
	case 3:
#line 25 "parse_rsp.rl"
	{
	data->err = 1;
}
	break;
	case 4:
#line 41 "parse_rsp.rl"
	{{( (data->p))++; goto _out; }}
	break;
#line 299 "parse_rsp.c"
		}
	}

_again:
	if ( ( (data->cs)) == 0 )
		goto _out;
	if ( ++( (data->p)) != ( (data->pe)) )
		goto _resume;
	_test_eof: {}
	if ( ( (data->p)) == eof )
	{
	const char *__acts = _req_machine_actions + _req_machine_eof_actions[( (data->cs))];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
#line 25 "parse_rsp.rl"
	{
	data->err = 1;
}
	break;
#line 321 "parse_rsp.c"
		}
	}
	}

	_out: {}
	}

#line 59 "parse_rsp.rl"

}

