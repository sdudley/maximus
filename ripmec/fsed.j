'fsed.j
'Screen design for MaxEd fullscreen editor

 KillMouseFields
 FontStyle Default HorizDir 1
 KillMouseFields
'Button bar
 FillStyle SolidFill Black
 Bar        0 322 639 350
 FontStyle Default HorizDir 1
 TextWindow 0 7 79 40 NO 0

'Background Block
 Color DarkGray
 Rectangle 01 322 639 334
 Color White
 Rectangle 00 321 638 333
 FillStyle SolidFill LightGray
 Bar       01 322 638 333
'Set button fields
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Blue
 ButtonStyle.DBack White
 ButtonStyle.UnderLine OFF
 ButtonStyle.HighlightKey ON
 ButtonStyle.BevSize 2
 ButtonStyle.Bevel ON
 ButtonStyle.Recess OFF
 ButtonStyle.Chisel OFF
 ButtonStyle.Sunken OFF
 ButtonStyle.Shadow OFF
 ButtonStyle.Bright White
 ButtonStyle.Dark Black
 ButtonStyle.Width 60
 ButtonStyle.Height 8
 ButtonStyle.Invertable ON
 ButtonStyle.Explode ON
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF
 ButtonStyle
'Buttons
 FontStyle Small HorizDir 4
 Button   5 324 0 0 #0 <>Save<>^z
 Button  70 324 0 0 #0 <>Abort<>^[^[
 Button 135 324 0 0 #0 <>Quote<>^k^r
 Button 200 324 0 0 #0 <>Copy<>^k^c
 Button 265 324 0 0 #0 <>PgDn<>^c
 Button 330 324 0 0 #0 <>PgUp<>^r
 Button 395 324 0 0 #0 <>Redraw<>^w
 Button 460 324 0 0 #0 <>Menu<>^k^h
 Button 525 324 0 0 #0 <>Help<>^n
 Color White
 TextXY 597 321 MaxEd
 Color Red
 TextXY 596 320 MaxEd
 FontStyle Default HorizDir 1

'EraseWindow

