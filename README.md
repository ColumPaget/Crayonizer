
SYNOSIS
=======

Crayonizer is an application that modifies the output of other text programs. It can color in text, trigger events on certain text matches, handle or translate keypresses, rewrite the command-line, alter the xterm title or add info-bars or popup menu bars. Uses include:

* Coloring output of programs like make/gcc, ping, ifconfig, netstat, nmap, tcpdump, etc to make them more readable, informative, or just prettier.
* Adding z-modem support to ssh.
* Adding '-p <port>' and '-i <identity file>' options to sftp by rewriting the command-line before passing it to the program.
* Setting xterm title to hostname upon ssh-ing to a host
* Setting terminal background color upon to hostname upon ssh-ing to a particular host
* Extracting 'now playing' information from mpg123 or mplayer
* Adding 'typing history' bars to programs that lack this feature


VERSION
========
2.4 


AUTHOR
=======

Crayonizer is (C) 2013 Colum Paget, libUseful is (C) 2009 Colum Paget. They are released under the GPL so you may do anything with them that the GPL allows.


CONTACT
========

Email: colums.projects@gmail.com


DISCLAIMER
===========

This is free software. It comes with no guarentees and I take no responsiblity if it makes your computer explode, opens a portal to the demon dimensions, or does anything at all.


INSTALL
========

SHOULD be as simple as './configure ; make; make install'. There are no library files, only a single 'crayonizer' executable, so you can copy that by hand to wherever you wish it to live. The default install will put it in /usr/local/bin unless a different prefix is configured with 'configure --prefix=<prefix>'. The example config files in 'examples' will be copied to '/etc/crayonizer.d'/ and symbolic links will be set up in '/usr/local/prebin' so that just inserting that directory as the first in your path should start crayonizing certain programs. Apart from the programs set up by symbolic link there's a 'terminal.conf' config intended for use with terminals like xterm, aterm and rxvt. This can be used like so:

```
		xterm -e "crayonizer terminal"
```

and it will do things like add popup menus to xterm, but this will require configuring xterm to use 7-bit input so that it sends 'alt-' escape strings. This can be achieved by adding:

```
xTerm*eightBitInput: false
XTerm.vt100.metaSendsEscape: true
```

to your .Xresources.

USAGE
======

```
	crayonizer -?             print help
	crayonizer -h             print help
	crayonizer -help          print help
	crayonizer --help         print help
	crayonizer -pmatch-help   print help for 'pmatch' pattern matches
	crayonizer -config-help   print help for config file
	crayonizer -stdin <entry name>  Read from stdin and crayonize it against the named entry in the config file.
	crayonizer <program path>	Run 'program' and crayonize it's output using matching entry in config file.
```

Normally, however, crayonizer will not be invoked under its own name, but invoked under the name of the program that it will run and crayonize.

Crayonizer has a lot of possible settings. The source distrbution comes with example config files in the 'examples' directory.


HOW IT WORKS
=============

Crayonizer reads instructions from /etc/crayonizer.conf or ~.crayonizer.conf, or /etc/crayonizer.d/<program name>.conf or ~/.crayonizer.d/<program name>.conf and uses them to color a program's output. You can run it against a program by:

```
		crayonizer <program path>
```

e.g.
 
```
		crayonizer /usr/sbin/tcpdump
```


However, crayonizer is intended to be installed into a directory that is at the 'front' of the user's PATH. Symbolic links are then made in that directory that have the same name as a command that one wishes to crayonize, but which point to the crayonizer executable. So, if we installed crayonizer to /usr/prebin, then we set our PATH to be:

```
	PATH=/usr/prebin:/usr/local/bin:/usr/bin:/bin
```

and then create symbolic links in prebin:

```
	ln -s crayonizer gcc
```

