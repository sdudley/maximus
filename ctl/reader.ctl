Reader Section

        % This gives the path to COMPRESS.CFG.  This control file is
        % used to define the archiving and unarchiving programs
        % available on your system.  Currently, these programs are only
        % used when compressing and decompressing QWK bundles.

        % Max's COMPRESS.CFG uses the same format as Squish's
        % COMPRESS.CFG, so one copy of COMPRESS.CFG can be shared by
        % both Max and Squish.

        Archivers       Compress.Cfg

        % This should be an eight-character abbreviation
        % of your BBS name, since this is the filename
        % that Max will use when sending QWK packets.
        % This name shouldn't include any spaces.  Only
        % valid DOS filename characters are permitted.
        % (A-Z / 0-9 / !@#$%&()_)

        Packet Name     MAXIMUS

        % The name of a BLANK directory to use for off-line
        % reading purposes.  Max will create several
        % subdirectories below this one - but you should
        % *NOT* play with anything contained therein.

        Work Directory  Olr\

        % The phone number of your BBS.  This will be placed
        % in the downloaded packets.  It *should* be
        % in the specific format "(xxx) yyy-zzzz", since
        % that is what the off-line readers expect.  Max
        % itself doesn't care, and will copy this string
        % out to CONTROL.DAT verbatim, but some off-line
        % readers may have problems if it's a different
        % format or longer than it currently is.

        Phone Number    (123) 456-7890

        % The Max Messages keyword allows you to set a limit
        % on the number of messages which can be downloaded
        % during one Browse or Download session.  To set
        % no limit on the number of messages which can be
        % downloaded, comment out this keyword.  1200 messages
        % translates to a 400K download (compressed with LHarc).

        Max Messages    1200

End Reader Section


