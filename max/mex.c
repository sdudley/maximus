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

#pragma off(unreferenced)
static char rcs_id[]="$Id: mex.c,v 1.1.1.1 2002/10/01 17:52:23 sdudley Exp $";
#pragma on(unreferenced)

#define MEX_VM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <io.h>
#include "prog.h"
#include "mm.h"
#include "modem.h"
#include "mex.h"
#include "max_msg.h"
#include "max_file.h"
#include "md5.h"
#include "userapi.h"

#ifdef MEX

#include "vm.h"
#include "mex_max.h"
#include "mexp.h"
#include "mexint.h"

struct _mex_instance_stack *pmisThis=0;

/* Table of intrinsic functions.  These are all defined in mexint.c */

static struct _usrfunc _intrinfunc[]=
{
  {"__printCHAR",             intrin_printchar,               0},
  {"__printINT",              intrin_printint,                0},
  {"__printLONG",             intrin_printlong,               0},
  {"__printSTRING",           intrin_printstring,             0},
  {"__printUNSIGNED_CHAR",    intrin_printchar,               0},
  {"__printUNSIGNED_INT",     intrin_printunsignedint,        0},
  {"__printUNSIGNED_LONG",    intrin_printunsignedlong,       0},
  {"ansi_detect",             intrin_ansi_detect,             0},
  {"call_close",              intrin_call_close,              0},
  {"call_numrecs",            intrin_call_numrecs,            0},
  {"call_open",               intrin_call_open,               0},
  {"call_read",               intrin_call_read,               0},
  {"carrier",                 intrin_carrier,                 0},
  {"chat_querystatus",        intrin_ChatQueryStatus,         0},
  {"chatstart",               intrin_chatstart,               0},
  {"class_abbrev",            intrin_class_abbrev,            0},
  {"class_info",              intrin_class_info,              0},
  {"class_loginfile",         intrin_class_loginfile,         0},
  {"class_name",              intrin_class_name,              0},
  {"class_to_priv",           intrin_class_to_priv,           0},
  {"close",                   intrin_close,                   0},
  {"compressor_num_to_name",  intrin_CompressorNumberToName,  0},
  {"create_static_data",      intrin_create_static_data,      0},
  {"create_static_string",    intrin_create_static_string,    0},
  {"dcd_check",               intrin_dcd_check,               0},
  {"destroy_static_data",     intrin_destroy_static_data,     0},
  {"destroy_static_string",   intrin_destroy_static_string,   0},
  {"display_file",            intrin_display_file,            0},
  {"do_more",                 intrin_do_more,                 0},
  {"file_area",               intrin_file_area,               0},
  {"fileareafindclose",       intrin_fileareafindclose,       0},
  {"fileareafindfirst",       intrin_fileareafindfirst,       0},
  {"fileareafindnext",        intrin_fileareafindnext,        0},
  {"fileareafindprev",        intrin_fileareafindprev,        0},
  {"fileareaselect",          intrin_fileareaselect,          0},
  {"filecopy",                intrin_filecopy,                0},
  {"filedate",                intrin_filedate,                0},
  {"fileexists",              intrin_fileexists,              0},
  {"filefindclose",           intrin_filefindclose,           0},
  {"filefindfirst",           intrin_filefindfirst,           0},
  {"filefindnext",            intrin_filefindnext,            0},
  {"filesize",                intrin_filesize,                0},
  {"get_static_data",         intrin_get_static_data,         0},
  {"get_static_string",       intrin_get_static_string,       0},
  {"getch",                   intrin_getch,                   0},
  {"hstr",                    intrin_lang_heap_string,        0},
  {"input_ch",                intrin_input_ch,                0},
  {"input_list",              intrin_input_list,              0},
  {"input_str",               intrin_input_str,               0},
  {"iskeyboard",              intrin_iskeyboard,              0},
  {"issnoop",                 intrin_issnoop,                 0},
  {"itostr",                  intrin_itostr,                  0},
  {"kbhit",                   intrin_kbhit,                   0},
  {"keyboard",                intrin_keyboard,                0},
  {"language_num_to_name",    intrin_LanguageNumberToName,    0},
  {"localkey",                intrin_localkey,                0},
  {"log",                     intrin_log,                     0},
  {"long_to_stamp",           intrin_long_to_stamp,           0},
  {"lstr",                    intrin_lang_string,             0},
  {"ltostr",                  intrin_ltostr,                  0},
  {"mdm_command",             intrin_mdm_command,             0},
  {"mdm_flow",                intrin_mdm_flow,                0},
  {"menu_cmd",                intrin_menu_cmd,                0},
  {"msg_area",                intrin_msg_area,                0},
  {"msgareafindclose",        intrin_msgareafindclose,        0},
  {"msgareafindfirst",        intrin_msgareafindfirst,        0},
  {"msgareafindnext",         intrin_msgareafindnext,         0},
  {"msgareafindprev",         intrin_msgareafindprev,         0},
  {"msgareaselect",           intrin_msgareaselect,           0},
  {"open",                    intrin_open,                    0},
  {"privok",                  intrin_privok,                  0},
  {"prm_string",              intrin_prm_string,              0},
  {"protocol_num_to_name",    intrin_ProtocolNumberToName,    0},
  {"read",                    intrin_read,                    0},
  {"readln",                  intrin_readln,                  0},
  {"remove",                  intrin_remove,                  0},
  {"rename",                  intrin_rename,                  0},
  {"reset_more",              intrin_reset_more,              0},
  {"rip_detect",              intrin_rip_detect,              0},
  {"rip_hasfile",             intrin_rip_hasfile,             0},
  {"rip_send",                intrin_rip_send,                0},
  {"screen_length",           intrin_screen_length,           0},
  {"screen_width",            intrin_screen_width,            0},
  {"seek",                    intrin_seek,                    0},
  {"set_output",              intrin_set_output,              0},
  {"set_static_data",         intrin_set_static_data,         0},
  {"set_static_string",       intrin_set_static_string,       0},
  {"set_textsize",            intrin_set_textsize,            0},
  {"shell",                   intrin_shell,                   0},
  {"sleep",                   intrin_sleep,                   0},
  {"snoop",                   intrin_snoop,                   0},
  {"stamp_string",            intrin_stamp_string,            0},
  {"stamp_to_long",           intrin_stamp_to_long,           0},
  {"strfind",                 intrin_strfind,                 0},
  {"stridx",                  intrin_stridx,                  0},
  {"strlen",                  intrin_strlen,                  0},
  {"strlower",                intrin_strlower,                0},
  {"strpad",                  intrin_strpad,                  0},
  {"strpadleft",              intrin_strpadlf,                0},
  {"strridx",                 intrin_strridx,                 0},
  {"strtoi",                  intrin_strtoi,                  0},
  {"strtok",                  intrin_strtok,                  0},
  {"strtol",                  intrin_strtol,                  0},
  {"strupper",                intrin_strupper,                0},
  {"substr",                  intrin_substr,                  0},
  {"tag_dequeue_file",        intrin_tag_dequeue_file,        0},
  {"tag_get_name",            intrin_tag_get_name,            0},
  {"tag_queue_file",          intrin_tag_queue_file,          0},
  {"tag_queue_size",          intrin_tag_queue_size,          0},
  {"tell",                    intrin_tell,                    0},
  {"term_length",             intrin_term_length,             0},
  {"term_width",              intrin_term_width,              0},
  {"time",                    intrin_time,                    0},
  {"time_check",              intrin_time_check,              0},
  {"timeadjust",              intrin_timeadjust,              0},
  {"timeadjustsoft",          intrin_timeadjustsoft,          0},
  {"timeleft",                intrin_timeleft,                0},
  {"timeon",                  intrin_timeon,                  0},
  {"timestamp",               intrin_timestamp,               0},
  {"strtrim",                 intrin_trim,                    0},
  {"uitostr",                 intrin_uitostr,                 0},
  {"ultostr",                 intrin_ultostr,                 0},
  {"usercreate",              intrin_usercreate,              0},
  {"userfilesize",            intrin_userfilesize,            0},
  {"userfindclose",           intrin_userfindclose,           0},
  {"userfindnext",            intrin_userfindnext,            0},
  {"userfindopen",            intrin_userfindopen,            0},
  {"userfindprev",            intrin_userfindprev,            0},
  {"userfindseek",            intrin_userfindseek,            0},
  {"userremove",              intrin_userremove,              0},
  {"userupdate",              intrin_userupdate,              0},
  {"vidsync",                 intrin_vidsync,                 0},
  {"write",                   intrin_write,                   0},
  {"writeln",                 intrin_writeln,                 0},
  {"xfertime",                intrin_xfertime,                0}
};

