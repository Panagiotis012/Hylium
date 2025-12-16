// Copyright (c) 2011-2020 The Hylium Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HYLIUM_QT_HYLIUMADDRESSVALIDATOR_H
#define HYLIUM_QT_HYLIUMADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class HyliumAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit HyliumAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Hylium address widget validator, checks for a valid hylium address.
 */
class HyliumAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit HyliumAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // HYLIUM_QT_HYLIUMADDRESSVALIDATOR_H
