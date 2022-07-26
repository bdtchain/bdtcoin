// Copyright (c) 2011-2014 The Bdtcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BDTCOIN_QT_BDTCOINADDRESSVALIDATOR_H
#define BDTCOIN_QT_BDTCOINADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class BdtcoinAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BdtcoinAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Bdtcoin address widget validator, checks for a valid bdtcoin address.
 */
class BdtcoinAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BdtcoinAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // BDTCOIN_QT_BDTCOINADDRESSVALIDATOR_H
