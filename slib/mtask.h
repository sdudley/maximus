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


cpp_begin()

int far pascal dvloaded(void);
int far pascal tvloaded(void);
int far pascal ddosloaded(void);
int far pascal pcmosloaded(void);
int far pascal winloaded(void);
int far pascal mlinkloaded(void);
int far pascal os2loaded(void);

void far pascal dvsleep(void);
void far pascal ddossleep(void);
void far pascal pcmossleep(void);
void far pascal winsleep(void);
void far pascal mlinksleep(void);
void far pascal spoolsleep(void);
void far pascal os2sleep(void);

cpp_end()

