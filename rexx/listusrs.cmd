/* rexx */

call _load.cmd
call _openusr.cmd

/* Print a header for the user listing */

say ''
say 'Name                              City                      Priv     UL     DL'
say '--------------------------------- -------------------- --------- ------ ------'

/* Now try to list a bunch of users */

huff=UserFileFindOpen(huf, '', '');

if huff=-1 then
        say 'Error doing userfilefindopen!'
else do
        do until UserFileFindNext(huff, '', '')=0
            say left(usr.name,33) left(usr.city,20) right(usr.priv,9) right(to_kbytes(usr.up),6) right(to_kbytes(usr.down),6)
        end

        rc=UserFileFindClose(huff);
end

call _closusr.cmd


to_kbytes:
        amount = arg(1)

        if (amount > 1000) then
          do
            amount = trunc(amount / 1000,1)

            ret = amount'M'
          end
        else
          do
            ret = amount'K'
          end

        return ret

