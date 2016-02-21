
#include "parse_req.h"
#include <stdio.h>

%%{
machine req_machine;

variable cs (data->cs);
variable p (data->p);
variable pe (data->pe);

action true_cond {
	data->flag = 1;
}

action false_cond {
	data->flag = 0;
}

action length {
	data->length *= 10;
	data->length += *data->p - '0';	
}

action on_err {
	data->err = 1;
}

ID = ^[ \r\n];
JS = (ID+ "." [jJ] [Ss] ((" " +) | ("?" ID* " "+))) @false_cond;
CSS = (ID+ "." [Cc] [Ss] [Ss] (" "+ | ("?" ID " "+))) @false_cond;

ContentLength = "Content-Length" ' ' * ":" ' ' * ((digit+) $length) ' ' *;

CRLF = "\r\n";
METHOD = ("GET" ' '+) @true_cond | ((ID+) ' '+);
URL = JS | CSS | (ID + ' ' +);
VER = "HTTP/" ( "0.9" | "1.0" | "1.1" ) ' '*;
reqline = METHOD  URL VER CRLF;
headline = (ContentLength | ((^[\r\n])+) ) CRLF;
headend = CRLF @{fbreak;};

write data;
}%%

void parse_req_beg(struct parse_data* data)
{
	%%{
		write init;
	}%%
}

void parse_req(struct parse_data* data)
{
	const char* eof = 0;
	%%{
	main := (reqline headline + headend )$err(on_err);
	write exec;
	}%%
}

