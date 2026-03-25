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
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "BehaviorTree/BehaviorTree.h"
    #include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
    #include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
    #include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
    #include "BehaviorTree/BlackboardComponent.h"
    #include "Misc/AutomationTest.h"
    #include "Tests/AeonAI/AeonAIAutomationTestTypes.h"

namespace BTServiceAttributesToBlackboardTests
{
    constexpr TCHAR SourceActorKeyName[] = TEXT("SourceActor");
    constexpr TCHAR ResourceFloatKeyName[] = TEXT("ResourceFloat");
    constexpr TCHAR ResourceIntKeyName[] = TEXT("ResourceInt");
    constexpr TCHAR ResourceBoolKeyName[] = TEXT("ResourceBool");

    UBlackboardData* CreateBlackboardData()
    {
        const auto BlackboardData = AeonAITests::CreateBlackboardData();
        AeonAITests::AddObjectBlackboardKey(BlackboardData, SourceActorKeyName, AActor::StaticClass());
        AeonAITests::AddBlackboardKey<UBlackboardKeyType_Float>(BlackboardData, ResourceFloatKeyName);
        AeonAITests::AddBlackboardKey<UBlackboardKeyType_Int>(BlackboardData, ResourceIntKeyName);
        AeonAITests::AddBlackboardKey<UBlackboardKeyType_Bool>(BlackboardData, ResourceBoolKeyName);
        return BlackboardData;
    }

    UBehaviorTree* CreateBehaviorTree(UBlackboardData* BlackboardData)
    {
        const auto BehaviorTree = AeonAITests::NewTransientObject<UBehaviorTree>();
        BehaviorTree->BlackboardAsset = BlackboardData;
        return BehaviorTree;
    }

    TArray<FAttributeToBlackboardMapping> CreateMappings()
    {
        TArray<FAttributeToBlackboardMapping> Mappings;

        FAttributeToBlackboardMapping FloatMapping;
        FloatMapping.Attribute = UAeonAITestAttributeSet::GetResourceAttribute();
        FloatMapping.BlackboardKey.SelectedKeyName = ResourceFloatKeyName;
        FloatMapping.BlackboardKey.SelectedKeyType = UBlackboardKeyType_Float::StaticClass();
        Mappings.Add(FloatMapping);

        FAttributeToBlackboardMapping IntMapping;
        IntMapping.Attribute = UAeonAITestAttributeSet::GetResourceAttribute();
        IntMapping.BlackboardKey.SelectedKeyName = ResourceIntKeyName;
        IntMapping.BlackboardKey.SelectedKeyType = UBlackboardKeyType_Int::StaticClass();
        Mappings.Add(IntMapping);

        FAttributeToBlackboardMapping BoolMapping;
        BoolMapping.Attribute = UAeonAITestAttributeSet::GetResourceAttribute();
        BoolMapping.BlackboardKey.SelectedKeyName = ResourceBoolKeyName;
        BoolMapping.BlackboardKey.SelectedKeyType = UBlackboardKeyType_Bool::StaticClass();
        Mappings.Add(BoolMapping);

        return Mappings;
    }

