# MIDI-GenGX V1 Changelog

## Phase 104 — Real Composer Corpus Preparation
- Connected verified real composer MIDI records to section, harmony, and motif analysis.
- Required complete musical enrichment before a sample enters composer learning.
- Added conditioned training dataset construction for prepared composer samples.
- Added dedicated CompositionRealComposerCorpusPreparationTests target.
- Extended build validation.

## Phase 104.1 — Test Fixture Correction
- Corrected the Phase 104 synthetic musical fixture so existing harmony and motif analyzers have sufficient evidence.
- No production inference, acceptance, or training logic was weakened.

## Phase 104.2 — Missing-Metadata Regression Fix
- Corrected the missing-metadata regression fixture to remove the metadata entry that matches the record under test.
- No production logic changed.

## Phase 105 — Real Composer Training Path
- Connected conditioned corpus training to the verified real-composer preparation pipeline.
- Removed the legacy direct dependency on the sparse sequence corpus builder from the training service.
- Added a regression test proving the service consumes the enriched preparation path.

## Phase 105.1 — Artifact Header Fix
- Replaced the nonexistent `CompositionMidiTrainingSequenceArtifact.h` include with the existing `CompositionMidiTrainingCorpusArtifact.h`.
- No runtime behavior changed.

## Phase 105.2 — Loader Header Fix
- Added the missing `CompositionMidiCorpusDirectoryLoader.h` include to the real-composer training service.
- No runtime behavior changed.

## Phase 105.3 — Enrichment Boundary Correction
- Made harmonic enrichment a hard requirement because the current 20D sequence representation consumes harmonic features.
- Stopped rejecting otherwise valid samples solely because motif-family analysis is unavailable to the current 20D model input.
- Preserved motif analysis computation for future motif-aware training features.

## Phase 105.4 — Exact Metadata Coverage Gate
- Added exact one-to-one MIDI sample ID ↔ verified metadata coverage validation before conditioned training.
- Prevented partial metadata catalogs from silently entering real composer training.
- No neural training algorithm changed.

## Phase 106 — Training Split Boundary
- Integrated the established composer/work-level train/validation/test partition into the conditioned corpus training service.
- Restricted neural training input to the training partition.
- Exposed training/validation/test cardinalities in the corpus training result.
- Added Phase 106 split-boundary validation.

## Phase 107 — Neural Evaluation
- Added forward-only conditioned sequence neural evaluation.
- Added post-training validation/test loss measurement.
- Deserialized the trained neural artifact for held-out evaluation.
- Preserved the training-only data boundary.
- Added dedicated evaluator tests.

## Phase 107.1 — Evaluator Fixture API Fix
- Updated the neural evaluator test fixture to the current `CompositionMidiTrainingSequence` API.
- No production evaluation or training logic changed.

## Phase 108 — Training Gradient Correctness
- Corrected output-layer gradients to match the measured MSE objective.
- Removed duplicate unscaled output-gradient accumulation.
- Preserved deterministic initialization and training behavior.
- Added explicit gradient-correctness validation.

## Phase 109 — Real Composer Training CLI
- Added external TSV metadata ingestion.
- Added a standalone real-composer training CLI.
- Added model artifact file output and training/evaluation metrics.
- Added a PowerShell wrapper for the first real corpus training run.
- Added metadata loader regression tests.

## Phase 109.1 — Metadata Fixture Encoding Fix
- Corrected the metadata loader test fixture to emit actual TSV tabs/newlines.
- No production parser logic changed.

## Phase 109.2 — Metadata Loader Finalization Fix
- Fixed metadata parsing finalization order.
- Parsed metadata is marked constructed before `isValid()` is evaluated.
- No file format or training behavior changed.

## Phase 109.3 — TSV Line-Ending and Fixture Lifetime Fix
- Made TSV verified-field parsing CRLF-safe.
- Closed metadata test fixture streams before reading them.
- Preserved the eight-field metadata schema and fail-closed behavior.

## Phase 110 — Conditioned Model Artifact Vocabulary
- Advanced conditioned model artifact format to version 2.
- Persisted actual composer/style/era/instrumentation vocabulary identities.
- Removed synthetic `category_N` reconstruction during deserialization.
- Added artifact vocabulary round-trip tests.

## Phase 110.1 — Vocabulary Container Type Fix
- Corrected the vocabulary category container in conditioned model artifact serialization.
- Added the missing `<array>` include.
- No artifact schema or runtime behavior changed.

## Phase 110.2 — Artifact Region Deserialization Fix
- Corrected conditioned model artifact deserialization order.
- Separated numeric header, parameter payload, and vocabulary string-table cursors.
- Added regression coverage for simultaneous parameter and vocabulary round-trip preservation.

## Phase 112 — Embedded Conditioned Model Runtime Loader
- Added runtime loader for embedded conditioned neural model bytes.
- Reuses the validated version-2 model artifact format.
- Preserves stored conditioning vocabulary identities.
- Added fail-closed invalid/null resource handling.
- Added dedicated runtime loader tests.

## Phase 113 — Real Model Artifact Qualification
- Added file-based qualification using the Phase 112 runtime loader.
- Added inspection of persisted conditioning vocabulary.
- Added fail-closed handling for missing model artifacts.
- Documented the supplied 67-piece Chopin experiment as an existing development artifact.

## Phase 114.1 — Composition Intent Build Integration
- Preserved the Phase 113 real-model artifact inspector sources and test target in the CMake graph.
- Added `CompositionIntentTests` using the existing project test-target conventions and `MultiThreadedDebugDLL` runtime.
- No production runtime, training, model, corpus, UI, or CPU-budget behavior changed.
