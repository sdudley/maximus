%
%
%
%
%
%             Maximus Version 3.00 - System Control File
%
%
%
%  This plain ASCII file is used as input for the SILT compiler (which
%  turns it into files needed for Maximus to operate).  To edit this
%  file, you should use either a straight ASCII editor, or a word
%  processor that saves files in "plain text" format.  Comments can be
%  placed in this control file by placing a percent sign ('%') or
%  other punctuation symbol as the first non-space character on a
%  line.  Everything after the punctuation symbol will be considered
%  a comment and will not be processed.
%
%
%
% Copyright 1990, 1995 by Lanius Corporation.  All rights reserved.
%    Maximus and Squish are trademarks of Lanius Corporation.
%
%
%
%
% The SYSTEM Section:
%
%    Everything inside this control file is categorized into several
%    different sections, to make the control file easier to deal with.
%    A section is started by entering the name of the section,
%    followed by the literal 'Section'.  The first section in this
%    control file is the system section, which tells Maximus about
%    your system, the name of your BBS, and general information that
%    belongs nowhere else in the control file.

System Section

        % The 'Name' command tells Maximus about the name of your BBS.
        % This is used as a default for your EchoMail origin lines,
        % unless you are using a custom origin line in one or more
        % areas.  Don't insert your system address at the end of this
        % line, as Maximus will add it for you automatically.

        Name            New Maximus System

        % The SysOp option tells Maximus about the name of the sysop.
        % However, just setting this to a user's name does not grant
        % that user any special privileges.  This keyword is only used
        % for display purposes and for the "To:" field when leaving
        % log-off comments.
        %
        % To grant a user real sysop powers, the user editor must be
        % used to adjust the user's privilege level to SYSOP.

        SysOp           New System Operator

        % 'Snoop' tells Maximus whether or not to have 'Snooping' by
        % enabled default.  When you are snooping, the screen displayed
        % on the local display is the same as the user's screen.
        %
        % If snoop is OFF, a status line display will be shown instead.

        Snoop

        % The 'Video' statement tells Maximus what type of video output
        % to use on your system.  This keyword is only supported
        % for DOS systems.
        %
        % If you are running on a 100% IBM compatible, 'Video IBM' will
        % always give the best performance.  Video IBM writes
        % directly to the screen buffer, although it is compatible
        % with DESQview shadow buffers.
        %
        % If you are using an old IBM CGA adapter, and if you see "snow"
        % when using 'Video IBM', you can 'Video IBM/snow' instead.
        % Although 'Video IBM/snow' is a bit slower than just 'Video
        % IBM', it eliminates all of the on-screen flickering.
        %
        % 'Video BIOS' instructs Maximus to write to the screen using the
        % standard BIOS routines.

        Video IBM
%       Video IBM/snow
%       Video BIOS

        % The 'Path' statements tell Maximus where to find various files
        % on your system.  The first path, the 'System' path, tells
        % Maximus where it can find its 'home base', where the executable
        % files are stored, and generally the directory which everything
        % else is based from.  Make sure to specify a complete path,
        % including a drive specifier and leading backslash.  By doing
        % so here, you can use relative pathnames throughout the rest
        % of this control file, which is what has been done for
        % this sample control file.

        Path System     C:\Max\

        % The 'Misc' directory is a directory for miscellaneous text files,
        % and other files which Maximus will display to the user at one
        % time or another.  It also contains the path to the Fxx.BBS
        % files, which will be displayed when the SysOp locally presses
        % a function key.

        Path Misc       Misc\

        % The following line defines the location of the language file
        % directory.  As a bare minimum, this directory must contain
        % an .LTF (Language Translation File) for each language file
        % you have declared in the language section.  The *.MAD, *.LTH
        % and *.H files are not required by Maximus.

        Path Language   Lang\

        % The 'Temp' path is simply that:  a path to a temporary directory.
        % This statement is required for uploads and other parts of system
        % operation.  WARNING:  Anything in this directory is 'fair
        % game' for Maximus to use, so chances are that if you put
        % something in this directory, it may not be there the next
        % time you look.  Make sure not to use this directory for
        % anything else.

        Path Temp       Temp\

        % This next line is provided only for compatibility with
        % the Opus V14 and V17 control-file formats.  Maximus itself
        % does not use this path.  Most users will never need this
        % option.

%       Path Outbound   \Binkley\Outbound\

        % The next keyword controls the location of Maximus'
        % inter-process communications directory.  This should normally
        % point to a RAM drive.  PLEASE SEE THE DOCUMENTATION BEFORE
        % ENABLING THIS KEYWORD!

        Path IPC        IPC

        % See the documentation for notes on implementing this keyword.
        %
        % WARNING!  IF THIS KEYWORD IS USED IMPROPERLY, YOUR HARD DISK
        % MAY BE DAMAGED!  PLEASE READ THE DOCUMENTATION BEFORE USING
        % THIS KEYWORD!
        %
        % IN ADDITION, SHARE MUST **ALWAYS** BE LOADED IF YOU WISH TO
        % USE SQUISH-FORMAT MESSAGE AREAS IN A NETWORK OR MULTITASKING
        % ENVIRONMENT.  CONCURRENTLY ACCESSING A SQUISH BASE WITHOUT
        % LOADING SHARE WILL CAUSE DAMAGE TO YOUR MESSAGE AREAS!

%       No Share.Exe

        % The 'File Password' statement tells Maximus where to store your
        % user file, which is a list of all users on your system, their
        % passwords, user information, and so forth.

        File Password   User

        % The 'No Password Encryption' statement tells Maximus to disable
        % the automatic password encryption feature.  Normally, Maximus
        % will encrypt a user's password as soon as the user logs on
        % and store the password in the encrypted format.
        %
        % However, if you wish to disable this automatic password
        % encryption, you may enable this keyword.  Maximus cannot
        % decrypt previously-encrypted passwords, but it can still
        % read the old encrypted passwords if this option is enabled.

%       No Password Encryption

        % The 'File Access' statement tells Maximus where to store the
        % priv levels database; privilege levels are defined individually
        % in the Access Section, and describe attributes of the various
        % classes of user on your system

        File Access     Access

        % The 'File Callers' statement defines a file to use for a call
        % overview log where Maximus saves information about each session.
        % Other than recording this informaiton, Maximus does not currently
        % use this file - it is created for the convenience of third party
        % utilities. This file is OPTIONAL and should be left commented out
        % if you don't need or want this log.
        % Maximus will automatically use '.bbs' as the file extension.

        File Callers    Callers

        % The 'Log File' is a file that Maximus writes to tell you, the
        % SysOp, what users have been doing on your system.  Maximus will
        % record most notable system events, such as a user logging on,
        % messages being entered, and so on.
        %
        % However, different levels of logging can be specified, based
        % on the amount of detail desired.
        %
        % A log level of 'Terse' specifies that only the bare essentials
        % are to be logged.  'Verbose' gives you a bit more information,
        % and 'Trace' does what it says -- it traces almost every action
        % of your users.
        %
        % NOTE!  If you do *not* specify a log file, none will be produced!
        %
        % The -l command-line parameter can be used to override the
        % name of the log file at run-time.

        Log File        Max.Log

