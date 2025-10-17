#include "musicplayer.h"
#include <QMouseEvent>
#include <QDir>
#include <QFileDialog>
#include <QTime>
#include <QStyle>
#include <QApplication>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QDirIterator>

/*
 * MusicPlayer 构造函数
 * 初始化音乐播放器窗口和组件
 *
 * @param parent：父窗口部件，用于窗口定位和内存管理
 */
MusicPlayer::MusicPlayer(QWidget *parent)
    : QWidget(parent), m_dragging(false), m_currentIndex(0)// 初始化父类和成员变量
{
    // 设置窗口属性：工具窗口、无边框、透明背景
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(280, 400);// 固定窗口尺寸

    // 初始化媒体播放组件
    m_player = new QMediaPlayer(this);// 创建媒体播放器
    m_audioOutput = new QAudioOutput(this);// 创建音频输出
    m_player->setAudioOutput(m_audioOutput);// 设置播放器的音频输出
    m_audioOutput->setVolume(0.6);// 默认音量设置60%

    // 初始化各种组件
    setupUI();// 设置用户界面
    setupConnections();// 建立信号槽连接
    applyBlueBlackTheme();// 应用蓝黑主题
    loadSongs();// 加载音乐文件
    createTrayIcon();// 创建系统托盘图标

    // 添加阴影效果增强视觉体验
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(10);// 阴影模糊半径
    shadowEffect->setColor(QColor(0, 0, 0, 160));// 半透明黑色阴影
    shadowEffect->setOffset(0, 5);// 阴影偏移量
    setGraphicsEffect(shadowEffect);//应用阴影效果

    // 如果有父窗口，将播放器定位在父窗口右侧
    if (parent) {
        QPoint parentPos = parent->mapToGlobal(QPoint(0, 0));// 获取父窗口全局坐标
        move(parentPos.x() + parent->width(), parentPos.y());// 移动到父窗口右侧
    }
}

/*
 * MusicPlayer 析构函数
 * 负责清理资源和确保安全退出
 */
MusicPlayer::~MusicPlayer()
{
    // 停止媒体播放，释放音频设备资源
    m_player->stop();

    // 清理系统托盘图标
    if (m_trayIcon) {
        m_trayIcon->hide();  // 隐藏托盘图标，避免残留
    }

    // 注意：QObject 的父子关系会自动销毁子对象
    // m_player 和 m_trayIcon 作为子对象会被自动删除
}

/*
 * 设置用户界面
 * 初始化音乐播放器的所有UI组件和布局
 */
