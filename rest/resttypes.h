#ifndef RESTTYPES_H
#define RESTTYPES_H
#include <QString>
#include <QVariant>

struct colVal {
    QString disp;
    QVariant val;
    bool operator==(const colVal& rh) const {
        return (this->disp==rh.disp) && (this->val==rh.val);
    }
};

struct cellData {
    QString display;
    QVariant edit;
    QColor background;
    QString tooltip;
};

struct colInfo {
    QString nam;
    QString col;
    QString snam;
    QString udt_name;
    bool is_pk = false;
    bool editable = false;
    bool checkable = false;
    int dec = 0;
    QString relnam;
    Qt::ItemFlags flags = Qt::NoItemFlags;
    colVal defaultVal;
    QVariant width;
};


#endif