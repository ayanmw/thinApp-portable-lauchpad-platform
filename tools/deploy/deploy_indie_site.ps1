# 独立站部署脚本 (GitHub Pages)
# 作者: MVP 应用商店上线工程师
# 日期: 2026-05-24
# 说明: 自动部署静态网站到 GitHub Pages

param(
    [Parameter(Mandatory=$true)]
    [string]$RepoUrl,            # GitHub 仓库 URL (例如: https://github.com/username/repo.git)
    
    [Parameter(Mandatory=$true)]
    [string]$LocalRepoPath,      # 本地仓库路径
    
    [string]$Branch = "gh-pages",  # GitHub Pages 分支 (通常是 gh-pages 或 main)
    
    [string]$BuildDir = "dist",    # 构建输出目录
    
    [string]$CommitMessage = "Deploy: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')",  # 提交信息
    
    [switch]$SkipBuild           # 跳过构建步骤 (如果网站已构建)
)

# 检查 Git 是否安装
function Test-Git {
    try {
        $GitVersion = git --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Output "✅ Git 已安装: $GitVersion"
            return $true
        }
    }
    catch {
        Write-Error "❌ 未找到 Git，请先安装: https://git-scm.com/"
        exit 1
    }
}

# 检查本地仓库是否存在
function Initialize-Repo {
    param(
        [string]$LocalRepoPath,
        [string]$RepoUrl
    )
    
    if (-not (Test-Path $LocalRepoPath)) {
        Write-Output "📥 本地仓库不存在，正在克隆..."
        git clone $RepoUrl $LocalRepoPath
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "❌ 仓库克隆失败"
            exit 1
        }
        
        Write-Output "✅ 仓库克隆成功: $LocalRepoPath"
    }
    else {
        Write-Output "✅ 本地仓库已存在: $LocalRepoPath"
        Set-Location $LocalRepoPath
        
        # 拉取最新代码
        Write-Output "📥 正在拉取最新代码..."
        git pull origin $Branch
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "❌ 拉取代码失败"
            exit 1
        }
        
        Write-Output "✅ 代码已更新"
    }
}

# 构建静态网站 (如果需要)
function Build-Website {
    param(
        [string]$LocalRepoPath
    )
    
    Write-Output "🔨 正在构建静态网站..."
    Set-Location $LocalRepoPath
    
    # 检查是否有 package.json (Node.js 项目)
    if (Test-Path "package.json") {
        Write-Output "📦 检测到 Node.js 项目，正在安装依赖..."
        npm install
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "❌ 依赖安装失败"
            exit 1
        }
        
        Write-Output "🔨 正在构建..."
        npm run build
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "❌ 构建失败"
            exit 1
        }
        
        Write-Output "✅ 网站构建成功"
    }
    else {
        Write-Output "⚠️  未检测到构建脚本，跳过构建步骤"
        Write-Output "💡 如果您的项目需要构建，请确保 package.json 存在"
    }
}

# 部署到 GitHub Pages
function Deploy-ToGitHubPages {
    param(
        [string]$LocalRepoPath,
        [string]$BuildDir,
        [string]$Branch,
        [string]$CommitMessage
    )
    
    Write-Output "🚀 正在部署到 GitHub Pages..."
    Set-Location $LocalRepoPath
    
    # 检查构建目录是否存在
    if (-not (Test-Path $BuildDir)) {
        Write-Error "❌ 构建目录不存在: $BuildDir"
        Write-Output "💡 请检查构建配置，或指定正确的构建目录"
        exit 1
    }
    
    # 切换到 gh-pages 分支 (如果不存在则创建)
    git checkout $Branch 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Output "🌿 分支 $Branch 不存在，正在创建..."
        git checkout -b $Branch
    }
    
    # 复制构建文件到根目录
    Write-Output "📦 正在复制构建文件..."
    Copy-Item -Path "$BuildDir\*" -Destination "." -Recurse -Force
    
    # 添加所有文件
    git add -A
    
    # 提交
    Write-Output "📝 正在提交..."
    git commit -m $CommitMessage
    
    if ($LASTEXITCODE -ne 0) {
        Write-Output "⚠️  没有变更需要提交"
    }
    else {
        Write-Output "✅ 提交成功"
    }
    
    # 推送到远程仓库
    Write-Output "📤 正在推送到远程仓库..."
    git push origin $Branch
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "❌ 推送失败"
        exit 1
    }
    
    Write-Output "✅ 部署成功！"
    Write-Output ""
    Write-Output "🌐 网站地址: https://$(git remote get-url origin | Split-Path -Leaf).github.io/$(($RepoUrl -split '/')[-1] -replace '\.git$', '')"
    Write-Output "⏳ 请等待 1-5 分钟让 GitHub Pages 生效"
}

# 主流程
try {
    Write-Output "🚀 开始部署独立站..."
    Write-Output "📂 本地仓库: $LocalRepoPath"
    Write-Output "🌿 分支: $Branch"
    Write-Output "📦 构建目录: $BuildDir"
    Write-Output ""
    
    # 步骤 1: 检查 Git
    Test-Git
    
    # 步骤 2: 初始化仓库
    Initialize-Repo -LocalRepoPath $LocalRepoPath -RepoUrl $RepoUrl
    
    # 步骤 3: 构建网站 (除非跳过)
    if (-not $SkipBuild) {
        Build-Website -LocalRepoPath $LocalRepoPath
    }
    else {
        Write-Output "⏭️  跳过构建步骤"
    }
    
    # 步骤 4: 部署到 GitHub Pages
    Deploy-ToGitHubPages -LocalRepoPath $LocalRepoPath -BuildDir $BuildDir `
        -Branch $Branch -CommitMessage $CommitMessage
    
    Write-Output ""
    Write-Output "🎉 独立站部署完成！"
    Write-Output ""
    Write-Output "📌 后续步骤:"
    Write-Output "   1. 等待 1-5 分钟让 GitHub Pages 生效"
    Write-Output "   2. 访问网站检查是否正常"
    Write-Output "   3. 配置自定义域名 (可选)"
    Write-Output "   4. 配置 CDN 加速 (如果下载量大)"
}
catch {
    Write-Error "❌ 部署过程中发生错误: $_"
    exit 1
}
