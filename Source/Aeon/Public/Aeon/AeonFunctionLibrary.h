#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AeonFunctionLibrary.generated.h"

class UAbilitySystemComponent;
struct FGameplayTag;

/**
 * Blueprint function library exposing useful functions used within Aeon.
 */
UCLASS()
class AEON_API UAeonFunctionLibrary : public UBlueprintFunctionLibrary

{
    GENERATED_BODY()

public:
    /**
     * Activate the ability identified by the AbilityTag on specified AbilitySystemComponent.
     * - If there are multiple abilities that are identified by the tag, then an ability is randomly selected.
     * - The function will then check costs and requirements before activating the ability.
     *
     * @param AbilitySystemComponent the AbilitySystemComponent.
     * @param AbilityTag the Tag identifying the ability.
     * @return true if the Ability is present and attempt to activate occured, but it may return false positives due to
     * failure later in activation.
     */
    UFUNCTION(BlueprintCallable, Category = "Aeon|Ability")
    static bool TryActivateRandomSingleAbilityByTag(UAbilitySystemComponent* AbilitySystemComponent,
                                                    const FGameplayTag AbilityTag);
};
