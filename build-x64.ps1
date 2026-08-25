$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

$VCTools = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231"
$WindowsKit = "C:\Program Files (x86)\Windows Kits\10"
$WindowsKitVersion = "10.0.26100.0"

$MsvcBinX64 = Join-Path $VCTools "bin\Hostx64\x64"
$MsvcLibX64 = Join-Path $VCTools "lib\x64"
$WindowsUmX64 = Join-Path $WindowsKit "Lib\$WindowsKitVersion\um\x64"
$WindowsUcrtX64 = Join-Path $WindowsKit "Lib\$WindowsKitVersion\ucrt\x64"

foreach ($PathToCheck in @(
    (Join-Path $ProjectRoot "JUCE"),
    $VCTools,
    $MsvcBinX64,
    $MsvcLibX64,
    $WindowsUmX64,
    $WindowsUcrtX64
)) {
    if (-not (Test-Path $PathToCheck)) {
        throw "Required path not found: $PathToCheck"
    }
}

$env:PATH = "$MsvcBinX64;$env:PATH"
$env:LIB = "$MsvcLibX64;$WindowsUcrtX64;$WindowsUmX64"
$env:LIBPATH = "$MsvcLibX64"
$env:VCToolsInstallDir = "$VCTools\"

$BuildDir = Join-Path $ProjectRoot "out\build\x64-Debug"
$ArtifactPath = Join-Path `
    $BuildDir `
    "MIDI_GenGX_artefacts\Debug\VST3\MIDI-GenGX.vst3"

$TestDomain = Join-Path `
    $BuildDir `
    "MIDI_GenGX_DomainTests.exe"

$TestEngine = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MusicalEngineTests.exe"

$TestGeneration = Join-Path `
    $BuildDir `
    "MIDI_GenGX_GenerationTests.exe"

$TestFingerprint = Join-Path `
    $BuildDir `
    "MIDI_GenGX_PhraseFingerprintTests.exe"

$TestMotifFingerprint = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifFingerprintTests.exe"

$TestMotifRelationship = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifRelationshipTests.exe"

$TestMotifOccurrenceGraph = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifOccurrenceGraphTests.exe"

$TestMotifRecurrenceProfile = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifRecurrenceProfileTests.exe"

$TestMotifRecurrenceMetrics = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifRecurrenceMetricsTests.exe"

$TestMotifKnowledgeRecord = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifKnowledgeRecordTests.exe"

$TestMotifKnowledgeCatalog = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MotifKnowledgeCatalogTests.exe"

$TestCompositionKnowledgeRecord = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionKnowledgeRecordTests.exe"

$TestCompositionKnowledgeGraph = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionKnowledgeGraphTests.exe"

$TestCompositionTransitionProfile = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionTransitionProfileTests.exe"

$TestCompositionKnowledgeSnapshot = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionKnowledgeSnapshotTests.exe"

$TestCompositionDatasetSample = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetSampleTests.exe"

$TestCompositionDataset = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetTests.exe"

$TestCompositionDatasetQuality = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetQualityTests.exe"

$TestCompositionDatasetPartition = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetPartitionTests.exe"

$TestCompositionDatasetSchema = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetSchemaTests.exe"

$TestCompositionDatasetManifest = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetManifestTests.exe"

$TestCompositionDatasetBatch = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetBatchTests.exe"

$TestCompositionDatasetNormalization = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetNormalizationTests.exe"

$TestCompositionDatasetPreparedView = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionDatasetPreparedViewTests.exe"

$TestCompositionLearningContract = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionLearningContractTests.exe"

$TestCompositionModel = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionModelTests.exe"

$TestCompositionNeuralModel = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionNeuralModelTests.exe"

$TestCompositionNeuralTrainer = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionNeuralTrainerTests.exe"

$TestCompositionModelEvaluation = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionModelEvaluationTests.exe"

$TestCompositionMusicalEvaluation = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMusicalEvaluationTests.exe"

$TestCompositionInferencePipeline = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionInferencePipelineTests.exe"

$TestCompositionRuntimeFeatureAdapter = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionRuntimeFeatureAdapterTests.exe"

$TestCompositionRuntimeInferenceAdapter = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionRuntimeInferenceAdapterTests.exe"

$TestCompositionRuntimeInferenceService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionRuntimeInferenceServiceTests.exe"

$TestCompositionNeuralModelArtifact = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionNeuralModelArtifactTests.exe"

$TestCompositionNeuralModelRuntimeLoader = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests.exe"

$TestCompositionNeuralArtifactTrainingService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests.exe"

$TestCompositionTrainingCorpusArtifact = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionTrainingCorpusArtifactTests.exe"

$TestCompositionMidiFileCorpusReader = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiFileCorpusReaderTests.exe"

