# Miner Authorization via Signet-Style Block Signatures

This document specifies how to restrict block production to an allowlist of miners using a "signet-style" authorization challenge that every block must satisfy.

It covers:
- The design and effect of signet-style authorization
- Activation at/after a target height so existing blocks remain valid
- How to configure the authorization challenge for up to 300 keys (100 Taproot miners + 200 legacy/segwit miners)
- Required code changes with example snippets
- Miner-side integration to produce valid block solutions
- Testing and rollout plan

---

## Goals
- Only authorized miners can produce valid blocks after a chosen activation height.
- Do not invalidate the existing chain (e.g., current height ~70k). The rule should activate at a future height.
- Support a large allowlist (e.g., 300 miners). Use Taproot (BIP341) for scalability.

---

## Overview of Signet-Style Authorization
- The node stores a fixed script (the "challenge") in consensus parameters.
- Every block must carry a valid "solution" to the challenge embedded in the coinbase witness commitment output.
- Validation calls `CheckSignetBlockSolution(block, consensus)` to verify the solution — if it fails, the block is invalid.
- Effect: Only miners with a private key that satisfies the challenge can mine valid blocks.

In this codebase, the verifier is already present:
- `src/signet.cpp` implements `CheckSignetBlockSolution`.
- `src/consensus/params.h` contains `signet_blocks` and `signet_challenge` fields.
- `Consensus::CheckBlock()` already enforces signet when `consensus.signet_blocks == true`.

We will add height-gated enforcement for mainnet so the rule activates at a future block height without changing historical validation.

---

## High-Level Design
1. Add a new consensus parameter `auth_activation_height` to gate authorization by height.
2. Configure a single Taproot challenge (P2TR scriptPubKey) that authorizes 300 miners.
   - 100 Taproot miners: provide 100 x-only pubkeys.
   - 200 legacy/segwit miners: provide their pubkeys; we derive x-only pubkeys for Taproot use.
   - The challenge is a Taproot output key that commits to a Tapscript tree with 300 leaves, each leaf script is `<xonly_pubkey> OP_CHECKSIG`.
3. Enforce `CheckSignetBlockSolution()` when connecting a block with `height >= auth_activation_height`.
4. Optionally keep or retire existing `CheckProofOfProtocol` at the same height.

---

## Activation Strategy
- Choose `auth_activation_height = H` strictly greater than current tip (e.g., 75,000 or 80,000).
- Blocks below `H` are validated as today.
- Blocks at or above `H` must carry a valid signet solution.

---

## Code Changes

### 1) Add a new consensus parameter
File: `src/consensus/params.h`

```cpp
// Add near other consensus params
int auth_activation_height{0};
```

### 2) Configure Mainnet parameters
File: `src/kernel/chainparams.cpp`, inside `class CMainParams` constructor

```cpp
// Set the activation height (example)
consensus.auth_activation_height = 80000; // Activate at height 80,000

// Set the signet challenge scriptPubKey bytes for the Taproot allowlist (see below)
consensus.signet_challenge.clear();
// consensus.signet_challenge = ParseHex("51"); // Example: OP_TRUE (testing only)
// For production, set to the Taproot P2TR scriptPubKey bytes: OP_1 <32-byte xonly output key>

// Keep global signet off on mainnet; we will enforce by height instead
consensus.signet_blocks = false;
```

Do not enable `consensus.signet_blocks` globally on mainnet; the legacy paths call the signet checker without height context. We enforce at height in the connect path instead.

### 3) Height-gated enforcement in block connect path
File: `src/validation.cpp`, in the function that connects a candidate block to the active chain (look near existing `CheckProofOfProtocol` call before `ConnectBlock`). Add:

```cpp
// Pseudocode: enforce signet-style authorization at/after activation height
const auto& consensus = GetConsensus();
if (pindexNew->nHeight >= consensus.auth_activation_height) {
    if (!CheckSignetBlockSolution(*pthisBlock, consensus)) {
        return FatalError(m_chainman.GetNotifications(), state,
                          _("Signet authorization failed (bad block signature)"));
    }
}
```

Rationale:
- This location ensures we know `pindexNew->nHeight` before enforcing.
- It does not affect historic blocks.
- The global signet check inside `Consensus::CheckBlock()` remains off for mainnet (since `signet_blocks=false`).

### 4) Optional: Gate or retire `CheckProofOfProtocol`
If you keep `CheckProofOfProtocol`, consider enforcing it only before or after the same activation height, and harden it:
- Remove backdoor txid bypass.
- Compare `scriptPubKey` bytes (not address strings) against an allowlist.
- Replace fixed per-output min-amount checks with a coinbase total value check.

---

## Building the Taproot Challenge (300 keys)

We recommend a single Taproot output (P2TR) that commits to a Tapscript tree with 300 leaves:
- Each leaf script: `<xonly_pubkey> OP_CHECKSIG`.
- Any authorized miner can reveal their leaf and provide a single BIP340 signature + control block.
- The final challenge is the scriptPubKey: `OP_1 <xonly_output_key>` (34 bytes).

Inputs required from you:
- 300 public keys. Preferred: x-only 32-byte pubkeys for each miner.
- If a miner only has a compressed ECDSA pubkey (33 bytes), derive the x-only pubkey by dropping the parity byte.

