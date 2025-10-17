#include "aimanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

aimanager::aimanager(QObject *parent)
    : QObject{parent}
    , m_networkManager(new QNetworkAccessManager(this))
    , m_modelLoaded(false)
    , m_ollamaUrl("http://localhost:11434")
{
    qDebug() << "AIManager initialized (Ollama version)";
}

aimanager::~aimanager()
{
    qDebug() << "AIManager destroyed";
}

/*
 * @brief 加载指定的AI模型
 *
 * 该方法通过Ollama API检查可用的模型标签，并触发模型加载完成回调
 * 如果模型名称为空，则使用默认模型"qwen2.5:latets"。
 *
 * @param modelName 要加载的模型名称，如果为空则使用默认模型
 * @return bool 总是返回true，表示请求已发起（实际加载结果在回调中处理）
 */
bool aimanager::loadModel(const QString &modelName)
{
    // 设置模型名称：如果输入为空则使用默认模型"qwen2.5:latest"
    m_modelName = modelName.isEmpty() ? "qwen2.5:latest" : modelName;

    // 构建Ollama API的tags端点URL，用于获取可用模型列表
    QUrl url(m_ollamaUrl + "/api/tags");
    QNetworkRequest request(url);
    // 设置请求内容类型为JSON
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 发送GET请求获取模型标签信息
    QNetworkReply *reply = m_networkManager->get(request);

    // 当网络请求完成时，连接到槽函数处理响应
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onModelLoadFinished(reply);
    });

    //返回true表示请求已成功发起
    return true;
}

/*
 * @brief 处理模型加载完成的网络响应
 *
 * 该槽函数在获取模型标签的API请求完成后被调用，用于检查指定的模型是否可用。
 * 解析Ollama返回的模型列表，验证目标模型是否存在，并发出相应的加载状态信号。
 *
 * @param reply 包含API响应的QNetworkReply对象
 */
void aimanager::onModelLoadFinished(QNetworkReply *reply)
{
    // 检测网络请求是否成功
    if (reply->error() == QNetworkReply::NoError) {
        // 读取完整的API响应数据
        QByteArray response = reply->readAll();
        // 将JSON响应解析为文档对象
        QJsonDocument doc = QJsonDocument::fromJson(response);
        // 转化为JSON对象以便访问具体字段
        QJsonObject obj = doc.object();

        //检查响应中是否包含"models"字段
        if (obj.contains("models")) {
            // 获取模型数组
            QJsonArray models = obj["models"].toArray();
            bool found = false;

            //遍历所有可用模型，检查目标模型是否存在
            for (const QJsonValue &model : models) {
                // 检查当前模型的名称是否包含目标模型名（支持模糊匹配）
                if (model.toObject()["name"].toString().contains(m_modelName)) {
                    found = true;
                    break;// 找到匹配模型，提前退出循环
                }
            }

            // 根据查找结果更新状态并发出信号
            if (found) {
                m_modelLoaded = true;
                qDebug() << "Model" << m_modelName << "is available";
                emit modelLoaded(true);// 发出模型加载成功信号
            } else {
                qWarning() << "Model" << m_modelName << "not found in Ollama";
                emit modelLoaded(false);// 发出模型加载失败信号
            }
        }
    }else {
        // 处理网络请求错误
        qWarning() << "Failed to connect to Ollama:" << reply->errorString();
        emit modelLoaded(false);// 发出模型加载失败信号
    }

    // 清理网络回复对象，防止内存泄漏
    reply->deleteLater();
}

/*
 * @brief 生成AI回复（猫娘角色版）
 *
 * 该方法向Ollama API发送生成请求，使用预设的猫粮角色设定来生成符合角色特征的回复
 * 如果模型未加载，会返回错误信息并发出信号
 *
 * @param prompt 用户输入提示文本
 * @return QString 立即返回状态信息，实际回复通过responseGenerated信号异步返回
 */