$TestCompositionMidiCorpusAnalysis = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiCorpusAnalysisTests.exe"

$TestCompositionMidiSectionAnalyzer = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiSectionAnalyzerTests.exe"

$TestCompositionMidiDatasetFeatureExtractor = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests.exe"

$TestCompositionMidiHarmony = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiHarmonyTests.exe"

$TestCompositionMidiMotifAnalysis = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiMotifAnalysisTests.exe"

$TestCompositionMidiDatasetBuilder = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiDatasetBuilderTests.exe"

$TestCompositionMidiCorpusDirectoryLoader = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests.exe"

$TestCompositionMidiTrainingPipeline = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiTrainingPipelineTests.exe"

$TestCompositionMidiCorpusTrainingService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests.exe"

$TestCompositionMidiTrainingEvaluationService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests.exe"

$TestCompositionNeuralGenerationService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionNeuralGenerationServiceTests.exe"

$TestCompositionAIMidiGenerationService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionAIMidiGenerationServiceTests.exe"

$TestCompositionMidiTrainingSequence = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiTrainingSequenceTests.exe"

$TestCompositionMidiTrainingCorpusArtifact = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests.exe"

$TestCompositionMidiSequenceWindow = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiSequenceWindowTests.exe"

$TestCompositionSequenceNeuralTrainer = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionSequenceNeuralTrainerTests.exe"

$TestCompositionSequenceNeuralTrainingService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests.exe"

$TestCompositionMidiSequenceCorpusBuilder = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests.exe"

$TestCompositionSequenceMetadata = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionSequenceMetadataTests.exe"

$TestCompositionComposerKnowledgeSample = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerKnowledgeSampleTests.exe"

$TestCompositionComposerKnowledgeCatalog = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerKnowledgeCatalogTests.exe"

$TestCompositionComposerKnowledgePartition = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerKnowledgePartitionTests.exe"

$TestCompositionComposerKnowledgeCorpusManifest = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests.exe"

$TestCompositionComposerKnowledgeTrainingCorpus = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests.exe"

$TestCompositionComposerCorpusSourceManifest = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerCorpusSourceManifestTests.exe"

$TestCompositionComposerKnowledgeCorpusAssembly = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests.exe"

$TestCompositionRealComposerCorpusIntake = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionRealComposerCorpusIntakeTests.exe"

$TestCompositionRealComposerCorpusPreparation = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionRealComposerCorpusPreparationTests.exe"

$TestCompositionConditionedSequenceNeuralModelRuntimeLoader = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests.exe"

$TestCompositionConditionedSequenceNeuralModelArtifact = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests.exe"

$TestCompositionConditionedSequenceNeuralEvaluator = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests.exe"

$TestCompositionConditionedTrainingDataset = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedTrainingDatasetTests.exe"

$TestCompositionConditionedSequenceNeuralTrainer = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests.exe"

$TestCompositionConditionedSequenceNeuralTrainingService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests.exe"

$TestCompositionConditionedTrainingRunService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedTrainingRunServiceTests.exe"

$TestCompositionConditionedCorpusTrainingService = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests.exe"

$TestCompositionSequenceMetadataFileLoader = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests.exe"

$RealComposerTrainingCli = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionRealComposerTrainingCli.exe"

$TestCompositionAIModelRuntimeProvider = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionAIModelRuntimeProviderTests.exe"

$TestCompositionAIConstraintAdapter = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionAIConstraintAdapterTests.exe"

$TestCompositionAIEngineBridge = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionAIEngineBridgeTests.exe"

$TestMusicalEngineAIGuidance = Join-Path `
    $BuildDir `
    "MIDI_GenGX_MusicalEngineAIGuidanceTests.exe"

$TestCompositionAIGenerationCoordinator = Join-Path `
    $BuildDir `
    "MIDI_GenGX_CompositionAIGenerationCoordinatorTests.exe"

$TestAIGenerationFunctional = Join-Path `
    $BuildDir `
    "MIDI_GenGX_AIGenerationFunctionalTests.exe"

$TestAIPluginRuntimeIntegration = Join-Path `
    $BuildDir `
    "MIDI_GenGX_AIPluginRuntimeIntegrationTests.exe"

$TestAIRuntimeGenerationModel = Join-Path `
    $BuildDir `
    "MIDI_GenGX_AIRuntimeGenerationModelTests.exe"

$TestPluginAIRuntimeModelLoading = Join-Path `
    $BuildDir `
    "MIDI_GenGX_PluginAIRuntimeModelLoadingTests.exe"

$TestPluginAIRuntimeEndToEnd = Join-Path `
    $BuildDir `
    "MIDI_GenGX_PluginAIRuntimeEndToEndTests.exe"

$UserVst3Dir = Join-Path `
    $env:LOCALAPPDATA `
    "Programs\Common\VST3"