When the shell goes looking for a command, it will find it first in /usr/prebin (if we've made the appropriate symbolic link) and will run that. It will, in fact, be running crayonizer, as that's what the symbolic link really points to. Crayonizer will start up and read its config file (either ~/.crayonizer.conf or /etc/crayonizer.conf) and will find a 'CrayonizerDir' entry in there. It will reset the PATH environment variable to NOT INCLUDE the CrayonizerDir (/usr/prebin) in this example. Thus, in this example, the new PATH will be:

```
	/usr/local/bin:/usr/bin:/bin
```

Crayonizer now shells the original command, but this time as the crayonizer install directory is not included in the PATH, the 'real' executable will be found and run. Crayonize will read the output of this command, and 'crayonize' it, applying colors and other modifications. The details of which 'crayonizations' to apply to which text are provided in the 'entry' sections of the config file.


CONFIG FILE
============

Crayonizer uses a config file to configure its behavior. It will look for config files in the following order (where '~' means 'user home directory'):

```
	~/.crayonizer.d/<program name>.conf
	~/.crayonizer.conf
	/etc/crayonizer.d/<program name>.conf
	/etc/crayonizer.conf
```

Note the leading '.' for entries in the user home directory, this is so they are 'hidden' in a normal 'ls' listing of the directory.

The crayonizer config file MUST contain a 'CrayonizerDir' entry so crayonizer knows where it lives so that it can avoid starting itself. If crayonizer spawns itself, the results will be really bad, because the new crayonize process will also spawn itself, as will the next, and the next, and the next (it's the programmatic equivalent from that scene from Disney's Fantasia where Mickey creates all the golem brooms). This is called a 'forkbomb' and it can take down some systems by filling up their processs tables. Since version 1.0 crayonizer tries to prevent this by setting an environment variable that tells child processes that there's already a crayonizer running, so don't start up any more. However, in some situations, like when crayonizer is used to crayonize a terminal, this feature is explicitly switched off, in which case the 'CrayonizerDir' environment variable is all that stands in the way of a forkbombing.

The config file also contains 'entry' sections for each program that you want to crayonize. These contain either settings related to running/crayonizing the program, or 'crayonization' lines of the form '<action type> <pattern match> <crayonizations> 


SETTINGS
=========

These change the behavior of crayonizer's interaction with the program that it's crayonizing. 

```
		command <program path>
```

Specifies a command to be run for this crayonization entry. Normally this is not needed, as crayonizer can figure out what command is being run by looking at its command-line arguments. However, with programs like terminal emulators we use an invocation of the form:

```
		xterm -e "crayonizer terminal"
```

and crayonizer is no longer run under a target name using symbolic links. In this situation the config must tell it what command to run and crayonize. See the 'TERMINAL EMULATORS' section below for more on this.

```
		passinput
```

Send keyboard input to the program we're crayonizing. If you don't add this then anything you type will be picked up by the shell when crayonizer stops running. So, you can type 'make' then type your next command, and have that run when make is finished. If you do add this, then everything you type will be sent to the program being crayonized (generally important if that program is, for instance, ssh).

```
		cmdline-sub <match> <substitution>
		cmdline-insert <match> <substitution>
		cmdline-append <match> <substitution>
```

Substitute patterns	on the command-line with other text. See 'COMMAND LINE REWRITING' below.


```
		stripansi
```

Strip any ANSI codes from the program output before crayonizing. This setting does allow 'clearscreen' or 'movecursor' codes through, but will strip out ANSI that changes colors or attributes that might interfere with crayonizing.

```
		stripxtitle
```

Strip any attempts to set the xterm title bar. The text that was intended for the title is stored in variable crayon_xtitle. See discussion of xterm features below.

```
		expectlines
```

Expect the program output to be made up of lines terminated by newline. This is particularly important if usig the 'section' match, which needs to have the entirity of a line read in before being applied. However, this setting can cause problems when used with programs that, for instance, output a prompt that isn't terminated by a newline, and then wait for user input.
		
```
		AllowChildCrayon
```

Allow child processes to run crayonizers. By default a crayonizer process sets an environment variable to prevent running another copy of itself. This is to prevent cyclic loops of crayonizers spawning crayonizers. However, if, say, one is using crayonizer to add features to an xterm, then we'd want to be able to run crayonized subprobrams in the xterm. We'd still want any programs run in the xterm to be crayonized if they're been configured to be so. The AllowChildCrayon disables setting the environment variable, thus allowing crayonizer sub-processes to be run.



COMMAND LINE REWRITING
======================

Crayonizer can rewrite the command-line that is to be passed to the crayonzied program. Three commands are available.

```
		cmdline-insert <pattern> <substitution>
		cmdline-append <pattern> <substitution>
		cmdline-sub <pattern> <substitution>
```

`cmdline-sub` pattern-matches some text on the command-line and replaces it with 'substitution'. `cmdline-insert` does the same, but it replaces the text with a space, and then inserts 'substitution' into the command-line just after the program name. `cmdline-append` places the substitution at the end of the command-line, again leaving a space where the matched text was.

`cmdline-insert` and `cmdline-append` can be used with a blank pattern, in which case they will insert or append the substitution without needing to match a string in the command-line. e.g.

```
		cmdline-insert "" " -g "
```

`cmdline-insert` and `cmdline-append` leave a space in place of the matched text to preserve the order of command-line arguments, and prevent arguments being 'squashed together'. `cmdline-sub` however allows you to squash two arguments together. To see why this is useful let's consider adding command-line options to the 'sftp' program.

'sftp' lacks the '-p' option for setting the port that people know from 'ssh', instead requiring the user to type '-oPort='. This is annoying. We can solve this with `cmdline-sub` like so:

```
		cmdline-sub ' -p ' ' -oPort='

```

This will change the command-line

```
		sftp user@myhost.com -p 1022
```	

to


```
		sftp user@myhost.com -oPort=1022
```

Unfortunately that's not the end of the matter. 'sftp' insists that options must come before the destination argument, so we need to use the `cmdline-insert` to move the option. This can be achieved using the $(match) variable, which converts to the value of the matched string. Thus:

```
		cmdline-insert " -p \D+ " "$(match)"
		cmdline-sub " -p " "-oPort="
```

So, for a command-line like:

```
		sftp user@myhost.com -p 1022 -oIdentityFile=~/.ssh/id_rsa
```

`cmdline-insert` will rewrite it to 

```
		sftp -p 1022 user@myhost.com -oIdentityFile=~/.ssh/id_rsa
```

Leaving a space to ensure that '-oIdentityFile=~/.ssh/id_rsa' isn't joined to 'user@myhost.com' to make a single meaningless argument. Having moved the argument we now replace it with the 'cmdline-sub' argument, which rewrites the command-line to:

```
		sftp -oPort=1022 user@myhost.com -oIdentityFile=~/.ssh/id_rsa
```

Which finally gives us a command-line that sftp will accept. The '-oIdentityFile=' argument could be replaced by a '-i' argument in a simliar way.




CRAYONIZATION LINES
====================

Crayonizations are settings that match the output of the target program, and take some action, for instance coloring in the matched string. 

e.g. 

```
		entry gcc
		{
		line error: red
		line warning: yellow
		}
```

If crayonizer finds itself run under the name 'gcc' (via symbolic link) then it will apply these crayonizations. If we want to apply the same commands to, say, g++, then rather than write a new entry for g++ we can use the '|' operator, thusly.

```
		entry gcc|g++
		{
		line error: red
		line warning: yellow
		}
```

In some situations you only want to crayonize output from a command if it has a certain argument. Thus to crayonize the 'cat' command if it has '/proc/cpuinfo' as an argument:

```
		entry cat /proc/cpuinfo
		{
		line bogomips magenta
		line 'cpu MHz' green
		line 'model name' yellow bold
		string Intel bold
		}
```

If you want to crayonize cat, tail, and head of /proc/cpuinfo, try:

```
		entry cat|tail|head /proc/cpuinfo
		{
		line bogomips magenta
		line 'cpu MHz' green
		line 'model name' yellow bold
		string Intel bold
		}
```

'line' entries apply the crayonizations to an entire line of text. If you only want to highlight a given word, then use 'string' matches. e.g.

```
		entry gcc|g++
		{
		string error: red
		string warning: yellow
		}
```

This entry will only color the words 'error:' and 'warning:', whereas the previous entry would color the entire lines those words occurred in.

If you've got any whitespace in the pattern match, then you'll need to use quotes, thusly:

```
		string "color this in red" red
```

If you want to only color a string if it has a certain numeric value, then use the 'value' match

```
		value " \D*%" >0 green
		value " \D*%" >30 yellow
		value " \D*%" >80 red
```

This will match anything that starts with a digit (\D) and ends with a '%' and color it appropriately if it's >  a value. Available comparison operators are '<', '>', '==' and '!=' (the latter two can be shortened to '=' and '!').

If you don't want the '%' crayonized in this case, you'd use the 'Text Extraction' on/off switch to exclude it. Thusly:

```
		value " \D*\-X%" >0 green
```

Multiple 'Value' matches can be combined in one line, thusly

```
		value " \D*%" >0 green >30 yellow >50 yellow bold >50 red >70 red inverse
```


Sometimes you will want to color some output with randomly selected attributes in order to distinguish between different values of the output. For this purpose the 'mapto' match is provided. This will sum the characters of the matched string into a value, and then use that value to select from a list of attributes. For example:

```
	 mapto "\-X IP\+X *:" red green yellow magenta cyan white black/red black/white "blue bold" "red inverse" 
```

will pick out the part of tcpdump output that identifies the two hosts that are communicating. It will convert the hostnames to a value by summing all their characters, and then use this value to select one of the subsequent list of attributes. Note that, for attributes with more than one argument, quotes are needed to distingish between two choices, and two attributes that are part of the same choice.

If you changed this 'mapto' to a 'linemapto', like this

```
	 linemapto "\-X IP\+X *:" red green yellow magenta cyan white black/red black/white "blue bold" "red inverse" 
```

Then the entire line will be colored, not just the matching string.

You can apply matches to a line or set of lines with 'lineno' e.g.

```
		lineno =5 red

```

would color line 5 in red

```
		lineno <6
		{
			string "^\A*:" bold cyan
			string "\D\D:\D\D:\D\D\-X up" white/blue
		}
```

would apply matches to only the first 6 lines

If you want to apply matches to a sub-section of a line, then:

```
	  section 15-20
  	{
  		value " \D* " <20 red =20 blue >20 green
  		string RT red
  	}
```

this will only apply the matches to a substring on the line in the range 15-20 characters. If using the 'section' match it's normally a good idea to use the 'expectlines' setting, to ensure that a full line has been read before applying matches.
	
'append' and 'prepend' add lines before and after the program is run. So:

```
		entry tar
		{
			append "Should you have used --dereference?" red bold
		}
```

prints a line at the end of every 'tar' command, reminding me to use --dereference when building tarballs

'cmdline' is used to match text in the command line. This is normally used to set environment variables in 'append' and 'prepend'.


KEYPRESSES
==========

Crayonizer can detect keypresses and trigger an action off them. For this use the 'keypress' statement. For example:

```
		keypress alt-i iconify
```

Often you'll want to use this to send a string to the application being crayonized:

```
		keypress alt-m send "make clean; make\n"	
```

Crayonizer recognizes the following keys:
	
		escape home insert delete end up down left right pgup pgdn pause backspace newline return F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15 menu F17 F18 F19 F20

these can be combined with the modifiers 'shift-' 'alt-' and 'ctrl-' e.g.	

```
			keypress menu call PopupMenu
			keypress shift-up maximize
			keypress shift-down demaximize
			keypress ctrl-menu send "pause\n"
```

it's 'menu' instead of 'F15' because on my keyboard the 'menu' button sends the F15 escape sequence

standard keys that send alphabetic characters can also be used, allowing one to specify 

```
			keypress alt-m call PopupMenu
			keypress ctrl-q send 'quit\n'
```


IF STATEMENTS
==============
	
Crayonizer provides a crude form of 'if' statement. It has the form:

```
		if <condition> <crayonizations>
```

e.g.

```
		if exists(/dev/dsp) setenv DEV=/dev/dsp
```

The types of condition available are:

```
		exists(path)					Check if file/directory exists
		isatty(stdin|stdout)	if stdin or stdout is a tty
		notatty(stdin|stdout)	if stdin or stdout is NOT a tty
		arg(arg)              Check for a command-line argument (1 character arguments)
		larg(arg)             Check for a command-line argument (long arguments)
		focus									Used with xterm, true if xterm has keyboard focus
		unfocus								Used with xterm, true if xterm DOES NOT have keyboard focus
```

and also

```
		"<value> = <value>"
		"<value> ! <value>"
		"<value> > <value>"
		"<value> < <value>"
```

Where '<value>' is either an environment variable in the form $(varname) or else is a string constant. The ! and = operators treat the values as strings, but the '<' and '>' values treat them as integers.


CRAYONIZER ACTIONS
===================

The currently recognized actions for crayonizer are:

		black white red green blue magenta cyan yellow darkgrey lightred lightgreen lightyellow lightblue lightmagenta lightcyan bold inverse uppercase lowercase blink caps underline hide basename replace setenv passto send echo send clrtoeol cleartoeol cls clearscreen altscreen normscreen dontcrayon setxtitle restorextitle raise lower iconify deiconify maximize demaximize wide high font fontup fontdown fgcolor bgcolor infobar querybar selectbar historybar call exec bell xselection

'caps' is a shorthand for 'uppercase'. 

Foreground/Background color combinations can be set by use of a '/', thusly:

```
		white/blue
```

'light' and 'dark' colors only work in terminals that support xterm 16-bit colors

'setenv' sets an environment variable, whose name is specified by the next argument. Currently environment variables can only be used in 'append', 'prepend', 'echo', 'send' and 'passto' commands. e.g.

```
		entry ssh
		{
			cmdline "\-X@\+X*" setenv SSH_DEST
			prepend "SSH $(SSH_DEST)" setxtitle hide
		}
```

'hide' suppresses text output. This currently only works on entire lines, but will probably be able to work on substrings in future.

'basename' changes the text extraction to the last part of a file path. So, if you match '/home/mydir/myfile.txt' the match will change to 'myfile.txt' and only that bit will be crayonized. 'basename' is positional, so:

```
		line "Playing *" bold cyan basename red
```

Will match any line with 'Playing' in it, say 'Playing /home/mydir/music.mp3', and color it in cyan, but then apply 'basename' matching just 'music.mp3' and color that in red.

'replace' utterly replaces the text extraction with something else. You'd use this when you want to trigger off a particular piece of text, but then update a titlebar or something with a completely different text string.

```
		string "done" replace "Processing complete" echo setxtitle
```

'echo' will simply print something out. e.g.

```
		string "foo" hide echo "bar"
```

will detect 'foo', hide it, and output 'bar' in its place. this differs from 'replace' because the text extraction isn't changed, so subsequent commands will see 'foo' not 'bar'

'send' will send a string to the application being crayonized. e.g.

```
		keypress ctrl-p send "pause\n"
```

'passto' will run a program, and pass the crayonized output to it. Thus:

```
		string "\*\*?B00000000000000\r" hide passto "/usr/bin/lrz -v " send "\r"
```

Will detect the zmodem attention string, hide it, and pass all subsequent output to lrz, and when that's finished running, will send "\r" back to the source of the zmodem attention string. This can be used to add zmodem support to ssh.

'dontcrayon' will suppress crayonization of future lines

'altscreen' will switch to the alternative screen buffer in terminals that support this

'normscreen' will switch back to the standard screen buffer	
	
'cleartoeol' and 'clrtoeol' will clear to the end of a crayonized line. This is normally used with lines that have a colored background, and where it is desired for this background to traverse the width of the page. Without clearing to the end of the line the colored background would effect only the printed text.

'cls' and 'clearscreen' will clear the screen

'bell' will send the bell character. The results will depend on how your terminal emulator is set up.

'call' will call a function. See 'functions' below.

'infobar' 'selectbar' 'querybar' and 'historybar' create single-line user interaction bars. See 'statusbars' below.

The remaining commands relate to xterm compliant terminal emulators

'setxtitle' sets the title-bar of terminals to the 'matched' text. 
'restorextitle' restores title-bar to its value before crayonizer ran.
'xselection' sets the current primary/clipboard selection. To work this may need a permission setting in your terminal emulator.
'raise' raise window to the top of the stack
'lower' lower window to the bottom of the stack
'iconify' minimize window to an icon
'deiconify' restore window from iconized state
'maximize' expand window to maximum size
'demaximize' return window to normal size
'wide' expand window to maximum width
'high' expand window to maximum height
'font <font name>' switch to named font
'fontup' switch to next font in xterms internal font list
'fontdown' switch to previous font in xterms internal font list
'bgcolor <color name>' set terminal background color
'fgcolor <color name>' set terminal foreground color



PMATCH PATTERN MATCHES
=======================

Crayonizer uses it's own 'pmatch' system for pattern matches (I couldn't get my head around regular expressions). Pmatch is based on the 'fnmatch' and 'glob' style of wildcarding used by the unix shell, but with many more features. This system is 'non greedy', so it matches the shortest string that it can, not the longest like POSIX regular expressions. Pmatch recognizes the following tokens:

	?:    Match any single character
	*:    Match any substring
	+:    Match zero or more of the previous match
	[]:   Match list of characters. e.g. [123456789]
	^:    Match start of line
	$:    Match end of line (this can be '\n' or end-of-input)
	\:    Quote a character, so it's not interpreted, unless it's one of the following interpretations.
	\xFF: Match a character by hex value, where 'FF' is the hex value. e.g. \x20 is 'space'
	\000: Match a character by octal value, where '000' is the octal value. e.g. \040 is 'space'
	\+S:   Turn pmatch switch on, where 'S' is the switch character.  Available switches: 'C' (case sensitivity) 'X' (text extraction) 'W' (wildcards) 'O' (overlap) 
	\-S:   Turn pmatch switch off, where 'S' is the switch character.  Available switches: 'C' (case sensitivity) 'X' (text extraction) 'W' (wildcards) 'O' (overlap) 
	\a:   Match 'bell' or 'alert' character (this is used a lot in xterm/vt220 escape sequences)
	\b:   backspace
	\d:   'delete' character (ascii 127)
	\e:   escape
	\l:   Any lowercase alphabectic character
	\n:   newline
	\r:   carriage return
	\t:   tab
	\A:   Any alphabetic charcter
	\B:   Any alpha-numeric character
	\C:   Any printable character (as decided by the 'isprint' C library function)
	\D:   Any decimal digit
	\S:   Any whitespace character
	\T:   'Text', any non-whitespace character
	\P:   Any punctuation character
	\X:   Any hexadecimal digit
	\U:   Any uppercase alphabetic character
	\b:   
	\n:   Match a newline
	\0:   Match end-of-input

