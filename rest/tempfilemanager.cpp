#include "tempfilemanager.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

TempFileManager& TempFileManager::instance() {
    static TempFileManager _instance;
    return _instance;
}

TempFileManager::TempFileManager(QObject *parent)
    : QObject(parent)
    , m_sessionDir(QDir::tempPath() + "/" + QApplication::applicationName() + "_XXXXXX")
{
    /*if (m_sessionDir.isValid()) {
        qDebug() << "Синглтон временных файлов запущен. Папка:" << m_sessionDir.path();
    } else {
        qCritical() << "Критическая ошибка: Не удалось создать временную папку сессии!";
    }*/
}

QString TempFileManager::createFile(const QByteArray &data, const QString &extension, bool open_in_os) {
    if (!m_sessionDir.isValid()) {
        return QString();
    }

    QString fileName = QString("%1/file_%2.%3")
                           .arg(m_sessionDir.path())
                           .arg(++m_fileCounter)
                           .arg(extension);

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();

        if (open_in_os) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }
        return fileName;
    }

    return QString();
}

QString TempFileManager::sessionDirPath() const {
    return m_sessionDir.isValid() ? m_sessionDir.path() : QString();
}
