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

/*# name=ARC/PAK/ZIP/LZH structure definitions
*/

#include "typedefs.h"

/****************************************************************************/
/*************************** .ARJ information *******************************/
/****************************************************************************/

struct _arj_id
{
  #define ARJ_ID    0xea60u

  word id;
  word hdr_size;
};

struct _arj_hdr
{
  byte hdr_size;
  byte arj_ver;

  byte ver_to_extract;
  byte host_os;

  byte arj_flags;
  byte method;

  byte filetype;
  byte rsvd;

  union stamp_combo mod_date;
  dword comp_size;
  dword orig_size;
  dword file_crc;
  word entry_pos;
  word access_mode;
  word host_data;

  /* extra data */
  /* filename */
  /* comment */
  /* dword hdr_crc; */
  /* word exthdr_size; */
  /* byte exthdr[n]; */
  /* compressed file */
};

/****************************************************************************/
/*************************** .LZH information *******************************/
/****************************************************************************/

struct _lhpre
{
  byte hdrsize;
  byte sum;
};


struct _lh0
{
  byte  method[5];
  dword compressed_size;    /* packed */
  dword uncompressed_size;  /* original */
  union stamp_combo date;
  byte  attr;               /* attr */
  byte  hdr_ver;            /* level */
  byte  namelen;
};

struct _lh2
{
/*  5 */  byte  method[5];
/*  7 */  dword compressed_size;
/* 11 */  dword uncompressed_size;
/* 15 */  time_t date;
/* 19 */  byte  attr;
/* 20 */  byte  hdr_ver;
/* 21 */  word  crc;
/*-----------------------*/
/* 23 */  byte  hdrdos;   /* HDRdos */
/* 24 */  word  ext_size; /* HDRextsize */
};


/****************************************************************************/
/*************************** .ARC information *******************************/
/****************************************************************************/

struct _archead
{
  byte  sigbyte;            /* eof signature byte                */
  byte  method;             /* method of storage                 */
  byte  name[13];           /* name of file                      */
  dword compressed_size;    /* size compressed                   */
  word  date;               /* file date                         */
  word  time;               /* file time                         */
  word  crc;                /* crc check for data integrity      */
  dword uncompressed_size;  /* length of original file           */
};


/****************************************************************************/
/*************************** .ZIP information *******************************/
/****************************************************************************/

#define ZIP_SIG 0x04034b50L
#define DIR_SIG 0x02014B50L
#define END_SIG 0x06054B50L

#define ZIP_STRSIG "\x50\x4b\x03\x04"
#define DIR_STRSIG "\x50\x4b\x01\x02"
#define END_STRSIG "\x50\x4b\x05\x06"

struct _zip_header
{
  struct
  {
    word ver_to_extract;
    word bits;
    word comp_method;
    word last_mod_time;
    word last_mod_date;
    dword crc;
    dword compressed_size;
    dword uncompressed_size;
    word filename_length;
    word xtra_field_length;
  } info;

  char *filename;
  char *xtra_field;
};


struct _dir_header
{
  struct
  {
    word ver_made_by;
    word ver_to_extract;
    word bits;
    word comp_method;
    word last_mod_time;
    word last_mod_date;
    dword crc;
    dword compressed_size;
    dword uncompressed_size;
    word filename_length;
    word xtra_field_length;
    word file_comment_length;
    word disk_number;
    word file_attr_int;
    dword file_attr_ext;
    dword local_header_offset;
  } info;

  char *filename;
  char *xtra_field;
  char *comment;
};


struct _dir_end
{
  struct
  {
    word this_disk;
    word start_directory_disk;
    word total_entries_this_disk;
    word total_entries_this_dir;
    dword size_of_dir;
    dword offset_start_dir;
    word zipfile_comment_length;
  } info;

  char *file_comment;
};


