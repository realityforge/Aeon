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

    #include "Animation/AnimMontage.h"
    #include "GameplayAbilitySpec.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AbilitySystem/AeonAbilityTask_PlayMontageAndWaitForEventTestTypes.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonAbilityTaskPlayMontageAndWaitForEventTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestEventTag, "Aeon.Test.AbilityTask.PlayMontageAndWaitForEvent");

    struct FAbilityTaskTestContext
    {
        TUniquePtr<AeonTests::FTestWorld> World;
        TObjectPtr<AAeonAutomationTestActor> Actor{ nullptr };
        TObjectPtr<UAeonAutomationTestGameplayAbility> Ability{ nullptr };
        TObjectPtr<UAnimMontage> Montage{ nullptr };
    };

    UAeonTestPlayMontageAndWaitForEventTask* CreateTaskForTest(FAutomationTestBase& Test,
                                                               FAbilityTaskTestContext& OutContext,
                                                               const bool bOnlyTriggerOnce = false,
                                                               const bool bOnlyMatchExact = true)
    {
        const auto Actor = AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(
            Test,
            OutContext.World,
            TEXT("Ability-system test actor"),
            TEXT("AeonPlayMontageAndWaitForEventTestWorld"));
        if (Test.TestNotNull(TEXT("Ability-system test actor should spawn"), Actor))
        {
            const auto AbilitySystemComponent = AeonTests::GetInitializedAbilitySystemComponent(Test, Actor);
            if (Test.TestNotNull(TEXT("Ability-system test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                const FGameplayAbilitySpec AbilitySpec(UAeonAutomationTestGameplayAbility::StaticClass());
                const auto AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
                const auto GrantedAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
                const auto Ability = GrantedAbilitySpec
                    ? Cast<UAeonAutomationTestGameplayAbility>(GrantedAbilitySpec->GetPrimaryInstance())
                    : nullptr;
                const auto Montage = AeonTests::NewTransientObject<UAnimMontage>(Actor);
                if (Test.TestNotNull(TEXT("Granted ability spec should be created"), GrantedAbilitySpec)
                    && Test.TestNotNull(TEXT("Granted ability instance should be created"), Ability)
                    && Test.TestNotNull(TEXT("Montage should be created"), Montage))
                {
                    Ability->SetIsActiveForTest(true);
                    OutContext.Actor = Actor;
                    OutContext.Ability = Ability;
                    OutContext.Montage = Montage;
                    return UAeonTestPlayMontageAndWaitForEventTask::CreateForTest(Ability,
                                                                                  Montage,
                                                                                  TestEventTag.GetTag(),
                                                                                  bOnlyTriggerOnce,
                                                                                  bOnlyMatchExact);
                }
            }
        }

        return nullptr;
    }

    UAeonTestPlayMontageAndWaitForEventListener*
    BindListener(FAutomationTestBase& Test, UObject* Outer, UAeonTestPlayMontageAndWaitForEventTask* Task)
    {
        const auto Listener = AeonTests::NewTransientObject<UAeonTestPlayMontageAndWaitForEventListener>(Outer);
        if (Test.TestNotNull(TEXT("Listener should be created"), Listener))
        {
            Task->OnCompleted.AddDynamic(Listener, &UAeonTestPlayMontageAndWaitForEventListener::HandleCompleted);
            Task->OnBlendOut.AddDynamic(Listener, &UAeonTestPlayMontageAndWaitForEventListener::HandleBlendOut);
            Task->OnInterrupted.AddDynamic(Listener, &UAeonTestPlayMontageAndWaitForEventListener::HandleInterrupted);
            Task->OnCancelled.AddDynamic(Listener, &UAeonTestPlayMontageAndWaitForEventListener::HandleCancelled);
            Task->EventReceived.AddDynamic(Listener, &UAeonTestPlayMontageAndWaitForEventListener::HandleEventReceived);
        }

        return Listener;
    }
} // namespace AeonAbilityTaskPlayMontageAndWaitForEventTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventActivationOrderTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.ActivationOrder",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventActivationOrderTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        Task->ActivateForTest();

        const auto MontageTask = Task->GetMontageTaskForTest();
        const auto GameplayEventTask = Task->GetGameplayEventTaskForTest();
        const auto& ActivationOrder = Task->GetActivationOrderForTest();

        const bool bResult = TestNotNull(TEXT("Montage sub-task should be created"), MontageTask)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created"), GameplayEventTask)
            && TestEqual(TEXT("Combined task should activate two sub-tasks"), ActivationOrder.Num(), 2)
            && TestEqual(TEXT("Gameplay-event task should activate before the montage task"),
                         ActivationOrder[0],
                         FString(TEXT("GameplayEvent")))
            && TestEqual(TEXT("Montage task should activate second"), ActivationOrder[1], FString(TEXT("Montage")))
            && TestTrue(TEXT("Montage sub-task should activate"), MontageTask->bActivatedForTest)
            && TestTrue(TEXT("Gameplay-event sub-task should activate"), GameplayEventTask->bActivatedForTest)
            && TestEqual(TEXT("Gameplay-event sub-task should preserve the event tag"),
                         GameplayEventTask->ConfiguredEventTagForTest,
                         AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag())
            && TestFalse(TEXT("Gameplay-event sub-task should keep listening for repeated events"),
                         GameplayEventTask->bOnlyTriggerOnceForTest)
            && TestTrue(TEXT("Gameplay-event sub-task should require exact tag matches"),
                        GameplayEventTask->bOnlyMatchExactForTest);
        Task->EndTask();
        return bResult;
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventForwardsGameplayEventsTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.ForwardsGameplayEvents",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventForwardsGameplayEventsTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->ActivateForTest();
        if (TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created"), Task->GetGameplayEventTaskForTest()))
        {
            Task->GetGameplayEventTaskForTest()->BroadcastEventReceivedForTest(
                AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag());

            const bool bResult =
                TestEqual(TEXT("Gameplay-event payload should be forwarded once"), Listener->EventCount, 1)
                && TestEqual(TEXT("Forwarded payload should retain the event tag"),
                             Listener->ReceivedEventTags[0],
                             AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag());
            Task->EndTask();
            return bResult;
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventRepeatedEventTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.RepeatedEvents",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventRepeatedEventTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->ActivateForTest();
        if (TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created"), Task->GetGameplayEventTaskForTest()))
        {
            Task->GetGameplayEventTaskForTest()->BroadcastEventReceivedForTest(
                AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag());
            Task->GetGameplayEventTaskForTest()->BroadcastEventReceivedForTest(
                AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag());

            const bool bResult =
                TestEqual(TEXT("Combined task should forward repeated gameplay events"), Listener->EventCount, 2);
            Task->EndTask();
            return bResult;
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventBlendOutDoesNotEndTaskTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.BlendOutDoesNotEndTask",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventBlendOutDoesNotEndTaskTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->ActivateForTest();
        const auto MontageTask = Task->GetMontageTaskForTest();
        const auto GameplayEventTask = Task->GetGameplayEventTaskForTest();
        if (TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestNotNull(TEXT("Montage sub-task should be created"), MontageTask)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created"), GameplayEventTask))
        {
            MontageTask->BroadcastBlendOutForTest();

            const bool bResult = TestEqual(TEXT("Blend-out should broadcast exactly once"), Listener->BlendOutCount, 1)
                && TestEqual(TEXT("Blend-out should capture one payload"), Listener->BlendOutPayloadTags.Num(), 1)
                && TestEqual(TEXT("Blend-out should forward an empty payload tag"),
                             Listener->BlendOutPayloadTags[0],
                             FGameplayTag::EmptyTag)
                && TestFalse(TEXT("Blend-out should not destroy the montage sub-task"), MontageTask->bDestroyedForTest)
                && TestFalse(TEXT("Blend-out should not destroy the gameplay-event sub-task"),
                             GameplayEventTask->bDestroyedForTest);
            Task->EndTask();
            return bResult;
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventCompletionCleansUpTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.CompletionCleansUp",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventCompletionCleansUpTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->ActivateForTest();
        const auto MontageTask = Task->GetMontageTaskForTest();
        const auto GameplayEventTask = Task->GetGameplayEventTaskForTest();
        if (TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestNotNull(TEXT("Montage sub-task should be created"), MontageTask)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created"), GameplayEventTask))
        {
            MontageTask->BroadcastCompletedForTest();

            const bool bResult = TestEqual(TEXT("Completion should broadcast once"), Listener->CompletedCount, 1)
                && TestEqual(TEXT("Completion should capture one payload"), Listener->CompletedPayloadTags.Num(), 1)
                && TestEqual(TEXT("Completion should forward an empty payload tag"),
                             Listener->CompletedPayloadTags[0],
                             FGameplayTag::EmptyTag)
                && TestTrue(TEXT("Completion should destroy the montage sub-task"), MontageTask->bDestroyedForTest)
                && TestTrue(TEXT("Completion should destroy the gameplay-event sub-task"),
                            GameplayEventTask->bDestroyedForTest);
            Task->EndTask();
            return bResult;
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventExternalCancelTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.ExternalCancel",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventExternalCancelTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->ActivateForTest();
        const auto MontageTask = Task->GetMontageTaskForTest();
        const auto GameplayEventTask = Task->GetGameplayEventTaskForTest();
        if (TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestNotNull(TEXT("Montage sub-task should be created"), MontageTask)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created"), GameplayEventTask))
        {
            Task->ExternalCancelForTest();

            const bool bResult = TestEqual(TEXT("External cancel should forward to the montage sub-task"),
                                           MontageTask->ExternalCancelCountForTest,
                                           1)
                && TestEqual(TEXT("External cancel should broadcast cancellation once"), Listener->CancelledCount, 1)
                && TestEqual(TEXT("External cancel should capture one payload"),
                             Listener->CancelledPayloadTags.Num(),
                             1)
                && TestEqual(TEXT("External cancel should forward an empty payload tag"),
                             Listener->CancelledPayloadTags[0],
                             FGameplayTag::EmptyTag)
                && TestTrue(TEXT("External cancel should destroy the montage sub-task"), MontageTask->bDestroyedForTest)
                && TestTrue(TEXT("External cancel should destroy the gameplay-event sub-task"),
                            GameplayEventTask->bDestroyedForTest);
            Task->EndTask();
            return bResult;
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventMontageCreationFailureTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.MontageCreationFailureCancels",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventMontageCreationFailureTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->SetForceMontageTaskNullForTest(true);
        Task->ActivateForTest();

        const bool bResult = TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestEqual(TEXT("Montage creation failure should broadcast cancellation"), Listener->CancelledCount, 1)
            && TestEqual(TEXT("Montage creation failure should capture one payload"),
                         Listener->CancelledPayloadTags.Num(),
                         1)
            && TestEqual(TEXT("Montage creation failure should forward an empty payload tag"),
                         Listener->CancelledPayloadTags[0],
                         FGameplayTag::EmptyTag)
            && TestNotNull(TEXT("Gameplay-event sub-task should be created before montage creation fails"),
                           Task->GetGameplayEventTaskForTest())
            && TestTrue(TEXT("Creation failure should clean up the gameplay-event sub-task"),
                        Task->GetGameplayEventTaskForTest()->bDestroyedForTest);
        Task->EndTask();
        return bResult;
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventGameplayEventCreationFailureTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.GameplayEventCreationFailureCancels",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventGameplayEventCreationFailureTest::RunTest(const FString&)
{
    AeonAbilityTaskPlayMontageAndWaitForEventTests::FAbilityTaskTestContext Context;
    if (const auto Task = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateTaskForTest(*this, Context))
    {
        const auto Listener =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::BindListener(*this, Context.Ability, Task);
        Task->SetForceGameplayEventTaskNullForTest(true);
        Task->ActivateForTest();

        const bool bResult = TestNotNull(TEXT("Listener should bind to the combined task"), Listener)
            && TestEqual(TEXT("Gameplay-event creation failure should broadcast cancellation"),
                         Listener->CancelledCount,
                         1)
            && TestEqual(TEXT("Gameplay-event creation failure should capture one payload"),
                         Listener->CancelledPayloadTags.Num(),
                         1)
            && TestEqual(TEXT("Gameplay-event creation failure should forward an empty payload tag"),
                         Listener->CancelledPayloadTags[0],
                         FGameplayTag::EmptyTag)
            && TestNull(TEXT("Gameplay-event sub-task should not be created when forced null"),
                        Task->GetGameplayEventTaskForTest())
            && TestNotNull(TEXT("Montage sub-task should still be created before activation fails"),
                           Task->GetMontageTaskForTest())
            && TestTrue(TEXT("Activation failure should clean up the montage sub-task"),
                        Task->GetMontageTaskForTest()->bDestroyedForTest);
        Task->EndTask();
        return bResult;
    }

    return false;
}

#endif
