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

int MsgAttrToFlavour(dword attr);
NETADDR *SblistToNetaddr(struct _sblist *sb, NETADDR *n);
struct _sblist *NetaddrToSblist(NETADDR *n, struct _sblist *sb);
int NetmailArea(struct _cfgarea *ar);
int BadmsgsArea(struct _cfgarea *ar);
void Report_Statistics(void);
void Free_Outbuf(void);
void Flush_Outbuf(void);
void Zero_Statistics(void);
void Do_Echotoss(char *etname,void (*ScanCmd)(struct _cfgarea *ar,HAREA opensq), int bad_scan_all, char *scan_after);
void Alloc_Outbuf(void);
int BusyFileOpen(char *name, int wait);
void BusyFileClose(char * name);
void Parse_Areas(char *);
struct _cfgarea * Declare_Area(char *path, char *tag, char *nodes, word type, word flag);
void Initialize_Variables(void);
void HoleAddPacket(char *name,BLIST far *bl);
void HoleOpenList(void);
void HoleCloseList(void);
void Hole_Read_Netmail_Area(void);
void Hole_Free_Netmail_Area(void);
word Hole_Add_To_Net(NETADDR *,char *,int );
MATCHOUT * HoleMatchOutOpen(NETADDR *who,int type,byte flavour);
int HoleMatchOutNext(MATCHOUT *);
void HoleMatchOutClose(MATCHOUT *);
void Link_Messages(char *etname);
void cdecl _junk_cdecl_proc(void);

void Alloc_Buffer(unsigned size);
void Free_Buffer(void);
void SetPktOfs(long ofs);
long GetPktOfs(void);
char *Get_To_Nul(void *msg,unsigned int *length,int pktfile,int gp);
dword crcstr(dword crc, byte *s);
void DiskFull(struct _cfgarea *ar);
void ErrOpening(char *name,char *filename);
char *GetNextToken(byte *start, byte *buf, byte *token);
byte *GetTearPos(byte *msgbuf, byte **endtear);
int DestIsHere(struct _pktprefix *prefix);
int DestIsHereA(NETADDR *dest);
char *FixOutboundName(word zone);
void Fill_Out_Pkthdr(struct _pkthdr *ph,
                     struct _sblist *us,
                     word them_zone, word them_net, word them_node, word them_point);
void Cleanup(void);
int MatchNN(NETADDR *m1, NETADDR *m2, int ign_zone);
int MatchNS(NETADDR *m1, struct _sblist *m2, int ign_zone);
int MatchSS(struct _sblist *m1, struct _sblist *m2, int ign_zone);
int DupesArea(struct _cfgarea *ar);
int CallExtern(char *cmd, word archiver);
void FloName(byte *out, NETADDR *n, byte flavour, word addmode);
void Parse_Config(char *cfgname);
void Munge_Outbound_Area(byte *cfgname,byte *tag);
int BusyFileExist(NETADDR *n);
unsigned long get_unique_number(void);
word Adjust_Pkt_Header(struct _pkthdr *pkt);
void HoleScanHole(void);
int BusyFileOpenNN(NETADDR *n, int wait);
void BusyFileCloseNN(NETADDR *n);
void HoleMoveOut(void);
int Merge_Pkts(byte *from,byte *to);
void HoleRemoveFromList(char *name);
void Scan_To(struct _sblist *scanto, XMSG *msg, char *text, dword attr, struct _cfgarea *ar, struct _statlist *sl);
MATCHOUT * MatchOutOpen(NETADDR *who,int type,byte flavour);
int MatchOutNext(MATCHOUT *mo);
void MatchOutClose(MATCHOUT *mo);

void S_LogOpen(char *name);
void S_LogClose(void);
void _stdc S_LogMsg(char *format,...);
void cdecl OS2FAR S_LogLine(char OS2FAR *string);

void Toss_Messages(char *echotoss, word last_max, time_t start);
void Scan_Messages(char *etname, NETADDR *scan, time_t start);
extern void Pack_Messages(struct _cfgarea *nets);
void Scan_Area(struct _cfgarea *ar, HAREA opensq);
int NeedToEcho(byte *msgbuf);
struct _sblist *Digitize_Seenbys(struct _cfgarea *ar, byte *text, unsigned *sb_num);
struct _sblist *Sort_And_Flatten(struct _sblist *sb, unsigned num_sb, unsigned additional);
unsigned Scan_To_All(struct _cfgarea *ar, struct _sblist *sb, unsigned num_sb, XMSG *msg, byte *ctxt, byte **afterkludge, UMSGID uid, int *do_kill, HAREA sq, dword *hwm);
int Add_To_FloFile(byte *fmtxt,byte *from,byte *to);
void HoleInitHole(void);
void HoleDeinitHole(void);
void Hole_Nuke_Bundles(void);
void MakeOutboundName(NETADDR *d, char *s);
word InvalidDate(byte *da);
void HoleRename(char *from, char *to);
void FloToArc(void);
void ArcToFlo(void);
void KillArc(void);
short OutToSflo(void);
void HandleArcRet(int arcret, char *cmd);
void HandleAttReqPoll(word action, byte **toscan);
void StatsWriteBlock(word type, word len, word actual_len, void *data);
void StatsWriteAreas(void);
void StatsOpen(void);
void StatsClose(void);
void usage(void);
unsigned FlavourToMsgAttr(byte flavour);
void OS2FAR * EXPENTRY sq_palloc(size_t size);
void EXPENTRY sq_pfree(void OS2FAR *ptr);
void OS2FAR * EXPENTRY sq_repalloc(void OS2FAR *ptr, size_t size);
void far * EXPENTRY sq_farpalloc(size_t size);
void EXPENTRY sq_farpfree(void far *ptr);
void far * EXPENTRY sq_farrepalloc(void far *ptr, size_t size);
void MashMsgid(char *begin, dword *msgid_hash, dword *msgid_serial);
int ProcessACUpdate(HAREA sq, struct _cfgarea *ar, XMSG *msg, char *ctrl, int *do_kill, UMSGID uid, dword *hwm, unsigned added, int *pfSkipScan);