    bool CreateServiceContext(FAutomationTestBase& Test,
                              TUniquePtr<AeonAITests::FTestWorld>& OutWorld,
                              AAeonAITestAIController*& OutController,
                              AAeonAITestPawn*& OutPawn,
                              UBehaviorTreeComponent*& OutBehaviorTreeComponent,
                              UAeonAITestAbilitySystemComponent*& OutAbilitySystemComponent,
                              UBlackboardData*& OutBlackboardData)
    {
        OutBlackboardData = CreateBlackboardData();
        if (Test.TestNotNull(TEXT("Blackboard data should be created"), OutBlackboardData))
        {
            if (AeonAITests::CreateAIContext(Test,
                                             OutWorld,
                                             OutController,
                                             OutPawn,
                                             OutBehaviorTreeComponent,
                                             OutBlackboardData,
                                             TEXT("AeonAIAttributesToBlackboardServiceTestWorld")))
            {
                OutAbilitySystemComponent = OutPawn->GetAeonAbilitySystemComponent();
                if (Test.TestNotNull(TEXT("AI pawn should expose an Aeon ASC"), OutAbilitySystemComponent))
                {
                    UObject* AttributeSetOuter = OutAbilitySystemComponent;
                    const auto OwnerActor = OutAbilitySystemComponent->GetOwnerActor();
                    if (!OwnerActor)
                    {
                        const auto AttributeSet =
                            NewObject<UAeonAITestAttributeSet>(AttributeSetOuter, NAME_None, RF_Transient);
                        if (Test.TestNotNull(TEXT("Service test attribute set should be created"), AttributeSet))
                        {
                            OutAbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    UBTService_AttributesToBlackboardTestNode* CreateConfiguredService(FAutomationTestBase& Test,
                                                                       UBlackboardData* BlackboardData)
    {
        const auto Service = AeonAITests::NewTransientObject<UBTService_AttributesToBlackboardTestNode>();
        if (Test.TestNotNull(TEXT("Attributes-to-blackboard service should be created"), Service))
        {
            Service->SetSourceActorKeyForTest(SourceActorKeyName);
            auto Mappings = CreateMappings();
            for (auto& Mapping : Mappings)
            {
                Mapping.BlackboardKey.ResolveSelectedKey(*BlackboardData);
            }

            Service->SetMappingsForTest(Mappings);
            const auto BehaviorTree = CreateBehaviorTree(BlackboardData);
            if (Test.TestNotNull(TEXT("Behavior tree should be created"), BehaviorTree))
            {
                Service->InitializeFromAsset(*BehaviorTree);
                return Service;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
} // namespace BTServiceAttributesToBlackboardTests

struct FBTService_AttributesToBlackboardTestAccess
{
    static void PollRefresh(UBTService_AttributesToBlackboard& Service, UBehaviorTreeComponent& OwnerComp)
    {
        Service.CacheProperties(OwnerComp);
        Service.WriteAllValues();
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTServiceAttributesToBlackboardWritesFloatIntAndBoolMappingsTest,
                                 "Aeon.AI.Services.AttributesToBlackboard.WritesFloatIntAndBoolMappings",
                                 AeonAITests::AutomationTestFlags)

bool FBTServiceAttributesToBlackboardWritesFloatIntAndBoolMappingsTest::RunTest(const FString&)
{
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    UAeonAITestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UBlackboardData* BlackboardData{ nullptr };
    if (BTServiceAttributesToBlackboardTests::CreateServiceContext(*this,
                                                                   World,
                                                                   Controller,
                                                                   Pawn,
                                                                   BehaviorTreeComponent,
                                                                   AbilitySystemComponent,
                                                                   BlackboardData))
    {
        AbilitySystemComponent->SetNumericAttributeBase(UAeonAITestAttributeSet::GetResourceAttribute(), 42.75f);
        Controller->GetBlackboardComponent()->SetValueAsObject(BTServiceAttributesToBlackboardTests::SourceActorKeyName,
                                                               Pawn);

        if (const auto Service = BTServiceAttributesToBlackboardTests::CreateConfiguredService(*this, BlackboardData))
        {
            Service->OnBecomeRelevant(*BehaviorTreeComponent, nullptr);
            const auto BlackboardComponent = Controller->GetBlackboardComponent();
            const auto ResourceBoolKeyId =
                BlackboardComponent->GetKeyID(BTServiceAttributesToBlackboardTests::ResourceBoolKeyName);
            return TestEqual(
                       TEXT("Float blackboard key should mirror the attribute value"),
                       BlackboardComponent->GetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName),
                       42.75f)
                && TestEqual(
                       TEXT("Int blackboard key should floor the attribute value"),
                       BlackboardComponent->GetValueAsInt(BTServiceAttributesToBlackboardTests::ResourceIntKeyName),
                       42)
                && TestNotEqual(TEXT("Bool blackboard key should resolve to a valid key"),
                                static_cast<int32>(ResourceBoolKeyId),
                                static_cast<int32>(FBlackboard::InvalidKey))
                && TestTrue(TEXT("Bool blackboard key should be true for a non-zero attribute value"),
                            BlackboardComponent->GetValue<UBlackboardKeyType_Bool>(ResourceBoolKeyId));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTServiceAttributesToBlackboardClearMissingPolicyZeroesMappedKeysTest,
                                 "Aeon.AI.Services.AttributesToBlackboard.ClearMissingPolicyZeroesMappedKeys",
                                 AeonAITests::AutomationTestFlags)

bool FBTServiceAttributesToBlackboardClearMissingPolicyZeroesMappedKeysTest::RunTest(const FString&)
{
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    UAeonAITestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UBlackboardData* BlackboardData{ nullptr };
    if (BTServiceAttributesToBlackboardTests::CreateServiceContext(*this,
                                                                   World,
                                                                   Controller,
                                                                   Pawn,
                                                                   BehaviorTreeComponent,
                                                                   AbilitySystemComponent,
                                                                   BlackboardData))
    {
        const auto BlackboardComponent = Controller->GetBlackboardComponent();
        BlackboardComponent->SetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName, 9.f);
        BlackboardComponent->SetValueAsInt(BTServiceAttributesToBlackboardTests::ResourceIntKeyName, 9);
        BlackboardComponent->SetValueAsBool(BTServiceAttributesToBlackboardTests::ResourceBoolKeyName, true);

        if (const auto Service = BTServiceAttributesToBlackboardTests::CreateConfiguredService(*this, BlackboardData))
        {
            Service->SetMissingPolicyForTest(EAttributesToBlackboard_MissingPolicy::Clear);
            Service->OnBecomeRelevant(*BehaviorTreeComponent, nullptr);

            return TestEqual(
                       TEXT("Clear missing policy should zero float keys"),
                       BlackboardComponent->GetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName),
                       0.f)
                && TestEqual(
                       TEXT("Clear missing policy should zero int keys"),
                       BlackboardComponent->GetValueAsInt(BTServiceAttributesToBlackboardTests::ResourceIntKeyName),
                       0)
                && TestFalse(
                       TEXT("Clear missing policy should clear bool keys"),
                       BlackboardComponent->GetValueAsBool(BTServiceAttributesToBlackboardTests::ResourceBoolKeyName));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTServiceAttributesToBlackboardLeaveMissingPolicyPreservesMappedKeysTest,
                                 "Aeon.AI.Services.AttributesToBlackboard.LeaveMissingPolicyPreservesMappedKeys",
                                 AeonAITests::AutomationTestFlags)

bool FBTServiceAttributesToBlackboardLeaveMissingPolicyPreservesMappedKeysTest::RunTest(const FString&)
{
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    UAeonAITestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UBlackboardData* BlackboardData{ nullptr };
    if (BTServiceAttributesToBlackboardTests::CreateServiceContext(*this,
                                                                   World,
                                                                   Controller,
                                                                   Pawn,
                                                                   BehaviorTreeComponent,
                                                                   AbilitySystemComponent,
                                                                   BlackboardData))
    {
        const auto BlackboardComponent = Controller->GetBlackboardComponent();
        BlackboardComponent->SetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName, 7.f);
        BlackboardComponent->SetValueAsInt(BTServiceAttributesToBlackboardTests::ResourceIntKeyName, 7);
        BlackboardComponent->SetValueAsBool(BTServiceAttributesToBlackboardTests::ResourceBoolKeyName, true);

        if (const auto Service = BTServiceAttributesToBlackboardTests::CreateConfiguredService(*this, BlackboardData))
        {
            Service->SetMissingPolicyForTest(EAttributesToBlackboard_MissingPolicy::Leave);
            Service->OnBecomeRelevant(*BehaviorTreeComponent, nullptr);

            return TestEqual(
                       TEXT("Leave missing policy should preserve float keys"),
                       BlackboardComponent->GetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName),
                       7.f)
                && TestEqual(
                       TEXT("Leave missing policy should preserve int keys"),
                       BlackboardComponent->GetValueAsInt(BTServiceAttributesToBlackboardTests::ResourceIntKeyName),
                       7)
                && TestTrue(
                       TEXT("Leave missing policy should preserve bool keys"),
                       BlackboardComponent->GetValueAsBool(BTServiceAttributesToBlackboardTests::ResourceBoolKeyName));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTServiceAttributesToBlackboardPollStrategyUpdatesOnTickTest,
                                 "Aeon.AI.Services.AttributesToBlackboard.PollStrategyUpdatesOnTick",
                                 AeonAITests::AutomationTestFlags)

bool FBTServiceAttributesToBlackboardPollStrategyUpdatesOnTickTest::RunTest(const FString&)
{
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    UAeonAITestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UBlackboardData* BlackboardData{ nullptr };
    if (BTServiceAttributesToBlackboardTests::CreateServiceContext(*this,
                                                                   World,
                                                                   Controller,
                                                                   Pawn,
                                                                   BehaviorTreeComponent,
                                                                   AbilitySystemComponent,
                                                                   BlackboardData))
    {
        AbilitySystemComponent->SetNumericAttributeBase(UAeonAITestAttributeSet::GetResourceAttribute(), 10.f);
        Controller->GetBlackboardComponent()->SetValueAsObject(BTServiceAttributesToBlackboardTests::SourceActorKeyName,
                                                               Pawn);

        if (const auto Service = BTServiceAttributesToBlackboardTests::CreateConfiguredService(*this, BlackboardData))
        {
            Service->SetUpdateStrategyForTest(EAttributesToBlackboard_UpdateStrategy::Poll);
            Service->OnBecomeRelevant(*BehaviorTreeComponent, nullptr);
            AbilitySystemComponent->SetNumericAttributeBase(UAeonAITestAttributeSet::GetResourceAttribute(), 15.f);
            FBTService_AttributesToBlackboardTestAccess::PollRefresh(*Service, *BehaviorTreeComponent);

            return TestEqual(TEXT("Poll strategy should refresh the blackboard value on tick"),
                             Controller->GetBlackboardComponent()->GetValueAsFloat(
                                 BTServiceAttributesToBlackboardTests::ResourceFloatKeyName),
                             15.f);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTServiceAttributesToBlackboardSubscribeStrategyRespondsToAttributeChangesTest,
                                 "Aeon.AI.Services.AttributesToBlackboard.SubscribeStrategyRespondsToAttributeChanges",
                                 AeonAITests::AutomationTestFlags)

bool FBTServiceAttributesToBlackboardSubscribeStrategyRespondsToAttributeChangesTest::RunTest(const FString&)
{
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    UAeonAITestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UBlackboardData* BlackboardData{ nullptr };
    if (BTServiceAttributesToBlackboardTests::CreateServiceContext(*this,
                                                                   World,
                                                                   Controller,
                                                                   Pawn,
                                                                   BehaviorTreeComponent,
                                                                   AbilitySystemComponent,
                                                                   BlackboardData))
    {
        AbilitySystemComponent->SetNumericAttributeBase(UAeonAITestAttributeSet::GetResourceAttribute(), 20.f);
        Controller->GetBlackboardComponent()->SetValueAsObject(BTServiceAttributesToBlackboardTests::SourceActorKeyName,
                                                               Pawn);

        if (const auto Service = BTServiceAttributesToBlackboardTests::CreateConfiguredService(*this, BlackboardData))
        {
            Service->SetUpdateStrategyForTest(EAttributesToBlackboard_UpdateStrategy::Subscribe);
            Service->SetOnlyUpdateWhenChangedForTest(true);
            Service->SetChangeToleranceForTest(0.5f);
            Service->OnBecomeRelevant(*BehaviorTreeComponent, nullptr);
            AbilitySystemComponent->SetNumericAttributeBase(UAeonAITestAttributeSet::GetResourceAttribute(), 20.25f);
            const auto BlackboardComponent = Controller->GetBlackboardComponent();
            const float ValueWithinTolerance =
                BlackboardComponent->GetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName);
            AbilitySystemComponent->SetNumericAttributeBase(UAeonAITestAttributeSet::GetResourceAttribute(), 21.f);

            return TestEqual(TEXT("Values within the configured tolerance should not churn the blackboard entry"),
                             ValueWithinTolerance,
                             20.f)
                && TestEqual(
                       TEXT("Subscribe strategy should update the blackboard after a meaningful attribute change"),
                       BlackboardComponent->GetValueAsFloat(BTServiceAttributesToBlackboardTests::ResourceFloatKeyName),
                       21.f);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif
