#include "functionmenu.h"
#include "musicplayer.h"
#include <QLayout>
#include <QMessageBox>
functionMenu::functionMenu(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlags(Qt::Popup|Qt::FramelessWindowHint);//设置窗口标志：弹出式无边框窗口
    setAttribute(Qt::WA_TranslucentBackground);//设置窗口属性：创建透明或半透明窗口
    // 在C++中，当你在成员函数中使用一个名称时，编译器按以下顺序查找：
    //         当前作用域的局部变量
    //         类的成员变量
    //         基类的成员
    //         命名空间中的名称
    //QVBoxLayout *m_layout;//局部变量，改为成员变量的方法来写
    m_layout = new QVBoxLayout(this);// 这里使用的是局部变量！改了，改为成员变量了,解决布局对象生命周期问题
    m_layout->setSpacing(10);//设置布局内各个组件之间的间隔距离。
    m_layout->setContentsMargins(10,10,10,10);//设置布局整体与其父容器边界之间的边距。左上右下
    //设置样式(可选)
    setStyleSheet(
        "QPushButton {"
        "   background-color: #2c3e50;"
        "   color: white;"
        "   border: 10px;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3498db;"
        "}"
        );
    //改为了addFunction统一管理
    // m_musicBtn = new QPushButton("音乐");
    // m_calendarBtn = new QPushButton("日历+计划书");
    // m_feature3Btn = new QPushButton("功能3");
    // m_feature4Btn = new QPushButton("功能4");
    // m_moreBtn = new QPushButton("...");
    // //可以设置图标(如果有的话)
    // //btn4->setIcon(QIcon(":/icon.png"));

    // //添加到布局
    // m_layout->addWidget(m_musicBtn);
    // m_layout->addWidget(m_calendarBtn);
    // m_layout->addWidget(m_feature3Btn);
    // m_layout->addWidget(m_feature4Btn);
    // m_layout->addWidget(m_moreBtn);
    // connect(m_musicBtn, &QPushButton::clicked, this, &functionMenu::musicClicked);
    // connect(m_calendarBtn, &QPushButton::clicked, this, &functionMenu::calendarClicked);

    //使用addFunction统一创建按钮
    addFunction("音乐");
    addFunction("日历+计划书");
    addFunction("功能3");
    addFunction("功能4");
    addFunction("...");

    // //连接特定信号示例
    // if(m_buttons.size()>0)
    // {
    //     connect(m_buttons[0], &QPushButton::clicked, this, &functionMenu::musicClicked);
    // }
    // if(m_buttons.size()>1)
    // {
    //     connect(m_buttons[1], &QPushButton::clicked, this, &functionMenu::calendarClicked);
    // }

}
functionMenu::~functionMenu()
{
    //为防止内存泄漏，建议在析构函数中释放资源

    // // 方案1：让布局自动管理（推荐）
    // delete m_layout;

    // 方案2：手动管理（如果需要特殊清理）
    for(QPushButton *btn:m_buttons)
    {
        delete btn;
    }
    delete m_layout;// // 布局会自动删除其管理的子组件
}

void functionMenu::addFunction(const QString &name, const QIcon &icon)
{
    QPushButton *btn = new QPushButton(name);
    if(!icon.isNull()) btn->setIcon(icon);
    m_layout->addWidget(btn);
    m_buttons.append(btn);

    connect(btn, &QPushButton::clicked,[this, name](){
        emit functionClicked(name);

        if (name == "音乐") {
            // 创建音乐播放器窗口，传递正确的父窗口
            MusicPlayer *musicPlayer = new MusicPlayer(this->parentWidget());
            musicPlayer->show();
        } else {
            QMessageBox::information(nullptr, name, "该功能正在开发中！");
        }
    });
}

void functionMenu::clearFunction()
{
    for(QPushButton *btn:m_buttons)
    {
        m_layout->removeWidget(btn);
        delete btn;
    }
    m_buttons.clear();
}

void functionMenu::setButtonStyle(const QString &style)
{
    setStyleSheet(style);
}

int functionMenu::functionCount() const
{
    return m_buttons.size();
}
