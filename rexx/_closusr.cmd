/* rexx */

rc = UserFileClose(huf);

if rc <> 1 then
do
    say 'Error closing user file!'
    exit
end

