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

#define N_INTRINFUNC (sizeof(_intrinfunc)/sizeof(_intrinfunc[0]))
#define StoreString(str, type, field, val) MexStoreStringAt(str + offsetof(type, field), val)

#define StoreByteString(str, type, field, val, size) MexStoreByteStringAt(str + offsetof(type, field), val, size)

#define REGS_ADDR regs_6
#define MAX_MEXFH 16

struct _mex_instance_stack    // Instance info for this invocation of MEX
{
  VMADDR vmaLinebuf;
  VMADDR vmaUser;
  VMADDR vmaMarea;
  VMADDR vmaFarea;
  VMADDR vmaMsg;
  VMADDR vmaID;
  VMADDR vmaSys;
  struct mex_sys *pms;
  struct mex_msg *pmm;
  struct mex_usr *pmu;
  struct mex_instancedata *pmid;
  struct _mex_instance_stack *next;
  HAFF hafFile;
  HAFF hafMsg;
  word fht[MAX_MEXFH];
  word fhCallers;
  HUF  huf;
  HUFF huff;
  long set_current;
  long set_last_msg;
  char szSetArea[MAX_ALEN];
#ifdef OBSOLETE
  word orig_timeremain;
  word orig_timeonline;
#endif
  sdword cbPriorMsg;
  sdword cbPriorFile;
} __attribute__((packed));


/* Definitions for the open() flags */

#define IOPEN_CREATE    0x01
#define IOPEN_READ      0x02
#define IOPEN_WRITE     0x04
#define IOPEN_RW        (IOPEN_READ|IOPEN_WRITE)
#define IOPEN_APPEND    0x08
#define IOPEN_BINARY    0x80

#define IOUTSIDE_RUN      0x01
#define IOUTSIDE_DOS      0x02
#define IOUTSIDE_REREAD   0x04

void StampToMexStamp(SCOMBO *psc, struct mex_stamp *pms);
void MexStampToStamp(struct mex_stamp *pms, SCOMBO *psc);
void MexExportUser(struct mex_usr *pusr, struct _usr *user);
void MexImportUser(struct mex_usr *pusr, struct _usr *user);
void MexImportData(struct _mex_instance_stack *pmis);
void MexExportData(struct _mex_instance_stack *pmis);

int MexAddFHandle(struct _mex_instance_stack *pmis, int fd);
int MexDelFHandle(struct _mex_instance_stack *pmis, int fd);