%       Log Mode        Terse
%       Log Mode        Verbose
        Log Mode        Trace

        % If you are running more than one copy of Maximus on-line from
        % the same physical hard disk, then you must specify a task
        % number. The task number will be included in the system log
        % file, and will be used to keep some separate information about
        % the two different nodes, and will make sure that none of the
        % files either copy of Maximus uses will clash.
        %
        % The -n command-line parameter can be used to override this
        % setting at run-time.

        Task    1

        % The following two keywords are only required for the OS/2
        % version of Maximus.
        %
        % 'MCP Pipe' tells Maximus where to find the MCP server pipe
        % for use in inter-process communications.
        %
        % 'MCP Sessions' tells Maximus the maximum number of on-line nodes
        % that you will use at one time.

        MCP Pipe        \pipe\maximus\mcp
        MCP Sessions    16

        % The following line specifies which MultiTasker (if any) you
        % are currently running.  By default, Maximus will attempt
        % to autodetect the current multitasker.  Unless you're
        % running some strange hardware, you can usually leave these
        % settings alone.

%       Multitasker     None
%       MultiTasker     DoubleDOS
%       MultiTasker     DESQview
%       MultiTasker     PC-MOS

        % The following statement instructs Maximus-DOS to swap
        % itself out of memory when running external programs.
        % Maximus will swap to XMS, EMS, or disk (in that
        % order).

        Swap

        % The next line is only needed for those systems which are
        % running external programs.  If you enable the next keyword,
        % Maximus will invoke a FOSSIL-based carrier-detect watchdog.
        % In plain terms, that means if you have this keyword
        % uncommented, and if a user drops carrier while running an
        % external program, Maximus will reboot your computer.
        % Presumably, you will have configured your system to load
        % up your BBS automatically when turned on, so this keyword
        % could save you from a few potential crashes.  However, if
        % you are only running bona-fide "Door" programs, which monitor
        % the carrier detect themselves, then you should leave this
        % keyword disabled.
        %
        % SPECIAL NOTE:  This function is controlled by your FOSSIL,
        % so it it may not work correctly on some older FOSSIL versions.
        %
        % Specifically, some early Opus!Comm versions were buggy,
        % and would not turn off the carrier detect after it had
        % been turned on, which resulted in Maximus rebooting (instead
        % of terminating gracefully) when a caller dropped carrier
        % inside of Maximus itself.
        %
        % WARNING!  This feature may cause undesired effects in a
        % multitasking or network environment.  If a user drops
        % carrier while in an outside program, the FOSSIL could
        % cause a reboot at any time.
        %
        % The Reboot keyword is for DOS systems only. 

%       Reboot

        % The following option dictates whether or not Maximus should
        % close all of its internal files upon going outside.  This
        % includes the log file, the user file, and a few other
        % miscellaneous files.  Usage of this keyword is mandatory if
        % you wish to allow external programs to add to the system
        % log.

        Dos Close Standard Files
End System Section

Include Access.Ctl

Equipment Section

        % The 'Output' keyword tells Maximus where to send its output,
        % whether that is the local monitor, or a COM port.
        % Specifying 'Local' for local output is generally
        % unnecessary, because the '-k' (keyboard) command-line parameter
        % does exactly the same thing.  Only COM ports from one through
        % eight are supported -- also note that your FOSSIL must support
        % the COM port you wish to use.  Please consult your FOSSIL
        % documentation if you're using a port other than COM1: or
        % COM2:.  By default, Maximus will use COM1: if no 'Output'
        % statements are given.
  
        Output  Com1
%       Output  Com2
%       Output  Com3
%       Output  Com4
%       Output  Com5
%       Output  Com6
%       Output  Com7
%       Output  Com8

