%          ----------------------------------------------------
%          MENUS.CTL -- The menus control file for Maximus-CBCS
%          ----------------------------------------------------
%
% This file controls all of the aspects of Max's menus, including
% the actions performed by each option, the overall menu structure,
% and the appearance of each menu.
%
% Each menu definition is started by a "Menu" keyword.  Immediately after
% the word "Menu" should be the filename to use for that menu file.  The
% .MNU extension will be added automatically.
%
% The only two menu names that Maximus requires are "MAIN" and "EDIT".
% The MAIN menu is called as soon as a user has logged on, and the
% EDIT menu is called whenever the user enters the BORED message editor.
% 
% Other than those two, you are completely free to name your menus as
% you wish.  Since you define the links between menus, the names you
% use for your other menus is of no consequence.
%
% Also, you are completely free to put as many items on as many menus
% as you want, although it is recommended that you keep the general menu
% structure the same.  You can have up to a maximum of 127 separate menu
% options on each individual menu.

Menu MAIN
        %
        % This option defines what the name of the menu will look like
        % on-screen, to the user.  This doesn't necessarily have to be
        % the same name as you specified above.

        Title           MAIN (%t mins)

        % This next option defines a customized *.BBS file to display
        % _instead_ of the canned, Max-generated menu.  The file you
        % specify will be shown to callers who enter this menu whenever
        % the menu options would normally be displayed.  If no
        % file is given, Max will generate the menu on its own.
        %
        % This file can contain any of the special MECCA tokens that
        % you can use in a normal .MEC file.
        %
        % If you specify a MenuFile, it will be enabled for all help
        % levels by default.  However, if you wish to enable the
        % menu for certain help levels only (such as NOVICE and
        % REGULAR), simply add the names of the levels after the
        % filename.  For example, "MenuFile Misc\MenuMain Novice"
        % would show the MenuFle to novice callers only, and
        % the Max-generated menu would be shown to everyone else.
        % See the Max TechRef manual for more information on this
        % option.

%       MenuFile        Misc\MenuMain
        MenuFile        Rip\Max_Main RIP

        % If you are using the customized .BBS MenuFile, then you must
        % tell Maximus how many lines on your screen that your custom
        % file takes.  If this value is not specified, or if it is incorrect,
        % then you will have problems with messages being scrolled off the
        % top of your screen because of your customized menufile.

%       MenuLength      14

        % This option is also only applicable if you're using a custom
        % *.BBS menufile.  As expected, if a user presses a key in the
        % middle of the menu with HOTFLASH turned on, the menu will abort
        % it's display, and jump to the option selected.  However, if
        % your *.BBS file has a coloured background, this operation may
        % cause the background to bleed into the rest of the screen.  The
        % solution is to specify an AVATAR colour number here, which will
        % be displayed BEFORE printing the key the user selected.  Normally,
        % this colour should have the background set to black, or whatever
        % colour your screen normally is.  An index of the AVATAR colour
        % codes (and their corresponding colours) can be found in the
        % program documentation.

