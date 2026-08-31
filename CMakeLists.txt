cmake_minimum_required(VERSION 3.22)

project(MIDI_GenGX VERSION 0.3.6 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE was not found at ${CMAKE_CURRENT_SOURCE_DIR}/JUCE. "
        "Keep the existing JUCE folder in the project root."
    )
endif()

add_subdirectory(JUCE)

# -------------------- Domain --------------------

set(MIDI_GENGX_DOMAIN_SOURCES
    Source/Domain/Key.h
    Source/Domain/Scale.h
    Source/Domain/Scale.cpp
    Source/Domain/Role.h
    Source/Domain/Style.h
    Source/Domain/Character.h
    Source/Domain/MusicalParameters.h
    Source/Domain/MusicalContext.h
    Source/Domain/GenrePresets.h
    Source/Domain/GenrePresets.cpp
    Source/Domain/GenerationIntent.h
    Source/Domain/CompositionIntent.h
    Source/Domain/AbletonOctaveConvention.h
)

add_library(MIDI_GenGX_Domain STATIC
    ${MIDI_GENGX_DOMAIN_SOURCES}
)

target_include_directories(MIDI_GenGX_Domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# -------------------- Musical Engine --------------------

set(MIDI_GENGX_MUSIC_SOURCES
    Source/Music/NoteEvent.h
    Source/Music/Phrase.h
    Source/Music/PhraseAnalysis.h
    Source/Music/PhraseAnalysis.cpp
    Source/Music/PhraseFingerprint.h
    Source/Music/PhraseFingerprint.cpp
    Source/Music/MotifFingerprint.h
    Source/Music/MotifFingerprint.cpp
    Source/Music/MotifRelationship.h
    Source/Music/MotifRelationship.cpp
    Source/Music/MotifOccurrenceGraph.h
    Source/Music/MotifOccurrenceGraph.cpp
    Source/Music/MotifRecurrenceProfile.h
    Source/Music/MotifRecurrenceProfile.cpp
    Source/Music/MotifRecurrenceMetrics.h
    Source/Music/MotifRecurrenceMetrics.cpp
    Source/Music/MotifKnowledgeRecord.h
    Source/Music/MotifKnowledgeRecord.cpp
    Source/Music/MotifKnowledgeCatalog.h
    Source/Music/MotifKnowledgeCatalog.cpp
    Source/Music/CompositionKnowledgeRecord.h
    Source/Music/CompositionKnowledgeRecord.cpp
    Source/Music/CompositionKnowledgeGraph.h
    Source/Music/CompositionKnowledgeGraph.cpp
    Source/Music/CompositionTransitionProfile.h
    Source/Music/CompositionTransitionProfile.cpp
    Source/Music/CompositionKnowledgeSnapshot.h
    Source/Music/CompositionKnowledgeSnapshot.cpp
    Source/Music/CompositionKnowledgeRepresentation.h
    Source/Music/CompositionKnowledgeRepresentation.cpp
    Source/Music/CompositionDatasetSample.h
    Source/Music/CompositionDatasetSample.cpp
    Source/Music/CompositionDataset.h
    Source/Music/CompositionDataset.cpp
    Source/Music/CompositionDatasetQuality.h
    Source/Music/CompositionDatasetQuality.cpp
    Source/Music/CompositionDatasetPartition.h
    Source/Music/CompositionDatasetPartition.cpp
    Source/Music/CompositionDatasetSchema.h
    Source/Music/CompositionDatasetSchema.cpp
    Source/Music/CompositionDatasetManifest.h
    Source/Music/CompositionDatasetManifest.cpp
    Source/Music/CompositionDatasetBatch.h
    Source/Music/CompositionDatasetBatch.cpp
    Source/Music/CompositionDatasetNormalization.h
    Source/Music/CompositionDatasetNormalization.cpp
    Source/Music/CompositionDatasetPreparedView.h
    Source/Music/CompositionDatasetPreparedView.cpp
    Source/Music/CompositionLearningContract.h
    Source/Music/CompositionLearningContract.cpp
    Source/Music/CompositionModel.h
    Source/Music/CompositionModel.cpp
    Source/Music/CompositionNeuralModel.h
    Source/Music/CompositionNeuralModel.cpp
    Source/Music/CompositionNeuralTrainer.h
    Source/Music/CompositionNeuralTrainer.cpp
    Source/Music/CompositionModelEvaluation.h
    Source/Music/CompositionModelEvaluation.cpp
    Source/Music/CompositionMusicalEvaluation.h
    Source/Music/CompositionMusicalEvaluation.cpp
    Source/Music/CompositionRuntimeFeatureAdapter.h
    Source/Music/CompositionRuntimeFeatureAdapter.cpp
    Source/Music/CompositionRuntimeInferenceAdapter.h
    Source/Music/CompositionRuntimeInferenceAdapter.cpp
    Source/Music/CompositionRuntimeInferenceService.h
    Source/Music/CompositionRuntimeInferenceService.cpp
    Source/Music/CompositionNeuralModelArtifact.h
    Source/Music/CompositionNeuralModelArtifact.cpp
    Source/Music/CompositionNeuralModelRuntimeLoader.h
    Source/Music/CompositionNeuralModelRuntimeLoader.cpp
    Source/Music/CompositionNeuralArtifactTrainingService.h
    Source/Music/CompositionNeuralArtifactTrainingService.cpp
    Source/Music/CompositionTrainingCorpusArtifact.h
    Source/Music/CompositionTrainingCorpusArtifact.cpp
    Source/Music/CompositionMidiCorpusRecord.h
    Source/Music/CompositionMidiCorpusRecord.cpp
    Source/Music/CompositionMidiFileCorpusReader.h
    Source/Music/CompositionMidiFileCorpusReader.cpp
    Source/Music/CompositionMidiCorpusAnalysis.h
    Source/Music/CompositionMidiCorpusAnalysis.cpp
    Source/Music/CompositionMidiSection.h
    Source/Music/CompositionMidiSectionAnalyzer.h
    Source/Music/CompositionMidiSectionAnalyzer.cpp
    Source/Music/CompositionMidiDatasetFeatureExtractor.h
    Source/Music/CompositionMidiDatasetFeatureExtractor.cpp
    Source/Music/CompositionMidiHarmony.h
    Source/Music/CompositionMidiHarmony.cpp
    Source/Music/CompositionMidiMotifAnalysis.h
    Source/Music/CompositionMidiMotifAnalysis.cpp
    Source/Music/CompositionMidiDatasetBuilder.h
    Source/Music/CompositionMidiDatasetBuilder.cpp
    Source/Music/CompositionMidiCorpusDirectoryLoader.h
    Source/Music/CompositionMidiCorpusDirectoryLoader.cpp
    Source/Music/CompositionMidiTrainingPipeline.h
    Source/Music/CompositionMidiTrainingPipeline.cpp
    Source/Music/CompositionMidiCorpusTrainingService.h
    Source/Music/CompositionMidiCorpusTrainingService.cpp
    Source/Music/CompositionMidiTrainingEvaluationService.h
    Source/Music/CompositionMidiTrainingEvaluationService.cpp
    Source/Music/CompositionNeuralGenerationService.h
    Source/Music/CompositionNeuralGenerationService.cpp
    Source/Music/CompositionAIMidiGenerationService.h
    Source/Music/CompositionAIMidiGenerationService.cpp
    Source/Music/CompositionMidiTrainingSequence.h
    Source/Music/CompositionMidiTrainingSequence.cpp
    Source/Music/CompositionMidiTrainingCorpusArtifact.h
    Source/Music/CompositionMidiTrainingCorpusArtifact.cpp
    Source/Music/CompositionSequenceLearningContract.h
    Source/Music/CompositionSequenceLearningContract.cpp
    Source/Music/CompositionMidiSequenceWindow.h
    Source/Music/CompositionMidiSequenceWindow.cpp
    Source/Music/CompositionSequenceNeuralModel.h
    Source/Music/CompositionSequenceNeuralModel.cpp
    Source/Music/CompositionSequenceNeuralTrainer.h
    Source/Music/CompositionSequenceNeuralTrainer.cpp
    Source/Music/CompositionSequenceNeuralModelArtifact.h
    Source/Music/CompositionSequenceNeuralModelArtifact.cpp
    Source/Music/CompositionSequenceNeuralTrainingService.h
    Source/Music/CompositionSequenceNeuralTrainingService.cpp
    Source/Music/CompositionMidiSequenceCorpusBuilder.h
    Source/Music/CompositionMidiSequenceCorpusBuilder.cpp
    Source/Music/CompositionSequenceMetadata.h
    Source/Music/CompositionSequenceMetadata.cpp
    Source/Music/CompositionSequenceMetadataCatalog.h
    Source/Music/CompositionSequenceMetadataCatalog.cpp
    Source/Music/CompositionSequenceMetadataArtifact.h
    Source/Music/CompositionSequenceMetadataArtifact.cpp
    Source/Music/CompositionConditionedTrainingSample.h
    Source/Music/CompositionConditionedTrainingSample.cpp
    Source/Music/CompositionConditioningVocabulary.h
    Source/Music/CompositionConditioningVocabulary.cpp
    Source/Music/CompositionConditionedTrainingDataset.h
    Source/Music/CompositionConditionedTrainingDataset.cpp
    Source/Music/CompositionConditionedTrainingDatasetArtifact.h
    Source/Music/CompositionConditionedTrainingDatasetArtifact.cpp
    Source/Music/CompositionConditionedSequenceNeuralModel.h
    Source/Music/CompositionConditionedSequenceNeuralModel.cpp
    Source/Music/CompositionConditionedMidiDecoder.cpp
    Source/Music/CompositionConditionedSequenceNeuralTrainer.h
    Source/Music/CompositionConditionedSequenceNeuralTrainer.cpp
    Source/Music/CompositionConditionedSequenceNeuralModelArtifact.h
    Source/Music/CompositionConditionedSequenceNeuralModelArtifact.cpp
    Source/Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h
    Source/Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.cpp
    Source/Music/CompositionConditionedSequenceNeuralModelArtifactFileInspector.h
    Source/Music/CompositionConditionedSequenceNeuralModelArtifactFileInspector.cpp
    Source/Music/CompositionConditionedSequenceNeuralTrainingService.h
    Source/Music/CompositionConditionedSequenceNeuralTrainingService.cpp
    Source/Music/CompositionConditionedTrainingRunService.h
    Source/Music/CompositionConditionedTrainingRunService.cpp
    Source/Music/CompositionComposerKnowledgeRepresentation.h
    Source/Music/CompositionComposerKnowledgeRepresentation.cpp
    Source/Music/CompositionGenreKnowledgeRepresentation.h
    Source/Music/CompositionGenreKnowledgeRepresentation.cpp
    Source/Music/CompositionSoundEngineeringKnowledgeRepresentation.h
    Source/Music/CompositionSoundEngineeringKnowledgeRepresentation.cpp
    Source/Music/CompositionKnowledgeTrainingDataset.h
    Source/Music/CompositionKnowledgeTrainingDataset.cpp
    Source/Music/CompositionKnowledgeTrainingDatasetArtifact.h
    Source/Music/CompositionKnowledgeTrainingDatasetArtifact.cpp
    Source/Music/CompositionComposerKnowledgeSample.h
    Source/Music/CompositionComposerKnowledgeSample.cpp
    Source/Music/CompositionComposerKnowledgeCatalog.h
    Source/Music/CompositionComposerKnowledgeCatalog.cpp
    Source/Music/CompositionComposerKnowledgePartition.h
    Source/Music/CompositionComposerKnowledgePartition.cpp
    Source/Music/CompositionComposerKnowledgeCorpusManifest.h
    Source/Music/CompositionComposerKnowledgeCorpusManifest.cpp
    Source/Music/CompositionComposerKnowledgeTrainingCorpus.h
    Source/Music/CompositionComposerKnowledgeTrainingCorpus.cpp
    Source/Music/CompositionComposerCorpusSourceManifest.h
    Source/Music/CompositionComposerCorpusSourceManifest.cpp
    Source/Music/CompositionComposerKnowledgeCorpusAssembly.h
    Source/Music/CompositionComposerKnowledgeCorpusAssembly.cpp
    Source/Music/CompositionRealComposerCorpusIntake.h
    Source/Music/CompositionRealComposerCorpusIntake.cpp
    Source/Music/CompositionRealComposerCorpusPreparation.h
    Source/Music/CompositionRealComposerCorpusPreparation.cpp
    Source/Music/CompositionConditionedCorpusTrainingService.h
    Source/Music/CompositionConditionedCorpusTrainingService.cpp
    Source/Music/CompositionSequenceMetadataFileLoader.h
    Source/Music/CompositionSequenceMetadataFileLoader.cpp
    Source/Music/CompositionRealComposerTrainingCli.cpp
    Source/Music/CompositionConditionedSequenceNeuralEvaluator.h
    Source/Music/CompositionConditionedSequenceNeuralEvaluator.cpp
    Source/Music/CompositionAIModelRuntimeProvider.h
    Source/Music/CompositionAIModelRuntimeProvider.cpp
    Source/Music/CompositionInferencePipeline.h
    Source/Music/CompositionInferencePipeline.cpp
    Source/Music/CompositionAIConstraintAdapter.h
    Source/Music/CompositionAIConstraintAdapter.cpp
    Source/Music/CompositionAIEngineBridge.h
    Source/Music/CompositionAIEngineBridge.cpp
    Source/Music/CompositionAIGenerationCoordinator.h
    Source/Music/CompositionAIGenerationCoordinator.cpp
    Source/Music/Motif.h
    Source/Music/Motif.cpp
    Source/Music/MotifDevelopment.h
    Source/Music/MotifDevelopment.cpp
    Source/Music/MotifPhraseComposer.h
    Source/Music/MotifPhraseComposer.cpp
    Source/Music/PhraseStructure.h
    Source/Music/PhraseStructure.cpp
    Source/Music/PhraseTensionArc.h
    Source/Music/PhraseTensionArc.cpp
    Source/Music/PhraseDevelopmentPlan.h
    Source/Music/PhraseDevelopmentPlan.cpp
    Source/Music/HarmonyPlan.h
    Source/Music/HarmonyPlan.cpp
    Source/Music/HarmonyGuidance.h
    Source/Music/HarmonyGuidance.cpp
    Source/Music/RhythmPlan.h
    Source/Music/RhythmPlan.cpp
    Source/Music/MelodicContour.h
    Source/Music/MelodicContour.cpp
    Source/Music/MelodicMotion.h
    Source/Music/MelodicMotion.cpp
    Source/Music/MelodicMotionGuidance.h
    Source/Music/MelodicMotionGuidance.cpp
    Source/Music/MelodicResolution.h
    Source/Music/MelodicResolution.cpp
    Source/Music/MelodicTendency.h
    Source/Music/MelodicTendency.cpp
    Source/Music/MusicalEngine.h
    Source/Music/MusicalEngine.cpp
)

add_library(MIDI_GenGX_Music STATIC
    ${MIDI_GENGX_MUSIC_SOURCES}
)

target_include_directories(MIDI_GenGX_Music
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(MIDI_GenGX_Music
    PUBLIC
        MIDI_GenGX_Domain
)

# -------------------- Plugin --------------------

juce_add_plugin(MIDI_GenGX
    COMPANY_NAME "MIDI-GenGX"
    PRODUCT_NAME "MIDI-GenGX"

    PLUGIN_MANUFACTURER_CODE MGGX
    PLUGIN_CODE MGG1

    FORMATS VST3

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE

    COPY_PLUGIN_AFTER_BUILD FALSE

    VST3_CATEGORIES "Instrument"
)

juce_generate_juce_header(MIDI_GenGX)

target_sources(MIDI_GenGX
    PRIVATE
        Source/Plugin/PluginProcessor.cpp
        Source/Plugin/PluginProcessor.h
        Source/Plugin/PluginEditor.cpp
        Source/Plugin/PluginEditor.h
        Source/Plugin/AIRuntimeGeneration.cpp
        Source/Plugin/AIRuntimeGeneration.h
        Source/Generation/PhraseGenerationWorker.cpp
        Source/Generation/PhraseGenerationWorker.h
        Source/Generation/GenerationActivationPolicy.cpp
        Source/Generation/GenerationActivationPolicy.h
)

target_include_directories(MIDI_GenGX
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(MIDI_GenGX
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MIDI_GenGX
    PRIVATE
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_core
        MIDI_GenGX_Music

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# -------------------- Standalone Domain tests --------------------
add_executable(MIDI_GenGX_DomainTests
    Source/Tests/DomainTests.cpp
)

target_include_directories(MIDI_GenGX_DomainTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_link_libraries(MIDI_GenGX_DomainTests PRIVATE MIDI_GenGX_Domain)
set_property(TARGET MIDI_GenGX_DomainTests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
add_test(NAME MIDI_GenGX_DomainTests COMMAND MIDI_GenGX_DomainTests)

# -------------------- Standalone Musical Engine tests --------------------
add_executable(MIDI_GenGX_MusicalEngineTests
    Source/Tests/MusicalEngineTests.cpp
)

target_include_directories(MIDI_GenGX_MusicalEngineTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_link_libraries(MIDI_GenGX_MusicalEngineTests PRIVATE MIDI_GenGX_Music)
set_property(TARGET MIDI_GenGX_MusicalEngineTests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
add_test(NAME MIDI_GenGX_MusicalEngineTests COMMAND MIDI_GenGX_MusicalEngineTests)

# -------------------- Generation activation tests --------------------
add_executable(MIDI_GenGX_GenerationTests
    Source/Tests/GenerationTests.cpp
    Source/Generation/GenerationActivationPolicy.cpp
)

target_include_directories(
    MIDI_GenGX_GenerationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

set_property(
    TARGET MIDI_GenGX_GenerationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_GenerationTests
    COMMAND MIDI_GenGX_GenerationTests
)

# -------------------- Phrase fingerprint tests --------------------
add_executable(MIDI_GenGX_PhraseFingerprintTests
    Source/Tests/PhraseFingerprintTests.cpp
)

target_include_directories(
    MIDI_GenGX_PhraseFingerprintTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_PhraseFingerprintTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_PhraseFingerprintTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_PhraseFingerprintTests
    COMMAND MIDI_GenGX_PhraseFingerprintTests
)

# -------------------- Motif fingerprint tests --------------------
add_executable(MIDI_GenGX_MotifFingerprintTests
    Source/Tests/MotifFingerprintTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifFingerprintTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifFingerprintTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifFingerprintTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifFingerprintTests
    COMMAND MIDI_GenGX_MotifFingerprintTests
)

# -------------------- Motif relationship tests --------------------
add_executable(MIDI_GenGX_MotifRelationshipTests
    Source/Tests/MotifRelationshipTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifRelationshipTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifRelationshipTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifRelationshipTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifRelationshipTests
    COMMAND MIDI_GenGX_MotifRelationshipTests
)

# -------------------- Motif occurrence graph tests --------------------
add_executable(MIDI_GenGX_MotifOccurrenceGraphTests
    Source/Tests/MotifOccurrenceGraphTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifOccurrenceGraphTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifOccurrenceGraphTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifOccurrenceGraphTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifOccurrenceGraphTests
    COMMAND MIDI_GenGX_MotifOccurrenceGraphTests
)

# -------------------- Motif recurrence profile tests --------------------
add_executable(MIDI_GenGX_MotifRecurrenceProfileTests
    Source/Tests/MotifRecurrenceProfileTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifRecurrenceProfileTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifRecurrenceProfileTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifRecurrenceProfileTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifRecurrenceProfileTests
    COMMAND MIDI_GenGX_MotifRecurrenceProfileTests
)

# -------------------- Motif recurrence metrics tests --------------------
add_executable(MIDI_GenGX_MotifRecurrenceMetricsTests
    Source/Tests/MotifRecurrenceMetricsTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifRecurrenceMetricsTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifRecurrenceMetricsTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifRecurrenceMetricsTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifRecurrenceMetricsTests
    COMMAND MIDI_GenGX_MotifRecurrenceMetricsTests
)

# -------------------- Motif knowledge record tests --------------------
add_executable(MIDI_GenGX_MotifKnowledgeRecordTests
    Source/Tests/MotifKnowledgeRecordTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifKnowledgeRecordTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifKnowledgeRecordTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifKnowledgeRecordTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifKnowledgeRecordTests
    COMMAND MIDI_GenGX_MotifKnowledgeRecordTests
)

# -------------------- Motif knowledge catalog tests --------------------
add_executable(MIDI_GenGX_MotifKnowledgeCatalogTests
    Source/Tests/MotifKnowledgeCatalogTests.cpp
)

target_include_directories(
    MIDI_GenGX_MotifKnowledgeCatalogTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MotifKnowledgeCatalogTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MotifKnowledgeCatalogTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MotifKnowledgeCatalogTests
    COMMAND MIDI_GenGX_MotifKnowledgeCatalogTests
)

# -------------------- Composition knowledge record tests --------------------
add_executable(MIDI_GenGX_CompositionKnowledgeRecordTests
    Source/Tests/CompositionKnowledgeRecordTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionKnowledgeRecordTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionKnowledgeRecordTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionKnowledgeRecordTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionKnowledgeRecordTests
    COMMAND MIDI_GenGX_CompositionKnowledgeRecordTests
)

# -------------------- Composition knowledge graph tests --------------------
add_executable(MIDI_GenGX_CompositionKnowledgeGraphTests
    Source/Tests/CompositionKnowledgeGraphTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionKnowledgeGraphTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionKnowledgeGraphTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionKnowledgeGraphTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionKnowledgeGraphTests
    COMMAND MIDI_GenGX_CompositionKnowledgeGraphTests
)

# -------------------- Composition transition profile tests --------------------
add_executable(MIDI_GenGX_CompositionTransitionProfileTests
    Source/Tests/CompositionTransitionProfileTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionTransitionProfileTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionTransitionProfileTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionTransitionProfileTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionTransitionProfileTests
    COMMAND MIDI_GenGX_CompositionTransitionProfileTests
)

# -------------------- Composition knowledge snapshot tests --------------------
add_executable(MIDI_GenGX_CompositionKnowledgeSnapshotTests
    Source/Tests/CompositionKnowledgeSnapshotTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionKnowledgeSnapshotTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionKnowledgeSnapshotTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionKnowledgeSnapshotTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionKnowledgeSnapshotTests
    COMMAND MIDI_GenGX_CompositionKnowledgeSnapshotTests
)

# -------------------- Composition knowledge representation tests --------------------
add_executable(MIDI_GenGX_CompositionKnowledgeRepresentationTests
    Source/Tests/CompositionKnowledgeRepresentationTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionKnowledgeRepresentationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Keep the standalone knowledge-representation test link independent of the
# caller's Visual Studio shell environment. The main build script uses the
# same toolchain locations; these directories provide the MSVC/UCRT libraries
# when the incremental target is built directly from a normal PowerShell.
set(MIDI_GENGX_MSVC_14_51_LIB_DIR
    "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/lib/x64")
set(MIDI_GENGX_WINDOWS_UM_LIB_DIR
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64")
set(MIDI_GENGX_WINDOWS_UCRT_LIB_DIR
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/ucrt/x64")

target_link_directories(
    MIDI_GenGX_CompositionKnowledgeRepresentationTests
    PRIVATE
        "${MIDI_GENGX_MSVC_14_51_LIB_DIR}"
        "${MIDI_GENGX_WINDOWS_UM_LIB_DIR}"
        "${MIDI_GENGX_WINDOWS_UCRT_LIB_DIR}"
)

target_link_libraries(
    MIDI_GenGX_CompositionKnowledgeRepresentationTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionKnowledgeRepresentationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionKnowledgeRepresentationTests
    COMMAND MIDI_GenGX_CompositionKnowledgeRepresentationTests
)

# -------------------- Composition dataset sample tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetSampleTests
    Source/Tests/CompositionDatasetSampleTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetSampleTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetSampleTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetSampleTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetSampleTests
    COMMAND MIDI_GenGX_CompositionDatasetSampleTests
)

# -------------------- Composition dataset tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetTests
    Source/Tests/CompositionDatasetTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetTests
    COMMAND MIDI_GenGX_CompositionDatasetTests
)

# -------------------- Composition dataset quality tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetQualityTests
    Source/Tests/CompositionDatasetQualityTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetQualityTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetQualityTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetQualityTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetQualityTests
    COMMAND MIDI_GenGX_CompositionDatasetQualityTests
)

# -------------------- Composition dataset partition tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetPartitionTests
    Source/Tests/CompositionDatasetPartitionTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetPartitionTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetPartitionTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetPartitionTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetPartitionTests
    COMMAND MIDI_GenGX_CompositionDatasetPartitionTests
)

# -------------------- Composition dataset schema tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetSchemaTests
    Source/Tests/CompositionDatasetSchemaTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetSchemaTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetSchemaTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetSchemaTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetSchemaTests
    COMMAND MIDI_GenGX_CompositionDatasetSchemaTests
)

# -------------------- Composition dataset manifest tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetManifestTests
    Source/Tests/CompositionDatasetManifestTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetManifestTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetManifestTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetManifestTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetManifestTests
    COMMAND MIDI_GenGX_CompositionDatasetManifestTests
)

# -------------------- Composition dataset batch tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetBatchTests
    Source/Tests/CompositionDatasetBatchTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetBatchTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetBatchTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetBatchTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetBatchTests
    COMMAND MIDI_GenGX_CompositionDatasetBatchTests
)

# -------------------- Composition dataset normalization tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetNormalizationTests
    Source/Tests/CompositionDatasetNormalizationTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetNormalizationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetNormalizationTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetNormalizationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetNormalizationTests
    COMMAND MIDI_GenGX_CompositionDatasetNormalizationTests
)

# -------------------- Composition dataset prepared view tests --------------------
add_executable(MIDI_GenGX_CompositionDatasetPreparedViewTests
    Source/Tests/CompositionDatasetPreparedViewTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionDatasetPreparedViewTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionDatasetPreparedViewTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionDatasetPreparedViewTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionDatasetPreparedViewTests
    COMMAND MIDI_GenGX_CompositionDatasetPreparedViewTests
)

# -------------------- Composition learning contract tests --------------------
add_executable(MIDI_GenGX_CompositionLearningContractTests
    Source/Tests/CompositionLearningContractTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionLearningContractTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionLearningContractTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionLearningContractTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionLearningContractTests
    COMMAND MIDI_GenGX_CompositionLearningContractTests
)

# -------------------- Composition model tests --------------------
add_executable(MIDI_GenGX_CompositionModelTests
    Source/Tests/CompositionModelTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionModelTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionModelTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionModelTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionModelTests
    COMMAND MIDI_GenGX_CompositionModelTests
)

# -------------------- Composition neural model tests --------------------
add_executable(MIDI_GenGX_CompositionNeuralModelTests
    Source/Tests/CompositionNeuralModelTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionNeuralModelTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionNeuralModelTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionNeuralModelTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionNeuralModelTests
    COMMAND MIDI_GenGX_CompositionNeuralModelTests
)

# -------------------- Composition neural trainer tests --------------------
add_executable(MIDI_GenGX_CompositionNeuralTrainerTests
    Source/Tests/CompositionNeuralTrainerTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionNeuralTrainerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionNeuralTrainerTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionNeuralTrainerTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionNeuralTrainerTests
    COMMAND MIDI_GenGX_CompositionNeuralTrainerTests
)

# -------------------- Composition model evaluation tests --------------------
add_executable(MIDI_GenGX_CompositionModelEvaluationTests
    Source/Tests/CompositionModelEvaluationTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionModelEvaluationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionModelEvaluationTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionModelEvaluationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionModelEvaluationTests
    COMMAND MIDI_GenGX_CompositionModelEvaluationTests
)

# -------------------- Composition musical evaluation tests --------------------
add_executable(MIDI_GenGX_CompositionMusicalEvaluationTests
    Source/Tests/CompositionMusicalEvaluationTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMusicalEvaluationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMusicalEvaluationTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMusicalEvaluationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMusicalEvaluationTests
    COMMAND MIDI_GenGX_CompositionMusicalEvaluationTests
)

# -------------------- Composition inference pipeline tests --------------------
add_executable(MIDI_GenGX_CompositionInferencePipelineTests
    Source/Tests/CompositionInferencePipelineTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionInferencePipelineTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionInferencePipelineTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionInferencePipelineTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionInferencePipelineTests
    COMMAND MIDI_GenGX_CompositionInferencePipelineTests
)

# -------------------- Composition runtime feature adapter tests --------------------
add_executable(MIDI_GenGX_CompositionRuntimeFeatureAdapterTests
    Source/Tests/CompositionRuntimeFeatureAdapterTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionRuntimeFeatureAdapterTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionRuntimeFeatureAdapterTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionRuntimeFeatureAdapterTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionRuntimeFeatureAdapterTests
    COMMAND MIDI_GenGX_CompositionRuntimeFeatureAdapterTests
)

# -------------------- Composition runtime inference adapter tests --------------------
add_executable(MIDI_GenGX_CompositionRuntimeInferenceAdapterTests
    Source/Tests/CompositionRuntimeInferenceAdapterTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionRuntimeInferenceAdapterTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionRuntimeInferenceAdapterTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionRuntimeInferenceAdapterTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionRuntimeInferenceAdapterTests
    COMMAND MIDI_GenGX_CompositionRuntimeInferenceAdapterTests
)

# -------------------- Composition runtime inference service tests --------------------
add_executable(MIDI_GenGX_CompositionRuntimeInferenceServiceTests
    Source/Tests/CompositionRuntimeInferenceServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionRuntimeInferenceServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionRuntimeInferenceServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionRuntimeInferenceServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionRuntimeInferenceServiceTests
    COMMAND MIDI_GenGX_CompositionRuntimeInferenceServiceTests
)

# -------------------- Composition neural model artifact tests --------------------
add_executable(MIDI_GenGX_CompositionNeuralModelArtifactTests
    Source/Tests/CompositionNeuralModelArtifactTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionNeuralModelArtifactTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionNeuralModelArtifactTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionNeuralModelArtifactTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionNeuralModelArtifactTests
    COMMAND MIDI_GenGX_CompositionNeuralModelArtifactTests
)

# -------------------- Composition neural model runtime loader tests --------------------
add_executable(MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests
    Source/Tests/CompositionNeuralModelRuntimeLoaderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests
    COMMAND MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests
)

# -------------------- Neural artifact training service tests --------------------
add_executable(MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests
    Source/Tests/CompositionNeuralArtifactTrainingServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests
    COMMAND MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests
)

# -------------------- Composition training corpus artifact tests --------------------
add_executable(MIDI_GenGX_CompositionTrainingCorpusArtifactTests
    Source/Tests/CompositionTrainingCorpusArtifactTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionTrainingCorpusArtifactTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionTrainingCorpusArtifactTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionTrainingCorpusArtifactTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionTrainingCorpusArtifactTests
    COMMAND MIDI_GenGX_CompositionTrainingCorpusArtifactTests
)

# -------------------- MIDI corpus reader tests --------------------
add_executable(MIDI_GenGX_CompositionMidiFileCorpusReaderTests
    Source/Tests/CompositionMidiFileCorpusReaderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiFileCorpusReaderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiFileCorpusReaderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiFileCorpusReaderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiFileCorpusReaderTests
    COMMAND MIDI_GenGX_CompositionMidiFileCorpusReaderTests
)

# -------------------- MIDI corpus analysis tests --------------------
add_executable(MIDI_GenGX_CompositionMidiCorpusAnalysisTests
    Source/Tests/CompositionMidiCorpusAnalysisTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiCorpusAnalysisTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiCorpusAnalysisTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiCorpusAnalysisTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiCorpusAnalysisTests
    COMMAND MIDI_GenGX_CompositionMidiCorpusAnalysisTests
)

# -------------------- MIDI section analyzer tests --------------------
add_executable(MIDI_GenGX_CompositionMidiSectionAnalyzerTests
    Source/Tests/CompositionMidiSectionAnalyzerTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiSectionAnalyzerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiSectionAnalyzerTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiSectionAnalyzerTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiSectionAnalyzerTests
    COMMAND MIDI_GenGX_CompositionMidiSectionAnalyzerTests
)

# -------------------- MIDI dataset feature extractor tests --------------------
add_executable(MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests
    Source/Tests/CompositionMidiDatasetFeatureExtractorTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests
    COMMAND MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests
)

# -------------------- MIDI harmony analysis tests --------------------
add_executable(MIDI_GenGX_CompositionMidiHarmonyTests
    Source/Tests/CompositionMidiHarmonyTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiHarmonyTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiHarmonyTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiHarmonyTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiHarmonyTests
    COMMAND MIDI_GenGX_CompositionMidiHarmonyTests
)

# -------------------- MIDI motif analysis tests --------------------
add_executable(MIDI_GenGX_CompositionMidiMotifAnalysisTests
    Source/Tests/CompositionMidiMotifAnalysisTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiMotifAnalysisTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiMotifAnalysisTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiMotifAnalysisTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiMotifAnalysisTests
    COMMAND MIDI_GenGX_CompositionMidiMotifAnalysisTests
)

# -------------------- MIDI dataset builder tests --------------------
add_executable(MIDI_GenGX_CompositionMidiDatasetBuilderTests
    Source/Tests/CompositionMidiDatasetBuilderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiDatasetBuilderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiDatasetBuilderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiDatasetBuilderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiDatasetBuilderTests
    COMMAND MIDI_GenGX_CompositionMidiDatasetBuilderTests
)

# -------------------- Composition intent tests --------------------
add_executable(MIDI_GenGX_CompositionIntentTests
    Source/Tests/CompositionIntentTests.cpp
)

target_link_libraries(MIDI_GenGX_CompositionIntentTests
    PRIVATE
        MIDI_GenGX_Domain
)

target_include_directories(MIDI_GenGX_CompositionIntentTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

set_property(
    TARGET MIDI_GenGX_CompositionIntentTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionIntentTests
    COMMAND MIDI_GenGX_CompositionIntentTests
)

# -------------------- MIDI corpus directory loader tests --------------------
add_executable(MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests
    Source/Tests/CompositionMidiCorpusDirectoryLoaderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests
    COMMAND MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests
)

# -------------------- MIDI training pipeline tests --------------------
add_executable(MIDI_GenGX_CompositionMidiTrainingPipelineTests
    Source/Tests/CompositionMidiTrainingPipelineTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiTrainingPipelineTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiTrainingPipelineTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiTrainingPipelineTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiTrainingPipelineTests
    COMMAND MIDI_GenGX_CompositionMidiTrainingPipelineTests
)

# -------------------- MIDI corpus training service tests --------------------
add_executable(MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests
    Source/Tests/CompositionMidiCorpusTrainingServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests
    COMMAND MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests
)

# -------------------- MIDI training evaluation service tests --------------------
add_executable(MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests
    Source/Tests/CompositionMidiTrainingEvaluationServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests
    COMMAND MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests
)

# -------------------- Neural generation service tests --------------------
add_executable(MIDI_GenGX_CompositionNeuralGenerationServiceTests
    Source/Tests/CompositionNeuralGenerationServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionNeuralGenerationServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionNeuralGenerationServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionNeuralGenerationServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionNeuralGenerationServiceTests
    COMMAND MIDI_GenGX_CompositionNeuralGenerationServiceTests
)

# -------------------- AI MIDI generation service tests --------------------
add_executable(MIDI_GenGX_CompositionAIMidiGenerationServiceTests
    Source/Tests/CompositionAIMidiGenerationServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionAIMidiGenerationServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionAIMidiGenerationServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionAIMidiGenerationServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionAIMidiGenerationServiceTests
    COMMAND MIDI_GenGX_CompositionAIMidiGenerationServiceTests
)

# -------------------- Phase 83 training sequence tests --------------------
add_executable(MIDI_GenGX_CompositionMidiTrainingSequenceTests
    Source/Tests/CompositionMidiTrainingSequenceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiTrainingSequenceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiTrainingSequenceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiTrainingSequenceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiTrainingSequenceTests
    COMMAND MIDI_GenGX_CompositionMidiTrainingSequenceTests
)

# -------------------- Phase 84 training corpus artifact tests --------------------
add_executable(MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests
    Source/Tests/CompositionMidiTrainingCorpusArtifactTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests
    COMMAND MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests
)

# -------------------- Phase 85 sequence window tests --------------------
add_executable(MIDI_GenGX_CompositionMidiSequenceWindowTests
    Source/Tests/CompositionMidiSequenceWindowTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiSequenceWindowTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiSequenceWindowTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiSequenceWindowTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiSequenceWindowTests
    COMMAND MIDI_GenGX_CompositionMidiSequenceWindowTests
)

# -------------------- Phase 86 sequence neural trainer tests --------------------
add_executable(MIDI_GenGX_CompositionSequenceNeuralTrainerTests
    Source/Tests/CompositionSequenceNeuralTrainerTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionSequenceNeuralTrainerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionSequenceNeuralTrainerTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionSequenceNeuralTrainerTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionSequenceNeuralTrainerTests
    COMMAND MIDI_GenGX_CompositionSequenceNeuralTrainerTests
)

# -------------------- Phase 87 sequence neural training service tests --------------------
add_executable(MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests
    Source/Tests/CompositionSequenceNeuralTrainingServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests
    COMMAND MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests
)

# -------------------- Phase 88 sequence corpus builder tests --------------------
add_executable(MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests
    Source/Tests/CompositionMidiSequenceCorpusBuilderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests
    COMMAND MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests
)

# -------------------- Phase 89 metadata tests --------------------
add_executable(MIDI_GenGX_CompositionSequenceMetadataTests
    Source/Tests/CompositionSequenceMetadataTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionSequenceMetadataTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionSequenceMetadataTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionSequenceMetadataTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionSequenceMetadataTests
    COMMAND MIDI_GenGX_CompositionSequenceMetadataTests
)

# -------------------- Phase 96 composer knowledge representation tests --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgeSampleTests
    Source/Tests/CompositionComposerKnowledgeSampleTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgeSampleTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgeSampleTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgeSampleTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgeSampleTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgeSampleTests
)

# -------------------- Phase 97 composer knowledge catalog tests --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgeCatalogTests
    Source/Tests/CompositionComposerKnowledgeCatalogTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgeCatalogTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgeCatalogTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgeCatalogTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgeCatalogTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgeCatalogTests
)

# -------------------- Phase 98 composer knowledge partition tests --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgePartitionTests
    Source/Tests/CompositionComposerKnowledgePartitionTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgePartitionTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgePartitionTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgePartitionTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgePartitionTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgePartitionTests
)

# -------------------- Phase 99 composer knowledge corpus manifest tests --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests
    Source/Tests/CompositionComposerKnowledgeCorpusManifestTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests
)

# -------------------- Phase 100 composer knowledge training corpus tests --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests
    Source/Tests/CompositionComposerKnowledgeTrainingCorpusTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests
)

