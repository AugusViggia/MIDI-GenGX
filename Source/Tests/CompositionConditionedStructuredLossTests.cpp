#include "Music/CompositionConditionedSequenceNeuralTrainer.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\\n';
        std::exit(1);
    }
}

void expectNear(double actual, double expected, const char* message)
{
    expect(std::isfinite(actual) &&
           std::abs(actual - expected) < 1.0e-12,
           message);
}

void testDefaultStructuredWeights()
{
    CompositionConditionedSequenceNeuralTrainingConfig config;

    expectNear(config.lossWeightForFeature(0), 3.0,
               "pitch feature uses pitch loss weight");
    expectNear(config.lossWeightForFeature(1), 1.0,
               "velocity feature uses velocity loss weight");
    expectNear(config.lossWeightForFeature(2), 2.0,
               "duration feature uses timing loss weight");
    expectNear(config.lossWeightForFeature(3), 2.0,
               "delta feature uses timing loss weight");
    expectNear(config.lossWeightForFeature(4), 0.25,
               "auxiliary feature uses auxiliary loss weight");
    expectNear(config.lossWeightForFeature(19), 0.25,
               "channel feature remains auxiliary");
}

void testCustomStructuredWeights()
{
    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.pitchLossWeight = 7.0;
    config.velocityLossWeight = 1.5;
    config.timingLossWeight = 4.0;
    config.auxiliaryLossWeight = 0.1;

    expectNear(config.lossWeightForFeature(0), 7.0,
               "custom pitch weight propagates");
    expectNear(config.lossWeightForFeature(1), 1.5,
               "custom velocity weight propagates");
    expectNear(config.lossWeightForFeature(2), 4.0,
               "custom duration weight propagates");
    expectNear(config.lossWeightForFeature(3), 4.0,
               "custom delta weight propagates");
    expectNear(config.lossWeightForFeature(7), 0.1,
               "custom auxiliary weight propagates");
}
}

int main()
{
    testDefaultStructuredWeights();
    testCustomStructuredWeights();

    std::cout << "MIDI-GenGX Phase 122B structured loss tests passed.\\n";
    return 0;
}
