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
static char rcs_id[]="$Id: f_con.c,v 1.1 2002/10/01 17:51:00 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: C)ontents functions
*/

#include <stdio.h>
#include <ctype.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max_file.h"
#include "f_con.h"
#include "arc.h"

static sword near Display_Contents(char *filename);
static sword near Read_Zip(int zipfile);
static sword near Zip_Scan_For_Header(char *buffer);
static sword near Zip_Read_Directory(int zipfile,long zip_pos,int offset);
static sword near Read_LzhArc(int type,int lzhfile);
static sword near do_ext_hdr(int lzhfile, byte **fname, byte *buf,word hdrlen);


/* Try and match an archive with the given name */

static word near ValidArc(char *fspec, char *ext)
{
  char *s;

  if ((s=strrchr(fspec, '.')) != NULL)
    *s='\0';

  strcat(fspec, ext);

  return IsInFilesBbs(&fah, fspec, NULL, NULL);
}



void File_Contents(void)
{
  char filename[PATHLEN];
  char filespec[PATHLEN];


  Puts(WHITE);

  WhiteN();
  *filename='\0';

  do
  {
    if (*filename)
      Display_File(0, NULL, PRM(hlp_contents));

    InputGets(filename, contents_of);
  }
  while (eqstri(filename, qmark));


  /* Make sure user doesn't specify path or device */
  
  Strip_Path(filename);

  if (! *filename)
    return;

  display_line=display_col=1;


  /* Add an extension, if none given */

  if (! strchr(filename,'.'))
  {
    if (ValidArc(filename, dot_lzh) || ValidArc(filename, dot_arc) ||
        ValidArc(filename, dot_pak) || ValidArc(filename, dot_arj)  ||
        ValidArc(filename, dot_zip))
      ;
  }

  if (File_Okay(filename))
  {
    char path[PATHLEN];

    path[0]=0;

    sprintf(filespec, ss,
            (IsInFilesBbs(&fah, filename, NULL, path) && *path)
              ? path : FAS(fah, downpath),
            filename);

    if (Display_Contents(filespec)==-1)
    {
      if (strrchr(filespec, '.'))
        *strrchr(filespec, '.')='\0';

      Printf(bad_arc, upper_fn(filename));
    }
  }
}



static sword near Display_Contents(char *filename)
{
  struct _lh0 *lh0=NULL;

  char buffer[SEEK_SIZE+10];
  char an[PATHLEN];

  int arcfile;
  sword ret;

  strcpy(an, filename);

  Printf(sarc, No_Path(upper_fn(an)));

  if ((arcfile=shopen(an, O_RDONLY | O_BINARY))==-1)
    return -1;

  read(arcfile, buffer, SEEK_SIZE-1);
  lseek(arcfile, 0L, SEEK_SET);

  lh0=(struct _lh0 *)(buffer+sizeof(struct _lhpre));

  if (*buffer=='P' && *(buffer+1)=='K')
    ret=Read_Zip(arcfile);
  else if (lh0->method[0]=='-' && lh0->method[1]=='l' &&
          (lh0->method[2]=='z' || lh0->method[2]=='h') &&
           isdigit(lh0->method[3]) && lh0->method[4]=='-')
    ret=Read_LzhArc(ARCHIVE_LZH, arcfile);
  else if (*buffer=='\x1a')
    ret=Read_LzhArc(ARCHIVE_ARC, arcfile);
  else if (*(word *)buffer==ARJ_ID)
    ret=Read_LzhArc(ARCHIVE_ARJ, arcfile);
  else ret=-1;

  close(arcfile);
  return ret;
}



static sword near Read_Zip(int zipfile)
{
  byte buffer[SEEK_SIZE+10];
  word found_header;
  long zip_pos;

  if (lseek(zipfile,0L,SEEK_END)==-1L)
  {
    close(zipfile);
    return -1;
  }

  zip_pos=tell(zipfile);
  found_header=FALSE;

  while ((zip_pos >= 0) && (! found_header))
  {
    if (zip_pos==0)
      zip_pos=-1L;
    else zip_pos -= min((SEEK_SIZE-4L),zip_pos);

    lseek(zipfile, zip_pos, SEEK_SET);

    if (zip_pos >= 0)
    {
      if (read(zipfile, buffer, SEEK_SIZE) <= 0)
        return -1;

      found_header=Zip_Scan_For_Header(buffer);
    }
  }

  if (found_header==FALSE)  /* Couldn't find central dir. header! */
    return -1;

  if (Zip_Read_Directory(zipfile, zip_pos, found_header)==-1)
    return -1;

  return 0;
}



