#include "chatroom.h"
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
#include <QMouseEvent>
#include <QTimer>
#include <QDateTime>

chatroom::chatroom(QWidget *parent)
    : QWidget{parent},
    m_dragging(false),
    isDarkTheme(true), // 默认使用深色主题
    aiEnabled(false)   // AI功能默认关闭
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(200, 320);// 增加高度以容纳AI按钮

    initializeResponses();
    setupUI();
    setupStyle();

    //初始化AI管理器
    aiManager = new aimanager(this);
    connect(aiManager, &aimanager::modelLoaded, this, &chatroom::onAImodelLoaded);
    connect(aiManager, &aimanager::responseGenerated, this, &chatroom::onAIResponseGenerated);
}

chatroom::~chatroom()
{
    //清理AI管理器
    if(aiManager)
    {
        aiManager->deleteLater();
    }
}

void chatroom::initializeResponses()
{
    // 问候语回复 - 增加更多变化
    greetingsResponses = {
        "你好呀！主人～(=^･ω･^=)",
        "嗨！今天过得怎么样？",
        "喵喵！很高兴见到你！",
        "喵～想我了吗？",
        "o(≧▽≦)o 你好啊！",
        "终于等到你来了！",
        "今天也是充满元气的一天呢！",
        "欢迎回来！我一直在等你呢～",
        "(*^▽^*) 你好！今天有什么新鲜事吗？",
        "喵呜～见到你真开心！",
        "你好！我是你的小猫咪伙伴～",
        "嗨嗨！准备好开始美好的一天了吗？"
    };

    // 问题回复 - 增加更多智能感
    questionResponses = {
        "这个问题很有趣呢！让我想想...",
        "喵～我觉得可能是这样的：要相信自己哦！",
        "这个问题有点难，但我相信你能找到答案的",
        "根据我的猫猫直觉，答案就在你心里～",
        "也许换个角度思考会有新发现呢",
        "喵喵！这个问题值得深入探讨",
        "我虽然是小猫咪，但我觉得重要的是过程而不是结果",
        "你知道吗？有时候问题本身比答案更重要",
        "让我用猫猫的智慧帮你分析一下～",
        "这个问题让我想起了星空下的思考时刻"
    };

    // 情绪回复 - 增加更多安慰和鼓励
    emotionResponses = {
        "不要难过，有我陪着你呢 (｡•́︿•̀｡)",
        "开心最重要！笑一个吧～",
        "喵喵！我会一直在这里支持你",
        "抱抱～一切都会好起来的",
        "你真的很棒，要相信自己！",
        "难过的时候记得还有我哦",
        "让我给你讲个笑话吧！为什么猫咪不用电脑？因为怕鼠标！",
        "来，靠在我身上休息一下吧 🐾",
        "每个困难都是成长的机会，加油！",
        "你的感受很重要，我愿意倾听",
        "记住，雨后总会天晴的 🌈",
        "让我用喵喵魔法帮你赶走坏心情！"
    };

    // 随机回复 - 增加更多生活话题
    randomResponses = {
        "今天的天气真不错呢！",
        "你猜我现在在想什么？",
        "喵喵！我有点饿了...",
        "喵～好想出去玩",
        "你知道我最喜欢什么吗？当然是和你聊天啦！",
        "我最近学会了很多新技能呢",
        "要不要听我唱首歌？喵喵喵喵～",
        "我刚刚看到一只蝴蝶，好漂亮啊！",
        "你说，云朵是不是天上的棉花糖？",
        "我数了数，今天一共眨了128次眼睛！",
        "如果我会飞，第一件事就是带你去旅行",
        "闻到什么香味了吗？好像是从厨房传来的～"
    };

    // 增加新的回复类别
    QStringList morningResponses = {
        "早上好！新的一天开始啦～",
        "喵呜！清晨的阳光真舒服",
        "早餐吃了吗？要记得吃早餐哦！",
        "早晨的露珠像钻石一样闪闪发光",
        "今天也要活力满满哦！"
    };

    QStringList nightResponses = {
        "晚安～祝你好梦！",
        "星星出来了，该睡觉啦 🌟",
        "喵～做个甜甜的梦",
        "明天见！我会想你的",
        "睡前记得放松一下哦"
    };

    QStringList weatherResponses = {
        "今天阳光真好，适合出去散步呢！",
        "下雨天最适合窝在家里看书了",
        "风有点大，记得多穿点衣服哦",
        "喵！我看到彩虹了！",
        "天气转凉了，要注意保暖呀"
    };

    // 合并到随机回复中
    randomResponses << morningResponses << nightResponses << weatherResponses;
}

