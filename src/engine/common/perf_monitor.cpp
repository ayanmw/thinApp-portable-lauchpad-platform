// perf_monitor.cpp - 性能监控模块实现文件
// 作者: 性能测试工程师
// 日期: 2026-05-23
// 说明: 实现性能指标采集、统计和导出功能
// 
// TODO: 当前为桩实现，等待编译通过后实际运行
// 依赖: nlohmann/json 库（JSON 导出）
// 依赖: Windows API（QueryPerformanceCounter, GetProcessMemoryInfo 等）

#include "perf_monitor.h"
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <psapi.h>  // GetProcessMemoryInfo

// ========== 单例模式实现 ==========

PerfMonitor& PerfMonitor::Instance() {
    static PerfMonitor instance;
    return instance;
}

PerfMonitor::PerfMonitor() {
    // 初始化同步锁
    InitializeCriticalSection(&m_cs);
}

PerfMonitor::~PerfMonitor() {
    // 释放同步锁
    DeleteCriticalSection(&m_cs);
    Clear();
}

// ========== 性能指标采集方法实现 ==========

void PerfMonitor::RecordStartupTime(double milliseconds) {
    AddDataPoint(PerfMetricType::StartupTime, milliseconds, "StartupTime");
}

void PerfMonitor::RecordMemoryUsage() {
    // TODO: 使用 GetProcessMemoryInfo() 获取当前进程内存占用
    double memoryMB = GetProcessMemoryUsageMB();
    AddDataPoint(PerfMetricType::MemoryUsage, memoryMB, "MemoryUsage");
}

void PerfMonitor::RecordMemoryUsage(double memoryMB) {
    AddDataPoint(PerfMetricType::MemoryUsage, memoryMB, "MemoryUsage");
}

void PerfMonitor::RecordUIResponseTime(double milliseconds, const std::string& eventName) {
    std::string context = "UIResponseTime";
    if (!eventName.empty()) {
        context += ":" + eventName;
    }
    AddDataPoint(PerfMetricType::UIResponseTime, milliseconds, context);
}

void PerfMonitor::RecordHookOverheadCPU() {
    // TODO: 使用 GetProcessTimes() 获取 CPU 时间，计算 CPU 占用百分比
    double cpuPercentage = GetProcessCPUPercentage();
    AddDataPoint(PerfMetricType::HookOverheadCPU, cpuPercentage, "HookOverheadCPU");
}

void PerfMonitor::RecordHookOverheadCPU(double cpuPercentage) {
    AddDataPoint(PerfMetricType::HookOverheadCPU, cpuPercentage, "HookOverheadCPU");
}

void PerfMonitor::RecordFileRedirectLatency(double milliseconds, const std::string& filePath) {
    std::string context = "FileRedirectLatency";
    if (!filePath.empty()) {
        context += ":" + filePath;
    }
    AddDataPoint(PerfMetricType::FileRedirectLatency, milliseconds, context);
}

void PerfMonitor::RecordRegRedirectLatency(double milliseconds, const std::string& regPath) {
    std::string context = "RegRedirectLatency";
    if (!regPath.empty()) {
        context += ":" + regPath;
    }
    AddDataPoint(PerfMetricType::RegRedirectLatency, milliseconds, context);
}

// ========== 性能数据管理方法实现 ==========

std::vector<PerfDataPoint> PerfMonitor::GetDataPoints(PerfMetricType type) const {
    auto it = m_dataPoints.find(type);
    if (it != m_dataPoints.end()) {
        return it->second;
    }
    return {};
}

PerfDataPoint PerfMonitor::GetLastDataPoint(PerfMetricType type) const {
    auto it = m_dataPoints.find(type);
    if (it != m_dataPoints.end() && !it->second.empty()) {
        return it->second.back();
    }
    // 返回空数据点（值为 -1 表示无效）
    return PerfDataPoint(type, -1.0, "");
}

