#!/usr/bin/env python3
"""
VirusTotal API 集成脚本
=======================
用于扫描文件并输出 VirusTotal 扫描报告（JSON 格式）。

用途：
  1. 手动扫描单个文件
  2. 集成到 CI/CD（每次构建后自动扫描）
  3. 批量扫描多个文件

依赖：
  - Python 3.6+
  - requests 库（pip install requests）

使用方法：
  1. 设置环境变量 VIRUSTOTAL_API_KEY：
     export VIRUSTOTAL_API_KEY="your_api_key_here"

  2. 扫描单个文件：
     python vt_scan.py --file hook_engine.dll

  3. 扫描多个文件：
     python vt_scan.py --dir ./build

  4. 集成到 CI/CD（GitHub Actions 示例）：
     - name: Scan with VirusTotal
       run: python tools/ci/vt_scan.py --dir ./build --fail-on-detect

获取 API Key：
  1. 访问 https://www.virustotal.com/gui/join
  2. 注册账号（免费）
  3. 访问 https://www.virustotal.com/gui/my-apikey
  4. 复制 API Key
"""

import os
import sys
import json
import time
import argparse
import hashlib
from pathlib import Path
from typing import Dict, List, Optional

try:
    import requests
except ImportError:
    print("Error: requests library not installed. Install with: pip install requests")
    sys.exit(1)

# VirusTotal API 配置
VT_API_URL = "https://www.virustotal.com/vtapi/v2"
VT_API_KEY = os.environ.get("VIRUSTOTAL_API_KEY")

# 检测阈值（超过此检测率则视为"不安全"）
DETECTION_THRESHOLD = 0.05  # 5%


def check_api_key() -> None:
    """检查 API Key 是否设置"""
    if not VT_API_KEY:
        print("Error: VIRUSTOTAL_API_KEY environment variable not set.")
        print("Get your API key from: https://www.virustotal.com/gui/my-apikey")
        print("\nExample:")
        print("  export VIRUSTOTAL_API_KEY='your_api_key_here'")
        sys.exit(1)


def calculate_file_hash(file_path: str) -> str:
    """计算文件的 SHA-256 哈希值"""
    sha256 = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            sha256.update(chunk)
    return sha256.hexdigest()


def upload_file(file_path: str) -> Optional[str]:
    """
    上传文件到 VirusTotal（扫描大文件）
    返回：file_id（用于查询扫描结果）
    """
    url = f"{VT_API_URL}/file/scan"
    params = {"apikey": VT_API_KEY}
    files = {"file": (Path(file_path).name, open(file_path, "rb"))}

    try:
        print(f"  Uploading {Path(file_path).name} ...")
        response = requests.post(url, params=params, files=files, timeout=60)
        response.raise_for_status()
        result = response.json()

        if result.get("response_code") == 1:
            file_id = result.get("scan_id")
            print(f"  Upload successful. Scan ID: {file_id}")
            return file_id
        else:
            print(f"  Error uploading file: {result.get('verbose_msg')}")
            return None
    except requests.exceptions.RequestException as e:
        print(f"  Error uploading file: {e}")
        return None
    finally:
        files["file"][1].close()


def get_file_report(file_hash: str) -> Optional[Dict]:
    """
    根据文件哈希值获取扫描报告
    返回：扫描报告（JSON）
    """
    url = f"{VT_API_URL}/file/report"
    params = {
        "apikey": VT_API_KEY,
        "resource": file_hash,
    }

    try:
        response = requests.get(url, params=params, timeout=30)
        response.raise_for_status()
        result = response.json()

        if result.get("response_code") == 1:
            return result
        elif result.get("response_code") == 0:
            print(f"  File not found in VirusTotal database (hash: {file_hash[:16]}...)")
            return None
        else:
            print(f"  Error getting report: {result.get('verbose_msg')}")
            return None
    except requests.exceptions.RequestException as e:
        print(f"  Error getting report: {e}")
        return None


def wait_for_scan_completion(file_id: str, max_wait: int = 60) -> Optional[Dict]:
    """
    等待扫描完成（轮询）
    返回：扫描报告（JSON）
    """
    url = f"{VT_API_URL}/file/report"
    params = {
        "apikey": VT_API_KEY,
        "resource": file_id,
    }

    print(f"  Waiting for scan to complete...")
    start_time = time.time()

    while time.time() - start_time < max_wait:
        try:
            response = requests.get(url, params=params, timeout=30)
            response.raise_for_status()
            result = response.json()

            if result.get("response_code") == 1:
                print(f"  Scan completed.")
                return result
            else:
                # 扫描未完成，等待 5 秒后重试
                time.sleep(5)
        except requests.exceptions.RequestException as e:
            print(f"  Error polling scan result: {e}")
            time.sleep(5)

    print(f"  Timeout: scan not completed within {max_wait} seconds.")
    return None


def analyze_report(report: Dict) -> Dict:
    """
    分析扫描报告，输出关键信息
    返回：分析结果（JSON）
    """
    positives = report.get("positives", 0)
    total = report.get("total", 0)
    detection_rate = positives / total if total > 0 else 0

    # 收集检测引擎和结果
    scans = report.get("scans", {})
    detected_engines = []
    for engine, result in scans.items():
        if result.get("detected"):
            detected_engines.append({
                "engine": engine,
                "result": result.get("result"),
                "version": result.get("version"),
                "update": result.get("update"),
            })

    analysis = {
        "file_name": report.get("scan_id", "").split("/")[-1],
        "file_hash": report.get("sha256", ""),
        "scan_date": report.get("scan_date", ""),
        "positives": positives,
        "total": total,
        "detection_rate": round(detection_rate * 100, 2),
        "is_safe": detection_rate < DETECTION_THRESHOLD,
        "detected_engines": detected_engines,
        "permalink": report.get("permalink", ""),
    }

    return analysis