/* EnterSymtabBlank
 *
 * This function creates an entry in the run-time symbol table
 * and allocates space for a particular symbol.  The memory allocated
 * is initialized to zero.
 */

static VMADDR EnterSymtabBlank(char *szName, int iSize)
{
  VMADDR vma;

  vma = MexEnterSymtab(szName, iSize);

  if (vma)
    memset(MexDSEG(vma), 0, iSize);

  return vma;
}

#if 0 /* unused */
void MexAddStr(IADDR * pia, char * str)
{
  *pia=MexStoreHeapByteString(str, strlen(str));
}
#endif

static void near MexStoreMarea(struct mex_marea * pma, PMAH pMah)
{
  MexKillStructString(mex_marea, pma, name);
  StoreString(MexPtrToVM(pma), struct mex_marea, name,        pMah->heap ? PMAS(pMah, name) : "");
  MexKillStructString(mex_marea, pma, descript);
  StoreString(MexPtrToVM(pma), struct mex_marea, descript,    pMah->heap ? PMAS(pMah, descript) : "");
  MexKillStructString(mex_marea, pma, path);
  StoreString(MexPtrToVM(pma), struct mex_marea, path,        pMah->heap ? PMAS(pMah, path) : "");
  MexKillStructString(mex_marea, pma, tag);
  StoreString(MexPtrToVM(pma), struct mex_marea, tag,         pMah->heap ? PMAS(pMah, echo_tag) : "");
  MexKillStructString(mex_marea, pma, attach_path);
  StoreString(MexPtrToVM(pma), struct mex_marea, attach_path, pMah->heap ? PMAS(pMah, attachpath) : "");
  MexKillStructString(mex_marea, pma, barricade);
  StoreString(MexPtrToVM(pma), struct mex_marea, barricade,   pMah->heap ? PMAS(pMah, barricade) : "");

  pma->division=(pMah->ma.attribs & MA_DIVBEGIN) ? 1 : (pMah->ma.attribs & MA_DIVEND) ? 2 : 0;
  pma->type=pMah->ma.type;
  pma->attribs=pMah->ma.attribs;
}

