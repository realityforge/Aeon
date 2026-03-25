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

#include "AIController.h"
#include "AbilitySystemInterface.h"
#include "Aeon/AbilitySystem/AeonAbilitySystemComponent.h"
#include "Aeon/AbilitySystem/AeonAttributeSetBase.h"
#include "Aeon/AbilitySystem/AeonGameplayAbility.h"
#include "AeonAI/Decorators/BTDecorator_HasMatchingGameplayTag.h"
#include "AeonAI/Services/BTService_AttributesToBlackboard.h"
#include "AeonAI/Tasks/BTTask_ActivateAbilityByTag.h"
#include "AeonAI/Tasks/BTTask_CancelAbilitiesByTag.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameFramework/Pawn.h"
#include "Tests/AeonAI/AeonAIAutomationTestHelpers.h"
#include "AeonAIAutomationTestTypes.generated.h"

UCLASS(NotBlueprintable)
class UAeonAITestAbilitySystemComponent final : public UAeonAbilitySystemComponent
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class UAeonAITestAttributeSet final : public UAeonAttributeSetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Test")
    FGameplayAttributeData Resource;
    ATTRIBUTE_ACCESSORS_BASIC(ThisClass, Resource)
};

UCLASS(NotBlueprintable)
class UAeonAITestGameplayAbility final : public UAeonGameplayAbility
{
    GENERATED_BODY()

public:
    inline static int32 ActivationCount = 0;
    inline static int32 EndCount = 0;

    explicit UAeonAITestGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    }

    static void ResetCounters()
    {
        ActivationCount = 0;
        EndCount = 0;
    }

    void SetAssetTagsForTest(const FGameplayTagContainer& Tags)
    {
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        AbilityTags = Tags;
        PRAGMA_ENABLE_DEPRECATION_WARNINGS
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
        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
};

UCLASS(NotBlueprintable)
class AAeonAITestPawn final : public APawn, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AAeonAITestPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
    {
        AbilitySystemComponent =
            CreateDefaultSubobject<UAeonAITestAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    }

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

    UAeonAITestAbilitySystemComponent* GetAeonAbilitySystemComponent() const { return AbilitySystemComponent; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAeonAITestAbilitySystemComponent> AbilitySystemComponent;
};

UCLASS(NotBlueprintable)
class AAeonAITestPlainPawn final : public APawn
{
    GENERATED_BODY()
};

UCLASS(NotBlueprintable)
class AAeonAITestAIController final : public AAIController
{
    GENERATED_BODY()

public:
    bool InitializeBlackboardForTest(UBlackboardData* BlackboardAsset)
    {
        UBlackboardComponent* BlackboardComponent{ nullptr };
        return UseBlackboard(BlackboardAsset, BlackboardComponent) && nullptr != BlackboardComponent;
    }

    UBehaviorTreeComponent* CreateBehaviorTreeComponentForTest()
    {
        if (!BehaviorTreeComponent)
        {
            BehaviorTreeComponent = NewObject<UBehaviorTreeComponent>(this, TEXT("BehaviorTreeComponent"));
            BrainComponent = BehaviorTreeComponent;
            BehaviorTreeComponent->RegisterComponent();
            if (const auto BlackboardComponent = GetBlackboardComponent())
            {
                BehaviorTreeComponent->CacheBlackboardComponent(BlackboardComponent);
            }
        }

        return BehaviorTreeComponent;
    }

    UBehaviorTreeComponent* GetBehaviorTreeComponentForTest() const { return BehaviorTreeComponent; }

private:
    UPROPERTY(Transient)
    TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};

UCLASS(NotBlueprintable)
class UBTDecorator_HasMatchingGameplayTagTestNode final : public UBTDecorator_HasMatchingGameplayTag
{
    GENERATED_BODY()

public:
    void SetBlackboardKeyForTest(const FName KeyName) { BlackboardKey.SelectedKeyName = KeyName; }

    void SetGameplayTagForTest(const FGameplayTag Tag) { GameplayTag = Tag; }

    bool EvaluateConditionForTest(UBehaviorTreeComponent& OwnerComp) const
    {
        return CalculateRawConditionValue(OwnerComp, nullptr);
    }

#if WITH_EDITOR
    FString GetDescriptionForTest() const { return GetStaticDescription(); }
#endif
};

UCLASS(NotBlueprintable)
class UBTTask_ActivateAbilityByTagTestNode final : public UBTTask_ActivateAbilityByTag
{
    GENERATED_BODY()

public:
    void SetAbilityTagSourceForTest(const EAeonAbilityTagSource Source) { AbilityTagSource = Source; }

    void SetAbilityTagForTest(const FGameplayTag Tag) { AbilityTag = Tag; }

    void SetAbilityTagKeyForTest(const FName KeyName) { AbilityTagKey.SelectedKeyName = KeyName; }

