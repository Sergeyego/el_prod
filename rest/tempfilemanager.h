#pragma once
#include <QObject>
#include <QTemporaryDir>
#include <QByteArray>
#include <QString>

class TempFileManager : public QObject {
    Q_OBJECT
public:
    // Точка доступа к единственному экземпляру класса
    static TempFileManager& instance();

    // Метод для создания файла. Возвращает полный путь к созданному файлу.
    QString createFile(const QByteArray &data, const QString &extension = "dat", bool open_in_os = true);

    // Метод для получения пути к самой временной папке (если нужно)
    QString sessionDirPath() const;

private:
    // Приватные конструкторы и деструктор для синглтона
    explicit TempFileManager(QObject *parent = nullptr);
    ~TempFileManager() override = default;

    // Запрещаем копирование и перемещение
    TempFileManager(const TempFileManager&) = delete;
    TempFileManager& operator=(const TempFileManager&) = delete;
    TempFileManager(TempFileManager&&) = delete;
    TempFileManager& operator=(TempFileManager&&) = delete;

    QTemporaryDir m_sessionDir;
    int m_fileCounter = 0;
};