'MAX_FILE.J
'File menu

 FillStyle SolidFill Black
 Bar        0 310 639 340
 FontStyle Default HorizDir 1
 KillMouseFields
 TextWindow 0 2 79 37 YES 0

'Background Block
 Color DarkGray
 Rectangle 01 313 639 337
 Color White
 Rectangle 00 312 638 336
 FillStyle SolidFill LightGray
 Bar       01 312 638 336
'Set button fields
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Blue
 ButtonStyle.DBack DarkGray
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
 ButtonStyle.Width 50
 ButtonStyle.Height 8
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF

'Buttons
 ButtonStyle.DFore Blue
 ButtonStyle
 Button   6 315 0 0 F <>Files<>F
 Button   6 327 0 0 L <>Locate<>L

 ButtonStyle.Width 80
 ButtonStyle
 Button  64 315 0 0 V <>View file<>V
 Button  64 327 0 0 C <>Contents<>C
 Button 147 315 0 0 T <>Tag files<>T
 Button 147 327 0 0 N <>New files<>N

 ButtonStyle.Width 70
 ButtonStyle.DFore Red
 ButtonStyle.UlineCol Yellow
 ButtonStyle
 Button 230 315 0 0 D <>Download<>D
 Button 230 327 0 0 U <>Upload<>U

 ButtonStyle.Width 50
 ButtonStyle.DFore Black
 ButtonStyle.UlineCol Red
 ButtonStyle
 Button 304 315 0 0 S <>Stats<>S

 ButtonStyle.Width 72
 ButtonStyle.DFore Blue
 ButtonStyle
 Button 358 315 0 0 R <>Raw dir<>R
 Button 358 327 0 0 O <>Override<>O

 ButtonStyle.Width 50
 ButtonStyle
 Button 434 315 0 0 H <>Hurl<>H
 Button 434 327 0 0 K <>Kill<>K

 ButtonStyle.DFore Red
 ButtonStyle.Width 10
 ButtonStyle
 Button 499 315 0 0 ? <>?<>?

 ButtonStyle.DFore Brown
 ButtonStyle.Width 50
 ButtonStyle
 Button 528 315 0 0 M <>MAIN<>M
 Button 528 327 0 0 A <>Areas<>A
 Button 582 315 0 0 \< <><Msgs<><
 Button 582 327 0 0 G <>G'bye<>G
