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

#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "AeonGameplayAbility.generated.h"

class UAeonAbilitySystemComponent;

/**
 * Strategy that the GameplayAbility uses to activate.
 */
UENUM(BlueprintType)
enum class EAeonAbilityActivationPolicy : uint8
{
    /**
     * Activate the ability when it is triggered.
     *
     * Triggered abilities are typically triggered on input.
     */
    OnTriggered,

    /** Activate the ability when it is given to the Avatar. */
    OnGiven
};

/**
 * The base GameplayAbility class used by Aeon.
 */
UCLASS(Abstract)
class AEON_API UAeonGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Aeon|Ability")
    EAeonAbilityActivationPolicy AbilityActivationPolicy{ EAeonAbilityActivationPolicy::OnTriggered };

protected:
    //~Begin UGameplayAbility Interface
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                            const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo,
                            bool bReplicateEndAbility,
                            bool bWasCancelled) override;
    //~End UGameplayAbility Interface

    /**
     * Return the AeonAbilitySystemComponent from the associated Actor.
     *
     * @return The AeonAbilitySystemComponent from the associated actor.
     */
    UFUNCTION(BlueprintPure, Category = "Aeon|Ability")
    UAeonAbilitySystemComponent* GetAeonAbilitySystemComponentFromActorInfo() const;

public:
    //~Begin UGameplayAbility Interface
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

    // Method has been overridden to incorporate (optional) support for AeonAbilityTagRelationshipMapping integration
    virtual bool
    DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent,
                                      const FGameplayTagContainer* SourceTags = nullptr,
                                      const FGameplayTagContainer* TargetTags = nullptr,
                                      FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    //~End UGameplayAbility Interface
};