# -------------------- Phase 101 corpus source manifest tests --------------------
add_executable(MIDI_GenGX_CompositionComposerCorpusSourceManifestTests
    Source/Tests/CompositionComposerCorpusSourceManifestTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerCorpusSourceManifestTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerCorpusSourceManifestTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerCorpusSourceManifestTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerCorpusSourceManifestTests
    COMMAND MIDI_GenGX_CompositionComposerCorpusSourceManifestTests
)

# -------------------- Phase 102 corpus assembly tests --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests
    Source/Tests/CompositionComposerKnowledgeCorpusAssemblyTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests
)

# -------------------- Phase 103 real composer corpus intake tests --------------------
add_executable(MIDI_GenGX_CompositionRealComposerCorpusIntakeTests
    Source/Tests/CompositionRealComposerCorpusIntakeTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionRealComposerCorpusIntakeTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionRealComposerCorpusIntakeTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionRealComposerCorpusIntakeTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionRealComposerCorpusIntakeTests
    COMMAND MIDI_GenGX_CompositionRealComposerCorpusIntakeTests
)

# -------------------- Phase 104 real composer preparation tests --------------------
add_executable(MIDI_GenGX_CompositionRealComposerCorpusPreparationTests
    Source/Tests/CompositionRealComposerCorpusPreparationTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionRealComposerCorpusPreparationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionRealComposerCorpusPreparationTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionRealComposerCorpusPreparationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionRealComposerCorpusPreparationTests
    COMMAND MIDI_GenGX_CompositionRealComposerCorpusPreparationTests
)



