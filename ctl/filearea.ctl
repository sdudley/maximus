%     -----------------------------------------------------------
%     FILEAREA.CTL -- a sample file area control file for Maximus
%     -----------------------------------------------------------
%
%
%  This file contains all of the information about your system's message
%  areas.  Each file area can be given a name.  This name consists
%  of letters, numbers, and any of the following characters:
%
%       . / _ ~ ! @ # $ % ^ & * ( ) - + = |
%
%  File area names can be up to nine numbers or letters long.  If you
%  wish, you can use numbers for area names, such that you'd have
%  area names like "1", "2", "3" and so forth.  However, it's usually
%  easier to give each area a short name which is easily remembered.

FileArea 0
        % ACS specifies the priv level required to access this
        % file area.
 
	ACS	Sysop
 
        % `Desc' specifies the name of the file area as it will
        % appear to the user.
 
	Desc	Unchecked uploads
 
        % `Download' specifies the DOWNLOAD path for this area.  This
        % directory should contain FILES.BBS and all of the files to
        % be made available.
 
	Download	File\Uncheck
 
        % `Upload' specifies the UPLOAD path for this area.  This directory
        % tells Max where to place files which are U)ploaded into this area.
        %
        % You will usually want to handle uploads one of two ways:
        %
        % 1) Validation required.  All "Upload" paths will point to the
        %    same, private directory.  This directory will have SysOp
        %    access only, and those files must be moved to the main
        %    file areas manually.
        %
        % 2) No validation required.  All "Upload" paths will point to
        %    the current "Download" path.  Files uploaded in this manner
        %    will be automatically placed in the file directory for
        %    each area, and users will be able to access those files
        %    right away.
        %
        % The default configuration uses the validation method, such that
        % all files will be uploaded to file area 0.
 
	Upload	File\Uncheck
 
 
        % This tells Maximus that the area is barricaded, and that
        % access can be controlled by the user's name or by a
        % password.  Please see the program documentation for more
        % information.
 
%       Barricade <menu_name> <barricade_file>
 
        % This tells Maximus to use an alternate file area menu
        % name while the user is in this area.  This can be used to
        % show certain options for one area only.
 
%       MenuName <orig_menu> <new_menu>
 
        % This option can be used to adjust the privilege level of
        % a menu option for one file area only.  Please see
        % the program documentation for more information.
 
%       Override <menu_name> <option_type> <ACS> [letter]
 
        % If the FILES.BBS-like catalog for this area does not
        % reside in the same directory as the files themselves,
        % you can specify the path and filename of the file
        % listing for this area.  This feature is especially
        % useful for CD-ROMs.
 
%       FileList        <file>
 
        % The 'Type' keyword can be used to select certain options
        % for this area.  Any of the following options can be
        % specified:
        %
        % Slow          File area is on a slow-access medium.  This
        %               tells Max and SILT to always assume that the
        %               area exists.
        %
        % Staged        Files from this area should be copied to the
        %               staging directory before initiating a file
        %               download. (See "Stage Path", below.)
        %
        % NoNew         This area is on a permanent storage medium
        %               and should be excluded from new file checks.
        %
        % CD            This area is on a CD-ROM.  This keyword
        %               enables all of the above options.  "Type CD"
        %               is equivalent to "Type Slow Staged NoNew".

%       Type CD

End FileArea

 
FileArea 1
        ACS             Demoted
        Desc            New Files
	Download	File\NewUp\
        Upload          File\Uncheck\
End FileArea

 
FileArea 2
        ACS             Demoted
        Desc            General Utilities
	Download	File\Util\
        Upload          File\Uncheck\
End FileArea

FileArea 3
        ACS             Demoted
        Desc            Maximus-related files
	Download	File\Max\
        Upload          File\Uncheck\
End FileArea

 
 