static void near MexStoreFarea(struct mex_farea * pfa, PFAH pFah)
{
  MexKillStructString(mex_farea, pfa, name);
  StoreString(MexPtrToVM(pfa), struct mex_farea, name,      pFah->heap ? PFAS(pFah, name) : "" );   
  MexKillStructString(mex_farea, pfa, descript);
  StoreString(MexPtrToVM(pfa), struct mex_farea, descript,  pFah->heap ? PFAS(pFah, descript) : "");
  MexKillStructString(mex_farea, pfa, downpath);
  StoreString(MexPtrToVM(pfa), struct mex_farea, downpath,  pFah->heap ? PFAS(pFah, downpath) : "");
  MexKillStructString(mex_farea, pfa, uppath);
  StoreString(MexPtrToVM(pfa), struct mex_farea, uppath,    pFah->heap ? PFAS(pFah, uppath) : "");  
  MexKillStructString(mex_farea, pfa, filesbbs);
  StoreString(MexPtrToVM(pfa), struct mex_farea, filesbbs,  pFah->heap ? PFAS(pFah, filesbbs) : "");
  MexKillStructString(mex_farea, pfa, barricade);
  StoreString(MexPtrToVM(pfa), struct mex_farea, barricade, pFah->heap ? PFAS(pFah, barricade) : "");

  pfa->division=(pFah->fa.attribs & FA_DIVBEGIN) ? 1 : (pFah->fa.attribs & FA_DIVEND) ? 2 : 0;
  pfa->attribs=pFah->fa.attribs;
}


