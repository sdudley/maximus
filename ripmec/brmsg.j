'brmsg_v.j
'Maximus browse msg selection, brief version

 KillMouseFields
 FontStyle Default HorizDir 1

'Draw a nice box with an indent for the title
 Color DarkGray
 Rectangle  31 161 550 220
 Color White
 Rectangle  30 160 549 229
 FillStyle SolidFill LightGray
 Bar        31 161 549 229

 Color Black
 Rectangle  35 175 544 224
 Color White
 Rectangle  36 176 545 225
 Color LightGray
 Rectangle  36 176 544 224

'Drop in the text
 Color Yellow
 TextXY 38 164 BROWSE
 Color Blue
 TextXY 50 182 Type:

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
 Button  50 200 0 0 A <>All<>A
 Button 120 200 0 0 N <>New<>N
 Button 190 200 0 0 Y <>Your mail<>Y
 Button 260 200 0 0 S <>Search<>S
 Button 330 200 0 0 F <>From msg#<>F
 Button 400 200 0 0 Q <>Quit<>Q
 Button 470 200 0 0 ? <>?Help<>?

 FontStyle Default HorizDir 1