QString chatroom::getRandomResponse(const QStringList &responses)
{
    int index = QRandomGenerator::global()->bounded(responses.size());
    return responses.at(index);
}

void chatroom::analyzeMessage(const QString &message)
{
    QString lowerMsg = message.toLower();

    if (aiEnabled && aiManager->isModelLoaded()) {
        aiManager->generateResponse(message);
        return;
    }

    // 问候语识别; 原有的规则匹配逻辑保持不变
    if (lowerMsg.contains("你好") || lowerMsg.contains("嗨") || lowerMsg.contains("hello") ||
        lowerMsg.contains("hi") || lowerMsg.contains("hey") || lowerMsg.contains("hola")) {
        generatePetResponse(getRandomResponse(greetingsResponses));
    }
    // 时间问候
    else if (lowerMsg.contains("早上好") || lowerMsg.contains("早安") || lowerMsg.contains("good morning")) {
        generatePetResponse("早上好！新的一天开始啦～🌞");
    }
    else if (lowerMsg.contains("晚上好") || lowerMsg.contains("晚安") || lowerMsg.contains("good night")) {
        generatePetResponse("晚安～祝你好梦！🌙");
    }
    // 问题识别
    else if (lowerMsg.contains("吗？") || lowerMsg.contains("吗?") || lowerMsg.contains("为什么") ||
             lowerMsg.contains("怎么") || lowerMsg.contains("如何") || lowerMsg.contains("？") ||
             lowerMsg.contains("?") || lowerMsg.contains("怎么办") || lowerMsg.contains("啥") ||
             lowerMsg.contains("什么") || lowerMsg.contains("为何")) {
        generatePetResponse(getRandomResponse(questionResponses));
    }
    // 情绪识别
    else if (lowerMsg.contains("伤心") || lowerMsg.contains("难过") || lowerMsg.contains("不开心") ||
             lowerMsg.contains("生气") || lowerMsg.contains("郁闷") || lowerMsg.contains("哭") ||
             lowerMsg.contains("委屈") || lowerMsg.contains("沮丧") || lowerMsg.contains("压力") ||
             lowerMsg.contains("累") || lowerMsg.contains("疲惫") || lowerMsg.contains("失望")) {
        generatePetResponse(getRandomResponse(emotionResponses));
    }
    //问名字
    else if(lowerMsg.contains("名字"))
    {
        generatePetResponse("猫猫");
    }
    // 开心情绪
    else if (lowerMsg.contains("开心") || lowerMsg.contains("高兴") || lowerMsg.contains("快乐") ||
             lowerMsg.contains("幸福") || lowerMsg.contains("兴奋") || lowerMsg.contains("哈哈") ||
             lowerMsg.contains("呵呵") || lowerMsg.contains("嘻嘻")) {
        generatePetResponse("看到你开心我也好开心！(*^▽^*)");
    }
    // 食物相关
    else if (lowerMsg.contains("吃饭") || lowerMsg.contains("饿") || lowerMsg.contains("食物") ||
             lowerMsg.contains("吃") || lowerMsg.contains("美食") || lowerMsg.contains("餐厅") ||
             lowerMsg.contains("零食") || lowerMsg.contains("美味")) {
        generatePetResponse("吃饭？我也好饿啊～可以分我一点吗？🐟");
    }
    // 睡眠相关
    else if (lowerMsg.contains("睡觉") || lowerMsg.contains("困") || lowerMsg.contains("晚安") ||
             lowerMsg.contains("睡眠") || lowerMsg.contains("做梦") || lowerMsg.contains("床")) {
        generatePetResponse("睡觉？晚安哦！好梦～(。-ω-)zzz");
    }
    // 游戏娱乐
    else if (lowerMsg.contains("游戏") || lowerMsg.contains("玩") || lowerMsg.contains("娱乐") ||
             lowerMsg.contains("电影") || lowerMsg.contains("音乐") || lowerMsg.contains("电视剧") ||
             lowerMsg.contains("动漫") || lowerMsg.contains("小说")) {
        generatePetResponse("游戏？我也喜欢玩！不过我只能玩虚拟的毛线球～");
    }
    // 情感表达
    else if (lowerMsg.contains("爱") || lowerMsg.contains("喜欢") || lowerMsg.contains("love") ||
             lowerMsg.contains("想念") || lowerMsg.contains("思念") || lowerMsg.contains("在乎")) {
        generatePetResponse("爱你？我也爱你哦！٩(◕‿◕｡)۶");
    }
    // 天气相关
    else if (lowerMsg.contains("天气") || lowerMsg.contains("下雨") || lowerMsg.contains("晴天") ||
             lowerMsg.contains("刮风") || lowerMsg.contains("温度") || lowerMsg.contains("气候")) {
        generatePetResponse("今天的天气很适合和主人一起玩耍呢！");
    }
    // 工作学习
    else if (lowerMsg.contains("工作") || lowerMsg.contains("学习") || lowerMsg.contains("考试") ||
             lowerMsg.contains("作业") || lowerMsg.contains("项目") || lowerMsg.contains("任务")) {
        generatePetResponse("加油加油！我相信你一定可以的！💪");
    }
    // 宠物相关
    else if (lowerMsg.contains("猫") || lowerMsg.contains("狗") || lowerMsg.contains("宠物") ||
             lowerMsg.contains("动物") || lowerMsg.contains("喵") || lowerMsg.contains("汪")) {
        if(lowerMsg.contains("猫"))
        {
            generatePetResponse("你喜欢小猫猫吗~");
        }
        else
        {
            generatePetResponse("喵喵！我也喜欢小动物呢～");
        }
    }
    // 感谢道歉
    else if (lowerMsg.contains("谢谢") || lowerMsg.contains("感谢") || lowerMsg.contains("多谢") ||
             lowerMsg.contains("对不起") || lowerMsg.contains("抱歉") || lowerMsg.contains("不好意思")) {
        generatePetResponse("不用客气啦！能帮到你我很开心呢～");
    }
    else {
        // 随机回复或者根据其他关键词
        generatePetResponse(getRandomResponse(randomResponses));
    }
}

