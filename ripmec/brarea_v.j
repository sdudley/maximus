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
 TextXY 48 62 Which areas do you wish to search:

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
 ButtonStyle.Width 350
 ButtonStyle.Height 12
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF

'Buttons
 ButtonStyle.DFore Black
 ButtonStyle
 Button 90  80 0 0 C <>Current area only<>C
 Button 90 100 0 0 T <>Tagged areas selected through T)ag command<>T
 Button 90 120 0 0 W <>Select areas by Wildcard<>W
 Button 90 140 0 0 G <>Select areas in a specific Group<>G
 Button 90 160 0 0 L <>Local and netmail areas only<>L
 Button 90 180 0 0 A <>Search All areas<>A
 Button 90 200 0 0 Q <>Quit<>Q

'For the 'Which areas' prompt
 Color Blue
 TextXY 48 222 Which areas (use "*" as wildcard):
 Color DarkGray
 Rectangle  45 237 538 250
 Color White
 Rectangle  44 236 537 249
 Color LightGray
 Rectangle  45 237 537 249
 TextWindow 6 30 66 30 NO 0
 EraseWindow