/* Set-up function.  Called after MEX environment is set up
 * but before any user functions are called.
 */

int EXPENTRY intrin_setup(void)
{
  int i;
  struct _mex_instance_stack *pmis;

  /* Allocate memory for our instance stack */

  if ((pmis=malloc(sizeof(*pmis)))==NULL)
    return -1;

  memset(pmis, 0, sizeof *pmis);

  /* Create various structures in the global namespace */

  pmis->vmaLinebuf=MexStoreString("input", linebuf);

  pmis->vmaID    = EnterSymtabBlank("id",    sizeof(struct mex_instancedata));
  pmis->vmaUser  = EnterSymtabBlank("usr",   sizeof(struct mex_usr));
  pmis->vmaMarea = EnterSymtabBlank("marea", sizeof(struct mex_marea));
  pmis->vmaFarea = EnterSymtabBlank("farea", sizeof(struct mex_farea));
  pmis->vmaMsg   = EnterSymtabBlank("msg",   sizeof(struct mex_msg));
  pmis->vmaSys   = EnterSymtabBlank("sys",   sizeof(struct mex_sys));

  /* Fill out the USER structure */

  pmis->pmu=MexDSEG(pmis->vmaUser);
  memset(pmis->pmu, '\0', sizeof(struct mex_usr));
  MexExportUser(pmis->pmu, &usr);

  /* Fill out the mex_instancedata structure */

  pmis->pmid=MexDSEG(pmis->vmaID);
  pmis->pmid->instant_video=1;
  pmis->pmid->task_num=task_num;
  pmis->pmid->local=local;

  #if defined(NT)
    pmis->pmid->port = local ? 0 : (word)ComGetHandle(hcModem);
  #elif defined(OS_2)
    pmis->pmid->port = ComGetFH(hcModem);
  #else
    pmis->pmid->port = port+1;
  #endif

  pmis->pmid->speed=baud;
  pmis->pmid->alias_system=!!(prm.flags & FLAG_alias);
  pmis->pmid->ask_name=!!(prm.flags & FLAG_ask_name);
  pmis->pmid->use_umsgid=!!(prm.flags2 & FLAG2_UMSGID);

  /* Fill out the mex_sys structure */

  pmis->pms = (struct mex_sys *)MexDSEG(pmis->vmaSys);
  pmis->pms->current_row = current_line;
  pmis->pms->current_col = current_col;
  pmis->pms->more_lines = display_line;

  /* Fill out the "marea" structure */

  MexStoreMarea(MexDSEG(pmis->vmaMarea), &mah);

  /* Fill out the "farea" structure */

  MexStoreFarea(MexDSEG(pmis->vmaFarea), &fah);

  /* Fill out the "msg" structure */

  pmis->pmm=MexDSEG(pmis->vmaMsg);

  if (!sq)
    memset(pmis->pmm, 0, sizeof *pmis->pmm);
  else
  {
    pmis->set_current = pmis->pmm->current = UIDnum(last_msg);
    pmis->pmm->num=MsgGetNumMsg(sq);
    pmis->pmm->high=UIDnum(MsgGetHighMsg(sq));
  }
  pmis->pmm->direction=(direction==DIRECTION_NEXT);

  /* Init file handle table */

  for (i=0; i < MAX_MEXFH; ++i)
    pmis->fht[i]=(word)-1;

  /* Set File/Message area search handles to NULL */

  pmis->hafFile=0;
  pmis->hafMsg=0;

  pmis->fhCallers=(word)-1;
  pmis->huf=NULL;
  pmis->huff=NULL;

  /* Push onto instance stack */

  pmis->next=pmisThis;
  pmisThis=pmis;

  return 0;
}