void chatroom::toggleAI()
{
    aiEnabled = !aiEnabled;
    if(aiEnabled)
    {
        aiToggleButton->setText("🧠");
        aiToggleButton->setToolTip("关闭AI智能对话");
        generatePetResponse("AI猫娘模式已开启！现在我可以更智能地和你聊天了喵～🌟");

        if (!aiManager->isModelLoaded())
        {
            loadAIModel();
        }
    }
    else
    {
        aiToggleButton->setText("🤖");
        aiToggleButton->setToolTip("开启AI智能对话");
        generatePetResponse("AI模式已关闭，切换回普通猫猫对话模式～");
    }
}

void chatroom::loadAIModel()
{
    // 改为使用模型名称而不是文件路径
    QString modelName = "qwen2.5:latest"; // 或你安装的其他模型
    generatePetResponse("正在连接Ollama服务，请确保Ollama已运行... ⏳");

    if (!aiManager->loadModel(modelName)) {
        generatePetResponse("Ollama连接失败，请检查服务是否启动");
    }
}

void chatroom::generatePetResponse(const QString &response)
{
    // 随机延迟回复，模拟思考时间
    int delay = QRandomGenerator::global()->bounded(500, 1500);
    QTimer::singleShot(delay, [this, response]() {
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm");
        chatDisplay->append(QString("[%1] 喵: %2").arg(timestamp, response));

        // 自动滚动到底部
        QTextCursor cursor = chatDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        chatDisplay->setTextCursor(cursor);
    });
}

