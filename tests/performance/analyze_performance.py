#!/usr/bin/env python3
"""
analyze_performance.py - 性能数据分析脚本

作者: 性能测试工程师
日期: 2026-05-23
说明: 解析 JSON 格式的性能测试结果，生成 HTML 报告

依赖:
    - json (标准库)
    - statistics (标准库)
    - jinja2 (需要安装: pip install jinja2)
    - matplotlib (需要安装: pip install matplotlib)

用法:
    python analyze_performance.py [options]

选项:
    --input-dir PATH   输入目录（包含 JSON 测试结果文件，默认: .）
    --output PATH       输出 HTML 报告路径（默认: performance_report.html）
    --open              生成报告后自动打开（需要 webbrowser 模块）

示例:
    python analyze_performance.py --input-dir . --output performance_report.html
"""

import json
import statistics
import os
import sys
import argparse
from pathlib import Path

# TODO: 实际使用时取消下面的注释，安装依赖
# import jinja2
# import matplotlib.pyplot as plt
# from matplotlib.backends.backend_pdf import PdfPages


# ========== 常量定义 ==========

# MVP 性能目标
MVP_TARGETS = {
    "startup_time": 3000,      # 3 秒 = 3000 毫秒
    "memory_usage": 200,        # 200 MB
    "ui_response_time": 100,    # 100 毫秒
    "hook_overhead_cpu": 5,      # 5% CPU
    "file_redirect_latency": 1,  # 1 毫秒
    "reg_redirect_latency": 0.5  # 0.5 毫秒
}

# JSON 测试结果文件列表
TEST_FILES = {
    "startup_time": "startup_benchmark.json",
    "memory_usage": "memory_benchmark.json",
    "ui_response_time": "ui_responsiveness_benchmark.json",
    "hook_overhead_cpu": "hook_overhead_benchmark.json"
}

# 性能指标显示名称
METRIC_DISPLAY_NAMES = {
    "startup_time": "启动速度 (毫秒)",
    "memory_usage": "内存占用 (MB)",
    "ui_response_time": "UI 响应时间 (毫秒)",
    "hook_overhead_cpu": "Hook 开销 (% CPU)",
    "file_redirect_latency": "文件重定向延迟 (毫秒)",
    "reg_redirect_latency": "注册表重定向延迟 (毫秒)"
}


# ========== 数据加载函数 ==========