# -------------------- Phase 90 conditioned dataset tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedTrainingDatasetTests
    Source/Tests/CompositionConditionedTrainingDatasetTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedTrainingDatasetTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedTrainingDatasetTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedTrainingDatasetTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedTrainingDatasetTests
    COMMAND MIDI_GenGX_CompositionConditionedTrainingDatasetTests
)

# -------------------- Phase 91 conditioned sequence neural trainer tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests
    Source/Tests/CompositionConditionedSequenceNeuralTrainerTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests
    COMMAND MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests
)

# -------------------- Phase 92 conditioned training service tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests
    Source/Tests/CompositionConditionedSequenceNeuralTrainingServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests
    COMMAND MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests
)

# -------------------- Phase 93 end-to-end conditioned training run tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedTrainingRunServiceTests
    Source/Tests/CompositionConditionedTrainingRunServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedTrainingRunServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedTrainingRunServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedTrainingRunServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedTrainingRunServiceTests
    COMMAND MIDI_GenGX_CompositionConditionedTrainingRunServiceTests
)

# -------------------- Phase 94 real corpus conditioned training tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests
    Source/Tests/CompositionConditionedCorpusTrainingServiceTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests
    COMMAND MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests
)

# -------------------- Phase 109 real composer training CLI --------------------
add_executable(MIDI_GenGX_CompositionRealComposerTrainingCli
    Source/Music/CompositionRealComposerTrainingCli.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionRealComposerTrainingCli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionRealComposerTrainingCli
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionRealComposerTrainingCli
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

# -------------------- Phase 109 metadata loader tests --------------------
add_executable(MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests
    Source/Tests/CompositionSequenceMetadataFileLoaderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests
    COMMAND MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests
)

# -------------------- Phase 107 neural evaluation tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests
    Source/Tests/CompositionConditionedSequenceNeuralEvaluatorTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests
    COMMAND MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests
)