void MusicPlayer::setupUI()
{
    // ==================== 标题栏区域 ====================
    // 创建标题栏容器，使用渐变背景和圆角设计
    QWidget *titleWidget = new QWidget(this);
    titleWidget->setFixedHeight(35);// 固定高度
    titleWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1a1a2e, stop:1 white); border-radius: 12px 12px 0 0;");

    //创建标题标签，显示应用名称和图标
    m_titleLabel = new QLabel("🎵 猫猫音乐 🎵", titleWidget);
    m_titleLabel->setAlignment(Qt::AlignCenter);// 居中对齐
    m_titleLabel->setStyleSheet("color: #4cc9f0; font-size: 13px; font-weight: bold; background: transparent;");

    // 最小化按钮
    m_minimizeBtn = new QPushButton("−", titleWidget);
    m_minimizeBtn->setFixedSize(22, 22);// 固定按钮尺寸
    m_minimizeBtn->setStyleSheet(/* 渐变蓝色主题样式 */
        "QPushButton {"
        "    background: rgba(76, 201, 240, 0.2);"
        "    color: #4cc9f0;"
        "    border: none;"
        "    border-radius: 11px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(76, 201, 240, 0.3);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(76, 201, 240, 0.4);"
        "}"
        );

    //关闭按钮
    m_closeBtn = new QPushButton("×", titleWidget);
    m_closeBtn->setFixedSize(22, 22);
    m_closeBtn->setStyleSheet(/* 渐变蓝色主题样式 */
        "QPushButton {"
        "    background: rgba(76, 201, 240, 0.2);"
        "    color: #4cc9f0;"
        "    border: none;"
        "    border-radius: 11px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(76, 201, 240, 0.3);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(76, 201, 240, 0.4);"
        "}"
        );

    // 标题栏布局：标签居左，按钮居右
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(10, 0, 10, 0);// 布局边距
    titleLayout->addWidget(m_titleLabel);// 添加标题标签
    titleLayout->addWidget(m_minimizeBtn);// 添加最小化按钮
    titleLayout->addWidget(m_closeBtn);// 添加关闭按钮

    // ==================== 歌曲信息区域 ====================
    QWidget *songInfoWidget = new QWidget(this);
    songInfoWidget->setStyleSheet(/* 半透明蓝黑背景带边框 */
        "background: rgba(26, 26, 46, 0.5);"
        "border-radius: 12px;"
        "border: 1px solid rgba(76, 201, 240, 0.3);"
        );

    //歌曲标题标签
    m_songTitle = new QLabel("蓝色幻想曲", songInfoWidget);
    m_songTitle->setAlignment(Qt::AlignCenter);
    m_songTitle->setStyleSheet(/* 白色文字样式 */
        "color: #e2e8f0;"
        " font-size: 14px;"
        " font-weight: bold;"
        " background: transparent;"
        " margin: 5px;"
        );

    // 歌手和时长信息标签
    m_songArtist = new QLabel("深蓝乐队 • 3:12", songInfoWidget);
    m_songArtist->setAlignment(Qt::AlignCenter);
    m_songArtist->setStyleSheet(/* 半透明白色文字样式 */
        "color: rgba(226, 232, 240, 0.8);"
        " font-size: 11px;"
        " background: transparent;"
        " margin: 5px;"
        );

    // 歌曲信息垂直布局
    QVBoxLayout *songInfoLayout = new QVBoxLayout(songInfoWidget);
    songInfoLayout->setSpacing(2);// 控件间距
    songInfoLayout->setContentsMargins(10, 8, 10, 8);// 布局边距
    songInfoLayout->addWidget(m_songTitle);// 添加歌曲标题
    songInfoLayout->addWidget(m_songArtist);// 添加歌手信息

    // ==================== 进度条区域 ====================
    QWidget *progressWidget = new QWidget(this);
    progressWidget->setStyleSheet("background: transparent;");// 透明背景

    //进度条滑块，水平方向
    m_progressSlider = new QSlider(Qt::Horizontal, progressWidget);
    m_progressSlider->setStyleSheet(/* 自定义滑块样式：蓝色渐变进度 */
        "QSlider::groove:horizontal {"
        "    border: none;"
        "    height: 4px;"
        "    background: rgba(76, 201, 240, 0.2);"
        "    border-radius: 2px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1a1a2e, stop:1 #4cc9f0);"
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #e2e8f0;"
        "    border: 2px solid #4cc9f0;"
        "    width: 12px;"
        "    margin: -6px 0;"
        "    border-radius: 6px;"
        "}"
        );

    //当前时间和总时间标签
    m_currentTime = new QLabel("0:00", progressWidget);
    m_currentTime->setStyleSheet("color: #1a1a2e; font-size: 10px;");

    m_totalTime = new QLabel("3:12", progressWidget);
    m_totalTime->setStyleSheet("color: #1a1a2e; font-size: 10px;");

    // 时间信息水平布局：当前时间居左，总时间居右
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->addWidget(m_currentTime);// 当前时间
    timeLayout->addStretch();// 弹性空间
    timeLayout->addWidget(m_totalTime);// 总时间

    // 进度条区域垂直布局
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->setSpacing(5);// 控件间距
    progressLayout->setContentsMargins(0, 0, 0, 0);// 布局边距
    progressLayout->addWidget(m_progressSlider);// 进度条
    progressLayout->addLayout(timeLayout);// 时间信息

    // ==================== 控制按钮区域 ====================
    QWidget *controlWidget = new QWidget(this);
    controlWidget->setStyleSheet("background: transparent;");

    //创建控制按钮；上一首、播放/暂停、下一首
    m_prevBtn = new QPushButton("⏮", controlWidget);
    m_playBtn = new QPushButton("▶", controlWidget);
    m_nextBtn = new QPushButton("⏭", controlWidget);

    // 统一的按钮样式：蓝黑色渐变主题
    QString buttonStyle =
        "QPushButton {"/* 渐变背景、蓝色边框、圆角设计 */
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 #1a1a2e, stop:1 #16213e);"
        "    color: #4cc9f0;"
        "    border: 1px solid #4cc9f0;"
        "    border-radius: 8px;"
        "    font-size: 12px;"
        "    min-width: 35px;"
        "    min-height: 35px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 #16213e, stop:1 #0f3460);"
        "    border: 1px solid #4361ee;"
        "}"
        "QPushButton:pressed {"
        "    background: #0f3460;"
        "}";

    m_prevBtn->setStyleSheet(buttonStyle);
    m_playBtn->setStyleSheet(buttonStyle);
    m_nextBtn->setStyleSheet(buttonStyle);
    m_playBtn->setFixedSize(45, 45);// 播放按钮稍大，突出显示

    // 控制按钮水平布局：按钮居中显示
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setSpacing(10);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addStretch();// 左侧弹性空间
    controlLayout->addWidget(m_prevBtn);// 上一首按钮
    controlLayout->addWidget(m_playBtn);// 播放按钮
    controlLayout->addWidget(m_nextBtn);// 下一首按钮
    controlLayout->addStretch();// 右侧弹性空间

    // ==================== 音量控制区域 ====================
    QWidget *volumeWidget = new QWidget(this);
    volumeWidget->setStyleSheet("background: transparent;");

    // 音量图标
    QLabel *volumeIcon = new QLabel("🔊", volumeWidget);
    volumeIcon->setStyleSheet("color: #4cc9f0; font-size: 12px;");

    // 音量滑块
    m_volumeSlider = new QSlider(Qt::Horizontal, volumeWidget);
    m_volumeSlider->setRange(0, 100);// 音量范围0-100
    m_volumeSlider->setValue(60);// 默认音量60%
    m_volumeSlider->setFixedHeight(20);// 固定高度
    m_volumeSlider->setStyleSheet(m_progressSlider->styleSheet());// 复用进度条样式

    // 音量控制水平布局：图标+滑块
    QHBoxLayout *volumeLayout = new QHBoxLayout(volumeWidget);
    volumeLayout->setSpacing(8);
    volumeLayout->setContentsMargins(0, 0, 0, 0);
    volumeLayout->addWidget(volumeIcon);// 音量图标
    volumeLayout->addWidget(m_volumeSlider);// 音量滑块

    // ==================== 播放列表区域 ====================
    QWidget *playlistWidget = new QWidget(this);
    playlistWidget->setStyleSheet("background: transparent;");

    // 播放列表标题
    QLabel *playlistLabel = new QLabel("🎼 播放列表", playlistWidget);
    playlistLabel->setStyleSheet("color: #4cc9f0; font-size: 12px; font-weight: bold;");

    // 播放列表控件
    m_playlist = new QListWidget(playlistWidget);
    m_playlist->setStyleSheet(/* 半透明背景、蓝色边框、自定义选中效果 */
        "QListWidget {"
        "    background: rgba(26, 26, 46, 0.4);"
        "    border: 1px solid rgba(76, 201, 240, 0.3);"
        "    border-radius: 8px;"
        "    color: #e2e8f0;"
        "    font-size: 11px;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    padding: 6px 10px;"
        "    border-bottom: 1px solid rgba(76, 201, 240, 0.1);"
        "    background: transparent;"
        "}"
        "QListWidget::item:selected {"
        "    background: rgba(76, 201, 240, 0.3);"
        "    border-radius: 4px;"
        "}"
        );

    // 播放列表垂直布局
    QVBoxLayout *playlistLayout = new QVBoxLayout(playlistWidget);
    playlistLayout->setSpacing(5);
    playlistLayout->setContentsMargins(0, 0, 0, 0);
    playlistLayout->addWidget(playlistLabel);// 播放列表标题
    playlistLayout->addWidget(m_playlist);// 播放列表控件

    // ==================== 主布局 ====================
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);// 控件间距
    mainLayout->setContentsMargins(12, 12, 12, 12);// 窗口边距

    // 按顺序添加所有UI组件
    mainLayout->addWidget(titleWidget);// 标题栏
    mainLayout->addWidget(songInfoWidget);// 歌曲信息
    mainLayout->addWidget(progressWidget);// 进度条
    mainLayout->addWidget(controlWidget);// 控制按钮
    mainLayout->addWidget(volumeWidget);// 音量控制
    m_playlist->setMaximumHeight(100);// 限制播放列表最大高度
    mainLayout->addWidget(playlistWidget);// 播放列表

    setLayout(mainLayout);// 应用主布局
}