/*
 * MexImportData
 *
 * Routine to import data back from the MEX data space into the structures
 * used by the Maximus proper.
 */

void MexImportData(struct _mex_instance_stack *pmis)
{
  MexImportUser(pmis->pmu, &usr);
  SetUserName(&usr, usrname);
  Set_Lang_Alternate(hasRIP());
  Find_Class_Number();
  MexImportString(linebuf, pmis->vmaLinebuf, BUFLEN);
}

/*
 * MexExportData
 *
 * Routine to export data from the Maximus proper into the MEX data
 * space.  This routine can be called multiple times during a MEX
 * session so that the BBS's data is synchronized with that of the
 * MEX program.
 *
 * For the first-time call used to set up these data structures, see
 * intrin_setup().
 */

void MexExportData(struct _mex_instance_stack *pmis)
{
  MexExportString(pmis->vmaLinebuf, linebuf);
  MexExportUser(pmis->pmu, &usr);
}


/* Export the current user into the MEX data space */

void MexExportUser(struct mex_usr *pusr, struct _usr *user)
{
  char szBirthDate[25];

#define StoreStringUser(field, val) StoreString(MexPtrToVM(pusr), struct mex_usr, field, val)
#define StoreByteStringUser(field, val, size) StoreByteString(MexPtrToVM(pusr), struct mex_usr, field, val, size)

  MexKillStructString(mex_usr, pusr, name);
  StoreStringUser(name,   user->name);
  MexKillStructString(mex_usr, pusr, city);
  StoreStringUser(city,   user->city);
  MexKillStructString(mex_usr, pusr, alias);
  StoreStringUser(alias,  user->alias);
  MexKillStructString(mex_usr, pusr, phone);
  StoreStringUser(phone,  user->phone);
  pusr->lastread_ptr=user->lastread_ptr;

  MexKillStructString(mex_usr, pusr, pwd);

#ifdef CANENCRYPT
  if (user->bits & BITS_ENCRYPT)
    StoreByteStringUser(pwd, user->pwd, MD5_SIZE);
  else
#endif
    StoreStringUser(pwd, user->pwd);

  pusr->times=user->times;
  pusr->call=user->call;
  pusr->help=user->help;
  pusr->video=user->video;
  pusr->nulls=user->nulls;
  pusr->hotkeys=!!(user->bits & BITS_HOTKEYS);
  pusr->notavail=!!(user->bits & BITS_NOTAVAIL);
  pusr->fsr=!!(user->bits & BITS_FSR);
  pusr->nerd=!!(user->bits & BITS_NERD);
  pusr->noulist=!!(user->bits & BITS_NOULIST);
  pusr->tabs=!!(user->bits & BITS_TABS);
  pusr->encrypted=!!(user->bits & BITS_ENCRYPT);
  pusr->badlogon=!!(user->bits2 & BITS2_BADLOGON);
  pusr->ibmchars=!!(user->bits2 & BITS2_IBMCHARS);
  pusr->bored=!!(user->bits2 & BITS2_BORED);
  pusr->more=!!(user->bits2 & BITS2_MORE);
  pusr->configured=!!(user->bits2 & BITS2_CONFIGURED);
  pusr->cls=!!(user->bits2 & BITS2_CLS);
  pusr->rip=!!(user->bits & BITS_RIP);
  pusr->priv=user->priv;
  MexKillStructString(mex_usr, pusr, dataphone);
  StoreStringUser(dataphone,  user->dataphone);
  pusr->time=user->time;
  pusr->deleted=!!(user->delflag & UFLAG_DEL);
  pusr->permanent=!!(user->delflag & UFLAG_PERM);
  pusr->msgs_posted=user->msgs_posted;
  pusr->msgs_read=user->msgs_read;
  pusr->width=user->width;
  pusr->len=user->len;
  pusr->credit=user->credit;
  pusr->debit=user->debit;
  pusr->xp_priv=user->xp_priv;
  StampToMexStamp(&user->xp_date, &pusr->xp_date);
  pusr->xp_mins=user->xp_mins;
  pusr->expdate=!!(user->xp_flag & XFLAG_EXPDATE);
  pusr->expmins=!!(user->xp_flag & XFLAG_EXPMINS);
  pusr->expdemote=!!(user->xp_flag & XFLAG_DEMOTE);
  pusr->expaxe=!!(user->xp_flag & XFLAG_AXE);
  pusr->sex=user->sex;
  StampToMexStamp(&user->ludate, &pusr->ludate);
  MexKillStructString(mex_usr, pusr, xkeys);
  StoreStringUser(xkeys,  Keys(user->xkeys));
  pusr->lang=user->lang;
  pusr->def_proto=user->def_proto;
  pusr->up=user->up;
  pusr->down=user->down;
  pusr->downtoday=user->downtoday;
  MexKillStructString(mex_usr, pusr, msg);
  StoreStringUser(msg,    user->msg);
  MexKillStructString(mex_usr, pusr, files);
  StoreStringUser(files,  user->files);
  pusr->compress=user->compress;

  /* Construct the user's date of birth in an easy-to-use format */

  sprintf(szBirthDate, "%04u.%02u.%02u",
          user->dob_year, user->dob_month, user->dob_day);

  MexKillStructString(mex_usr, pusr, dob);
  StoreStringUser(dob, szBirthDate);

  StampToMexStamp(&user->date_1stcall, &pusr->date_1stcall);
  StampToMexStamp(&user->date_pwd_chg, &pusr->date_pwd_chg);
  pusr->nup=user->nup;
  pusr->ndown=user->ndown;
  pusr->ndowntoday=user->ndowntoday;
  pusr->time_added=user->time_added;
  pusr->point_credit=user->point_credit;
  pusr->point_debit=user->point_debit;
  StampToMexStamp(&user->date_newfile, &pusr->date_newfile);
}