static sword near Zip_Scan_For_Header(char *buffer)
{
  byte *x;

  if ((x=memstr(buffer,END_STRSIG,SEEK_SIZE,strlen(END_STRSIG))) != NULL)
    return (sword)(x-buffer);
  else return 0;
}



static sword near Zip_Read_Directory(int zipfile,long zip_pos,int offset)
{
  byte temp[5];
  byte method[8];
  byte attributes[5];
  byte *filename;
  byte *file_comment=NULL;
  byte nonstop;
  int ratio_percent;
  int ratio_decimal;

  dword total_uncompressed;
  dword total_compressed;

  word num_files;

  struct _dir_end dir_end;
  struct _dir_header dir_head;

  union stamp_combo stamp;


  /* Seek to the End_Of_Central_Directory record, which we found the        *
   * location of earlier.                                                   */

  lseek(zipfile, (long)((long)zip_pos+(long)offset+4L), SEEK_SET);
  read(zipfile, (char *)&dir_end.info, sizeof(dir_end.info));


  /* We don't support multi-disk ZIPfiles */

  if (dir_end.info.this_disk != 0)
    return -1;

  nonstop=FALSE;
  display_line=display_col=1;

  Puts("\n\n");

  lseek(zipfile, dir_end.info.offset_start_dir, SEEK_SET);

  total_uncompressed=total_compressed=0;
  num_files=0;

  Puts(ziphdr1);
  Puts(ziphdr2);

/*     "12345678 Implode 1234567 86.0% 11-18-89 18:59 12345678 --w* ASDF.DOC"
       "컴컴컴컴         컴컴컴� 컴컴�                              컴컴\n"
*/

  for (;;)
  {
    if (read(zipfile, temp, 4) != 4)
      return -1;

    /* If we are at a directory entry */

    if (memcmp(temp,DIR_STRSIG,4)==0)
    {
      Mdm_check();

      if (halt() || MoreYnBreak(&nonstop, NULL))
        break;

      /* Read in this entry's information */

      if (read(zipfile, (char *)&dir_head.info, sizeof(dir_head.info)) !=
              sizeof(dir_head.info))
        return -1;


      /* Allocate memory for filename and comments... */

      if ((filename=(char *)malloc(dir_head.info.filename_length+1))==NULL)
        return -1;

      if (dir_head.info.file_comment_length)
      {
        if ((file_comment=
               (char *)malloc(dir_head.info.file_comment_length+1))==NULL)
        {
          /* It isn't critical if we can't allocate the memory for the      *
           * file comment, so just zero out the field and continue.         */

          dir_head.info.file_comment_length=0;
        }
      }


      /* Read in the filename */
      
      if (read(zipfile, filename, dir_head.info.filename_length) !=
            (signed)dir_head.info.filename_length)
        return -1;
      
      filename[dir_head.info.filename_length]='\0';


      /* Skip over the extra field */

      lseek(zipfile,(long)dir_head.info.xtra_field_length,SEEK_CUR);


      /* Read in the file comment */

      if (dir_head.info.file_comment_length)
      {
        *file_comment='\0';
        
        read(zipfile,file_comment,dir_head.info.file_comment_length);

        file_comment[dir_head.info.file_comment_length]='\0';
        Strip_Ansi(file_comment,0,0);
      }


      /* Remove any funky ANSI sequences from the filename and comment */

      Strip_Ansi(filename,0,0);


      /* Figure out the compression method */

      switch (dir_head.info.comp_method)
      {
        case 0:
          strcpy(method,zip_store);
          break;

        case 1:
          strcpy(method,zip_shrunk);
          break;

        case 2:
        case 3:
        case 4:
        case 5:
          strcpy(method, zip_reduce);
          method[6]=(char)('0'+(dir_head.info.comp_method-1));
          break;

        case 6:
          strcpy(method, zip_implode);
          break;

        case 7:
        case 8:
          strcpy(method, zip_deflate);
          break;

        default:
          strcpy(method, unknown);
          break;
      }


      /* Grab the file's datestamp */

      stamp.dos_st.date=dir_head.info.last_mod_date;
      stamp.dos_st.time=dir_head.info.last_mod_time;


      /* Get the file's attributes */

      strcpy(attributes,four_blanks);
      attributes[3]='\0';

      /* Only get attributes for MS-DOS -- Others are implementation
         dependent.                                                   */

      if ((dir_head.info.ver_made_by >> 8)==0)
      {
        if (dir_head.info.file_attr_ext & MSDOS_HIDDEN)
          attributes[0]='h';

        if (dir_head.info.file_attr_ext & MSDOS_SYSTEM)
          attributes[1]='s';

        if (dir_head.info.file_attr_ext & MSDOS_READONLY)
          attributes[2]='r';
        else attributes[2]='w';
      }

      if (!dir_head.info.uncompressed_size)
        dir_head.info.uncompressed_size=1L;

      ratio_percent = (int)(100-((dir_head.info.compressed_size*100)/
                           dir_head.info.uncompressed_size));

      if (ratio_percent==100)
        ratio_percent = 99;

      ratio_decimal = (int)(1000-((dir_head.info.compressed_size*1000)/
                           dir_head.info.uncompressed_size)) % 10;

      Printf(zip_format,
             dir_head.info.uncompressed_size,
             method,
             dir_head.info.compressed_size,
             ratio_percent,
             ratio_decimal,
             stamp.msg_st.date.mo,
             stamp.msg_st.date.da,
             (stamp.msg_st.date.yr+80) % 100,
             stamp.msg_st.time.hh,
             stamp.msg_st.time.mm,
             dir_head.info.crc,
             attributes,
             (dir_head.info.bits & 0x01) ? '*' : ' ',
             filename);


      /* Now display the file comment... */

      if (dir_head.info.file_comment_length)
        Printf(zip_cmnt,file_comment);


      /* Display a warning message if file is encrypted... */

      if (dir_head.info.bits & 0x01)
        Printf(zip_encrypt,filename);


      /* Add this to the totals... */

      total_uncompressed += dir_head.info.uncompressed_size;
      total_compressed += dir_head.info.compressed_size;

      num_files++;


      /* Release the memory allocated for the filename, and possibly the    *
       * file comment.                                                      */

      free(filename);

      if (dir_head.info.file_comment_length)
        free(file_comment);
    }
    else if (memcmp(temp,END_STRSIG,4)==0)
    {
      display_line += 2;

      if (MoreYnBreak(&nonstop,NULL))
        break;

      /* Found end of directory record, so we do post-processing here,
         and exit.                                                    */

       Puts(zip_trail1);

      if (total_uncompressed)
        Printf(zip_trail2,
               total_uncompressed,total_compressed,
               (int)(100-((total_compressed*100)/total_uncompressed)),
               (int)(1000-((total_compressed*1000)/total_uncompressed)) % 10,
               num_files);

      return 0;
    }
    else return -1; /* Else found gibberish! */
  }

  /* We must have gotten here by a 'N' answer to a "More [Y,n,=]?" prompt */
  return 0;
}