PMatch Switches:

The \+ and \- operators can be used to turn on and off certain features of the pmatch system. 

	\+C:  turn on case sensitivity in matches  (default)
	\-C:  turn off case sensitivity in matches
	\+W:  turn on wildcards (default)
	\-W:  turn off wildcards (so '*' and '?' are no longer wildcards)
	\+O:  allow overlapping matches (default)
	\-O:  do not allow overlapping matches, so '\D+\S' will only return one match for '1234 '
	\+X:  allow text extraction from this point (default)
	\-X:  turn off text extraction from this point

The \+X and \-X switches turn 'text extraction' on and off. This allows you to match a string, but only crayonize a substring of the matched string. For example:

  \-X Match this whole string but \+X Crayonize only this bit \-X but not this bit

the \-W switch turns off wildcarding, so only switches are honored. This turns the pmatch into a straight strcmp until turned back on with \+W

TEXT SUBSTITUTIONS
==================

actions like 'echo' and 'replace' take a text argument. This argument can accept a number of substitutions.

	$(name)		a string like this will be replaced by the environment variable 'name'.

	%% 		This is replaced with '%'
	%h		This is replaced with the current host name
	%t		Time in hour:minute format
	%T		Time in hour:minute:seconds format
	%D		Date in year/month/day format
	%H		Hour
	%M		Minute
	%S		Seconds
	%a		Day name
	%b		Month name
	%d		day number
	%m		month number
	%Y		4-digit year
	%y		2-digit year
	%c		localhost cpu usage 
	%L		localhost system load
	%f		localhost memory usage
	%F		localhost disk usage
	
