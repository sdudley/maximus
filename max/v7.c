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
static char rcs_id[]="$Id: v7.c,v 1.3 2003/11/23 13:13:33 paltas Exp $";
#pragma on(unreferenced)

/*# name=Version 7 nodelist module
    credit=(C) Copyright 1987-91, Bit Bucket Software, a Delaware Corporation
*/

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "mm.h"
#include "v7.h"
#include "v7p.h"

#define get_nodelist_name(a) ;

static char unwrk[] = " EANROSTILCHBDMUGPKYWFVJXZQ-'0123456789";
#ifndef UNIX
static char nodelist_base[]="NODEX";
#else
static char nodelist_base[]="nodex";
#endif
static size_t namelen;

int V7FindNode(NETADDRP opus_addr, struct _newnode *node, char *net_info)
{
  long record;
  char index_filename[PATHLEN];

  node->NetNumber=node->NodeNumber=0;

  get_nodelist_name(opus_addr);           /* fill in nodelist.base  */
  *index_filename='\0';                   /* "null-terminated string*/
  
  strcpy(index_filename, net_info);       /* take nodelist path   */
  strcat(index_filename, nodelist_base);  /* add in the file name */
#ifndef UNIX
  strcat(index_filename, ".NDX");         /* add in the file ext  */
#else
  strcat(index_filename, ".ndx");         /* add in the file ext  */
#endif

  record=btree(index_filename, (void *)opus_addr, addr_compare);

  if (record == -1)
    return 0;
  else return get_ver7_info((dword)record, opus_addr, node, net_info);
}


int V7FindName(char *name, NETADDRP faddr, struct _newnode *node, char *net_info)
{
  char last_name_first[80];
  char index_filename[PATHLEN];
  char midname[80];
  char *c, *p, *m;
  long record;

  faddr->zone=faddr->net=faddr->node=faddr->point=(word)-1;

  /* Start of temp name buff   */
   
  c=midname;
   
  p=name;
  m=NULL;

  /* Find last name */
   
  while ((*c=*p++) != 0)
  {
    if (*c == ' ')
      m=c;

    c++;
  }

  if (m)
  {
    *m++='\0';                         /* Terminate the first half  */
    strcpy(last_name_first, m);        /* Now copy the last name    */
    strcat(last_name_first, ", ");     /* Insert a comma and space  */
    strcat(last_name_first, midname);  /* Finally copy first half   */
  }
  else strcpy(last_name_first, midname);/* Use whole name otherwise  */

  fancy_str(last_name_first);           /* Get caps in where needed  */
  namelen=(int)strlen(last_name_first);     /* Calc length now           */

  *index_filename='\0';                     /* "null-terminated string"  */

  strcpy(index_filename, net_info);     /* take nodelist path        */
#ifndef UNIX
  strcat(index_filename, "SYSOP.NDX");  /* add in the file name      */
#else
  strcat(index_filename, "sysop.ndx");  /* add in the file name      */
#endif

  record=btree(index_filename, (void *)last_name_first, name_compare);

  if (record==-1)
    return 0;

  return (get_ver7_info((dword)record, faddr, node, net_info));
}


int near addr_compare(void *key, void *desired, int len)
{
  int k;

  if ((k = ((NETADDRP)key)->zone - ((NETADDRP)desired)->zone) != 0)
    return (k);

  if ((k=((NETADDRP)key)->net - ((NETADDRP)desired)->net) != 0)
    return (k);

  if ((k = ((NETADDRP)key)->node - ((NETADDRP)desired)->node) != 0)
    return (k);

  if (len == 6)
    ((NETADDRP)key)->point=0;

  return ((NETADDRP)key)->point - ((NETADDRP)desired)->point;
} 


int near name_compare (void *key, void *desired, int len)
{
  int sc;
  NW(len);

  if ((sc=strnicmp((char *)key, (char *)desired, len)) != 0)
    return sc;

  return (strlen((char *)key) - strlen(desired));
}


