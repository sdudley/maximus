'Message and/or File area selection in RIP

 KillMouseFields
 FontStyle Default Horizdir 1

'Main box
 Color Darkgray
 Rectangle 21 59 531 299
 Color White
 Rectangle 20 58 530 298
 FillStyle SolidFill LightGray
 Bar       21 59 530 298

'Title
 Color Red
 TextXY    25 61 MESSAGE AREA SELECTION
 Color Magenta
 TextXY   210 61 <section name>

'Inner border
 Color DarkGray
 Rectangle 22 70 525 273
 Color White
 Rectangle 23 71 526 274
 Color LightGray
 Rectangle 23 71 525 273

'Set standard button fields
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Blue
 ButtonStyle.DBack White
 ButtonStyle.UnderLine OFF
 ButtonStyle.HighlightKey ON
 ButtonStyle.BevSize 2
 ButtonStyle.Bevel ON
 ButtonStyle.Recess ON
 ButtonStyle.Chisel OFF
 ButtonStyle.Sunken OFF
 ButtonStyle.Shadow ON
 ButtonStyle.Bright White
 ButtonStyle.Dark Black
 ButtonStyle.Width 92
 ButtonStyle.Height 12
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ ON
 ButtonStyle.Reset OFF
'Buttons
 ButtonStyle.DFore Black
 ButtonStyle
 Button  28 281 0 0 Q <>Quit<>^M
 Button 128 281 0 0 A <>All Areas<>=^M
 Button 228 281 0 0 M <>More Areas<>*^M
 Button 228 281 0 0 A <>List Again<>*^M
 Button 328 281 0 0 U <>Level Up<>..^M
 Button 428 281 0 0 T <>Top Level<>/^M

'Areas
 ButtonStyle.Mouse ON
 ButtonStyle.DFore Blue
 ButtonStyle.DBack White
 ButtonStyle.UnderLine OFF
 ButtonStyle.HighlightKey OFF
 ButtonStyle.BevSize 2
 ButtonStyle.Bevel OFF
 ButtonStyle.Recess OFF
 ButtonStyle.Chisel OFF
 ButtonStyle.Sunken OFF
 ButtonStyle.Shadow OFF
 ButtonStyle.Bright White
 ButtonStyle.Dark Black
 ButtonStyle.Width 500
 ButtonStyle.Height 8
 ButtonStyle.Invertable ON
 ButtonStyle.Explode OFF
 ButtonStyle.Selected OFF
 ButtonStyle.ADJ OFF
 ButtonStyle.Reset OFF
 ButtonStyle.LeftJustify ON
 ButtonStyle

 Button 24  72 0 0 #0 <>Unchecked/Unsorted File Uploads<>SYSOP.Uploads^M
 Button 24  80 0 0 #0 <>Decadence: Information & Files List<>DEC.Info^M
 Button 24  88 0 0 #0 <>Decadence: Offline Mail Readers & Support<>DEC.Olr^M
 Button 24  96 0 0 #0 <>General: ANSI Art, Pictures & Animations<>GEN.Ansi^M
 Button 24 104 0 0 #0 <>General: Archivers & BBS Utilities<>GEN.Arc^M
 Button 24 112 0 0 #0 <>General: Astrological Files<>GEN.Astr^M
 Button 24 120 0 0 #0 <>General: Crafts & Hobbies<>GEN.Craft^M
 Button 24 128 0 0 #0 <>General: Red Dwarf Fandom<>GEN.Dwarf^M
 Button 24 136 0 0 #0 <>General: Editing Software<>GEN.Edit^M
 Button 24 264 0 0 #0 <>--This is the last area--<>^M
