'STAT.J
'User Statistics

'Reset everything
 ResetWindows
 FontStyle Default HorizDir 1

'Paint the sign
 Color Blue
 Rectangle 01 01 639 10
 Color LightCyan
 Rectangle 00 00 638 09
 FillStyle SolidFill LightBlue
 Bar       01 01 638 09

'Now the statistics box
 Color DarkGray
 Rectangle 21 17 620 270
 Color White
 Rectangle 20 16 619 269
 FillStyle SolidFill LightGray
 Bar       21 17 619 269

 Color DarkGray
 Rectangle 29 23 612 262
 Color White
 Rectangle 30 24 613 263


'Now the "Your statistics for ...." bit
 Color Yellow
 TextXY 4 02 Your statistics for

'TIME statistics

 Color Black
 Rectangle 48 32 227 44
 Color White
 Rectangle 49 33 228 45
 FillStyle SolidFill DarkGray
 Bar       49 33 227 44
 Color Yellow
 TextXY    68 35 TIME

 Move 100 50
 Color Blue
 Text Time on line, this call
 Color DarkGray
 Text .........

 Move 100 58
 Color Blue
 Text Time remaining for this call
 Color DarkGray
 Text ....

 Move 100 66
 Color Blue
 Text Time of previous calls today
 Color DarkGray
 Text ....

 Move 100 74
 Color Blue
 Text Calls to date
 Color DarkGray
 Text ...................


'FILES statistics

 Color Black
 Rectangle 48 88 227 100
 Color White
 Rectangle 49 89 228 101
 FillStyle SolidFill DarkGray
 Bar       49 89 227 100
 Color Yellow
 TextXY    68 91 FILES

 Move 100 106
 Color Blue
 Text UL (from you)
 Color DarkGray
 Text ...................

 Move 100 114
 Color Blue
 Text DL (to you)
 Color DarkGray
 Text .....................

 Move 100 122
 Color Blue
 Text DL today
 Color DarkGray
 Text ........................

 Move 100 130
 Color Blue
 Text DL available for today
 Color DarkGray
 Text ..........

'MATRIX statistics

 Color Black
 Rectangle 48 144 227 156
 Color White
 Rectangle 49 145 228 157
 FillStyle SolidFill DarkGray
 Bar       49 144 227 156
 Color Yellow
 TextXY    68 147 MATRIX

 Move 100 162
 Color Blue
 Text Credits (on account)
 Color DarkGray
 Text ............

 Move 100 170
 Color Blue
 Text Debits (expenditures)
 Color DarkGray
 Text ...........

'SUBSCRIPTION statistics

 Color Black
 Rectangle 48 184 227 196
 Color White
 Rectangle 49 185 228 197
 FillStyle SolidFill DarkGray
 Bar       49 184 227 196
 Color Yellow
 TextXY    68 187 SUBSCRIPTION

 Move 100 202
 Color Blue
 Text Minutes remaining
 Color DarkGray
 Text ...............

 Move 100 210
 Color Blue
 Text Expiry date
 Color DarkGRay
 Text .....................


'MISCELLANEOUS statistics

 Color Black
 Rectangle 48 224 227 236
 Color White
 Rectangle 49 225 228 237
 FillStyle SolidFill DarkGray
 Bar       49 224 227 236
 Color Yellow
 TextXY    68 227 MISCELLANEOUS

 Move 100 242
 Color Blue
 Text Last password change
 Color DarkGray
 Text ............
