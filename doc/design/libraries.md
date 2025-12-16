# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libhylium_cli*         | RPC client functionality used by *hylium-cli* executable |
| *libhylium_common*      | Home for common functionality shared by different executables and libraries. Similar to *libhylium_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libhylium_consensus*   | Consensus functionality used by *libhylium_node* and *libhylium_wallet*. |
| *libhylium_crypto*      | Hardware-optimized functions for data encryption, hashing, message authentication, and key derivation. |
| *libhylium_kernel*      | Consensus engine and support library used for validation by *libhylium_node*. |
| *libhyliumqt*           | GUI functionality used by *hylium-qt* and *hylium-gui* executables. |
| *libhylium_ipc*         | IPC functionality used by *hylium-node* and *hylium-gui* executables to communicate when [`-DENABLE_IPC=ON`](multiprocess.md) is used. |
| *libhylium_node*        | P2P and RPC server functionality used by *hyliumd* and *hylium-qt* executables. |
| *libhylium_util*        | Home for common functionality shared by different executables and libraries. Similar to *libhylium_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libhylium_wallet*      | Wallet functionality used by *hyliumd* and *hylium-wallet* executables. |
| *libhylium_wallet_tool* | Lower-level wallet functionality used by *hylium-wallet* executable. |
| *libhylium_zmq*         | [ZeroMQ](../zmq.md) functionality used by *hyliumd* and *hylium-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. An exception is *libhylium_kernel*, which, at some future point, will have a documented external interface.

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`add_library(hylium_* ...)`](../../src/CMakeLists.txt) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libhylium_node* code lives in `src/node/` in the `node::` namespace
  - *libhylium_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libhylium_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libhylium_util* code lives in `src/util/` in the `util::` namespace
  - *libhylium_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

hylium-cli[hylium-cli]-->libhylium_cli;

hyliumd[hyliumd]-->libhylium_node;
hyliumd[hyliumd]-->libhylium_wallet;

hylium-qt[hylium-qt]-->libhylium_node;
hylium-qt[hylium-qt]-->libhyliumqt;
hylium-qt[hylium-qt]-->libhylium_wallet;

hylium-wallet[hylium-wallet]-->libhylium_wallet;
hylium-wallet[hylium-wallet]-->libhylium_wallet_tool;

libhylium_cli-->libhylium_util;
libhylium_cli-->libhylium_common;

libhylium_consensus-->libhylium_crypto;

libhylium_common-->libhylium_consensus;
libhylium_common-->libhylium_crypto;
libhylium_common-->libhylium_util;

libhylium_kernel-->libhylium_consensus;
libhylium_kernel-->libhylium_crypto;
libhylium_kernel-->libhylium_util;

libhylium_node-->libhylium_consensus;
libhylium_node-->libhylium_crypto;
libhylium_node-->libhylium_kernel;
libhylium_node-->libhylium_common;
libhylium_node-->libhylium_util;

libhyliumqt-->libhylium_common;
libhyliumqt-->libhylium_util;

libhylium_util-->libhylium_crypto;

libhylium_wallet-->libhylium_common;
libhylium_wallet-->libhylium_crypto;
libhylium_wallet-->libhylium_util;

libhylium_wallet_tool-->libhylium_wallet;
libhylium_wallet_tool-->libhylium_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class hylium-qt,hyliumd,hylium-cli,hylium-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Crypto* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus, crypto, and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libhylium_wallet* and *libhylium_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libhylium_crypto* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libhylium_consensus* should only depend on *libhylium_crypto*, and all other libraries besides *libhylium_crypto* should be allowed to depend on it.

- *libhylium_util* should be a standalone dependency that any library can depend on, and it should not depend on other libraries except *libhylium_crypto*. It provides basic utilities that fill in gaps in the C++ standard library and provide lightweight abstractions over platform-specific features. Since the util library is distributed with the kernel and is usable by kernel applications, it shouldn't contain functions that external code shouldn't call, like higher level code targeted at the node or wallet. (*libhylium_common* is a better place for higher level code, or code that is meant to be used by internal applications only.)

- *libhylium_common* is a home for miscellaneous shared code used by different Hylium Core applications. It should not depend on anything other than *libhylium_util*, *libhylium_consensus*, and *libhylium_crypto*.

- *libhylium_kernel* should only depend on *libhylium_util*, *libhylium_consensus*, and *libhylium_crypto*.

- The only thing that should depend on *libhylium_kernel* internally should be *libhylium_node*. GUI and wallet libraries *libhyliumqt* and *libhylium_wallet* in particular should not depend on *libhylium_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be able to get it from *libhylium_consensus*, *libhylium_common*, *libhylium_crypto*, and *libhylium_util*, instead of *libhylium_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libhyliumqt*, *libhylium_node*, *libhylium_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](../../src/interfaces/) abstract interfaces.

## Work in progress

- Validation code is moving from *libhylium_node* to *libhylium_kernel* as part of [The libhyliumkernel Project #27587](https://github.com/hylium/hylium/issues/27587)
