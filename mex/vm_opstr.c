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

/* $Id: vm_opstr.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

#define COMPILING_MEX_VM
#include <string.h>
#include "prog.h"
#include "vm.h"

int op_scopy(INST *inst, struct _args *arg)
{
  byte *s1, *s2;
  word s1len;
  IADDR new;

  /* Get a pointer to the original string */
  
  s1=fetch(FormString, &arg->a1);
  

  /* Grab the length word from up front */
  
  s1len=*(word *)s1;
  

  /* Increment the string pointer past that word */
  
  s1 += sizeof(word);

  
  /* Create a new address to hold the new string */
  
  new.segment=SEG_GLOBAL;

  
  /* Allocate memory for it on the heap */
  
  if ((new.offset=hpalloc(sizeof(word)+s1len))==END_HEAP)
    vm_err(err_no_ss);
  
  new.indirect=FALSE;


  /* Now find the address of the allocated location */

  s2=pbDs+new.offset;

  
  /* Copy the length of the string */
  
  *(word *)s2=s1len;
  
  
  /* And copy the string itself. */
  
  memmove(s2+sizeof(word), s1, s1len);


  /* Now, free any string that the assigned-to string may have contained,   *
   * since we're overwriting its contents, we should free anything it       *
   * pointed to so that we don't chew up memory in the heap.                */

  /* (only kill it if it's a heap object) */

  if (arg->a2.segment==SEG_GLOBAL && 
      arg->a2.offset >= vmh.lGlobSize + vmh.lStackSize &&
      (inst->arg2.addr.segment != SEG_TEMP ||
       inst->arg2.addr.indirect))
  {
    /* The last line in the 'if' is needed cause a temp register will never
     * have a prior, unsaved string in it.  Strings in temps will always
     * be SKILLed before we try to assign something to them, so doing it
     * here would be redundant.  However, if we are using a temporary
     * register as an index, we still need to kill the destination.
     */

    kill_str(&arg->a2, &inst->arg2.addr);
  }

  
  /* Finally, update the destination and point the copied-to string to the  *
   * new string text.                                                       */

  store(&inst->arg2.addr, FormAddr, &new);
  return 0;
}



int op_scat(INST *inst, struct _args *arg)
{
  byte *s1, *s2, *sto;
  word s1len, s2len, maxlen;
  IADDR new;

  /* Get a pointer to the original strings */
  
  s1=fetch(FormString, &arg->a1);
  s2=fetch(FormString, &arg->a2);
  

  /* Grab the length word from up front */
  
  s1len=*(word *)s1;
  s2len=*(word *)s2;
  

  /* Increment the string pointer past that word */
  
  s1 += sizeof(word);
  s2 += sizeof(word);

  
  /* Create a new address to hold the new string */
  
  new.segment=SEG_GLOBAL;


  /* Calculate how long the destination string needs to be */
  
  
  maxlen=s1len+s2len;
  

  /* Allocate memory for it on the heap */
  
  if ((new.offset=hpalloc(sizeof(word)+maxlen))==END_HEAP)
    vm_err(err_no_ss);
  
  new.indirect=FALSE;


  /* Now find the address of the allocated location */
  
  sto=fetch(FormString, &new);

  
  /* Copy the length of the string */
  
  *(word *)sto=maxlen;
  sto += sizeof(word);
  
  
  /* And catenate the two strings */
  
  memmove(sto,       s1, s1len);
  memmove(sto+s1len, s2, s2len);


/*  kill_str(&arg->a2, &inst->arg2.addr); */

  
  /* Finally, update the destination and point the copied-to string to the  *
   * new string text.                                                       */

  store(&inst->res.dest, FormAddr, &new);

  return 0;
}



int op_skill(INST *inst, struct _args *arg)
{
  kill_str(&arg->a1, &inst->arg1.addr);
  return 0;
}