$UserPluginPath = Join-Path `
    $UserVst3Dir `
    "MIDI-GenGX.vst3"

if (Test-Path $BuildDir) {
    Write-Host "Cleaning previous x64 Debug build..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host "Configuring x64 Debug..." -ForegroundColor Cyan

cmake `
    -S $ProjectRoot `
    -B $BuildDir `
    -G "Ninja" `
    -DCMAKE_BUILD_TYPE=Debug `
    -DCMAKE_C_COMPILER="$MsvcBinX64\cl.exe" `
    -DCMAKE_CXX_COMPILER="$MsvcBinX64\cl.exe"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed."
}

Write-Host "Building VST3..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI-GenGX.vst3" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "VST3 build failed."
}

if (-not (Test-Path $ArtifactPath)) {
    throw "VST3 artefact not found: $ArtifactPath"
}

Write-Host "Building CompositionSequenceMetadataFileLoaderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionSequenceMetadataFileLoaderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceMetadataFileLoaderTests build failed."
}

& $TestCompositionSequenceMetadataFileLoader

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceMetadataFileLoaderTests failed."
}

Write-Host "COMPOSITION SEQUENCE METADATA FILE LOADER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionRealComposerTrainingCli..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionRealComposerTrainingCli" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRealComposerTrainingCli build failed."
}

Write-Host "REAL COMPOSER TRAINING CLI BUILD PASSED" -ForegroundColor Green

Write-Host "PLUGIN BUILD SUCCESSFUL" -ForegroundColor Green

Write-Host "Building DomainTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_DomainTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "DomainTests build failed."
}

& $TestDomain

if ($LASTEXITCODE -ne 0) {
    throw "DomainTests failed."
}

Write-Host "DOMAIN TESTS PASSED" -ForegroundColor Green

Write-Host "Building MusicalEngineTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MusicalEngineTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MusicalEngineTests build failed."
}

& $TestEngine

if ($LASTEXITCODE -ne 0) {
    throw "MusicalEngineTests failed."
}

Write-Host "MUSICAL ENGINE TESTS PASSED" -ForegroundColor Green

Write-Host "Building GenerationTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_GenerationTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "GenerationTests build failed."
}

& $TestGeneration

if ($LASTEXITCODE -ne 0) {
    throw "GenerationTests failed."
}

Write-Host "GENERATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building PhraseFingerprintTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_PhraseFingerprintTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "PhraseFingerprintTests build failed."
}

& $TestFingerprint

if ($LASTEXITCODE -ne 0) {
    throw "PhraseFingerprintTests failed."
}

Write-Host "PHRASE FINGERPRINT TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifFingerprintTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifFingerprintTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifFingerprintTests build failed."
}

& $TestMotifFingerprint

if ($LASTEXITCODE -ne 0) {
    throw "MotifFingerprintTests failed."
}

Write-Host "MOTIF FINGERPRINT TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifRelationshipTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifRelationshipTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifRelationshipTests build failed."
}

& $TestMotifRelationship

if ($LASTEXITCODE -ne 0) {
    throw "MotifRelationshipTests failed."
}

Write-Host "MOTIF RELATIONSHIP TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifOccurrenceGraphTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifOccurrenceGraphTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifOccurrenceGraphTests build failed."
}

& $TestMotifOccurrenceGraph

if ($LASTEXITCODE -ne 0) {
    throw "MotifOccurrenceGraphTests failed."
}

Write-Host "MOTIF OCCURRENCE GRAPH TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifRecurrenceProfileTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifRecurrenceProfileTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifRecurrenceProfileTests build failed."
}

& $TestMotifRecurrenceProfile

if ($LASTEXITCODE -ne 0) {
    throw "MotifRecurrenceProfileTests failed."
}

Write-Host "MOTIF RECURRENCE PROFILE TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifRecurrenceMetricsTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifRecurrenceMetricsTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifRecurrenceMetricsTests build failed."
}

& $TestMotifRecurrenceMetrics

if ($LASTEXITCODE -ne 0) {
    throw "MotifRecurrenceMetricsTests failed."
}

Write-Host "MOTIF RECURRENCE METRICS TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifKnowledgeRecordTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifKnowledgeRecordTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifKnowledgeRecordTests build failed."
}

& $TestMotifKnowledgeRecord

if ($LASTEXITCODE -ne 0) {
    throw "MotifKnowledgeRecordTests failed."
}

Write-Host "MOTIF KNOWLEDGE RECORD TESTS PASSED" -ForegroundColor Green

Write-Host "Building MotifKnowledgeCatalogTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MotifKnowledgeCatalogTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MotifKnowledgeCatalogTests build failed."
}

& $TestMotifKnowledgeCatalog

