'Main.J
' --- Main Menu ---

  KillMouseFields
  ResetWindows
  TextWindow 0 0 0 0 0 0

  'Background
    FillStyle ClosedotFill LightBlue
    Bar 0 0 639 335

  'Paint Screen areas for text
      FillStyle ClosedotFill Blue
      Bar  90 26 570 54
      Bar  40 70 620 330
      'Bar 4 98 +123 +17
      Color DarkGray
      Rectangle 81 17 556 42
      Rectangle 31 61 606 317
      Color White
      Rectangle 80 16 557 41
      Rectangle 30 60 605 316
      FillStyle SolidFill LightGray
      Bar  81 17 557 41
      Bar  31 61 605 316
      Color DarkGray
      Rectangle 36 76 601 312
      Color White
      Rectangle 37 77 601 312
      Color LightGray
      Rectangle 37 77 600 311

  'Paint in your BBS name
    FontStyle Triplex HorizDir 2
    Color White
    TextXY 86 16 Welcome to ... [sys_name]
    Color Blue
    TextXY 85 15 Welcome to ... [sys_name]

  'Paint in the menu title
    FontStyle Default HorizDir 1
    Color White
    TextXY 37 65 MAIN MENU:
    TextXY 411 65 [remain] minutes remaining
    Color Red
    TextXY 36 64 MAIN MENU:
    Color Black
    TextXY 410 64 [remain] minutes remaining

  'set button fields
    ButtonStyle.Mouse ON
    ButtonStyle.DFore Blue
    ButtonStyle.DBack White
    ButtonStyle.UlineCol Red
    ButtonStyle.UnderLine OFF
    ButtonStyle.HighLightKey ON
    ButtonStyle.LeftJustify ON
    ButtonStyle.Bevel ON
    ButtonStyle.BevSize 2
    ButtonStyle.Recess OFF
    ButtonStyle.Chisel OFF
    ButtonStyle.Sunken OFF
    ButtonStyle.Shadow ON
    ButtonStyle.Bright White
    ButtonStyle.Dark DarkGray
    ButtonStyle.Width 100
    ButtonStyle.Height 16
    ButtonStyle.Invertable ON
    ButtonStyle.Explode OFF
    ButtonStyle.Selected OFF
    ButtonStyle.Reset ON
    ButtonStyle.ADJ ON
    ButtonStyle

  'Paste button commands
    FontStyle Small HorizDir 4
    Button 110  95 0 0 M email.icn    <>Message areas<>m
    Button 110 140 0 0 F diskette.icn <>File areas<>f
    Button 110 185 0 0 C tnk.icn      <>Change setup<>c
    Button 110 230 0 0 B news.icn     <>Bulletins<>b
    Button 110 275 0 0 O ewrite.icn   <>Off-line reader<>o

    Button 300  95 0 0 G horizon.icn  <>Goodbye<>g
    Button 300 140 0 0 S noshuf.icn   <>Statistics<>s
    Button 300 185 0 0 U caticon.icn  <>User list<>u
    Button 300 230 0 0 $ dollar.icn   <>$MEX Test<>$
    Button 300 275 0 0 V blademtr.icn <>Version of BBS<>v

    Button 490  95 0 0 Y ge.icn       <>Yell for SysOp<>y
    Button 490 140 0 0 W guesswht.icn <>Who is On<>w
    Button 490 185 0 0 / trivia.icn   <>/Chat Menu<>/
    Button 490 230 0 0 ? helpicon.icn <>?Help!<>?
    Button 490 275 0 0 # golf.icn     <>#Sysop Menu<>#

  'put icons on the screen

   LoadIcon  42  80 CopyPut 0 email.icn
   LoadIcon  42 125 CopyPut 0 diskette.icn
   LoadIcon  42 170 CopyPut 0 tnk.icn
   LoadIcon  42 215 CopyPut 0 news.icn
   LoadIcon  42 260 CopyPut 0 ewrite.icn

   LoadIcon 230  80 CopyPut 0 horizon.icn
   LoadIcon 230 125 CopyPut 0 noshuf.icn
   LoadIcon 230 170 CopyPut 0 caticon.icn
   LoadIcon 230 215 CopyPut 0 dollar.icn
   LoadIcon 230 260 CopyPut 0 blademtr.icn

   LoadIcon 418  80 CopyPut 0 ge.icn
   LoadIcon 418 125 CopyPut 0 guesswht.icn
   LoadIcon 418 170 CopyPut 0 trivia.icn
   LoadIcon 418 215 CopyPut 0 helpicon.icn
   LoadIcon 418 260 CopyPut 0 golf.icn
