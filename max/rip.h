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

/*
 * Rip commands
 */

#define RIP_ESCAPE        '!'
#define RIP_LEADIN        '|'
#define RIP_LEADINSTR     "!|"
#define RIP_LEADINALT     "\x01|"

  /* ---- RIP Level 0 commands ---- */

#define RIP_TEXT_WINDOW   'w'     /* ----------- Set text window ----------- */
                                        /* x0:2,y0:2,x1:2,y1:2,wrap:1,size:1 */
                                            /* size=0:8x8 79/42, 1:7*7 90/42 */
                                  /* 2:8*14 79/24 3:7*14 90/24 4:16*14 39/24 */
#define RIP_VIEWPORT      'v'     /* -------- Set graphics viewport ---------*/
                                                      /* x0:2,y0:2,x1:2,y1:2 */
                                                         /* x=0-649, y=0,349 */
#define RIP_RESET_WINDOWS '*'     /* - Reset & clear graphic/text windows -- */
                                                                  /* No args */
#define RIP_ERASE_WINDOW  'e'     /* --------- Clear text window ----------- */
                                                                  /* No args */
#define RIP_ERASE_VIEW    'E'     /* -------- Erase graphics window -------- */
                                                                  /* No args */
#define RIP_GOTOXY        'g'     /* ---------- Move text cursor ----------- */
                                                                 /* x:2, y:2 */
                                                  /* Coordinates are 0-based */
#define RIP_HOME          'H'     /* ---------- Home text cursor ----------- */
                                                                  /* No args */
#define RIP_ERASE_EOL     '>'     /* --- Erase text line to end of line ---- */
                                                                  /* No args */
#define RIP_COLOR         'c'     /* ----- Set graphics drawing colour ----- */
                                                                 /* colour:2 */
#define RIP_SET_PALETTE   'Q'     /* - Sets attribute in 16-colour palette - */
                                                       /* c0:2,c1:2,...c15:2 */
#define RIP_ONE_PALETTE   'a'     /* -- Sets one attribute in the palette -- */
                                                                  /* c:2,v:2 */
#define RIP_WRITE_MODE    'W'     /* ----- Sets graphics drawing mode ------ */
                                                                      /* m:2 */
                                                        /* 0=overwrite 1=XOR */
#define RIP_MOVE          'm'     /* - Moves the current graphics position - */
                                                                 /* x:2, y:2 */
#define RIP_TEXT          'T'     /* ---- Puts text at current position ---- */
                                                              /* Text string */
#define RIP_TEXT_XY       '@'     /* ---- Put text at specific position ---- */
                                                      /* x:2,y:2 text string */
#define RIP_FONT_STYLE    'Y'     /* -- Sets font style orientation & size - */
                                                /* font:2,dir:2,size:2,res:2 */
                      /* font: 0=8x8, 1=triplex 2=small 3=sansserif 4=gothic */
                                             /* dir: 0=horizontal 1=vertical */
                                /* size: 1=default, 2-10=magification factor */
#define RIP_PIXEL         'X'     /* ---------- puts single pixel ---------- */
                                                                  /* x:2,y:2 */
#define RIP_LINE          'L'     /* ------------- Draws a line ------------ */
                                                      /* x0:2,y0:2,x1:2,y1:2 */
#define RIP_RECTANGLE     'R'     /* ---------- Draws a rectangle ---------- */
                                                      /* x0:2,y0:2,y1:2,y2:2 */
#define RIP_BAR           'B'     /* ------- Draws filled rectangle -------- */
                                                      /* x0:2,y0:2,y1:2,y2:2 */
#define RIP_CIRCLE        'C'     /* ----------- Draws a circle ------------ */
                                             /* xcentre:2,ycentre:2,radius:2 */
#define RIP_OVAL          'O'     /* ------- Draws an eliptical arc -------- */
                                                  /* x:2,y:2,x_rad:2,y_rad:2 */
#define RIP_FILLED_OVAL   'o'     /* -- Draws and fills an eliptical oval -- */
                                                  /* x:2,y:2,x_rad:2,y_rad:2 */
#define RIP_ARC           'A'     /* -------- Draws a circular arc --------- */
                                   /* x:2,y:2,start_ang:2,end_ang:2,radius:2 */
#define RIP_OVAL_ARC      'V'     /* ------- Draws an elipitical arc ------- */
                            /* x:2,y:2,start_ang:2,end_ang:2,x_rad:2,y_rad:2 */
#define RIP_PIE_SLICE     'I'     /* ----- Draws a circular pie slice ------ */
                                   /* x:2,y:2,start_ang:2,end_ang:2,radius:2 */
