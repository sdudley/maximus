%           ----------------------------------------------
%           MSGAREA.CTL - sample message area control file
%           ----------------------------------------------
%
%
%  This file describes your system's message areas.  Each message
%  area can be given a name.  This name consists of letters,
%  numbers, and any of the following characters:
%
%       . / _ ~ ! @ # $ % ^ & * ( ) - + = |
%
%  WARNING!  If you happen to "renumber" the area numbers within this
%  file (such as by changing message area 2 to area 1, and then changing
%  area 3 to area 2, and so on), make sure to delete \MAX\MTAG.* and
%  \MAX\OLR\DATS\*.*.  This is only a concern when changing the
%  name/number of every area; those two files contain information about
%  tagged message areas and the QWK area pointers.  If you change many
%  area numbers without deleting these files, uploaded QWK messages and
%  message tagging information may be incorrect.
 
% By default, area 1 is used for storing log-off comments.  If
% you wish to change the name or number of this area, you must
% adjust the "Comment Area" option in MAX.CTL.

MsgArea 1
        % The 'ACS' keyword defines the privilege level required to
        % access this area.  If the user's priv is not high enough, they
        % won't be able to get in to this area, and Max will pretend
        % that it doesn't even exist.
 
        ACS     Transient
 
        % The Desc keyword gives a verbose description of the message
        % appear.  This will be shown to users who enter the area.  The
        % description must be less than 60 characters long.
 
        Desc    Comments to the SysOp

        % The Style keyword sets various options for this message area.
        % Many different attributes can be specified on one "style" line.
        %
        % FILE FORMAT.  Only one of the next two options may be specified:
        %
        %       Squish          Use the Squish file format for storing
        %                       messages in this area.  This is the default.
        %
        %       *.MSG           Use the FidoNet-standard *.MSG format for
        %                       storing messages in this area.
        %
        % DISTRIBUTION.  Only one of the next four options may be specified:
        %
        %       Local           This is a local message area.  Messages
        %                       entered in this area do not leave your
        %                       system.
        %
        %       Net             This is a NetMail area.  Areas of this
        %                       type are used for private, point-to-point
        %                       communication with users on remote system.
        %
        %       Echo            This is an EchoMail area.  Messages
        %                       entered in this area will be broadcast
        %                       to all nodes which receive the area.  In
        %                       addition, messages entered here will have a
        %                       tear and origin line appended automatically.
        %
        %       Conf            This is a conference area.  Messages
        %                       entered in this area will be broadcast to
        %                       all node swhich receive the area.  In
        %                       addition, messages entered here will have a
        %                       PID kludge appended automatically.
        %
        % MESSAGE VISIBILITY.  Either or both of the following options
        % may be specified:
        %
        %       Pub             Allow public messages in this area.
        %                       Public messages can be read by all
        %                       users.
        %
        %       Pvt             Allow private messages in this area.
        %                       Private mesasges can only be read
        %                       by the sysop and the addressee.
        %
        % Note that both "Pub" and "Pvt" can be specified to permit
        % both types of messages.
        %
        % MISCELLANEOUS OPTIONS.  See the documentation for detailed
        % information on the use of these options.
        %
        % HighBit       - support 8-bit characters in this area
        % Anon          - allow anonymous messages
        % ReadOnly      - do not allow message posting
        % NoNameKludge  - do not add kludge containing user's real name
        % RealName      - force the use of user's real name
        % Alias         - force the user of user's alias
        % Audit         - message auditing features for this area
        % Hidden        - area is not visible on menu

        Style   Squish Local Pvt

        % The 'Path' keyword tells Maximus where to store the messages
        % in this area.  For *.MSG-format areas, a directory will
        % be created using this name.  For Squish-format areas,
        % the path is treated as a base filename, and a number of
        % extensions will be added.
 
	Path	Msg\Private

        % The following options are also commonly used.  See
        % the program documentation for information on other area
        % keywords:
        %
        % Barricade <menu_name> <barricade_file>
        % 
        %   This can be used to selectively adjust users' priv levels
        %   when they enter this area.  Please see the program
        %   documentation for more information.
        % 
        % MenuName <orig_menu> <new_menu>
        % 
        %   This keyword instructs Max to use a special menu for this
        %   area only.  When the user has this message area selected,
        %   whenever a system function tries to display <orig_menu>
        %   (typically the MESSAGE menu), Maximus will substitute
        %   <new_menu> instead.
        % 
        % Override <menu_name> <option_type> <ACS> [letter]
        % 
        %   This option can be used to adjust the privilege level of
        %   a menu option for one message area only.  Please see
        %   the program documentation for more information.
        % 
        % Renum Max <msgs>
        % Renum Days <days>
        % 
        %   These two options can be used to selectively purge
        %   messages from the message areas with an automated
        %   program.  Please see the program documentation for
        %   more details.
 
	Renum Max       100
End MsgArea

 
% This is a local message area which contains public messages.

MsgArea 2
        ACS     Demoted
 
	Desc	Public Messages
        Style   Squish Local Pub Pvt
        Path    Msg\Public

        % Keep up to 150 messages in this area
        Renum Max       150
End MsgArea

 
 
% This defines an EchoMail area called "MUFFIN", which is the
% Maximus support echo.  This area will be stored in the
% Squish message format.

MsgArea MUF

        ACS     Sysop
        Desc    FidoNet Echo: MUFFIN (Maximus Tips'n'Tricks)
        Style   Squish Echo Pub

        % The "Tag" keyword specifies the canonical message area
        % name for EchoMail areas.  This tag should be the same
        % as defined in your EchoMail processor.  (For Squish,
        % this is the name specified on an "EchoArea" line.)

        Tag     MUFFIN
 
        Path    Msg\Muffin
 
        % For EchoMail areas, you can also specify a custom origin line.
        %
        % Instead of using the default origin line, which is specified
        % in MAX.CTL (using the "Name" keyword), a custom origin line
        % may be used for each area.  The format of a custom origin is:
        %
        %       Origin <primary_addr> <seenby_addr> [text]
        %
        % <primary_addr> is the address to use in ^aMSGID kludges
        % and on the origin line.  A period (".") may be specified
        % to use the default address.
        %
        % <seenby_addr> is the address to use on the SEEN-BY line.
        % A period (".") may be specified to use the default address.
        %
        % [text] is optional, and is simply the origin line you wish
        % to use for this area.  It may be up to 60 characters long,
        % and will get truncated if it is longer.  If you specify no
        % [text], or no origin statement altogether, then Maximus
        % will use the `System' line in MAX.CTL as the origin line
        % for this area.
 
        %       Origin  . . Maxmius is great!
 
	Renum Max       100
End MsgArea

 
 
MsgArea NET
        % This defines your primary NetMail area, accessible with an
        % area "number" of NET.  Note that this area defaults to the
        % Squish message format.
 
	ACS	Sysop
	Desc	NetMail
	Path	Msg\Net
        Style   Net Pvt

        Renum Max 100
End MsgArea

 
