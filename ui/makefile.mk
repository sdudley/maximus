.SOURCE.lib: $(LIBDIR)
.SOURCE.obs: $(COMP)$(MODE)\s
.SOURCE.obm: $(COMP)$(MODE)\m
.SOURCE.obc: $(COMP)$(MODE)\c
.SOURCE.obl: $(COMP)$(MODE)\l
.SOURCE.obf: $(COMP)$(MODE)
.SOURCE.obF: $(COMP)$(MODE)

CFLAGS   = $(C_DEBUG) $($(COMP)_STDOPS_SIZ) $(C_OUTDIR)$@

OBJS    :=      tui.obJ         tui_menu.obJ    tui_list.obJ    \
                tui_dlg.obJ     tui_mous.obJ    tui_win.obJ

.IF $(MODE)==p
OBJS += tuipmous.obJ
.ELSE
.IF $(MODE)==n
OBJS += tuipmous.obJ
.END
.END


.IF $(FLAT)==YES
$(COMP)$(MODE)tuif.lib: $(strip $(OBJS:s/J/f/))
.ELSE
all: $(COMP)$(MODE)tuis.lib $(COMP)$(MODE)tuim.lib $(COMP)$(MODE)tuic.lib $(COMP)$(MODE)tuil.lib

.IF $(COMP)==MC
$(COMP)$(MODE)tuis.lib: $(strip $(OBJS:s/J/S/))
$(COMP)$(MODE)tuim.lib: $(strip $(OBJS:s/J/M/))
$(COMP)$(MODE)tuic.lib: $(strip $(OBJS:s/J/C/))
$(COMP)$(MODE)tuil.lib: $(strip $(OBJS:s/J/L/))
.ELSE
$(COMP)$(MODE)tuis.lib: $(strip $(OBJS:s/J/s/))
$(COMP)$(MODE)tuim.lib: $(strip $(OBJS:s/J/m/))
$(COMP)$(MODE)tuic.lib: $(strip $(OBJS:s/J/c/))
$(COMP)$(MODE)tuil.lib: $(strip $(OBJS:s/J/l/))
.END

.END

$(OBJS:s/obJ/c/): tui.h

tui_dlg.obj: tui_dlg.h

rcs .PHONY:
        rin *.c *.h *.cc *.asm *.mk *.lnk


