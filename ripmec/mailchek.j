  'Check for mail popup

  FontStyle Default HorizDir 1
  Color DarkGray
  Rectangle 26 26 440 236
  Color White
  Rectangle 25 25 439 235
  FillStyle SolidFill LightGray
  Bar 26 26 439 235
  Color Black
  Rectangle 30 30 434 230
  Color White
  Rectangle 31 31 435 231
  Color LightGray
  Rectangle 31 31 434 230

  LoadIcon 37 37 0 0 mailbox.icn

  Color Blue
  TextXY 200 40 Would you like to check the
  TextXY 200 50 message areas for your mail?

  KillMouseFields
  'set button fields
  ButtonStyle.Mouse ON
  ButtonStyle.DFore Blue
  ButtonStyle.DBack White
  ButtonStyle.UlineCol Red
  ButtonStyle.UnderLine OFF
  ButtonStyle.HighLightKey ON
  ButtonStyle.LeftJustify OFF
  ButtonStyle.RightJustify OFF
  ButtonStyle.Bevel ON
  ButtonStyle.BevSize 2
  ButtonStyle.Recess OFF
  ButtonStyle.Chisel OFF
  ButtonStyle.Sunken OFF
  ButtonStyle.Shadow ON
  ButtonStyle.Bright White
  ButtonStyle.Dark DarkGray
  ButtonStyle.Width 200
  ButtonStyle.Height 16
  ButtonStyle.Invertable ON
  ButtonStyle.Explode OFF
  ButtonStyle.Selected OFF
  ButtonStyle.Reset ON
  ButtonStyle.ADJ ON
  ButtonStyle

 'Paste button commands
  FontStyle Default HorizDir 1
  Button 210  80 0 0 N <>Not right now, thanks!<>n
  Button 210 120 0 0 A <>Check All areas<>a
  Button 210 160 0 0 L <>Check Local areas only<>l

  Color DarkGray
  TextXY 200 215 [sys_name]