/* Import a user from the MEX data space into the usr struct */

void MexImportUser(struct mex_usr *pusr, struct _usr * user)
{
  char xkeys[PATHLEN];
  char szBirthDate[25];
  int yy, mm, dd;

  #define GetStringUser(usrvar, mex_field) MexStringCopy(usrvar, &pusr->mex_field, sizeof(usrvar))

  GetStringUser(user->name, name);
  GetStringUser(user->city, city);
  GetStringUser(user->alias, alias);
  GetStringUser(user->phone, phone);
  user->lastread_ptr=pusr->lastread_ptr;

#ifdef CANENCRYPT
  if (pusr->encrypted)
  {
    char *szSrc;

    szSrc = MexFetch(FormString, &pusr->pwd);
    memcpy(user->pwd, szSrc + sizeof(word), sizeof user->pwd);
  }
  else
#endif
    GetStringUser(user->pwd, pwd);

  user->times=pusr->times;
  user->help=pusr->help;
  user->video=pusr->video;
  user->nulls=pusr->nulls;

  user->bits=0;
  user->bits2=0;

  if (pusr->hotkeys)
    user->bits |= BITS_HOTKEYS;

  if (pusr->notavail)
    user->bits |= BITS_NOTAVAIL;

  if (pusr->fsr)
    user->bits |= BITS_FSR;

  if (pusr->nerd)
    user->bits |= BITS_NERD;

  if (pusr->noulist)
    user->bits |= BITS_NOULIST;

  if (pusr->tabs)
    user->bits |= BITS_TABS;

  if (pusr->encrypted)
    user->bits |= BITS_ENCRYPT;

  if (pusr->badlogon)
    user->bits2 |= BITS2_BADLOGON;

  if (pusr->ibmchars)
    user->bits2 |= BITS2_IBMCHARS;

  if (pusr->bored)
    user->bits2 |= BITS2_BORED;

  if (pusr->more)
    user->bits2 |= BITS2_MORE;

  if (pusr->configured)
    user->bits2 |= BITS2_CONFIGURED;

  if (pusr->cls)
    user->bits2 |= BITS2_CLS;

  if (pusr->rip)
  {
    user->bits  |= (BITS_RIP|BITS_HOTKEYS|BITS_FSR);
    user->bits2 |= BITS2_CLS;
  }

  user->priv=pusr->priv;

  GetStringUser(user->dataphone, dataphone);
  user->time=pusr->time;

  user->delflag=0;

  if (pusr->deleted)
    user->delflag |= UFLAG_DEL;

  if (pusr->permanent)
    user->delflag |= UFLAG_PERM;

  user->msgs_posted=pusr->msgs_posted;
  user->msgs_read=pusr->msgs_read;
  user->width=pusr->width;
  user->len=pusr->len;
  user->credit=pusr->credit;
  user->debit=pusr->debit;
  user->xp_priv=pusr->xp_priv;
  MexStampToStamp(&pusr->xp_date, &user->xp_date);
  user->xp_mins=pusr->xp_mins;

  user->xp_flag = 0;

  if (pusr->expdate)
    user->xp_flag |= XFLAG_EXPDATE;

  if (pusr->expmins)
    user->xp_flag |= XFLAG_EXPMINS;

  if (pusr->expdemote)
    user->xp_flag |= XFLAG_DEMOTE;

  if (pusr->expaxe)
    user->xp_flag |= XFLAG_AXE;

  user->sex=pusr->sex;
  MexStampToStamp(&pusr->ludate, &user->ludate);

  strcpy(xkeys, "/");
  GetStringUser(xkeys+1, xkeys);
  user->xkeys=SZKeysToMask(xkeys);

  user->lang=pusr->lang;
  user->def_proto=pusr->def_proto;
  user->up=pusr->up;
  user->down=pusr->down;
  user->downtoday=pusr->downtoday;

  user->compress=pusr->compress;

  /* Now parse the user's birthdate string */

  MexStringCopy(szBirthDate, &pusr->dob, sizeof(szBirthDate));

  if (sscanf(szBirthDate, "%u.%u.%u", &yy, &mm, &dd)==3)
  {
    user->dob_year=yy;
    user->dob_month=mm;
    user->dob_day=dd;
  }

  MexStampToStamp(&pusr->date_1stcall, &user->date_1stcall);
  MexStampToStamp(&pusr->date_pwd_chg, &user->date_pwd_chg);
  user->nup=pusr->nup;
  user->ndown=pusr->ndown;
  user->ndowntoday=pusr->ndowntoday;
  user->time_added=pusr->time_added;
  user->point_credit=pusr->point_credit;
  user->point_debit=pusr->point_debit;
  MexStampToStamp(&pusr->date_newfile, &user->date_newfile);

  GetStringUser(user->msg, msg);
  GetStringUser(user->files, files);
}




