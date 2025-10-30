/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

    // --------------------------------------------------- //
    // Ability Trait Tags
    // --------------------------------------------------- //

    // Abilities with this tag will be activated immediately on being granted to the AbilitySystemComponent.
    UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Aeon_Ability_Trait_ActivateOnGiven)

} // namespace AeonGameplayTags

#undef UE_API