%       Output  Local

        % The following statement specifies the highest baud rate that
        % your system can support.

        Baud Maximum    19200

        % This is the command that Maximus will send to the modem when a
        % user logs off.  Everything in this string is sent as-is to
        % the modem, with the exception of the following special characters:
        %
        %   v : Sets DTR low
        %   ^ : Sets DTR high
        %   ~ : This pauses for one second
        %   ` : This pauses for 1/20th of a second
        %

        % The following default string is a bit lengthy, but it seems to
        % work for all of the modems it has been tried on.

        Busy  v~~^~~|~ATZ|~ATM0H1|

        % The following strings are only required for the internal 
        % Wait-For-Caller command.  These strings use the same command
        % chracters as the "Busy" string.

        % This string is sent to the modem when the Maximus WFC 
        % (Waiting For Caller) subsystem starts up.  This should
        % initialize your modem and set the appropriate defaults.
        %
        % If you are running a high-speed modem (9600 or above),
        % the following init statement should include the &B1
        % option (to specify a locked com port).

        % For 9600 bps (or faster) modems:

        Init    ~v~````|~^``AT&B1H0S0=0|

        % For 2400 bps (or slower) modems:
        %
        %Init    ~v~````|~^``ATH0S0=0|

        % For a modem-controlled answering sequence, use the following
        % string, and COMMENT OUT the 'Ring' and 'Answer' strings.  Only
        % use this if your modem won't properly answer the phone.

%       Init    ~v~````|~^``ATH0S0=1|

        % This string is what your modem sends to the computer when
        % your telephone rings.

        Ring    Ring

        % This string is sent to the modem in response to a ring.
        % Normally, this command should tell the modem to take the
        % phone off-hook and to start the answer sequence.
        %
        % WARNING!  This command may not work for semi-Hayes-
        % compatible modems.  If Max isn't answering the phone at
        % all (and you're seeing a bunch of "Ring" messages in
        % the status window), try commenting out this statement
        % and using the "Init" statement (above) with an "S0=1".

        Answer  ATA|

        % This string should be returned by your modem when a connection
        % is established.  The link rate (300, 1200, 2400, and so on)
        % should follow the string you specify.  The following will
        % work for most Hayes-compatible modems.

        Connect Connect

        % This is the carrier mask to use for your modem.  If you
        % don't know what this does, then please leave it alone.  If
        % you *do* need to change it, then please remember that the
        % value is in decimal, not hexadecimal.

        Mask Carrier    128

        % The following three lines specify what type of handshaking
        % to use with your modem.  If you want your users to be able
        % to use ^S and ^Q to pause and re-start their screen display,
        % then you must use the 'XON' keyword.

        % Also, if you are running a high-speed modem (9600 or above),
        % CTS handshaking should be enabled.
        %
        % NOTE:  These statements have no effect when running under
        % OS/2.  Maximus directly inherits the port characteristics
        % from the calling process.  See the installation instructions
        % for information on setting up the port using the 'MODE'
        % command.

        Mask Handshaking        XON
        Mask Handshaking        CTS
%       Mask Handshaking        DSR

        % Use this if your modem requires a BREAK to clear its internal
        % buffer.  This is only necessary with modems that *do* have
        % an internal buffer -- generally, modems which are 2400 baud
        % or lower do not have an internal buffer and do not need this
        % keyword.  If you are not sure if you have an internal
        % buffer or not, check your modem manual for details.
        %
        % !!!!!!!!!!! WARNING !!!!!!!!!!!!
        %
        % Do not enable this option unless you are POSITIVE that your
        % modem supports this feature.  At present, only the HST
        % supports this function, and even then, the HST must be
        % set to AT&Y0 for it to function correctly.  If your
        % HST is not configured for AT&Y0, or your modem does
        % not support a "destructive, nonexpedited" break,
        % do NOT enable this feature.

%       Send Break to Clear Buffer

        % The "No Critical Handler" keyword tells Max to turn off the
        % internal critical error handler.  Normally, Max will fail
        % all critical errors, and continue with the BBS's operation.
        % If this behaviour is not desired, the following keyword can
        % be used to turn off the critical error handler so that
        % an external handler can be installed.

%       No Critical Handler
End Equipment Section


Matrix and EchoMail Section

        % The following group of lines specify which network addresses
        % you wish to use.  If you're not a member of FidoNet or
        % a FidoNet Technology Network, you can ignore this section.  If
        % you are a member, read on.
        %
        % The following lines contain all of your matrix addresses, up
        % to a maximum of sixteen.  The first address specified will
        % be your PRIMARY address.  This address will identify your
        % network address on outgoing mail.  The following addresses
        % are secondary addresses and are not used for outgoing mail.
        %
        % SPECIAL NOTE FOR POINTS:  If you are using Maximus to run
        % your point, then you must follow some guidelines for the
        % Address statements.
        %
        % For 3D (pointnet) points:
        %
        %   For the *first* statement, put your full network address,
        %   INCLUDING the point number, as you wish it to appear in your
        %   EchoMail Origin Line.
        %
        %   For the *second* statement, use your fake network address,
        %   which will be given to you by your bossnode.  Example setup
        %   for an average 3D point:
        %
        %
        %       Address 1:249/106.2     % <-- Don't forget to include the
        %                               %     point number, or else this
        %                               %     feature will not work.
        %
        %       Address 1:31623/2.0     % This is the fake network address
        %                               % assigned to you by your bossnode.
        %
        % For 4D points:
        %
        %   Use your real 4D pointnet address for the first
        %   "address" statement.  For the second address statement,
        %   simply use the address of your bossnode.
        %
        %   Example:
        %
        %       Address 1:249/106.2     % Your full 4D point address
        %       Address 1:249/106       % Your bossnode's address

        Address 1:-1/-1.0               % Primary FidoNet number

        % For points:

%       Address 1:249/106.2             % Main address
%       Address 1:31623/2               % FakeNet address

        % The following statement tells Maximus how and where to find
        % your nodelist files.  Maximus supports the Version 5,
        % version 6, version 7, and FrontDoor nodelist formats.
        % If no nodelist version is specified, version 6 is used
        % by default.)
        %
        % The 'NetInfo' path tells Maximus where to find your
        % nodelist files.  The following files must be present:
        %
        % For version 5:        NODELIST.IDX
        %                       NODELIST.SYS
        %
        % For version 6:        NODELIST.IDX
        %                       NODELIST.DAT
        %
        % For version 7:        NODEX.NDX
        %                       SYSOP.NDX
        %                       NODEX.DAT
        %
        % For FrontDoor 2.x:    NODELIST.FDX
        %                       USERLIST.FDX
        %                       FDNET.PVT
        %                       FDPOINT.PVT
        %                       PHONE.FDX
        %                       PHONE.FDA
        %                       NODELIST.### (raw uncompiled nodelist)
        %
        % If you don't need a nodelist, or if you keep your nodelist
        % in the same directory as Maximus, then you can leave this
        % statement commented out.

%       Nodelist        Version 5
%       Nodelist        Version 6
        Nodelist        Version 7
%       Nodelist        Version FD

%       Path NetInfo    List\

        % The FidoUser keyword specifies the path and filename of a
        % standard "SysOp List".  This list can be generated by
        % most popular nodelist compilers.  When this keyword is
        % enabled, Max will search the SysOp list whenever a NetMail
        % message is entered.  If the addressee is found in the list,
        % the network address will be entered for you automatically.
        %
        % If using the Version 7 or FrontDoor nodelist format,
        % name lookup is automatically provided, so this keyword
        % is redundant.

        FidoUser        Fidouser.Lst

        % With the following statement enabled, Maximus will write to a
        % named file whenever a user enters an echomail message.  What will
        % be written to the file are the echo tag(s) (the "MsgName" item
        % in an area definition) of the areas that the user entered
        % messages in.  You can use this file as input for your
        % mail exporting program (such as SquishMail).  Using an EchoMail
        % tosslog will significantly reduce the amount of time it takes
        % to scan and pack EchoMail messages.

        Log EchoMail    EchoToss.Log

        % The following two statements tell Maximus which ErrorLevels to
        % exit with when a user performs a certain action.  This is useful
        % for batch files, because your batch file can trap a specific
        % errorlevel, and do the appropriate processing based on the
        % errorlevel found.  If you don't know what an errorlevel is, then
        % please consult your DOS manual.

        % The 'After Edit' statement tells Maximus which errorlevel to exit
        % with when the user enters a NetMail message.

        After Edit      Exit    11

        % The 'After EchoMail' statement tells Maximus which errorlevel to
        % exit with when the user enters an echomail message, or both
        % an EchoMail and a NetMail message.

        After EchoMail  Exit    12

        % The following lines control the display of EchoMail control
        % information.  Each EchoMail message usually contains four or
        % five lines of control information, which only adds clutter to
        % the screen.  Maximus allows one to screen out these control
        % lines from users below a certain priv level, or to disable
        % them entirely.  You can separately control the priv required
        % to view the ^A kludge lines, and also the priv required
        % to see SEEN-BYs. (If you don't want these lines to be visible
        % to anyone, including yourself, then set the priv level to
        % 'Hidden' instead.)

        Message Show Ctl_A to Hidden
        Message Show Seenby to SysOp

        % The next keyword determines the priv level required to view a
        % private message which is NOT addressed to you.  Normally, this
        % option is only available to SysOps, for monitoring of private
        % messages placed on his/her BBS.  However, if you need to allow
        % others to see private messages, then you can set this priv
        % level lower.  Setting this priv below AsstSysOp is NOT
        % recommended.

        Message Show Private to SysOp

        % The following lines concern NetMail message attribute
        % information. By default, Maximus will sent a 'plain-jane'
        % netmail message, with no special attributes.  However, you
        % can tell Maximus to automatically add certain attributes, or
        % query the user to find out which attributes s/he wants.
        %
        % Some of the attributes will allow the user/sysop to use some
        % special features -- for example, the 'FileAttach' attribute
        % lets the user send a file along with his/her netmail
        % message, which is why it is a good idea to only make this
        % option available to SYSOP and above.
        %
        % All of the options starting with 'Ask' will cause Maximus to
        % query the user as to whether or not s/he wants that
        % attribute set.
        %
        % All of the options starting with 'Assume' will automatically
        % set that particular attribute.  If you don't want a specific
        % attribute available to anyone, then set the priv. to
        % 'Hidden'.
        %
        % Note that these options function somewhat differently when
        % the full-screen reader option is enabled.  Instead of
        % specifically asking the user about each flag, Maximus will
        % allow the user to select one or more flags in the
        % "attribute" field of the message header.  (The attribute
        % field is in the top right-hand corner of the header, right
        % above the message date.)
        %
        % To get a list of available attribute keys in the full-screen
        % reader, type "?" when the cursor is on the attribute field.

        Message Edit Ask Private          Hidden
        Message Edit Ask Crash            Sysop
        Message Edit Ask FileAttach       Sysop
        Message Edit Ask KillSent         Hidden
        Message Edit Ask Hold             Sysop
        Message Edit Ask FileRequest      Hidden
        Message Edit Ask UpdateRequest    Hidden
        Message Edit Ask LocalAttach      Normal

        Message Edit Assume Private       Hidden
        Message Edit Assume Crash         Hidden
        Message Edit Assume FileAttach    Hidden
        Message Edit Assume KillSent      Hidden
        Message Edit Assume Hold          Hidden
        Message Edit Assume FileRequest   Hidden
        Message Edit Assume UpdateRequest Hidden

        % The following is a special case of the above, and is only
        % valid with the 'Ask' command.  This command specifies
        % the priv level needed to do a bombing run with the F)orward
        % command.  If you wish to disable a command for everyone
        % (including yourself), set the command's priv level to
        % 'Hidden'.

        Message Edit Ask FromFile               Sysop

        % The 'Gate NetMail' option instructs Maximus to send all
        % out-of-zone mail through the ZoneGate.  With this keyword
        % uncommented, Maximus will place addressing information inside
        % the message which instructs your packer to send it to the
        % ZoneGate.  If this keyword is commented out, Maximus
        % won't place any routing information inside an inter-zone
        % message which will cause it to be sent directly to the
        % destination.
        %
        % NOTE:  it is best to allow your packer to perform gaterouting.
        %        Unless your packer is incapable of gaterouting a message,
        %        it is suggested that this keyword be left commented
        %        out.

%       Gate NetMail

        % The following command controls the sending of netmail messages
        % to unlisted nodes.  Unless the user's priv level is at least
        % the priv specified, then s/he will not be able to enter a
        % message to a non-existant address.  If the user IS allowed
        % to send such a message, then his/her matrix account will be
        % deducted by the amount specified (in cents).  Use 0 if
        % messages to unlisted nodes are free of charge.

        Message Send Unlisted                   Sysop   50

End Matrix and EchoMail Section


Session Section

***************************** Log-on information ****************************

        % The following statements tell Maximus what baud rate the user
        % must have to get access to certain features.  'Logon Baud'
        % specifies what the user's baud rate must be just to get on
        % the system -- this is in addition to the minimum baud rates
        % specified below in the 'Session Limits' portion of the Session
        % section.  The second statement, 'NonTTY Baud', tells Maximus
        % what speeds users must call at to use either ANSI or AVATAR
        % graphics.

%       Min Logon  Baud 1200
        Min NonTTY Baud 1200

        % The next statement tells Maximus how to treat new callers, and
        % what priv. level to assign them.  If you're running a closed
        % system, which means that no users should be admitted, then
        % use the 'Logon Preregistered' keyword, which tells Maximus to
        % display the 'Application' file to the new user, and then
        % hang up.  Otherwise, you should insert the priv level which
        % you wish to be assigned to new users here.

        Logon Level     Limited
%       Logon           Preregistered

        % The next statement tells Maximus how much time to give to callers
        % for logging in.  (That includes displaying the logo, prompting
        % the user for his/her name and password, and displaying the
        % application file, if applicable.)  The default is 10 minutes.

        Logon Timelimit 15

        % The following file is just a bit of protection for the users,
        % which lets him/her know if someone has been trying to gain
        % access to his/her account.  If a hacker tries to log in to a
        % user's account and fails all five password attempts (or hangs up
        % before entering the correct password), then a flag will be set in
        % the user's profile.  When the real user logs on again with the
        % correct password, Maximus will display the 'Bad Logon' file to the
        % user, which should probably say something along the lines of:
        % 'Someone has been trying to hack your account; better go change
        % your password quick!'

        Uses BadLogon   Misc\BadLogon

        % Using this option tells Maximus that you want to prompt new users
        % for their telephone number when they log on.

        Ask Phone

        % Max 3.0 has full support for aliases, otherwise known
        % as "handles".  Max has two keywords which control alias
        % support: "Alias System" and "Ask Alias".
        %
        % In general, "Alias System" instructs Max to use the user's
        % alias by default, across the entire system.  "Ask Alias"
        % instructs Max to ask the user for an alias, but not to use
        % it unless specifically directed to do so.
        %
        % Generally, if you have "Alias System" enabled, you'll also
        % want to enable "Ask Alias".  However, the converse is not
        % always true.
        %
        % If a user has been assigned an alias, the user can always
        % use either the real name or the alias at the "What is your
        % name:" prompt, regardless of the state of the "Ask Alias" and
        % "Alias System" keywords.
        %
        % The following table describes the relationship between the two
        % keywords:
        %
        %
        %                                      ASK ALIAS
        %
        %               +---------------------------+-----------------------+
        %               | YES                       | NO                    |
        %         +-----+---------------------------+-----------------------+
        %         |     | New users prompted for    | New users not         |
        %         | YES | alias at log-on.  By      | prompted for          |
        %         |     | default, messages entered | alias at log-on.      |
        %         |     | will use the alias        | If user gives self    |
        %         |     | unless "Use Realname"     | an alias at the change|
        %         |     | is used for that          | menu, this functions  |
        %         |     | area.  Users show up      | indetically to        |
        %         |     | on W)ho is On as          | YES/YES.  Otherwise,  |
        %         |     | alias.  Alias field       | functions identically |
        %         |     | is searched and alias     | to NO/NO.             |
        %         |     | field is displayed        |                       |
        % ALIAS   |     | when doing a userlist.    |                       |
        %         +-----+---------------------------+-----------------------+
        % SYSTEM  |     | New users prompted        | No alias use          |
        %         | NO  | for alias at log-on.      | whatsoever.           |
        %         |     | By default, messages      |                       |
        %         |     | entered will use          |                       |
        %         |     | the real name unless      |                       |
        %         |     | "Use Alias" is used       |                       |
        %         |     | for that area.  Users     |                       |
        %         |     | show up on W)ho is On     |                       |
        %         |     | as real name.  Userlist   |                       |
        %         |     | displays and searches     |                       |
        %         |     | real name.                |                       |
        %         +-----+---------------------------+-----------------------+

%       Alias System
%       Ask Alias

        % The keyword instructs Maximus to permit usernames that contain
        % only a single word.  This option may be useful on systems that
        % support aliases.  (Maximus will also suppress the default
        % "What is your LAST name:" prompt when this keyword is
        % enabled.)

%       Single Word Names

        % The following two statements tell Maximus the areas in which
        % new users should be placed.  By default, all new users will
        % start off in message area 1 and file area 1.  However, if
        % you wish to change this default, you can do so here.

        First Message Area  1
        First File Area     1

        % The following keywords turn on terminal graphics checking when
        % a user logs in with either or both ANSI and RIP graphics
        % enabled. If the type of capability is not indicated by the
        % auto-detect sequence, Maximus will prompt the user and
        % ask them to verify that it is ok to proceed with them
        % enabled.

%       Check ANSI
        Check RIP

************************************ Menus **********************************

        % The following two items define the defaults for menu-file
        % display.  The first option, 'Menu Path', specifies where Max
        % is to find all of the default *.Mnu files at the beginning of the
        % session.  This path can get changed later by certain commands,
        % but this option dictates where Maximus will first look.  If you
        % leave the option blank, then Maximus will look in the 'Path System'
        % directory for the menu files.

        Menu Path       Menus

        % This option tells Maximus the name of the menu to display
        % after showing WELCOME.BBS to the user.  Unless you have a
        % special case, this should almost always be the main menu.

        First Menu      Main


******************************** RIP Graphics *******************************

        % This keyword sets the minimum speed that is required for a user
        % to enable RIP graphics.  To disable RIP graphics support, set
        % this value to 65535.

        Min RIP Baud    65535

        % The RIP Path tells Maximus where to find the RIP icon and scene
        % files, as used by the [ripsend] and [ripdisplay] MECCA tokens.

        RIP Path        RIP

***************************** General Filenames *****************************

        % These contain the paths and names of various display files
        % used throughout the system.  If the keyword in front of the
        % file is 'Uses', then SILT will check to make sure that the
        % file exists while compiling the PRM file. If the file is
        % missing, then SILT will abort with an error.  This command is
        % generally good to use on your important system files, those
        % that Maximus won't run without.  However, if you don't wish to
        % have Maximus check all of your files (for time reasons), then
        % you can change the 'Uses' in front of each filename in this
        % section to 'File'.

        % This item is the name of the .BBS file to display (don't include
        % the extension!) between the "MAXIMUS vx.yy" and
        % "Please enter your name:" prompts.  It should usually be
        % fairly short, and should generally give a bit of information
        % about your system, including your [the Sysop's] name, the
        % system name, etc.  The logo file should be no larger than
        % one kilobyte, and it shouldn't include any ANSI sequences or
        % cursor codes.

        Uses Logo       Misc\Logo

        % The following file is displayed AFTER a new user enters a name
        % at the prompt, but BEFORE he/she is prompted "First Last [Y,n]?".

        Uses NotFound   Misc\Notfound

        % This is the file that is displayed to all new callers, right
        % after they enter their name and password.  If you're running a
        % closed system, then this file should tell the user why they
        % weren't allowed on, or optionally hang up right away.  Otherwise,
        % this file should generally tell the user about what will be
        % expected of them, what the system rules are, etc.

        Uses Application Misc\Applic

        % This is the file that is displayed to all callers who have called
        % your system more than seven times, right after they enter
        % their name and password, but before reaching the main menu.
        % NOTE:  If you wish to have more than one file displayed between
        % the login prompt and the main menu, you should use 
        % an '[onexit]' MECCA code to display it after your welcome
        % file.  You can chain as many of these calls as you wish,
        % so you can have an unlimited number of files to display 
        % between the login prompt and the main menu.

        Uses Welcome    Misc\Welcome

        % This is the file displayed to new users right before they are
        % asked to enter their password.  This should generally tell the
        % user about password specifications, such as maximum length,
        % the fact that there can't be any spaces in the password, etc.

        Uses NewUser1   Misc\Newuser1

        % This file is displayed after a new user logs in but before
        % the standard configuration questions (ANSI graphics, RIP support,
        % and so on) are asked.
        %
        % After displaying this file, if the 'usr.configured' flag is set,
        % Maximus will skip the internal configuration questions.  This
        % feature can be used by a MEX script to bypass the internal
        % configuration sequence.

%       Uses Configure  Misc\Configure

        % The 'NewUser2' file is what is displayed to new users, in
        % lieu of the 'Welcome' file.  See above for more detail.
        % (Most systems will either want to have this the same as their
        % welcome file, or have it point to something similar to the
        % 'Application' file.)

        Uses NewUser2   Misc\Newuser2

        % The 'Rookie' file is what is displayed to users who have called
        % between two and seven times.  See above under 'Welcome' for more
        % details.  If this statement is commented out, Maximus will
        % display the welcome screen instead.

%       Uses Rookie     Misc\Rookie

        % Max uses the quote file to obtain random quotations for the
        % user.  Quotes can be displayed by using the '[quote]' token
        % in a .MEC file.  The quote file should contain plain ASCII
        % text, with a blank line between every quote.  Each time
        % the '[quote]' token is used, Maximus will draw another
        % item from the file and display it to the user.

        Uses Quote      Misc\Quotes

        % This is the file that Maximus displays to the user when s/he
        % logs on, if s/he has been on the system too long for the
        % current day.  It should generally specify what the user's time
        %limit is, and how long s/he has actually been on.

        Uses DayLimit   Misc\Daylimit

        % This file is displayed to the user right after the welcome
        % file, but before the main menu, if the user has been on
        % previously that day.  It should generally contain some sort of
        % warning telling the user how much time s/he has left.

        Uses TimeWarn   Misc\Timewarn

        % This is what gets displayed to the user if s/he has attempted
        % to log on at a too-slow baud rate.

        Uses TooSlow    Misc\Tooslow

        % This is displayed when a user selects a message area that
        % doesn't exist, or s/he doesn't have access to.  This is
        % optional;  If no file is defined, then Maximus will just
        % display the standard "That area doesn't exist!" message
        % instead.

%       Uses Cant_Enter_Area Misc\CantEntr

******************** Message and file section information *******************

        % The following two statements control the location of Maximus'
        % message and file-area data files.  The SILT compiler will
        % create these for you, so just make sure that these two
        % point to a valid filename.

        MessageData             marea
        FileData                farea

        % The following statement controls the keys which can be used
        % on the menu for the A)rea Change command, in both the
        % message and file areas.  The first key in the sequence
        % controls the Change-To-Prior-Area command, while the second
        % key controls the Change-To-Next-Area command.  Finally, the
        % third character controls the List-Areas command.

        Area Change Keys        []?

************************** File section information *************************

        % The following two items define the format of the header and
        % format lines for the file area's A)rea command.  Due to
        % the complexity of this topic, it is covered only in the
        % control-file reference.  However, the following two default
        % lines will cause Maximus to behave as it did in version 1.00.
        % If you don't know what you're doing, then you should
        % probably leave these alone.

        Format FileHeader %x16%x01%x0fFile Areas %x16%x01%x0d컴컴컴컴컴컴컴%x0a%x0a
        Format FileFormat %x16%x01%x0d%-20#%x16%x01%x07 ... %x16%x01%x03%-n%x0a

        % If you want your file-area menu to have a two-column display,
        % comment the above 'Format FileFormat' line out, and uncomment
        % this instead:

%       Format FileFormat %x16%x01%x0e%x16%x01%x0d%-17.17#%x16%x01%x07.. %x16%x01%x03%-19.19n%2c%x0a

        % This is what will be displayed at the END of the menu.  By
        % default, this is disabled.

%       Format FileFooter

        % This option tells Maximus what the *highest* area to search
        % with the L)ocate command.  Although Locate will not search
        % passworded/barricaded areas by default, you may want
        % to give a certain area a limited amount of protection
        % by telling Maximus not to search it.  Note:  This limit
        % ALSO applies to the "A>" (Area Next) and "A<" (Area Previous)
        % commands in the file area.  If a user wishes to access an
        % area above the one you have specified, then s/he must type
        % the actual number in.  Any areas above the one you specify also
        % will *not* be shown on the Maximus-generated A)rea list.
        % If no highest area is specified, then all areas will be
        % searched by default.

%       Highest FileArea        15

        % The following option tells Maximus to check the available disk
        % space before allowing an upload, and to abort the upload if
        % there is less than the specified number of kilobytes free
        % on the upload drive.

        Upload Space Free       200

        % The following command is a compliment to 'Upload Space
        % Free'.  This is the file displayed when there is less 
        % than the specified amount of space free left on the 
        % upload drive.

        Uses NoSpace            Misc\NoSpace

        % The following command tells Maximus to create a separate log
        % of files uploaded to your system.  This file is straight
        % ASCII, and contains a list of who the uploader was, the
        % file's name, the file's size, and the date/time the file
        % was uploaded.

        Upload Log              UL.Log

        % The Upload Check Dupe option instructs Max to check for
        % duplicate files when the U)pload command is selected.  When
        % this feature is enabled, Max will automatically detect
        % and stop users from uploading duplicate files.  To use this
        % feature, you must recompile your file areas with FB after
        % making any changes.  For more information on FB, please
        % see the program documentation.
        %
        % By default, the dupe checker will ignore file extensions.
        % (In other words, if there's a file called FILENAME.ZIP, an
        % upload of FILENAME.LZH will be refused.)  However, because
        % there may be a legitimate reason for uploading a file
        % with the same base name (but a different extension), the
        % "Upload Check Dupe Extension" keyword instructs Max to
        % check BOTH the filename and the extension when looking for
        % duplicate uploads.

        Upload Check Dupe
%       Upload Check Dupe Extension

        % The "FileList Margin" keyword instructs Max to indent the
        % specified number of columns after wrapping a long file
        % description.  By default, Max will indent the description
        % by 34 columns to make it line up with the description on
        % the line above.  However, if you're running an external
        % program to (for example) count the number of times that
        % a particular file has been downloaded, you may want to
        % increase this number to make everything line up
        % properly.

%       FileList Margin 34

        % The 'File Date' command tells Maximus which way to
        % display dates inside the file areas.   You can choose from
        % one of several formats, including U.S.A.,
        % Canadian/British, Japanese, and scientific. In
        % addition, you can tell Maximus to get the files' dates and
        % sizes directly from their directory entries, or you can enter
        % the dates into FILES.BBS itself, for greater speed when
        % displaying file catalogs  n CD-ROMs and WORMs.
        %
        % <type> can be either of 'Automatic' or 'Manual'.
        % 'Automatic' means that Maximus will look at the file's
        % directory entry to determine both the file's size and date.
        % If <type> is 'Manual', then Maximus won't look at the
        % directory entry at all, and will assume that the size and date
        % information is imbedded in FILES.BBS itself, as ASCII
        % text.
        %
        % [format] specifies the format to use for file-entry date
        % stamps, and it can be any of the following options:
        %
        %      mm-dd-yy  (U.S.A., default)
        %      dd-mm-yy  (Canada/England)
        %      yy-mm-dd  (Japanese)
        %      yymmdd    (Scientific)
        %
        % If <type> is 'Automatic', then the format above will be used
        % when DISPLAYING files' directory entries.  In other
        % words, it will be generated at runtime.  However, if <type> is
        % 'Manual', then Max will insert the date, in the format
        % specified, into the FILES.BBS catalog when the file is
        % UPLOADED, and will get the date from FILES.BBS from that
        % point on.  You must manually insert dates for any preexisting
        % files in the file areas.
        %
        % The format specified by [format] will also be used when
        % prompting the user for a date while doing  a new-files check.

        File Date Automatic     mm-dd-yy
%       File Date Automatic     dd-mm-yy
%       File Date Automatic     yy-mm-dd
%       File Date Automatic     yymmdd
%       File Date Manual        mm-dd-yy

        % The following keyword determines the errorlevel that Maximus
        % will exit with for 'Type Errorlevel' external protocols.
        % For more information on protocols, please see PROTOCOL.CTL.

        External Protocol Errorlevel    9

        % This specifies the filename to display when a user asks
        % for help in the L)ocate command.

        Uses LocateHelp         Hlp\Locate

        % This specifies the filename to display when a user asks for
        % help in the C)ontents command.

        Uses ContentsHelp       Hlp\Contents

        % This file is what is displayed to the user when s/he is at
        % a too-slow baud rate, and attempts to download or upload a
        % file.

        Uses XferBaud           Misc\Xferbaud

        % This is just a flat .MEC file containing a list of the file areas
        % available on your system.  This will be displayed if a user
        % enters the '?' command on the A)rea change menu.  If this is
        % commented out, Max will generate such a list itself, and display
        % it to the caller.  However, this is generally slower than
        % using a dump file.  (Also, you can customize the dump file,
        % whereas the Maximus area menus is fairly simple.)

%       Uses FileAreas          Misc\Filearea

        % This option defines a mini-essay that Maximus will display to the
        % user when s/he enters an invalid filename to upload, such as
        % a filename with two periods, more than 12 characters for the
        % filename, etc.

        Uses Filename_Format    Misc\Fformat

        % If the next keyword is uncommented, the specified file will be
        % displayed at the D)ownload protocol selection screen, instead
        % of the "canned" menu which Maximus normally generates.

%       Uses ProtocolDump       Misc\Protocol

        % For CD-ROMs or other slow storage media, the "Stage Path"
        % keyword can be used to declare a temporary transfer area
        % on a hard drive. When this keyword is enabled, files in
        % areas with the "Staged" or "CD" qualifiers will be copied
        % to this area before they are sent to the user.

        Stage Path Stage

        % The following keyword can be used to check uploaded files
        % for viruses.  When a file is uploaded, Max will call the
        % specified batch file, using the following command-line
        % parameters:
        %
        % c:\file\upload\ vgademo .zip c:\max\misc
        %       (1)         (2)   (3)      (4)
        %
        % (1) is the directory where the new upload was placed.
        % (2) is the root filename of the new upload.
        % (3) is the extension of the new upload.
        % (4) is the path of the Max "misc" directory, without a backslash.
        %
        % Max will call this batch file once for each file uploaded.  For
        % more information on this keyword, please see the program
        % documentation.

%       Upload Check Virus      Vircheck.Bat

************************** Local file attaches *******************

        % This keyword specifies the root name of the file attach
        % database. This should only specify path and up to four
        % letters for the root of the database name. Only the root
        % can be specified, since a number of additional database
        % files are created using that name.

        Attach Base     Att

        % This is the directory used as the default holding area
        % for local file attaches. This can be overridden on an
        % area by area basis by using the "AttachPath" keyword in
        % the area definition MsgArea.Ctl.

        Attach Path     Attach

        % This is the archiver type used for local attach storage
        % and must be defined in Compress.Cfg

        Attach Archiver ZIP

        % This keyword tells Maximus how to handle received file
        % attaches.
        %
        % With 'Kill Attach Never', Maximus will never kill a
        % received file attach unless the associated message is
        % deleted.
        %
        % With 'Kill Attach Ask [acs]', Maximus will prompt the user
        % to delete the attach upon receipt.  (If the user's privilege
        % level is less than the optional ACS, the attach will be
        % deleted without asking anyway, as with 'Kill Attach Always'.)
        %
        % With 'Kill Attach Always', Maximus will always delete a
        % received file attach.

        Kill Attach     Ask Normal


************************ Message section information ************************

        % This keyword defines the maximum size of a message that can
        % be uploaded using the Msg_Upload command.

        MaxMsgSize      8192

        % The following two items define the format of the header and
        % format lines for the message area's A)rea command.  Due to
        % the complexity of this topic, it is covered only in the
        % documentation.  However, the following two default lines
        % will cause Maximus to behave just as Maximus 1.00 did.  If
        % you don't know what you're doing, then you should
        % probably leave these alone.

        Format MsgHeader  %x16%x01%x0fMessage Areas %x16%x01%x0d컴컴컴컴컴컴컴%x0a%x0a
        Format MsgFormat  %x16%x01%x0e%*%x16%x01%x0d%-20#%x16%x01%x07 ... %x16%x01%x03%n%x0a

        % If you want your message area menu to have a two-column display,
        % comment the above 'Format MsgFormat' line out, and uncomment
        % this instead:

%       Format MsgFormat  %x16%x01%x0e%*%x16%x01%x0d%-16.16#%x16%x01%x07.. %x16%x01%x03%-19.19n%2c%x0a

        % This is what will be displayed at the END of the menu.  By
        % default, this is disabled.

%       Format MsgFooter

        % The following command enables the <Left> and <Right> keys
        % for message reading, in addition to the <Alt-R> (reply),
        % <Alt-E> (enter) and <Alt-K> (kill) keys.

        Arrow Keys to Read

        % The following option is almost identical to the "Highest
        % FileArea" command, except it limits the behavior of the
        % S)can command, instead of the Locate command.  See above
        % for more details.

%       Highest MsgArea         80

        % This option is the same as the File Area display file.  This file
        % will be displyed to users at the A)rea Change prompt.  For more
        % information, please see the "Uses FileAreas" token.

%       Uses MsgAreas           Misc\Msgarea

        % This is the file displayed to the user after using the
        % mailchecker, if they have no mail waiting.

        Uses NoMail             Misc\NoMail

        % This is the file displayed to the user when s/he selects '?'
        % in the MaxEd full-screen editor.

        Uses MaxEdHelp          Hlp\FSED

        % This is the file displayed to first-time, NOVICE-level callers
        % when they first enter the BORED editor.

        Uses BOREDhelp          Hlp\1stEdit

        % This is the file displayed when users select the E)dit option on
        % the BORED editor menu.

        Uses ReplaceHelp        Hlp\Rep_Edit

        % This is the file displayed when a user is starting to enter
        % a message header. This normally contains information about
        % message attributes, using aliases, 'anonymous' areas and so
        % forth.

        Uses HeaderHelp         Misc\Hdrentry

        % This file is displayed as the user entered the message editor,
        % irrespective of the type of editor used. This can offer additional
        % help, or set up the screen display suitably for RIP callers etc.

        Uses EntryHelp          Misc\Msgentry

        % What follows is the name of an external editor to run instead
        % of MaxEd.  If you're replying to a message, the message
        % will be automatically quoted for you and placed in a file
        % called MSGTMPxx.$$$ in the current directory, where 'xx'
        % represents the current task number.
        %
        % Maximus will expect to find the actual message text in that
        % same file and it will save it as your message.  If that file
        % cannot be found, then Maximus will abort your message entry.
        %
        % This editor can be used by any LOCAL user who is in a class
        % with the "MailFlags LocalEditor" setting in access.ctl.
        %
        % Also, if you want your editor to be usable from remote,
        % ensure that the first character of your editor string as
        % an "at sign" ('@').  The editor will then be available to users
        % who are in a class with the "MailFlags Editor" setting in
        % access.ctl.
        %
        % If a "%s" appears in the editor string, then it will be
        % replaced by the name of the temporary message file which
        % Maximus expects to find the reply in.

%       Local Editor            C:\Util\Q.Exe %s
%       Local Editor            @C:\Util\Emedit.Exe %s

        % The following two options control what Maximus will allow users
        % to do when entering a message.  If you have the 'Edit Disable
        % UserList' option uncommented, then users will not be able to
        % get a list of all the users on this system by pressing a '?'
        % at the "To:" prompt.  (Note that the userlist will still be
        % checked for entering private messages -- however, users just
        % won't be able to look at the list.)

%       Edit Disable            Userlist

        % This option disables the use of the MaxEd full-screen editor,
        % if you don't want to have your users using it, for some reason
        % or another.

%       Edit Disable            MaxEd

        % This option tells Maximus what to do with private messages that
        % a user reads.  By default, Maximus will ask users if they
        % want to kill the message upon receiving it (to help keep your
        % disk clean).  However, you can also instruct Maximus to
        % automatically delete received messages with no confirmation.

        Kill Private            Never
%       Kill Private            Always
%       Kill Private            Ask

        % The following two commands define the priv level required
        % to kill and delete messages (respectively) within either
        % the B)rowse command or the internal mailchecker.
        %
        % Note that Maximus performs additional checks to determine
        % whether or not the kill and reply options should be made
        % available to the user.  Maximus will look for a menu
        % called "MESSAGE", and it will examine the privilege
        % for the Msg_Reply and Msg_Kill commands contained therein.
        %
        % If the user's priv level is not high enough to permit access
        % to the above commands, then they also will not appear on
        % the mailchecker menu.
        %
        % Note that the "MESSAGE" menu name is not configurable.
        % However, users do not necessarily need to be able to
        % access this menu from elsewhere; it is possible to
        % use another menu name for your main message menu, as
        % long as the "MESSAGE" menu exists and contains the above
        % two options.
        %
        % For example, if your main message menu is called
        % something other than "MESSAGE", you should have at least
        % the following dummy menu present in your MENUS.CTL,
        % specifying the appropriate access levels for the
        % Msg_Reply and Msg_Kill options:
        %
        %  Menu MESSAGE
        %          Title      MESSAGE (%t mins)
        %          MenuHeader Message
        %          Msg_Reply            Demoted  "Reply to a message"
        %          Msg_Kill             Demoted  "Kill (delete) msg"
        %  End Menu

        Mailchecker Kill        Transient
        Mailchecker Reply       Transient

        % When a user enters a message in an anonymous message area,
        % using a name other than their own, an ^A kludge line
        % will be appended to the message, stating who the real author
        % of the message was.  Normally, only other SysOps can see this
        % kludge, and it's useful when tracking down users who abuse
        % the anonymous feature.  You should leave this option commented
        % out, unless you have a special case where true anonymity 
        % is required.
        %
        % NOTE!  This feature can also be toggled, on an area-by-area
        % basis, through the definitions in MSGAREA.CTL.

%       No RealName Kludge

        % Use UMSGIDs means that every message ever entered in a Squish
        % base will have a unique message number which will never be
        % reused, regardless of deletions.  This provides the "constant
        % message number" feature that has been requested by many.  This
        % feature only works for Squish-style bases, so *.MSG areas will
        % remain the same as before.

%       Use UMSGIDs

        % The following keyword specifies the area number in which
        % log-off comments should be placed.  (This area number also 
        % applies to the Leave_Comment menu function and the 
        % [leave_comment] MECCA token.)

        Comment Area            1

************************** Message tracking system **********************

        % The following keywords, when uncommented, enable the Relational
        % Message Tracking System (MTS). This feature makes it easy for
        % organizations to ensure that technical support questions are
        % answered on a timely basis.

        % This keyword specifies the root name of the tracking
        % database. This should only specify path and up to four
        % letters for the root of the database name. Only the root
        % can be specified, since a number of additional database
        % files are creating using that name.
        %
        % For example, the following database files would be created
        % for a base name of "c:\max\trk":
        %
        %       c:\max\trkmsg.db
        %       c:\max\trkmsg.i00
        %       c:\max\trkmsg.i01
        %       c:\max\trkmsg.i02
        %       c:\msg\trkarea.i00
        %       c:\msg\trkown.i00

        Track Base Trk

        % This keyword controls the priv level required to view
        % tracking information of messages owned by others.  To view
        % tracking information in a message, that message must either
        % be owned by you, or you must have at least the priv level
        % specified by this keyword.

        Track View Clerk

        % This keyword controls the priv level required to modify
        % tracking information for messages which are not owned by you.
        % This priv is also required to delete messages from the tracking
        % database and to access the Track/Admin menu.

        Track Modify Clerk

        % This keyword gives the name of the 'exclusion list' that Max
        % uses to control automatic message assignment.  By default,
        % Max will add messages to the tracking database if:
        % 
        % 1) the message area is declared with the 'Audit' keyword
        % 2) the message area has a default owner assigned
        % 3) the user entering the message is not the default owner
        % 
        % In some situations, it may also be desirable to NOT track
        % messages which were entered by certain users.  The exclusion
        % list is a flat text file, one name per line, which lists
        % the names of all users to be excluded from the tracking system.
        % 
        % Messages entered by these users can be manually placed in
        % the tracking system using the Track/Insert command, but
        % automatic insertion will be disabled.

%       Track Exclude Exclude.Trk



****************************** Outside information **************************

        % The 'Leaving' file is displayed to the user when they go
        % outside via a menu option, and the 'Returning' option is
        % displayed when they return.  Please see the accompanying
        % control file, MENUS.CTL, for more information.

%       Uses Leaving            Misc\Leaving
%       Uses Returning          Misc\Return

        % (For Maximus-DOS only)
        %
        % Normally, when Maximus runs an external program for whatever
        % reason, it will save the current directory on all of the
        % currently-accessible drives, in case an external program
        % changes or interferes with the current directory.  You MUST
        % inform Maximus of valid hard drive letters on your system.
        %
        % Normally, this statement should include all drives on the
        % system, EXCEPT any removable devices, such as floppy disk or
        % CD-ROM drives.  If you accidentally do specify a floppy
        % drive with this command, then Maximus will try to access it
        % every time it executes an external program.  However, it is
        % always okay to specify more drives than you currently have
        % available.  If you wish to disable directory saving
        % entirely, then don't specify any drives to save.
        %
        % The following command tells Maximus to save the directories
        % of drives C: through Z:.  WARNING:  If you have floppy
        % drives which use drive designations other than A: or B:,
        % make sure to edit the following statement!  Also ensure that
        % you remove any references to CD-ROM drives.
        %
        % NOTE!  OS/2 automatically saves and restores all
        % drives, so this keyword is ignored when using OS/2.

        Save Directories        CDEFGHIJKLMNOPQRSTUVWXYZ

        % Use this instead, if you want to disable directory saving:

%       Save Directories

        % The following two items define the files that are displayed
        % to the user when the Sysop leaves and returns from DOS
        % with Alt-J.  If you don't define these files, then nothing
        % will be displayed.

        Uses Shell_Leaving      Misc\ShellBye
        Uses Shell_Returning    Misc\ShellHi

        % This option tells Maximus to use an external program for the
        % C)hat key, instead of calling the internal CHAT module.
        % Make sure to include all necessary command-line parameters
        % here.
        %
        % To run a MEX program instead of an external .exe, simply
        % use a ":" as the first character in the command name.

%       Chat External           Chat.Exe Scott Dudley /P:1

        % To use the MEXchat full-screen chat utility, use the following
        % line:

%       Chat External           :M\MexChat



******************************** Logoff items *******************************

        % This tells Maximus which errorlevel to exit with after each
        % caller.  If none is specified, then this defaults to errorlevel
        % five.  Note that this, and ALL other errorlevel must be in the
        % range of five to 255.  Maximus uses the first four errorlevels
        % internally, but the others are free for you to use.

        After Call Exit         5

        % This file is displayed to the user after s/he selects the
        % G)oodbye option from one of the menus, and confirms that
        % s/he wants to disconnect.

        Uses ByeBye             Misc\Byebye

******************************* Yell Information ****************************

        % The Tunes file contains a list of "songs" to play on the local
        % PC speaker.  By default, Max only uses the tune file for
        % the Y)ell command, but alternate tunes can also be played
        % using the [tune] MECCA token.  This keyword specifies the
        % location of the tunes file; by default, Max uses TUNES.BBS
        % in the main Max directory.

        Uses Tunes              Tunes

        % Maximus is capable of logging local user-to-sysop chat sessions.
        % Although this logging can be toggled manually with <Alt-C>, 
        % some SysOps may wish to have the chat log turned on all the
        % time.  The following keyword causes Max to start logging as
        % soon as chat mode is entered; if the chat log is not required,
        % it can be toggled using <Alt-C> as normal.
        %
        % Chat captures will always go into the Max root directory,
        % using a filename of CHATLOG.*, using the current node number
        % as the file's extension.

%       Chat Capture On

        % This command controls the default state of the noise-maker for
        % the yell command.  This option can be toggled by using the
        % '!' local keyboard command while a user is on-line.  This
        % flag is reset every time you recompile your .PRM file -- if
        % you wish to have this flag default to OFF, then uncomment the
        % following line.  To leave it on, make sure the following line
        % is commented out.  NOTE: This flag is in *addition* to the Yell
        % statements above, and if turned off, no Y)ell command will
        % make noise on your side, no matter what the current time.

%       Yell    Off

********************** Miscellaneous session information ********************

        % The following statement tells Maximus to enable high-bit
        % characters in most areas of the BBS.  When this statement
        % is enabled, users can log on using high-bit characters in
        % their names, enter high-bit file descriptions, and so on.
        %
        % However, you must still manually enable the "Style HighBit"
        % flag for each individual message area.
        %
        % Warning: if this keyword is enabled, callers who have
        % their terminal programs configured for 7 bits will
        % not be able to log onto your system.

%       Global High Bit

        % These following two statements tell Maximus how to display the
        % local date and time for messages, files, etc.  Everything in
        % the string is copied verbatim, except for the special, two-
        % character sequences beginning with a percentage sign.  The
        % following sequences will translate into different elements of
        % the date and time:
        %
        %   %A - Either 'am' or 'pm', as appropriate.
        %   %B - The month, as a decimal.
        %   %C - The month, in abbreviated form
        %   %D - The day-of-month, as a decimal
        %   %E - The hour, in the range of 1 to 12
        %   %H - The hour, in the range of 0 to 23
        %   %M - The minute
        %   %S - The second
        %   %Y - The year, without the century

        Format Date             %C-%D-%Y
        Format Time             %H:%M:%S

        % Canadian date format:
%       Format Date             %D-%B-%Y

        % Japanese date format:
%       Format Date             %Y-%B-%D

        % Time, 24hrs including time zone
%       Format Time             %H:%M est


        % When the next option is uncommented, Maximus will display a
        % status line at the bottom of the screen while a remote caller
        % is on-line.  If you do not wish to have a status line, then
        % comment out this option.  NOTE:  This option will only
        % work if you are using the 'Video IBM' video mode!  If you
        % try to use the status line with another video mode, then
        % Maximus won't display the status line.

        StatusLine

        % Next, the following file is what is displayed to users
        % attempting to access a barricaded area, before entering the
        % required password.

        Uses Barricade          Misc\Barricad

        % When using the internal chat mode, the following option
        % defines the name of the text file to be displayed when chat
        % mode is entered.  If none is specified, then the default
        % "CHAT: start" is displayed.

%       Uses BeginChat          Misc\ChatBegn

        % This is the compliment to the 'Chat Begin' command.  The
        % Chat End command controls the file that will be displayed
        % instead of the default "END CHAT".

%       Uses EndChat            Misc\ChatEnd

        % The next keyword controls the input timeout safety feature,
        % for local log-ons.  With this keyword uncommented, Maximus
        % will automatically log a local user off, after five minutes
        % of inactivity.  If this keyword is commented out, then
        % Maximus will let a local user delay for any length of time.
        %
        % NOTE: this feature is always active for remote callers.

%       Local Input Timeout

        % The 'Input Timeout' keyword tells Max how long it should
        % wait for a user to press a key.  By default, Max will wait
        % for 4 minutes, send a warning message, wait another 2 minutes,
        % and if no keys are pressed during that time, Max will then
        % hang up.
        %
        % This keyword can be used to change the length of the
        % original 4-minute waiting period.  Max will wait for the
        % number of minutes you specify, send a warning message, and
        % then wait one minute in addition.  The value for 
        % "Input Timeout" should be between 1 and 127.

        Input Timeout           4

 End Session Section



% The following two statements are to provide compatibility with the
% Opus 1.03 and 1.10 .PRM file structures.  To generate Opus .PRM files,
% uncomment either or both of the following lines:

% This is the version 14 (Opus 1.03) control file.

%Version14       Ver14.Prm

% This is the version 17 (Opus 1.10) control file.

%Version17       Ver17.Prm

% The following six statements specify files to Include into
% this control file.  SILT will read the files specified, and
% behave exactly as if they were part of this file.  In other
% words, you can insert the Areas and Menus into this file if
% you have a burning desire, but it is usually more convenient
% to store them separately.

Include Colors.Ctl
Include Reader.Ctl
Include Language.Ctl
Include Msgarea.Ctl
Include Filearea.Ctl
Include Menus.Ctl
Include Protocol.Ctl