# -------------------- Phase 110 conditioned model artifact tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests
    Source/Tests/CompositionConditionedSequenceNeuralModelArtifactTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests
    COMMAND MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests
)

# -------------------- Phase 112 conditioned runtime loader tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests
    Source/Tests/CompositionConditionedSequenceNeuralModelRuntimeLoaderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests
    COMMAND MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests
)

# -------------------- Phase 113 real model artifact inspector --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests
    Source/Tests/CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests
    COMMAND MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactFileInspectorTests
)

# -------------------- Composition AI model runtime provider tests --------------------
add_executable(MIDI_GenGX_CompositionAIModelRuntimeProviderTests
    Source/Tests/CompositionAIModelRuntimeProviderTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionAIModelRuntimeProviderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionAIModelRuntimeProviderTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionAIModelRuntimeProviderTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionAIModelRuntimeProviderTests
    COMMAND MIDI_GenGX_CompositionAIModelRuntimeProviderTests
)

# -------------------- Composition AI constraint adapter tests --------------------
add_executable(MIDI_GenGX_CompositionAIConstraintAdapterTests
    Source/Tests/CompositionAIConstraintAdapterTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionAIConstraintAdapterTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionAIConstraintAdapterTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionAIConstraintAdapterTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionAIConstraintAdapterTests
    COMMAND MIDI_GenGX_CompositionAIConstraintAdapterTests
)

