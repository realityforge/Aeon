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

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemInterface.h"
#include "Aeon/AbilitySystem/AeonAbilitySystemComponent.h"
#include "Aeon/AbilitySystem/AeonAttributeSetBase.h"
#include "AeonAnimation/AnimNotifies/AnimNotifyState_ApplyGameplayEffect.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayEffect.h"
#include "ScalableFloat.h"
#include "Tests/AeonAnimation/AeonAnimationAutomationTestHelpers.h"
#include "AeonAnimationAutomationTestTypes.generated.h"

UCLASS(NotBlueprintable)
class UAeonAnimationTestAbilitySystemComponent final : public UAeonAbilitySystemComponent
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class AAeonAnimationTestActor final : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AAeonAnimationTestActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
        RootComponent = SkeletalMeshComponent;
        AbilitySystemComponent =
            CreateDefaultSubobject<UAeonAnimationTestAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    }

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    USkeletalMeshComponent* GetSkeletalMeshComponentForTest() const { return SkeletalMeshComponent; }

    UAeonAnimationTestAbilitySystemComponent* GetAeonAbilitySystemComponent() const { return AbilitySystemComponent; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAeonAnimationTestAbilitySystemComponent> AbilitySystemComponent;
};

UCLASS(NotBlueprintable)
class UAeonAnimationTestGameplayEffectAttributeSet final : public UAeonAttributeSetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Test")
    FGameplayAttributeData EffectValue;
    ATTRIBUTE_ACCESSORS_BASIC(ThisClass, EffectValue)
};

UCLASS(NotBlueprintable, Abstract)
class UAeonAnimationTestGameplayEffectBase : public UGameplayEffect
{
    GENERATED_BODY()

protected:
    void ConfigureModifier(const float Magnitude, const EGameplayEffectDurationType InDurationPolicy)
    {
        DurationPolicy = InDurationPolicy;
        if (EGameplayEffectDurationType::HasDuration == InDurationPolicy)
        {
            DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.25f));
        }

        auto& Modifier = Modifiers.AddDefaulted_GetRef();
        Modifier.Attribute = UAeonAnimationTestGameplayEffectAttributeSet::GetEffectValueAttribute();
        Modifier.ModifierOp = EGameplayModOp::Additive;
        Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Magnitude));
    }
};

UCLASS(NotBlueprintable)
class UAeonAnimationTestInfiniteGameplayEffect final : public UAeonAnimationTestGameplayEffectBase
{
    GENERATED_BODY()

public:
    UAeonAnimationTestInfiniteGameplayEffect() { ConfigureModifier(-0.65f, EGameplayEffectDurationType::Infinite); }
};

UCLASS(NotBlueprintable)
class UAeonAnimationTestDurationGameplayEffect final : public UAeonAnimationTestGameplayEffectBase
{
    GENERATED_BODY()

public:
    UAeonAnimationTestDurationGameplayEffect() { ConfigureModifier(-0.65f, EGameplayEffectDurationType::HasDuration); }
};

UCLASS(NotBlueprintable)
class UAeonAnimationTestApplyFailureNotify final : public UAnimNotifyState_ApplyGameplayEffect
{
    GENERATED_BODY()

protected:
    virtual FActiveGameplayEffectHandle ApplyEffect(UAbilitySystemComponent* AbilitySystemComponent,
                                                    const UGameplayEffect* Effect) const override
    {
        return FActiveGameplayEffectHandle();
    }
};
