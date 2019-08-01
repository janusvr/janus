#include "leaphands.h"

void LeapHand::SetJSON(const QMap<QString, QVariant> & map)
{    
    //hand
    QStringList state = map["state"].toString().trimmed().split(" ");

    is_active = true;

    //length can be 16 or 336 (336 is reverse compatibility for finger tracking packets)
    if (state.length() == 16 || state.length() == 336) {
        int cur_index = 0;
        for (int k=0; k<16; ++k) {
            basis.data()[k] = state[cur_index].toFloat();
            ++cur_index;
        }
    }    
}

QString LeapHand::GetJSON() const
{
    QString json_str;
    json_str += "{\"state\":\"";

    for (int k=0; k<16; ++k) {
        json_str += QString::number(basis.data()[k], 'f', 3) + " ";
    }

    json_str += "\"}";
    //qDebug() << "hands_json:" << json_str;
    return json_str;
}

LeapHand LeapHand::Interpolate(const LeapHand & h1, const LeapHand & h2, const float t)
{   
    LeapHand h;
    h.is_active = (h1.is_active && h2.is_active);   
    h.basis = MathUtil::InterpolateMatrices(h1.basis, h2.basis, t);
    return h;
}