if ($LASTEXITCODE -ne 0) {
    throw "MotifKnowledgeCatalogTests failed."
}

Write-Host "MOTIF KNOWLEDGE CATALOG TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionKnowledgeRecordTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionKnowledgeRecordTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionKnowledgeRecordTests build failed."
}

& $TestCompositionKnowledgeRecord

if ($LASTEXITCODE -ne 0) {
    throw "CompositionKnowledgeRecordTests failed."
}

Write-Host "COMPOSITION KNOWLEDGE RECORD TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionKnowledgeGraphTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionKnowledgeGraphTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionKnowledgeGraphTests build failed."
}

& $TestCompositionKnowledgeGraph

if ($LASTEXITCODE -ne 0) {
    throw "CompositionKnowledgeGraphTests failed."
}

Write-Host "COMPOSITION KNOWLEDGE GRAPH TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionTransitionProfileTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionTransitionProfileTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionTransitionProfileTests build failed."
}

& $TestCompositionTransitionProfile

if ($LASTEXITCODE -ne 0) {
    throw "CompositionTransitionProfileTests failed."
}

Write-Host "COMPOSITION TRANSITION PROFILE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionKnowledgeSnapshotTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionKnowledgeSnapshotTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionKnowledgeSnapshotTests build failed."
}

& $TestCompositionKnowledgeSnapshot

if ($LASTEXITCODE -ne 0) {
    throw "CompositionKnowledgeSnapshotTests failed."
}

Write-Host "COMPOSITION KNOWLEDGE SNAPSHOT TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetSampleTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetSampleTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetSampleTests build failed."
}

& $TestCompositionDatasetSample

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetSampleTests failed."
}

Write-Host "COMPOSITION DATASET SAMPLE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetTests build failed."
}

& $TestCompositionDataset

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetTests failed."
}

Write-Host "COMPOSITION DATASET TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetQualityTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetQualityTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetQualityTests build failed."
}

& $TestCompositionDatasetQuality

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetQualityTests failed."
}

Write-Host "COMPOSITION DATASET QUALITY TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetPartitionTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetPartitionTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetPartitionTests build failed."
}

& $TestCompositionDatasetPartition

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetPartitionTests failed."
}

Write-Host "COMPOSITION DATASET PARTITION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetSchemaTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetSchemaTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetSchemaTests build failed."
}

& $TestCompositionDatasetSchema

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetSchemaTests failed."
}

Write-Host "COMPOSITION DATASET SCHEMA TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetManifestTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetManifestTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetManifestTests build failed."
}

& $TestCompositionDatasetManifest

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetManifestTests failed."
}

Write-Host "COMPOSITION DATASET MANIFEST TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetBatchTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetBatchTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetBatchTests build failed."
}

& $TestCompositionDatasetBatch

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetBatchTests failed."
}

Write-Host "COMPOSITION DATASET BATCH TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetNormalizationTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetNormalizationTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetNormalizationTests build failed."
}

& $TestCompositionDatasetNormalization

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetNormalizationTests failed."
}

Write-Host "COMPOSITION DATASET NORMALIZATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionDatasetPreparedViewTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionDatasetPreparedViewTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetPreparedViewTests build failed."
}

& $TestCompositionDatasetPreparedView

if ($LASTEXITCODE -ne 0) {
    throw "CompositionDatasetPreparedViewTests failed."
}

Write-Host "COMPOSITION DATASET PREPARED VIEW TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionLearningContractTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionLearningContractTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionLearningContractTests build failed."
}

& $TestCompositionLearningContract

if ($LASTEXITCODE -ne 0) {
    throw "CompositionLearningContractTests failed."
}

Write-Host "COMPOSITION LEARNING CONTRACT TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionModelTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionModelTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionModelTests build failed."
}

& $TestCompositionModel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionModelTests failed."
}

Write-Host "COMPOSITION MODEL TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionNeuralModelTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionNeuralModelTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralModelTests build failed."
}

& $TestCompositionNeuralModel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralModelTests failed."
}

Write-Host "COMPOSITION NEURAL MODEL TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionNeuralTrainerTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionNeuralTrainerTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralTrainerTests build failed."
}

& $TestCompositionNeuralTrainer

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralTrainerTests failed."
}

Write-Host "COMPOSITION NEURAL TRAINER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionModelEvaluationTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionModelEvaluationTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionModelEvaluationTests build failed."
}

& $TestCompositionModelEvaluation

if ($LASTEXITCODE -ne 0) {
    throw "CompositionModelEvaluationTests failed."
}

Write-Host "COMPOSITION MODEL EVALUATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMusicalEvaluationTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMusicalEvaluationTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMusicalEvaluationTests build failed."
}

& $TestCompositionMusicalEvaluation

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMusicalEvaluationTests failed."
}

