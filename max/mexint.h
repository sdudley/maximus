/*
 * Maximus Version 3.02
 * Copyright 1989, 2002 by Lanius Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __MEXINT_H_DEFINED
  #define __MEXINT_H_DEFINED

  typedef struct _mexargs
  {
    IADDR last;
    word arg_size;
  } __attribute__((packed)) MA, *PMA;


  /* Helper macro for destroying structure-based strings */

  #define MexKillStructString(stype, pstr, field) \
            _MexKillStructString(pstr,offsetof(struct stype, field))


  word EXPENTRY intrin_create_static_string(void);
  word EXPENTRY intrin_set_static_string(void);
  word EXPENTRY intrin_get_static_string(void);
  word EXPENTRY intrin_destroy_static_string(void);
  word EXPENTRY intrin_create_static_data(void);
  word EXPENTRY intrin_set_static_data(void);
  word EXPENTRY intrin_get_static_data(void);
  word EXPENTRY intrin_destroy_static_data(void);
  word EXPENTRY intrin_printstring(void);
  word EXPENTRY intrin_strlen(void);
  word EXPENTRY intrin_printlong(void);
  word EXPENTRY intrin_printint(void);
  word EXPENTRY intrin_printchar(void);
  word EXPENTRY intrin_printunsignedlong(void);
  word EXPENTRY intrin_printunsignedint(void);
  word EXPENTRY intrin_printunsignedchar(void);
  word EXPENTRY intrin_time(void);
  word EXPENTRY intrin_timestamp(void);
  word EXPENTRY intrin_kbhit(void);
  word EXPENTRY intrin_getch(void);
  word EXPENTRY intrin_localkey(void);
  word EXPENTRY intrin_vidsync(void);
  word EXPENTRY intrin_timeleft(void);
  word EXPENTRY intrin_timeon(void);
  word EXPENTRY intrin_timeadjustsoft(void);
  word EXPENTRY intrin_timeadjust(void);
  word EXPENTRY intrin_sleep(void);
  word EXPENTRY intrin_open(void);
  word EXPENTRY intrin_read(void);
  word EXPENTRY intrin_readln(void);
  word EXPENTRY intrin_write(void);
  word EXPENTRY intrin_writeln(void);
  word EXPENTRY intrin_seek(void);
  word EXPENTRY intrin_tell(void);
  word EXPENTRY intrin_close(void);
  word EXPENTRY intrin_itostr(void);
  word EXPENTRY intrin_ltostr(void);
  word EXPENTRY intrin_uitostr(void);
  word EXPENTRY intrin_ultostr(void);
  word EXPENTRY intrin_strtoi(void);
  word EXPENTRY intrin_strtol(void);
  word EXPENTRY intrin_strpad(void);
  word EXPENTRY intrin_strpadlf(void);
  word EXPENTRY intrin_strfind(void);
  word EXPENTRY intrin_stridx(void);
  word EXPENTRY intrin_strridx(void);
  word EXPENTRY intrin_substr(void);
  word EXPENTRY intrin_shell(void);

  word EXPENTRY intrin_ProtocolNumberToName(void);
  word EXPENTRY intrin_CompressorNumberToName(void);
  word EXPENTRY intrin_LanguageNumberToName(void);
  word EXPENTRY intrin_ChatQueryStatus(void);
  word EXPENTRY intrin_prm_string(void);
  word EXPENTRY intrin_log(void);
  word EXPENTRY intrin_stamp_string(void);
  word EXPENTRY intrin_menu_cmd(void);
  word EXPENTRY intrin_tag_queue_file(void);
  word EXPENTRY intrin_tag_dequeue_file(void);
  word EXPENTRY intrin_tag_queue_size(void);
  word EXPENTRY intrin_tag_get_name(void);
  word EXPENTRY intrin_reset_more(void);
  word EXPENTRY intrin_do_more(void);
  word EXPENTRY intrin_display_file(void);
  word EXPENTRY intrin_input_str(void);
  word EXPENTRY intrin_input_ch(void);
  word EXPENTRY intrin_input_list(void);
  word EXPENTRY intrin_stamp_to_long(void);
  word EXPENTRY intrin_long_to_stamp(void);
  word EXPENTRY intrin_set_output(void);
  word EXPENTRY intrin_lang_string(void);
  word EXPENTRY intrin_lang_heap_string(void);
  word EXPENTRY intrin_term_width(void);
  word EXPENTRY intrin_term_length(void);
  word EXPENTRY intrin_screen_width(void);
  word EXPENTRY intrin_screen_length(void);
  word EXPENTRY intrin_set_textsize(void);
  word EXPENTRY intrin_rip_send(void);
  word EXPENTRY intrin_rip_hasfile(void);
  word EXPENTRY intrin_ansi_detect(void);
  word EXPENTRY intrin_rip_detect(void);
  word EXPENTRY intrin_keyboard(void);
  word EXPENTRY intrin_iskeyboard(void);
  word EXPENTRY intrin_snoop(void);
  word EXPENTRY intrin_issnoop(void);

  word EXPENTRY intrin_fileareafindfirst(void);
  word EXPENTRY intrin_fileareafindnext(void);
  word EXPENTRY intrin_fileareafindprev(void);
  word EXPENTRY intrin_fileareafindclose(void);
  word EXPENTRY intrin_fileareaselect(void);
  word EXPENTRY intrin_file_area(void);

  word EXPENTRY intrin_msgareafindfirst(void);
  word EXPENTRY intrin_msgareafindnext(void);
  word EXPENTRY intrin_msgareafindprev(void);
  word EXPENTRY intrin_msgareafindclose(void);
  word EXPENTRY intrin_msgareaselect(void);
  word EXPENTRY intrin_msg_area(void);

  word EXPENTRY intrin_rename(void);
  word EXPENTRY intrin_remove(void);
  word EXPENTRY intrin_filecopy(void);
  word EXPENTRY intrin_fileexists(void);
  word EXPENTRY intrin_filesize(void);
  word EXPENTRY intrin_filedate(void);

  word EXPENTRY intrin_filefindfirst(void);
  word EXPENTRY intrin_filefindnext(void);
  word EXPENTRY intrin_filefindclose(void);

  word EXPENTRY intrin_strtok(void);
  word EXPENTRY intrin_strupper(void);
  word EXPENTRY intrin_strlower(void);
  word EXPENTRY intrin_trim(void);

  word EXPENTRY intrin_time_check(void);
  word EXPENTRY intrin_dcd_check(void);
  word EXPENTRY intrin_mdm_command(void);
  word EXPENTRY intrin_mdm_flow(void);
  word EXPENTRY intrin_carrier(void);

  word EXPENTRY intrin_class_info(void);
  word EXPENTRY intrin_privok(void);
  word EXPENTRY intrin_class_abbrev(void);
  word EXPENTRY intrin_class_name(void);
  word EXPENTRY intrin_class_loginfile(void);
  word EXPENTRY intrin_class_to_priv(void);

  word EXPENTRY intrin_chatstart(void);

  word EXPENTRY intrin_xfertime(void);

  word EXPENTRY intrin_call_open(void);
  word EXPENTRY intrin_call_close(void);
  word EXPENTRY intrin_call_numrecs(void);
  word EXPENTRY intrin_call_read(void);

  word EXPENTRY intrin_userfindopen(void);
  word EXPENTRY intrin_userfindnext(void);
  word EXPENTRY intrin_userfindprev(void);
  word EXPENTRY intrin_userfindclose(void);
  word EXPENTRY intrin_userfilesize(void);
  word EXPENTRY intrin_userupdate(void);
  word EXPENTRY intrin_usercreate(void);
  word EXPENTRY intrin_userremove(void);
  word EXPENTRY intrin_userfindseek(void);

  void _MexKillStructString(void *pstr, int increment);
  void MexReturnStringBytes(char *s, int len);
  void MexReturnString(char *s);
  void MexArgBegin(PMA pma);
  word MexArgGetWord(PMA pma);
  dword MexArgGetDword(PMA pma);
  void * MexArgGetRef(PMA pma);
  char * MexArgGetString(PMA pma, int fPassByRef);
  char * MexArgGetRefString(PMA pma, IADDR *pia, word *pwLen);
  char * MexArgGetNonRefString(PMA pma, IADDR *pia, word *pwLen);
  byte MexArgGetByte(PMA pma);
  word MexArgEnd(PMA pma);
  void MexStringCopy(char *dest, IADDR *piSrc, int max_len);
  void MexImportString(char *szDest, VMADDR vma, int iMax);
  void MexExportString(VMADDR vma, char *szSrc);
  void EXPENTRY intrin_hook_before(void);
  void EXPENTRY intrin_hook_after(void);


  extern struct _mex_instance_stack *pmisThis;

#endif /* __MEXINT_H_DEFINED */

