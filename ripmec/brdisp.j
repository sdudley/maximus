'BRDISP.J
'Browe get display type

 KillMouseFields
 FontStyle Default HorizDir 1

'Draw a nice box with an indent for the title
 Color DarkGray
 Rectangle  31 161 350 220
 Color White
 Rectangle  30 160 349 219
 FillStyle SolidFill LightGray
 Bar        31 161 349 219

 Color Black
 Rectangle  35 175 344 214
 Color White
 Rectangle  36 176 345 215
 Color LightGray
 Rectangle  36 176 344 214

'Drop in the text
 Color Yellow
 TextXY 38 164 BROWSE
 Color Blue
 TextXY 50 182 Display:

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
 Button  50 196 0 0 R <>Read<>R
 Button 120 196 0 0 L <>List<>L
 Button 190 196 0 0 P <>Pack-QWK<>P
 Button 260 196 0 0 Q <>Quit<>Q

 FontStyle Default HorizDir 1