Write-Host "COMPOSITION MUSICAL EVALUATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionInferencePipelineTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionInferencePipelineTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionInferencePipelineTests build failed."
}

& $TestCompositionInferencePipeline

if ($LASTEXITCODE -ne 0) {
    throw "CompositionInferencePipelineTests failed."
}

Write-Host "COMPOSITION INFERENCE PIPELINE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionRuntimeFeatureAdapterTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionRuntimeFeatureAdapterTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRuntimeFeatureAdapterTests build failed."
}

& $TestCompositionRuntimeFeatureAdapter

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRuntimeFeatureAdapterTests failed."
}

Write-Host "COMPOSITION RUNTIME FEATURE ADAPTER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionRuntimeInferenceAdapterTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionRuntimeInferenceAdapterTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRuntimeInferenceAdapterTests build failed."
}

& $TestCompositionRuntimeInferenceAdapter

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRuntimeInferenceAdapterTests failed."
}

Write-Host "COMPOSITION RUNTIME INFERENCE ADAPTER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionRuntimeInferenceServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionRuntimeInferenceServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRuntimeInferenceServiceTests build failed."
}

& $TestCompositionRuntimeInferenceService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRuntimeInferenceServiceTests failed."
}

Write-Host "COMPOSITION RUNTIME INFERENCE SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionNeuralModelArtifactTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionNeuralModelArtifactTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralModelArtifactTests build failed."
}

& $TestCompositionNeuralModelArtifact

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralModelArtifactTests failed."
}

Write-Host "COMPOSITION NEURAL MODEL ARTIFACT TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionNeuralModelRuntimeLoaderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionNeuralModelRuntimeLoaderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralModelRuntimeLoaderTests build failed."
}

& $TestCompositionNeuralModelRuntimeLoader

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralModelRuntimeLoaderTests failed."
}

Write-Host "COMPOSITION NEURAL MODEL RUNTIME LOADER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionNeuralArtifactTrainingServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionNeuralArtifactTrainingServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralArtifactTrainingServiceTests build failed."
}

& $TestCompositionNeuralArtifactTrainingService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralArtifactTrainingServiceTests failed."
}

Write-Host "COMPOSITION NEURAL ARTIFACT TRAINING SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionTrainingCorpusArtifactTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionTrainingCorpusArtifactTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionTrainingCorpusArtifactTests build failed."
}

& $TestCompositionTrainingCorpusArtifact

if ($LASTEXITCODE -ne 0) {
    throw "CompositionTrainingCorpusArtifactTests failed."
}

Write-Host "COMPOSITION TRAINING CORPUS ARTIFACT TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiFileCorpusReaderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiFileCorpusReaderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiFileCorpusReaderTests build failed."
}

& $TestCompositionMidiFileCorpusReader

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiFileCorpusReaderTests failed."
}

Write-Host "COMPOSITION MIDI FILE CORPUS READER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiCorpusAnalysisTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiCorpusAnalysisTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiCorpusAnalysisTests build failed."
}

& $TestCompositionMidiCorpusAnalysis

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiCorpusAnalysisTests failed."
}

Write-Host "COMPOSITION MIDI CORPUS ANALYSIS TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiSectionAnalyzerTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiSectionAnalyzerTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiSectionAnalyzerTests build failed."
}

& $TestCompositionMidiSectionAnalyzer

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiSectionAnalyzerTests failed."
}

Write-Host "COMPOSITION MIDI SECTION ANALYZER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiDatasetFeatureExtractorTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiDatasetFeatureExtractorTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiDatasetFeatureExtractorTests build failed."
}

& $TestCompositionMidiDatasetFeatureExtractor

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiDatasetFeatureExtractorTests failed."
}

Write-Host "COMPOSITION MIDI DATASET FEATURE EXTRACTOR TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiHarmonyTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiHarmonyTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiHarmonyTests build failed."
}

& $TestCompositionMidiHarmony

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiHarmonyTests failed."
}

Write-Host "COMPOSITION MIDI HARMONY TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiMotifAnalysisTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiMotifAnalysisTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiMotifAnalysisTests build failed."
}

& $TestCompositionMidiMotifAnalysis

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiMotifAnalysisTests failed."
}

Write-Host "COMPOSITION MIDI MOTIF ANALYSIS TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiDatasetBuilderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiDatasetBuilderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiDatasetBuilderTests build failed."
}

& $TestCompositionMidiDatasetBuilder

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiDatasetBuilderTests failed."
}

Write-Host "COMPOSITION MIDI DATASET BUILDER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiCorpusDirectoryLoaderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiCorpusDirectoryLoaderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiCorpusDirectoryLoaderTests build failed."
}

& $TestCompositionMidiCorpusDirectoryLoader

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiCorpusDirectoryLoaderTests failed."
}