# -------------------- Composition AI engine bridge tests --------------------
add_executable(MIDI_GenGX_CompositionAIEngineBridgeTests
    Source/Tests/CompositionAIEngineBridgeTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionAIEngineBridgeTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionAIEngineBridgeTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionAIEngineBridgeTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionAIEngineBridgeTests
    COMMAND MIDI_GenGX_CompositionAIEngineBridgeTests
)

# -------------------- Musical Engine AI guidance tests --------------------
add_executable(MIDI_GenGX_MusicalEngineAIGuidanceTests
    Source/Tests/MusicalEngineAIGuidanceTests.cpp
)

target_include_directories(
    MIDI_GenGX_MusicalEngineAIGuidanceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_MusicalEngineAIGuidanceTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_MusicalEngineAIGuidanceTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_MusicalEngineAIGuidanceTests
    COMMAND MIDI_GenGX_MusicalEngineAIGuidanceTests
)

# -------------------- Composition AI generation coordinator tests --------------------
add_executable(MIDI_GenGX_CompositionAIGenerationCoordinatorTests
    Source/Tests/CompositionAIGenerationCoordinatorTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionAIGenerationCoordinatorTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionAIGenerationCoordinatorTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionAIGenerationCoordinatorTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionAIGenerationCoordinatorTests
    COMMAND MIDI_GenGX_CompositionAIGenerationCoordinatorTests
)