static long near btree(char *filename, void *desired, int (near *compare)(void *key, void *desired, int len))
{
  int j, k, l;
  struct _ndx *nodeidx = NULL;
  struct _ndx *noderef = NULL;
  struct _IndxRef *ip = NULL;
  struct _LeafRef *lp = NULL;
  char aline[160];
  char *tp;
  char *np;

  long record, foundrec = -1L;
  int count;

  int stream;

  if ((stream = sopen(filename, O_RDONLY | O_BINARY | O_NOINHERIT, SH_DENYNO,
                      S_IREAD|S_IWRITE))==-1)
      return (-1L);                            /* no file, no work to do */

  if ((nodeidx=malloc(NDX_SIZE))==NULL ||
      (noderef=malloc(NDX_SIZE))==NULL)
  {
    if (nodeidx)
      free(nodeidx);

    close(stream);
    return -1L;
  }


  /* Get CtlRec */

  if (get7node (stream, 0L, noderef) != noderef)
  {
    free(noderef);
    free(nodeidx);
    close(stream);
    return -1L;
  }


  /* The guts of the matter -- walk from CtlRec to Leaf */

  record=noderef->ndx.CtlBlk.CtlRoot;

  /* Read the first Index node. */
  
  if (get7node(stream, 
               (unsigned long) (record * noderef->ndx.CtlBlk.CtlBlkSize),
               nodeidx) != nodeidx)
  {
    free(noderef);
    free(nodeidx);
    close (stream);
    return -1L;
  }


  /* Follow the node tree until we either match a key right in the index    *
   * node, or locate the leaf node which must contain the entry.            */

  while (nodeidx->ndx.INodeBlk.IndxFirst != -1)
  {
    if ((count=nodeidx->ndx.INodeBlk.IndxCnt) == 0)
    {
      j=0;
/*    free(noderef);
      free(nodeidx);
      close (stream);
      return (-1L);*/
    }
    else for (j=0; j < count; j++)    /* check 20 or less */
    {
      ip=&(nodeidx->ndx.INodeBlk.IndxRef[j]);
      tp=(char *) nodeidx + ip->IndxOfs;

      k=l=ip->IndxLen;

      for (np=aline; k > 0; k--)
        *np++=*tp++;

      *np='\0'; /*SJD Mon  07-06-1992  10:57:52 */

      k=(*compare)((void *)aline, desired, l);

      if (k > 0)
        break;

      if (k == 0)
      {

        /* Key matches in the index node. Since we're just doing lookup, we *
         * can assume its pointer is valid. If we were doing updates, that  *
         * assumption would not work, because leaf nodes update first. So   *
         * in an update environment, the entire code segment relating to    *
         * k == 0 should not execute, and we should walk the tree all the   *
         * way down.                                                        */

        free(noderef);
        free(nodeidx);
        close(stream);

        return (nodeidx->ndx.INodeBlk.IndxRef[j].IndxData);
      }
    }

    if (j == 0)
       record=nodeidx->ndx.INodeBlk.IndxFirst;
    else record=(nodeidx->ndx.INodeBlk.IndxRef[--j]).IndxPtr;

    if (get7node(stream, 
                (unsigned long) (record * noderef->ndx.CtlBlk.CtlBlkSize),
                nodeidx) != nodeidx)
    {
      free(noderef);
      free(nodeidx);
      close (stream);
      return (-1L);
    }
  }

  /* We can only get here if we've found the leafnode which contains our    *
   * entry.                                                                 */

  /* Find our guy here or die trying. */

  if ((count=nodeidx->ndx.LNodeBlk.IndxCnt) != 0)
  {
    /* Search for a higher key */

    for (j=0; j < count; j++) /* check 30 or less */
    {
      lp=&(nodeidx->ndx.LNodeBlk.LeafRef[j]);
      tp=(char *) nodeidx + lp->KeyOfs;

      k=l=lp->KeyLen;

      for (np=aline; k > 0; k--)
        *np++ = *tp++;

      *np='\0'; /*SJD Mon  07-06-1992  10:58:01 */

      k=(*compare)((void *)aline, desired, l);

      if (k > 0)
        break;
       
      if (k == 0)
      {
        foundrec=(nodeidx->ndx.LNodeBlk.LeafRef[j]).KeyVal;
        break;
      }
    }
  }

  free(noderef);
  free(nodeidx);
  close (stream);

  return (foundrec);
}


