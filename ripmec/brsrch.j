'brsrch.j
'Maximus browse search, terse version

 KillMouseFields
 FontStyle Default HorizDir 1

'Draw a nice box with an indent for the title
 Color DarkGray
 Rectangle  31 121 550 260
 Color White
 Rectangle  30 120 549 259
 FillStyle SolidFill LightGray
 Bar        31 121 549 259

 Color Black
 Rectangle  35 135 544 254
 Color White
 Rectangle  36 136 545 255
 Color LightGray
 Rectangle  36 136 544 254

'Drop in the text
 Color Yellow
 TextXY 38 124 BROWSE
 Color Blue
 TextXY 48 142 Search where (any or all of):

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
 ButtonStyle.Width 70
 ButtonStyle.Height 12
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF
 ButtonStyle.CheckBoxGroup ON

'Buttons
 ButtonStyle.DFore Black
 ButtonStyle.Mouse ON
 ButtonStyle.Selected OFF
 ButtonStyle
 Button  90 160 0 0 T <>To<>[[1:]T
 Button 190 160 0 0 F <>From<>[[2:]F
 Button 290 160 0 0 S <>Subject<>[[3:]S
 Button 390 160 0 0 B <>Body<>[[4:]B

 ButtonStyle.Mouse OFF
 ButtonStyle.Selected ON
 ButtonStyle
 Button 360 650 -1 -1 #0<><>[[5:]^M

 ButtonStyle.CheckBoxGroup OFF
 ButtonStyle.DFore Red
 ButtonStyle.UlineCol White
 ButtonStyle.Width 100
 ButtonStyle.LeftJustify OFF
 ButtonStyle.Selected OFF
 ButtonStyle
 Button  90 200 0 0 ? <>?Help<>^X?^M
 Button 225 200 0 0 Q <>Quit<>^XQ^M
 Button 360 200 0 0 O <>Ok!<>[[12340]

'For the 'Which areas' prompt
' Color Blue
' TextXY 48 222 Which areas (use "*" as wildcard):
' Color DarkGray
' Rectangle  45 237 538 250
' Color White
' Rectangle  44 236 537 249
' Color LightGray
' Rectangle  45 237 537 249
' TextWindow 6 30 66 30 NO 0
'EraseWindow
