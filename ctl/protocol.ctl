% ------------
% PROTOCOL.CTL
% ------------
%
% This file is used to define external protocols usable with Max 2.0.
% Both Opus-compatible and non-standard protocols can be used
% directly by Maximus.
%
% If you're using a locked baud rate, THESE PROTOCOL ENTRIES MAY NEED
% TO BE MODIFIED.  You should replace all "%W"'s in this file with your
% locked baud rate (such as 19200 or 38400).
%
% Also, if you're running 4DOS (or any other alternate shell), you
% should replace all occurences of "command.com" with the name of
% your shell (such as 4dos.com).  Failure to do this may cause erratic
% results when calling external protocols.

%
% ASCII protocol.  This is the Opus-compatible OATE transfer module.
% To use, simply uncomment this protocol and place ASCII.EXE in
% your \MAX directory.
%

% Protocol ASCII
% ;       Type            Batch
% ;       Type            Errorlevel
% ;       Type            Bi
%         Type            Opus
%
%         LogFile         Ascii%K.Log
%         ControlFile     Ascii%K.Ctl
%         DownloadCmd     Ascii.Exe Ascii%K.Ctl -p%p -b%W -t%k -m%d -f%D -r%t
%         UploadCmd       Ascii.Exe Ascii%K.Ctl -p%p -b%W -t%k -m%d -f%D -r%t
%         DownloadString  Send %s
%         UploadString    Get %s
%         DownloadKeyword Sent
%         UploadKeyword   Got
%         FilenameWord    1
%         DescriptWord    4
% End Protocol


%
% Kermit protocol.  This is the Opus-compatible OKERMIT transfer module.
% To use, simply uncomment this protocol and place KERMIT.EXE in your
% \MAX directory.
%

% Protocol Kermit
%         Type            Batch
% ;       Type            Errorlevel
% ;       Type            Bi
%         Type            Opus
%
%         LogFile         Kermit%K.Log
%         ControlFile     Kermit%K.Ctl
%         DownloadCmd     Kermit.Exe -p%p -b%W -t%k -m%d -f%D -r%t Kermit%K.Ctl
%         UploadCmd       Kermit.Exe -p%p -b%W -t%k -m%d -f%D -r%t Kermit%K.Ctl
%         DownloadString  Send %s
%         UploadString    Get %s
%         DownloadKeyword Sent
%         UploadKeyword   Got
%         FilenameWord    1
%         DescriptWord    4
% End Protocol

%
% MPt protocol.  This is the protocol designed by MPt Systems.  To use,
% simply uncomment this protocol and place MPT.EXE in your \MAX directory.
%

% Protocol MPt
%         Type            Batch
% ;       Type            Errorlevel
% ;       Type            Bi
% ;       Type            Opus
%
%         LogFile         Mpt.Log
%         ControlFile     Mpt.Ctl
%         DownloadCmd     mpt A+ B- D+ F+ N+ P%P S0 M- W- HF Lmpt.log s @mpt.ctl
%         UploadCmd       mpt A+ B- D+ F+ N+ P%P S0 M- W- HF Lmpt.log r %v
%         DownloadString  %s
%         UploadString
%         DownloadKeyword S
%         UploadKeyword   R
%         FilenameWord    10
%         DescriptWord    0
% End Protocol

%
% Zmodem-90 MobyTurbo protocol.  To use, uncomment this protocol and
% place DSZ.EXE or DSZ.COM in your \MAX directory.
%
% NOTE!  You must create the following RDSZ.BAT in your Max directory
% to ensure that DSZ handles transfers correctly:
%
%       echo off
%       set dszlog=%1
%       dsz port %2 speed restrict %3 %4 %5 %6 %7 %8 %9
%       set dszlog=
%
% Without this batch file, protocol logging will not be enabled.
%
% WARNING!  A registered copy of DSZ is required to properly handle
% uploads.  Without a registered copy, uploads will be placed into
% your \MAX directory!
%

% Protocol DSZ/Moby
%         Type            Batch
% ;       Type            Errorlevel
% ;       Type            Bi
% ;       Type            Opus
%
%         LogFile         DSZ%K.Log
%         ControlFile     DSZ%K.Ctl
%         DownloadCmd     command.com /c rdsz DSZ%K.Log %P %W ha both sz -m -Z @%x:DSZ%K.Ctl
%         UploadCmd       command.com /c rdsz DSZ%K.Log %P %W ha both rz %v
%         DownloadString  %s
%         UploadString
%         DownloadKeyword z
%         UploadKeyword   Z
%         FilenameWord    10
%         DescriptWord    0
% End Protocol


% The BiModem protocol from Erik Labs.  To use, simply uncomment this
% protocol entry and place BIMODEM.COM and associated configuration
% files in the \MAX directory.
%
% WARNING!  Make sure that BiModem is NOT configured to send files
% from the current working directory, or else BiModem will create
% a security hole on your system.

% Protocol BiModem
%         Type            Batch
% ;       Type            Errorlevel
%         Type            Bi
% ;       Type            Opus
%
%         LogFile         Bimodem.Log
%         ControlFile     Bimodem.Ctl
%         DownloadCmd     bimodem /B %W /L %P /T %t /R %V /U @Bimodem.Ctl
%         UploadCmd       bimodem /B %W /L %P /T %t /R %V
%         DownloadString  %s
%         UploadString
%         DownloadKeyword " S "
%         UploadKeyword   " R "
%         FilenameWord    5
%         DescriptWord    0
% End Protocol