%       MenuColour      15

        % This section of the control file defines the available
        % commands for this menu.

        % The name of the COMMAND to perform is in the left column.
        % This tells Maximus what it should do when this option is
        % selected.  Examples of commands are "Goodbye",
        % "Display_File" and "Msg_Browse".
        %
        % Following the command is an optional argument; some
        % commands need arguments, but others don't.  If the argument
        % contains any spaces (such as would a DOS command line),
        % then make sure to replace them with underscores, which will
        % get translated back into spaces when the menu is read back
        % in.  The only common commands which use arguments are
        % Display_File, Display_Menu and MEX.
        %
        % After the argument is that command's access level.  This
        % access level consists of a privilege level and an optional
        % set of keys.  A user will only be able to select that
        % option if his/her priv level is equal to or higher than the
        % specified level.  In addition, if you wish to restrict a
        % command to users which carry certain keys, simply place a
        % slash and a list of key numbers/letters after the privilege
        % level.  For example, to restrict a command so that only
        % Normal-level users with keys 1 and C can access a command,
        % use a priv level of "Normal/1C".
        %
        % Following the priv level is the command description.  This
        % description will be used when creating NOVICE-level menus,
        % and the first letter of that command will be used as the
        % selection character, so make sure that each command has a
        % unique first character.

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

        Display_Menu    Message                 Demoted "Message areas"
        Display_Menu    File                    Demoted "File areas"
        Display_Menu    Change                  Demoted "Change setup"

        % The above three options tell Maximus to chain to a different
        % *.Mnu file, whose name is specified as an argument.  Please
        % make sure NOT to include a path before the name of your menu,
        % since Maximus will add that according to the currently-defined
        % menu path.

        Goodbye                               Transient "Goodbye (log off)"
        MEX             M\Stats                 Demoted "Statistics"
  NoDsp Press_Enter                             Demoted "S"
        Userlist                                Demoted "UserList"
        Version                                 Demoted "Version of BBS"

        Display_File    Misc\YellReq            Demoted "Yell for SysOp"
  NoDsp Yell                                    Demoted "Y"

        % The above options are fairly generic, and basically do what
        % they appear to do.

        % When selected, this option displays a *.BBS file to the user.
        % You can have as many of these custom files as you wish.

        Display_File    Misc\Bulletin           Demoted "Bulletins"

        % These commands display the off-line reader and SysOp menus.

        Display_Menu    Reader                  Demoted "Off-line reader"
        Display_Menu    Sysop                     Sysop "#Sysop menu"
        Display_Menu    MEX                       Sysop "$MEX Samples"

        % The next commands are only of use to multi-line systems.
        % PLEASE SEE THE DOCUMENTATION BEFORE USING THESE
        % COMMANDS!

        Who_Is_On                               Demoted "Who is On"
  NoDsp Press_Enter                             Demoted "W"
        Display_Menu    Chat                    Demoted "/Chat menu"

        % The following is the help file for the main menu.  Unless you
        % specify a help file, such as done below, users will NOT be able
        % to press `?' for help.

        Display_File    Hlp\Main                Demoted "?help"

End Menu


Menu MESSAGE
        Title           MESSAGE (%t mins)

        HeaderFile      :m\headmsg

        % To display a custom file instead of the canned Max menu, use
        % this:

%       MenuFile        Misc\MenuMsg
        MenuFile        RIP\Max_Msg RIP

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

  NoRIP Msg_Area                                Demoted "Area change"
    RIP MEX             M\Msgarea               Demoted "Area change"
  NoDsp Msg_Area                                Demoted "[" "["
  NoDsp Msg_Area                                Demoted "]" "]"
        Read_Next                               Demoted "Next message"
        Read_Previous                           Demoted "Previous message"
        Msg_Enter                               Demoted "Enter message"
        Msg_Reply                               Demoted "Reply to a message"
        Msg_Browse                              Demoted "Browse messages"
        Msg_Change                              Demoted "Change current msg"
        Read_Nonstop                            Demoted "=ReadNonStop"
        Read_Original                           Demoted "-ReadOriginal"
        Read_Reply                              Demoted "+ReadReply"
        Msg_Current                             Demoted "*ReadCurrent"
        Msg_List                                Demoted "List (brief)"
        Msg_Tag                                 Demoted "Tag areas"
        Display_Menu    Main                  Transient "Main menu"
        Display_Menu    File                  Transient "Jump to file areas"
        Goodbye                               Transient "Goodbye (log off)"
        Msg_Kill                                Demoted "Kill (delete) msg"
        Msg_Upload                              Demoted "Upload a message"
        Msg_Forward                             Demoted "Forward (copy)"
        Msg_Reply_Area  .                       Demoted "$Reply Elsewhere"
        Msg_Download_Attach                     Demoted "^Download Attaches"
        Msg_Hurl                                  Sysop "Hurl (move)"
        Msg_Xport                                 Sysop "Xport to disk"
        Msg_Edit_User                             Sysop "@Edit user"
        Msg_Kludges                               Sysop "!Toggle Kludges"
        Msg_Unreceive                             Sysop "#Unreceive Msg"
  NoDsp Same_Direction                          Demoted "|"
  NoDsp Read_Individual                         Demoted "0"
  NoDsp Read_Individual                         Demoted "1"
  NoDsp Read_Individual                         Demoted "2"
  NoDsp Read_Individual                         Demoted "3"
  NoDsp Read_Individual                         Demoted "4"
  NoDsp Read_Individual                         Demoted "5"
  NoDsp Read_Individual                         Demoted "6"
  NoDsp Read_Individual                         Demoted "7"
  NoDsp Read_Individual                         Demoted "8"
  NoDsp Read_Individual                         Demoted "9"
  NoDsp Msg_Change                              Demoted "`46"     ;<Alt-C>
  NoDsp Read_Previous                           Demoted "`75"     ;<Left>
  NoDsp Read_Original                           Demoted "`115"    ;<Ctrl-Left>
  NoDsp Read_Next                               Demoted "`77"     ;<Right>
  NoDsp Read_Reply                              Demoted "`116"    ;<Ctrl-Right>
  NoDsp Msg_Reply                               Demoted "`16"     ;<Alt-Q>
  NoDsp Msg_Reply                               Demoted "`19"     ;<Alt-R>
  NoDsp Msg_Kill                                Demoted "`37" "=" ;<Alt-K>

  Local Display_File    Hlp\Msg                 Demoted "?help"
 Matrix Display_File    Hlp\Mail                Demoted "?help"
   Echo Display_File    Hlp\Echo                Demoted "?help"
        %