static int near get_ver7_info(unsigned long pos, NETADDRP faddr, struct _newnode *node, char *net_info)
{
  struct _vers7 vers7;
  char my_phone[40];
  char my_pwd[9];
  char aline[160];
  char aline2[160];
  char *fst;
  char temp[80];                                 /* we build file names here*/
  int stream, l;
  char *buf, *p;

  strcpy(temp, net_info);                     /* take nodelist path */
  strcat(temp, nodelist_base);                /* add in the filename*/
#ifndef UNIX
  strcat(temp, ".DAT");                       /* then the extension */
#else
  strcat(temp, ".dat");                       /* then the extension */
#endif
  
  if ((stream=sopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT, SH_DENYNO,
                    S_IREAD | S_IWRITE))==-1)
  {
    return 0;
  }

  if (lseek(stream, (long)pos, SEEK_SET) != (long)pos)
  {
    close(stream);
    return 0;
  }

  if (read(stream, (char *)&vers7, VER7_SIZE) != VER7_SIZE)
  {
    close(stream);
    return 0;
  }
  
  if ((buf=malloc(l=vers7.Phone_len+vers7.Password_len+vers7.pack_len))==NULL)
  {
    close(stream);
    return 0;
  }
  
  if (read(stream, buf, l) != l)
  {
    free(buf);
    close(stream);
    return 0;
  }

  close(stream);

  p=buf;

  memset(my_phone, '\0', 40);
  memset(my_pwd, '\0', 9);
  memset(aline, '\0', 160);
  memset(aline2, '\0', 160);

  memmove(my_phone, p, vers7.Phone_len);
  memmove(my_pwd, p += vers7.Phone_len, vers7.Password_len);
  memmove(aline2, p+vers7.Password_len, vers7.pack_len);

  free(buf);

  unpk(aline2, aline, vers7.pack_len);

  memset(node, 0, sizeof (struct _newnode));

  node->NetNumber  = vers7.Net;
  node->NodeNumber = vers7.Node;
  node->Cost       = vers7.MsgFee;
  node->RealCost = vers7.CallCost;
  memmove(node->SystemName, aline, min(33, vers7.Bname_len));
  
  if (vers7.Bname_len > 33)
    vers7.Bname_len=33;
  
  node->SystemName[vers7.Bname_len]='\0';
  fancy_str(node->SystemName);

  fst = &aline[0] + vers7.Bname_len + vers7.Sname_len;
  memcpy(node->PhoneNumber, my_phone, min(39, vers7.Phone_len));
  node->PhoneNumber[min(39,vers7.Phone_len)] = '\0';

  memcpy(node->MiscInfo, fst, min(29, vers7.Cname_len));
  node->MiscInfo[min(29, vers7.Cname_len)] = '\0';
  fancy_str( node->MiscInfo );

  memcpy (node->Password, my_pwd, min(8, vers7.Password_len));
  node->HubNode=vers7.HubNode;
  node->BaudRate=vers7.BaudRate;
  node->ModemType=vers7.ModemType;
  node->NodeFlags=vers7.NodeFlags;

  faddr->zone  = vers7.Zone;
  faddr->net   = vers7.Net;
  faddr->node  = vers7.Node;

  if (vers7.NodeFlags & B_point)
    faddr->point = vers7.HubNode;
  else faddr->point = 0;

  return 1;
}

static struct _ndx * near get7node(int stream, dword pos, struct _ndx *ndx)
{
  lseek (stream, (long) pos, SEEK_SET);

  if (read(stream, (char *)ndx, NDX_SIZE) != NDX_SIZE)
  {
    close(stream);
    return NULL;
  }
  
  return ndx;
}



static void near unpk(char *instr,char *outp,int count)
{
  struct chars
  {
    byte c1;
    byte c2;
  };

  union
  {
    unsigned short w1;
    struct chars d;
  } u;

  short i, j;
  char obuf[4];

  *outp='\0';

  while (count)
  {
    u.d.c1=*instr++;
    u.d.c2=*instr++;
    count -= 2;

    for (j=2; j >= 0; j--)
    {
      i=u.w1 % 40;
      u.w1 /= 40;
      
      obuf[j]=unwrk[i];
    }

    obuf[3]='\0';
    strcat(outp, obuf);
  }
}


