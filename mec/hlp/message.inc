[rip]!|*
[endrip][moreoff]
[yellow]MESSAGE HELP: [lightcyan]Type the letter of the command that you need help with.

[yellow]A[lightcyan])rea Change       [yellow]N[lightcyan])ext Message
[yellow]R[lightcyan])eply to message  [yellow]=[lightcyan])Read Nonstop
[yellow]U[lightcyan])pload Message    [yellow]J[lightcyan])ump to Files
[yellow]K[lightcyan])ill Message      [yellow]E[lightcyan])nter Message
[yellow]P[lightcyan])rior Message     [yellow]+[lightcyan])Read Reply
[yellow]-[lightcyan])Read Original    [yellow]G[lightcyan])oodbye (log off)
[yellow]B[lightcyan])rowse Messages   [yellow]T[lightcyan])ag Areas
[yellow]*[lightcyan])Current Message  [yellow]F[lightcyan])orward Message
[yellow]$[lightcyan])Reply Elsewhere  [yellow]^[lightcyan])Download Attaches
[acs sysop][yellow]@[lightcyan])Edit User

[yellow]?[lightcyan]) General information about this area
[yellow]0[lightcyan]) [lightred]QUIT HELP

[white]Select: [gray menu]anper=-+lsigkufbtfj*@?$^0|

[choice]a[goto Area]
[choice]n[goto NextMsg]
[choice]p[goto PrevMsg]
[choice]e[goto EnterMsg]
[choice]r[goto Reply]
[choice]=[goto NonStop]
[choice]-[goto ReadOrig]
[choice]+[goto ReadReply]
[choice]g[goto Goodbye]
[choice]k[goto Kill]
[choice]u[goto Upload]
[choice]f[goto Forward]
[choice]b[goto browse]
[choice]t[goto tag]
[choice]j[goto jjump]
[choice]*[goto current]
[choice]$[goto replyelse]
[choice]^[goto dlatt]
[acs sysop][choice]@[goto edit_user]
[choice]?[goto Help]
[choice]0[quit]
[choice]|[quit]
[top]

[/dlatt yellow]        DOWNLOAD ATTACHES

        [white]This option allows you to download any file attaches
that other users may have sent to you.  Normally, Maximus will prompt
you to download the file when reading a message that contains an attach.

However, this command allows you to download of your attaches, regardless
of the area containing the original message. [goto Next]

[/replyelse yellow]        REPLY ELSEWHERE

        [white]This command allows you to reply to the current message
in a different message area.  Before entering the message header
information, Maximus will prompt you for the destination area of the
message.  When you save the reply, Maximus will place it in the
specified area. [goto Next]

[/Area yellow]        AREA CHANGE

        [white]The Area Change command lets you change to another message area.
