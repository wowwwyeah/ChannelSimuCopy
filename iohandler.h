#ifndef IOHANDLER_H
#define IOHANDLER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "channelparaconifg.h"

class IOHandler : public QObject
{
    Q_OBJECT


public:
    enum FileFormat {
        CSV,
        JSON,
        XML,
        INI
    };
    Q_ENUM(FileFormat)

    explicit IOHandler(QObject *parent = nullptr);

    // 导出数据到文件
    bool exportData(const ModelParaSetting &data,
                    const QString &filePath,
                    const QString &format,
                    QString *errorMessage = nullptr);

    // 从文件导入数据
    ModelParaSetting importData(const QString &filePath,
                                  FileFormat format,
                                  QString *errorMessage = nullptr);

    // 自动检测文件格式并导入
    ModelParaSetting importDataAutoDetect(const QString &filePath,
                                            QString *errorMessage = nullptr);

    // 获取支持的文件格式过滤器
    static QString getFormatFilter(FileFormat format);
    static QString getAllSupportedFilters();

    static QString enumToString(FileFormat value);
    static FileFormat stringToEnum(const QString &str, bool *ok = nullptr);
    QString generateFileNameByExtension(const QString &extension);

signals:
    void exportProgress(int percent);
    void importProgress(int percent);

private:
    // CSV格式处理
    bool exportToCSV(const ModelParaSetting &config, const QString &filePath, QString *errorMessage);
    ModelParaSetting importFromCSV(const QString &filePath, QString *errorMessage);

    // JSON格式处理
    bool exportToJSON(const ModelParaSetting &config, const QString &filePath, QString *errorMessage);
    ModelParaSetting importFromJSON(const QString &filePath, QString *errorMessage);

    // XML格式处理
    bool exportToXML(const ModelParaSetting &config, const QString &filePath, QString *errorMessage);
    ModelParaSetting importFromXML(const QString &filePath, QString *errorMessage);

    // INI格式处理
    bool exportToINI(const ModelParaSetting &config, const QString &filePath, QString *errorMessage);

    // 检测文件格式
    FileFormat detectFileFormat(const QString &filePath);

    // CSV行解析辅助函数
    QStringList parseCSVLine(const QString &line);

    QJsonObject pathToJson(const MultiPathType &path) const;
    MultiPathType pathFromJson(const QJsonObject &obj);
    QJsonObject configToJson(const ModelParaSetting &config) const;
    ModelParaSetting configFromJson(const QJsonObject &obj);
    void pathToXml(QXmlStreamWriter &writer, const MultiPathType &path) const;
    MultiPathType pathFromXml(QXmlStreamReader &reader);
    void configToXml(QXmlStreamWriter &writer, const ModelParaSetting &config) const;
    ModelParaSetting configFromXml(QXmlStreamReader &reader);
    QString pathToCsvLine(const MultiPathType &path) const;
    MultiPathType pathFromCsvLine(const QString &line);
    QString configToCsv(const ModelParaSetting &config) const;
    ModelParaSetting configFromCsv(const QString &csvContent);
};

#endif // IOHANDLER_H