static sword near Read_LzhArc(int type,int lzhfile)
{
  struct _lhpre pre;
  struct _lh0 *lh0=NULL;
  struct _lh2 *lh2;
  byte *lhtemp=NULL, *cur;

  struct _archead archead;
  struct tm *filed;
  
  struct _arj_id aid;
  struct _arj_hdr *arj=NULL;

  byte *fname, *p;
  byte attr[6];
  byte method[10];
  byte nonstop;

  word tempword, crc;
  word num_files, percent, first;

  sword hsize;

  dword total_compressed, total_uncompressed;
  dword compressed_size=0L, uncompressed_size=0L;
  dword dwcrc;

  union stamp_combo file_date;


  
  first=TRUE;

  num_files=0;
  total_compressed=total_uncompressed=0L;
  nonstop=FALSE;

  Puts(arc_h1);
  Puts(arc_h2);
  
  while (! eof(lzhfile))
  {
    Mdm_check();

    if (halt() || MoreYnBreak(&nonstop,NULL))
      return 0;

    strcpy(attr, four_blanks);

    if (type==ARCHIVE_LZH)
    {
      /* Read in the pre-header, contining the block length and artithmetic *
       * checksum of the header.                                            */
         
      if (read(lzhfile,(char *)&pre,sizeof(struct _lhpre)) !=
                sizeof(struct _lhpre))
      {
        break;
      }
      
      if (pre.hdrsize==0 || (lhtemp=malloc(pre.hdrsize))==NULL)
        break;

      /* Now read in the junk into a flat array */
      
      if (read(lzhfile,lhtemp,pre.hdrsize) < pre.hdrsize-1)
        break;

      /* If not a hdr, get out of loop */
      
      if (pre.hdrsize==0)
        break;

      lh0=(struct _lh0 *)lhtemp;
      fname=NULL;
      
      if (lh0->hdr_ver==0 || lh0->hdr_ver==1)
      {
        /* Grab the right date format */
        file_date=lh0->date;
        
        #ifdef LITTLE_ENDIAN

        /* Swap the byte order of the file's date */
        file_date.ldate=((file_date.ldate >> 16) | (file_date.ldate << 16));

        #endif
          
        /* Pointer to access data right after the fixed-length structure */
        
        cur=lhtemp+sizeof(struct _lh0);

        
        /* The filename directly follows the structure itself */

        if ((fname=malloc(lh0->namelen+1))==NULL)
          break;
        
        memmove(fname, cur, lh0->namelen);
        fname[lh0->namelen]='\0';
        
        cur += lh0->namelen;
        
        
        /* After the var-length filename is the CRC */
        
        crc=*(word *)cur;
        cur += sizeof(word);
        
        /* Skip the next byte */
        
        cur++;
        
        /* Get the extended headers for this file */
        
        if (lh0->hdr_ver==1)
        {
          if ((hsize=do_ext_hdr(lzhfile, &fname, NULL, *(word *)cur))==-1)
            break;
          else lh0->compressed_size -= hsize;
        }
      }
      else if (lh0->hdr_ver != 2)
        break; /* unknown header ver */
      else  /* version 2 */
      {
        lh2=(struct _lh2 *)lhtemp;

        /* Grab the crc from the type2 header */
        crc=lh2->crc;

        /* Convert the unix-style date to a dos bitmap */
        
        filed=localtime(&lh2->date);
        TmDate_to_DosDate(filed,&file_date);
        
        if ((hsize=do_ext_hdr(-1, &fname, lhtemp+sizeof(struct _lh2),
                              lh2->ext_size))==-1)
        {
          break;
        }
      }
      
      if (lh0->attr & MSDOS_ARCHIVE)
        attr[0]='a';

      if (lh0->attr & MSDOS_SYSTEM)
        attr[1]='s';

      if (lh0->attr & MSDOS_HIDDEN)
        attr[2]='h';

      if (lh0->attr & MSDOS_READONLY)
        attr[3]='r';
      else attr[3]='w';



      strcpy(method," ");
      strncpy(method+1,lh0->method,5);
      method[6]='\0';

      uncompressed_size=lh0->uncompressed_size;
      compressed_size=lh0->compressed_size;
    }
    else if (type==ARCHIVE_ARC)
    {
      if (read(lzhfile, (char *)&archead, sizeof(struct _archead)) != 
               sizeof(struct _archead))
      {
        break;
      }

      if (archead.method==0)
        break;

      if (archead.method >= 1 && archead.method < NUM_COMPRESS_TYPE)
        strcpy(method, compression_type[archead.method]);
      else
        strcpy(method, compression_type[0]);

      file_date.dos_st.date=archead.date;
      file_date.dos_st.time=archead.time;

      compressed_size=archead.compressed_size;
      uncompressed_size=archead.uncompressed_size;
      
      fname=archead.name;
      crc=archead.crc;
    }
    else if (type==ARCHIVE_ARJ)
    {
      /* Try to read the pre-header that comes before each real header */

      if (read(lzhfile, (char *)&aid, sizeof(struct _arj_id)) !=
                             sizeof(struct _arj_id) ||
          aid.id != ARJ_ID)
      {
        break;
      }

      /* End of archive */
      if (aid.hdr_size==0)
        break;

      /* Allocate some memory to hold the real header */
      
      if ((lhtemp=malloc(aid.hdr_size))==NULL)
        break;
      
      
      /* Read in the real header */

      if (read(lzhfile, lhtemp, aid.hdr_size) != (signed)aid.hdr_size)
        break;
      

      /* Read in the header's CRC */

      if (read(lzhfile, (char *)&dwcrc, sizeof(dword)) != sizeof(dword))
        break;


      arj=(struct _arj_hdr *)lhtemp;
      cur=lhtemp+arj->hdr_size;
      

      /* The filename comes directly after the header block */
      
      fname=cur;
      
      /* Skip past the filename */

      cur += strlen(cur)+1;
      
      /* Skip past the comment */
      
      cur += strlen(cur)+1;

      /* Skip over any extended headers */

      while (read(lzhfile, (char *)&tempword, sizeof(word)) && tempword)
        lseek(lzhfile, (long)tempword+4L, SEEK_CUR);
      
      if (arj->method==0)
        strcpy(method, zip_store);
      else
      {
        strcpy(method, zip_reduce);
        method[6]=(char)('0'+arj->method);
      }

      file_date=arj->mod_date;

      #ifdef LITTLE_ENDIAN /* iAPx86 */
        
        /* Reverse byte order */
        
        file_date.ldate=((file_date.ldate >> 16) | (file_date.ldate << 16));

      #endif

      if (!first)
      {
        compressed_size=arj->comp_size;
        uncompressed_size=arj->orig_size;
      }

      crc=(word)arj->file_crc;
    }
    else break;


    /* Now output the name of the file, size, etc. */

    if (uncompressed_size)
      percent=(int)(100-((compressed_size*100)/uncompressed_size));
    else 
    {
      percent=0;
      uncompressed_size=1L;
    }
      

    if (type != ARCHIVE_ARJ || !first)
    {
      p=fname;
      
      if (strrchr(p, '/'))
        p=strrchr(p, '/')+1;
      
      if (strrchr(p, '\\'))
        p=strrchr(p, '\\')+1;

      Printf(lzh_type,
             p,
             uncompressed_size,
             compressed_size,
             percent,
             (int)(1000-((compressed_size*1000)/uncompressed_size)) % 10,
             file_date.msg_st.date.mo,
             file_date.msg_st.date.da,
             (file_date.msg_st.date.yr+80) % 100,
             file_date.msg_st.time.hh,
             file_date.msg_st.time.mm,
             file_date.msg_st.time.ss << 1,
             attr,
             method,
             crc);
    }

    total_compressed += compressed_size;
    total_uncompressed += uncompressed_size;

    /* Now move over to the next header */
    
    if (type==ARCHIVE_LZH)
    {
      /* If it's a v2 lzh header, deduct two bytes, since for v0/v1, the    *
       * 'pre' count didn't include the main header, whereas the            *
       * v2 format does.                                                    */

      lseek(lzhfile,
              lh0->compressed_size - (lh0->hdr_ver==2 ? 2 : 0),
              SEEK_CUR);

      free(lhtemp);
      free(fname);
    }
    else if (type==ARCHIVE_ARC)
      lseek(lzhfile, archead.compressed_size, SEEK_CUR);
    else if (type==ARCHIVE_ARJ)
    {
      /* Skip over the compressed file */

      /* !@#$!@#$ arj bugs.  arj->comp_size is junk for the beginning       *
       * header in fd201.arj.                                               */

      if (!first)
        lseek(lzhfile, arj->comp_size, SEEK_CUR);

      free(lhtemp);
    }
    
    first=FALSE;

    num_files++;
  }

  Puts(arc_t1);

  if (total_uncompressed)
      Printf(arc_t2, num_files,
         total_uncompressed,
         total_compressed,
         (int)(100-((total_compressed*100)/total_uncompressed)),
         (int)(1000-((total_compressed*1000)/total_uncompressed)) % 10);

  return 0;
}


