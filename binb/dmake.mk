# Disable warnings for macros redefined here that were given
# on the command line.

__.SILENT   :=  $(.SILENT)
.SILENT     :=  yes

TMPDIR      :=  $(TEMP)

.NOTABS     := y

.IMPORT .IGNORE: TMPDIR SHELL COMSPEC

.IF $(SHELL)==$(NULL)
  SHELL   :=  $(COMSPEC)
.END

SHELLFLAGS  :=  /c
SHELLMETAS  :=  *"?<>

GROUPSHELL  :=  $(SHELL)
GROUPFLAGS  :=  $(SHELLFLAGS)

.IF $(HOSTMODE)==r
GROUPSUFFIX :=  .bat
.ELSE
GROUPSUFFIX :=  .cmd
.END

DIVFILE      =  $(TMPFILE:s,/,\\)

MAKE         =  $(MAKECMD) $(MFLAGS)

# Turn warnings back to previous setting.

.SILENT := $(__.SILENT)