End Menu

Menu FILE
        Title           FILE (%t mins)

        HeaderFile      :m\headfile
        MenuFile        RIP\Max_File RIP

        % To display a custom file instead of the canned Max menu, use
        % this:

%       MenuFile        Misc\MenuFile

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

  NoRIP File_Area                               Demoted "Area change"
    RIP MEX             M\FileArea              Demoted "Area change"
  NoDsp File_Area                               Demoted "[" "["
  NoDsp File_Area                               Demoted "]" "]"
        File_Locate                             Demoted "Locate a file"
        File_Titles                             Demoted "File titles"
        File_View                               Demoted "View text file"
        File_Download                           Demoted "Download (receive)"
        File_Upload                             Demoted "Upload (send)"
        MEX             M\Stats                 Demoted "Statistics"
  NoDsp Press_Enter                             Demoted "S"
        File_Contents                           Demoted "Contents (archive)"
        File_Tag                                Demoted "Tag (queue) files"
        File_NewFiles                           Demoted "New files scan"
        File_Raw                                  Sysop "Raw directory"
        File_Kill                                 Sysop "Kill file"
        File_Hurl                                 Sysop "Hurl (move)"
        File_Override                             Sysop "Override path"
        Display_Menu    Main                  Transient "Main menu"
        Display_Menu    Message               Transient "Jump to msg. areas"
        Goodbye                               Transient "Goodbye (log off)"
        Display_File    Hlp\FileHelp            Demoted "?help"
        %
End Menu

Menu CHANGE
        Title           CHANGE (%t mins)
        HeaderFile      :m\headchg

%       MenuFile        Misc\MenuChg

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

        Chg_Alias                                Hidden "Alias"
        Chg_Phone                               Demoted "#Telephone no."
        Chg_City                                Demoted "City"
        Chg_Password                            Demoted "Password"
        Chg_Help                                Demoted "Help level"
        Chg_Nulls                               Demoted "Nulls"
        Chg_Width                               Demoted "Width"
        Chg_Length                              Demoted "Length"
        Chg_Tabs                                Demoted "Tabs"
        Chg_Video                               Demoted "Video mode"
        Chg_RIP                                 Demoted "RIP Graphics"
        Chg_Editor                              Demoted "FullScrnEdit"
        Chg_Clear                               Demoted "Screen clear"
        Chg_IBM                                 Demoted "IBM characters"
        Chg_Hotkeys                             Demoted "!Hotkeys"
        Chg_Language                            Demoted "@Language"
        Chg_Userlist                            Demoted "%ShowInUserlist"
        Chg_Protocol                            Demoted "$Protocol default"
        Chg_Archiver                            Demoted "&Archiver default"
        Chg_FSR                                 Demoted "^FullScrnRead"
        Chg_More                                Demoted "*More Prompt"
        Display_Menu    Main                  Transient "Main Menu"
  NoDsp Display_Menu    Main                  Transient "Q"
  NoDsp Display_Menu    Main                  Transient "|"
        Display_File    Hlp\Change              Demoted "?help"
        %
End Menu

Menu EDIT
        Title           EDIT (%t mins)

%       MenuFile        Misc\MenuEdit

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

        Edit_Save                             Transient "Save message"
        Edit_Abort                            Transient "Abort message"
        Edit_List                             Transient "List message"
        Edit_Edit                             Transient "Edit line"
        Edit_Insert                           Transient "Insert line"
        Edit_Delete                           Transient "Delete line"
        Edit_Quote                            Transient "Quote message"
        Edit_Continue                         Transient "Continue"
        Edit_To                               Transient "To"
        Edit_Subj                             Transient "JsubJect"
        Edit_From                                 Sysop "From"
        Edit_Handling                             Sysop "Handling"
        Read_DiskFile                             Sysop "Read from disk"
        Display_File    Hlp\Editor            Transient "?help"
        %