VARIABLES
=========

Crayonizer stores some information in environment variables, which can then be used in text substitutions using the '$(name)' method.

crayon_xtitle		this variable holds any text that's been captured by the 'stripxtitle' action. This is text that would normally go in the titlebar of your terminal emulator, but which you've captured with crayonizer instead. You can use 'setxtitle' to whatever you want the titlebar to be, and by using the 'crayon_xtitle' variable you can include the captured text string in your title. You can set the 'crayon_xtitle' variable yourself using 'setenv' and then other programs will overwrite it.

crayon_old_xtitle		this variable holds the xtitle of the window at program startup. If the 'restorextitle' action is called, then this variable supplies the value that the terminal title bar reverts to. If you use 'setenv' to change this variable, then you change what the titlebar reverts to. Not all terminal emulators support querying the titlebar, so it may not always be possible to restore from this value, which is why the next variable is supplied

crayon_default_xtitle		this variable holds a fall-back xtitle that is displayed if ever the titlebar gets set to blank. You can set this in your config to ensure the terminal title bar is never blank.

crayon_duration		this variable is updated when a timer is called (see 'EVENTS' below), and at program end, and can be used in 'append' config entries. It stores the amount of time that the crayonizer process was running for, in seconds. For example:

```
		append "Processing took $(crayon_duration) seconds"
```

