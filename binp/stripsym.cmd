/* rexx */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

file=''
args = arg(1)

do until fname=''
  parse var args fname args

  if fname <> '' then do

        /* Get list of all matching files */

        rc=SysFileTree(fname, 'file.', 'FO');

        do i=1 to file.0
                pos = lastpos('.', file.i);
                newname = substr(file.i, 1, pos-1)'.sym';
/*
                say 'name='file.i', newname='newname
*/

                exepos=pos('\exe\', newname)

                if exepos <> 0 then
                do
                        newname2=substr(newname, 1, exepos-1)'\sym\'substr(newname, exepos+5, length(newname)-exepos-5+1)
                        newname=newname2
                end

                'wstrip' file.i '.' newname

        end
  end
end

