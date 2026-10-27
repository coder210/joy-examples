# 将 objects/bomb 的"逐帧目录"素材打包为引擎所需的"横向等帧精灵表"。
# 引擎（draw_player_pass.h 的 sprite_add_clips）要求帧宽 = 图高（正方形帧），
# 而炸弹原始帧为 96x108（非正方形），故每帧 pad 成 108x108（左右各留 6px 透明边）。
# 用法：powershell -ExecutionPolicy Bypass -File pack_bomb_sheets.ps1
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$srcRoot = Join-Path $root '..\assets\joy_side_template\sprites\objects\bomb'
$outDir  = Join-Path $root '..\assets\joy_side_template\sprites\objects\bomb_sheet'
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

[int]$tile = 108   # 正方形帧边长（与炸弹原始高度一致）

# 每个动作：源目录 -> 输出文件名
$anims = @(
    @{ dir = '1-Bomb Off';   out = 'off.png' },
    @{ dir = '2-Bomb On';    out = 'on.png' },
    @{ dir = '3-Explotion';  out = 'explosion.png' }
)

foreach ($a in $anims) {
    $inDir = Join-Path $srcRoot $a.dir
    if (-not (Test-Path $inDir)) { throw "缺少素材目录: $inDir" }
    # 按数字序排序（1,2,...,10 而非字符串 1,10,2,...）
    $files = @(Get-ChildItem $inDir -Filter *.png | Sort-Object { [int]($_.BaseName) })
    [int]$n = $files.Count
    if ($n -eq 0) { throw "目录内无 png: $inDir" }
    if ($n -gt 16) { throw "$($a.out) 帧数 $n 超过 ANIM_MAX_CLIPS=16" }

    # 尺寸一致性校验（全部应为 96x108）
    foreach ($f in $files) {
        $b = [IO.File]::ReadAllBytes($f.FullName)
        [int]$w = [BitConverter]::ToUInt32([byte[]](@($b[19],$b[18],$b[17],$b[16])),0)
        [int]$h = [BitConverter]::ToUInt32([byte[]](@($b[23],$b[22],$b[21],$b[20])),0)
        if ($w -ne 96 -or $h -ne 108) { throw "$($f.Name) 尺寸 ${w}x${h} 非预期(应为96x108)" }
    }

    [int]$canvasW = $n * $tile
    $canvas = New-Object System.Drawing.Bitmap($canvasW, $tile)
    $g = [System.Drawing.Graphics]::FromImage($canvas)
    $g.Clear([System.Drawing.Color]::FromArgb(0,0,0,0))
    for ([int]$i = 0; $i -lt $n; $i++) {
        $src = [System.Drawing.Image]::FromFile($files[$i].FullName)
        [int]$pad = [int](($tile - $src.Width) / 2)   # 居中：左右各 (108-96)/2 = 6px
        $rect = New-Object System.Drawing.Rectangle(($i * $tile + $pad), 0, $src.Width, $src.Height)
        $g.DrawImage($src, $rect)
        $src.Dispose()
    }
    $g.Dispose()
    $outPath = Join-Path $outDir $a.out
    $canvas.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $canvas.Dispose()
    Write-Host ("{0,-14} {1} 帧 -> {2}" -f $a.out, $n, $outPath)
}
Write-Host "done."