void MusicPlayer::setupConnections()
{
    connect(m_playBtn, &QPushButton::clicked, this, &MusicPlayer::playPause);
    connect(m_prevBtn, &QPushButton::clicked, this, &MusicPlayer::prevSong);
    connect(m_nextBtn, &QPushButton::clicked, this, &MusicPlayer::nextSong);
    connect(m_closeBtn, &QPushButton::clicked, this, &MusicPlayer::hide);
    connect(m_minimizeBtn, &QPushButton::clicked, this, &MusicPlayer::minimizeToTray);  // 新增
    connect(m_progressSlider, &QSlider::sliderMoved, m_player, &QMediaPlayer::setPosition);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MusicPlayer::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MusicPlayer::onDurationChanged);
    connect(m_playlist, &QListWidget::itemClicked, this, &MusicPlayer::onPlaylistItemClicked);
    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_audioOutput->setVolume(value / 100.0);
    });
}

void MusicPlayer::applyBlueBlackTheme()
{
    // 使用与聊天室相同的深蓝黑色主题
    setStyleSheet(
        "MusicPlayer {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "        stop:0 #1a1a2e, stop:0.5 #16213e, stop:1 #0f3460);"
        "    border-radius: 16px;"
        "    border: 2px solid #4cc9f0;"
        "}"
        );
}