# -------------------- AI generation functional tests --------------------
add_executable(MIDI_GenGX_AIGenerationFunctionalTests
    Source/Tests/AIGenerationFunctionalTests.cpp
)

target_include_directories(
    MIDI_GenGX_AIGenerationFunctionalTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_AIGenerationFunctionalTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_AIGenerationFunctionalTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_AIGenerationFunctionalTests
    COMMAND MIDI_GenGX_AIGenerationFunctionalTests
)

# -------------------- AI plugin runtime integration tests --------------------
add_executable(MIDI_GenGX_AIPluginRuntimeIntegrationTests
    Source/Tests/AIPluginRuntimeIntegrationTests.cpp
)

target_include_directories(
    MIDI_GenGX_AIPluginRuntimeIntegrationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_BINARY_DIR}/MIDI_GenGX_artefacts/JuceLibraryCode
        ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules
)

target_link_libraries(
    MIDI_GenGX_AIPluginRuntimeIntegrationTests
    PRIVATE
        MIDI_GenGX
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
)

target_compile_definitions(
    MIDI_GenGX_AIPluginRuntimeIntegrationTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

set_property(
    TARGET MIDI_GenGX_AIPluginRuntimeIntegrationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_AIPluginRuntimeIntegrationTests
    COMMAND MIDI_GenGX_AIPluginRuntimeIntegrationTests
)

# -------------------- AI runtime model integration tests --------------------
add_executable(MIDI_GenGX_AIRuntimeGenerationModelTests
    Source/Tests/AIRuntimeGenerationModelTests.cpp
)

target_include_directories(
    MIDI_GenGX_AIRuntimeGenerationModelTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_BINARY_DIR}/MIDI_GenGX_artefacts/JuceLibraryCode
        ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules
)