Write-Host "COMPOSITION MIDI CORPUS DIRECTORY LOADER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiTrainingPipelineTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiTrainingPipelineTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingPipelineTests build failed."
}

& $TestCompositionMidiTrainingPipeline

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingPipelineTests failed."
}

Write-Host "COMPOSITION MIDI TRAINING PIPELINE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiCorpusTrainingServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiCorpusTrainingServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiCorpusTrainingServiceTests build failed."
}

& $TestCompositionMidiCorpusTrainingService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiCorpusTrainingServiceTests failed."
}

Write-Host "COMPOSITION MIDI CORPUS TRAINING SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiTrainingEvaluationServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiTrainingEvaluationServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingEvaluationServiceTests build failed."
}

& $TestCompositionMidiTrainingEvaluationService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingEvaluationServiceTests failed."
}

Write-Host "COMPOSITION MIDI TRAINING EVALUATION SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionNeuralGenerationServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionNeuralGenerationServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralGenerationServiceTests build failed."
}

& $TestCompositionNeuralGenerationService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionNeuralGenerationServiceTests failed."
}

Write-Host "COMPOSITION NEURAL GENERATION SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionAIMidiGenerationServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionAIMidiGenerationServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIMidiGenerationServiceTests build failed."
}

& $TestCompositionAIMidiGenerationService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIMidiGenerationServiceTests failed."
}

Write-Host "COMPOSITION AI MIDI GENERATION SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiTrainingSequenceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiTrainingSequenceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingSequenceTests build failed."
}

& $TestCompositionMidiTrainingSequence

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingSequenceTests failed."
}

Write-Host "COMPOSITION MIDI TRAINING SEQUENCE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiTrainingCorpusArtifactTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiTrainingCorpusArtifactTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingCorpusArtifactTests build failed."
}

& $TestCompositionMidiTrainingCorpusArtifact

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiTrainingCorpusArtifactTests failed."
}

Write-Host "COMPOSITION MIDI TRAINING CORPUS ARTIFACT TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiSequenceWindowTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiSequenceWindowTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiSequenceWindowTests build failed."
}

& $TestCompositionMidiSequenceWindow

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiSequenceWindowTests failed."
}

Write-Host "COMPOSITION MIDI SEQUENCE WINDOW TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionSequenceNeuralTrainerTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionSequenceNeuralTrainerTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceNeuralTrainerTests build failed."
}

& $TestCompositionSequenceNeuralTrainer

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceNeuralTrainerTests failed."
}

Write-Host "COMPOSITION SEQUENCE NEURAL TRAINER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionSequenceNeuralTrainingServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionSequenceNeuralTrainingServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceNeuralTrainingServiceTests build failed."
}

& $TestCompositionSequenceNeuralTrainingService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceNeuralTrainingServiceTests failed."
}

Write-Host "COMPOSITION SEQUENCE NEURAL TRAINING SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionMidiSequenceCorpusBuilderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionMidiSequenceCorpusBuilderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiSequenceCorpusBuilderTests build failed."
}

& $TestCompositionMidiSequenceCorpusBuilder

if ($LASTEXITCODE -ne 0) {
    throw "CompositionMidiSequenceCorpusBuilderTests failed."
}

Write-Host "COMPOSITION MIDI SEQUENCE CORPUS BUILDER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionSequenceMetadataTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionSequenceMetadataTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceMetadataTests build failed."
}

& $TestCompositionSequenceMetadata

if ($LASTEXITCODE -ne 0) {
    throw "CompositionSequenceMetadataTests failed."
}

Write-Host "COMPOSITION SEQUENCE METADATA TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerKnowledgeSampleTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerKnowledgeSampleTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeSampleTests build failed."
}

& $TestCompositionComposerKnowledgeSample

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeSampleTests failed."
}

Write-Host "COMPOSITION COMPOSER KNOWLEDGE REPRESENTATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerKnowledgeCatalogTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerKnowledgeCatalogTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeCatalogTests build failed."
}

& $TestCompositionComposerKnowledgeCatalog

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeCatalogTests failed."
}

Write-Host "COMPOSITION COMPOSER KNOWLEDGE CATALOG TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerKnowledgePartitionTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerKnowledgePartitionTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgePartitionTests build failed."
}

& $TestCompositionComposerKnowledgePartition

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgePartitionTests failed."
}

Write-Host "COMPOSITION COMPOSER KNOWLEDGE PARTITION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerKnowledgeCorpusManifestTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerKnowledgeCorpusManifestTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeCorpusManifestTests build failed."
}

& $TestCompositionComposerKnowledgeCorpusManifest

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeCorpusManifestTests failed."
}

