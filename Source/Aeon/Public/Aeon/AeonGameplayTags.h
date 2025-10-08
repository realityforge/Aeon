#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

#define UE_API AEON_API

namespace AeonGameplayTags
{
    // --------------------------------------------------- //
    // Input Tags
    // --------------------------------------------------- //

    // If present in an abilities InputTags then cancel ability once the input is released
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_CancelOnRelease)
    // If present in an abilities InputTags then pressing the input will toggle activation state
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Toggle)

} // namespace AeonGameplayTags

#undef UE_API