or, if you want to run a notification for long-running processes

```
		if "$(crayon_duration) > 60" playsound /usr/share/sounds/finished.wav
```

would play a sound when the process exited *if the process took longer than a minute*


FUNCTIONS
=========

Crayonizer allows the defining of functions, like so:

```
		function ontimer
		{
			if "focused" replace "$(crayon_xtitle)   cpu: %L%% mem: %f%% /: %F%%      %H:%M: %a %D" setxtitle
		}

		function GoLarge
		{
			bell
			maximize
			font "dejavu mono:size=20"
		}

		keypress alt-up call GoLarge
		timer 5 call ontimer
```


STATUSBARS
==========

Crayonizer allows you to create one-line status-bars at the bottom of the screen. There are three types:

```
		infobar <actions> <text>                          displays a line of text
		querybar <actions> <text> <varname>               displays a prompt, accepts input, stores it in the named variable
		selectbar <actions> <choices> <selection name>    displays a list of options, calls a selection handler
		historybar <actions>                              displays a list of previously typed lines
```

you might create a bar like this permanently for a program

```
		onstart "" infobar yellow/blue "KEYS: q=quit s=save e=edit <=prev >=next"
```

or have one popup on a keypress

```
		keypress alt-m querybar "ENTER FILENAME: " SendFile
```

