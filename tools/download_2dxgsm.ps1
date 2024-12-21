if ((Invoke-WebRequest "https://djtrackers.com/2dx-gsm/manifest.yml") -match "(https?://.*?/2dx-gsm/80/.*?.dll)") {
    Invoke-WebRequest $Matches[0] -OutFile 2dx-gsm.dll
    Copy-Item 2dx-gsm.dll data/
}
