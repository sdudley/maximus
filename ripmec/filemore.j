'FILEMORE.J
'Message menu/toolbar

      KillMouseFields
      FontStyle Default Horizdir 1
    'Background Block
      FillStyle SolidFill Black
      Bar       0 310 640 350
      Color DarkGray
'      Rectangle 1 322 270 334
      Rectangle 1 322 165 334
      Color White
'      Rectangle 0 321 269 333
      Rectangle 0 321 164 333
      FillStyle SolidFill LightGray
'      Bar       1 322 269 333
      Bar       1 322 164 333
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
'      ButtonStyle.Width 96
'      ButtonStyle
'      Button 167 324 0 0 T <>Tag Files<>T
