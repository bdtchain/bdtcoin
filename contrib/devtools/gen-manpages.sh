#!/usr/bin/env bash
# Copyright (c) 2016-2019 The Bdtcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C
TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
BUILDDIR=${BUILDDIR:-$TOPDIR}

BINDIR=${BINDIR:-$BUILDDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

BDTCOIND=${BDTCOIND:-$BINDIR/bdtcoind}
BDTCOINCLI=${BDTCOINCLI:-$BINDIR/bdtcoin-cli}
BDTCOINTX=${BDTCOINTX:-$BINDIR/bdtcoin-tx}
WALLET_TOOL=${WALLET_TOOL:-$BINDIR/bdtcoin-wallet}
BDTCOINQT=${BDTCOINQT:-$BINDIR/qt/bdtcoin-qt}

[ ! -x $BDTCOIND ] && echo "$BDTCOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
read -r -a BDTCVER <<< "$($BDTCOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }')"

# Create a footer file with copyright content.
# This gets autodetected fine for bdtcoind if --version-string is not set,
# but has different outcomes for bdtcoin-qt and bdtcoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$BDTCOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $BDTCOIND $BDTCOINCLI $BDTCOINTX $WALLET_TOOL $BDTCOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BDTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BDTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