def print_analysis(analysis: Dict) -> None:
    """打印分析结果（人类可读格式）"""
    print(f"\n{'=' * 60}")
    print(f"File: {analysis['file_name']}")
    print(f"SHA-256: {analysis['file_hash']}")
    print(f"Scan Date: {analysis['scan_date']}")
    print(f"\nDetection: {analysis['positives']} / {analysis['total']} engines")
    print(f"Detection Rate: {analysis['detection_rate']}%")
    print(f"Safe: {'✓ Yes' if analysis['is_safe'] else '✗ No'}")
    print(f"\nPermalink: {analysis['permalink']}")

    if analysis["detected_engines"]:
        print(f"\nDetected by:")
        for engine in analysis["detected_engines"]:
            print(f"  - {engine['engine']}: {engine['result']}")
    print(f"{'=' * 60}\n")


def scan_file(file_path: str, wait: bool = True) -> Optional[Dict]:
    """
    扫描单个文件
    返回：分析结果（JSON）
    """
    print(f"\nScanning: {file_path}")

    # 计算文件哈希值
    file_hash = calculate_file_hash(file_path)
    print(f"  SHA-256: {file_hash[:16]}...")

    # 先尝试获取已有报告（如果文件已被扫描过）
    print(f"  Checking existing report...")
    report = get_file_report(file_hash)
    if report:
        print(f"  Found existing report.")
        analysis = analyze_report(report)
        print_analysis(analysis)
        return analysis

    # 没有已有报告，上传文件扫描
    if wait:
        file_id = upload_file(file_path)
        if not file_id:
            return None

        # 等待扫描完成
        report = wait_for_scan_completion(file_id)
        if not report:
            return None

        analysis = analyze_report(report)
        print_analysis(analysis)
        return analysis
    else:
        # 仅上传，不等待
        file_id = upload_file(file_path)
        if file_id:
            print(f"  File uploaded. Check report later with scan_id: {file_id}")
            return {"scan_id": file_id, "status": "uploaded"}
        else:
            return None


def scan_directory(dir_path: str, extensions: List[str] = None, wait: bool = True) -> List[Dict]:
    """
    扫描目录中的所有文件（可选过滤扩展名）
    返回：所有文件的分析结果列表
    """
    results = []
    dir_path = Path(dir_path)

    if not dir_path.is_dir():
        print(f"Error: {dir_path} is not a directory.")
        return results

    # 收集文件
    if extensions:
        files = []
        for ext in extensions:
            files.extend(dir_path.rglob(f"*{ext}"))
    else:
        files = list(dir_path.rglob("*"))

    # 过滤：仅保留文件（排除目录）
    files = [f for f in files if f.is_file()]

    print(f"\nFound {len(files)} file(s) to scan.\n")

    for file_path in files:
        analysis = scan_file(str(file_path), wait=wait)
        if analysis:
            results.append(analysis)

        # VirusTotal API 限制：免费版 4 次/分钟
        time.sleep(15)

    return results


def save_report(results: List[Dict], output_file: str) -> None:
    """保存扫描报告到 JSON 文件"""
    with open(output_file, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2, ensure_ascii=False)
    print(f"\nReport saved to: {output_file}")


def main():
    parser = argparse.ArgumentParser(
        description="VirusTotal API 集成脚本 - 扫描文件并输出报告",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  1. 扫描单个文件:
     python vt_scan.py --file hook_engine.dll

  2. 扫描目录中的所有 .dll 和 .exe 文件:
     python vt_scan.py --dir ./build --ext .dll .exe

  3. 扫描但不等待结果（仅上传）:
     python vt_scan.py --dir ./build --no-wait

  4. 扫描完成后，若检测率 > 5%%，则退出码为 1（用于 CI/CD）:
     python vt_scan.py --dir ./build --fail-on-detect

设置 API Key:
  export VIRUSTOTAL_API_KEY="your_api_key_here"

  或创建 .env 文件:
    VIRUSTOTAL_API_KEY=your_api_key_here
"""
    )

    parser.add_argument("--file", type=str, help="扫描单个文件")
    parser.add_argument("--dir", type=str, help="扫描目录中的所有文件")
    parser.add_argument("--ext", type=str, nargs="+", help="仅扫描指定扩展名的文件（例如 .dll .exe）")
    parser.add_argument("--no-wait", action="store_true", help="上传文件后不等待扫描完成")
    parser.add_argument("--fail-on-detect", action="store_true", help="若检测率 > 5%%，则退出码为 1")
    parser.add_argument("--output", type=str, default="vt_report.json", help="输出报告文件路径（默认: vt_report.json）")

    args = parser.parse_args()

    # 检查参数
    if not args.file and not args.dir:
        parser.print_help()
        sys.exit(1)

    # 检查 API Key
    check_api_key()

    # 扫描文件
    results = []

    if args.file:
        analysis = scan_file(args.file, wait=not args.no_wait)
        if analysis:
            results.append(analysis)

    if args.dir:
        analyses = scan_directory(args.dir, extensions=args.ext, wait=not args.no_wait)
        results.extend(analyses)

    # 保存报告
    if results:
        save_report(results, args.output)

    # 判断是否失败（用于 CI/CD）
    if args.fail_on_detect:
        for analysis in results:
            if not analysis.get("is_safe", True):
                print("\nError: Some files have detection rate > 5%. Exiting with code 1.")
                sys.exit(1)

    print("\nAll files scanned successfully.")


if __name__ == "__main__":
    main()
