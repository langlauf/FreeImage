VERSION 5.00
Begin VB.Form FormFreeImage 
   Caption         =   "FreeImageTest"
   ClientHeight    =   1500
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   2955
   LinkTopic       =   "Form1"
   ScaleHeight     =   1500
   ScaleWidth      =   2955
   StartUpPosition =   3  'Windows-Standard
   Begin VB.CommandButton CommandConvert 
      Caption         =   "Convert earth.jpg -> bmp"
      Height          =   375
      Left            =   360
      TabIndex        =   1
      Top             =   840
      Width           =   2055
   End
   Begin VB.CommandButton CommandShowDetails 
      Caption         =   "ShowDetails"
      Height          =   375
      Left            =   360
      TabIndex        =   0
      Top             =   240
      Width           =   2055
   End
End
Attribute VB_Name = "FormFreeImage"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub CommandShowDetails_Click()
    Dim obj As New FREEIMAGE_ACTIVEXLib.Application
    obj.Test "hello from within the freeimage-ActiveX-server called from VB", "My title"
    Set obj = Nothing

End Sub

Private Sub CommandConvert_Click()
    
    Dim obj As New FREEIMAGE_ACTIVEXLib.Image
    Dim erg As Integer
    
    On Error Resume Next
    erg = obj.Open("earth.jpg")
    If Err.Number <> 0 Or erg <> 1 Then
        MsgBox "failed to open file"
    Else
        erg = obj.Save("earth.bmp")
        If Err.Number <> 0 Or erg <> 1 Then
            MsgBox "failed to save file"
        Else
            MsgBox "successfully converted !"
        End If
    End If
    Set obj = Nothing

End Sub

