#include "common.h"

void PrintPMatchHelp()
{
printf("CRAYONIZER PMATCH HELP\n\n");
printf("Because I couldn't get my head around regular expressions, Crayonizer uses it's own pattern match system, called 'pmatch'. I find that it's simple and powerful, YMMV.\n\n");
printf("As this is the first release there are probably still some bugs in the pmatch code, and new matches and features are likely to be added in future\n\n");
printf("PMatch Operators:\n\n");
printf("	?:	Match any single character\n");
printf("	*:	Match any substring\n");
printf("	^:	Match start of line\n");
printf("	$:	Match end of line\n");
printf("	[]:	Match list of characters. e.g. [123456789]\n");
printf("	\\:	Quote a character, so it's not interpreted, unless it's one of the following interpretations.\n");
printf("	\\xFF:	Match a character by hex value, where 'FF' is the hex value. e.g. \\x20 is 'space'\n");
printf("	\\000:	Match a character by octal value, where '000' is the octal value. e.g. \\x040 is 'space'\n");
printf("	\\+S	Turn pmatch switch on or off, where 'S' is the switch character.\n");
printf("		( Available switches: 'C' (case sensitivity) 'X' (text extraction) 'W' (wildcards) )\n");
printf("	\\b:	backspace\n");
printf("	\\e:	escape\n");
printf("	\\l:	Any lowercase alphabectic character\n");
printf("	\\n:	newline\n");
printf("	\\r:	carriage return\n");
printf("	\\t:	tab\n");
printf("	\\A:	Any alphabetic charcter\n");
printf("	\\B:	Any alpha-numeric character\n");
printf("	\\D:	Any decimal digit\n");
printf("	\\S:	Any whitespace character\n");
printf("	\\P:	Any punctuation character\n");
printf("	\\X:	Any hexadecimal digit\n");
printf("	\\U:	Any uppercase alphabetic character\n");
printf("\nPMatch Switches:\n\n");
printf("The \\+ and \\- operators can be used to turn on and off certain features of the pmatch system. For instance, \\-C would turn off case sensitivity, and \\+C would turn it back on again.\n");
printf("The \\+X and \\-X switches turn 'text extraction' on and off. This allows you to match a string, but only crayonize a substring of the matched string. For example:\n\n");
printf("		\\-X Match this whole string but \\+X Crayonize only this bit \\-X but not this bit\n\n");
printf("the \\-W switch turns off wildcarding, so only switches are honored. This turns the pmatch into a straight strcmp until turned back on with \\+W\n");
}


void PrintConfigFileHelp()
{
printf("CRAYONIZER CONFIG HELP\n\n");
printf("Crayonizer first looks in ~/.crayonizer.conf and then in /etc/crayonizer.conf for its settings.\n");
printf("Settings files MUST contain an entry like:\n\n	CrayonizerDir /usr/prebin\n\nSo that crayonizer knows where it's installed, so that it doesn't call itself and create a spawning loop (forkbomb).\n\n");
printf("An example crayonizer.conf is provided with the distribution.\n\n");

printf("For each program that you want to crayonize, there should be an entry something like:\n\n");
printf("entry make\n{\n  line ' version ' bold magenta\n  line ' error: *' red\n  line ' undefined reference to ' red\n  line ' warning: ' yellow\n  line ' note: ' cyan\n  string '^*:' bold\n  string '\\-XTarget: *\\+X*\\-X\\S' bold inverse yellow\n}\n\n");

printf("'entry' lines have the form '<action type> <pattern match> <attributes>' where:\n\n");
printf("	action types: \n");
printf("		'if' very simple if statement, that can check environment variables and the like.\n");
printf("		'line' to crayonize an entire line.\n");
printf("		'string' to crayonize a substring\n");
printf("		'value' to crayonize a substring based on its numeric value (see below)\n");
printf("		'mapto' to crayonize a by picking from a list of options (see below)\n");
printf("		'linemapto' crayonize an entire line by matching a substring and by picking from a list of options (see below)\n");
printf("		'prepend' add a line before program output.\n");
printf("		'append' add a line after program output.\n");
printf("		'cmdline' match something on the command-line instead of in program output\n");
printf("		'args' is a special case, see below.\n");
printf("		'passinput'	Pass keyboard input to program whose output we are crayonizing\n"); 
printf("	pattern match: a 'pmatch' pattern (crayonizer -pmatch-help explains this).\n");
printf("	attributes: crayonizations to apply.\n\n");
printf("Available crayonizations are:\n");
printf("	dontcrayon: Abort crayonizations, just run program\n");
printf("	black, white, red, yellow, green, blue, magenta, cyan: Colors\n");
printf("	bold, blink, underline, inverse: Terminal attributes.\n");
printf("	uppercase, lowercase, caps, hide, basename: Text conversions.\n");
printf("	setxtitle, restorextitle: Change and restore xterm title bar (restore only works in aterm).\n");
printf("	fgcolor bgcolor xtermfgcolor xtermbgcolor rxvtfgcolor rxvtbgcolor: Set terminal default forground or background color\n");
printf("	altscreen normscreen: switch to 'alternative screen buffer (like vim) and back to normal screen buffer\n");
printf("	echo <string>: Print string (this string cannot be crayonized in this version)\n");
printf("	setenv <name>: Set environment variable\n");
printf("	send <string>: Send string to program whose output we're crayonizing\n");
printf("	clearscreen: clear terminal screen\n");
printf("	cls: clear terminal screen\n");
printf("	playsound <wav file path>: play a .wav file\n");
printf("	passto <program>: Pass output of the program we're crayonizing to another program, until that program exits (can be rz, for instance)\n");
printf("	Foreground/Background colors can be represented as <forecolor>/<backcolor> e.g. yellow/red\n\n");

printf("'value' lines have a special form, like this:\n\n");
printf("	value ' \\D*%% packet loss' =0 green >0 yellow >10 red >30 red bold\n\n");
printf("This extracts a numeric value from a string, and then colors it according to which operators it matches (operators: <,>,=,!)\n");
printf("'mapto' lines $also have a special form. The matched string is summed to a numeric value, and then one of the following attribute combinations is selected and used. This allows you to, say, generate colors for hostnames in tcpdump. e.g.\n\n");
printf("	mapto \"\\-X IP\\+X *:\" red green yellow magenta cyan white black/red black/white \"blue bold\"\n\n");

printf("	Note that in mapto 'blue bold' has to be placed in quotes, as otherwise they will be seen as two seperate mapto choices, rather than two attributes that together form once choice.\n\n");
printf("'linemapto' matches a substring, chooses a crayonization for it, and then applies that to the whole string.\n\n");
printf("'append' and 'prepend' add text either before or after the output. The text is specified where the 'match' string would normally be, and can be crayonized with colors etc.\n\n");
printf("'cmdline' matches from the commandline, rather than from program output. This is normally used to set environment variables that can then be used in append/prepend.\n\n");
printf("One other type of entry line exists, 'args'. This allows you to insert arguments into the command-line to be run. This exists solely to turn off annoying gcc warnings by adding '-Wno-write-strings' to its command line. It has the form\n\n");
printf("	args -Wno-write-strings\n\n");
}


