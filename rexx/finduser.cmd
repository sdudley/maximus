/* Sample REXX program demonstrating usage of the USER.BBS API.
 *
 * Usage:
 *
 *     finduser.cmd [<username>]
 *
 * Examples:
 *
 *      finduser                (with no arguments, displays info about all users)
 *      finduser Joe User       (with an argument, displays only the named user)
 */

call _load.cmd
call _openusr.cmd

if arg(1) <> '' then
        say 'Looking for "'arg(1)'"...'

huff=UserFileFindOpen(huf, arg(1), '');

if huff = -1 then
do
    say 'Could not find user "'arg(1)'" in user file.'
    rc=UserFileFindClose(huff);
    call _closusr.cmd
    exit
end

do until UserFileFindNext(huff, arg(1), '') = 0
        say '---------------------------------'
        say '     Name:' usr.name
        say '     City:' usr.city
        say '    Alias:' usr.alias
        say '    Phone:' usr.phone
        say '   Access:' usr.priv'/'usr.xkeys
end

rc=UserFileFindClose(huff)

call _closusr.cmd

