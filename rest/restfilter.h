#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>

/**
 * @brief Класс RestFilter реализует value-семантику (семантику значений).
 * При вызове метода add() подфильтр КОПИРУЕТСЯ в родительскую группу.
 * Любые изменения исходного объекта ПОСЛЕ вызова add() не влияют на дерево.
 */

class RestFilter {
public:
    enum class Op {
        Eq, Ne, Gt, Gte, Lt, Lte, Like, Ilike, Between, In, Nin, Null, Nnull
    };

    enum class Group {
        And, Or
    };

    RestFilter();
    explicit RestFilter(Group groupType);

    static RestFilter rule(const QString &tablename, const QString &field, Op op, const QJsonValue &value = QJsonValue());
    static RestFilter ruleBetween(const QString &tablename, const QString &field, const QJsonValue &min, const QJsonValue &max);
    static RestFilter ruleIn(const QString &tablename, const QString &field, const QJsonArray &values, bool notIn = false);

    RestFilter& add(const RestFilter &otherFilter);
    RestFilter& addRule(const QString &tablename, const QString &field, Op op, const QJsonValue &value = QJsonValue());

    bool isGroup() const { return m_isGroup; }
    bool isValid() const { return m_isValid; }
    QString lastError() const { return m_lastError; }

    QJsonObject toJsonObject() const;
    QString toJsonString(QJsonDocument::JsonFormat format = QJsonDocument::Compact) const;
    QString toJsonStringIfValid(QJsonDocument::JsonFormat format = QJsonDocument::Compact) const;

private:
    static QString opToString(Op op);
    static QString groupToString(Group group);

    bool m_isGroup;
    bool m_isValid;
    QString m_lastError;
    Group m_groupType;
    QList<RestFilter> m_subFilters;

    QString m_tablename;
    QString m_field;
    Op m_op;
    QJsonValue m_value;
};