#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QColor>

namespace launchpad {

// 应用卡片控件（显示应用图标、名称、沙箱状态）
class AppCard : public QWidget {
    Q_OBJECT

public:
    // 构造函数
    // parent: 父窗口指针
    explicit AppCard(QWidget* parent = nullptr);

    // 析构函数
    ~AppCard();

    // 设置应用信息
    // icon: 应用图标
    // name: 应用名称
    // isSandbox: 是否启用沙箱
    void setAppInfo(const QIcon& icon, const QString& name, bool isSandbox);

protected:
    // 绘制事件（绘制沙箱标识边框）
    void paintEvent(QPaintEvent* event) override;

private:
    // 初始化 UI
    void initUI();

    // UI 组件
    QLabel* icon_label_;      // 应用图标标签
    QLabel* name_label_;      // 应用名称标签
    QLabel* sandbox_label_;   // 沙箱状态标签

    // 状态
    bool is_sandbox_;         // 是否启用沙箱
};

}  // namespace launchpad
