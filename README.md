# MIDI-GenGX V1 — Phase 112 — Embedded Conditioned Model Runtime Loader

We have now clarified the product boundary:

```text
CORPUS / MIDI / metadata
        ↓
     development
        ↓
      training
        ↓
  learned model artifact
        ↓
       plugin
        ↓
      user
```

The user must not download or prepare composer MIDI corpora.

Phase 112 creates the runtime-side loader that accepts an embedded model
artifact byte buffer and reconstructs the conditioned neural model in memory.

It supports:

```text
static embedded bytes
memory/resource bytes
validated model artifact
actual vocabulary identities
```

Invalid or null model resources fail closed.

This is the bridge between the model produced during development and the
future VST3 packaging layer.

No real Chopin corpus is shipped by this phase and no model is generated in
this phase.

## Next milestone

The next meaningful step is to define the final plugin resource packaging
contract so a trained `.mgcn` artifact can be embedded into the VST3 build
without requiring the user to download any corpus or model.