#define RIP_OVAL_PIE_SLICE 'i'    /* ---- Draws an eliptical pie slice ----- */
                            /* x:2,y:2,start_ang:2,end_ang:2,x_rad:2,y_rad:2 */
#define RIP_BEZIER        'Z'     /* ----- Draws a custom bezier curve ----- */
                            /* x1:2,y1:2,x2:2,y2:2,x3:2,y3:2,x4:2,y4:2,cnt:2 */
#define RIP_POLYGON       'P'     /* ----------- Draws a polygon ----------- */
                                               /* n:2,x1:2,x2:2,...xn:2,yn:2 */
#define RIP_FILL_POLY     'p'     /* ------ Draws and fills a polygon ------ */
                                               /* n:2,x1:2,x2:2,...xn:2,yn:2 */
#define RIP_FILL          'F'     /* ----------- Fills a region ------------ */
                                                         /* x:2,y:2,border:2 */
#define RIP_LINE_STYLE    '='     /* ----- Sets line style & lickness ------ */
                                               /* style:2,user_pal:4,thick:2 */
                   /* style: 0=solid, 1=dotted, 2=centered 3=dashed 4=custom */
  #define LINE_SOLID    0
  #define LINE_DOTTED   1
  #define LINE_CENTERED 2
  #define LINE_DASHED   3
  #define LINE_CUSTOM   4
                                                 /* thick: 1=1pixel 3=3pixel */
  #define LINE_THIN     1
  #define LINE_THICK    3

#define RIP_FILL_STYLE    'S'     /* ----------- Sets fill style ----------- */
                                                       /* pattern:2,colour:2 */
               /* 00    Fill with background color (color #0)
                  01    Solid Fill (fill color)
                  02    Line Fill ................. ----------- (thick lines)
                  03    Light Slash Fill .......... /  /  /  /  (thin lines)
                  04    Normal Slash Fill ......... // // // // (thick lines)
                  05    Normal Backslash Fill ..... \\ \\ \\ \\ (thick lines)
                  06    Light Backslash Fill ...... \  \  \  \  (thin lines)
                  07    Light Hatch Fill .......... ########### (thin lines)
                  08    Heavy Cross Hatch Fill .... XXXXXXXXXXX (thin lines)
                  09    Interleaving Line Fill .... +-+-+-+-+-+ (thin lines)
                  0A    Widely spaced dot fill .... . : . : . : (pixels only)
                  0B    Closely spaced dot fill ... ::::::::::: (pixels only) */
#define RIP_FILL_PATTERN  's'     /* -- Sets user-definable fill pattern --- */
                                               /* c1:2,c2:2,...c8:2,colour:2 */
#define RIP_NO_MORE       '#'     /* --------- End of RIP sequences -------- */

  /* ---- RIP Level 1 commands ---- */

#define RIP_MOUSE         'M'     /* ------- Define hot mouse region ------- */
                         /* num:2,x0:2,y0:2,x1:2,y1:2,clk:1,clr:1,res:5,text */
#define RIP_KILL_MOUSE_FIELDS 'L' /* Destroy any defined hot mouse regions - */
                                                                  /* No args */
#define RIP_BEGIN_TEXT    'T'     /* --- Defines rectangular text region --- */
                                                /* x1:2,y1:2,x2:2,y2:2,res:2 */
#define RIP_REGION_TEXT   't'     /* --- Display a line of text in region -- */
                                                    /* justify:1,text string */
#define RIP_END_TEXT      'E'     /* ---- End a rectangular text region  --- */
                                                                  /* No args */
#define RIP_GET_IMAGE     'C'     /* -- Copies region into the clipboard --- */
                                                /* x0:2,y0:2,x1:2,y1:2,res:1 */
#define RIP_PUT_IMAGE     'P'     /* ---------- Pastes clipboard ----------- */
                                                     /* x:2,y:2,mode:2,res:1 */
                                      /* mode: 0=copy 1=xor 2=or 3=and 4=not */
#define RIP_WRITE_ICON    'W'     /* ------- Write clipboard to icon ------- */
                                                          /* res:1, filename */
#define RIP_LOAD_ICON     'I'     /* ------- Load and display an icon ------ */
                                /* x:2,y:2,mode:2,clipboard:1,res:2,filename */