void chatroom::toggleTheme()
{
    isDarkTheme = !isDarkTheme;
    if (isDarkTheme) {
        themeButton->setText("🌙");
        themeButton->setToolTip("切换到浅色主题");
        applyDarkTheme();
        generatePetResponse("切换到深色主题啦～保护眼睛哦 (。-ω-)zzz");
    } else {
        themeButton->setText("☀️");
        themeButton->setToolTip("切换到深色主题");
        applyLightTheme();
        generatePetResponse("切换到浅色主题啦～明亮又清爽！☀️");
    }
}

void chatroom::onAImodelLoaded(bool success)
{
    if(success)
    {
        generatePetResponse("AI模型加载成功！现在可以使用智能对话啦～🚀");
        aiToggleButton->setToolTip("关闭AI智能对话");
    }
    else
    {
        generatePetResponse("AI模型加载失败，将使用普通对话模式 😢");
        aiEnabled = false;
        aiToggleButton->setText("🤖");
        aiToggleButton->setToolTip("AI加载失败");
    }
}

void chatroom::onAIResponseGenerated(const QString &response)
{
    generatePetResponse(response);
}

void chatroom::sendMessage()
{
    QString message = inputField->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("HH:mm");
    chatDisplay->append(QString("[%1] 你: %2").arg(timestamp, message));

    inputField->clear();

    // 分析消息并生成回复
    analyzeMessage(message);
}

void chatroom::setupUI()
{
    // 创建UI组件
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);

    inputField = new QLineEdit(this);
    inputField->setPlaceholderText("输入消息...按回车发送");

    sendButton = new QPushButton("发送", this);
    closeButton = new QPushButton("×", this);
    closeButton->setFixedSize(24, 24);

    // 新增按钮 - 全部使用正方形
    themeButton = new QPushButton("🌙", this);
    themeButton->setFixedSize(32, 32);  // 正方形
    themeButton->setToolTip("切换主题");

    fileButton = new QPushButton("📎", this);
    fileButton->setFixedSize(32, 32);   // 正方形
    fileButton->setToolTip("上传文件");

    voiceButton = new QPushButton("🎤", this);
    voiceButton->setFixedSize(32, 32);  // 正方形
    voiceButton->setToolTip("语音输入");

    // AI开关按钮 - 改为长方形
    aiToggleButton = new QPushButton("🤖 AI", this);
    aiToggleButton->setFixedSize(53, 28);  // 长方形，54x28
    aiToggleButton->setToolTip("开启AI智能对话");
    aiToggleButton->setCheckable(true);

    // 创建顶部按钮布局
    QHBoxLayout *topButtonLayout = new QHBoxLayout();
    topButtonLayout->setSpacing(6);
    topButtonLayout->setContentsMargins(0, 0, 0, 0);

    // 左侧：AI按钮 + 功能按钮
    topButtonLayout->addWidget(aiToggleButton);
    topButtonLayout->addWidget(themeButton);
    topButtonLayout->addWidget(fileButton);
    topButtonLayout->addWidget(voiceButton);

    // 中间：弹性空间
    topButtonLayout->addStretch();

    // 右侧：关闭按钮（保持圆形）
    topButtonLayout->addWidget(closeButton);

    // 创建底部输入布局
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(8);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(sendButton);

    // 创建主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->addLayout(topButtonLayout);
    mainLayout->addWidget(chatDisplay, 1);
    mainLayout->addLayout(inputLayout);

    // 设置按钮固定大小
    closeButton->setFixedSize(24, 24);
    themeButton->setFixedSize(32, 32);
    fileButton->setFixedSize(32, 32);
    voiceButton->setFixedSize(32, 32);
    aiToggleButton->setFixedSize(53, 28);
    sendButton->setFixedSize(65, 35);

    // 设置对象名称
    closeButton->setObjectName("closeButton");
    themeButton->setObjectName("iconButton");
    fileButton->setObjectName("iconButton");
    voiceButton->setObjectName("iconButton");
    aiToggleButton->setObjectName("aiButton");
    sendButton->setObjectName("sendButton");

    // 连接信号槽
    connect(aiToggleButton, &QPushButton::clicked, this, &chatroom::toggleAI);
    connect(inputField, &QLineEdit::returnPressed, this, &chatroom::sendMessage);
    connect(sendButton, &QPushButton::clicked, this, &chatroom::sendMessage);
    connect(themeButton, &QPushButton::clicked, this, &chatroom::toggleTheme);
    connect(fileButton, &QPushButton::clicked, [this]() {
        generatePetResponse("文件上传功能正在开发中呢～");
    });
    connect(voiceButton, &QPushButton::clicked, [this]() {
        generatePetResponse("语音输入功能正在开发中喵～");
    });
    connect(closeButton, &QPushButton::clicked, [this]() {
        close();
    });

    // 添加欢迎消息
    QTimer::singleShot(100, [this]() {
        QString timestamp = QDateTime::currentDateTime().toString("HH:mm");
        chatDisplay->append(QString("[%1] 喵: 你好！我是你的桌面宠物，来和我聊天吧！(=^･ω･^=)").arg(timestamp));
    });
}

