'barea.j
'Maximus browse areas selection, expert version

 KillMouseFields
 FontStyle Default HorizDir 1

'Draw a nice box with an indent for the title
 Color DarkGray
 Rectangle  31 161 550 260
 Color White
 Rectangle  30 160 549 259
 FillStyle SolidFill LightGray
 Bar        31 161 549 259

 Color Black
 Rectangle  35 175 544 254
 Color White
 Rectangle  36 176 545 255
 Color LightGray
 Rectangle  36 176 544 254

'Drop in the text
 Color Yellow
 TextXY 38 164 BROWSE
 Color Blue
 TextXY 50 182 Which areas:

'Set button fields
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Blue
 ButtonStyle.DBack DarkGray
 ButtonStyle.UnderLine OFF
 ButtonStyle.HighlightKey ON
 ButtonStyle.LeftJustify OFF
 ButtonStyle.BevSize 2
 ButtonStyle.Bevel ON
 ButtonStyle.Recess OFF
 ButtonStyle.Chisel OFF
 ButtonStyle.Sunken OFF
 ButtonStyle.Shadow OFF
 ButtonStyle.Bright White
 ButtonStyle.Dark Black
 ButtonStyle.Width 60
 ButtonStyle.Height 11
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF

'Buttons
 ButtonStyle.DFore Black
 ButtonStyle
 FontStyle Small HorizDir 4
 Button  50 200 0 0 C <>Current<>C
 Button 120 200 0 0 T <>Tagged<>T
 Button 190 200 0 0 W <>Wildcard<>W
 Button 260 200 0 0 G <>Group<>G
 Button 330 200 0 0 L <>Local<>L
 Button 400 200 0 0 A <>All<>A
 Button 470 200 0 0 Q <>Quit<>Q

'For the 'Which areas' prompt
 FontStyle Default HorizDir 1
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

