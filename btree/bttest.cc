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
static char rcs_id[]="$Id: bttest.cc,v 1.1.1.1 2002/10/01 17:49:23 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <conio.h>
#include "btree.h"

int intcomp(void *a1, void *a2)
{
  return (*(int *)a1-*(int *)a2);
}

int *addr_int(int i)
{
  static int si;
  si=i;
  return &si;
}

int main(void)
{
  BTREE b1, b2, b3, b4, bt;

  if (!b1.open("b1.i00", intcomp, intcomp, sizeof(int), TRUE))
    printf("Error opening base b1.i00!\n");

  if (!b2.open("b2.i00", intcomp, intcomp, sizeof(int), TRUE))
    printf("Error opening base b2.i00!\n");

  if (!b3.open("b3.i00", intcomp, intcomp, sizeof(int), TRUE, 4))
    printf("Error opening base b3.i00!\n");

  if (!b4.open("b4.i00", intcomp, intcomp, sizeof(int), TRUE))
    printf("Error opening base b4.i00!\n");

  if (!bt.open("bt.i00", intcomp, intcomp, sizeof(int), TRUE, 4))
    printf("Error opening base bt.i00!\n");

  int i, *pi;

  srand(15);

  printf("Inserting stuff in bt\n");

  bt.insert(addr_int(10), IF_NORM);
  bt.insert(addr_int(20), IF_NORM);
  bt.insert(addr_int(30), IF_NORM);
  bt.insert(addr_int(40), IF_NORM);
  bt.insert(addr_int(5), IF_NORM);
  bt.insert(addr_int(15), IF_NORM);
  bt.insert(addr_int(25), IF_NORM);
  bt.insert(addr_int(35), IF_NORM);
  bt.insert(addr_int(45), IF_NORM);
  bt.insert(addr_int(46), IF_NORM);
  bt.insert(addr_int(47), IF_NORM);
  bt.insert(addr_int(16), IF_NORM);
  bt.insert(addr_int(17), IF_NORM);
  bt.insert(addr_int(6), IF_NORM);
  bt.insert(addr_int(18), IF_NORM);
  bt.insert(addr_int(19), IF_NORM);
  bt.insert(addr_int(31), IF_NORM);
  bt.insert(addr_int(32), IF_NORM);
  bt.insert(addr_int(11), IF_NORM);

  printf("Removing stuff from bt\n");

  bt.remove(addr_int(46));
  bt.remove(addr_int(47));
  bt.remove(addr_int(31));
  bt.remove(addr_int(32));
  bt.remove(addr_int(18));
  bt.remove(addr_int(19));
  bt.remove(addr_int(10));
  bt.remove(addr_int(11));
  bt.remove(addr_int(30));
  bt.remove(addr_int(40));
  bt.remove(addr_int(35));
  bt.remove(addr_int(25));
  bt.remove(addr_int(35));
  bt.remove(addr_int(6));
  bt.remove(addr_int(15));
  bt.remove(addr_int(45));
  bt.remove(addr_int(17));
  bt.remove(addr_int(5));
  bt.remove(addr_int(20));
  bt.remove(addr_int(16));

  printf("Inserting stuff in b1\n");

  for (i=100; i >= 1; i--)
    b1.insert(addr_int(i), IF_NORM);

  printf("Checking for stuff in b1\n");

  for (i=100; i >= 1; i--)
    if ((pi=(int *)b1.lookup(addr_int(i), NULL))==NULL || *pi != i)
      printf("1.1: can't find %d!\n", i);


  printf("Inserting more stuff in b1\n");

  for (i=101; i < 200; i++)
    b1.insert(addr_int(i), IF_NORM);

  printf("Checking for more stuff in b1\n");

  for (i=101; i < 200; i++)
    if ((pi=(int *)b1.lookup(addr_int(i), NULL))==NULL || *pi != i)
      printf("1.2: can't find %d!\n", i);

  printf("Inserting stuff in b2\n");

  b2.insert(addr_int(20), IF_NORM);
  b2.insert(addr_int(19), IF_NORM);
  b2.insert(addr_int(18), IF_NORM);
  b2.insert(addr_int(17), IF_NORM);
  b2.insert(addr_int(16), IF_NORM);
  b2.insert(addr_int(15), IF_NORM);
  b2.insert(addr_int(14), IF_NORM);
  b2.insert(addr_int(13), IF_NORM);
  b2.insert(addr_int(12), IF_NORM);
  b2.insert(addr_int(11), IF_NORM);
  b2.insert(addr_int(10), IF_NORM);
  b2.insert(addr_int(9), IF_NORM);
  b2.insert(addr_int(8), IF_NORM);
  b2.insert(addr_int(7), IF_NORM);
  b2.insert(addr_int(6), IF_NORM);
  b2.insert(addr_int(5), IF_NORM);
  b2.insert(addr_int(4), IF_NORM);
  b2.insert(addr_int(3), IF_NORM);
  b2.insert(addr_int(2), IF_NORM);
  b2.insert(addr_int(1), IF_NORM);

  printf("Checking for stuff in b2\n");

  if (*(int *)b2.lookup(addr_int(20), NULL) != 20)
    printf("2: Can't find 20\n");
  if (*(int *)b2.lookup(addr_int(19), NULL) != 19)
    printf("2: Can't find 19\n");
  if (*(int *)b2.lookup(addr_int(18), NULL) != 18)
    printf("2: Can't find 18\n");
  if (*(int *)b2.lookup(addr_int(17), NULL) != 17)
    printf("2: Can't find 17\n");
  if (*(int *)b2.lookup(addr_int(16), NULL) != 16)
    printf("2: Can't find 16\n");
  if (*(int *)b2.lookup(addr_int(15), NULL) != 15)
    printf("2: Can't find 15\n");
  if (*(int *)b2.lookup(addr_int(14), NULL) != 14)
    printf("2: Can't find 14\n");
  if (*(int *)b2.lookup(addr_int(13), NULL) != 13)
    printf("2: Can't find 13\n");
  if (*(int *)b2.lookup(addr_int(12), NULL) != 12)
    printf("2: Can't find 12\n");
  if (*(int *)b2.lookup(addr_int(11), NULL) != 11)
    printf("2: Can't find 11\n");
  if (*(int *)b2.lookup(addr_int(10), NULL) != 10)
    printf("2: Can't find 10\n");
  if (*(int *)b2.lookup(addr_int(9), NULL) != 9)
    printf("2: Can't find 9\n");
  if (*(int *)b2.lookup(addr_int(8), NULL) != 8)
    printf("2: Can't find 8\n");
  if (*(int *)b2.lookup(addr_int(7), NULL) != 7)
    printf("2: Can't find 7\n");
  if (*(int *)b2.lookup(addr_int(6), NULL) != 6)
    printf("2: Can't find 6\n");
  if (*(int *)b2.lookup(addr_int(5), NULL) != 5)
    printf("2: Can't find 5\n");
  if (*(int *)b2.lookup(addr_int(4), NULL) != 4)
    printf("2: Can't find 4\n");
  if (*(int *)b2.lookup(addr_int(3), NULL) != 3)
    printf("2: Can't find 3\n");
  if (*(int *)b2.lookup(addr_int(2), NULL) != 2)
    printf("2: Can't find 2\n");
  if (*(int *)b2.lookup(addr_int(1), NULL) != 1)
    printf("2: Can't find 1\n");


  printf("Generating random numbers for B3\n");

#define NUM 500

  static int a[NUM];

  for (i=0; i < NUM; i++)
  {
foo:
    a[i]=rand();

    for (int j=0; j < i; j++)
      if (a[i]==a[j])
        goto foo;
  }

  printf("Inserting stuff in b3\n");

  for (i=0; i < NUM; i++)
    b3.insert(addr_int(a[i]), IF_NORM);

  printf("Checking for stuff in b3\n");

  for (i=0; i < NUM; i++)
    if (*(int *)b3.lookup(addr_int(a[i]), NULL) != a[i])
      printf("3: error finding %d!\n", a[i]);


  printf("Removing stuff from b3\n");

  srand(0x1238L);

  for (i=0; i < NUM; i++)
  {
    printf("removing a[%d]=%d\n", i, a[i]);

#if 1
    for (int j=i; j < NUM; j++)
    {
      if (*(int *)b3.lookup(addr_int(a[j]), NULL) != a[j])
      {
        printf("\n3: about to remove a[%d]=%d (just removed a[%d]=%d), "
               "but couldn't find a[%d]=%d\n",
               i, a[i], i-1, a[i-1], j, a[j]);

        getch();
      }
    }
#endif

//  b3.validate();


    if (!b3.remove(addr_int(a[i])))
    {
      printf("3: couldn't remove %d (%d)!\n", a[i], i);
      getch();
    }
  }

  printf("Making sure that b3 is empty\n");

  for (i=0; i < 500; i++)
    if (b3.lookup(addr_int(a[i]), NULL) || b3.remove(addr_int(a[i])))
      printf("3: we were somehow able to still find a[%d]=%d!\n", i,
             a[i]);

  printf("Inserting stuff in b4\n");

  b4.insert(addr_int(4), IF_NORM);
  b4.insert(addr_int(9), IF_NORM);
  b4.insert(addr_int(5), IF_NORM);
  b4.insert(addr_int(8), IF_NORM);
  b4.insert(addr_int(6), IF_NORM);
  b4.insert(addr_int(1), IF_NORM);
  b4.insert(addr_int(10), IF_NORM);
  b4.insert(addr_int(3), IF_NORM);
  b4.insert(addr_int(11), IF_NORM);
  b4.insert(addr_int(12), IF_NORM);
  b4.insert(addr_int(2), IF_NORM);
  b4.insert(addr_int(13), IF_NORM);
  b4.insert(addr_int(7), IF_NORM);
  b4.insert(addr_int(14), IF_NORM);
  b4.insert(addr_int(4), IF_NORM);

  printf("Checking for stuff in b4\n");

  if (*(int *)b4.lookup(addr_int(4), NULL) != 4)
    printf("4: can't find 4\n");
  if (*(int *)b4.lookup(addr_int(9), NULL) != 9)
    printf("4: can't find 9\n");
  if (*(int *)b4.lookup(addr_int(5), NULL) != 5)
    printf("4: can't find 55n");
  if (*(int *)b4.lookup(addr_int(8), NULL) != 8)
    printf("4: can't find 8\n");
  if (*(int *)b4.lookup(addr_int(6), NULL) != 6)
    printf("4: can't find 6\n");
  if (*(int *)b4.lookup(addr_int(1), NULL) != 1)
    printf("4: can't find 1\n");
  if (*(int *)b4.lookup(addr_int(10), NULL) != 10)
    printf("4: can't find 10\n");
  if (*(int *)b4.lookup(addr_int(3), NULL) != 3)
    printf("4: can't find 3\n");
  if (*(int *)b4.lookup(addr_int(11), NULL) != 11)
    printf("4: can't find 11\n");
  if (*(int *)b4.lookup(addr_int(12), NULL) != 12)
    printf("4: can't find 12\n");
  if (*(int *)b4.lookup(addr_int(2), NULL) != 2)
    printf("4: can't find 2\n");
  if (*(int *)b4.lookup(addr_int(13), NULL) != 13)
    printf("4: can't find 13\n");
  if (*(int *)b4.lookup(addr_int(7), NULL) != 7)
    printf("4: can't find 7\n");
  if (*(int *)b4.lookup(addr_int(14), NULL) != 14)
    printf("4: can't find 14\n");

  bt.close();
  b4.close();
  b3.close();
  b2.close();
  b1.close();

  return 0;
}