/*

Tnanks to Phil Katz for this ZIP information.  Copied verbatim from
his APPNOTE.TXT:

General Format of a ZIP file
----------------------------

  Files stored in arbitrary order.  Large zipfiles can span multiple
  diskette media.

  Overall zipfile format:

    [local file header+file data] . . .
    [central directory] end of central directory record


  A.  Local file header:

        local file header signature     4 bytes  (0x04034b50)
        version needed to extract       2 bytes
        general purpose bit flag        2 bytes
        compression method              2 bytes
        last mod file time              2 bytes
        last mod file date              2 bytes
        crc-32                          4 bytes
        compressed size                 4 bytes
        uncompressed size               4 bytes
        filename length                 2 bytes
        extra field length              2 bytes

        filename (variable size)
        extra field (variable size)


  B.  Central directory structure:

      [file header] . . .  end of central dir record

      File header:

        central file header signature   4 bytes  (0x02014b50)
        version made by                 2 bytes
        version needed to extract       2 bytes
        general purpose bit flag        2 bytes
        compression method              2 bytes
        last mod file time              2 bytes
        last mod file date              2 bytes
        crc-32                          4 bytes
        compressed size                 4 bytes
        uncompressed size               4 bytes
        filename length                 2 bytes
        extra field length              2 bytes
        file comment length             2 bytes
        disk number start               2 bytes
        internal file attributes        2 bytes
        external file attributes        4 bytes
        relative offset of local header 4 bytes

        filename (variable size)
        extra field (variable size)
        file comment (variable size)

      End of central dir record:

        end of central dir signature    4 bytes  (0x06054b50)
        number of this disk             2 bytes
        number of the disk with the
        start of the central directory  2 bytes
        total number of entries in
        the central dir on this disk    2 bytes
        total number of entries in
        the central dir                 2 bytes
        size of the central directory   4 bytes
        offset of start of central
        directory with respect to
        the starting disk number        4 bytes
        zipfile comment length          2 bytes
        zipfile comment (variable size)




  C.  Explanation of fields:

      version made by

          The upper byte indicates the host system (OS) for the
          file.  Software can use this information to determine
          the line record format for text files etc.  The current
          mappings are:

          0 - IBM (MS-DOS)      1 - Amiga       2 - VMS
          3 - *nix              4 - VM/CMS
          5 thru 255 - unused

          The lower byte indicates the version number of the
          software used to encode the file.  The value/10
          indicates the major version number, and the value
          mod 10 is the minor version number.

      version needed to extract

          The minimum software version needed to extract the
          file, mapped as above.

      general purpose bit flag:

          bit 0: If set, indicates that the file is encrypted.
          bit 1: If the compression method used was type 6,
                 Imploding, then this bit, if set, indicates
                 an 8K sliding dictionary was used.  If clear,
                 then a 4K sliding dictionary was used.
          bit 2: If the compression method used was type 6,
                 Imploding, then this bit, if set, indicates
                 an 3 Shannon-Fano trees were used to encode the
                 sliding dictionary output.  If clear, then 2
                 Shannon-Fano trees were used.
          Note:  Bits 1 and 2 are undefined if the compression
                 method is other than type 6 (Imploding).

          The upper three bits are reserved and used internally
          by the software when processing the zipfile.  The
          remaining bits are unused in version 1.0.

      compression method:

          (see accompanying documentation for algorithm
          descriptions)

          0 - The file is stored (no compression)
          1 - The file is Shrunk
          2 - The file is Reduced with compression factor 1
          3 - The file is Reduced with compression factor 2
          4 - The file is Reduced with compression factor 3
          5 - The file is Reduced with compression factor 4
          6 - The file is Imploded

      date and time fields:

          The date and time are encoded in standard MS-DOS
          format.

      CRC-32:

          The CRC-32 algorithm was generously contributed by
          David Schwaderer and can be found in his excellent
          book "C Programmers Guide to NetBIOS" published by
          Howard W. Sams & Co. Inc.  The 'magic number' for
          the CRC is 0xdebb20e3.  The proper CRC pre and post
          conditioning is used, meaning that the CRC register
          is pre-conditioned with all ones (a starting value
          of 0xffffffff) and the value is post-conditioned by
          taking the one's complement of the CRC residual.

      compressed size:
      uncompressed size:

          The size of the file compressed and uncompressed,
          respectively.

      filename length:
      extra field length:
      file comment length:

          The length of the filename, extra field, and comment
          fields respectively.  The combined length of any
          directory record and these three fields should not
          generally exceed 65,535 bytes.

      disk number start:

          The number of the disk on which this file begins.

      internal file attributes:

          The lowest bit of this field indicates, if set, that
          the file is apparently an ASCII or text file.  If not
          set, that the file apparently contains binary data.
          The remaining bits are unused in version 1.0.

      external file attributes:

          The mapping of the external attributes is
          host-system dependent (see 'version made by').  For
          MS-DOS, the low order byte is the MS-DOS directory
          attribute byte.

      relative offset of local header:

          This is the offset from the start of the first disk on
          which this file appears, to where the local header should
          be found.

      filename:

          The name of the file, with optional relative path.
          The path stored should not contain a drive or
          device letter, or a leading slash.  All slashes
          should be forward slashes '/' as opposed to
          backwards slashes '\' for compatibility with Amiga
          and Unix file systems etc.

      extra field:

          This is for future expansion.  If additional information
          needs to be stored in the future, it should be stored
          here.  Earlier versions of the software can then safely
          skip this file, and find the next file or header.  This
          field will be 0 length in version 1.0.

      file comment:

          The comment for this file.


      number of this disk:

          The number of this disk, which contains central
          directory end record.

      number of the disk with the start of the central directory:

          The number of the disk on which the central
          directory starts.

      total number of entries in the central dir on this disk:

          The number of central directory entries on this disk.

      total number of entries in the central dir:

          The total number of files in the zipfile.


      size of the central directory:

          The size (in bytes) of the entire central directory.

      offset of start of central directory with respect to
      the starting disk number:

          Offset of the start of the central direcory on the
          disk on which the central directory starts.

      zipfile comment length:

          The length of the comment for this zipfile.

      zipfile comment:

          The comment for this zipfile.


  D.  General notes:

      1)  All fields unless otherwise noted are unsigned and stored
          in Intel low-byte:high-byte, low-word:high-word order.

      2)  String fields are not null terminated, since the
          length is given explicitly.

      3)  Local headers should not span disk boundries.  Also, even
          though the central directory can span disk boundries, no
          single record in the central directory should be split
          across disks.

      4)  The entries in the central directory may not necessarily
          be in the same order that files appear in the zipfile.

UnShrinking
-----------

Shrinking is a Dynamic Ziv-Lempel-Welch compression algorithm
with partial clearing.  The initial code size is 9 bits, and
the maximum code size is 13 bits.  Shrinking differs from
conventional Dynamic Ziv-lempel-Welch implementations in several
respects:

1)  The code size is controlled by the compressor, and is not
    automatically increased when codes larger than the current
    code size are created (but not necessarily used).  When
    the decompressor encounters the code sequence 256
    (decimal) followed by 1, it should increase the code size
    read from the input stream to the next bit size.  No
    blocking of the codes is performed, so the next code at
    the increased size should be read from the input stream
    immediately after where the previous code at the smaller
    bit size was read.  Again, the decompressor should not
    increase the code size used until the sequence 256,1 is
    encountered.

2)  When the table becomes full, total clearing is not
    performed.  Rather, when the compresser emits the code
    sequence 256,2 (decimal), the decompressor should clear
    all leaf nodes from the Ziv-Lempel tree, and continue to
    use the current code size.  The nodes that are cleared
    from the Ziv-Lempel tree are then re-used, with the lowest
    code value re-used first, and the highest code value
    re-used last.  The compressor can emit the sequence 256,2
    at any time.


Expanding
---------

The Reducing algorithm is actually a combination of two
distinct algorithms.  The first algorithm compresses repeated
byte sequences, and the second algorithm takes the compressed
stream from the first algorithm and applies a probabilistic
compression method.

The probabilistic compression stores an array of 'follower
sets' S(j), for j=0 to 255, corresponding to each possible
ASCII character.  Each set contains between 0 and 32
characters, to be denoted as S(j)[0],...,S(j)[m], where m<32.
The sets are stored at the beginning of the data area for a
Reduced file, in reverse order, with S(255) first, and S(0)
last.

The sets are encoded as { N(j), S(j)[0],...,S(j)[N(j)-1] },
where N(j) is the size of set S(j).  N(j) can be 0, in which
case the follower set for S(j) is empty.  Each N(j) value is
encoded in 6 bits, followed by N(j) eight bit character values
corresponding to S(j)[0] to S(j)[N(j)-1] respectively.  If
N(j) is 0, then no values for S(j) are stored, and the value
for N(j-1) immediately follows.

Immediately after the follower sets, is the compressed data
stream.  The compressed data stream can be interpreted for the
probabilistic decompression as follows:


let Last-Character <- 0.
loop until done
    if the follower set S(Last-Character) is empty then
        read 8 bits from the input stream, and copy this
        value to the output stream.
    otherwise if the follower set S(Last-Character) is non-empty then
        read 1 bit from the input stream.
        if this bit is not zero then
            read 8 bits from the input stream, and copy this
            value to the output stream.
        otherwise if this bit is zero then
            read B(N(Last-Character)) bits from the input
            stream, and assign this value to I.
            Copy the value of S(Last-Character)[I] to the
            output stream.

    assign the last value placed on the output stream to
    Last-Character.
end loop


B(N(j)) is defined as the minimal number of bits required to
encode the value N(j)-1.


The decompressed stream from above can then be expanded to
re-create the original file as follows:


let State <- 0.

loop until done
    read 8 bits from the input stream into C.
    case State of
        0:  if C is not equal to DLE (144 decimal) then
                copy C to the output stream.
            otherwise if C is equal to DLE then
                let State <- 1.

        1:  if C is non-zero then
                let V <- C.
                let Len <- L(V)
                let State <- F(Len).
            otherwise if C is zero then
                copy the value 144 (decimal) to the output stream.
                let State <- 0

        2:  let Len <- Len + C
            let State <- 3.

        3:  move backwards D(V,C) bytes in the output stream
            (if this position is before the start of the output
            stream, then assume that all the data before the
            start of the output stream is filled with zeros).
            copy Len+3 bytes from this position to the output stream.
            let State <- 0.
    end case
end loop


The functions F,L, and D are dependent on the 'compression
factor', 1 through 4, and are defined as follows:

For compression factor 1:
    L(X) equals the lower 7 bits of X.
    F(X) equals 2 if X equals 127 otherwise F(X) equals 3.
    D(X,Y) equals the (upper 1 bit of X) * 256 + Y + 1.
For compression factor 2:
    L(X) equals the lower 6 bits of X.
    F(X) equals 2 if X equals 63 otherwise F(X) equals 3.
    D(X,Y) equals the (upper 2 bits of X) * 256 + Y + 1.
For compression factor 3:
    L(X) equals the lower 5 bits of X.
    F(X) equals 2 if X equals 31 otherwise F(X) equals 3.
    D(X,Y) equals the (upper 3 bits of X) * 256 + Y + 1.
For compression factor 4:
    L(X) equals the lower 4 bits of X.
    F(X) equals 2 if X equals 15 otherwise F(X) equals 3.
    D(X,Y) equals the (upper 4 bits of X) * 256 + Y + 1.

Imploding
---------

The Imploding algorithm is actually a combination of two distinct
algorithms.  The first algorithm compresses repeated byte
sequences using a sliding dictionary.  The second algorithm is
used to compress the encoding of the sliding dictionary ouput,
using multiple Shannon-Fano trees.

The Imploding algorithm can use a 4K or 8K sliding dictionary
size. The dictionary size used can be determined by bit 1 in the
general purpose flag word, a 0 bit indicates a 4K dictionary
while a 1 bit indicates an 8K dictionary.

The Shannon-Fano trees are stored at the start of the compressed
file. The number of trees stored is defined by bit 2 in the
general purpose flag word, a 0 bit indicates two trees stored, a
1 bit indicates three trees are stored.  If 3 trees are stored,
the first Shannon-Fano tree represents the encoding of the
Literal characters, the second tree represents the encoding of
the Length information, the third represents the encoding of the
Distance information.  When 2 Shannon-Fano trees are stored, the
Length tree is stored first, followed by the Distace tree.

The Literal Shannon-Fano tree, if present is used to represent
the entire ASCII character set, and contains 256 values.  This
tree is used to compress any data not compressed by the sliding
dictionary algorithm.  When this tree is present, the Minimum
Match Length for the sliding dictionary is 3.  If this tree is
not present, the Minimum Match Length is 2.

The Length Shannon-Fano tree is used to compress the Length part
of the (length,distance) pairs from the sliding dictionary
output. The Length tree contains 64 values, ranging from the
Minimum Match Length, to 63 plus the Minimum Match Length.

The Distance Shannon-Fano tree is used to compress the Distance
part of the (length,distance) pairs from the sliding dictionary
output. The Distance tree contains 64 values, ranging from 0 to
63, representing the upper 6 bits of the distance value.  The
distance values themselves will be between 0 and the sliding
dictionary size, either 4K or 8K.

The Shannon-Fano trees themselves are stored in a compressed
format. The first byte of the tree data represents the number of
bytes of data representing the (compressed) Shannon-Fano tree
minus 1.  The remaining bytes represent the Shannon-Fano tree
data encoded as:

    High 4 bits: Number of values at this bit length + 1. (1 - 16)
    Low  4 bits: Bit Length needed to represent value + 1. (1 - 16)

The Shannon-Fano codes can be constructed from the bit lengths
using the following algorithm:

1)  Sort the Bit Lengths in ascending order, while retaining the
    order of the original lengths stored in the file.

2)  Generate the Shannon-Fano trees:

    Code <- 0
    CodeIncrement <- 0
    LastBitLength <- 0
    i <- number of Shannon-Fano codes - 1   (either 255 or 63)

    loop while i >= 0
        Code = Code + CodeIncrement
        if BitLength(i) <> LastBitLength then
            LastBitLength=BitLength(i)
            CodeIncrement = 1 shifted left (16 - LastBitLength)
        ShannonCode(i) = Code
        i <- i - 1
    end loop


3)  Reverse the order of all the bits in the above ShannonCode()
    vector, so that the most significant bit becomes the least
    significant bit.  For example, the value 0x1234 (hex) would
    become 0x2C48 (hex).

4)  Restore the order of Shannon-Fano codes as originally stored
    within the file.

Example:

    This example will show the encoding of a Shannon-Fano tree
    of size 8.  Notice that the actual Shannon-Fano trees used
    for Imploding are either 64 or 256 entries in size.

Example:   0x02, 0x42, 0x01, 0x13

    The first byte indicates 3 values in this table.  Decoding the
    bytes:
            0x42 = 5 codes of 3 bits long
            0x01 = 1 code  of 2 bits long
            0x13 = 2 codes of 4 bits long

    This would generate the original bit length array of:
    (3, 3, 3, 3, 3, 2, 4, 4)

    There are 8 codes in this table for the values 0 thru 7.  Using the
    algorithm to obtain the Shannon-Fano codes produces:

                                  Reversed     Order     Original
Val  Sorted   Constructed Code      Value     Restored    Length
---  ------   -----------------   --------    --------    ------
0:     2      1100000000000000        11       101          3
1:     3      1010000000000000       101       001          3
2:     3      1000000000000000       001       110          3
3:     3      0110000000000000       110       010          3
4:     3      0100000000000000       010       100          3
5:     3      0010000000000000       100        11          2
6:     4      0001000000000000      1000      1000          4
7:     4      0000000000000000      0000      0000          4


The values in the Val, Order Restored and Original Length columns
now represent the Shannon-Fano encoding tree that can be used for
decoding the Shannon-Fano encoded data.  How to parse the
variable length Shannon-Fano values from the data stream is beyond the
scope of this document.  (See the references listed at the end of
this document for more information.)  However, traditional decoding
schemes used for Huffman variable length decoding, such as the
Greenlaw algorithm, can be succesfully applied.

The compressed data stream begins immediately after the
compressed Shannon-Fano data.  The compressed data stream can be
interpreted as follows:

loop until done
    read 1 bit from input stream.

    if this bit is non-zero then       (encoded data is literal data)
        if Literal Shannon-Fano tree is present
            read and decode character using Literal Shannon-Fano tree.
        otherwise
            read 8 bits from input stream.
        copy character to the output stream.
    otherwise                   (encoded data is sliding dictionary match)
        if 8K dictionary size
            read 7 bits for offset Distance (lower 7 bits of offset).
        otherwise
            read 6 bits for offset Distance (lower 6 bits of offset).

        using the Distance Shannon-Fano tree, read and decode the
          upper 6 bits of the Distance value.

        using the Length Shannon-Fano tree, read and decode
          the Length value.

        Length <- Length + Minimum Match Length

        if Length = 63 + Minimum Match Length
            read 8 bits from the input stream,
            add this value to Length.

        move backwards Distance+1 bytes in the output stream, and
        copy Length characters from this position to the output
        stream.  (if this position is before the start of the output
        stream, then assume that all the data before the start of
        the output stream is filled with zeros).
end loop

References:

    Storer, James A. "Data Compression, Methods and Theory"

    Held, Gilbert  "Data Compression, Techniques and Applications,
                    Hardware and Software Consideraions"

*/


