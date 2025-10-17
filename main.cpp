#include<QApplication>
#include<QWidget>
#include<QRandomGenerator>
#include<QMovie>
#include<QLabel>
#include<QTimer>
#include<QMenu>
#include<QMessageBox>
#include"functionmenu.h"
#include"chatroom.h"

class DraggableWidget : public QWidget
{
public:
    DraggableWidget(QWidget *parent = nullptr) : QWidget(parent), m_dragging(false)
    {
        setMouseTracking(true);//启用鼠标跟踪
    }
protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if(event->button()==Qt::LeftButton)
        {
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            m_dragging = true;
            event->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPos() - m_dragPosition);
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_dragging = false;
            event->accept();
        }
    }

private:
    bool m_dragging;
    QPoint m_dragPosition;
};

int main(int argc,char *argv[])
{
    QApplication app(argc,argv);
    DraggableWidget w;//使用可拖拽的窗口类
    w.setWindowFlag(Qt::FramelessWindowHint);//隐藏标题栏窗口
    w.setWindowTitle("o(=•ェ•=)m");
    w.setFixedSize(240,240);

    //gif动画窗口
    QLabel *gifLabel = new QLabel(&w);
    gifLabel->setAlignment(Qt::AlignCenter);
    QTimer *switchTimer = new QTimer(&w);
    auto loadRandomGif = [&]()
    {
        int num = QRandomGenerator::global()->bounded(1,12);//生成1~11;上限1，下限12(不包含12)
        QString animationPath = QString(":/image/animation%1.gif").arg(num);
        QMovie *movie = new QMovie(animationPath);
        if(movie->isValid())
        {
            QMovie *oldMovie = gifLabel->movie();
            if(oldMovie)
            {
                oldMovie->stop();
                delete oldMovie;
            }
            gifLabel->setMovie(movie);
            movie->start();
        }
        else
        {
            //报错
            QMessageBox::warning(&w,"资源缺失",QString("动画资源文件不存在:\n%1\n请确保文件已添加到资源中。").arg(animationPath));
        }
    };
    QObject::connect(switchTimer,&QTimer::timeout,[&](){
        loadRandomGif();
    });
    loadRandomGif();//初始加载一次
    switchTimer->start(5000);//启动定时器，5秒切换一次

    //主窗口文字标签1
    QLabel *titleLabel = new QLabel("你好啊！o(=•ェ•=)m",&w);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        R"(
            QLabel
            {
                background-color: black;
                color: white;
                font-size: 16px;
                font-weight: bold;
                border: none;
            }
        )"
    );
    titleLabel->setGeometry(0,0,240,30);

    //为窗口 w 创建一个右键上下文菜单，包含四个选项：
    QMenu *contextMenu = new QMenu(&w);
    contextMenu->addSeparator();//分隔符,目前显示还有点问题
    //使用 CSS-like 语法美化 Qt 控件; 菜单样式设置：在显示前设置菜单的视觉样式
    contextMenu->setStyleSheet(// 在创建菜单后立即设置，不再每次右键都设置
        "QMenu {"
        "   background-color: black;"//深蓝色背景-#2c3e50，目前改为了黑色
        "   border: 1px solid #34495e;"//边框颜色
        "   border-radius: 5px;"//圆角边框
        "   padding: 5px;"//内边距
        "}"
        "QMenu::item {"
        "   padding: 8px 20px 8px 20px;" // 菜单项内边距,上右下左
        "   color: #ecf0f1;"             // 文字颜色（浅色）
        "   border-radius: 3px;"         // 菜单项圆角
        "}"
        "QMenu::item:selected {"
        "   background-color: #3498db;"  // 选中项背景色（蓝色）
        "   color: white;"               // 选中项文字颜色
        "}"
        "QMenu::separator {"
        "   height: 1px;"               // 分隔符高度
        "   background-color: #34495e;" // 分隔符颜色
        "   margin: 5px 0px 5px 0px;"   // 分隔符边距
        "}"
    );
    QAction *functionmenuAction = contextMenu->addAction("显示功能菜单");//向 rightMenu 这个菜单中添加一个菜单项,文本显示为"显示功能菜单"
    QAction *chatAction = contextMenu->addAction("聊天");
    QAction *quitAction = contextMenu->addAction("退出");
    QAction *helpAction = contextMenu->addAction("帮助");
    w.setContextMenuPolicy(Qt::CustomContextMenu);//必须设置此项;.setContextMenuPolic()设置该部件处理上下文菜单（通常由鼠标右键点击触发）的策略;Qt::CustomContextMenu我将不使用默认的上下文菜单行为，而是要自己处理并显示一个自定义的菜单。
    //处理自定义右键菜单请求;
    // 信号连接
    //     信号源：&w（窗口部件）的 customContextMenuRequested 信号

    //             触发条件：当用户在部件上请求上下文菜单时（通常是鼠标右键点击）
    // 参数意义
    //     const QPoint &pos：鼠标点击位置的局部坐标（相对于部件 w 的坐标）
    // Lambda 函数的作用
    //     捕获列表 [&]：通过引用捕获外部变量，可以访问 contextMenu 等外部对象
    //             坐标转换：使用 w.mapToGlobal(pos) 将局部坐标转换为屏幕全局坐标
    //                     菜单显示：调用 contextMenu->exec() 在正确位置显示菜单
    //工作流程
    //  用户右键点击 → 触发 customContextMenuRequested 信号 →Lambda 函数执行 → 坐标转换 → 显示菜单
    QObject::connect(&w,&QWidget::customContextMenuRequested,[&](const QPoint &pos)//[]捕获列表，()参数，{}函数体，Lambda 表达式
        {
            contextMenu->exec(w.mapToGlobal(pos));//转换为全局坐标位置
        }
    );
    // triggered 和 clicked 有什么区别？
    //     triggered: 用户通过任何方式触发的动作（点击、快捷键、菜单等;主要用于 QAction 类
    //     clicked: 仅当用户鼠标点击时触发;主要用于 QPushButton、QToolButton 等按钮类组件
    //功能菜单显示
    QObject::connect(functionmenuAction,&QAction::triggered,[&](){
        //QMessageBox::information(nullptr, "提示", "功能菜单正在开发中");
        //创建一个function实例，但不会立即显示
        functionMenu *function = new functionMenu(&w);
        //QPoint menuPos = w.mapToGlobal(QPoint(w.width(),0));//&w窗口右侧
        QPoint menuPos = w.mapToGlobal(QPoint(-function->width()-10,0));//设置functionMenu的位置，&w窗口左侧
        function->move(menuPos);
        function->show();
    });
    QObject::connect(chatAction,&QAction::triggered,[&](){
        // QMessageBox::information(nullptr, "聊天功能", "即将支持与桌面宠物聊天互动！");
        //创建一个chatroom实例，但不会立即显示
        chatroom *chat = new chatroom(&w);
        QPoint chatPos = w.mapToGlobal(QPoint(0,w.height()));
        chat->move(chatPos);
        chat->show();
    });
    // 连接退出动作到退出程序
    QObject::connect(quitAction,&QAction::triggered,[](){
        QApplication::quit();// 直接退出应用程序
    });
    QObject::connect(helpAction,&QAction::triggered,[](){
        QMessageBox::information(nullptr, "帮助", "帮助提示功能正在开发中");
    });

    w.show();
    return app.exec();
}