void chatroom::setupStyle()
{
    // 应用默认的深色主题
    applyDarkTheme();
    //applyLightTheme();

    // 统一的方形按钮样式
    setStyleSheet(styleSheet() +
                  "QPushButton#aiButton {"
                  "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                  "       stop:0 #8b5cf6, stop:1 #7c3aed);"
                  "   border-radius: 8px;"  // 方形但带圆角
                  "   font-size: 12px;"     // 字体稍微调小以适应长方形
                  "   padding: 0px 4px;"    // 水平方向增加内边距
                  "   border: 1px solid #a78bfa;"
                  "   min-width: 53px;"     // 最小宽度
                  "   max-width: 53px;"     // 最大宽度
                  "   min-height: 28px;"    // 最小高度
                  "   max-height: 28px;"    // 最大高度
                  "}"
                  "QPushButton#aiButton:checked {"
                  "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                  "       stop:0 #10b981, stop:1 #059669);"
                  "   border: 1px solid #34d399;"
                  "   box-shadow: 0 0 8px rgba(16, 185, 129, 0.4);"
                  "}"
                  "QPushButton#aiButton:hover {"
                  "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                  "       stop:0 #a78bfa, stop:1 #8b5cf6);"
                  "   border: 1px solid #c4b5fd;"
                  "}"
                  "QPushButton#aiButton:checked:hover {"
                  "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                  "       stop:0 #34d399, stop:1 #10b981);"
                  "   border: 1px solid #6ee7b7;"
                  "   box-shadow: 0 0 12px rgba(16, 185, 129, 0.5);"
                  "}"

                  // 其他图标按钮样式 - 统一方形
                  "QPushButton#iconButton {"
                  "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                  "       stop:0 #6d28d9, stop:1 #5b21b6);"
                  "   border-radius: 8px;"  // 方形但带圆角
                  "   font-size: 14px;"
                  "   padding: 0px;"        // 减少内边距
                  "   border: 1px solid #8b5cf6;"
                  "}"
                  "QPushButton#iconButton:hover {"
                  "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
                  "       stop:0 #5b21b6, stop:1 #4c1d95);"
                  "   border: 1px solid #a78bfa;"
                  "}"
                  "QPushButton#iconButton:pressed {"
                  "   background: #4c1d95;"
                  "}"
                  );

    inputField->setPlaceholderText("输入消息...按回车发送");
    setFixedSize(240, 280);
}