void PrintUsage()
{
			printf("crayonizer (Crayonizer) %s\n\n",Version);
			printf("USAGE:\n");
			printf("In normal usage crayonizer is put in a directory that preceedes all others in a users PATH.\n");
			printf("A symbolic link is made to crayonizer under the name of the command that's going to be crayonized and an entry for that command is put in th config file (either /etc/crayonizer.conf for all users, or ~/.crayonizer.conf per user). The config file MUST also contain an entry specifying the 'Crayonizer directory', so that crayonizer can construct a PATH that prevents it from running itself.\n\n");
			printf("EXAMPLE: (crayonize make)\n");
			printf("mkdir /usr/prebin\n");
			printf("cp crayonizer /usr/prebin\n");
			printf("ln -s /usr/prebin/crayonizer /usr/prebin/make\n");
			printf("PATH=/usr/prebin:/usr/local/bin:/usr/bin:/bin\n\n");
			printf("******** edit /etc/crazyonizer.conf *********\n");
			printf("CrayonizerDir /usr/prebin\n");
			printf("entry make\n");
			printf("{\n");
			printf("line ' version ' bold magenta\n");
			printf("line ' error: *' red underline\n");
			printf("line ' undefined reference to ' red\n");
			printf("line ' warning: ' yellow\n");
			printf("line ' note: ' cyan\n");
			printf("string '^*:' bold\n");
			printf("string '\\-XTarget: *\\+X*\\-X\\S' bold inverse yellow\n");
			printf("}\n");
			printf("*********************************************\n\n");
			printf("The output of make should now be crayonized. If you want to crayonize gcc and g++ when they are run directly (not out of make) you can apply these rules to all three programs by using the 'OR' operator in the 'entry' line. Like this: make|gcc|g++\n\n");
			printf("When not used to crayonize another command, and just called as itself, crayonizer accepts the following args\n\n");
			printf("	-stdin: <entry name>	Read lines from stdin and crayonize them using the named entry in the config file\n");
			printf("	-v:		print version\n");
			printf("	-version:	print version\n");
			printf("	--version:	print version\n");
			printf("	-?:		print this help\n");
			printf("	-h:		print this help\n");
			printf("	-help:		print this help\n");
			printf("	--help:		print this help\n");
			printf("	-config-help:	print config file help\n");
			printf("	-pmatch-help:	print 'pmatch' pattern match help\n\n");
			printf("Crayonizer is copyright 2.33 Colum Paget\n");
			printf("It is released under the GNU Public Licence\n"); 
			printf("Bug reports to: colums.projects@gmail.com\n");
}

