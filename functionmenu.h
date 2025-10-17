#ifndef FUNCTIONMENU_H
#define FUNCTIONMENU_H

#include <QWidget>
// #include <QLayout>
#include <QVBoxLayout>
#include <QPushButton>
class functionMenu : public QWidget
{
    Q_OBJECT
public:
    explicit functionMenu(QWidget *parent = nullptr);//构造函数
    ~functionMenu();//析构函数

    void addFunction(const QString &name,const QIcon &icon = QIcon());//改进中，按钮管理优化;方法，支持动态添加功能按钮,方法提供了默认参数，使用方便
    void clearFunction();
    void setButtonStyle(const QString &style);//改进中，样式表硬编码，建议提供设置方法
    int functionCount() const;
private:
    // // 生命周期管理
    // //     布局对象需要在类的整个生命周期中存在
    // //     如果只在构造函数中创建而不保存指针，布局会在构造函数结束时被销毁
    // //     声明为成员变量确保布局持续有效
    QVBoxLayout *m_layout;//竖直布局
    // QPushButton *m_musicBtn, *m_calendarBtn, *m_feature3Btn,*m_feature4Btn,*m_moreBtn;
    QList<QPushButton*>m_buttons;//统一管理按钮
signals:
    // void musicClicked();
    // void calendarClicked();
    void functionClicked(const QString &functionName);//通用信号，便于统一处理
    //...
};

#endif // FUNCTIONMENU_H