def load_json_file(file_path):
    """
    加载 JSON 文件
    
    Args:
        file_path: JSON 文件路径
        
    Returns:
        dict: JSON 数据，加载失败返回 None
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"[WARNING] 文件未找到: {file_path}")
        return None
    except json.JSONDecodeError as e:
        print(f"[ERROR] JSON 解析错误: {file_path} - {e}")
        return None
    except Exception as e:
        print(f"[ERROR] 加载文件失败: {file_path} - {e}")
        return None


def extract_numeric_results(data, metric_type):
    """
    从 JSON 数据中提取数值结果
    
    Args:
        data: JSON 数据（dict）
        metric_type: 性能指标类型
        
    Returns:
        list: 数值列表
    """
    if data is None:
        return []
    
    results = []
    
    if metric_type == "startup_time":
        # 从 results 数组中提取 startup_time_ms
        if "results" in data and isinstance(data["results"], list):
            for item in data["results"]:
                if "startup_time_ms" in item:
                    results.append(item["startup_time_ms"])
    
    elif metric_type == "memory_usage":
        # 从 results 字典中提取内存值
        if "results" in data and isinstance(data["results"], dict):
            if "base_memory_mb" in data["results"]:
                results.append(data["results"]["base_memory_mb"])
            if "memory_with_5_apps_mb" in data["results"]:
                results.append(data["results"]["memory_with_5_apps_mb"])
    
    elif metric_type == "ui_response_time":
        # 从 results 数组中提取 response_time_ms
        if "results" in data and isinstance(data["results"], list):
            for item in data["results"]:
                if "response_time_ms" in item:
                    results.append(item["response_time_ms"])
    
    elif metric_type == "hook_overhead_cpu":
        # 从 results 字典中提取 hook_overhead
        if "results" in data and isinstance(data["results"], dict):
            if "hook_overhead" in data["results"]:
                results.append(data["results"]["hook_overhead"])
    
    return results


# ========== 统计计算函数 ==========

def calculate_statistics(values):
    """
    计算统计信息（平均值、中位数、P95、P99）
    
    Args:
        values: 数值列表
        
    Returns:
        dict: 统计信息
    """
    if not values:
        return {
            "count": 0,
            "average": None,
            "median": None,
            "p95": None,
            "p99": None
        }
    
    sorted_values = sorted(values)
    n = len(sorted_values)
    
    # 平均值
    average = statistics.mean(sorted_values)
    
    # 中位数
    median = statistics.median(sorted_values)
    
    # P95
    p95_idx = min(int(n * 0.95), n - 1)
    p95 = sorted_values[p95_idx]
    
    # P99
    p99_idx = min(int(n * 0.99), n - 1)
    p99 = sorted_values[p99_idx]
    
    return {
        "count": n,
        "average": round(average, 2),
        "median": round(median, 2),
        "p95": round(p95, 2),
        "p99": round(p99, 2)
    }


def check_target_met(metric_type, stats):
    """
    检查是否达到 MVP 性能目标
    
    Args:
        metric_type: 性能指标类型
        stats: 统计信息
        
    Returns:
        bool: 达标返回 True，未达标返回 False
    """
    if stats["average"] is None:
        return False
    
    target = MVP_TARGETS.get(metric_type)
    if target is None:
        return False
    
    # 注意：对于某些指标，越低越好（启动时间、内存、响应时间、延迟）
    # 对于 Hook 开销，也需要低于目标值
    return stats["average"] < target


# ========== 报告生成函数 ==========

def generate_html_report(stats_dict, output_path):
    """
    生成 HTML 报告
    
    TODO: 实际使用时需要安装 jinja2 库
    TODO: 需要创建 performance_report_template.html 模板文件
    
    Args:
        stats_dict: 统计信息字典（包含所有指标）
        output_path: 输出 HTML 文件路径
    """
    # 桩实现：输出简单的 HTML 报告
    html_content = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AI ThinApp Portable Launchpad Platform - 性能测试报告</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        table { border-collapse: collapse; width: 100%; background-color: white; }
        th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }
        th { background-color: #4CAF50; color: white; }
        .target-met { color: green; font-weight: bold; }
        .target-not-met { color: red; font-weight: bold; }
        .chart { margin-top: 30px; }
    </style>
</head>
<body>
    <h1>AI ThinApp Portable Launchpad Platform</h1>
    <h2>性能测试报告</h2>
    <p>生成时间: """ + __import__('datetime').datetime.now().strftime("%Y-%m-%d %H:%M:%S") + """</p>
    
    <h3>性能指标统计</h3>
    <table>
        <tr>
            <th>性能指标</th>
            <th>平均值</th>
            <th>中位数</th>
            <th>P95</th>
            <th>P99</th>
            <th>MVP 目标</th>
            <th>是否达标</th>
        </tr>
"""
    
    # 添加表格行
    for metric_type, display_name in METRIC_DISPLAY_NAMES.items():
        if metric_type in stats_dict:
            stats = stats_dict[metric_type]
            target = MVP_TARGETS.get(metric_type, "N/A")
            met = check_target_met(metric_type, stats)
            
            html_content += f"""        <tr>
            <td>{display_name}</td>
            <td>{stats['average'] if stats['average'] is not None else 'N/A'}</td>
            <td>{stats['median'] if stats['median'] is not None else 'N/A'}</td>
            <td>{stats['p95'] if stats['p95'] is not None else 'N/A'}</td>
            <td>{stats['p99'] if stats['p99'] is not None else 'N/A'}</td>
            <td>{target}</td>
            <td class="{'target-met' if met else 'target-not-met'}">{'是' if met else '否'}</td>
        </tr>
"""
    
    html_content += """    </table>
    
    <h3>建议优化点</h3>
    <ul>
"""
    
    # 添加建议优化点
    for metric_type, display_name in METRIC_DISPLAY_NAMES.items():
        if metric_type in stats_dict:
            stats = stats_dict[metric_type]
            met = check_target_met(metric_type, stats)
            if not met:
                html_content += f"""        <li>{display_name}: 未达标，当前平均值 {stats['average']}，目标值 {MVP_TARGETS.get(metric_type)}</li>
"""
    
    html_content += """    </ul>
    
    <div class="chart">
        <h3>性能趋势图</h3>
        <p>TODO: 使用 matplotlib 生成图表并嵌入</p>
    </div>
    
</body>
</html>"""
    
    # 写入 HTML 文件
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(html_content)
    
    print(f"[INFO] HTML 报告已生成: {output_path}")


# ========== 主函数 ==========

def main():
    # 解析命令行参数
    parser = argparse.ArgumentParser(description='性能数据分析脚本')
    parser.add_argument('--input-dir', default='.', help='输入目录（包含 JSON 测试结果文件）')
    parser.add_argument('--output', default='performance_report.html', help='输出 HTML 报告路径')
    parser.add_argument('--open', action='store_true', help='生成报告后自动打开')
    args = parser.parse_args()
    
    print("=" * 60)
    print("性能数据分析脚本")
    print("=" * 60)
    print(f"输入目录: {args.input_dir}")
    print(f"输出文件: {args.output}")
    print()
    
    # 加载并分析所有测试结果
    stats_dict = {}
    
    for metric_type, filename in TEST_FILES.items():
        file_path = os.path.join(args.input_dir, filename)
        print(f"[INFO] 加载文件: {filename}")
        
        data = load_json_file(file_path)
        if data is None:
            print(f"[WARNING] 跳过指标: {metric_type}")
            continue
        
        # 提取值
        values = extract_numeric_results(data, metric_type)
        if not values:
            print(f"[WARNING] 未找到有效数据: {metric_type}")
            continue
        
        # 计算统计信息
        stats = calculate_statistics(values)
        stats_dict[metric_type] = stats
        
        # 检查是否达标
        met = check_target_met(metric_type, stats)
        
        print(f"  指标: {METRIC_DISPLAY_NAMES.get(metric_type, metric_type)}")
        print(f"  数据点数: {stats['count']}")
        print(f"  平均值: {stats['average']}")
        print(f"  中位数: {stats['median']}")
        print(f"  P95: {stats['p95']}")
        print(f"  P99: {stats['p99']}")
        print(f"  MVP 目标: {MVP_TARGETS.get(metric_type)}")
        print(f"  是否达标: {'是' if met else '否'}")
        print()
    
    # 生成 HTML 报告
    print("[INFO] 生成 HTML 报告...")
    generate_html_report(stats_dict, args.output)
    
    # 自动打开报告
    if args.open:
        import webbrowser
        webbrowser.open(f"file://{os.path.abspath(args.output)}")
    
    print()
    print("=" * 60)
    print("分析完成")
    print("=" * 60)
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