int op_slval(INST *inst, struct _args *arg)
{
  byte *str, *chptr;
  word slen, oldslen;
  word index;
  IADDR addr;

  str=fetch(FormString, &arg->a1);
  slen=*(word *)str;
  str += sizeof(word);

  index=arg->w2;

  if (index > 65000u || index==0)
    vm_err("string bounds exception");

  /* If we try to get the rvalue of a character past the end of the string, *
   * then we need to expand the string...                                   */

  if (index > slen)
  {
    oldslen=slen;

    /* If the character we want is beyond the end of the string, we need    *
     * to dynamically expand it.                                            */
       
    addr.segment=SEG_GLOBAL;
    addr.indirect=0;

    if ((addr.offset=hpalloc(index+sizeof(word)))==END_HEAP)
      vm_err(err_no_ss);
    

    /* Now move the string to the new location */
    
    memmove(pbDs+addr.offset, str-2, index+2);
    

    /* Now try to free the old string, assuming that it wasn't the stupid   *
     * "null string" at Glob:0 in the data segment, which can't be freed.   */

    if (slen && arg->a1.offset != 0)
      hpfree(str-pbDs-2);
    

    /* Now set the poinetr to the string to the place we copied it to. */
    
    str=pbDs+addr.offset;


    /* Update the new length of the string */

    slen=index;
    *(word *)str=slen;


    /* Now jump the string pointer past the length word */

    str += sizeof(word);


    /* Now blank-pad the string up 'till the offset we want */

    memset(str+oldslen, ' ', slen-oldslen);
    

    /* Now store the address of the new string in our operand */
    
    store(&inst->arg1.addr, FormAddr, &addr);
  }


  /* Find the location of the character, and return a pointer to it */
    
  chptr=&str[index-1];
  addr.segment=SEG_GLOBAL;
  addr.offset=(VMADDR)(chptr-pbDs);
  addr.indirect=FALSE;

  store(&inst->res.dest, FormAddr, &addr);
  
  return 0;
}



/* Evaluate a string as an rvalue (directly extract a character) */

int op_srval(INST *inst, struct _args *arg)
{
  byte *str;
  word slen;
  word index;
  byte ch;

  str=fetch(FormString, &arg->a1);
  slen=*(word *)str;
  str += sizeof(word);

  index=arg->w2;

  if (index > 65000u || index==0)
    vm_err("string bounds exception");

  /* If we try to get the rvalue of a character past the end of the string, *
   * make it into a space.  Otherwise, grab the character from the middle   *
   * of the array.                                                          */

  if (index > slen)
    ch=' ';
  else ch=str[index-1];

  store(&inst->res.dest, FormByte, &ch);

  return 0;
}



int op_slogical(INST *inst, struct _args *arg)
{
  byte *s1, *s2;
  word s1len, s2len, minlen;
  word res;
  sword cmp;
  
  s1=fetch(FormString, &arg->a1);
  s2=fetch(FormString, &arg->a2);
  
  s1len=*(word *)s1;
  s2len=*(word *)s2;
  minlen=min(s1len, s2len);
  
  s1 += sizeof(word);
  s2 += sizeof(word);

  cmp=strncmp(s1, s2, minlen);
  
  switch (inst->opcode)
  {
    case QOP_SGT: res=((cmp==0) ? (s1len >  s2len) : (cmp > 0));    break;
    case QOP_SGE: res=((cmp==0) ? (s1len >= s2len) : (cmp >= 0));   break;
    case QOP_SEQ: res=(cmp==0 && s1len==s2len);                     break;
    case QOP_SNE: res=(cmp != 0 || s1len != s2len);                 break;
    case QOP_SLE: res=((cmp==0) ? (s1len <= s2len) : (cmp <= 0));   break;
    case QOP_SLT: res=((cmp==0) ? (s1len <  s2len) : (cmp < 0));    break;
    default:      vm_err(err_invalid_opcode);
  }

  store(&inst->res.dest, FormWord, &res);
  return 0;
}