double PerfMonitor::GetAverage(PerfMetricType type) const {
    auto points = GetDataPoints(type);
    if (points.empty()) {
        return -1.0;
    }
    double sum = std::accumulate(points.begin(), points.end(), 0.0,
                                 [](double acc, const PerfDataPoint& p) {
                                     return acc + p.value;
                                 });
    return sum / points.size();
}

double PerfMonitor::GetMedian(PerfMetricType type) const {
    auto points = GetDataPoints(type);
    if (points.empty()) {
        return -1.0;
    }
    
    std::vector<double> values;
    for (const auto& p : points) {
        values.push_back(p.value);
    }
    
    std::sort(values.begin(), values.end());
    size_t n = values.size();
    
    if (n % 2 == 0) {
        return (values[n/2 - 1] + values[n/2]) / 2.0;
    } else {
        return values[n/2];
    }
}

double PerfMonitor::GetP95(PerfMetricType type) const {
    auto points = GetDataPoints(type);
    if (points.empty()) {
        return -1.0;
    }
    
    std::vector<double> values;
    for (const auto& p : points) {
        values.push_back(p.value);
    }
    
    std::sort(values.begin(), values.end());
    size_t n = values.size();
    size_t idx = static_cast<size_t>(std::ceil(0.95 * n)) - 1;
    
    return values[std::min(idx, n - 1)];
}

double PerfMonitor::GetP99(PerfMetricType type) const {
    auto points = GetDataPoints(type);
    if (points.empty()) {
        return -1.0;
    }
    
    std::vector<double> values;
    for (const auto& p : points) {
        values.push_back(p.value);
    }
    
    std::sort(values.begin(), values.end());
    size_t n = values.size();
    size_t idx = static_cast<size_t>(std::ceil(0.99 * n)) - 1;
    
    return values[std::min(idx, n - 1)];
}

// ========== 性能目标检查方法实现 ==========

bool PerfMonitor::CheckStartupTimeTarget() const {
    double avg = GetAverage(PerfMetricType::StartupTime);
    if (avg < 0) return false;  // 无数据
    return avg < 3000.0;  // < 3 秒 = 3000 毫秒
}

bool PerfMonitor::CheckMemoryUsageTarget() const {
    double avg = GetAverage(PerfMetricType::MemoryUsage);
    if (avg < 0) return false;  // 无数据
    return avg < 200.0;  // < 200 MB
}

bool PerfMonitor::CheckUIResponseTimeTarget() const {
    double avg = GetAverage(PerfMetricType::UIResponseTime);
    if (avg < 0) return false;  // 无数据
    return avg < 100.0;  // < 100 ms
}

bool PerfMonitor::CheckHookOverheadCPUTarget() const {
    double avg = GetAverage(PerfMetricType::HookOverheadCPU);
    if (avg < 0) return false;  // 无数据
    return avg < 5.0;  // < 5% CPU
}

bool PerfMonitor::CheckFileRedirectLatencyTarget() const {
    double avg = GetAverage(PerfMetricType::FileRedirectLatency);
    if (avg < 0) return false;  // 无数据
    return avg < 1.0;  // < 1 ms
}

bool PerfMonitor::CheckRegRedirectLatencyTarget() const {
    double avg = GetAverage(PerfMetricType::RegRedirectLatency);
    if (avg < 0) return false;  // 无数据
    return avg < 0.5;  // < 0.5 ms
}

// ========== 导出方法实现 ==========

// TODO: 依赖 nlohmann/json 库，需要安装 vcpkg install nlohmann-json
bool PerfMonitor::ExportToJSON(const std::string& filePath) const {
    // TODO: 实现 JSON 导出
    // 示例结构：
    // {
    //   "startup_time": { "average": 2500, "median": 2400, "p95": 2800, "p99": 2900, "target_met": true },
    //   "memory_usage": { ... },
    //   ...
    // }
    
    // 桩实现：输出到控制台
    ExportToConsole();
    return false;  // TODO: 返回 true 当实际实现完成后
}

