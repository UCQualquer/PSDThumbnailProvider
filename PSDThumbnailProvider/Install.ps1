regsvr32 PSDThumbnailProvider.dll

taskkill /f /im explorer.exe
Start-Sleep -Milliseconds 200

Remove-Item $Env:LOCALAPPDATA\Microsoft\Windows\Explorer\* -include thumbcache_*.db

start explorer.exe