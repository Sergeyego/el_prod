#include "RestFilter.h"
#include <QJsonDocument>
#include <QDebug>

RestFilter::RestFilter()
    : m_isGroup(true)
    , m_isValid(true)
    , m_groupType(Group::And)
{
}

RestFilter::RestFilter(Group groupType)
    : m_isGroup(true)
    , m_isValid(true)
    , m_groupType(groupType)
{
}

RestFilter RestFilter::rule(const QString &tablename, const QString &field, Op op, const QJsonValue &value)
{
    RestFilter filter;
    filter.m_isGroup = false;
    filter.m_tablename = tablename.trimmed();
    filter.m_field = field.trimmed();
    filter.m_op = op;
    filter.m_value = value;

    if (filter.m_tablename.isEmpty() || filter.m_field.isEmpty()) {
        filter.m_isValid = false;
        filter.m_lastError = "Имя таблицы (tablename) или поля (field) не могут быть пустыми.";
        qWarning() << "RestFilter Error:" << filter.m_lastError;
        return filter;
    }

    if (op == Op::Between) {
        if (!value.isArray() || value.toArray().size() != 2) {
            filter.m_isValid = false;
            filter.m_lastError = QString("Оператор BETWEEN требует массив строго из 2 элементов для поля %1.%2").arg(tablename, field);
            qWarning() << "RestFilter Error:" << filter.m_lastError;
            return filter;
        }
    }

    if (op == Op::In || op == Op::Nin) {
        if (!value.isArray() || value.toArray().isEmpty()) {
            filter.m_isValid = false;
            filter.m_lastError = QString("Операторы IN/NIN требуют непустой массив значений для поля %1.%2").arg(tablename, field);
            qWarning() << "RestFilter Error:" << filter.m_lastError;
            return filter;
        }
    }

    if (op != Op::Null && op != Op::Nnull && op != Op::Between && op != Op::In && op != Op::Nin) {
        if (value.isObject() || value.isArray()) {
            filter.m_isValid = false;
            filter.m_lastError = QString("Скалярный оператор не может принимать JSON-объект или массив в поле %1.%2").arg(tablename, field);
            qWarning() << "RestFilter Error:" << filter.m_lastError;
            return filter;
        }

        if (value.isNull() || value.isUndefined() || (value.isString() && value.toString().trimmed().isEmpty())) {
            filter.m_isValid = false;
            filter.m_lastError = QString("Оператор требует заполненного значения для поля %1.%2").arg(tablename, field);
            qWarning() << "RestFilter Error:" << filter.m_lastError;
        }
    }

    return filter;
}

RestFilter RestFilter::ruleBetween(const QString &tablename, const QString &field, const QJsonValue &min, const QJsonValue &max)
{
    //Защита типов. Границы диапазона BETWEEN не могут быть объектами или массивами
    if (min.isObject() || min.isArray() || max.isObject() || max.isArray()) {
        RestFilter invalidFilter;
        invalidFilter.m_isValid = false;
        invalidFilter.m_lastError = QString("Границы диапазона BETWEEN для поля %1.%2 не могут быть JSON-объектами или массивами").arg(tablename, field);
        qWarning() << "RestFilter Error:" << invalidFilter.m_lastError;
        return invalidFilter;
    }

    if (min.isNull() || min.isUndefined() || max.isNull() || max.isUndefined() ||
        (min.isString() && min.toString().trimmed().isEmpty()) ||
        (max.isString() && max.toString().trimmed().isEmpty()))
    {
        RestFilter invalidFilter;
        invalidFilter.m_isValid = false;
        invalidFilter.m_lastError = QString("Обе границы оператора BETWEEN должны быть заполнены для поля %1.%2").arg(tablename, field);
        qWarning() << "RestFilter Error:" << invalidFilter.m_lastError;
        return invalidFilter;
    }

    QJsonArray range;
    range.append(min);
    range.append(max);
    return RestFilter::rule(tablename, field, Op::Between, range);
}

RestFilter RestFilter::ruleIn(const QString &tablename, const QString &field, const QJsonArray &values, bool notIn)
{
    if (values.isEmpty()) {
        RestFilter invalidFilter;
        invalidFilter.m_isValid = false;
        invalidFilter.m_lastError = QString("Оператор IN/NIN требует наличия хотя бы одного элемента для поля %1.%2").arg(tablename, field);
        qWarning() << "RestFilter Error:" << invalidFilter.m_lastError;
        return invalidFilter;
    }

    // Дополнительная валидация вложенных элементов массива
    for (const auto &val : values) {
        if (val.isObject() || val.isArray() || val.isNull() || val.isUndefined()) {
            RestFilter invalidFilter;
            invalidFilter.m_isValid = false;
            invalidFilter.m_lastError = QString("Элементы массива IN/NIN для поля %1.%2 должны быть строго примитивными типами").arg(tablename, field);
            qWarning() << "RestFilter Error:" << invalidFilter.m_lastError;
            return invalidFilter;
        }
    }

    return RestFilter::rule(tablename, field, notIn ? Op::Nin : Op::In, values);
}

RestFilter& RestFilter::add(const RestFilter &otherFilter)
{
    if (!m_isGroup) {
        m_isValid = false;
        m_lastError = "Критическая ошибка сборки: Нельзя добавлять подфильтры к конечному правилу. Используйте группу (AND/OR).";
        qWarning() << "RestFilter Build Error:" << m_lastError;
        return *this;
    }

    if (!otherFilter.isValid()) {
        m_isValid = false;
        m_lastError = QString("Группа содержит невалидный подфильтр: ") + otherFilter.lastError();
    }

    m_subFilters.append(otherFilter);
    return *this;
}

RestFilter& RestFilter::addRule(const QString &tablename, const QString &field, Op op, const QJsonValue &value)
{
    return add(RestFilter::rule(tablename, field, op, value));
}

QJsonObject RestFilter::toJsonObject() const
{
    QJsonObject json;

    if (!m_isValid) {
        json["__client_error__"] = m_lastError;
    }

    if (m_isGroup) {
        json["group"] = groupToString(m_groupType);
        QJsonArray rulesArray;
        for (const auto &subFilter : m_subFilters) {
            rulesArray.append(subFilter.toJsonObject());
        }
        json["rules"] = rulesArray;
    } else {
        json["tablename"] = m_tablename;
        json["field"] = m_field;
        json["op"] = opToString(m_op);
        if (m_op != Op::Null && m_op != Op::Nnull) {
            json["value"] = m_value;
        }
    }

    return json;
}

QString RestFilter::toJsonString(QJsonDocument::JsonFormat format) const
{
    QJsonDocument doc(toJsonObject());
    return doc.toJson(format);
}

QString RestFilter::toJsonStringIfValid(QJsonDocument::JsonFormat format) const
{
    if (!m_isValid) {
        return QString();
    }
    return toJsonString(format);
}

QString RestFilter::groupToString(Group group) { return (group == Group::And) ? "and" : "or"; }

QString RestFilter::opToString(Op op)
{
    switch (op) {
    case Op::Eq:      return "eq";      case Op::Ne:      return "ne";
    case Op::Gt:      return "gt";      case Op::Gte:     return "gte";
    case Op::Lt:      return "lt";      case Op::Lte:     return "lte";
    case Op::Like:    return "like";    case Op::Ilike:   return "ilike";
    case Op::Between: return "between"; case Op::In:      return "in";
    case Op::Nin:     return "nin";
    case Op::Null:    return "null";
    case Op::Nnull:   return "nnull";
    }
    return "eq";
}