/* Termination function -- called when Mex execution of program
 * finishes.
 */

void EXPENTRY intrin_term(short *psRet)
{
  int i;
  struct _mex_instance_stack *pmis;
  char szOldMsg[MAX_ALEN];
  char szOldFile[MAX_ALEN];


  NW(psRet);

  /* Retrieve the current instance information for this copy of mex */

  pmis=pmisThis;
  pmisThis=pmisThis->next;

  if (pmis->fhCallers != (word)-1)
    close(pmis->fhCallers);

  if (pmis->huf!=NULL)
  {
    if (pmis->huff!=NULL)
      UserFileFindClose(pmis->huff);
    UserFileClose(pmis->huf);
  }

  /* Close search handles if still open */

  if (pmis->hafFile)
    AreaFileFindClose(pmis->hafFile);
  if (pmis->hafMsg)
    AreaFileFindClose(pmis->hafMsg);

  /* Make sure all files are closed */

  for (i=0; i < MAX_MEXFH; ++i)
    if (pmis->fht[i]!=(word)-1)
      close(pmis->fht[i]);

  /* Now copy back the input buffer and other global information */

  MexStringCopy(linebuf, MexDSEG(pmis->vmaLinebuf), BUFLEN);


  /* Handle changes to usr.msg and usr.files */

  strcpy(szOldMsg, usr.msg);
  strcpy(szOldFile, usr.files);


  /* Import the current user structure */

  MexImportUser(pmis->pmu, &usr);

  /* Handle changes to the lastread pointer and reading direction */

  if (pmis->set_current != pmis->pmm->current)
  {
    last_msg=pmis->pmm->current;

    if (sq && (prm.flags2 & FLAG2_UMSGID))
      last_msg=MsgUidToMsgn(sq, last_msg, UID_NEXT);
  }


  /* Try to change the current message area */

  if (!eqstri(szOldMsg, usr.msg))
  {
    BARINFO bi;

    memset(&bi, 0, sizeof bi);

    if (!PopPushMsgArea(usr.msg, &bi))
      strcpy(usr.msg, szOldMsg);
  }


  /* Try to change the current file area */

  if (!eqstri(szOldFile, usr.files))
  {
    BARINFO bi;

    memset(&bi, 0, sizeof bi);

    if (!PopPushFileArea(usr.files, &bi))
      strcpy(usr.files, szOldFile);
  }


  direction=(pmis->pmm->direction) ? DIRECTION_NEXT : DIRECTION_PREVIOUS;

  baud=pmis->pmid->speed;

  /* Free this stack entry */

  free(pmis);
}



