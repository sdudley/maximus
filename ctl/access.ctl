
    % User levels section
    %
    % This section describes the user levels used on the BBS.
    % The default scheme is compatible with Maximus version 2.x
    % and earlier.
    %
    % Any number of privilege levels may be defined for use with Maximus.
    %
    % Each 'Access' subsection defines one level of privilege, where:
    %
    % Access <name>             <name> is the symbolic name used to
    %                           reference this privilege levels. This
    %                           symbol may be used interchangably with
    %                           'Level'.
    %
    %                           <name> must start with an alphabetic
    %                           character and can contain any combination
    %                           of letters, digits and underscore
    %                           but no spaces or tabs. <name> must be unique.
    %
    %   Level       <lvl>       <lvl> is the privilege level, and is used
    %                           to reflect the relation of one privilege
    %                           level to another. The level used MUST be
    %                           unique.
    %
    %   Desc        <desc>      <desc> is an optional description. If not
    %                           specified here, the access name itself is
    %                           assumed. <desc> MAY contain tabs, spaces
    %                           letters, digits, punctuation with no
    %                           rules as to format.
    %
    %   Key         <letter>    This is the key letter used to signify
    %                           the name of this level for the older
    %                           MECCA tokens [?below] [?above] [?line]
    %                           [?file] and so forth. By default, the
    %                           uppercased first letter from the access
    %                           name is used, but 'Key' is a means of
    %                           overriding this.
    %
    %   Time        <mins>      <mins> specifies the maximum number of
    %                           minutes that a user at this level may
    %                           be logged in per individual session.
    %                           There is no default.
    %
    %   Cume        <mins>      <mins> here specifies the maximum number
    %                           of minutes that a user at this level may
    %                           be logged in PER DAY across all sessions
    %                           for that day. There is no default.
    %
    %   Calls       <no>        <no> is the maximum number of times per
    %                           day that a user at this level is allowed
    %                           to log on. A value of -1 means unlimited,
    %                           although Cume time will of course still
    %                           apply.
    %
    %   LogonBaud   <baud>      This is the lowest baud rate that a user
    %                           at this level may use. The default is that
    %                           any baud rate is acceptable.
    %
    %   XferBaud    <baud>      This is the lowest baud rate that a user
    %                           at this level may used to download files.
    %                           The default is that a caller at any baud
    %                           rate is allowed to download.
    %
    %   FileLimit   <kbs>       This is the maximum number of kilobytes
    %                           that a user may download per day.
    %
    %   FileRatio   <amt>       <amt> specifies the ratio of dl:up that
    %                           a user must achieve in order to download
    %                           files. This download ratio only comes
    %                           into force when the user's download
    %                           reaches the amount specified for 'RatioFree'.
    %
    %   RatioFree   <kbs>       This is the amount which a user may download,
    %                           in kilobytes, before the FileRatio is 
    %                           applied.
    %
    %   UploadReward <value>
    %                           This item tells Maximus how much time
    %                           to give back to the user for uploading, as
    %                           a percentage.  For example, a value of
    %                           '100%' will add back only the same
    %                           amount of time the user spent
    %                           uploading.  In other words, the user
    %                           will still have the same time left
    %                           as before s/he started the upload.
    %                           If you wish to REWARD users for
    %                           uploading, then you can increase the
    %                           value above 100%.  For example, a
    %                           value of 200% will give the user two
    %                           extra seconds for every second s/he
    %                           spends uploading.  To not compensate
    %                           for uploads, set the reward to 0%.
    %
    %
    %   LoginFile   <filename>  <filename> is displayed to all users of this
    %                           class immediately after logging on.
    %                           If <filename> does not contain a path
    %                           specification, Maximus assumes that it is in
    %                           the Misc directory.
    %
    %   Flags       <words>     These flags apply to all users of a particular
    %                           class and usually specify if an action does or
    %                           does not apply. One or more 'Flags' lines may
    %                           be specified for a user class.
    %
    %                           Valid values for <words> are:
    %
    %                   Hangup          Hang up immediately, either on
    %                                   login AFTER the Loginfile has been
    %                                   displayed, or immediately should the
    %                                   user's class be modified while the
    %                                   user is on-line.
    %                   Hide            Automatically assumes 'don't display
    %                                   in userlist' for all users in this
    %                                   class, irrespective of the user flag
    %                                   setting.
    %                   ShowHidden      Allows this user to see all users
    %                                   in the userlist, regardless of the
    %                                   'don't display in user list' setting
    %                                   of each user and user class.
    %                   ShowAllFiles    Displays all files, even those which
    %                                   are normally hidden from view by the
    %                                   '@' character before the filename in
    %                                   a file area's files list.
    %                   DloadHidden     Allows a user to download files which
    %                                   are either hidden or not in the files
    %                                   list for a file area.
    %                   UploadAny       Allows a user to upload any file,
    %                                   bypassing checks for .bbs/.gbs/.rbs
    %                                   files and files listed in the
    %                                   BADFILES.BBS.
    %                   NoFileLimit     Allows a user in this class to download
    %                                   any number/amount of files with no
    %                                   limits checking.
    %                   NoTimeLimit     Disables time and input timeout
    %                                   checking for this user class.
    %                   NoLimits        Shorthand for both NoFileLimit and
    %                                   NoTimeLimit.
    %
    %   MailFlags    <words>    These flags affect entry of messages via
    %                           Maximus.
    %
    %                           Values values for <words> are:
    %
    %                   ShowPvt         Displays all messages in any area
    %                                   regardless of Private status or to
    %                                   whom the message is addressed or
    %                                   written by.
    %                   Editor          Provides access for users of this
    %                                   class to use the defined external
    %                                   message editor if one is defined
    %                                   for remote use.
    %                   LocalEditor     Provides access for users of this
    %                                   class to use the defined external
    %                                   message editor if one is defined
    %                                   for local use.
    %                   NetFree         Maximus does not charge for netmail
    %                                   for users in this class.
    %                   MsgAttrAny      Provides access for users of this
    %                                   class to change any message, or
    %                                   set any attribute (normally reserved
    %                                   only for sysop).
    %                   WriteRdOnly     Allows the user to post messages
    %                                   in an area defined as ReadOnly.
    %                   NoRealName      Prevents MAXIMUS from ever adding the
    %                                   ^aREALNAME: kludge into areas which
    %                                   are either anonymous or allow use of
    %                                   alias.
    %
    %   UserFlags   <value>     This is a an optional flag that allows the
    %                           sysop to define their own class access flags
    %                           for use in MEX scripts. Each 'bit' in the
    %                           value specified represents one of 32 possible
    %                           flags; eg. a value of 1 sets bit 0 ON and
    %                           all others OFF, a value of 127 sets bits 0
    %                           through 6 ON and all others off.
    %                           This value may be specified in decimal or
    %                           hexadecimal, the latter with the prefix
    %                           "0x" or "$".
    %
    %   Oldpriv     <value>     This is the privilege level used for
    %                           compatibility with Maximus 2.x.  Maximus 3.0
    %                           itself does not use this value.  If you are
    %                           adding extra privilege classes, beyond those
    %                           that came with the standard system, you should
    %                           copy the "Oldpriv" value from one of the
    %                           adjacent classes.
    %
    % End Access                End of current access level.