Write-Host "COMPOSITION COMPOSER KNOWLEDGE CORPUS MANIFEST TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerKnowledgeTrainingCorpusTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerKnowledgeTrainingCorpusTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeTrainingCorpusTests build failed."
}

& $TestCompositionComposerKnowledgeTrainingCorpus

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeTrainingCorpusTests failed."
}

Write-Host "COMPOSITION COMPOSER KNOWLEDGE TRAINING CORPUS TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerCorpusSourceManifestTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerCorpusSourceManifestTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerCorpusSourceManifestTests build failed."
}

& $TestCompositionComposerCorpusSourceManifest

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerCorpusSourceManifestTests failed."
}

Write-Host "COMPOSITION COMPOSER CORPUS SOURCE MANIFEST TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionComposerKnowledgeCorpusAssemblyTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionComposerKnowledgeCorpusAssemblyTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeCorpusAssemblyTests build failed."
}

& $TestCompositionComposerKnowledgeCorpusAssembly

if ($LASTEXITCODE -ne 0) {
    throw "CompositionComposerKnowledgeCorpusAssemblyTests failed."
}

Write-Host "COMPOSITION COMPOSER KNOWLEDGE CORPUS ASSEMBLY TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionRealComposerCorpusIntakeTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionRealComposerCorpusIntakeTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRealComposerCorpusIntakeTests build failed."
}

& $TestCompositionRealComposerCorpusIntake

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRealComposerCorpusIntakeTests failed."
}

Write-Host "COMPOSITION REAL COMPOSER CORPUS INTAKE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionRealComposerCorpusPreparationTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionRealComposerCorpusPreparationTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRealComposerCorpusPreparationTests build failed."
}

& $TestCompositionRealComposerCorpusPreparation

if ($LASTEXITCODE -ne 0) {
    throw "CompositionRealComposerCorpusPreparationTests failed."
}

Write-Host "COMPOSITION REAL COMPOSER CORPUS PREPARATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedSequenceNeuralEvaluatorTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedSequenceNeuralEvaluatorTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralEvaluatorTests build failed."
}

& $TestCompositionConditionedSequenceNeuralEvaluator

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralEvaluatorTests failed."
}

Write-Host "COMPOSITION CONDITIONED SEQUENCE NEURAL EVALUATOR TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedSequenceNeuralModelArtifactTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedSequenceNeuralModelArtifactTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralModelArtifactTests build failed."
}

& $TestCompositionConditionedSequenceNeuralModelArtifact

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralModelArtifactTests failed."
}

Write-Host "COMPOSITION CONDITIONED SEQUENCE NEURAL MODEL ARTIFACT TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedSequenceNeuralModelRuntimeLoaderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedSequenceNeuralModelRuntimeLoaderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralModelRuntimeLoaderTests build failed."
}

& $TestCompositionConditionedSequenceNeuralModelRuntimeLoader

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralModelRuntimeLoaderTests failed."
}

Write-Host "COMPOSITION CONDITIONED SEQUENCE NEURAL MODEL RUNTIME LOADER TESTS PASSED" -ForegroundColor Green













Write-Host "Building CompositionConditionedTrainingDatasetTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedTrainingDatasetTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedTrainingDatasetTests build failed."
}

& $TestCompositionConditionedTrainingDataset

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedTrainingDatasetTests failed."
}

Write-Host "COMPOSITION CONDITIONED TRAINING DATASET TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedSequenceNeuralTrainerTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainerTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralTrainerTests build failed."
}

& $TestCompositionConditionedSequenceNeuralTrainer

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralTrainerTests failed."
}

Write-Host "COMPOSITION CONDITIONED SEQUENCE NEURAL TRAINER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedSequenceNeuralTrainingServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedSequenceNeuralTrainingServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralTrainingServiceTests build failed."
}

& $TestCompositionConditionedSequenceNeuralTrainingService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedSequenceNeuralTrainingServiceTests failed."
}

Write-Host "COMPOSITION CONDITIONED SEQUENCE NEURAL TRAINING SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedTrainingRunServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedTrainingRunServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedTrainingRunServiceTests build failed."
}

& $TestCompositionConditionedTrainingRunService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedTrainingRunServiceTests failed."
}

Write-Host "COMPOSITION CONDITIONED TRAINING RUN SERVICE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionConditionedCorpusTrainingServiceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionConditionedCorpusTrainingServiceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedCorpusTrainingServiceTests build failed."
}

& $TestCompositionConditionedCorpusTrainingService

if ($LASTEXITCODE -ne 0) {
    throw "CompositionConditionedCorpusTrainingServiceTests failed."
}

Write-Host "COMPOSITION CONDITIONED CORPUS TRAINING SERVICE TESTS PASSED" -ForegroundColor Green




























Write-Host "Building CompositionAIModelRuntimeProviderTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionAIModelRuntimeProviderTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIModelRuntimeProviderTests build failed."
}

