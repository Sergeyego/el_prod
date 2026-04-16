#ifndef RESTCONNECTION_H
#define RESTCONNECTION_H

#include <QObject>


class RestConnection : public QObject
{
    Q_OBJECT
public:
    static RestConnection *instance();
    void setUrl(const QString &u);
    void setToken(const QString &t);
    QString getUrl() const;
    QString getToken() const;

protected:
    explicit RestConnection(QObject *parent = nullptr);

private:
    static RestConnection *connection_instance;
    QString url;
    QString token;
};

#endif // RESTCONNECTION_H