Access Transient
        Level           0
        Desc            Transient
        Alias           Twit
        Key             T
        Time            15
        Cume            15
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       0
        FileRatio       0
        RatioFree       250
        UploadReward    0
%       LoginFile       Misc\PrivT
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         -2
End Access

Access Demoted
        Level           10
        Desc            Demoted
        Alias           Disgrace
        Key             D
        Time            45
        Cume            60
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       500
        FileRatio       0
        RatioFree       500
        UploadReward    50
%       LoginFile       Misc\PrivD
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         0
End Access

Access Limited
        Level           20
        Desc            Limited
        Key             L
        Time            60
        Cume            90
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       1000
        FileRatio       0
        RatioFree       1000
        UploadReward    100
%       LoginFile       Misc\PrivL
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         1
End Access

Access Normal
        Level           30
        Desc            Normal
        Key             N
        Time            60
        Cume            90
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       1000
        UploadReward    100
%       LoginFile       Misc\PrivN
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         2
End Access

Access Worthy
        Level           40
        Desc            Worthy
        Key             W
        Time            60
        Cume            90
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       1000
        UploadReward    100
%       LoginFile       Misc\PrivW
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         3
End Access

Access Privil
        Level           50
        Desc            Privil
        Key             P
        Time            60
        Cume            90
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       2000
        UploadReward    100
%       LoginFile       Misc\PrivP
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         4
End Access

Access Favored
        Level           60
        Desc            Favored
        Key             F
        Time            60
        Cume            90
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       5000
        UploadReward    100
%       LoginFile       Misc\PrivF
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         5
End Access

Access Extra
        Level           70
        Desc            Extra
        Key             E
        Time            60
        Cume            90
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       8000
        UploadReward    100
%       LoginFile       Misc\PrivE
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         6
End Access

Access Clerk
        Level           80
        Desc            Clerk
        Key             C
        Time            90
        Cume            120
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       10000
        UploadReward    100
%       LoginFile       Misc\PrivC
%       Flags
%       MailFlags
%       UserFlags
        Oldpriv         7
End Access

Access AsstSysop
        Level           90
        Desc            AsstSysop
        Key             A
        Time            120
        Cume            180
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       5000
        FileRatio       0
        RatioFree       10000
        UploadReward    100
%       LoginFile       Misc\PrivA
        Flags           ShowHidden ShowAllFiles DloadHidden
        MailFlags       LocalEditor NetFree WriteRdOnly
%       UserFlags
        Oldpriv         8
End Access

Access Sysop
        Level           100
        Desc            Sysop
        Key             S
        Time            1440
        Cume            1440
        Calls           -1
        LogonBaud       300
        XferBaud        300
        FileLimit       30000
        FileRatio       0
        RatioFree       10000
        UploadReward    100
%       LoginFile       Misc\PrivS
        Flags           ShowHidden ShowAllFiles DloadHidden UploadAny NoFileLimit
        MailFlags       LocalEditor NetFree WriteRdOnly MsgAttrAny NoRealName
%       UserFlags
        Oldpriv         10
End Access

Access Hidden
        Level           65535
        Desc            Hidden
        Key             H
        Time            0
        Cume            0
        Calls           0
        LogonBaud       300
        XferBaud        1200
        FileLimit       0
        FileRatio       0
        RatioFree       0
        UploadReward    0
%       LoginFile       Misc\PrivH
        Flags           HangUp Hide
%       MailFlags
%       UserFlags
        Oldpriv         11
End Access