target_link_libraries(
    MIDI_GenGX_AIRuntimeGenerationModelTests
    PRIVATE
        MIDI_GenGX
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
)

target_compile_definitions(
    MIDI_GenGX_AIRuntimeGenerationModelTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

set_property(
    TARGET MIDI_GenGX_AIRuntimeGenerationModelTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_AIRuntimeGenerationModelTests
    COMMAND MIDI_GenGX_AIRuntimeGenerationModelTests
)

# -------------------- Plugin AI runtime model loading tests --------------------
add_executable(MIDI_GenGX_PluginAIRuntimeModelLoadingTests
    Source/Tests/PluginAIRuntimeModelLoadingTests.cpp
)

target_include_directories(
    MIDI_GenGX_PluginAIRuntimeModelLoadingTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_BINARY_DIR}/MIDI_GenGX_artefacts/JuceLibraryCode
        ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules
)

target_link_libraries(
    MIDI_GenGX_PluginAIRuntimeModelLoadingTests
    PRIVATE
        MIDI_GenGX
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
)

target_compile_definitions(
    MIDI_GenGX_PluginAIRuntimeModelLoadingTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

set_property(
    TARGET MIDI_GenGX_PluginAIRuntimeModelLoadingTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_PluginAIRuntimeModelLoadingTests
    COMMAND MIDI_GenGX_PluginAIRuntimeModelLoadingTests
)

# -------------------- Plugin AI end-to-end generation tests --------------------
add_executable(MIDI_GenGX_PluginAIRuntimeEndToEndTests
    Source/Tests/PluginAIRuntimeEndToEndTests.cpp
)

target_include_directories(
    MIDI_GenGX_PluginAIRuntimeEndToEndTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_BINARY_DIR}/MIDI_GenGX_artefacts/JuceLibraryCode
        ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules
)

