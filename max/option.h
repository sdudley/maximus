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

/*# name=Menu server enumerations and structures
*/

#ifndef OPTION_H_DEFINED__
#define OPTION_H_DEFINED__

/* Enumeration `option' -- All possible values for menu.option[x].type */

typedef enum
{
  nothing,

  MISC_BLOCK=100, display_menu, display_file, message, file, other,
                  o_press_enter, key_poke, clear_stacked, o_if,
                  o_menupath, o_cls, mex, link_menu, o_return,


  XTERN_BLOCK=200, xtern_erlvl, xtern_dos, xtern_run, xtern_chain,
                   xtern_concur,

  MAIN_BLOCK=300, goodbye, statistics, o_yell, userlist, o_version,
                  user_editor, leave_comment, climax,

  MSG_BLOCK=400, same_direction, read_next, read_previous,
                 enter_message, msg_reply, read_nonstop,
                 read_original, read_reply, msg_list, msg_scan,
                 msg_inquir, msg_kill, msg_hurl, forward, msg_upload,
                 xport, read_individual, msg_checkmail, msg_change,
                 msg_tag, msg_browse, msg_current, msg_edit_user,
                 msg_upload_qwk, msg_toggle_kludges, msg_unreceive,
                 msg_restrict, msg_area, msg_track, msg_dload_attach,
                 msg_reply_area,

  FILE_BLOCK=500, locate, file_titles, file_type, upload, download, raw,
                  file_kill, contents, file_hurl, override_path,
                  newfiles, file_tag, file_area,

  /* Options generally found on the Change Setup menu */

  CHANGE_BLOCK=600, chg_city, chg_password, chg_help, chg_nulls,
                    chg_width, chg_length, chg_tabs, chg_more,
                    chg_video, chg_editor, chg_clear, chg_ibm,
                    chg_phone, chg_realname, chg_hotkeys,
                    chg_language, chg_userlist, chg_protocol,
                    chg_fsr, chg_archiver, chg_rip,

  EDIT_BLOCK=700, edit_save, edit_abort, edit_list, edit_edit,
                  edit_insert, edit_delete, edit_continue, edit_to,
                  edit_from, edit_subj, edit_handling, read_diskfile,
                  edit_quote,

  /* Stuff that was hacked on after the original implementation */

  CHAT_BLOCK=800, who_is_on, o_page, o_chat_cb, chat_toggle, o_chat_pvt,
    
  END_BLOCK,


  /* Everything below here is RESERVED by Maximus for future uses!         *
   * Also, everything ABOVE is fairly stable.  If changes have to be made, *
   * the old options above will NOT be re-used.  For example, if the       *
   * `edit_insert' command should become obsoleted for some reason, that   *
   * slot would either get retired and do nothing, or perform the NEW      *
   * edit_insert function.                                                 */

  rsvd=32766  /* This was stuck in to make sure that the `option'          *
               * enumeration uses a word, instead of a byte, in case we    *
               * really expand this structure sometime soon.               */

} option;


#ifdef SILT

struct _st {
    option opt;
    char *token;
};

#ifdef SILT_INIT