/*

Ä MUFFIN (1:249/106.1) ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ MUFFIN Ä
 Msg  : 215 of 230                  Rcv                                         
 From : Bert Gregory                1:142/470               28 Sep 92  16:23:58 
 To   : Scott Dudley                                        30 Sep 92  23:36:08 
 Subj : SQZ1082E                                                                
ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Scott, I am trying to add SQZ to maximus for users to be able to
view SQZ archives

Can you tell me what to put as "Ident" in Compress.Cfg to make this work?

The following Info is from the SQZ Tech Doc's.
------------------------------------------------------------------------- --
DOSversion:     3.3 and onwards,
                2.0 to 3.2 will work as long as you use UPPERCASE in all
                filenames and don't use SFX/SFXjr.

Memory:         For packing;    ÷400 kB, but more than 330 kB
                Decoding;       ÷96 kB

                These figures does not include SQZ itself which requires
                ÷50 kB and filenames which call for their length + 5 bytes
                in overhead (450/380 : 150).

                These figures can be shrinked at cost of speed.

Others:         Given filespec may result in a maximum of 4096 filenames
                per run. The archive in itself will handle any number
                of files.

                A maximum of 64 filespec's for files to skip.

                A maximum of 16 extension's for files to copy.
----------------------------------------------------------------------------- --
Archive header:
    offset  0..4:   Id:         HLSQZ       (072d 076d 083d 081d 090d)
            5:      Version:    1           (049d)
            6:      OS:     76543210
                            xxxxx000    0   PC-DOS
                            xxxxx001    1   OS/2
                            xxxxx010    2   MVS
                            xxxxx011    3   HPSF(OS/2)
                            xxxxx100    4   Amiga
                            xxxxx101    5   Macintosh
                            xxxxx110    6   *nix
                            xxxxx111    7   Future use
                            XXXXXxxx        Future use
            7:      Flag    76543210
                            xxxxxxx1        Big Endian
                            xxxxxx1x        DOS format for Date and Time
                                            in file: E8631E19.
                                            Decoded:    1992/08/30 12.31.32
                                            MSB                     LSB
                                            19      1E      63      E8
                                            76543210765432107654321076543210
                                            00011001000111100110001111101000
                                            YYYYYYYMMMMDDDDDHHHHHMMMMMM22222
                                            12     8   30   12   31    16
                                            +=1980                     *=2
                                            1992   08  30   12   31    32
                            xxxxx1xx        Security Envelope
                            XXXXXxxx        Future use
----------------------------------------------------------------------------- --
File header:
    offset  Size        Comment
    0       1           Header size and type
                        0       ->  End of archive
                        1       ->  Comment
                        2       ->  Password
                        3       ->  Security envelope
                        4..18   ->  future use
                        19..    ->  normal file header
                        if normal file
    1       1           Header algebraic sum  & 0FFh
    0       1:76543210
              xxxxXXXX  Method 0..4(15)
              xxx1xxxx  Security envelope should follow
              XXXxxxxx  Future use
    1       4           Compressed size
    5       4           Original size
    9       4           Last DateTime
    13      1           Attributes
    14      4           CRC
    18..    (size-18)   filename, w/o \0.


If  End of archive, done

If  > 18 normal file

    Read HeaderSum(1 byte)
    Read size bytes
    Calculate headersum
        {short i; unsigned short s = 0U;
        for(i = 0; i < size; i++)
            s += header[i];
        if(headersum != (unsigned char)s) WRONG HEADERSUM
    header[size] = '\0';    // just to makes things easier to handle, ie.
                            // zero terminate filename
    <= 18
    Next word gives number of bytes which are used, excluding this word
    COMMENT:
        0   2           Number of bytes in comment
                        Uncompressed size = this field - 7
        2   2           Number of bytes compressed
        4   1:76543210
              xxxxXXXX  Method 0..4(15)
              xxx1xxxx  Security envelope should follow
              XXXxxxxx  Future use
        5   4           CRC
        9   size-9      Comment

Thanks for your assistance....

Bert

--- DB 1.38/003165
 * Origin: ***Take A 'BYTE' BBS!*** (203)793-1790 (1:142/470)
SEEN-BY: 249/99 106 253/68 649/312 720/705
.PATH: 142/470 333 141/820 865 333 888 13/13 12/12 249/99 106


*/


