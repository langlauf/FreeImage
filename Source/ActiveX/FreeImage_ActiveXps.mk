
FreeImage_ActiveXps.dll: dlldata.obj FreeImage_ActiveX_p.obj FreeImage_ActiveX_i.obj
	link /dll /out:FreeImage_ActiveXps.dll /def:FreeImage_ActiveXps.def /entry:DllMain dlldata.obj FreeImage_ActiveX_p.obj FreeImage_ActiveX_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del FreeImage_ActiveXps.dll
	@del FreeImage_ActiveXps.lib
	@del FreeImage_ActiveXps.exp
	@del dlldata.obj
	@del FreeImage_ActiveX_p.obj
	@del FreeImage_ActiveX_i.obj
