'brdisp_v.j
'Maximus browse display selection, verbose version

 KillMouseFields
 FontStyle Default HorizDir 1

'Draw a nice box with an indent for the title
 Color DarkGray
 Rectangle  31 41 550 170
 Color White
 Rectangle  30 40 549 169
 FillStyle SolidFill LightGray
 Bar        31 41 549 169

 Color Black
 Rectangle  35 55 544 164
 Color White
 Rectangle  36 56 545 165
 Color LightGray
 Rectangle  36 56 544 164

'Drop in the text
 Color Yellow
 TextXY 38 44 BROWSE: Message search & retrieval
 Color Blue
 TextXY 48 62 Display what:

'Set button fields
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Blue
 ButtonStyle.DBack DarkGray
 ButtonStyle.UnderLine OFF
 ButtonStyle.HighlightKey ON
 ButtonStyle.LeftJustify ON
 ButtonStyle.BevSize 2
 ButtonStyle.Bevel ON
 ButtonStyle.Recess OFF
 ButtonStyle.Chisel OFF
 ButtonStyle.Sunken OFF
 ButtonStyle.Shadow OFF
 ButtonStyle.Bright White
 ButtonStyle.Dark Black
 ButtonStyle.Width 380
 ButtonStyle.Height 12
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF

'Buttons
 ButtonStyle.DFore Black
 ButtonStyle
 Button 90  80 0 0 R <>Read (show entire message)<>R
 Button 90 100 0 0 L <>List (show message to/from/subj one per line)<>L
 Button 90 120 0 0 P <>Pack (pack in QWE format for download)<>P
 Button 90 140 0 0 Q <>Quit<>Q



