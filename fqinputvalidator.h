#ifndef FQINPUTVALIDATOR_H
#define FQINPUTVALIDATOR_H

#include <QValidator>

class FqInputValidator : public QValidator
{
public:
    FqInputValidator() {}
    virtual State validate(QString &str, int &pos) const {
        str.remove(' ');
/*
        bool ok=true;
        qint64 val = str.toULongLong(&ok);
        if (!ok)
            return QValidator::Invalid;
*/
        QString tmp;
        int len = str.length();
        for (int idx=0; idx<len; idx++) {
            if (idx != 0 && (idx % 3) == 0)
                tmp += ' ';
            tmp += str[len - idx - 1];
        }
        str = tmp;
        pos = (pos > 0) ? (pos-1) : 0;
        return QValidator::Acceptable;
    }
protected:

};


#endif // FQINPUTVALIDATOR_H