void MusicPlayer::loadSongs()
{
    //清空现有歌曲
    m_songs.clear();
    m_playlist->clear();
    //扫描资源文件中的MP3文件
    QStringList musicFilters;
    musicFilters << "*.mp3"<<"*.wav"<<"*.flac";
    //扫描资源文件中的音乐文件
    QDirIterator it(":", musicFilters, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        QString filePath = it.next();
        QString fileName = QFileInfo(filePath).fileName();
        QString baseName = QFileInfo(filePath).baseName(); // 不带扩展名的文件名
        // 添加到歌曲列表
        m_songs.append(qMakePair(baseName, filePath));
        m_playlist->addItem(QString("🎵 %1").arg(baseName));

        qDebug() << "找到音乐文件:" << filePath;
    }
    // 如果没有找到音乐文件，添加默认提示
    if (m_songs.isEmpty()) {
        m_songs.append(qMakePair(QString("未找到音乐文件"), QString("请将MP3文件添加到资源中")));
        m_playlist->addItem("🎵 未找到音乐文件");
        qDebug() << "未找到音乐文件，请检查资源文件配置";
    }

    // 设置第一首歌为当前歌曲
    if (!m_songs.isEmpty()) {
        m_songTitle->setText(m_songs[0].first);
        m_songArtist->setText("准备播放...");
        m_playlist->setCurrentRow(0);
    }
}



