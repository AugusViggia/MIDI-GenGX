# Phase 114 — Composition Intent Schema

## Purpose

Define the stable internal representation that the future MIDI-GenGX Music Intelligence Engine will receive before composition reasoning and model generation.

## What this phase establishes

`CompositionIntent` combines:

- Existing explicit musical context from `GenerationIntent`.
- Natural-language prompt.
- Composer influences with weighted strength.
- Genre tags.
- Structural preferences.
- Sound-engineering / production constraints.
- Generation duration policy.
- Hard-constraint flags.
- Soft-source precedence.

## Priority contract

Hard constraints remain authoritative. For soft information, the default ordering is:

1. Explicit selectors.
2. Prompt interpretation.
3. Genre knowledge.
4. Composer knowledge.
5. Generation preferences.

The schema is intentionally a contract rather than a resolver. Conflict-resolution behavior will be implemented in the next phase.

## Training boundary

This phase does not train a model. It defines the representation that will later feed the training-data pipeline and runtime composition reasoning.

The first training loop remains planned for the training-dataset phase after the knowledge schemas are established.
