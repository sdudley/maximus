'MSGHDR.J
'--- Message menu in Maximus ---

 FontStyle Default HorizDir 1
 KillMouseFields

'Redraw status line
 Color DarkGray
 Rectangle 01 01 639 12
 Color White
 Rectangle 00 00 638 11
 FillStyle SolidFill LightGray
 Bar       01 01 638 11
 Move      04 03
 Color Green
 Text Msg Areaÿ
 Color Magenta
 Text P.C_PLUSPLUS
 Color Black
 Text :ÿ
 Color Blue
 Text C++ Language Programming

'Paint main viewer window
 'Panels
  Color DarkGray
  Rectangle 01 14 639 55
  Color White
  Rectangle 00 13 638 54
  FillStyle SolidFill LightGray
  Bar       01 14 638 54
  Color Darkgray
  Line      04 50 633 50
  Color White
  Line      05 51 634 51

 'Field Labels
  Color Black
  TextXY 04 16 Msg#:
  Color Black
  TextXY 92 16 high
  Color Black
  TextXY 04 24 From:
  Color Black
  TextXY 04 32 To:
  Color Black
  TextXY 04 40 Subj:

 'Field contents
  Color Blue
  TextXY  48 16 00000
  Color Blue
  TextXY 128 16 00000
  Color Magenta
  TextXY 180 16 - 10000 + 20000

  FillStyle SolidFill LightGray
  Bar    311 16 624 23
  Color LightGreen
  TextXY 312 16 Pvt Crash w/File

  FillStyle SolidFill LightGray
  Bar    47 24 300 31
  Color Blue
  TextXY 48 24 12345678901234567890123456789012

  FillStyle SolidFill LightGray
  Bar    311 24 457 31
  Color Brown
  TextXY 312 24 14 Jan 95 15:22:32

  FillStyle SolidFill LightGray
  Bar    463 24 624 31
  Color Red
  TextXY 464 24 3:632/348

  FillStyle SolidFill LightGray
  Bar    47 32 300 39
  Color Blue
  TextXY 48 32 12345678901234567890123456789012

  FillStyle SolidFill LightGray
  Bar    311 32 457 39
  Color Brown
  TextXY 312 32 14 Jan 95 15:22:32

  FillStyle SolidFill LightGray
  Bar    463 32 624 39
  Color Red
  TextXY 464 32 3:632/348

  FillStyle SolidFill LightGray
  Bar    47 40 624 47
  Color Blue
  TextXY 48 40 12345678901234567890123456789012345678901234567890123456789012345678901
