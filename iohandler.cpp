#include "iohandler.h"
#include <QFileInfo>
#include <QDebug>
#include <QMetaEnum>

IOHandler::IOHandler(QObject *parent) : QObject(parent)
{
}

bool IOHandler::exportData(const ModelParaSetting &data, const QString &filePath,
                           const QString &format, QString *errorMessage)
{
    try {
        switch (stringToEnum(format)) {
        case CSV:
            return exportToCSV(data, filePath, errorMessage);
        case JSON:
            return exportToJSON(data, filePath, errorMessage);
        case XML:
            return exportToXML(data, filePath, errorMessage);
        case INI:
            return exportToXML(data, filePath, errorMessage);
        default:
            if (errorMessage) *errorMessage = "不支持的文件格式";
            return false;
        }
    } catch (const std::exception &e) {
        if (errorMessage) *errorMessage = QString("导出失败: %1").arg(e.what());
        return false;
    }
}

ModelParaSetting IOHandler::importData(const QString &filePath, FileFormat format,
                                         QString *errorMessage)
{
    ModelParaSetting config;
    try {
        switch (format) {
        case CSV:
            config = importFromCSV(filePath, errorMessage);
            return config;
        case JSON:
            config = importFromJSON(filePath, errorMessage);
            return config;
        case XML:
            config = importFromXML(filePath, errorMessage);
            return config;
        default:
            if (errorMessage) *errorMessage = "不支持的文件格式";
            return config;
        }
    } catch (const std::exception &e) {
        if (errorMessage) *errorMessage = QString("导入失败: %1").arg(e.what());
        return config;
    }
}

ModelParaSetting IOHandler::importDataAutoDetect(const QString &filePath, QString *errorMessage)
{
    FileFormat format = detectFileFormat(filePath);
    return importData(filePath, format, errorMessage);
}

QString IOHandler::getFormatFilter(FileFormat format)
{
    switch (format) {
    case CSV: return "CSV文件 (*.csv)";
    case JSON: return "JSON文件 (*.json)";
    case XML: return "XML文件 (*.xml)";
    default: return "所有文件 (*.*)";
    }
}

QString IOHandler::getAllSupportedFilters()
{
    return "CSV文件 (*.csv);;JSON文件 (*.json);;XML文件 (*.xml);;所有文件 (*.*)";
}

QString IOHandler::enumToString(FileFormat value)
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<FileFormat>();
    return QString::fromUtf8(metaEnum.valueToKey(value));
}

IOHandler::FileFormat IOHandler::stringToEnum(const QString &str, bool *ok)
{
    qDebug() << "format:%1" << str;
    const QMetaEnum metaEnum = QMetaEnum::fromType<FileFormat>();
    int value = metaEnum.keyToValue(str.toUtf8().constData(), ok);
    qDebug() << "value:%1" << value;
    return static_cast<FileFormat>(value);
}

QString IOHandler::generateFileNameByExtension(const QString &extension)
{
    // 确保扩展名以点开头
    QString validExtension = extension.startsWith('.') ? extension : "." + extension;

    // 使用时间戳生成唯一文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");

    return QString("file_%1%2").arg(timestamp).arg(validExtension);
}

// CSV导出实现
bool IOHandler::exportToCSV(const ModelParaSetting &data, const QString &filePath, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = "无法创建文件";
        return false;
    }

    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#else
    stream.setEncoding(QStringConverter::Utf8);
#endif

    stream << configToCsv(data);

    file.close();
    emit exportProgress(100);
    return true;
}

// CSV导入实现
ModelParaSetting IOHandler::importFromCSV(const QString &filePath, QString *errorMessage)
{
    ModelParaSetting config;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = "无法打开文件";
        return config;
    }

    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#else
    stream.setEncoding(QStringConverter::Utf8);
#endif

    QString csvContent = stream.readAll();
    config = configFromCsv(csvContent);

    file.close();
    emit importProgress(100);
    return config;
}

