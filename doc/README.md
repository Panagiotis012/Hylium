Hylium Core
=============

Hylium Core is a fork of Bitcoin Core. It reuses the Bitcoin protocol
implementation under the MIT license while running a distinct Hylium network
(separate genesis block, message start, ports, and address prefixes).

Setup
---------------------
Hylium Core is the original Hylium client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Hylium transactions, which requires several hundred gigabytes or more of disk space. Depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to several days or more.

To download Hylium Core, visit [hyliumcore.org](https://hyliumcore.org/en/download/).

Running
---------------------
The following are some helpful notes on how to run Hylium Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/hylium-qt` (GUI) or
- `bin/hyliumd` (headless)
- `bin/hylium` (wrapper command)

The `hylium` command supports subcommands like `hylium gui`, `hylium node`, and `hylium rpc` exposing different functionality. Subcommands can be listed with `hylium help`.

### Windows

Unpack the files into a directory, and then run hylium-qt.exe.

### macOS

Drag Hylium Core to your applications folder, and then run Hylium Core.

### Need Help?

* See the documentation at the [Hylium Wiki](https://en.hylium.it/wiki/Main_Page)
for help and more information.
* Ask for help on [Hylium StackExchange](https://hylium.stackexchange.com).
* Ask for help on #hylium on Libera Chat. If you don't have an IRC client, you can use [web.libera.chat](https://web.libera.chat/#hylium).
* Ask for help on the [HyliumTalk](https://hyliumtalk.org/) forums, in the [Technical Support board](https://hyliumtalk.org/index.php?board=4.0).

Building
---------------------
The following are developer notes on how to build Hylium Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows-msvc.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)

Development
---------------------
The Hylium repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.hyliumcore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)
- [Internal Design Docs](design/)

### Resources
* Discuss on the [HyliumTalk](https://hyliumtalk.org/) forums, in the [Development & Technical Discussion board](https://hyliumtalk.org/index.php?board=6.0).
* Discuss project-specific development on #hylium-core-dev on Libera Chat. If you don't have an IRC client, you can use [web.libera.chat](https://web.libera.chat/#hylium-core-dev).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [hylium.conf Configuration File](hylium-conf.md)
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
