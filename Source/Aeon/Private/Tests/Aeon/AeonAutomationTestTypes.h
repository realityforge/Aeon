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

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Aeon/AbilitySystem/AeonAbilitySystemComponent.h"
#include "Aeon/AbilitySystem/AeonAttributeSetBase.h"
#include "Aeon/AbilitySystem/AeonGameplayAbility.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayEffect.h"
#include "Misc/AutomationTest.h"
#include "AeonAutomationTestTypes.generated.h"

UCLASS(NotBlueprintable)
class UAeonAutomationTestAbilitySystemComponent final : public UAeonAbilitySystemComponent
{
    GENERATED_BODY()

public:
    int32 GetReplicatedLooseTagCountForTest(const FGameplayTag& Tag) const
    {
        return GetReplicatedLooseTags().TagMap.FindRef(Tag);
    }
};

UCLASS(NotBlueprintable)
class AAeonAutomationTestActor final : public AActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AAeonAutomationTestActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        AbilitySystemComponent =
            CreateDefaultSubobject<UAeonAutomationTestAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    }

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UAeonAutomationTestAbilitySystemComponent* GetAeonAbilitySystemComponent() const { return AbilitySystemComponent; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAeonAutomationTestAbilitySystemComponent> AbilitySystemComponent;
};

UCLASS(NotBlueprintable)
class UAeonAutomationTestGameplayAbility final : public UAeonGameplayAbility
{
    GENERATED_BODY()

public:
    inline static int32 ActivationCount = 0;
    inline static int32 EndCount = 0;
    inline static bool bLastEndWasCancelled = false;

    explicit UAeonAutomationTestGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    }

    static void ResetMutableStateForTest()
    {
        auto* Ability = GetMutableDefault<ThisClass>();
        check(Ability);

        Ability->SetAssetTagsForTest(FGameplayTagContainer());
        Ability->ActivationRequiredTags.Reset();
        Ability->ActivationBlockedTags.Reset();
        Ability->SourceRequiredTags.Reset();
        Ability->SourceBlockedTags.Reset();
        Ability->TargetRequiredTags.Reset();
        Ability->TargetBlockedTags.Reset();
        Ability->NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
        Ability->InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    }

    static void ResetCounters()
    {
        ActivationCount = 0;
        EndCount = 0;
        bLastEndWasCancelled = false;
        ResetMutableStateForTest();
    }

    void SetAssetTagsForTest(const FGameplayTagContainer& Tags)
    {
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        AbilityTags = Tags;
        PRAGMA_ENABLE_DEPRECATION_WARNINGS

        auto& AssetTags = EditorGetAssetTags();
        AssetTags.Reset();
        AssetTags.AppendTags(Tags);
    }

    void SetActivationRequiredTagsForTest(const FGameplayTagContainer& Tags) { ActivationRequiredTags = Tags; }

    void SetActivationBlockedTagsForTest(const FGameplayTagContainer& Tags) { ActivationBlockedTags = Tags; }

    void SetSourceRequiredTagsForTest(const FGameplayTagContainer& Tags) { SourceRequiredTags = Tags; }

    void SetSourceBlockedTagsForTest(const FGameplayTagContainer& Tags) { SourceBlockedTags = Tags; }

    void SetTargetRequiredTagsForTest(const FGameplayTagContainer& Tags) { TargetRequiredTags = Tags; }

    void SetTargetBlockedTagsForTest(const FGameplayTagContainer& Tags) { TargetBlockedTags = Tags; }

    void SetNetExecutionPolicyForTest(const EGameplayAbilityNetExecutionPolicy::Type Policy)
    {
        NetExecutionPolicy = Policy;
    }

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override
    {
        Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
        ActivationCount++;
    }

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                            const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo,
                            const bool bReplicateEndAbility,
                            const bool bWasCancelled) override
    {
        EndCount++;
        bLastEndWasCancelled = bWasCancelled;
        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
};

UCLASS(NotBlueprintable)
class UAeonAutomationTestGameplayEffect final : public UGameplayEffect
{
    GENERATED_BODY()

public:
    explicit UAeonAutomationTestGameplayEffect(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        DurationPolicy = EGameplayEffectDurationType::Infinite;
    }
};

UCLASS(NotBlueprintable)
class UAeonAutomationTestAttributeSet final : public UAeonAttributeSetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Test")
    FGameplayAttributeData Resource;
    ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Resource)

    UPROPERTY(BlueprintReadOnly, Category = "Test")
    FGameplayAttributeData MaxResource;
    ATTRIBUTE_ACCESSORS_BASIC(ThisClass, MaxResource)

    void AddTagIfValueAboveThresholdForTest(const FGameplayAttribute& Attribute,
                                            const FGameplayTag& Tag,
                                            const float Value,
                                            const float ThresholdValue,
                                            const bool bReplicated = false,
                                            const float ErrorTolerance = 0.001f) const
    {
        AddTagIfValueAboveThreshold(Attribute, Tag, Value, ThresholdValue, bReplicated, ErrorTolerance);
    }

    void AddTagIfValueBelowThresholdForTest(const FGameplayAttribute& Attribute,
                                            const FGameplayTag& Tag,
                                            const float Value,
                                            const float ThresholdValue,
                                            const bool bReplicated = false,
                                            const float ErrorTolerance = 0.001f) const
    {
        AddTagIfValueBelowThreshold(Attribute, Tag, Value, ThresholdValue, bReplicated, ErrorTolerance);
    }

    void AdjustAttributeAfterMaxValueChangesForTest(const FGameplayAttribute& Attribute,
                                                    const float OldMaxValue,
                                                    const float NewMaxValue,
                                                    const float ErrorTolerance = 0.001f) const
    {
        AdjustAttributeAfterMaxValueChanges(Attribute, OldMaxValue, NewMaxValue, ErrorTolerance);
    }
};

UCLASS(NotBlueprintable)
class AAeonAutomationTestGameState final : public AGameStateBase, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AAeonAutomationTestGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        AbilitySystemComponent =
            CreateDefaultSubobject<UAeonAutomationTestAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    }

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UAeonAutomationTestAbilitySystemComponent* GetAeonAbilitySystemComponent() const { return AbilitySystemComponent; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAeonAutomationTestAbilitySystemComponent> AbilitySystemComponent;
};

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
namespace AeonTests
{
    inline UAeonAutomationTestAbilitySystemComponent*
    GetInitializedAbilitySystemComponent(FAutomationTestBase& Test, AAeonAutomationTestActor* Actor)
    {
        if (Test.TestNotNull(TEXT("Ability-system test actor should spawn"), Actor))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            if (Test.TestNotNull(TEXT("Ability-system test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                AbilitySystemComponent->InitAbilityActorInfo(Actor, Actor);
                return AbilitySystemComponent;
            }
        }

        return nullptr;
    }

    template <typename TAttributeSet>
    TAttributeSet* CreateOwnedAttributeSet(FAutomationTestBase& Test,
                                           UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent)
    {
        if (Test.TestNotNull(TEXT("Ability-system test actor should expose an Aeon ASC"), AbilitySystemComponent))
        {
            const auto OwnerActor = AbilitySystemComponent->GetOwner();
            if (Test.TestNotNull(TEXT("Ability-system test actor should own the Aeon ASC"), OwnerActor))
            {
                const auto AttributeSet = NewObject<TAttributeSet>(OwnerActor, NAME_None, RF_Transient);
                if (Test.TestNotNull(TEXT("Test attribute set should be created"), AttributeSet))
                {
                    AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
                    return AttributeSet;
                }
            }
        }

        return nullptr;
    }
} // namespace AeonTests
#endif
