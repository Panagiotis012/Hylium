Hylium Core integration/staging tree
=====================================

Hylium Core is a renamed, forward-looking fork of Bitcoin Core. It inherits the
Bitcoin protocol implementation and hardens it for a separate network (different
message start bytes, ports, address prefixes, and seeds) so it will not
cross-talk with Bitcoin nodes. Branding is Hylium; codebase heritage is Bitcoin
Core (MIT license).

For an immediately usable, binary version of the Hylium Core software, see
https://hyliumcore.org/en/download/ (when available).

What is Hylium Core?
---------------------

Hylium Core connects to the Hylium peer-to-peer network to download and fully
validate blocks and transactions. It also includes a wallet and graphical user
interface, which can be optionally built.

Network parameters (Hylium):
- P2P/RPC ports: mainnet 9333/9332, testnet3 19335/19332, testnet4 29333/29332, regtest 39444/39443.
- Message start bytes: mainnet f3c2e1b0, testnet3 a39c876e, testnet4 b3a17f55, regtest c1d2e3f4.
- bech32 HRP: `hyl` (main), `thyl` (test), `rhyl` (regtest).
- Base58 prefixes: P2PKH 38, P2SH 53, WIF 176 (mainnet).
- Seeds: placeholders `seed1.hylium.org`, `seed2.hylium.org` (replace with live nodes before public launch).

Roadmap highlights (work-in-progress):
- Fast sync via assumeUTXO snapshots (to be produced/published for Hylium).
- Privacy-first payment flows (PayJoin/Silent Payments) and better relay policies.
- Vault-style spending policies for safer hot wallets.
- L2-friendly fee/policy tuning for channels.

Further information about Hylium Core is available in the [doc folder](/doc).

License
-------

Hylium Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/license/MIT.

Development Process
-------------------

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/hylium/hylium/tags) are created
regularly from release branches to indicate new official, stable release versions of Hylium Core.

The https://github.com/hylium-core/gui repository is used exclusively for the
development of the GUI. Its master branch is identical in all monotree
repositories. Release branches and tags do not exist, so please do not fork
that repository unless it is for development reasons.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled during the generation of the build system) with: `ctest`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python.
These tests can be run (if the [test dependencies](/test) are installed) with: `build/test/functional/test_runner.py`
(assuming `build` is your build directory).

The CI (Continuous Integration) systems make sure that every pull request is tested on Windows, Linux, and macOS.
The CI must pass on all commits before merge to avoid unrelated CI failures on new pull requests.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Hylium Core's Transifex page](https://explore.transifex.com/hylium/hylium/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
