; MEXBank Configuration file

; Configuration verbs:
;
;   MaxDepositKb    XXXX        ; Defines maximum deposit kb (daily)
;   MaxDepositTime  XXXX        ; Defines maximum deposit minutes (daily)
;   MaxWithdrawKb   XXXX        ; Defines maximum withdrawal kb (daily)
;   MaxWithdrawTime XXXX        ; Defines maximum withdrawal time (daily)
;   MinKb           XXXX        ; Minimum amount kb to keep
;   MinTime         XXXX        ; Minimum amount of time to keep
;
;   BankData        filename    ; Location of bank data file
;                               ; If no path, Max system dir is used


MaxDepositKb    1000
MaxDepositTime  60
MaxWithdrawKb   2000
MaxWithdrawTime 60
MinKb           0
MinTime         2

BankData        MexBank.Dat
BankHelp        Misc\Mexbank

; Format of MaxBank.Dat
;
; Each record is 128 bytes long, consisting of:
;
;   string[01..35]  User's name
;   string[36..44]  Date last used
;   string[45..53]  Kb Balance
;   string[54..62]  Time Balance
;   string[63..72]  Deposited today kb
;   string[73..80]  Deposited today time
;
