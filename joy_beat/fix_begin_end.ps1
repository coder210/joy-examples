function FixFile($path) {
    $c = Get-Content $path -Raw
    $changed = $false
    
    # Lua: set_color + clear -> begin
    # Match: set_color(ren, 0, 0, 0, 255)\n    render2d.clear(ren)
    $c = $c -replace "set_color\(ren,\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)\s*\n\s*render2d\.clear\(ren\)", "begin(ren, `$1, `$2, `$3, `$4)"
    if ($LASTEXITCODE -or $c -match "begin\(") { $changed = $true }
    
    # Lua: present -> end_frame
    $c = $c -replace "render2d\.present\(ren\)", "render2d.end_frame(ren)"
    if ($c -match "end_frame") { $changed = $true }
    
    # JS: set_color + clear -> begin
    $c = $c -replace "set_color\(ren,\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\);\s*\n\s*render2d\.clear\(ren\)", "begin(ren, `$1, `$2, `$3, `$4);"
    
    # JS: present -> end_frame
    $c = $c -replace "render2d\.present\(ren\)", "render2d.end_frame(ren)"
    
    if ($changed -or $c -match "end_frame") {
        [System.IO.File]::WriteAllText($path, $c, [System.Text.Encoding]::UTF8)
        Write-Host "Fixed $((Get-Item $path).Name)"
    }
}

Get-ChildItem "e:\c++\joy\joy_beat\assets\joy_beat\scripts\*.lua" | ForEach-Object { FixFile $_.FullName }
Get-ChildItem "e:\c++\joy\joy_beat\assets\joy_beat\scripts\*.js" | ForEach-Object { FixFile $_.FullName }
