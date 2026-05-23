#include "app_card.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QPalette>

namespace launchpad {

// 构造函数
AppCard::AppCard(QWidget* parent) : QWidget(parent),
    icon_label_(nullptr),
    name_label_(nullptr),
    sandbox_label_(nullptr),
    is_sandbox_(false) {
    
    // 初始化 UI
    initUI();
}

// 析构函数
AppCard::~AppCard() {
    // 清理资源（Qt 对象树自动管理子对象）
}

// 初始化 UI
void AppCard::initUI() {
    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(5);
    
    // 创建图标标签
    icon_label_ = new QLabel(this);
    icon_label_->setAlignment(Qt::AlignCenter);
    icon_label_->setFixedSize(64, 64);
    layout->addWidget(icon_label_);
    
    // 创建名称标签
    name_label_ = new QLabel(this);
    name_label_->setAlignment(Qt::AlignCenter);
    name_label_->setWordWrap(true);
    name_label_->setMaximumWidth(100);
    QFont nameFont = name_label_->font();
    nameFont.setPointSize(10);
    name_label_->setFont(nameFont);
    layout->addWidget(name_label_);
    
    // 创建沙箱状态标签
    sandbox_label_ = new QLabel(this);
    sandbox_label_->setAlignment(Qt::AlignCenter);
    QFont sandboxFont = sandbox_label_->font();
    sandboxFont.setPointSize(8);
    sandbox_label_->setFont(sandboxFont);
    layout->addWidget(sandbox_label_);
    
    // 设置 Fluent UI 风格样式表
    setStyleSheet(R"(
        AppCard {
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 8px;
        }
        AppCard:hover {
            background-color: #F5F5F5;
            border: 1px solid #0078D4;
        }
        QLabel {
            color: #323130;
        }
    )");
    
    // 设置固定大小
    setFixedSize(120, 150);
}

// 设置应用信息
void AppCard::setAppInfo(const QIcon& icon, const QString& name, bool isSandbox) {
    // 设置图标
    icon_label_->setPixmap(icon.pixmap(64, 64));
    
    // 设置名称
    name_label_->setText(name);
    
    // 设置沙箱状态
    is_sandbox_ = isSandbox;
    if (is_sandbox_) {
        sandbox_label_->setText("✅ 沙箱模式");
        sandbox_label_->setStyleSheet("color: #107C10;");
    } else {
        sandbox_label_->setText("⚠️ 真实模式");
        sandbox_label_->setStyleSheet("color: #D13438;");
    }
    
    // 更新绘制
    update();
}

// 绘制事件（绘制沙箱标识边框）
void AppCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制圆角矩形背景
    QPainterPath path;
    path.addRoundedRect(rect(), 8, 8);
    painter.fillPath(path, Qt::white);
    
    // 绘制边框
    QPen pen;
    if (is_sandbox_) {
        // 沙箱模式：绿色边框
        pen.setColor(QColor(16, 124, 16));  // #107C10
        pen.setWidth(2);
    } else {
        // 真实模式：红色边框
        pen.setColor(QColor(209, 52, 56));  // #D13438
        pen.setWidth(2);
    }
    
    painter.setPen(pen);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
    
    // 绘制沙箱标识（右上角绿色圆点）
    if (is_sandbox_) {
        painter.setBrush(QBrush(QColor(16, 124, 16)));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(width() - 15, 5, 10, 10);
    }
}

}  // namespace launchpad
