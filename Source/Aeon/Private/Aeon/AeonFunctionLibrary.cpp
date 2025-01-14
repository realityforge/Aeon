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
#include "Aeon/AeonFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aeon/Logging.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AeonFunctionLibrary)

bool UAeonFunctionLibrary::TryActivateRandomSingleAbilityByTag(UAbilitySystemComponent* AbilitySystemComponent,
                                                               const FGameplayTag AbilityTag)
{
    if (ensureAlways(AbilityTag.IsValid()))
    {
        if (ensureAlways(AbilitySystemComponent))
        {
            TArray<FGameplayAbilitySpec*> AbilitySpecs;
            const auto GameplayTags = AbilityTag.GetSingleTagContainer();
            AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTags, AbilitySpecs);
            if (!AbilitySpecs.IsEmpty())
            {
                const auto& AbilitySpec = AbilitySpecs[FMath::RandRange(0, AbilitySpecs.Num() - 1)];
                check(AbilitySpec);
                return !AbilitySpec->IsActive() ? AbilitySystemComponent->TryActivateAbility(AbilitySpec->Handle)
                                                : false;
            }
            else
            {
                AEON_VERY_VERBOSE_ALOG("TryActivateRandomSingleAbilityByTag invoked with tag '%s' found no "
                                       "matching 'Activatable' GameplayAbilitySpecs on actor '%s'. Either no such "
                                       "Ability exists or the matching abilities are blocked or missing required tags",
                                       *AbilityTag.GetTagName().ToString(),
                                       *AbilitySystemComponent->GetOwnerActor()->GetActorNameOrLabel());
                return false;
            }
        }
        else
        {
            AEON_VERBOSE_ALOG("TryActivateRandomSingleAbilityByTag invoked with tag '%s' on invalid actor",
                              *AbilityTag.GetTagName().ToString());
            return false;
        }
    }
    else
    {
        AEON_VERBOSE_ALOG("TryActivateRandomSingleAbilityByTag invoked with empty tag on actor '%s'",
                          *AbilitySystemComponent->GetOwnerActor()->GetActorNameOrLabel());
        return false;
    }
}