// CSV行解析辅助函数
QStringList IOHandler::parseCSVLine(const QString &line)
{
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];

        if (c == '\"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '\"') {
                current += '\"';
                i++; // 跳过下一个引号
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            result.append(current);
            current.clear();
        } else {
            current += c;
        }
    }

    result.append(current);
    return result;
}

// JSON导出实现
bool IOHandler::exportToJSON(const ModelParaSetting &config, const QString &filePath, QString *errorMessage)
{
    QJsonObject rootObj = configToJson(config);
    QJsonDocument doc(rootObj);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = "无法创建文件";
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented)); // 格式化输出，便于阅读
    file.close();
    return true;
}

// JSON导入实现
ModelParaSetting IOHandler::importFromJSON(const QString &filePath, QString *errorMessage)
{
    ModelParaSetting config;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = "无法打开文件";
        return config;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        if (errorMessage) *errorMessage = "无效的JSON格式";
        return config;
    }

    config = configFromJson(doc.object());

    return config;
}

// XML导出实现
bool IOHandler::exportToXML(const ModelParaSetting &data, const QString &filePath, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = "无法创建文件";
        return false;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    configToXml(xmlWriter, data);
    file.close();
    return true;
}

// XML导入实现
ModelParaSetting IOHandler::importFromXML(const QString &filePath, QString *errorMessage)
{
    ModelParaSetting config;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = "无法打开文件";
        return config;
    }

    QXmlStreamReader xmlReader(&file);
    config = configFromXml(xmlReader);
    return config;
}

// 检测文件格式
IOHandler::FileFormat IOHandler::detectFileFormat(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    if (suffix == "csv") return CSV;
    if (suffix == "json") return JSON;
    if (suffix == "xml") return XML;

    // 尝试通过文件内容检测
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray content = file.read(1024);
        file.close();

        if (content.startsWith("{") || content.startsWith("[")) {
            return JSON;
        } else if (content.startsWith("<")) {
            return XML;
        } else if (content.contains(',') || content.contains('"')) {
            return CSV;
        }
    }

    return CSV; // 默认返回CSV
}

// MultiPathType 转 JSON
QJsonObject IOHandler::pathToJson(const MultiPathType &path) const
{
    QJsonObject obj;
    obj["pathNum"] = path.pathNum;
    obj["relativDelay"] = path.relativDelay;
    obj["antPower"] = path.antPower;
    obj["freShift"] = path.freShift;
    obj["freSpread"] = path.freSpread;
    obj["dopplerType"] = path.dopplerType;
    return obj;
}

// JSON 转 MultiPathType
MultiPathType IOHandler::pathFromJson(const QJsonObject &obj)
{
    MultiPathType path;
    path.pathNum = obj["pathNum"].toInt();
    path.relativDelay = obj["relativDelay"].toInt();
    path.antPower = obj["antPower"].toInt();
    path.freShift = obj["freShift"].toInt();
    path.freSpread = obj["freSpread"].toInt();
    path.dopplerType = obj["dopplerType"].toInt();
    return path;
}

// ModelParaSetting 转 JSON
QJsonObject IOHandler::configToJson(const ModelParaSetting &config) const
{
    QJsonObject obj;
    obj["channelNum"] = config.channelNum;
    obj["modelType"] = config.modelType;
    obj["modelName"] = config.modelName;
    obj["noisePower"] = config.noisePower;
    obj["signalAnt"] = config.signalAnt;
    obj["comDistance"] = config.comDistance;
    obj["multipathNum"] = config.multipathNum;
    obj["filterNum"] = config.filterNum;

    // 多径列表转 JSON 数组
    QJsonArray multipathArray;
    for (const auto &path : config.multipathType) {
        multipathArray.append(pathToJson(path));
    }
    obj["multipathType"] = multipathArray;

    return obj;
}

// JSON 转 ModelParaSetting
ModelParaSetting IOHandler::configFromJson(const QJsonObject &obj)
{
    ModelParaSetting config;
    config.channelNum = obj["channelNum"].toInt();
    config.modelType = obj["modelType"].toInt();
    config.modelName = obj["modelName"].toString();
    config.noisePower = obj["noisePower"].toDouble();
    config.signalAnt = obj["signalAnt"].toDouble();
    config.comDistance = obj["comDistance"].toDouble();
    config.multipathNum = obj["multipathNum"].toInt();
    config.filterNum = obj["filterNum"].toInt();

    // 解析多径列表
    QJsonArray multipathArray = obj["multipathType"].toArray();
    for (const auto &item : multipathArray) {
        config.multipathType.append(pathFromJson(item.toObject()));
    }
    return config;
}

