'HEADMSG.J
'Message area header

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
'Now the "The MESSAGE Section" bit
 Color Yellow
 TextXY 4 19 The MESSAGE Section

'Now draw status line
 Color DarkGray
 Rectangle 01 01 639 12
 Color White
 Rectangle 00 00 638 11
 FillStyle SolidFill LightGray
 Bar       01 01 638 11
 Move      04 03
 Color Green
 Text Msg Area
 Color Magenta
 Text P.C_PLUSPLUS
 Color Black
 Text :
 Color Blue
 Text C++ Language Programming

 Color Yellow
 TextXY 545 04 00000/99999

'There are no messages in this area
 Color White
 TextXY 4 40 There are no messages in this area.
'You haven't read any of these
 Color LightGreen
 TextXY 4 50 You haven't read any of these.

 'Window for message stats
  TextWindow 0 5 79 37 NO 0
  Home


'TextWindow 00 02 79 41 YES 0
'EraseWindow

