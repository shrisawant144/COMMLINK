; Compile via scripts/package-windows.py, which supplies the definitions below.
Unicode true
Target amd64-unicode
Name "CommLink"
OutFile "${OUTPUT_EXE}"
!ifdef APP_ICON
Icon "${APP_ICON}"
!endif
RequestExecutionLevel user
SilentInstall silent
AutoCloseWindow true
CRCCheck force
SetCompressor /SOLID lzma
SetCompressorDictSize 32
SetDatablockOptimize on
VIProductVersion "${WIN_VERSION}"
VIAddVersionKey "ProductName" "CommLink"
VIAddVersionKey "FileDescription" "CommLink - single-file Windows launcher"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "LegalCopyright" "CommLink contributors; MIT license"

Section
    ClearErrors
    ; NSIS allocates a unique private temporary directory for each launch and
    ; removes it when this wrapper exits, after ExecWait returns.
    InitPluginsDir
    IfErrors extraction_failed
    SetOutPath "$PLUGINSDIR\App"
    File /r "${PAYLOAD_DIR}\*"
    IfErrors extraction_failed

    ClearErrors
    ExecWait '"$PLUGINSDIR\App\${APP_EXE}"' $0
    IfErrors launch_failed
    ; Leaving the extracted working directory allows Windows to remove it.
    SetOutPath "$TEMP"
    SetErrorLevel $0
    Goto done

extraction_failed:
    SetOutPath "$TEMP"
    MessageBox MB_OK|MB_ICONSTOP "CommLink could not unpack its temporary files. Check that your Windows temporary folder is writable and has enough free space."
    SetErrorLevel 1
    Goto done

launch_failed:
    SetOutPath "$TEMP"
    MessageBox MB_OK|MB_ICONSTOP "CommLink could not start. Please check that you are using 64-bit Windows and try again."
    SetErrorLevel 2

done:
SectionEnd
