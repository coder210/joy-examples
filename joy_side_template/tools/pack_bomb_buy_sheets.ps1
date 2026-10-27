# 将 bomb_buy 的"逐帧目录"素材打包为引擎所需的"横向等帧精灵表"。
#
# 引擎约定（joy_side_template/passes/draw_player_pass.h:73-82）：
#   每个动作槽一张横向精灵表，帧宽 = 图高，帧数 = 图宽 / 图高（自动探测）。
# 硬约束（joy/janim.h:50）：
#   ANIM_MAX_CLIPS = 16，单动作最多 16 帧，超出会被截断导致循环断裂。
#
# 用法：powershell -ExecutionPolicy Bypass -File pack_bomb_buy_sheets.ps1
Add-Type -AssemblyName System.Drawing
$ErrorActionPreference = 'Stop'

$root   = Split-Path -Parent $PSScriptRoot                                  # joy_side_template
$srcDir = Join-Path $root 'assets/joy_side_template/sprites/characters/bomb_buy'
$outDir = Join-Path $root 'assets/joy_side_template/sprites/characters/bomb_buy_sheet'

# out    : 输出文件名（不含扩展名），对应 characters.json 的 anims 值
# dir    : bomb_buy 下的动作目录
# stride : 抽帧步长（1=全取，2=隔帧抽取）
$jobs = @(
    @{ out = 'idle';   dir = '1-Idle';        stride = 2 },  # 26 帧 -> 13：超过 16 上限，隔帧抽取以保留完整循环
    @{ out = 'run';    dir = '2-Run';         stride = 1 },  # 14
    @{ out = 'hit';    dir = '7-Hit';         stride = 1 },  # 8
    @{ out = 'death';  dir = '9-Dead Ground'; stride = 1 },  # 4
    @{ out = 'ground'; dir = '6-Ground';      stride = 1 }   # 3
)

if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

foreach ($j in $jobs) {
    $inDir = Join-Path $srcDir $j.dir
    if (-not (Test-Path $inDir)) { throw "缺少素材目录: $inDir" }

    # 按文件名数字排序（1,2,...,26；不能按字符串排序，否则 1,10,11,...,2）
    $files = Get-ChildItem -Path $inDir -Filter *.png |
             Sort-Object { [int][System.IO.Path]::GetFileNameWithoutExtension($_.Name) }
    if ($files.Count -eq 0) { throw "目录内无 png: $inDir" }

    # 抽帧
    $picked = @()
    for ($i = 0; $i -lt $files.Count; $i += $j.stride) { $picked += $files[$i] }
    if ($picked.Count -gt 16) { throw "$($j.out) 帧数 $($picked.Count) 超过 ANIM_MAX_CLIPS=16" }

    # 校验所有帧尺寸一致（横向拼接的前提）
    $probe = [System.Drawing.Image]::FromFile($picked[0].FullName)
    $fw = $probe.Width; $fh = $probe.Height
    $probe.Dispose()
    foreach ($f in $picked) {
        $im = [System.Drawing.Image]::FromFile($f.FullName)
        if ($im.Width -ne $fw -or $im.Height -ne $fh) {
            $im.Dispose(); throw "$($j.out) 帧尺寸不一致: $($f.Name)"
        }
        $im.Dispose()
    }

    # 横向拼接（保留 alpha）
    $sheet = [System.Drawing.Bitmap]::new($fw * $picked.Count, $fh,
                [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($sheet)
    $g.CompositingMode    = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
    $g.InterpolationMode  = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $g.PixelOffsetMode    = [System.Drawing.Drawing2D.PixelOffsetMode]::None
    for ($i = 0; $i -lt $picked.Count; $i++) {
        $im  = [System.Drawing.Image]::FromFile($picked[$i].FullName)
        $dst = [System.Drawing.Rectangle]::new($i * $fw, 0, $fw, $fh)
        $src = [System.Drawing.Rectangle]::new(0, 0, $fw, $fh)
        $g.DrawImage($im, $dst, $src, [System.Drawing.GraphicsUnit]::Pixel)
        $im.Dispose()
    }
    $g.Dispose()

    $outPath = Join-Path $outDir ($j.out + '.png')
    $sheet.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $sheet.Dispose()

    Write-Host ("{0,-8} <- {1,-16} {2,2} 帧  ->  {3}x{4}" -f `
        $j.out, $j.dir, $picked.Count, ($fw * $picked.Count), $fh)
}

Write-Host "`n输出目录: $outDir"
