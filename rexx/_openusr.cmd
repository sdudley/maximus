/* rexx */

/* Try to open the user file */

huf = UserFileOpen('user', '');

if huf = -1 then
do
        say 'Error opening user file!'
        exit
end

