# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libbdtcoin_cli*         | RPC client functionality used by *bdtcoin-cli* executable |
| *libbdtcoin_common*      | Home for common functionality shared by different executables and libraries. Similar to *libbdtcoin_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libbdtcoin_consensus*   | Consensus functionality used by *libbdtcoin_node* and *libbdtcoin_wallet*. |
| *libbdtcoin_crypto*      | Hardware-optimized functions for data encryption, hashing, message authentication, and key derivation. |
| *libbdtcoin_kernel*      | Consensus engine and support library used for validation by *libbdtcoin_node*. |
| *libbdtcoinqt*           | GUI functionality used by *bdtcoin-qt* and *bdtcoin-gui* executables. |
| *libbdtcoin_ipc*         | IPC functionality used by *bdtcoin-node*, *bdtcoin-wallet*, *bdtcoin-gui* executables to communicate when [`-DWITH_MULTIPROCESS=ON`](multiprocess.md) is used. |
| *libbdtcoin_node*        | P2P and RPC server functionality used by *bdtcoind* and *bdtcoin-qt* executables. |
| *libbdtcoin_util*        | Home for common functionality shared by different executables and libraries. Similar to *libbdtcoin_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libbdtcoin_wallet*      | Wallet functionality used by *bdtcoind* and *bdtcoin-wallet* executables. |
| *libbdtcoin_wallet_tool* | Lower-level wallet functionality used by *bdtcoin-wallet* executable. |
| *libbdtcoin_zmq*         | [ZeroMQ](../zmq.md) functionality used by *bdtcoind* and *bdtcoin-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. An exception is *libbdtcoin_kernel*, which, at some future point, will have a documented external interface.

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`add_library(bdtcoin_* ...)`](../../src/CMakeLists.txt) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libbdtcoin_node* code lives in `src/node/` in the `node::` namespace
  - *libbdtcoin_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libbdtcoin_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libbdtcoin_util* code lives in `src/util/` in the `util::` namespace
  - *libbdtcoin_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

bdtcoin-cli[bdtcoin-cli]-->libbdtcoin_cli;

bdtcoind[bdtcoind]-->libbdtcoin_node;
bdtcoind[bdtcoind]-->libbdtcoin_wallet;

bdtcoin-qt[bdtcoin-qt]-->libbdtcoin_node;
bdtcoin-qt[bdtcoin-qt]-->libbdtcoinqt;
bdtcoin-qt[bdtcoin-qt]-->libbdtcoin_wallet;

bdtcoin-wallet[bdtcoin-wallet]-->libbdtcoin_wallet;
bdtcoin-wallet[bdtcoin-wallet]-->libbdtcoin_wallet_tool;

libbdtcoin_cli-->libbdtcoin_util;
libbdtcoin_cli-->libbdtcoin_common;

libbdtcoin_consensus-->libbdtcoin_crypto;

libbdtcoin_common-->libbdtcoin_consensus;
libbdtcoin_common-->libbdtcoin_crypto;
libbdtcoin_common-->libbdtcoin_util;

libbdtcoin_kernel-->libbdtcoin_consensus;
libbdtcoin_kernel-->libbdtcoin_crypto;
libbdtcoin_kernel-->libbdtcoin_util;

libbdtcoin_node-->libbdtcoin_consensus;
libbdtcoin_node-->libbdtcoin_crypto;
libbdtcoin_node-->libbdtcoin_kernel;
libbdtcoin_node-->libbdtcoin_common;
libbdtcoin_node-->libbdtcoin_util;

libbdtcoinqt-->libbdtcoin_common;
libbdtcoinqt-->libbdtcoin_util;

libbdtcoin_util-->libbdtcoin_crypto;

libbdtcoin_wallet-->libbdtcoin_common;
libbdtcoin_wallet-->libbdtcoin_crypto;
libbdtcoin_wallet-->libbdtcoin_util;

libbdtcoin_wallet_tool-->libbdtcoin_wallet;
libbdtcoin_wallet_tool-->libbdtcoin_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class bdtcoin-qt,bdtcoind,bdtcoin-cli,bdtcoin-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Crypto* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus, crypto, and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libbdtcoin_wallet* and *libbdtcoin_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libbdtcoin_crypto* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libbdtcoin_consensus* should only depend on *libbdtcoin_crypto*, and all other libraries besides *libbdtcoin_crypto* should be allowed to depend on it.

- *libbdtcoin_util* should be a standalone dependency that any library can depend on, and it should not depend on other libraries except *libbdtcoin_crypto*. It provides basic utilities that fill in gaps in the C++ standard library and provide lightweight abstractions over platform-specific features. Since the util library is distributed with the kernel and is usable by kernel applications, it shouldn't contain functions that external code shouldn't call, like higher level code targeted at the node or wallet. (*libbdtcoin_common* is a better place for higher level code, or code that is meant to be used by internal applications only.)

- *libbdtcoin_common* is a home for miscellaneous shared code used by different Bdtcoin Core applications. It should not depend on anything other than *libbdtcoin_util*, *libbdtcoin_consensus*, and *libbdtcoin_crypto*.

- *libbdtcoin_kernel* should only depend on *libbdtcoin_util*, *libbdtcoin_consensus*, and *libbdtcoin_crypto*.

- The only thing that should depend on *libbdtcoin_kernel* internally should be *libbdtcoin_node*. GUI and wallet libraries *libbdtcoinqt* and *libbdtcoin_wallet* in particular should not depend on *libbdtcoin_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be able to get it from *libbdtcoin_consensus*, *libbdtcoin_common*, *libbdtcoin_crypto*, and *libbdtcoin_util*, instead of *libbdtcoin_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libbdtcoinqt*, *libbdtcoin_node*, *libbdtcoin_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](../../src/interfaces/) abstract interfaces.

## Work in progress

- Validation code is moving from *libbdtcoin_node* to *libbdtcoin_kernel* as part of [The libbdtcoinkernel Project #27587](https://github.com/bdtchain/bdtcoin/issues/27587)
