#ifndef CHATROOM_H
#define CHATROOM_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPoint>
#include <QStringList>
#include <QRandomGenerator>
#include <QKeyEvent>
#include "aimanager.h"
class chatroom : public QWidget
{
    Q_OBJECT
public:
    explicit chatroom(QWidget *parent = nullptr);
     ~chatroom();

    void keyPressEvent(QKeyEvent *event) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void sendMessage();
    void generatePetResponse(const QString &response);
    void toggleTheme(); // 主题切换槽函数
    void onAImodelLoaded(bool success);//AI模型加载完成槽函数
    void onAIResponseGenerated(const QString &response);//AI回复生成槽函数

private:
    QTextEdit *chatDisplay;
    QLineEdit *inputField;
    QPushButton *sendButton;
    QPushButton *closeButton;
    QPushButton *themeButton;
    QPushButton *fileButton;
    QPushButton *voiceButton;
    QPushButton *aiToggleButton;//AI开关按钮
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;

    bool m_dragging;
    QPoint m_dragPosition;
    bool isDarkTheme; // 主题状态标志
    bool aiEnabled;//AI功能开关状态

    aimanager *aiManager;//AI管理器实例

    QStringList greetingsResponses;
    QStringList questionResponses;
    QStringList emotionResponses;
    QStringList randomResponses;
    QStringList specialResponses;

    void setupUI();
    void setupStyle();
    void applyDarkTheme();    // 深色主题
    void applyLightTheme();   // 浅色主题
    void initializeResponses();
    QString getRandomResponse(const QStringList &responses);
    void analyzeMessage(const QString &message);
    void toggleAI();//切换AI功能；
    void loadAIModel();//加载AI模型
};

#endif // CHATROOM_H