// MultiPathType 转 XML
void IOHandler::pathToXml(QXmlStreamWriter &writer, const MultiPathType &path) const
{
    writer.writeStartElement("MultiPath"); // 多径节点
    writer.writeAttribute("pathNum", QString::number(path.pathNum));
    writer.writeTextElement("relativDelay", QString::number(path.relativDelay));
    writer.writeTextElement("antPower", QString::number(path.antPower));
    writer.writeTextElement("freShift", QString::number(path.freShift));
    writer.writeTextElement("freSpread", QString::number(path.freSpread));
    writer.writeTextElement("dopplerType", QString::number(path.dopplerType));
    writer.writeEndElement(); // </MultiPath>
}

// XML 转 MultiPathType
MultiPathType IOHandler::pathFromXml(QXmlStreamReader &reader)
{
    MultiPathType path;
    // 读取 pathNum 属性
    path.pathNum = reader.attributes().value("pathNum").toInt();

    // 遍历子节点
    while (reader.readNextStartElement()) {
        if (reader.name() == "relativDelay") {
            path.relativDelay = reader.readElementText().toInt();
        } else if (reader.name() == "antPower") {
            path.antPower = reader.readElementText().toInt();
        } else if (reader.name() == "freShift") {
            path.freShift = reader.readElementText().toInt();
        } else if (reader.name() == "freSpread") {
            path.freSpread = reader.readElementText().toInt();
        } else if (reader.name() == "dopplerType") {
            path.dopplerType = reader.readElementText().toInt();
        } else {
            reader.skipCurrentElement(); // 跳过未知节点
        }
    }
    return path;
}

// ModelParaSetting 转 XML
void IOHandler::configToXml(QXmlStreamWriter &writer, const ModelParaSetting &config) const
{
    writer.writeStartDocument(); // XML 文档头
    writer.writeStartElement("ModelParaSetting"); // 根节点

    // 基础参数
    writer.writeTextElement("channelNum", QString::number(config.channelNum));
    writer.writeTextElement("modelType", QString::number(config.modelType));
    writer.writeTextElement("modelName", config.modelName);
    writer.writeTextElement("noisePower", QString::number(config.noisePower, 'f', 2));
    writer.writeTextElement("signalAnt", QString::number(config.signalAnt, 'f', 2));
    writer.writeTextElement("comDistance", QString::number(config.comDistance, 'f', 2));
    writer.writeTextElement("multipathNum", QString::number(config.multipathNum));
    writer.writeTextElement("filterNum", QString::number(config.filterNum));

    // 多径列表
    writer.writeStartElement("MultiPathList");
    for (const auto &path : config.multipathType) {
        pathToXml(writer, path);
    }
    writer.writeEndElement(); // </MultiPathList>

    writer.writeEndElement(); // </ModelParaSetting>
    writer.writeEndDocument();
}