Algorithm summary:
1. Build 300 leaf scripts: `script_i = <xonly_pubkey_i> OP_CHECKSIG`.
2. Compute each leaf hash: `H_tapleaf = sha256(tagged("TapLeaf"), version || varint(len(script_i)) || script_i)`.
3. Recursively pair and hash leaf/internal nodes using tagged hash `TapBranch`, with lexicographic ordering of the two children at each level.
4. Choose an internal (hidden) key (often the 32-byte all-zero key is used with tweaking, or a designated key you control).
5. Compute tap tweak: `tweak = sha256(tagged("TapTweak"), internal_xonly || merkle_root)`. Then `output_key = internal_key + tweak*G`.
6. The signet challenge scriptPubKey is: `OP_1 <xonly(output_key)>`.

The resulting bytes (hex) go into `consensus.signet_challenge`.

> Note: Implementing the Taptree builder can be done offline (e.g., Python with `secp256k1` + a BIP341 helper) and committed as data. Only the final scriptPubKey bytes are needed by the node.

### Example (testing-only) challenge
For initial wiring tests, you can temporarily set the challenge to `OP_TRUE`:

```cpp
consensus.signet_challenge = std::vector<uint8_t>{OP_TRUE};
```

This will accept any block (no authorization); use only to verify plumbing, then replace with your P2TR challenge.

---

## Miner-Side: Producing a Valid Solution

The validation code (`CheckSignetBlockSolution`) expects the signet solution to be placed in the coinbase transaction’s witness commitment output. Internally it:
- Locates the standard witness commitment (OP_RETURN with header 0xaa21a9ed…)
- Extracts an additional section with header `0xec c7 da a2`
- Parses a serialized `scriptSig` and `scriptWitness.stack`
- Reconstructs a spend of a synthetic UTXO paying to `consensus.signet_challenge`
- Verifies the signature(s) with `VerifyScript`

Miner steps:
1. Construct the candidate block (as usual).
2. Call `SignetTxs::Create(block, challenge)` to obtain the synthetic `to_spend` and `to_sign` transactions.
3. Compute the appropriate sighash and produce a Schnorr signature (BIP340) using your authorized private key for the leaf you own. For Taproot, the witness stack typically includes: `<sig>` `<script>` `<controlblock>`.
4. Serialize the `scriptSig` (usually empty for segwit) and the witness stack into a byte vector (same encoding that `CTransaction` uses).
5. Insert those bytes into the coinbase’s witness commitment output under the `0xec c7 da a2` section.

If you use a custom miner, add a hook after assembling the coinbase to inject the signet solution bytes as described. For pool software, a small helper tool can be invoked to produce the solution given the current candidate block header and merkle root.

---

## Testing
- Unit tests: adapt `src/test/validation_tests.cpp::signet_parse_tests` with your challenge.
- Functional tests: sync a pair of nodes across the activation height and verify that blocks without a valid solution are rejected.
- Temporary challenge `OP_TRUE` can be used to verify enforcement plumbing before switching to the real P2TR challenge.

---

## Rollout Plan
1. Choose `auth_activation_height = H` (e.g., 80,000).
2. Distribute the 300 authorized keys and generate the Taproot challenge.
3. Commit `consensus.auth_activation_height` and `consensus.signet_challenge` (P2TR scriptPubKey bytes) to mainnet params.
4. Update miner software to emit valid solutions.
5. Tag a release; notify miners; monitor upgrade readiness and proceed to height H.

---

## Appendix A: Example Code Snippets

Add consensus parameter (src/consensus/params.h):
```cpp
struct Params {
    // ... existing fields ...
    int auth_activation_height{0};
};
```

Mainnet params (src/kernel/chainparams.cpp):
```cpp
CMainParams::CMainParams() {
    // ... existing params ...
    consensus.auth_activation_height = 80000; // example

    // Replace with your real P2TR challenge bytes
    // consensus.signet_challenge = ParseHex("5120<64 hex of xonly output key>");

    consensus.signet_blocks = false;
}
```

Enforcement (src/validation.cpp), near block connect:
```cpp
const auto& consensus = GetConsensus();
if (pindexNew->nHeight >= consensus.auth_activation_height) {
    if (!CheckSignetBlockSolution(*pthisBlock, consensus)) {
        return FatalError(m_chainman.GetNotifications(), state,
                          _("Signet authorization failed (bad block signature)"));
    }
}
```

---

## Appendix B: Keeping or Retiring CheckProofOfProtocol
If you retain `CheckProofOfProtocol`:
- Gate it with the same activation height (either only-before or only-after H).
- Remove hardcoded bypass txid.
- Compare `scriptPubKey` bytes with an allowlist, not address strings.
- Validate the whole coinbase payout rather than per-output thresholds.

If you migrate entirely to signet-style authorization, `CheckProofOfProtocol` can be removed after activation.

---

## FAQ
- Why Taproot? It scales to hundreds of authorized keys via a Merkle tree and uses compact Schnorr signatures.
- Can we mix ECDSA-only miners? Yes, by deriving x-only pubkeys and having them sign using BIP340. If that’s not possible, a dual-challenge design (Taproot + P2WSH) is feasible but more complex.
- Does this affect past blocks? No. Enforcement starts at `auth_activation_height`.
