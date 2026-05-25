# Build Environment Setup Blockers (Retry)

## Project
AI ThinApp Portable Launchpad Platform

## Date
2026-05-23

## Blocker 1: Elevated Privileges Not Available

### Description
Cannot run PowerShell scripts with elevated (administrator) privileges in the current environment (webchat session, subagent).

### Impact
- Cannot run `install_cmake.ps1` with administrator rights
- Cannot run `setup_vcpkg.ps1` with administrator rights
- Cannot install software to system directories (e.g., C:\Program Files\)
- Cannot modify system-level environment variables (system PATH)

### Attempted Solution
Tried to run PowerShell script with `elevated=true` parameter:
```powershell
exec(command="powershell -ExecutionPolicy Bypass -File install_cmake.ps1", elevated=true)
```

### Error Message
```
elevated is not available right now (runtime=direct).
Failing gates: enabled (tools.elevated.enabled / agents.list[].tools.elevated.enabled)
Context: provider=webchat session=agent:5jll72izmcev0ajs:subagent:98e928a3-75f9-4358-a5fc-c1dcf5bc11df
```

### Workaround Options

#### Option 1: Manual Installation (Recommended)
1. User runs the created scripts manually with administrator privileges
2. Scripts location:
   - `D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform\tools\build\install_cmake.ps1`
   - `D:\anmw_work_proj\ai-thinApp-portable-launchpad-platform\tools\build\setup_vcpkg.ps1`
3. Right-click PowerShell -> "Run as administrator"
4. Execute scripts manually

#### Option 2: Install to User Directory (Non-Admin)
Modify scripts to install to user directory:
- CMake: Install to `%LOCALAPPDATA%\CMake` (no admin required)
- vcpkg: Clone to user directory (e.g., `%USERPROFILE%\vcpkg`)
- Add to user PATH (not system PATH)

#### Option 3: Request Environment Change
User needs to enable elevated execution in OpenClaw configuration:
- Enable `tools.elevated.enabled`
- Configure `tools.elevated.allowFrom.webchat`

### Recommendation
Use **Option 1** (manual installation) for now, as it's the most reliable method.
The scripts have been created with proper ASCII encoding and English comments.

### Status
BLOCKED - Waiting for user to run scripts manually or enable elevated privileges.

---

## Next Steps

1. User runs `install_cmake.ps1` as administrator
2. User runs `setup_vcpkg.ps1` as administrator
3. User verifies installation:
   - `cmake --version`
   - `vcpkg version`
4. User re-runs this subagent or continues with verification steps

---

## Scripts Created

| Script | Path | Status |
|--------|------|--------|
| CMake Install | `tools/build/install_cmake.ps1` | CREATED (needs manual execution) |
| vcpkg Setup | `tools/build/setup_vcpkg.ps1` | PENDING (will create next) |

---

## Notes
- All scripts use pure ASCII encoding (no Chinese, no BOM)
- All comments are in English
- Scripts include error handling and verification steps
- Scripts support Tsinghua mirror for faster download in China