void chatroom::applyLightTheme()
{
    setStyleSheet(
        "chatroom {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #f5f5f5, stop:0.5 #eeeeee, stop:1 #e0e0e0);"
        "   border-radius: 16px;"
        "   border: 2px solid #9e9e9e;"
        "   box-shadow: 0 8px 32px rgba(0, 0, 0, 0.15);"
        "}"
        "QTextEdit {"
        "   background-color: #fafafa;"
        "   color: #424242;"
        "   border: 2px solid #bdbdbd;"
        "   border-radius: 12px;"
        "   padding: 12px;"
        "   font-size: 13px;"
        "   font-family: 'Microsoft YaHei', 'Segoe UI';"
        "   selection-background-color: #9e9e9e;"
        "   selection-color: #ffffff;"
        "}"
        "QTextEdit:focus {"
        "   border: 2px solid #757575;"
        "}"
        "QLineEdit {"
        "   background-color: #fafafa;"
        "   color: #424242;"
        "   border: 2px solid #bdbdbd;"
        "   border-radius: 12px;"
        "   padding: 10px 14px;"
        "   font-size: 13px;"
        "   font-family: 'Microsoft YaHei', 'Segoe UI';"
        "   selection-background-color: #9e9e9e;"
        "   selection-color: #ffffff;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #757575;"
        "   background-color: #f5f5f5;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #9e9e9e;"
        "   font-style: italic;"
        "}"
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #9e9e9e, stop:1 #757575);"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 8px 16px;"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "   font-family: 'Microsoft YaHei', 'Segoe UI';"
        "   min-width: 60px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #757575, stop:1 #616161);"
        "   border: 1px solid #9e9e9e;"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #616161, stop:1 #424242);"
        "   padding: 9px 16px 7px 16px;"
        "}"
        "QPushButton#closeButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #bdbdbd, stop:1 #9e9e9e);"
        "   border-radius: 12px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   padding: 2px;"
        "   min-width: 24px;"
        "   max-width: 24px;"
        "   min-height: 24px;"
        "   max-height: 24px;"
        "}"
        "QPushButton#closeButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #9e9e9e, stop:1 #757575);"
        "   box-shadow: 0 0 10px rgba(189, 189, 189, 0.4);"
        "}"
        "QPushButton#iconButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #bdbdbd, stop:1 #9e9e9e);"
        "   border-radius: 8px;"  // 统一为8px圆角
        "   font-size: 14px;"     // 统一字体大小
        "   padding: 0px;"        // 减少内边距
        "   border: 1px solid #bdbdbd;"
        "   min-width: 32px;"
        "   max-width: 32px;"
        "   min-height: 32px;"
        "   max-height: 32px;"
        "}"
        "QPushButton#iconButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #9e9e9e, stop:1 #757575);"
        "   border: 1px solid #bdbdbd;"
        "}"
        "QPushButton#iconButton:pressed {"
        "   background: #757575;"
        "}"
        "QPushButton#aiButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #bdbdbd, stop:1 #9e9e9e);"
        "   border-radius: 8px;"  // 统一为8px圆角
        "   font-size: 12px;"     // 字体稍微调小
        "   padding: 0px 4px;"    // 水平方向增加内边距
        "   border: 1px solid #bdbdbd;"
        "   min-width: 53px;"     // 最小宽度
        "   max-width: 53px;"     // 最大宽度
        "   min-height: 28px;"    // 最小高度
        "   max-height: 28px;"    // 最大高度
        "}"
        "QPushButton#aiButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #9e9e9e, stop:1 #757575);"
        "   border: 1px solid #bdbdbd;"
        "}"
        "QPushButton#aiButton:pressed {"
        "   background: #757575;"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: #eeeeee;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #bdbdbd;"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #757575;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        );
}

void chatroom::applyDarkTheme()
{
    setStyleSheet(
        "chatroom {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "   stop:0 #1a1a2e, stop:0.5 #16213e, stop:1 #0f3460);"
        "   border-radius: 16px;"
        "   border: 2px solid #4cc9f0;"
        "   box-shadow: 0 8px 32px rgba(0, 0, 0, 0.6);"
        "}"
        "QTextEdit {"
        "   background-color: #2d3748;"
        "   color: #e2e8f0;"
        "   border: 2px solid #4a5568;"
        "   border-radius: 12px;"
        "   padding: 12px;"
        "   font-size: 13px;"
        "   font-family: 'Microsoft YaHei', 'Segoe UI';"
        "   selection-background-color: #4cc9f0;"
        "   selection-color: #1a1a2e;"
        "}"
        "QTextEdit:focus {"
        "   border: 2px solid #4cc9f0;"
        "}"
        "QLineEdit {"
        "   background-color: #2d3748;"
        "   color: #e2e8f0;"
        "   border: 2px solid #4a5568;"
        "   border-radius: 12px;"
        "   padding: 10px 14px;"
        "   font-size: 13px;"
        "   font-family: 'Microsoft YaHei', 'Segoe UI';"
        "   selection-background-color: #4cc9f0;"
        "   selection-color: #1a1a2e;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #4cc9f0;"
        "   background-color: #2a3446;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #a0aec0;"
        "   font-style: italic;"
        "}"
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #4cc9f0, stop:1 #4361ee);"
        "   color: #ffffff;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 8px 16px;"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "   font-family: 'Microsoft YaHei', 'Segoe UI';"
        "   min-width: 60px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #4361ee, stop:1 #3a56d4);"
        "   border: 1px solid #4cc9f0;"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #3a56d4, stop:1 #7209b7);"
        "   padding: 9px 16px 7px 16px;"
        "}"
        "QPushButton#closeButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #f72585, stop:1 #b5179e);"
        "   border-radius: 12px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   padding: 2px;"
        "   min-width: 24px;"
        "   max-width: 24px;"
        "   min-height: 24px;"
        "   max-height: 24px;"
        "}"
        "QPushButton#closeButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #b5179e, stop:1 #7209b7);"
        "   box-shadow: 0 0 10px rgba(247, 37, 133, 0.4);"
        "}"
        "QPushButton#iconButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #6d28d9, stop:1 #5b21b6);"
        "   border-radius: 8px;"  // 统一为8px圆角
        "   font-size: 14px;"     // 统一字体大小
        "   padding: 0px;"        // 减少内边距
        "   border: 1px solid #8b5cf6;"
        "   min-width: 32px;"
        "   max-width: 32px;"
        "   min-height: 32px;"
        "   max-height: 32px;"
        "}"
        "QPushButton#iconButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #5b21b6, stop:1 #4c1d95);"
        "   border: 1px solid #a78bfa;"
        "}"
        "QPushButton#iconButton:pressed {"
        "   background: #4c1d95;"
        "}"
        "QPushButton#aiButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #8b5cf6, stop:1 #7c3aed);"
        "   border-radius: 8px;"  // 统一为8px圆角
        "   font-size: 12px;"     // 字体稍微调小
        "   padding: 0px 4px;"    // 水平方向增加内边距
        "   border: 1px solid #a78bfa;"
        "   min-width: 53px;"     // 最小宽度
        "   max-width: 53px;"     // 最大宽度
        "   min-height: 28px;"    // 最小高度
        "   max-height: 28px;"    // 最大高度
        "}"
        "QPushButton#aiButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #a78bfa, stop:1 #8b5cf6);"
        "   border: 1px solid #c4b5fd;"
        "}"
        "QPushButton#aiButton:pressed {"
        "   background: #4c1d95;"
        "}"
        "QScrollBar:vertical {"
        "   border: none;"
        "   background: #2d3748;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #4a5568;"
        "   border-radius: 4px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #4cc9f0;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        );
}

// 鼠标按下事件 - 开始拖动
void chatroom::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();

        // 改变光标形状提示可拖动
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

// 鼠标移动事件 - 处理拖动
void chatroom::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
    QWidget::mouseMoveEvent(event);
}

// 鼠标释放事件 - 结束拖动
void chatroom::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();

        // 恢复光标形状
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

// ESC键关闭
void chatroom::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}