struct _st silt_table[] = {
  {msg_reply_area,  "msg_reply_area"},
  {msg_dload_attach,"msg_download_attach"},
  {msg_track      , "msg_track"},
  {link_menu      , "link_menu"},
  {o_return       , "return"},
  {mex            , "mex"},
  {msg_restrict   , "msg_restrict"},
  {climax         , "climax"},
  {msg_toggle_kludges, "msg_kludges"},
  {msg_unreceive  , "msg_unreceive"},
  {msg_upload_qwk , "msg_upload_qwk"},
  {chg_archiver   , "chg_archiver"},
  {msg_edit_user  , "msg_edit_user"},
  {chg_fsr        , "chg_fsr"},
  {msg_current    , "msg_current"},
  {msg_browse     , "msg_browse"},
  {chg_userlist   , "chg_userlist"},
  {chg_protocol   , "chg_protocol"},
  {msg_tag        , "msg_tag"},
  {chg_language   , "chg_language"},
  {file_tag       , "file_tag"},
  {o_chat_cb      , "chat_cb"},
  {o_chat_pvt     , "chat_pvt"},
  {chg_hotkeys    , "chg_hotkeys"},
  {msg_change     , "msg_change"},
  {chat_toggle    , "chat_toggle"},
  {o_page         , "chat_page"},
  {o_menupath     , "menupath"},
  {display_menu   , "display_menu"},
  {display_file   , "display_file"},
  {xtern_erlvl    , "xtern_erlvl"},
  {xtern_dos      , "xtern_dos"},
  {xtern_dos      , "xtern_os2"},
  {xtern_dos      , "xtern_shell"},
  {xtern_run      , "xtern_run"},
  {xtern_chain    , "xtern_chain"},
  {xtern_concur   , "xtern_concur"},
  {key_poke       , "key_poke"},
  {clear_stacked  , "clear_stacked"},
  {goodbye        , "goodbye"},
/*{statistics     , "statistics"},*/
  {o_yell         , "yell"},
  {userlist       , "userlist"},
  {o_version      , "version"},
  {msg_area       , "msg_area"},
  {file_area      , "file_area"},
  {same_direction , "same_direction"},
  {read_next      , "read_next"},
  {read_previous  , "read_previous"},
  {enter_message  , "msg_enter"},
  {msg_reply      , "msg_reply"},
  {read_nonstop   , "read_nonstop"},
  {read_original  , "read_original"},
  {read_reply     , "read_reply"},
  {msg_list       , "msg_list"},
  {msg_scan       , "msg_scan"},
  {msg_inquir     , "msg_inquire"},
  {msg_kill       , "msg_kill"},
  {msg_hurl       , "msg_hurl"},
  {forward        , "msg_forward"},
  {msg_upload     , "msg_upload"},
  {xport          , "msg_xport"},
  {read_individual, "read_individual"},
  {msg_checkmail  , "msg_checkmail"},
  {locate         , "file_locate"},
  {file_titles    , "file_titles"},
  {file_type      , "file_view"},
  {upload         , "file_upload"},
  {download       , "file_download"},
  {raw            , "file_raw"},
  {file_kill      , "file_kill"},
  {contents       , "file_contents"},
  {file_hurl      , "file_hurl"},
  {chg_city       , "chg_city"},
  {chg_password   , "chg_password"},
  {chg_help       , "chg_help"},
  {chg_nulls      , "chg_nulls"},
  {chg_width      , "chg_width"},
  {chg_length     , "chg_length"},
  {chg_tabs       , "chg_tabs"},
  {chg_more       , "chg_more"},
  {chg_video      , "chg_video"},
  {chg_editor     , "chg_editor"},
  {chg_clear      , "chg_clear"},
  {chg_ibm        , "chg_ibm"},
  {chg_rip        , "chg_rip"},
  {edit_save      , "edit_save"},
  {edit_abort     , "edit_abort"},
  {edit_list      , "edit_list"},
  {edit_edit      , "edit_edit"},
  {edit_insert    , "edit_insert"},
  {edit_delete    , "edit_delete"},
  {edit_continue  , "edit_continue"},
  {edit_to        , "edit_to"},
  {edit_from      , "edit_from"},
  {edit_subj      , "edit_subj"},
  {edit_handling  , "edit_handling"},
  {o_cls          , "cls"},
  {user_editor    , "user_editor"},
  {override_path  , "file_override"},
  {who_is_on      , "who_is_on"},
  {read_diskfile  , "read_diskfile"},
  {edit_quote     , "edit_quote"},
  {chg_phone      , "chg_phone"},
  {chg_realname   , "chg_realname"},
  {chg_realname   , "chg_alias"},
  {leave_comment  , "leave_comment"},
  {message        , "message"},
  {file           , "file"},
  {other          , "other"},
  {newfiles       , "file_newfiles"},
  {o_if           , "if"},
  {o_press_enter  , "press_enter"}
};

int silt_table_size=sizeof(silt_table)/sizeof(silt_table[0]);

#else
  extern int silt_table_size;
  extern struct _st silt_table[];
#endif /* SILT_INIT */
#endif /* SILT */

#endif /* OPTION_H_DEFINED__ */

