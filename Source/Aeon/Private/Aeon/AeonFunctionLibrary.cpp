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
                AEON_VERBOSE_ALOG("TryActivateAbilityByTag invoked with tag '%s' found no "
                                  "matching AbilitySpecs on actor '%s'",
                                  *AbilityTag.GetTagName().ToString(),
                                  *AbilitySystemComponent->GetOwnerActor()->GetActorNameOrLabel());
                return false;
            }
        }
        else
        {
            AEON_VERBOSE_ALOG("TryActivateAbilityByTag invoked with tag '%s' on invalid actor",
                              *AbilityTag.GetTagName().ToString());
            return false;
        }
    }
    else
    {
        AEON_VERBOSE_ALOG("TryActivateAbilityByTag invoked with empty tag on actor '%s'",
                          *AbilitySystemComponent->GetOwnerActor()->GetActorNameOrLabel());
        return false;
    }
}
