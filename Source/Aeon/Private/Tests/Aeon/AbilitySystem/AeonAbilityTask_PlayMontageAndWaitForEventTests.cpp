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

    #include "Abilities/GameplayAbility.h"
    #include "Aeon/AbilitySystem/AeonAbilityTask_PlayMontageAndWaitForEvent.h"
    #include "Animation/AnimMontage.h"
    #include "Animation/AnimSequenceBase.h"
    #include "GameplayAbilitySpec.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AbilitySystem/AeonAbilityTask_PlayMontageAndWaitForEventTestTypes.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonAbilityTaskPlayMontageAndWaitForEventTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestEventTag, "Aeon.Test.AbilityTask.PlayMontageAndWaitForEvent.Event");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestChildEventTag, "Aeon.Test.AbilityTask.PlayMontageAndWaitForEvent.Event.Child");

    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor = AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(
                Test,
                OutWorld,
                TEXT("Ability-task test actor"),
                TEXT("AeonAbilityTaskPlayMontageAndWaitForEventTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    AAeonAbilityTaskPlayMontageAndWaitForEventCharacter*
    CreateAbilitySystemCharacter(FAutomationTestBase& Test, TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Character =
                AeonTests::SpawnActorInFreshTestWorld<AAeonAbilityTaskPlayMontageAndWaitForEventCharacter>(
                    Test,
                    OutWorld,
                    TEXT("Ability-task test character"),
                    TEXT("AeonAbilityTaskPlayMontageAndWaitForEventCharacterWorld")))
        {
            const auto AbilitySystemComponent = Character->GetAeonAbilitySystemComponent();
            if (Test.TestNotNull(TEXT("Ability-task test character should expose an Aeon ASC"), AbilitySystemComponent))
            {
                AbilitySystemComponent->InitAbilityActorInfo(Character, Character);
                return Character;
            }
        }

        return nullptr;
    }

    UAnimSequenceBase* LoadIdleAnimationForTest()
    {
        return LoadObject<UAnimSequenceBase>(
            nullptr,
            TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/Tutorial_Idle.Tutorial_Idle"));
    }

    UAnimMontage* CreateDynamicMontageForTest(FAutomationTestBase& Test, UAnimSequenceBase* const Animation)
    {
        const auto Montage = Animation ? UAnimMontage::CreateSlotAnimationAsDynamicMontage(Animation,
                                                                                           FName(TEXT("DefaultSlot")),
                                                                                           0.1f,
                                                                                           0.1f,
                                                                                           1.f,
                                                                                           1,
                                                                                           -1.f,
                                                                                           0.f)
                                       : nullptr;
        return Test.TestNotNull(TEXT("Dynamic montage should be created from the test animation"), Montage) ? Montage
                                                                                                            : nullptr;
    }

    UGameplayAbility* ActivateAbility(FAutomationTestBase& Test, UAbilitySystemComponent* AbilitySystemComponent)
    {
        UAeonAutomationTestGameplayAbility::ResetCounters();

        const FGameplayAbilitySpec Spec(UAeonAutomationTestGameplayAbility::StaticClass());
        const auto Handle = AbilitySystemComponent->GiveAbility(Spec);
        const auto AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
        if (Test.TestNotNull(TEXT("Granted test ability should exist"), AbilitySpec)
            && Test.TestTrue(TEXT("Granted test ability should activate"),
                             AbilitySystemComponent->TryActivateAbility(Handle))
            && Test.TestTrue(TEXT("Granted test ability should become active"), AbilitySpec->IsActive()))
        {
            return Test.TestNotNull(TEXT("Granted test ability should create a primary instance"),
                                    AbilitySpec->GetPrimaryInstance())
                ? AbilitySpec->GetPrimaryInstance()
                : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    struct FActivatedTaskFixture
    {
        AAeonAbilityTaskPlayMontageAndWaitForEventCharacter* Character{ nullptr };
        UGameplayAbility* Ability{ nullptr };
        UAnimSequenceBase* Animation{ nullptr };
        UAnimMontage* Montage{ nullptr };
        UAeonAbilityTask_PlayMontageAndWaitForEvent* Task{ nullptr };
    };

    FActivatedTaskFixture ActivateGameplayEventTask(FAutomationTestBase& Test,
                                                    TUniquePtr<AeonTests::FTestWorld>& OutWorld,
                                                    const FGameplayTag EventTag,
                                                    const bool bOnlyTriggerOnce = false,
                                                    const bool bOnlyMatchExact = true)
    {
        FActivatedTaskFixture Fixture;
        Fixture.Character = CreateAbilitySystemCharacter(Test, OutWorld);
        if (!Fixture.Character)
        {
            return Fixture;
        }

        Fixture.Ability = ActivateAbility(Test, Fixture.Character->GetAeonAbilitySystemComponent());
        Fixture.Animation = LoadIdleAnimationForTest();
        if (!Test.TestNotNull(TEXT("Activated ability instance should be available"), Fixture.Ability)
            || !Test.TestNotNull(TEXT("Test idle animation should be available"), Fixture.Animation))
        {
            return Fixture;
        }

        Fixture.Montage = CreateDynamicMontageForTest(Test, Fixture.Animation);
        if (!Fixture.Montage)
        {
            return Fixture;
        }

        Fixture.Task = UAeonAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(Fixture.Ability,
                                                                                               NAME_None,
                                                                                               Fixture.Montage,
                                                                                               EventTag,
                                                                                               1.f,
                                                                                               NAME_None,
                                                                                               true,
                                                                                               1.f,
                                                                                               bOnlyTriggerOnce,
                                                                                               bOnlyMatchExact);
        if (Test.TestNotNull(TEXT("Montage-and-event task should be created"), Fixture.Task))
        {
            Fixture.Task->ReadyForActivation();
            Test.TestEqual(TEXT("Task activation should start the requested montage"),
                           Fixture.Character->GetAeonAbilitySystemComponent()->GetCurrentMontage(),
                           Fixture.Montage);
        }

        return Fixture;
    }
} // namespace AeonAbilityTaskPlayMontageAndWaitForEventTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventExternalCancelBroadcastsCancelledTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.ExternalCancel.BroadcastsCancelled",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventExternalCancelBroadcastsCancelledTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateAbilitySystemActor(*this, World))
    {
        const auto Ability =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::ActivateAbility(*this,
                                                                            Actor->GetAeonAbilitySystemComponent());
        const auto Listener =
            AeonTests::NewTransientObject<UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener>(Actor);
        if (TestNotNull(TEXT("Delegate listener should be created"), Listener)
            && TestNotNull(TEXT("Activated ability instance should be available"), Ability))
        {
            const auto Task =
                UAeonAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(Ability,
                                                                                        NAME_None,
                                                                                        nullptr,
                                                                                        FGameplayTag::EmptyTag);
            if (TestNotNull(TEXT("Montage-and-event ability task should be created"), Task))
            {
                Task->OnCancelled.AddDynamic(
                    Listener,
                    &UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener::HandleCancelled);
                Task->OnInterrupted.AddDynamic(
                    Listener,
                    &UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener::HandleInterrupted);

                Task->ExternalCancel();

                return TestEqual(TEXT("External cancel should broadcast the cancelled delegate once"),
                                 Listener->CancelledCount,
                                 1)
                    && TestEqual(TEXT("External cancel should not broadcast the interrupted delegate"),
                                 Listener->InterruptedCount,
                                 0)
                    && TestTrue(TEXT("Cancelled delegate should use an empty event tag"),
                                !Listener->LastCancelledEventTag.IsValid())
                    && TestTrue(TEXT("Cancelled delegate should use an empty payload event tag"),
                                !Listener->LastCancelledPayloadEventTag.IsValid());
            }
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventStopPlayingMontageResetsRootMotionScaleTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.StopPlayingMontage.ResetsRootMotionScale",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventStopPlayingMontageResetsRootMotionScaleTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Character =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::CreateAbilitySystemCharacter(*this, World))
    {
        const auto Ability =
            AeonAbilityTaskPlayMontageAndWaitForEventTests::ActivateAbility(*this,
                                                                            Character->GetAeonAbilitySystemComponent());
        const auto Animation = AeonAbilityTaskPlayMontageAndWaitForEventTests::LoadIdleAnimationForTest();
        const auto AnimInstance = Character->GetMesh()->GetAnimInstance();
        if (TestNotNull(TEXT("Activated ability instance should be available"), Ability)
            && TestNotNull(TEXT("Character should expose an anim instance"), AnimInstance)
            && TestNotNull(TEXT("Test idle animation should be available"), Animation))
        {
            const auto DynamicMontage = Character->GetAeonAbilitySystemComponent()->PlaySlotAnimationAsDynamicMontage(
                Ability,
                Ability->GetCurrentActivationInfo(),
                Animation,
                FName(TEXT("DefaultSlot")),
                0.1f,
                0.1f,
                1.f,
                0.f);

            if (TestNotNull(TEXT("ASC should create a dynamic montage for the test animation"), DynamicMontage)
                && TestEqual(TEXT("ASC should report the dynamic montage as current"),
                             Character->GetAeonAbilitySystemComponent()->GetCurrentMontage(),
                             DynamicMontage))
            {
                const auto Task = UAeonTestPlayMontageAndWaitForEventTask::CreateForTest(Ability, DynamicMontage, 2.5f);
                if (TestNotNull(TEXT("Montage-and-event test task should be created"), Task))
                {
                    Character->SetAnimRootMotionTranslationScale(2.5f);
                    Character->GetAeonAbilitySystemComponent()->SeedAnimatingMontageForTest(Ability, DynamicMontage);

                    return TestTrue(TEXT("StopPlayingMontage should succeed while the ability owns the active montage"),
                                    Task->StopPlayingMontageForTest())
                        && TestEqual(TEXT("Stopping the montage should reset root-motion translation scale"),
                                     Character->GetAnimRootMotionTranslationScale(),
                                     1.f);
                }
            }
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTaskPlayMontageAndWaitForEventExactMatchIgnoresChildTagTest,
                                 "Aeon.AbilityTask.PlayMontageAndWaitForEvent.GameplayEvent.ExactMatchIgnoresChildTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventExactMatchIgnoresChildTagTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    auto Fixture = AeonAbilityTaskPlayMontageAndWaitForEventTests::ActivateGameplayEventTask(
        *this,
        World,
        AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag(),
        false,
        true);
    const auto Listener =
        AeonTests::NewTransientObject<UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener>(Fixture.Character);
    if (TestNotNull(TEXT("Gameplay-event listener should be created"), Listener)
        && TestNotNull(TEXT("Gameplay-event task should be available"), Fixture.Task)
        && TestNotNull(TEXT("Gameplay-event character should be available"), Fixture.Character))
    {
        Fixture.Task->OnEventReceived.AddDynamic(
            Listener,
            &UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener::HandleEventReceived);

        FGameplayEventData Payload;
        Payload.EventTag = AeonAbilityTaskPlayMontageAndWaitForEventTests::TestChildEventTag.GetTag();
        Fixture.Character->GetAeonAbilitySystemComponent()->HandleGameplayEvent(Payload.EventTag, &Payload);
        Fixture.Task->EndTask();

        return TestEqual(TEXT("Exact tag matching should ignore child gameplay events"),
                         Listener->EventReceivedCount,
                         0);
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonAbilityTaskPlayMontageAndWaitForEventInexactMatchAcceptsChildTagTest,
    "Aeon.AbilityTask.PlayMontageAndWaitForEvent.GameplayEvent.InexactMatchAcceptsChildTag",
    AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventInexactMatchAcceptsChildTagTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    auto Fixture = AeonAbilityTaskPlayMontageAndWaitForEventTests::ActivateGameplayEventTask(
        *this,
        World,
        AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag(),
        false,
        false);
    const auto Listener =
        AeonTests::NewTransientObject<UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener>(Fixture.Character);
    if (TestNotNull(TEXT("Gameplay-event listener should be created"), Listener)
        && TestNotNull(TEXT("Gameplay-event task should be available"), Fixture.Task)
        && TestNotNull(TEXT("Gameplay-event character should be available"), Fixture.Character))
    {
        Fixture.Task->OnEventReceived.AddDynamic(
            Listener,
            &UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener::HandleEventReceived);

        FGameplayEventData Payload;
        Payload.EventTag = AeonAbilityTaskPlayMontageAndWaitForEventTests::TestChildEventTag.GetTag();
        Fixture.Character->GetAeonAbilitySystemComponent()->HandleGameplayEvent(Payload.EventTag, &Payload);
        Fixture.Task->EndTask();

        return TestEqual(TEXT("Inexact tag matching should receive child gameplay events"),
                         Listener->EventReceivedCount,
                         1)
            && TestEqual(TEXT("Inexact tag matching should forward the actual child event tag"),
                         Listener->LastEventReceivedTag,
                         AeonAbilityTaskPlayMontageAndWaitForEventTests::TestChildEventTag.GetTag())
            && TestEqual(TEXT("Inexact tag matching should preserve the payload event tag"),
                         Listener->LastEventReceivedPayloadEventTag,
                         AeonAbilityTaskPlayMontageAndWaitForEventTests::TestChildEventTag.GetTag());
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonAbilityTaskPlayMontageAndWaitForEventOnlyTriggerOnceStopsLaterEventsTest,
    "Aeon.AbilityTask.PlayMontageAndWaitForEvent.GameplayEvent.OnlyTriggerOnceStopsLaterEvents",
    AeonTests::AutomationTestFlags)
bool FAeonAbilityTaskPlayMontageAndWaitForEventOnlyTriggerOnceStopsLaterEventsTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    auto Fixture = AeonAbilityTaskPlayMontageAndWaitForEventTests::ActivateGameplayEventTask(
        *this,
        World,
        AeonAbilityTaskPlayMontageAndWaitForEventTests::TestEventTag.GetTag(),
        true,
        false);
    const auto Listener =
        AeonTests::NewTransientObject<UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener>(Fixture.Character);
    if (TestNotNull(TEXT("Gameplay-event listener should be created"), Listener)
        && TestNotNull(TEXT("Gameplay-event task should be available"), Fixture.Task)
        && TestNotNull(TEXT("Gameplay-event character should be available"), Fixture.Character))
    {
        Fixture.Task->OnEventReceived.AddDynamic(
            Listener,
            &UAeonAbilityTaskPlayMontageAndWaitForEventDelegateListener::HandleEventReceived);

        FGameplayEventData Payload;
        Payload.EventTag = AeonAbilityTaskPlayMontageAndWaitForEventTests::TestChildEventTag.GetTag();
        Fixture.Character->GetAeonAbilitySystemComponent()->HandleGameplayEvent(Payload.EventTag, &Payload);
        Fixture.Character->GetAeonAbilitySystemComponent()->HandleGameplayEvent(Payload.EventTag, &Payload);
        Fixture.Task->EndTask();

        return TestEqual(TEXT("OnlyTriggerOnce should ignore gameplay events after the first match"),
                         Listener->EventReceivedCount,
                         1);
    }

    return false;
}

#endif