/* Routine used for writing MEX error messages to the Maximus log */

void _stdc MexLog(char *szStr, ...)
{
  char szLogLine[PATHLEN*2];
  va_list va;

  va_start(va, szStr);
  vsprintf(szLogLine, szStr, va);
  va_end(va);

  logit(szLogLine);
}


/* Intrinsic MEX startup function -- just call the DLL MexExecute function */

int MexStartupIntrin(char *pszFile, char *pszArgs, dword fFlag)
{
  static int cMexInvoked=0;   /* Recursion count for this function */
  int save_inchat;
  int rc=-1;

  save_inchat = inchat;

  /* Update the screen before calling MEX */

  vbuf_flush();

  /* Make sure that MEX is not invoked recursively */

  if (cMexInvoked)
    logit(log_mex_no_reentrancy);
  else
  {
    cMexInvoked++;

    rc=MexExecute(pszFile, pszArgs, fFlag, N_INTRINFUNC, _intrinfunc,
                  intrin_setup, intrin_term, MexLog,
                  intrin_hook_before, intrin_hook_after);

    cMexInvoked--;
  }

  inchat = save_inchat;

  return rc;
}




/* Invoke the MEX interpreter on the specified file */

int Mex(char *file)
{
  char *dup;
  char *p;
  char *pszArgs;
  int rc;

  if ((dup=strdup(file))==NULL)
  {
    logit(mem_none);
    return -1;
  }

  /* Convert underscores to spaces */

  while ((p=strchr(dup, '_')) != NULL)
    *p=' ';

  /* Separate the command into filename and arguments */

  if ((p=firstchar(dup, " ", 2))==NULL)
    pszArgs="";
  else
  {
    p[-1]=0;
    pszArgs=p;
  }

  rc=MexStartupIntrin(dup, pszArgs, 0 /*VMF_DEBEXE | VMF_DEBHEAP*/);

  free(dup);

  return rc;
}

void StampToMexStamp(SCOMBO *psc, struct mex_stamp *pms)
{
  pms->date.day=psc->msg_st.date.da;
  pms->date.month=psc->msg_st.date.mo;
  pms->date.year=psc->msg_st.date.yr;

  pms->time.hh=psc->msg_st.time.hh;
  pms->time.mm=psc->msg_st.time.mm;
  pms->time.ss=psc->msg_st.time.ss;
}

void MexStampToStamp(struct mex_stamp *pms, SCOMBO *psc)
{
  psc->msg_st.date.da=pms->date.day;
  psc->msg_st.date.mo=pms->date.month;
  psc->msg_st.date.yr=pms->date.year;

  psc->msg_st.time.hh=pms->time.hh;
  psc->msg_st.time.mm=pms->time.mm;
  psc->msg_st.time.ss=pms->time.ss;
}


#endif /* MEX */

