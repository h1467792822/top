
#include "parse.h"
#include <stdio.h>

%%{
machine req_machine;

variable cs (data->cs);
variable p (data->p);
variable pe (data->pe);

action true_cond {
	data->true_flag = 1;
}

action false_cond {
	data->false_flag = 1;
}

action length {
	data->length *= 10;
	data->length += *data->p - '0';	
}

action on_err {
	data->err = 1;
}

ID = ^[ \r\n];

HtmlType = "Content-Type" ' ' * ":" ' ' * "text/html" ' ' *;
ContentLength = "Content-Length" ' ' * ":" ' ' * ((digit+) $length) ' ' *;
ContentEncoding = "Content-Encoding" ' ' * ":" ' ' * (^[\r\n])+;
TransferEncoding = "Transfer-Encoding" ' ' * ":" ' ' * (^[\r\n])+;

CRLF = "\r\n";
VER = "HTTP/" ( "0.9" | "1.0" | "1.1" ) ' '*;
CODE = ("200 " @true_cond | (digit+ " ") @false_cond) (^[\r\n])+;
rsqline = VER CODE CRLF;
headline = (ContentLength | ContentEncoding @false_cond | TransferEncoding @false_cond | HtmlType @true_cond | ((^[\r\n])+) ) CRLF;
headend = CRLF @{fbreak;};

write data;
}%%

void parse_rsq_beg(struct parse_data* data)
{
	%%{
		write init;
	}%%
}

void parse_rsq(struct parse_data* data)
{
	const char* eof = 0;
	%%{
	main := (rsqline headline + headend )$err(on_err);
	write exec;
	}%%
}

