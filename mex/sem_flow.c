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
static char rcs_id[]="$Id: sem_flow.c,v 1.1 2002/10/01 17:54:02 sdudley Exp $";
#pragma on(unreferenced)

#include "prog.h"
#include "mex.h"


PATCH *GenJumpOut();


/* Generate the first part of the code for the 'if' statement, and for      *
 * code handling ELSE conditions.                                           */

PATCH IfTest(DATAOBJ *obj)
{
  PATCH p;
  char tn[MAX_TN_LEN];

  p.quad=this_quad;
  p.next=NULL;

  if (obj)
  {
    Generate(QOP_JZ, obj, NULL, NULL);
    MaybeFreeTemporary(obj, TRUE);
  
    if (!IsIntegral(obj->type) && obj)
      error(MEXERR_INVALIDTYPEFORSTMT, TypeName(obj->type, tn), "if");
  }

  return p;
}


void IfEnd(PATCH *if_test, ELSETYPE *elses)
{
  BackPatch(if_test,          elses->else_label);
  BackPatch(elses->patchout, this_quad);
}



void ElseHandler(ELSETYPE *e)
{
  PATCH *ret;
  
  ret=e->patchout=smalloc(sizeof(PATCH));
  ret->quad=this_quad;
  ret->next=NULL;
  Generate(QOP_JMP, NULL, NULL, NULL);
  e->else_label=this_quad;
}


/* Semantic routine to generate the test at the bottom of a "do ... while()"
 * loop.  If the condition evaluates to non-zero, a jump is generated back
 * to the top of the loop.
 */

void GenDoWhileOut(WHILETYPE *w, DATAOBJ *obj)
{
  char tn[MAX_TN_LEN];

  w->jump.quad = this_quad;
  w->jump.next = NULL;

  if (obj && !IsIntegral(obj->type))
    error(MEXERR_INVALIDTYPEFORSTMT, TypeName(obj->type, tn), "do while");

  Generate(QOP_JNZ, obj, NULL, NULL);
  BackPatch(&w->jump, w->top_quad);

  MaybeFreeTemporary(obj, TRUE);
}


void WhileTest(WHILETYPE *w, DATAOBJ *obj)
{
  char tn[MAX_TN_LEN];

  w->jump.quad=this_quad;
  w->jump.next=NULL;

  if (obj && !IsIntegral(obj->type))
    error(MEXERR_INVALIDTYPEFORSTMT, TypeName(obj->type, tn), "while");

  Generate(QOP_JZ, obj, NULL, NULL);
  MaybeFreeTemporary(obj, TRUE);
}


/* This function generates code to handle the test at the top of a
 * for() loop.  This code is generated at the top of the loop,
 * right after the code for the init and the code for the loop test
 * has been generated.
 *
 * See comments in mex_tab.y for information relating to how the labels
 * in the for() loop are declared.
 */

void GenForTest(FORTYPE *pfr, DATAOBJ *pdo)
{
  char tn[MAX_TN_LEN];

  if (pdo && !IsIntegral(pdo->type))
    error(MEXERR_INVALIDTYPEFORSTMT, TypeName(pdo->type, tn), "for");

  /* Omit the jump to done if we have a null test condition (which
   * indicates an infinite loop).
   */

  if (pdo)
  {
    /* generate "jz done" */

    pfr->paJzDone.quad = this_quad;
    pfr->paJzDone.next = NULL;

    Generate(QOP_JZ, pdo, NULL, NULL);
  }

  /* generate "jmp body" */

  pfr->paJmpBody.quad = this_quad;
  pfr->paJmpBody.next = NULL;

  Generate(QOP_JMP, NULL, NULL, NULL);

  MaybeFreeTemporary(pdo, TRUE);
}


/* Generate code to do a "jmp test" for a for() loop.  This code is
 * generated right after the "post" code.
 */

void GenForJmpTest(FORTYPE *pfr)
{
  pfr->paJmpTest.quad = this_quad;
  pfr->paJmpTest.next = NULL;

  Generate(QOP_JMP, NULL, NULL, NULL);
}


/* Code to be generated at the bottom of the body code.  This
 * generates a jump back to the loop test, but it also cleans up by
 * backpatching the existing quads.
 */

void GenForJmpPostAndCleanup(FORTYPE *pfr)
{
  /* generate "jmp post" */

  pfr->paJmpPost.quad = this_quad;
  pfr->paJmpPost.next = NULL;

  Generate(QOP_JMP, NULL, NULL, NULL);

  /* Now patch all of our jump instructions so that they go to the right
   * place.
   */

  BackPatch(&pfr->paJzDone,  this_quad);
  BackPatch(&pfr->paJmpBody, pfr->vmBody);
  BackPatch(&pfr->paJmpTest, pfr->vmTest);
  BackPatch(&pfr->paJmpPost, pfr->vmPost);
}




/* Generate code at the bottom of a while() loop.  This contains code
 * to jump back to the top part of the loop, in addition to backpatching
 * all of the jumps to an appropriate location.
 */

void GenWhileOut(WHILETYPE *w)
{
  PATCH here;
  
  /* Save the current quad number, so we can patch the offset */
  
  here.quad=this_quad;
  here.next=NULL;
  
  /* Generate a jump back to the top of the loop */
  
  Generate(QOP_JMP, NULL, NULL, NULL);
  
  /* Now, backpatch the first JZ quad */
  
  BackPatch(&w->jump, this_quad);
  
  /* Finally, patch this one to jump to the top... */

  BackPatch(&here, w->top_quad);
}

