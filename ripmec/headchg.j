'CHANGE.J
'Change menu/toolbar
  'Reset everything
   KillMouseFields
   ResetWindows
   TextWindow 0 0 0 0 0 0
  'Paint the sign
   Color DarkGray
   Rectangle 0 0 638 11
   Color LightBlue
   Rectangle 0 0 637 10
   FillStyle SolidFill Blue
   Bar 1 1 638 9
  'Now the "The CHANGE SETUP Section" bit
   Color Yellow
   TextXY 4 2 The CHANGE SETUP Section

  'Main panel
   Color DarkGray
   Rectangle 0 14 638 259
   Color White
   Rectangle 0 14 637 258
   FillStyle Solidfill LightGray
   Bar 1 15 638 258
   Color DarkGray
   Rectangle 4 18 633 254
   Color White
   Rectangle 5 19 634 255
   Color LightGray
   Rectangle 5 19 633 254

'Now create all of the change fields as buttons
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Black
 ButtonStyle.DBack DarkGray
 ButtonStyle.UnderLine ON
 ButtonStyle.HighlightKey ON
 ButtonStyle.RightJustify ON
 ButtonStyle.BevSize 2
 ButtonStyle.Bevel ON
 ButtonStyle.Recess OFF
 ButtonStyle.Chisel OFF
 ButtonStyle.Sunken OFF
 ButtonStyle.Shadow OFF
 ButtonStyle.Bright White
 ButtonStyle.Dark DarkGray
 ButtonStyle.Width 200
 ButtonStyle.Height 14
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF
 ButtonStyle

 Color Yellow

 Button  10  30 0 0 A <>Alias<>A
 TextXY 220  34 Traveller
 Button  10  50 0 0 T <>Voice Telephone<>T
 TextXY 220  54 (03) 9938-9828
 Button  10  70 0 0 C <>City, State<>C
 TextXY 220  74 Kiesson City, Victoria
 Button  10  90 0 0 B <>Birth date<>B
 TextXY 220  94 1959.08.06

 ButtonStyle.Width 130
 ButtonStyle.DFore Blue
 ButtonStyle

 Color Magenta

 Button  10 130 0 0 H <>Help level<>H
 TextXY 150 134 NOVICE
 Button  10 150 0 0 W <>screen Width<>W
 TextXY 150 154 80
 Button  10 170 0 0 V <>Video mode<>V
 TextXY 150 174 ANSI
 Button  10 190 0 0 F <>Fullscrn Edit<>F
 TextXY 150 194 YES
 Button  10 210 0 0 $ <>$Protocol<>$
 TextXY 150 214 Zmodem
 Button  10 230 0 0 & <>&Archiver<>&
 TextXY 150 234 ZIP

 Button 220 130 0 0 N <>Nulls<>N
 TextXY 360 134 0
 Button 220 150 0 0 L <>screen Length<>L
 TextXY 360 154 43
 Button 220 170 0 0 * <>*More prompt<>*
 TextXY 360 174 YES
 Button 220 190 0 0 I <>IBM-PC chars<>I
 TextXY 360 194 YES
 Button 220 210 0 0 @ <>@Language<>@
 TextXY 360 214 English
 Button 220 230 0 0 G <>Gender<>G
 TextXY 360 234 Male

 Button 450 130 0 0 % <>%In userlist<>%
 TextXY 590 134 YES
 Button 450 150 0 0 #0 <>Tabs<>
 TextXY 590 154 YES
 Button 450 170 0 0 S <>Screen clear<>S
 TextXY 590 174 YES
 Button 450 190 0 0 ! <>!Hotkeys<>\!
 TextXY 590 194 YES
 Button 450 210 0 0 ^ <>^F/S Reader<>\^
 TextXY 590 214 YES
 Button 450 230 0 0 R <>Rip grapics<>R
 TextXY 590 234 YES

 TextWindow 0 33 80 43 YES 0
 RawText Hello
