#ifndef AIMANAGER_H
#define AIMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class aimanager : public QObject
{
    Q_OBJECT
public:
    explicit aimanager(QObject *parent = nullptr);
    ~aimanager();

    Q_INVOKABLE bool loadModel(const QString &modelName = "qwen2.5:latest"); // 改为模型名
    Q_INVOKABLE QString generateResponse(const QString &prompt);
    Q_INVOKABLE bool isModelLoaded() const;

signals:
    void modelLoaded(bool success);
    void responseGenerated(const QString &response);

private slots:
    void onModelLoadFinished(QNetworkReply *reply);
    void onGenerateFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    bool m_modelLoaded;
    QString m_modelName;
    QString m_ollamaUrl; // Ollama 服务地址，默认 http://localhost:11434
};

#endif // AIMANAGER_H
