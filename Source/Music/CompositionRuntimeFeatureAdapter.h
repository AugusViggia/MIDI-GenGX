#pragma once

#include "../Domain/MusicalContext.h"
#include "CompositionDatasetSchema.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionRuntimeFeatures
{
    std::vector<double> globalFeatures;
    std::vector<double> sectionFeatures;

    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionRuntimeFeatureAdapter
{
    static constexpr int version = 1;

    CompositionRuntimeFeatures build(
        const midigengx::domain::MusicalContext& context) const noexcept;
};

} // namespace midigengx::music