& $TestCompositionAIModelRuntimeProvider

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIModelRuntimeProviderTests failed."
}

Write-Host "COMPOSITION AI MODEL RUNTIME PROVIDER TESTS PASSED" -ForegroundColor Green







Write-Host "Building CompositionAIConstraintAdapterTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionAIConstraintAdapterTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIConstraintAdapterTests build failed."
}

& $TestCompositionAIConstraintAdapter

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIConstraintAdapterTests failed."
}

Write-Host "COMPOSITION AI CONSTRAINT ADAPTER TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionAIEngineBridgeTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionAIEngineBridgeTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIEngineBridgeTests build failed."
}

& $TestCompositionAIEngineBridge

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIEngineBridgeTests failed."
}

Write-Host "COMPOSITION AI ENGINE BRIDGE TESTS PASSED" -ForegroundColor Green

Write-Host "Building MusicalEngineAIGuidanceTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_MusicalEngineAIGuidanceTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "MusicalEngineAIGuidanceTests build failed."
}

& $TestMusicalEngineAIGuidance

if ($LASTEXITCODE -ne 0) {
    throw "MusicalEngineAIGuidanceTests failed."
}

Write-Host "MUSICAL ENGINE AI GUIDANCE TESTS PASSED" -ForegroundColor Green

Write-Host "Building CompositionAIGenerationCoordinatorTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_CompositionAIGenerationCoordinatorTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIGenerationCoordinatorTests build failed."
}

& $TestCompositionAIGenerationCoordinator

if ($LASTEXITCODE -ne 0) {
    throw "CompositionAIGenerationCoordinatorTests failed."
}

Write-Host "COMPOSITION AI GENERATION COORDINATOR TESTS PASSED" -ForegroundColor Green

Write-Host "Building AIGenerationFunctionalTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_AIGenerationFunctionalTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "AIGenerationFunctionalTests build failed."
}

& $TestAIGenerationFunctional

if ($LASTEXITCODE -ne 0) {
    throw "AIGenerationFunctionalTests failed."
}

Write-Host "AI GENERATION FUNCTIONAL TESTS PASSED" -ForegroundColor Green

Write-Host "Building AIPluginRuntimeIntegrationTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_AIPluginRuntimeIntegrationTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "AIPluginRuntimeIntegrationTests build failed."
}

& $TestAIPluginRuntimeIntegration

if ($LASTEXITCODE -ne 0) {
    throw "AIPluginRuntimeIntegrationTests failed."
}

Write-Host "AI PLUGIN RUNTIME INTEGRATION TESTS PASSED" -ForegroundColor Green

Write-Host "Building AIRuntimeGenerationModelTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_AIRuntimeGenerationModelTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "AIRuntimeGenerationModelTests build failed."
}

& $TestAIRuntimeGenerationModel

if ($LASTEXITCODE -ne 0) {
    throw "AIRuntimeGenerationModelTests failed."
}

Write-Host "AI RUNTIME GENERATION MODEL TESTS PASSED" -ForegroundColor Green

Write-Host "Building PluginAIRuntimeModelLoadingTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_PluginAIRuntimeModelLoadingTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "PluginAIRuntimeModelLoadingTests build failed."
}

& $TestPluginAIRuntimeModelLoading

if ($LASTEXITCODE -ne 0) {
    throw "PluginAIRuntimeModelLoadingTests failed."
}

Write-Host "PLUGIN AI RUNTIME MODEL LOADING TESTS PASSED" -ForegroundColor Green

Write-Host "Building PluginAIRuntimeEndToEndTests..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --target "MIDI_GenGX_PluginAIRuntimeEndToEndTests" `
    --config Debug `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "PluginAIRuntimeEndToEndTests build failed."
}

& $TestPluginAIRuntimeEndToEnd

if ($LASTEXITCODE -ne 0) {
    throw "PluginAIRuntimeEndToEndTests failed."
}

Write-Host "PLUGIN AI RUNTIME END-TO-END TESTS PASSED" -ForegroundColor Green




if (Test-Path $UserPluginPath) {
    try {
        Remove-Item -Recurse -Force $UserPluginPath -ErrorAction Stop
    }
    catch {
        throw @"
MIDI-GenGX.vst3 is locked.

Close Ableton Live and any other host using MIDI-GenGX,
then run this script again.
"@
    }
}

New-Item `
    -ItemType Directory `
    -Force `
    -Path $UserVst3Dir | Out-Null

Copy-Item `
    -Recurse `
    -Force `
    $ArtifactPath `
    $UserPluginPath

if (-not (Test-Path $UserPluginPath)) {
    throw "VST3 installation failed: $UserPluginPath"
}

Write-Host "MIDI-GenGX.vst3 installed successfully." -ForegroundColor Green
