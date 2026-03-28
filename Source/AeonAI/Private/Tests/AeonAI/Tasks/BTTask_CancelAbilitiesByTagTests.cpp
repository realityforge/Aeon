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

namespace BTTaskCancelAbilitiesByTagTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityTag, "Aeon.Test.AI.Tasks.CancelAbility");

    constexpr TCHAR AbilityTagKeyName[] = TEXT("AbilityTag");

    UBlackboardData* CreateBlackboardData()
    {
        const auto BlackboardData = AeonAITests::CreateBlackboardData();
        AeonAITests::AddBlackboardKey<UBlackboardKeyType_Name>(BlackboardData, AbilityTagKeyName);
        return BlackboardData;
    }

    bool GrantAndActivateTaggedAbility(FAutomationTestBase& Test,
                                       UAeonAITestAbilitySystemComponent* AbilitySystemComponent,
                                       FGameplayAbilitySpecHandle& OutHandle)
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
            OutHandle = AbilitySystemComponent->GiveAbility(Spec);
            if (Test.TestTrue(TEXT("AI test ability should be granted"), OutHandle.IsValid()))
            {
                return Test.TestTrue(TEXT("AI test ability should activate successfully"),
                                     AbilitySystemComponent->TryActivateAbility(OutHandle));
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
} // namespace BTTaskCancelAbilitiesByTagTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskCancelAbilitiesByTagCancelsActiveMatchingAbilityTest,
                                 "Aeon.AI.Tasks.CancelAbilitiesByTag.CancelsActiveMatchingAbility",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskCancelAbilitiesByTagCancelsActiveMatchingAbilityTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskCancelAbilitiesByTagTests::TestAbilityTag.GetTag();
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
                                     TEXT("AeonAITaskCancelAbilitiesByTagPropertySourceTestWorld")))
    {
        FGameplayAbilitySpecHandle Handle;
        if (BTTaskCancelAbilitiesByTagTests::GrantAndActivateTaggedAbility(*this,
                                                                           Pawn->GetAeonAbilitySystemComponent(),
                                                                           Handle))
        {
            const auto Task = AeonAITests::NewTransientObject<UBTTask_CancelAbilitiesByTagTestNode>();
            Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromProperty);
            Task->SetAbilityTagForTest(AbilityTag);

            const auto Result = Task->ExecuteTask(*BehaviorTreeComponent, nullptr);
            const auto AbilitySpec = Pawn->GetAeonAbilitySystemComponent()->FindAbilitySpecFromHandle(Handle);
            return TestEqual(TEXT("Property-sourced cancel task should succeed when it cancels an active ability"),
                             Result,
                             EBTNodeResult::Succeeded)
                && TestNotNull(TEXT("Granted ability spec should still exist after cancellation"), AbilitySpec)
                && TestFalse(TEXT("Granted ability should no longer be active after cancellation"),
                             AbilitySpec->IsActive());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskCancelAbilitiesByTagBlackboardSourceResolvesConfiguredTagTest,
                                 "Aeon.AI.Tasks.CancelAbilitiesByTag.BlackboardSourceResolvesConfiguredTag",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskCancelAbilitiesByTagBlackboardSourceResolvesConfiguredTagTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskCancelAbilitiesByTagTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    if (AeonAITests::CreateAIContext(*this,
                                     World,
                                     Controller,
                                     Pawn,
                                     BehaviorTreeComponent,
                                     BTTaskCancelAbilitiesByTagTests::CreateBlackboardData(),
                                     TEXT("AeonAITaskCancelAbilitiesByTagBlackboardSourceTestWorld")))
    {
        Controller->GetBlackboardComponent()->SetValueAsName(BTTaskCancelAbilitiesByTagTests::AbilityTagKeyName,
                                                             AbilityTag.GetTagName());

        const auto Task = AeonAITests::NewTransientObject<UBTTask_CancelAbilitiesByTagTestNode>();
        Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromBlackboard);
        Task->SetAbilityTagKeyForTest(BTTaskCancelAbilitiesByTagTests::AbilityTagKeyName);

        return TestTrue(TEXT("Blackboard-sourced cancel task should resolve the configured gameplay tag"),
                        Task->ResolveAbilityTagForTest(*BehaviorTreeComponent) == AbilityTag);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskCancelAbilitiesByTagHonorsFailIfNoAbilitiesCancelledTest,
                                 "Aeon.AI.Tasks.CancelAbilitiesByTag.HonorsFailIfNoAbilitiesCancelled",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskCancelAbilitiesByTagHonorsFailIfNoAbilitiesCancelledTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskCancelAbilitiesByTagTests::TestAbilityTag.GetTag();
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
                                     TEXT("AeonAITaskCancelAbilitiesByTagFailIfNoneTestWorld")))
    {
        const auto Task = AeonAITests::NewTransientObject<UBTTask_CancelAbilitiesByTagTestNode>();
        Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromProperty);
        Task->SetAbilityTagForTest(AbilityTag);
        Task->SetFailIfNoAbilitiesCancelledForTest(true);

        return TestEqual(TEXT("Cancel task should fail when configured to fail if no active abilities are cancelled"),
                         Task->ExecuteTask(*BehaviorTreeComponent, nullptr),
                         EBTNodeResult::Failed);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTTaskCancelAbilitiesByTagFailsWithoutAbilitySystemComponentTest,
                                 "Aeon.AI.Tasks.CancelAbilitiesByTag.FailsWithoutAbilitySystemComponent",
                                 AeonAITests::AutomationTestFlags)

bool FBTTaskCancelAbilitiesByTagFailsWithoutAbilitySystemComponentTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = BTTaskCancelAbilitiesByTagTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    if (AeonAITests::CreateTestWorld(*this, World, TEXT("AeonAITaskCancelAbilitiesByTagMissingASCTestWorld")))
    {
        World->Get()->CreateAISystem();
        const auto Controller = World->SpawnActor<AAeonAITestAIController>();
        const auto Pawn = World->SpawnActor<AAeonAITestPlainPawn>();
        if (TestNotNull(TEXT("AI controller should spawn"), Controller)
            && TestNotNull(TEXT("Plain pawn should spawn"), Pawn))
        {
            Controller->Possess(Pawn);
            const auto BehaviorTreeComponent = Controller->CreateBehaviorTreeComponentForTest();
            if (TestNotNull(TEXT("Behavior tree component should be created"), BehaviorTreeComponent))
            {
                const auto Task = AeonAITests::NewTransientObject<UBTTask_CancelAbilitiesByTagTestNode>();
                Task->SetAbilityTagSourceForTest(EAeonAbilityTagSource::FromProperty);
                Task->SetAbilityTagForTest(AbilityTag);
                AddExpectedError(TEXT("CancelAbilitiesByTag task failed: no AbilitySystemComponent found on Pawn"),
                                 EAutomationExpectedErrorFlags::Contains,
                                 1);

                return TestEqual(TEXT("Cancel task should fail when the controlled pawn has no ASC"),
                                 Task->ExecuteTask(*BehaviorTreeComponent, nullptr),
                                 EBTNodeResult::Failed);
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

#endif