#define RIP_BUTTON_STYLE  'B'     /* --------- Define button style --------- */
/* w:2,h:2,o:2,f:2,s:2,df:2,db:2,br:2,dk:2,surf:2,grp:2,flags2:2,ul:2,cc:2,res:6 */

  #define LABEL_ABOVE     0x00
  #define LABEL_LEFT      0x01
  #define LABEL_CENTRE    0x02
  #define LABEL_RIGHT     0x03
  #define LABEL_BENEATH   0x04

  #define BUTTON_CLIP     0x0001    /* Button is a "Clipboard Button"  */
  #define BUTTON_INVERT   0x0002    /* Button is "invertable"          */
  #define BUTTON_RESET    0x0004    /* Reset screen after button click */
  #define BUTTON_CHISEL   0x0008    /* Display Chisel special effect   */
  #define BUTTON_RECESS   0x0010    /* Display Recessed special effect */
  #define BUTTON_SHADOW   0x0020    /* Dropshadow the label (if any)   */
  #define BUTTON_STAMP    0x0040    /* Auto-stamp image onto Clipboard */
  #define BUTTON_ICON     0x0080    /* Button is an "Icon Button"      */
  #define BUTTON_PLAIN    0x0100    /* Button is a "Plain Button"      */
  #define BUTTON_BEVEL    0x0200    /* Display Bevel special effect    */
  #define BUTTON_MOUSE    0x0400    /* Button is a Mouse Button        */
  #define BUTTON_HOTKEY   0x0800    /* Underline hot-key in label      */
  #define BUTTON_HOTICON  0x1000    /* Make Icon Button use Hot Icons  */
  #define BUTTON_ADJVERT  0x2000    /* Adjust vert. centering of label */
  #define BUTTON_RADIO    0x4000    /* Button belongs to a Radio Group */
  #define BUTTON_SUNKEN   0x8000    /* Display Sunken special effect   */

  #define FLAGS2_CHKBOX   0x01      /* Button is a checkbox group      */
  #define FLAGS2_HILITE   0x02      /* Highlight hotkey character      */
  #define FLAGS2_EXPLODE  0x04      /* Zoom out when clicked           */
  #define FLAGS2_LJUST    0x08      /* Left justify label              */
  #define FLAGS2_RJUST    0x10      /* Right justify label             */

#define RIP_BUTTON        'U'     /* -------- Define a mouse button  ------- */
                          /* x0:2.y0:2,x1:2,y1:2,hotkey:2,flags:1,res:1,text */
#define RIP_DEFINE        'D'     /* -------- Define a text variable ------- */
                                                       /* flags:3 res:2 text */
                         /* Flags: 1=save_to_db 2=no_blank 3=non_interactive */
  #define DEFFLAG_SAVE    0x01
  #define DEFFLAG_NOBLANK 0x02
  #define DEFFLAG_NONINT  0x04
#define RIP_QUERY         '\x1b'  /* -- Query contents of a text variable -- */
                                                        /* mode:1 res:3 text */
  #define QUERY_IMMED     0x00
  #define QUERY_GRCLICK   0x01
  #define QUERY_TXCLICK   0x02
#define RIP_COPY_REGION   'G'     /* --------- Copy screen region ---------- */
                                    /* x0:2 y0:2 x1:2 y1:2 res:2 dest_line:2 */

#define RIP_READ_SCENE    'R'     /* ------ Playback a local RIP scene ----- */
                                                        /* res:8 filename... */
#define RIP_FILE_QUERY    'F'     /* ------ Query existance of a file ------ */
                                                 /* mode:2 res:4 filename... */
                 /* Mode: 0=exists(0,1) 1=+CR 2=+.size 3=+.info 4=+.ext-info */

  /* Level 10 commands */

#define RIP_BLOCK_MODE    '\x1b'  /* ------ Enter block transfer mode ------ */
                                   /* proto:2 file_type:2 res:4 [filename:2] */
  /* Values for proto */
  #define PROTO_XMODEM_CHK  0
  #define PROTO_XMODEM_CRC  1
  #define PROTO_XMODEM_1K   2
  #define PROTO_XMODEM_1KG  3
  #define PROTO_KERMIT      4
  #define PROTO_YMODEM_B    5
  #define PROTO_YMODEM_G    6
  #define PROTO_ZMODEM      7     /* No filename required */
  /* Values for type */
  #define FT_RIPDISPLAY     0     /* RIP sequences - display */
  #define FT_RIPSTORE       1     /* RIP sequences - store them */
  #define FT_ICON           2     /* Icon, store in proper directories */
  #define FT_HELP           3     /* Help files - store in proper directories */
  #define FT_COMPDYN        4     /* Composite dynamic file - store (batch only) */
  #define FT_ACTDYN         5     /* Active dynamic file - store & playback (batch only) */



