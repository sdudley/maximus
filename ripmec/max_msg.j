'MSGMENU.J
'Message menu/toolbar

 FillStyle SolidFill Black
 Bar        0 310 639 340
 FontStyle Default HorizDir 1
 KillMouseFields
 TextWindow 0 6 79 37 YES 0

'Background Block
 Color DarkGray
 Rectangle 01 310 639 336
 Color White
 Rectangle 00 309 638 335
 FillStyle SolidFill LightGray
 Bar       01 310 638 335
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
 ButtonStyle.Height 9
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF
'Buttons
 ButtonStyle.DFore Black
 ButtonStyle
 Button   5 312 0 0 N <>Next<>N
 Button   5 325 0 0 P <>Prev<>P
 Button  60 312 0 0 * <>*Curr<>*
 Button  60 325 0 0 = <>=Cont<>=
 Button 115 312 0 0 - <>-Orig<>-
 Button 115 325 0 0 + <>+Reply<>+
 Button 170 312 0 0 B <>Browse<>B
 Button 170 325 0 0 L <>List<>L
 Button 225 312 0 0 $ <>$RpEls<>$
 Button 225 325 0 0 T <>Tags<>T

 ButtonStyle.DFore Blue
 ButtonStyle
 Button 280 312 0 0 E <>Enter<>E
 Button 280 325 0 0 R <>Reply<>R
 Button 335 312 0 0 U <>Upload<>U
 Button 335 325 0 0 C <>Change<>C
 Button 390 312 0 0 K <>Kill<>K
 Button 390 325 0 0 F <>Fwd<>F
 Button 445 312 0 0 H <>Hurl<>H
 Button 445 325 0 0 X <>Xport<>X

 ButtonStyle.DFore Red
 ButtonStyle.Width 14
 ButtonStyle
 Button 500 312 0 0 ? <>?<>?
 Button 500 325 0 0 ! <>!<>!
 Button 519 312 0 0 @ <>@<>@
 Button 519 325 0 0 ^ <>^<>^

 ButtonStyle.DFore Brown
 ButtonStyle.Width 45
 ButtonStyle
 Button 537 312 0 0 M <>MAIN<>M
 Button 537 325 0 0 A <>Areas<>A
 Button 587 312 0 0 > <>Files<>>
 Button 587 325 0 0 G <>G'bye<>G

'EraseWindow