static sword near do_ext_hdr(int lzhfile, byte **fname, byte *buf,word hdrlen)
{
  byte *ext, *p;
  word extsize=0;

  p=buf;
  
  while (hdrlen)
  {
    /* If we were supplied a buffer, then the extended headers were already *
     * read in.  This is due to the asinine way in which LH 2.xx handles    *
     * the different header versions - for header version 0/1, the 'pre'    *
     * length refers to the length of the first main header.  However       *
     * for version 2, it represents the length of the WHOLE header,         *
     * including the extended stuff.  If that's the case, we've already     *
     * read it in, so we then just shift a pointer through our buffer.      */
       
    if (buf)
    {
      ext=p;
      p += hdrlen;
    }
    else
    {
      if ((ext=malloc(hdrlen))==NULL)
        break;

      /* Read the extended header into memory*/

      if (read(lzhfile, ext, hdrlen) != (signed)hdrlen)
        break;
    }

    if (*ext==0)  /* CRC header; ignore */
      ;
    else if (*ext==1) /* filename header */
    {
      if (*fname)
        free(*fname);

      if ((*fname=malloc(hdrlen))==NULL)
        return -1;

      memmove(*fname, ext+1, hdrlen-3);
      (*fname)[hdrlen-3]='\0';
    }

    /* The last two bytes of the header are the length of the NEXT    *
     * extended header...                                             */

    extsize += hdrlen;
    hdrlen=*(word *)((byte *)ext+hdrlen-2);
    
    if (!buf)
      free(ext);
  }
  
  return extsize;
}