The messages on this BBS are divided into categories, usually by subject.
Before entering a message, make sure that you are in the right message area,
since some special areas are sent via telephone at great cost to other
bulletin boards (these type of areas are usually called `EchoMail' areas),
and people may get upset if you leave an inappropriate message in the
wrong area. [goto Next]

[/NextMsg yellow]        READ NEXT MESSAGE

        [white]The Next Message command will display the next message in the
current area.  To keep reading messages in this direction, you can press
the ENTER key at the next prompt. [goto Next]

[/PrevMsg yellow]        READ PRIOR MESSAGE

        [white]The Prior Message command will display the previous message in the
current area.  To keep reading messages in this direction, you can press
the ENTER key at the next prompt. [goto Next]

[/EnterMsg yellow]        ENTER A MESSAGE

        [white]The Enter Message command allows you to send a message to someone
else on this system.  After you select this command, the BBS will prompt
you for some information it needs to know to send the message -- Such
as who the message is to, the subject of the message, and whether or not
the message is private.  After entering this information, then you will
be placed into one of the message editors; either the BORED line editor,
or the MaxEd full-screen editor.  (Your configuration in the Change Setup
menu will determine which editor is started.)  Once you're in the editor,
you can enter your message.  If you decide that you didn't really want to
send the message, you can choose the ABORT command; otherwise, you can
choose the SAVE command to write the message to disk. [goto Next]

[/Reply yellow]        REPLY TO A MESSAGE

        [white]The Reply command allows you to send a response to the author
whose message you read last.  The reply command is similar to the Enter
command, except some of the message fields will be filled in for you
(the author of the message you're replying to will be automatically inserted
into the "To:" field of the message you're writing), and you can agree
to those defaults by pressing ENTER.  Also, once in the editor, you can
also QUOTE the previous message, which helps to preserve message-thread
continuity. [goto Next]

[/Nonstop yellow]        READ NON-STOP

        [white]The Non-Stop Read command allows you to read all of the messages
in the current area (starting with the current message), without
pausing between each message.  This is useful if your terminal program has a
`capture' or `buffer' command, which will let you save the messages to disk
for later perusal. [goto Next]

[/ReadOrig yellow]        READ ORIGINAL

        [white]The Read Original command will display the message which the
current message is a reply of.  (You can tell if a message is a reply
to another, since there will be a "*** This is a reply to #xx" tag at the
bottom of the message.) [goto Next]

[/ReadReply yellow]        READ REPLY

        [white]The Read Reply command will display any replies to the current
message.  (You can tell if a message has any replies, since there will be
a "*** There is a reply.  See #xx" tag at the bottom of the message.) [goto Next]

[include GOODBYE.INC]

[/Kill yellow]        KILL A MESSAGE

        [white]The Kill command allows you to delete a message in the current area.
However, you can only kill messages which are TO or FROM you; you cannot
delete someone else's mail unless you are the SysOp. [goto Next]

[/Upload yellow]        UPLOAD A MESSAGE

        [white]The Upload command allows you to directly upload a message
to the current area.  This is identical to the Enter Message command,
except instead of invoking the editor, MaxEd will start an Xmodem upload.
Instead of typing in your message, you should tell your terminal to upload
the pre-typed file containing the message text.  This is much easier than
doing an ASCII upload into the BORED editor, since you don't have to fiddle
with pace characters, line delays, etc. [goto Next]

[/Forward yellow]        FORWARD

        [white]The Forward command allows you to make a copy of a message in the
current area, and send it to someone else.  Just type in the message number
to forward, and enter the name of the person you wish to send the message
to.  You can also forward a message directly into another area by typing
the area number when prompted. [goto Next]

[/browse yellow]        BROWSE

        [white]The Browse command allows you to perform many sophisticated
search and retrieval commands on the message base.  Browse can display
all new messages, search for keywords, pack messages into QWK bundles,
list messages one to a line, and more.  For more information, select
the Browse command and choose "?" at the first menu. [goto next]

[/tag yellow]        TAG AREAS

        [white]The Tag Areas command allows you to select a set of
messages areas which interest you.  The set of areas that you select
can be referenced through either the browse command or the QWK mail
packer.  Tagging and then browsing is a quick way of finding messages
that you're looking for. [goto next]

[/current yellow]        READ CURRENT

        [white]The Read Current option allows you to reread the current
message.  Maximus will display the message currently pointed to by
your lastread pointer.  The current forward/reverse reading direction
will not be modified by this command. [goto next]

[/edit_user yellow]        EDIT USER

        [white]This command allows you to invoke the user editor
based on the "From:" address of the current message.  When selecting
this command, Max will perform a search on the user who created the
message.  If this user is found, Max will display the user in the
user editor, ready for changes to be made.  This command is only
available to SysOps. [goto next]

[/jjump yellow]        JUMP TO FILES

        [white]This command will take you directly to the file
areas.  Instead of going back to the main menu and then selecting
F)iles, the Jump command allows you to directly switch between
the message and file sections. [goto next]

[/Next]

[white enter top]

