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
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemInterface.h"
#include "Aeon/AbilitySystem/AeonAbilitySystemComponent.h"
#include "Aeon/AbilitySystem/AeonAbilityTask_PlayMontageAndWaitForEvent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoreMinimal.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"
#include "AeonAbilityTask_PlayMontageAndWaitForEventTestTypes.generated.h"

UCLASS(NotBlueprintable)
class UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener final : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void HandleCancelled(const FGameplayTag InEventTag, const FGameplayEventData& InPayload)
    {
        ++CancelledCount;
        LastCancelledEventTag = InEventTag;
        LastCancelledPayloadEventTag = InPayload.EventTag;
    }

    UFUNCTION()
    void HandleInterrupted(const FGameplayTag InEventTag, const FGameplayEventData& InPayload)
    {
        ++InterruptedCount;
        LastInterruptedEventTag = InEventTag;
        LastInterruptedPayloadEventTag = InPayload.EventTag;
    }

    UFUNCTION()
    void HandleEventReceived(const FGameplayTag InEventTag, const FGameplayEventData& InPayload)
    {
        ++EventReceivedCount;
        LastEventReceivedTag = InEventTag;
        LastEventReceivedPayloadEventTag = InPayload.EventTag;
    }

    int32 CancelledCount{ 0 };
    int32 InterruptedCount{ 0 };
    int32 EventReceivedCount{ 0 };
    FGameplayTag LastCancelledEventTag{ FGameplayTag::EmptyTag };
    FGameplayTag LastCancelledPayloadEventTag{ FGameplayTag::EmptyTag };
    FGameplayTag LastInterruptedEventTag{ FGameplayTag::EmptyTag };
    FGameplayTag LastInterruptedPayloadEventTag{ FGameplayTag::EmptyTag };
    FGameplayTag LastEventReceivedTag{ FGameplayTag::EmptyTag };
    FGameplayTag LastEventReceivedPayloadEventTag{ FGameplayTag::EmptyTag };
};

UCLASS(NotBlueprintable)
class UAeonAbilityTaskPlayMontageAndWaitForEventTestAbilitySystemComponent final : public UAeonAbilitySystemComponent
{
    GENERATED_BODY()

public:
    void SeedAnimatingMontageForTest(UGameplayAbility* const InAbility, UAnimMontage* const InMontage)
    {
        LocalAnimMontageInfo.AnimatingAbility = InAbility;
        LocalAnimMontageInfo.AnimMontage = InMontage;
        if (InAbility)
        {
            InAbility->SetCurrentMontage(InMontage);
        }
    }
};

UCLASS(NotBlueprintable)
class AAeonAbilityTaskPlayMontageAndWaitForEventCharacter final : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AAeonAbilityTaskPlayMontageAndWaitForEventCharacter(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        AbilitySystemComponent =
            CreateDefaultSubobject<UAeonAbilityTaskPlayMontageAndWaitForEventTestAbilitySystemComponent>(
                TEXT("AbilitySystemComponent"));

        static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(
            TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP"));
        if (SkeletalMeshAsset.Succeeded())
        {
            GetMesh()->SetSkeletalMeshAsset(SkeletalMeshAsset.Object);
        }

        static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBlueprintClass(
            TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_AnimBlueprint"));
        if (AnimBlueprintClass.Succeeded())
        {
            GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            GetMesh()->SetAnimInstanceClass(AnimBlueprintClass.Class);
        }
    }

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UAeonAbilityTaskPlayMontageAndWaitForEventTestAbilitySystemComponent* GetAeonAbilitySystemComponent() const
    {
        return AbilitySystemComponent;
    }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAeonAbilityTaskPlayMontageAndWaitForEventTestAbilitySystemComponent> AbilitySystemComponent;
};

UCLASS(NotBlueprintable)
class UAeonTestPlayMontageAndWaitForEventTask final : public UAeonAbilityTask_PlayMontageAndWaitForEvent
{
    GENERATED_BODY()

public:
    static UAeonTestPlayMontageAndWaitForEventTask* CreateForTest(UGameplayAbility* const OwningAbility,
                                                                  UAnimMontage* const MontageToPlay,
                                                                  const float InAnimRootMotionTranslationScale = 1.f)
    {
        const auto Task = NewAbilityTask<ThisClass>(OwningAbility, NAME_None);
        Task->InitializeTask(MontageToPlay,
                             1.f,
                             NAME_None,
                             true,
                             InAnimRootMotionTranslationScale,
                             FGameplayTag::EmptyTag,
                             false,
                             true);
        return Task;
    }

    bool StopPlayingMontageForTest() { return StopPlayingMontage(); }
};
