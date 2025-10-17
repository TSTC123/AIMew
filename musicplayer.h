#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QDebug>  // 调试支持
#include <QFileInfo>  // 文件信息支持
class MusicPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void playPause();
    void nextSong();
    void prevSong();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onPlaylistItemClicked(QListWidgetItem *item);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void minimizeToTray();  // 新增

private:
    void setupUI();
    void setupConnections();
    void applyBlueBlackTheme();
    void loadSongs();
    void createTrayIcon();

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    // UI组件
    QLabel *m_titleLabel;
    QLabel *m_songTitle;
    QLabel *m_songArtist;
    QSlider *m_progressSlider;
    QSlider *m_volumeSlider;
    QLabel *m_currentTime;
    QLabel *m_totalTime;
    QPushButton *m_playBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QListWidget *m_playlist;
    QPushButton *m_closeBtn;
    QPushButton *m_minimizeBtn;

    // 拖动相关
    bool m_dragging;
    QPoint m_dragPosition;

    // 歌曲列表
    QList<QPair<QString, QString>> m_songs;
    int m_currentIndex;

    // 系统托盘
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;

    void updateCurrentSong();
};

#endif // MUSICPLAYER_H
