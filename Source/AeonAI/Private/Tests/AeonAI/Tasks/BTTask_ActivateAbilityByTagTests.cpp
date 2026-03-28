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

    #include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"
    #include "BehaviorTree/BlackboardComponent.h"
    #include "GameplayAbilitySpec.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/AeonAI/AeonAIAutomationTestTypes.h"

namespace BTTaskActivateAbilityByTagTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityTag, "Aeon.Test.AI.Tasks.ActivateAbility");

    constexpr TCHAR AbilityTagKeyName[] = TEXT("AbilityTag");

    UBlackboardData* CreateBlackboardData()
    {
        const auto BlackboardData = AeonAITests::CreateBlackboardData();
        AeonAITests::AddBlackboardKey<UBlackboardKeyType_Name>(BlackboardData, AbilityTagKeyName);
        return BlackboardData;
    }

    bool GrantTaggedAbility(FAutomationTestBase& Test, UAeonAITestAbilitySystemComponent* AbilitySystemComponent)
    {
        const FGameplayTag AbilityTag = TestAbilityTag.GetTag();
        if (Test.TestNotNull(TEXT("AI test ASC should be valid"), AbilitySystemComponent))
        {
            UAeonAITestGameplayAbility::ResetCounters();
            FGameplayTagContainer AbilityTagContainer;
            AbilityTagContainer.AddTag(AbilityTag);
            GetMutableDefault<UAeonAITestGameplayAbility>()->SetAssetTagsForTest(AbilityTagContainer);

            FGameplayAbilitySpec Spec(UAeonAITestGameplayAbility::StaticClass());
            Spec.GetDynamicSpecSourceTags().AddTag(AbilityTag);
            const auto Handle = AbilitySystemComponent->GiveAbility(Spec);
            return Test.TestTrue(TEXT("AI test ability should be granted"), Handle.IsValid());
        }
        else
        {
            return false;
        }
    }
} // namespace BTTaskActivateAbilityByTagTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskActivateAbilityByTagPropertySourceActivatesMatchingAbilityTest,
                                 "Aeon.AI.Tasks.ActivateAbilityByTag.PropertySourceActivatesMatchingAbility",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskActivateAbilityByTagPropertySourceActivatesMatchingAbilityTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskActivateAbilityByTagTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    if (AeonAITests::CreateAIContext(*this,
                                     World,
                                     Controller,
                                     Pawn,
                                     BehaviorTreeComponent,
                                     nullptr,
                                     TEXT("AeonAITaskActivateAbilityByTagPropertySourceTestWorld"))
        && BTTaskActivateAbilityByTagTests::GrantTaggedAbility(*this, Pawn->GetAeonAbilitySystemComponent()))
    {
        const auto Task = AeonAITests::NewTransientObject<UBTTask_ActivateAbilityByTagTestNode>();
        Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromProperty);
        Task->SetAbilityTagForTest(AbilityTag);

        const auto Result = Task->ExecuteTask(*BehaviorTreeComponent, nullptr);
        return TestEqual(TEXT("Property-sourced ability tag should activate the matching ability"),
                         Result,
                         EBTNodeResult::Succeeded)
            && TestEqual(TEXT("Property-sourced execution should activate exactly one ability"),
                         UAeonAITestGameplayAbility::ActivationCount,
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskActivateAbilityByTagBlackboardSourceResolvesConfiguredTagTest,
                                 "Aeon.AI.Tasks.ActivateAbilityByTag.BlackboardSourceResolvesConfiguredTag",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskActivateAbilityByTagBlackboardSourceResolvesConfiguredTagTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskActivateAbilityByTagTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    if (AeonAITests::CreateAIContext(*this,
                                     World,
                                     Controller,
                                     Pawn,
                                     BehaviorTreeComponent,
                                     BTTaskActivateAbilityByTagTests::CreateBlackboardData(),
                                     TEXT("AeonAITaskActivateAbilityByTagBlackboardSourceTestWorld")))
    {
        Controller->GetBlackboardComponent()->SetValueAsName(BTTaskActivateAbilityByTagTests::AbilityTagKeyName,
                                                             AbilityTag.GetTagName());

        const auto Task = AeonAITests::NewTransientObject<UBTTask_ActivateAbilityByTagTestNode>();
        Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromBlackboard);
        Task->SetAbilityTagKeyForTest(BTTaskActivateAbilityByTagTests::AbilityTagKeyName);

        return TestTrue(TEXT("Blackboard-sourced ability tag should resolve to the configured gameplay tag"),
                        Task->ResolveAbilityTagForTest(*BehaviorTreeComponent) == AbilityTag);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskActivateAbilityByTagFailsForInvalidBlackboardValueTest,
                                 "Aeon.AI.Tasks.ActivateAbilityByTag.FailsForInvalidBlackboardValue",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskActivateAbilityByTagFailsForInvalidBlackboardValueTest::RunTest(const FString&)
{
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    if (AeonAITests::CreateAIContext(*this,
                                     World,
                                     Controller,
                                     Pawn,
                                     BehaviorTreeComponent,
                                     BTTaskActivateAbilityByTagTests::CreateBlackboardData(),
                                     TEXT("AeonAITaskActivateAbilityByTagInvalidBlackboardValueTestWorld")))
    {
        AddExpectedError(TEXT("ActivateAbilityByTag task failed: Blackboard key"),
                         EAutomationExpectedErrorFlags::Contains,
                         1);
        AddExpectedError(TEXT("ActivateAbilityByTag task failed: Resolved tag is invalid"),
                         EAutomationExpectedErrorFlags::Contains,
                         1);

        const auto Task = AeonAITests::NewTransientObject<UBTTask_ActivateAbilityByTagTestNode>();
        Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromBlackboard);
        Task->SetAbilityTagKeyForTest(BTTaskActivateAbilityByTagTests::AbilityTagKeyName);

        return TestEqual(TEXT("Missing blackboard value should fail the activation task"),
                         Task->ExecuteTask(*BehaviorTreeComponent, nullptr),
                         EBTNodeResult::Failed);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskActivateAbilityByTagFailsWithoutAbilitySystemComponentTest,
                                 "Aeon.AI.Tasks.ActivateAbilityByTag.FailsWithoutAbilitySystemComponent",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskActivateAbilityByTagFailsWithoutAbilitySystemComponentTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskActivateAbilityByTagTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    if (!AeonAITests::CreateTestWorld(*this, World, TEXT("AeonAITaskActivateAbilityByTagMissingASCTestWorld")))
    {
        return false;
    }

    World->Get()->CreateAISystem();
    const auto Controller = World->SpawnActor<AAeonAITestAIController>();
    const auto Pawn = World->SpawnActor<AAeonAITestPlainPawn>();
    if (!TestNotNull(TEXT("AI controller should spawn"), Controller)
        || !TestNotNull(TEXT("Plain pawn should spawn"), Pawn))
    {
        return false;
    }

    Controller->Possess(Pawn);
    const auto BehaviorTreeComponent = Controller->CreateBehaviorTreeComponentForTest();
    if (!TestNotNull(TEXT("Behavior tree component should be created"), BehaviorTreeComponent))
    {
        return false;
    }

    const auto Task = AeonAITests::NewTransientObject<UBTTask_ActivateAbilityByTagTestNode>();
    Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromProperty);
    Task->SetAbilityTagForTest(AbilityTag);
    AddExpectedError(TEXT("ActivateAbilityByTag task failed: no AbilitySystemComponent found on Pawn"),
                     EAutomationExpectedErrorFlags::Contains,
                     1);

    return TestEqual(TEXT("Task should fail when the controlled pawn has no ASC"),
                     Task->ExecuteTask(*BehaviorTreeComponent, nullptr),
                     EBTNodeResult::Failed);
}

#endif