End Menu


Menu CHAT
        Title           CHAT (%t mins)

        HeaderFile      :m\headchat
%       MenuFile        Misc\MenuChat

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

        Chat_CB                                 Demoted "CB chat"
        Chat_Page                               Demoted "Page user"
        Chat_Pvt                                Demoted "Answer page"
        Chat_Toggle                             Demoted "Toggle status"
        Display_Menu    MAIN                    Demoted "Main menu"
        Goodbye                                 Demoted "Goodbye (log off)"
        Display_File    Misc\ChatHelp           Demoted "?help"
End Menu


Menu READER
        Title           READER (%t mins)
        HeaderFile      Misc\OLR_Hdr

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

        Msg_Tag                                 Demoted "Tag area"
        Msg_Browse                              Demoted "Download new msgs" "tnp"
        Msg_Upload_QWK                          Demoted "Upload replies"
        Chg_Protocol                            Demoted "Protocol default"
        Chg_Archiver                            Demoted "Archiver default"
        Msg_Restrict                            Demoted "Restrict date"
        Display_Menu    Main                  Transient "Main menu"
        Goodbye                               Transient "Goodbye (log off)"
        Display_File    Hlp\OLR                 Demoted "?help"
  NoDsp Display_Menu    Main                  Transient "|"
End Menu


Menu MEX
        Title           MEX SAMPLES (%t mins)
        HeaderFile      Misc\MexInfo

        MEX             M\Userlist              Demoted "1 Userlist"
        MEX             M\Callers               Demoted "2 Callers"
        MEX             M\ChgDOB                Demoted "3 Change Birthdate"
        MEX             M\ChgSex                Demoted "4 Change Gender"
        MEX             M\Card                  Demoted "5 21-like game"
        MEX             M\Dir                   Demoted "6 Directory list"
        MEX             M\Dorinfo               Demoted "7 Generate DORINFO"
        MEX             M\Class                 Demoted "8 User class info."
        MEX             M\Lastrd                Demoted "9 Show lastread"
        MEX             M\Mexbank               Demoted "A Time bank"
        MEX             M\PStat                 Demoted "B PSTAT (OS2 only)"
        MEX             M\Mexchat               Demoted "C Full-screen chat"
        MEX             M\Taglist               Demoted "D Tagged file list"
        Goodbye                                 Demoted "Goodbye"
        Display_Menu    Main                    Demoted "Main menu"
End Menu

Menu SYSOP
        Title           SYSOP (%t mins)

        % Name of cmd    Optional arguments      Priv.   Command as it
        % to execute     for cmd, if any         Level   appears to user
        % ------------  --------------------- --------- -----------------

        % This command invokes the internal Maximus user editor.

        User_Editor                                Sysop "User editor"

        % This tells Maximus that you want to execute an external
        % program (in this case, COMMAND.COM) when the `O' menu option
        % is selected.  Other options are available, such as `Xtern_Dos'
        % (which can run .BAT or .CMD files) and `Xtern_Erlvl'
        % (which completely unloads Maximus from memory).
        %
        % Also, when specifying the command/errorlevel to execute,
        % don't forget to use underscores instead of spaces!
        %
        % Examples:
        %
        %   Xtern_Run   C:\Max\Bonk_/XN       Sysop "Nodelist editor"
        %   Xtern_Dos   D:\Path\Runme.Bat Transient "Run XYZ program"
        %   Xtern_Erlvl 45_Ongame.Bat       Demoted "On-line Games"
        %
        % If you're running 4DOS, replace the following
        % "Command.Com" with "4Dos.Com".
        %
        % OS/2 users should use the following statements instead:
        %
        % UsrRemote  Xtern_Run  maxpipe.exe_%p_cmd.exe    Sysop "OS shell"
        % UsrLocal   Xtern_Run  cmd.exe                   Sysop "OS shell"

UsrRemote Xtern_Run     Command.Com_>com%P_<com%P   Sysop "OS shell"
UsrLocal  Xtern_Run     Command.Com                 Sysop "OS shell"

        Display_Menu    Main                        Sysop "Main menu"
  NoDsp Display_Menu    Main                        Sysop "Q"
  NoDsp Display_Menu    Main                        Sysop "|"
End Menu

