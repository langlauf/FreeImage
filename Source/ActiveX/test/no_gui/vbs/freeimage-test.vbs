' ==========================================================
' FreeImage ActiveX-Interface
'
' Design and implementation by
' - Markus Loibl (markus.loibl@epost.de)
'
' This file is part of FreeImage 2.6.0
'
' COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
' OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
' THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
' OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
' CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
' THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
' SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
' PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
' THIS DISCLAIMER.
'
' Use at your own risk!
' ==========================================================

Option explicit

Const cloneImage  = "earth.jpg"
Const colorImage  = "earth.tif"


' path :       without \ at the end
'              directories have to exist
' extension:   lower case
' quality:     jpeg: quality (bad)->(super):  0 .. 100
Const srcDir      = "."
Const dstDir      = "."
Const srcExt      = "tif"
Const dstExt      = "jpg"
Const quality     = 80

dim ctrlAppl
dim ctrlImage

set ctrlAppl  = createObject("FreeImage.Application")
set ctrlImage = createObject("FreeImage.Image")

AskForTests

set ctrlImage = Nothing
set ctrlAppl  = Nothing

' #########################################################################

Sub AskForTests
	Dim erg

	erg = MsgBox("Run basic application tests ?", vbYesNo)
	if erg = vbYes then
		DoBasicTest
	End if

	erg = MsgBox("Clone " + cloneImage + " ?", vbYesNo)
	if erg = vbYes then
		DoClone
	End if

	erg = MsgBox("Change colors for " + colorImage + " ?", vbYesNo)
	if erg = vbYes then
		DoChangeColors
	End if

	erg = MsgBox("Convert all " + srcExt + "-files in " + Chr(13) + srcDir + Chr(13) + " to " + dstExt + " (" + CStr(quality) + ") in" + Chr(13) + dstDir + Chr(13) + " ?", vbYesNo)
	if erg = vbYes then
		DoConvert
	End if

End Sub


' #########################################################################

Function ChangeExtension(file, newExtension)
	
	' changes file-extension

	Dim posPoint

	posPoint = InStrRev(file, ".")

	if posPoint > 0 then
		ChangeExtension =  Left(file, posPoint-1) + "." + newExtension
	Else
		ChangeExtension =  file + "." + newExtension
	End if
	
End Function

' #########################################################################

Sub DoBasicTest

	dim title
	dim text

	' shows copyright

	title = "Version: " + ctrlAppl.Version
	text = "hello from within the freeimage-ActiveX-server called from VBScript" + Chr(13) + "Copyright: " + Chr(13) + ctrlAppl.Copyright
	ctrlAppl.Test text, "Internal: " + title

End Sub

' #########################################################################

Sub DoClone

	dim erg
	dim ctrl2
	dim res

	On Error Resume Next
	
	erg = ctrlImage.Open(cloneImage)
	if erg <> 1 or err.number <> 0 then
		MsgBox "couldn't open " + cloneImage
		exit sub
	end if 

	set ctrl2 = ctrlImage.Clone
	ctrl2.ChangeColors 8, 0
	
	ctrlImage.Save(ChangeExtension("org_" + cloneImage, "bmp"))
	ctrl2.Save(ChangeExtension("gray_"+ cloneImage, "bmp"))

	set ctrl2 = Nothing

End Sub

' #########################################################################

Sub DoChangeColors

	ConvertImage colorImage,   1, 0
	ConvertImage colorImage,   8, 0
	ConvertImage colorImage, 256, 0
	ConvertImage colorImage, 256, 1
	ConvertImage colorImage,  16, 555
	ConvertImage colorImage,  16, 565
	ConvertImage colorImage,  24, 0
	ConvertImage colorImage,  32, 0

End Sub

Sub ConvertImage(image, color, param)

	dim dErg
	dim fileResult

	On Error Resume Next

	fileResult = ChangeExtension(CStr(color) + "_" + CStr(param) + image, "tif")

	dErg = ctrlImage.Open(image)
	if err.number=0 then
		dErg = ctrlImage.ChangeColors(color, param)
		if err.number=0 then
			if dErg = 1 then
				dErg = ctrlImage.Save(fileResult)
				if err.number=0 then
					msg = msg + "Successfully converted " + image + " to " + fileResult + chr(13)
				else
					msg = msg + "Couldn't save file "  + fileResult +chr(13)
				end if
			elseif dErg = 2 then
				msg = msg + "Not neccessary to change color " + fileResult +chr(13)
			end if
		else
			msg = msg + "Couldn't convert " + image + " to " + fileResult + chr(13)
		end if 
	else
		msg = msg + "Couldn't open " + image + chr(13)
	end if

End Sub

' #########################################################################

Sub DoConvert

	dim strDone
	dim strFailed
	Dim fmt
	Dim erg


   ' get extension
   fmt = ctrlAppl.GetImageType(0, dstExt)
   ctrlAppl.ShowMessages = 0

   ' all files
   ConvertAllFiles fmt, strDone, strFailed

	' Show results

	if Len(strDone) > 0 then
	  MsgBox "These files were converted:" + Chr(13) + strDone, vbOKOnly, "Converting rasterfiles © M. Loibl"
	end if

	if Len(strFailed) > 0 then
	  MsgBox "These files had errors:" + Chr(13) + strFailed, vbOKOnly, "Converting rasterfiles © M. Loibl"
	else
	  MsgBox "No errors occured", vbOKOnly, "Converting rasterfiles © M. Loibl"
	end if

End Sub

' ----------------------------------------------------------------------

Sub ConvertAllFiles(fmt, strDone, strFailed)
   Dim fso, f, f1, fc, s

   ' loops through all files
   Set fso = CreateObject("Scripting.FileSystemObject")
   Set f = fso.GetFolder(srcDir)
   Set fc = f.Files
   For Each f1 in fc
      ' filename   f1.name
      ' Extension  fso.GetExtensionName(f1.name)
      ' path       f1.path

      ' checks extension
      if StrComp(LCase(fso.GetExtensionName(f1.name)), srcExt) = 0 then
         ConvertThisFile fmt, strDone, strFailed, f1
      End if
   Next

End Sub

' ----------------------------------------------------------------------

' convert this file
Sub ConvertThisFile(fmt, strDone, strFailed, objFile)
  Dim outFile
  Dim erg
  Dim colors
  dim ziel

  On Error Resume Next

  ziel = Left(objFile.name, Len(objFile.name) - Len(srcExt)) + dstExt
  outFile = dstDir + "\" + ziel

  ' open
  erg = ctrlImage.Open(objFile.path)
  if erg <> 1 or err.number <> 0 then
    strFailed = strFailed + Chr(13) + objFile.name + ": (Open)"
    exit sub
  end if

  ' convert to 24-bit
  colors = ctrlImage.BPP
  if colors <> 24 then
    erg = ctrlImage.ChangeColors(24, 0)
    if erg <> 1 or err.number <> 0 then
      strFailed = strFailed + Chr(13) +  objFile.name + ": (converting to 24-bit)"
      exit sub
    end if
  End if

  ' saving
  erg = ctrlImage.SaveEx(fmt, outFile, quality)
  if erg <> 1 or err.number <> 0 then
    strFailed = strFailed + Chr(13) +  ziel + ": (saving)"
    exit sub
  end if

  strDone = strDone + Chr(13) +  objFile.name
  
  if colors <> 24 then
      strDone = strDone + Chr(9) + "(" + CStr(colors) + " .. 24 bit)"
  end if
End Sub

