'HEADFILE.J
'File area header

'Reset everything
 ResetWindows
 FontStyle Default HorizDir 1
'Paint the sign
 Color DarkGray
 Rectangle 01 17 639 28
 Color LightBlue
 Rectangle 00 16 638 27
 FillStyle SolidFill Blue
 Bar       01 17 638 27
'Now the "The FILES Section" bit
 Color Yellow
 TextXY 4 19 The FILES Section

'Now draw status line
 Color DarkGray
 Rectangle 01 01 639 12
 Color White
 Rectangle 00 00 638 11
 FillStyle SolidFill LightGray
 Bar       01 01 638 11
 Move      04 03
 Color Green
 Text File Areaÿ
 Color Magenta
 Text DOS.GAMES
 Color Black
 Text :ÿ
 Color Blue
 Text General: MS-DOS Games

'TextWindow 00 02 79 28 YES 0
'EraseWindow

'  FillStyle SolidFill Black
'  Bar 0 315 640 350
'  TextWindow 0 9 80 28 YES 0

