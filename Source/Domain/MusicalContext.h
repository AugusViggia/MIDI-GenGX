#pragma once

#include "Character.h"
#include "Key.h"
#include "MusicalParameters.h"
#include "Role.h"
#include "Scale.h"
#include "Style.h"

#include <string>

namespace midigengx::domain
{

struct MusicalContext
{
    Key key = Key::C;
    Scale scale{ScaleType::Minor};

    Role role = Role::Lead;
    Style style = Style::ProgressiveHouse;
    Character character = Character::Emotional;

    MusicalParameters parameters{};

    std::string prompt;

    void normalize()
    {
        parameters.clamp();
    }
};

} // namespace midigengx::domain
