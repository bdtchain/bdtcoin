Bdtcoin Core
=============

Setup
---------------------
Bdtcoin Core is the original Bdtcoin client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Bdtcoin transactions, which requires several hundred gigabytes or more of disk space. Depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to several days or more.

To download Bdtcoin Core, visit [bdtcoincore.org](https://bdtcoin.org/en/download/).

Running
---------------------
The following are some helpful notes on how to run Bdtcoin Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/bdtcoin-qt` (GUI) or
- `bin/bdtcoind` (headless)

### Windows

Unpack the files into a directory, and then run bdtcoin-qt.exe.

### macOS

Drag Bdtcoin Core to your applications folder, and then run Bdtcoin Core.

### Need Help?

* See the documentation at the [Bdtcoin Wiki](https://en.bdtcoin.it/wiki/Main_Page)
for help and more information.
* Ask for help on [Bdtcoin StackExchange](https://bdtcoin.stackexchange.com).
* Ask for help on #bdtcoin on Libera Chat. If you don't have an IRC client, you can use [web.libera.chat](https://web.libera.chat/#bdtcoin).
* Ask for help on the [BdtcoinTalk](https://bdtcointalk.org/) forums, in the [Technical Support board](https://bdtcointalk.org/index.php?board=4.0).

Building
---------------------
The following are developer notes on how to build Bdtcoin Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows-msvc.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)

Development
---------------------
The Bdtcoin repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.bdtcoincore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)
- [Internal Design Docs](design/)

### Resources
* Discuss on the [BdtcoinTalk](https://bdtcointalk.org/) forums, in the [Development & Technical Discussion board](https://bdtcointalk.org/index.php?board=6.0).
* Discuss project-specific development on #bdtcoin-core-dev on Libera Chat. If you don't have an IRC client, you can use [web.libera.chat](https://web.libera.chat/#bdtcoin-core-dev).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [bdtcoin.conf Configuration File](bdtcoin-conf.md)
- [CJDNS Support](cjdns.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [I2P Support](i2p.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [Managing Wallets](managing-wallets.md)
- [Multisig Tutorial](multisig-tutorial.md)
- [Offline Signing Tutorial](offline-signing-tutorial.md)
- [P2P bad ports definition and list](p2p-bad-ports.md)
- [PSBT support](psbt.md)
- [Reduce Memory](reduce-memory.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Transaction Relay Policy](policy/README.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
