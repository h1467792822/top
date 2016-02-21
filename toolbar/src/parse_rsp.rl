
#include "parse.h"
#include <stdio.h>

%%{
machine req_machine;

variable p (data->p);
variable pe (data->pe);
access data->;

action true_cond {
	data->true_flag = 1;
	parse_data_dump(data,"true_cond");
}

action false_cond {
	data->false_flag = 1;
	parse_data_dump(data,"false_cond");
}

action length {
	data->length *= 10;
	data->length += *data->p - '0';	
}

action on_err {
	data->err = 1;
	parse_data_dump(data,"on_err");
}

ID = ^[ \r\n];

HtmlType = "Content-Type" ' ' * ":" ' ' * "text/html" ' ' *;
ContentLength = "Content-Length" ' ' * ":" ' ' * ((digit+) $length) ' ' *;
ContentEncoding = "Content-Encoding" ' ' * ":" ' ' * (^[\r\n])+;
TransferEncoding = "Transfer-Encoding" ' ' * ":" ' ' * (^[\r\n])+;

CRLF = "\r\n";
VER = "HTTP/" ( "0.9" | "1.0" | "1.1" ) ' '*;
OK = "200";
CODE = (OK " " @true_cond | (((digit+) - OK) " ") @false_cond) (^[\r\n])+;
rspline = VER CODE CRLF;
headline = (ContentLength | ContentEncoding @false_cond | TransferEncoding @false_cond | HtmlType @true_cond | ((^[\r\n])+) ) CRLF;
headend = CRLF @{fbreak;};

write data;
}%%

void parse_rsp_beg(struct parse_data* data)
{
	%%{
		write init;
	}%%
}

void parse_rsp(struct parse_data* data)
{
	const char* eof = 0;
	%%{
	main := (rspline headline + headend )$err(on_err);
	write exec noend;
	}%%
}

