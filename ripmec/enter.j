'ENTER.J
'Press ENTER to continue prompt for Maximus

'Query 0 $SMF$

'set button fields
  KillMouseFields
  FontStyle Default HorizDir 1
  ButtonStyle.Reset OFF
  ButtonStyle.Mouse ON
  ButtonStyle.DFore Black
  ButtonStyle.DBack Black
  ButtonStyle.UlineCol White
  ButtonStyle.UnderLine OFF
  ButtonStyle.HighLightKey ON
  ButtonStyle.Bevel ON
  ButtonStyle.Bevsize 2
  ButtonStyle.Recess OFF
  ButtonStyle.Chisel OFF
  ButtonStyle.Sunken OFF
  ButtonStyle.Shadow OFF
  ButtonStyle.Bright White
  ButtonStyle.Dark DarkGray
  ButtonStyle.Width 635
  ButtonStyle.Height 10
  ButtonStyle.Invertable ON
  ButtonStyle.Explode OFF
  ButtonStyle.Selected OFF
  ButtonStyle.LeftJustify OFF
  ButtonStyle.ADJ ON
  ButtonStyle

'Paste button commands
  Button 01 324 0 0 O <>Press OK to continue<>^M

'Restore previous screen
 FillStyle SolidFill Black
 Bar     00 320 639 350
'Query 0 $RMF$
