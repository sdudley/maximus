'MSGMORE.J
'Message area "more" prompt

      KillMouseFields
      FontStyle Default HorizDir 1
    'Background Block
      FillStyle SolidFill Black
      Bar       0 316 630 350
      Color DarkGray
      Rectangle 1 322 639 334
      Color White
      Rectangle 0 321 638 333
      FillStyle SolidFill LightGray
      Bar       1 322 638 333
      Color Blue
      TextXY 10 324 More?

    'set button fields
      ButtonStyle.Mouse ON
      ButtonStyle.DFore Blue
      ButtonStyle.DBack DarkGray
      ButtonStyle.UnderLine OFF
      ButtonStyle.HighLightKey ON
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
      ButtonStyle.DFore Black
      ButtonStyle
      Button  57 324 0 0 Y <>Yes<>Y
      Button 112 324 0 0 N <>No<>N
      Button 167 324 0 0 = <>=Cont<>=
      Button 222 324 0 0 * <>*Again<>*
      Button 277 324 0 0 P <>Prev<>^@K
      Button 332 324 0 0 E <>Next<>^@M
      Button 387 324 0 0 - <>-orig<>-
      Button 442 324 0 0 + <>+reply<>+
      Button 497 324 0 0 R <>Reply<>R
      ButtonStyle.Width 82
      ButtonStyle
      Button 552 324 0 0 % <>PvtReply<>%
