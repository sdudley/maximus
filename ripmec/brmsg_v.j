'barea_v.j
'Maximus browse areas selection, verbose version

 KillMouseFields
 FontStyle Default HorizDir 1

'Draw a nice box with an indent for the title
 Color DarkGray
 Rectangle  31 41 550 260
 Color White
 Rectangle  30 40 549 259
 FillStyle SolidFill LightGray
 Bar        31 41 549 259

 Color Black
 Rectangle  35 55 544 254
 Color White
 Rectangle  36 56 545 255
 Color LightGray
 Rectangle  36 56 544 254

'Drop in the text
 Color Yellow
 TextXY 38 44 BROWSE: Message search & retrieval
 Color Blue
 TextXY 48 62 Type of messages to show:

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
 ButtonStyle.Width 370
 ButtonStyle.Height 12
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF

'Buttons
 ButtonStyle.DFore Black
 ButtonStyle
 Button 90  80 0 0 A <>All messages in selected area(s)<>A
 Button 90 100 0 0 N <>New messages (everything since last read)<>N
 Button 90 120 0 0 Y <>Your mail (everything addressed to YOU)<>Y
 Button 90 140 0 0 S <>Search (specify to/from/subj/body keywords)<>S
 Button 90 160 0 0 F <>From a specified message # to the last msg<>F
 Button 90 180 0 0 Q <>Quit<>Q
 Button 90 200 0 0 ? <>?Help<>?