// XML 转 ModelParaSetting
ModelParaSetting IOHandler::configFromXml(QXmlStreamReader &reader)
{
    ModelParaSetting config;

    // 遍历 XML 节点
    while (reader.readNextStartElement()) {
        if (reader.name() == "ModelParaSetting") {
            // 解析根节点子元素
            while (reader.readNextStartElement()) {
                if (reader.name() == "channelNum") {
                    config.channelNum = reader.readElementText().toInt();
                } else if (reader.name() == "modelType") {
                    config.modelType = reader.readElementText().toInt();
                } else if (reader.name() == "modelName") {
                    config.modelName = reader.readElementText();
                } else if (reader.name() == "noisePower") {
                    config.noisePower = reader.readElementText().toDouble();
                } else if (reader.name() == "signalAnt") {
                    config.signalAnt = reader.readElementText().toDouble();
                } else if (reader.name() == "comDistance") {
                    config.comDistance = reader.readElementText().toDouble();
                } else if (reader.name() == "multipathNum") {
                    config.multipathNum = reader.readElementText().toInt();
                } else if (reader.name() == "filterNum") {
                    config.filterNum = reader.readElementText().toInt();
                } else if (reader.name() == "MultiPathList") {
                    // 解析多径列表
                    while (reader.readNextStartElement()) {
                        if (reader.name() == "MultiPath") {
                            config.multipathType.append(pathFromXml(reader));
                        } else {
                            reader.skipCurrentElement();
                        }
                    }
                } else {
                    reader.skipCurrentElement();
                }
            }
        } else {
            reader.skipCurrentElement();
        }
    }

    // 检查 XML 错误
    if (reader.hasError()) {
        qWarning() << "XML 解析错误：" << reader.errorString();
    }
    return config;
}

// MultiPathType 转 CSV 行
QString IOHandler::pathToCsvLine(const MultiPathType &path) const
{
    QStringList fields;
    fields << QString::number(path.pathNum)
           << QString::number(path.relativDelay)
           << QString::number(path.antPower)
           << QString::number(path.freShift)
           << QString::number(path.freSpread)
           << QString::number(path.dopplerType);
    return fields.join(","); // 逗号分隔（CSV 标准分隔符）
}

// CSV 行转 MultiPathType
MultiPathType IOHandler::pathFromCsvLine(const QString &line)
{
    MultiPathType path;
    QStringList fields = line.split(",", Qt::SkipEmptyParts);
    if (fields.size() >= 6) {
        path.pathNum = fields[0].toInt();
        path.relativDelay = fields[1].toInt();
        path.antPower = fields[2].toInt();
        path.freShift = fields[3].toInt();
        path.freSpread = fields[4].toInt();
        path.dopplerType = fields[5].toInt();
    }
    return path;
}

// ModelParaSetting 转 CSV
QString IOHandler::configToCsv(const ModelParaSetting &config) const
{
    QStringList lines;

    // 第1行：基础参数表头（可选，便于阅读）
    lines << "channelNum,modelType,modelName,noisePower,signalAnt,comDistance,multipathNum,filterNum";
    // 第2行：基础参数值
    QStringList baseFields;
    baseFields << QString::number(config.channelNum)
               << QString::number(config.modelType)
               << config.modelName
               << QString::number(config.noisePower, 'f', 2)
               << QString::number(config.signalAnt, 'f', 2)
               << QString::number(config.comDistance, 'f', 2)
               << QString::number(config.multipathNum)
               << QString::number(config.filterNum);
    lines << baseFields.join(",");

    // 第3行：多径参数表头
    lines << "pathNum,relativDelay,antPower,freShift,freSpread,dopplerType";
    // 第4行开始：多径参数值
    for (const auto &path : config.multipathType) {
        lines << pathToCsvLine(path);
    }

    return lines.join("\n"); // 换行分隔行
}

// CSV 转 ModelParaSetting
ModelParaSetting IOHandler::configFromCsv(const QString &csvContent)
{
    ModelParaSetting config;
    QStringList lines = csvContent.split("\n", Qt::SkipEmptyParts);
    if (lines.size() < 2) return config;

    // 解析基础参数（第2行，跳过表头）
    QStringList baseFields = lines[1].split(",", Qt::SkipEmptyParts);
    if (baseFields.size() >= 8) {
        config.channelNum = baseFields[0].toInt();
        config.modelType = baseFields[1].toInt();
        config.modelName = baseFields[2];
        config.noisePower = baseFields[3].toDouble();
        config.signalAnt = baseFields[4].toDouble();
        config.comDistance = baseFields[5].toDouble();
        config.multipathNum = baseFields[6].toInt();
        config.filterNum = baseFields[7].toInt();
    }

    // 解析多径参数（从第4行开始，跳过表头）
    for (int i = 3; i < lines.size(); ++i) {
        config.multipathType.append(pathFromCsvLine(lines[i]));
    }

    return config;
}
