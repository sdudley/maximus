/* rexx */

call _load.cmd
call _openusr.cmd

say ''

if arg(1)='' then
do
        say 'Usage:'
        say '   SETKEYS <keys>'
        say ''
        say 'SETKEYS will set the specified keys in all user records.'
        say ''
        say 'Example:'
        say ''
        say '   SETKEYS 12B'
        say ''
        say '   This will set keys 1, 2 and B in all user records.'

        exit
end

keys = arg(1)
count = 1

say 'Setting keys "'keys'" for all users:'

/* Now try to list a bunch of users */

huff=UserFileFindOpen(huf, '', '');

if huff=-1 then
        say 'User file is empty!'
else do
        do until UserFileFindNext(huff, '', '')=0

                countMod20 = trunc(((count / 20) - (count % 20)) * 20)

                if countMod20 = 0 then
                        say '    User 'count'... '

                count = count + 1

                usr.xkeys = usr.xkeys''arg(1)

                if (UserFileUpdate(huf, usr.name, usr.alias) <> 1) then
                        say 'Error updating user record for 'usr.name

        end

        rc=UserFileFindClose(huff);
end

call _closusr.cmd