statusbars behave differently than other command-lines if combined. Most command-lines will be executed one after the other, but statusbars will instead by cycled through on each trigger. Consider this:

```
		keypress menu
		{
			selectbar yellow/blue "tiny ansi boxy fkp small norm big huge" font-size
			selectbar yellow/red "raise lower iconify maximize demaximize wide high" xterm-actions
			selectbar black/cyan "blue red green purple cyan" bgcolor
			querybar yellow/blue "Value:" QueryFunc
		}
```

in this setup each subsquent keypress will display the next bar, allowing the user to cycle through them.

selectbars do not call functions. Instead one supplies a 'selection list', which is a list of potential matches for the selected item. The below example shows the selection list 'font-size'

```
		selection font-size
		{
			boxy font "edges"
			ansi font "smoothansi"
			fkp font "fkp"
			tiny font "droid sans mono:size=7"
			small font "droid sans mono:size=9"
			norm font "dejavu sans mono"
			big font "dejavu sans mono:size=16"
			huge font "dejavu mono:size=20"
		}
	
		keypress alt-f selectbar yellow/blue "tiny ansi boxy fkp small norm big huge" font-size
```

When the user selects from the list of choices, the selection list is run and the matching line executed.

Historybars allow the user to pull up, edit and then enter any of the last ten things typed. They call a function, which usually means they are used like this