void MusicPlayer::createTrayIcon()
{
    // 创建托盘图标
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/image/icon.png")); // 请替换为实际图标路径
    m_trayIcon->setToolTip("猫猫音乐");

    // 创建托盘菜单
    m_trayMenu = new QMenu(this);
    QAction *restoreAction = new QAction("恢复窗口", this);
    QAction *quitAction = new QAction("退出", this);

    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayMenu->addAction(restoreAction);
    m_trayMenu->addAction(quitAction);

    m_trayIcon->setContextMenu(m_trayMenu);

    // 托盘图标点击事件
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MusicPlayer::onTrayIconActivated);

    m_trayIcon->show();
}

void MusicPlayer::minimizeToTray()
{
    // 最小化到系统托盘
    hide();
    if (m_trayIcon) {
        m_trayIcon->showMessage(
            "猫猫音乐播放器",
            "播放器已最小化到系统托盘",
            QSystemTrayIcon::Information,
            2000
            );
    }
}

//播放逻辑
void MusicPlayer::playPause()
{
    if(m_songs.isEmpty())
    {
        qDebug()<<"没有可播放的音乐文件";
        return;
    }
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
    {
        m_player->pause();
        m_playBtn->setText("▶");
    }
    else
    {
        QString filePath = m_songs[m_currentIndex].second;

        // 修改这里：使用 QUrl 而不是 QUrl::fromLocalFile
        if (filePath.startsWith(":")) {
            // 资源文件
            m_player->setSource(QUrl(filePath));
        } else {
            // 本地文件
            m_player->setSource(QUrl::fromLocalFile(filePath));
        }

        m_player->play();
        m_playBtn->setText("⏸");

        // 更新歌曲信息
        m_songTitle->setText(m_songs[m_currentIndex].first);
        m_songArtist->setText("播放中...");

        qDebug() << "正在播放:" << filePath;
    }
}


void MusicPlayer::updateCurrentSong()
{
    if (m_currentIndex >= 0 && m_currentIndex < m_songs.size()) {
        m_playlist->setCurrentRow(m_currentIndex);
        m_songTitle->setText(m_songs[m_currentIndex].first);

        // 停止当前播放
        m_player->stop();
        m_playBtn->setText("▶");

        // 如果是真实音乐文件，准备播放
        QString filePath = m_songs[m_currentIndex].second;
        if (QFile::exists(filePath)) {
            m_songArtist->setText("准备播放");
            m_player->setSource(QUrl::fromLocalFile(filePath));
        } else {
            m_songArtist->setText("文件不存在");
        }

        // 重置进度条
        m_progressSlider->setValue(0);
        m_currentTime->setText("0:00");
    }
}

void MusicPlayer::nextSong()
{
    if (m_songs.isEmpty()) return;

    m_currentIndex = (m_currentIndex + 1) % m_songs.size();
    updateCurrentSong();

    // 如果正在播放，自动播放下一首
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        QTimer::singleShot(100, this, &MusicPlayer::playPause);
    }
}

void MusicPlayer::prevSong()
{
    m_currentIndex = (m_currentIndex - 1 + m_songs.size()) % m_songs.size();
    updateCurrentSong();
}

void MusicPlayer::onPositionChanged(qint64 position)
{
    if (!m_progressSlider->isSliderDown()) {
        m_progressSlider->setValue(position);
    }

    QTime currentTime(0, 0);
    currentTime = currentTime.addMSecs(position);
    m_currentTime->setText(currentTime.toString("m:ss"));
}

void MusicPlayer::onDurationChanged(qint64 duration)
{
    m_progressSlider->setRange(0, duration);

    QTime totalTime(0, 0);
    totalTime = totalTime.addMSecs(duration);
    m_totalTime->setText(totalTime.toString("m:ss"));
}

void MusicPlayer::onPlaylistItemClicked(QListWidgetItem *item)
{
    m_currentIndex = m_playlist->row(item);
    updateCurrentSong();
}

void MusicPlayer::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (isHidden()) {
            showNormal();
            activateWindow();
        } else {
            hide();
        }
    }
}

void MusicPlayer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void MusicPlayer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
    }
    QWidget::mouseMoveEvent(event);
}

void MusicPlayer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void MusicPlayer::closeEvent(QCloseEvent *event)
{
    // 重写关闭事件：最小化到托盘而不是退出
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}
