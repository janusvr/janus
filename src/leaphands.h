#ifndef LEAPHANDS_H
#define LEAPHANDS_H

#include <QtGui>
#include <cfloat>

#include "mathutil.h"

class LeapHand
{
public:

    LeapHand() :
        is_active(false)
    {
    }

    void SetJSON(const QMap<QString, QVariant> & map);
    QString GetJSON() const;

    static LeapHand Interpolate(const LeapHand & h1, const LeapHand & h2, const float t);

    bool is_active;        
    QMatrix4x4 basis;        
};

#endif // LEAPHANDS_H