void PerfMonitor::ExportToConsole() const {
    // 输出所有性能指标到控制台（调试用）
    for (const auto& pair : m_dataPoints) {
        PerfMetricType type = pair.first;
        const auto& points = pair.second;
        
        std::cout << "Metric: " << PerfMetricTypeToString(type) << std::endl;
        std::cout << "  Data Points: " << points.size() << std::endl;
        std::cout << "  Average: " << GetAverage(type) << std::endl;
        std::cout << "  Median: " << GetMedian(type) << std::endl;
        std::cout << "  P95: " << GetP95(type) << std::endl;
        std::cout << "  P99: " << GetP99(type) << std::endl;
        std::cout << "  Target Met: ";
        switch (type) {
            case PerfMetricType::StartupTime: std::cout << (CheckStartupTimeTarget() ? "Yes" : "No"); break;
            case PerfMetricType::MemoryUsage: std::cout << (CheckMemoryUsageTarget() ? "Yes" : "No"); break;
            case PerfMetricType::UIResponseTime: std::cout << (CheckUIResponseTimeTarget() ? "Yes" : "No"); break;
            case PerfMetricType::HookOverheadCPU: std::cout << (CheckHookOverheadCPUTarget() ? "Yes" : "No"); break;
            case PerfMetricType::FileRedirectLatency: std::cout << (CheckFileRedirectLatencyTarget() ? "Yes" : "No"); break;
            case PerfMetricType::RegRedirectLatency: std::cout << (CheckRegRedirectLatencyTarget() ? "Yes" : "No"); break;
        }
        std::cout << std::endl << std::endl;
    }
}

void PerfMonitor::Clear() {
    EnterCriticalSection(&m_cs);
    m_dataPoints.clear();
    LeaveCriticalSection(&m_cs);
}

size_t PerfMonitor::GetDataPointCount() const {
    size_t count = 0;
    for (const auto& pair : m_dataPoints) {
        count += pair.second.size();
    }
    return count;
}

// ========== 私有方法实现 ==========

void PerfMonitor::AddDataPoint(PerfMetricType type, double value, const std::string& context) {
    EnterCriticalSection(&m_cs);
    m_dataPoints[type].emplace_back(type, value, context);
    LeaveCriticalSection(&m_cs);
}

// ========== 辅助函数实现 ==========

double GetCurrentTimestampMs() {
    // TODO: 使用 QueryPerformanceCounter() 获取高精度时间戳
    return 0.0;  // 桩实现
}

double GetProcessMemoryUsageMB() {
    // TODO: 使用 GetProcessMemoryInfo() 获取当前进程内存占用
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);  // 转换为 MB
    }
    return -1.0;  // 获取失败
}

double GetProcessCPUPercentage() {
    // TODO: 使用 GetProcessTimes() 获取 CPU 时间，计算 CPU 占用百分比
    // 需要两次调用计算时间差
    return 0.0;  // 桩实现
}

std::string PerfMetricTypeToString(PerfMetricType type) {
    switch (type) {
        case PerfMetricType::StartupTime: return "StartupTime";
        case PerfMetricType::MemoryUsage: return "MemoryUsage";
        case PerfMetricType::UIResponseTime: return "UIResponseTime";
        case PerfMetricType::HookOverheadCPU: return "HookOverheadCPU";
        case PerfMetricType::FileRedirectLatency: return "FileRedirectLatency";
        case PerfMetricType::RegRedirectLatency: return "RegRedirectLatency";
        default: return "Unknown";
    }
}

PerfMetricType StringToPerfMetricType(const std::string& str) {
    if (str == "StartupTime") return PerfMetricType::StartupTime;
    if (str == "MemoryUsage") return PerfMetricType::MemoryUsage;
    if (str == "UIResponseTime") return PerfMetricType::UIResponseTime;
    if (str == "HookOverheadCPU") return PerfMetricType::HookOverheadCPU;
    if (str == "FileRedirectLatency") return PerfMetricType::FileRedirectLatency;
    if (str == "RegRedirectLatency") return PerfMetricType::RegRedirectLatency;
    throw std::invalid_argument("Invalid PerfMetricType string: " + str);
}