    FGameplayTag ResolveAbilityTagForTest(UBehaviorTreeComponent& OwnerComp) { return ResolveAbilityTag(OwnerComp); }
};

UCLASS(NotBlueprintable)
class UBTTask_CancelAbilitiesByTagTestNode final : public UBTTask_CancelAbilitiesByTag
{
    GENERATED_BODY()

public:
    void SetAbilityTagSourceForTest(const EAeonAbilityTagSource Source) { AbilityTagSource = Source; }

    void SetAbilityTagForTest(const FGameplayTag Tag) { AbilityTag = Tag; }

    void SetAbilityTagKeyForTest(const FName KeyName) { AbilityTagKey.SelectedKeyName = KeyName; }

    void SetFailIfNoAbilitiesCancelledForTest(const bool bFail) { bFailIfNoAbilitiesCancelled = bFail; }

    FGameplayTag ResolveAbilityTagForTest(UBehaviorTreeComponent& OwnerComp) { return ResolveAbilityTag(OwnerComp); }
};

UCLASS(NotBlueprintable)
class UBTService_AttributesToBlackboardTestNode final : public UBTService_AttributesToBlackboard
{
    GENERATED_BODY()

public:
    void SetSourceActorKeyForTest(const FName KeyName) { SourceActorKey.SelectedKeyName = KeyName; }

    void SetMappingsForTest(const TArray<FAttributeToBlackboardMapping>& InMappings) { Mappings = InMappings; }

    void SetUpdateStrategyForTest(const EAttributesToBlackboard_UpdateStrategy Strategy) { UpdateStrategy = Strategy; }

    void SetMissingPolicyForTest(const EAttributesToBlackboard_MissingPolicy Policy) { MissingPolicy = Policy; }

    void SetOnlyUpdateWhenChangedForTest(const bool bOnlyWhenChanged) { bOnlyUpdateWhenChanged = bOnlyWhenChanged; }

    void SetChangeToleranceForTest(const float InTolerance) { ChangeTolerance = InTolerance; }
};

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
namespace AeonAITests
{
    inline UBlackboardData* CreateBlackboardData()
    {
        return NewTransientObject<UBlackboardData>();
    }

    template <typename TKeyType>
    TKeyType* AddBlackboardKey(UBlackboardData* BlackboardData, const FName KeyName)
    {
        check(BlackboardData);
        const auto KeyType = BlackboardData->UpdatePersistentKey<TKeyType>(KeyName);
        BlackboardData->UpdateKeyIDs();
        return KeyType;
    }

    inline UBlackboardKeyType_Object*
    AddObjectBlackboardKey(UBlackboardData* BlackboardData, const FName KeyName, UClass* BaseClass)
    {
        const auto KeyType = AddBlackboardKey<UBlackboardKeyType_Object>(BlackboardData, KeyName);
        check(KeyType);
        KeyType->BaseClass = BaseClass;
        BlackboardData->UpdateKeyIDs();
        return KeyType;
    }

    inline bool CreateAIContext(FAutomationTestBase& Test,
                                TUniquePtr<FTestWorld>& OutWorld,
                                AAeonAITestAIController*& OutController,
                                AAeonAITestPawn*& OutPawn,
                                UBehaviorTreeComponent*& OutBehaviorTreeComponent,
                                UBlackboardData* BlackboardAsset,
                                const TCHAR* WorldName)
    {
        if (CreateTestWorld(Test, OutWorld, WorldName))
        {
            OutWorld->Get()->CreateAISystem();
            OutController = OutWorld->SpawnActor<AAeonAITestAIController>();
            OutPawn = OutWorld->SpawnActor<AAeonAITestPawn>();

            if (!Test.TestNotNull(TEXT("AI controller should spawn"), OutController)
                || !Test.TestNotNull(TEXT("AI pawn should spawn"), OutPawn))
            {
                return false;
            }

            OutController->Possess(OutPawn);
            const auto AbilitySystemComponent = OutPawn->GetAeonAbilitySystemComponent();
            if (!Test.TestNotNull(TEXT("AI pawn should expose an Aeon ASC"), AbilitySystemComponent))
            {
                return false;
            }

            AbilitySystemComponent->InitAbilityActorInfo(OutPawn, OutPawn);

            if (BlackboardAsset
                && !Test.TestTrue(TEXT("AI controller should initialize the blackboard"),
                                  OutController->InitializeBlackboardForTest(BlackboardAsset)))
            {
                return false;
            }

            OutBehaviorTreeComponent = OutController->CreateBehaviorTreeComponentForTest();
            return Test.TestNotNull(TEXT("Behavior tree component should be created"), OutBehaviorTreeComponent);
        }

        return false;
    }
} // namespace AeonAITests
#endif