target_link_libraries(
    MIDI_GenGX_PluginAIRuntimeEndToEndTests
    PRIVATE
        MIDI_GenGX
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
)

target_compile_definitions(
    MIDI_GenGX_PluginAIRuntimeEndToEndTests
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

set_property(
    TARGET MIDI_GenGX_PluginAIRuntimeEndToEndTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_PluginAIRuntimeEndToEndTests
    COMMAND MIDI_GenGX_PluginAIRuntimeEndToEndTests
)

# -------------------- Phase 116 composer knowledge representation --------------------
add_executable(MIDI_GenGX_CompositionComposerKnowledgeRepresentationTests
    Source/Tests/CompositionComposerKnowledgeRepresentationTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionComposerKnowledgeRepresentationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionComposerKnowledgeRepresentationTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionComposerKnowledgeRepresentationTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionComposerKnowledgeRepresentationTests
    COMMAND MIDI_GenGX_CompositionComposerKnowledgeRepresentationTests
)

add_executable(MIDI_GenGX_CompositionGenreKnowledgeRepresentationTests
    Source/Tests/CompositionGenreKnowledgeRepresentationTests.cpp
)

target_link_libraries(MIDI_GenGX_CompositionGenreKnowledgeRepresentationTests
    PRIVATE
        MIDI_GenGX_Music
        MIDI_GenGX_Domain
)

add_test(
    NAME MIDI_GenGX_CompositionGenreKnowledgeRepresentationTests
    COMMAND MIDI_GenGX_CompositionGenreKnowledgeRepresentationTests
)


add_executable(MIDI_GenGX_CompositionSoundEngineeringKnowledgeRepresentationTests
    Source/Tests/CompositionSoundEngineeringKnowledgeRepresentationTests.cpp
)

target_link_libraries(MIDI_GenGX_CompositionSoundEngineeringKnowledgeRepresentationTests
    PRIVATE
        MIDI_GenGX_Music
        MIDI_GenGX_Domain
)

add_test(
    NAME MIDI_GenGX_CompositionSoundEngineeringKnowledgeRepresentationTests
    COMMAND MIDI_GenGX_CompositionSoundEngineeringKnowledgeRepresentationTests
)

add_executable(MIDI_GenGX_CompositionKnowledgeTrainingDatasetTests
    Source/Tests/CompositionKnowledgeTrainingDatasetTests.cpp
)

target_link_libraries(MIDI_GenGX_CompositionKnowledgeTrainingDatasetTests
    PRIVATE
        MIDI_GenGX_Music
        MIDI_GenGX_Domain
)

set_property(
    TARGET MIDI_GenGX_CompositionKnowledgeTrainingDatasetTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionKnowledgeTrainingDatasetTests
    COMMAND MIDI_GenGX_CompositionKnowledgeTrainingDatasetTests
)

# -------------------- Phase 121A conditioned model fine-tuning --------------------
add_executable(MIDI_GenGX_CompositionConditionedSequenceNeuralFineTuningCli
    Source/Music/CompositionConditionedSequenceNeuralFineTuningCli.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedSequenceNeuralFineTuningCli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedSequenceNeuralFineTuningCli
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedSequenceNeuralFineTuningCli
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

# -------------------- Phase 121 baseline vs final evaluation --------------------
add_executable(MIDI_GenGX_Phase121BaselineVsFinalEvaluation
    Source/Music/Phase121BaselineVsFinalEvaluationCli.cpp
)

target_include_directories(
    MIDI_GenGX_Phase121BaselineVsFinalEvaluation
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_Phase121BaselineVsFinalEvaluation
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_Phase121BaselineVsFinalEvaluation
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

# -------------------- Phase 121 validation/test comparison --------------------
add_executable(MIDI_GenGX_Phase121ValidationTestComparison
    Source/Music/Phase121ValidationTestComparisonCli.cpp
)

target_include_directories(
    MIDI_GenGX_Phase121ValidationTestComparison
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_Phase121ValidationTestComparison
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_Phase121ValidationTestComparison
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

# -------------------- Phase 121 real autoregressive rollout comparison --------------------
add_executable(MIDI_GenGX_Phase121RealAutoregressiveRolloutComparison
    Source/Music/Phase121RealAutoregressiveRolloutComparisonCli.cpp
)

target_include_directories(
    MIDI_GenGX_Phase121RealAutoregressiveRolloutComparison
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_Phase121RealAutoregressiveRolloutComparison
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_Phase121RealAutoregressiveRolloutComparison
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

# -------------------- Phase 122 generated MIDI musical metrics --------------------
add_executable(MIDI_GenGX_Phase122GeneratedMidiMusicalMetrics
    Source/Music/Phase122GeneratedMidiMusicalMetricsCli.cpp
)

target_include_directories(
    MIDI_GenGX_Phase122GeneratedMidiMusicalMetrics
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_Phase122GeneratedMidiMusicalMetrics
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_Phase122GeneratedMidiMusicalMetrics
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

# -------------------- Phase 122B structured loss tests --------------------
add_executable(MIDI_GenGX_CompositionConditionedStructuredLossTests
    Source/Tests/CompositionConditionedStructuredLossTests.cpp
)

target_include_directories(
    MIDI_GenGX_CompositionConditionedStructuredLossTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(
    MIDI_GenGX_CompositionConditionedStructuredLossTests
    PRIVATE
        MIDI_GenGX_Music
)

set_property(
    TARGET MIDI_GenGX_CompositionConditionedStructuredLossTests
    PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

add_test(
    NAME MIDI_GenGX_CompositionConditionedStructuredLossTests
    COMMAND MIDI_GenGX_CompositionConditionedStructuredLossTests
)