```
	function sendhist
	{
		send
	}


	entry sftp
	{
	passinput
	keypress alt-h historybar white/blue "" sendhist
	}
``` 




EVENTS
======

Crayonizer has a few event functions that get called.

'onstart' allows you to perform some actions at startup *without appending or prepending any lines*. onstart expects an argument that supplies an initial string (as no strings have yet been captured from the crayonized program)  e.g.

```
		onstart "$(USER)@$(HOSTNAME)" setxtitle setenv DEFAULT_XTITLE setenv XTITLE fgcolor #FAFAFA bgcolor #300030
```

	or

```
		onstart "$(USER)@$(HOSTNAME)" 
		{
			setxtitle setenv DEFAULT_XTITLE 
			setenv crayon_xtitle 
			fgcolor #FAFAFA
			bgcolor #300030
		}
```

'onexit' is called on program exit

'timer' will trigger an event every so many seconds. e.g.

```
		timer 5 echo "hello"
```

you would probably use 'timer' with a function call like so:

```
		function EveryTenSecs
		{
			replace "Running for $(crayon_duration) seconds" setxtitle
		}

		timer 10 call EveryTenSecs
```


TERMINAL EMULATORS
==================

Terminal emulators are a special case where we want to capture keypresses from the emulator and inject escape sequences into it. This is done by running crayonizer as the 'shell' for the emulator. A minimal config entry for a terminal emulator would look like

```
	CrayonizerDir /usr/prebin
	entry terminal
	{
		AllowChildCrayon
		passinput
		command /bin/bash

		keypress alt-m send "make\n"
		keypress alt-r send "reset\n"
	}
```

We 'passinput' because obviously we want to type into our terminal emulator an have it go to the shell. The 'command' config line specifies what shell to run. 'AllowChildCrayon' allows processes run within that shell to be crayonized by their own crayonizer, which makes the 'CrayonizerDir' setting all the more important, because it's the only thing now stopping crayonizers spawning other crayonizers incessantly. Finally, this terminal config has a couple of keybindings for often used commands.

To use our terminal config with xterm we would either store it in a crayonizer.conf config file, or in /etc/crayonizer.d/terminal.conf. We'd then run the terminal like so:

```
		xterm -e "crayonizer terminal"
```



