VERSION 5.00
Object = "{AF709562-19F1-46C5-A1D6-BDE4C1954AE0}#1.0#0"; "FreeImageXD.dll"
Begin VB.Form FormFreeImage 
   Caption         =   "FreeImageTest"
   ClientHeight    =   3840
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   3645
   LinkTopic       =   "Form1"
   ScaleHeight     =   3840
   ScaleWidth      =   3645
   StartUpPosition =   3  'Windows-Standard
   Begin VB.TextBox zoom 
      Height          =   375
      Left            =   1080
      TabIndex        =   3
      Top             =   1200
      Width           =   1815
   End
   Begin VB.CommandButton CommandConvert 
      Caption         =   "Convert earth.jpg -> bmp"
      Height          =   375
      Left            =   360
      TabIndex        =   1
      Top             =   720
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
   Begin FREEIMAGE_ACTIVEXLibCtl.Painting objPainting 
      Height          =   1935
      Left            =   240
      TabIndex        =   4
      Top             =   1800
      Width           =   3135
      _cx             =   5080
      _cy             =   5080
      AutoSize        =   0   'False
      Enabled         =   -1  'True
      Valid           =   -1  'True
      SRC             =   "earth.jpg"
      Zoom            =   1
   End
   Begin VB.Label Label1 
      Caption         =   "Zoom"
      Height          =   375
      Left            =   240
      TabIndex        =   2
      Top             =   1200
      Width           =   735
   End
End
Attribute VB_Name = "FormFreeImage"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' objPainting

Private Sub CommandShowDetails_Click()
    Dim obj As New FREEIMAGE_ACTIVEXLibCtl.Application
    obj.Test "hello from within the freeimage-ActiveX-server called from VB", "My title"
    Set obj = Nothing

End Sub

Private Sub CommandConvert_Click()
    
    Dim obj As New FREEIMAGE_ACTIVEXLibCtl.Application
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

Private Sub Form_Load()
    objPainting.SRC = "earth.jpg"
    Zoom.Text = CDbl(1 / 2) ' use correct locales
    objPainting.Zoom = CDbl(Zoom.Text)
End Sub


Private Sub zoom_Change()
    Dim zoomLevel As Double
    
    If (Len(Zoom.Text) = 0) Then Exit Sub
    
    zoomLevel = CDbl(Zoom.Text)
    If (zoomLevel <> 0#) Then
        objPainting.Zoom = zoomLevel
    End If
End Sub