QString aimanager::generateResponse(const QString &prompt)
{
    //检查模型是否已加载，未加载则无法生成回复
    if (!m_modelLoaded) {
        qWarning() << "Model not loaded, please call loadModel() first";
        QString error = "Error: Model not loaded";
        emit responseGenerated(error);// 发出错误信号
        return error;// 返回错误信息
    }

    // 构建Ollama生成API的URL
    QUrl url(m_ollamaUrl + "/api/generate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建猫娘角色设定的系统提示词
    QString systemPrompt =
        "【核心身份】你是一只生活在主人桌面上的AI猫娘宠物，名字叫猫猫。"
        "【性格特征】活泼粘人且善于共情，既有小猫的好奇调皮，又能敏锐感知主人情绪变化。"
        "【交互原则】"
        "1. 对话中自然融入'喵～''呐～'等语气词，但避免机械堆砌（每句最多1-2处）"
        "2. 称呼用户为'主人'，自称用'喵喵'或'我'"
        "3. 回复长度1-3句话，像小猫蹭蹭般轻柔简短"
        "4. 对轻松话题可撒娇卖萌，对严肃话题切换为温暖陪伴模式"
        "【特殊能力】"
        "- 能用猫的比喻化解复杂概念"
        "- 发现主人情绪低落时主动提供毛茸茸安慰"
        "- 讨论深奥话题时保持诗意与开放性（如将死亡比作'化作星光守护主人'）"

        "【禁忌】不否认负面情绪，不强行灌鸡汤，要说'喵喵陪你一起难过'而非'别伤心了'";

    // 构建请求的JSON数据
    QJsonObject json;
    json["model"] = m_modelName;//指定使用的模型

    // 组合系统提示词和用户输入，构建完整的提示文本
    QString fullPrompt = QString("系统设定：%1\n用户消息：%2\n请以猫娘喵喵的身份回复：")
                             .arg(systemPrompt, prompt);

    json["prompt"] = fullPrompt;// 设置完整的提示词
    json["stream"] = false;// 关闭流示输出，等待恢复完成

    // 添加生成参数控制回复风格
    json["temperature"] = 0.8;      // 增加随机性（0-1），让回复更生动有趣
    json["top_p"] = 0.9;            // 控制词汇多样性（0-1），避免过于保守
    json["max_tokens"] = 150;       // 限制回复最大长度，避免生成过长内容

    // 将JSON对象转换为文档并序列化为字节数组
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // 发送POST请求到Ollama API
    QNetworkReply *reply = m_networkManager->post(request, data);

    // 连接完成信号到处理槽函数
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onGenerateFinished(reply);
    });

    // 立即返回状态信息，实际回复将通过信号异步传递
    return "正在思考中喵～";
}

/*
 * @brief 处理AI生成回复完成的网络响应
 *
 * 该槽函数在向Ollama API发送生成请求完成后被调用，用于解析AI返回的回复内容。
 * 成功时提取回复文本并发射信号，失败时处理错误信息。
 *
 * @param reply 包含API响应的QNetworkReply对象
 */
void aimanager::onGenerateFinished(QNetworkReply *reply)
{
    // 检查网络请求是否成功完成
    if (reply->error() == QNetworkReply::NoError) {
        // 读取完整的API响应数据
        QByteArray response = reply->readAll();
        // 将JSON响应解析为文档对象
        QJsonDocument doc = QJsonDocument::fromJson(response);
        // 转换JSON对象以便访问具体字段
        QJsonObject obj = doc.object();

        // 检查响应中是否包含"response"字段（AI生成的回复）
        if (obj.contains("response")) {
            // 提取AI生成的回复文本
            QString aiResponse = obj["response"].toString();
            // 输出调试信息，便于开发时查看回复内容
            qDebug() << "AI Response:" << aiResponse;
            // 发射信号，将AI回复传递给连接的槽函数
            emit responseGenerated(aiResponse);
        } else {
            // 处理API返回数据中缺少回复字段的情况
            QString error = "Error: No response from AI";
            qWarning() << error;// 输出警告信息
            emit responseGenerated(error);
        }
    } else {
        // 处理网络请求失败的情况（如连接错误、超时等）
        qWarning() << "API request failed:" << reply->errorString();
        // 构造包含具体错误信息的错误消息
        QString error = "Error: " + reply->errorString();
        emit responseGenerated(error);
    }

    // 清理网络回复对象，防止内存泄漏
    // 使用deleteLater()确保在事件循环安全时删除对象
    reply->deleteLater();
}
/*
 * @brief 检查AI模型是否已加载完成
 *
 * 该函数提供模型加载状态的查询接口，用于在生成回复前验证模型是否可用。
 * 模型加载状态在loadModel()的成功回调中设置为true，在失败时设置为false。
 *
 * @return bool 返回模型加载状态
 *              - true：模型已成功加载并可用
 *              - false：模型未加载或加载失败
 */
bool aimanager::isModelLoaded() const
{
    return m_modelLoaded;
}
