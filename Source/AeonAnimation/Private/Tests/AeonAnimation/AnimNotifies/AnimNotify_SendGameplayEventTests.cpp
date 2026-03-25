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

    #include "Abilities/GameplayAbilityTypes.h"
    #include "AeonAnimation/AnimNotifies/AnimNotify_SendGameplayEvent.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/AeonAnimation/AeonAnimationAutomationTestHelpers.h"
    #include "Tests/AeonAnimation/AeonAnimationAutomationTestTypes.h"

namespace AnimNotifySendGameplayEventTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestGameplayEventTag, "Aeon.Test.Animation.Notify.Event");
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifySendGameplayEventNotifyNameIncludesTagTest,
                                 "Aeon.Animation.AnimNotify.SendGameplayEvent.NotifyNameIncludesTag",
                                 AeonAnimationTests::AutomationTestFlags)
bool FAnimNotifySendGameplayEventNotifyNameIncludesTagTest::RunTest(const FString&)
{
    const FGameplayTag EventTag = AnimNotifySendGameplayEventTests::TestGameplayEventTag.GetTag();
    const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotify_SendGameplayEvent>();
    if (TestNotNull(TEXT("Anim notify should be created"), Notify)
        && TestTrue(TEXT("EventTag should be set"),
                    AeonAnimationTests::SetPropertyValue(*this, Notify, TEXT("EventTag"), EventTag)))
    {
        const auto NotifyName = Notify->GetNotifyName_Implementation();
        return TestTrue(TEXT("Notify name should contain the configured gameplay event tag"),
                        NotifyName.Contains(EventTag.ToString()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifySendGameplayEventNotifyDispatchesGameplayEventTest,
                                 "Aeon.Animation.AnimNotify.SendGameplayEvent.NotifyDispatchesGameplayEvent",
                                 AeonAnimationTests::AutomationTestFlags)
bool FAnimNotifySendGameplayEventNotifyDispatchesGameplayEventTest::RunTest(const FString&)
{
    const FGameplayTag EventTag = AnimNotifySendGameplayEventTests::TestGameplayEventTag.GetTag();
    FGameplayTagContainer EventTagContainer;
    EventTagContainer.AddTag(EventTag);

    TUniquePtr<AeonAnimationTests::FTestWorld> World =
        MakeUnique<AeonAnimationTests::FTestWorld>(TEXT("AeonAnimationNotifySendGameplayEventTestWorld"));
    if (!TestNotNull(TEXT("Animation automation test world should be created"), World.Get())
        || !TestTrue(TEXT("Animation automation test world should be valid"), World->IsValid()))
    {
        return false;
    }
    else
    {
        const auto Actor = World->SpawnActor<AAeonAnimationTestActor>();
        if (!TestNotNull(TEXT("Animation test actor should spawn"), Actor))
        {
            return false;
        }
        else
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            if (TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                AbilitySystemComponent->InitAbilityActorInfo(Actor, Actor);

                bool bReceivedEvent = false;
                FGameplayTag ReceivedTag;
                FDelegateHandle DelegateHandle = AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
                    EventTagContainer,
                    FGameplayEventTagMulticastDelegate::FDelegate::CreateLambda(
                        [&bReceivedEvent, &ReceivedTag](const FGameplayTag EventTag, const FGameplayEventData* Data) {
                            bReceivedEvent = nullptr != Data;
                            ReceivedTag = EventTag;
                        }));

                const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotify_SendGameplayEvent>();
                if (TestNotNull(TEXT("Anim notify should be created"), Notify)
                    && TestTrue(TEXT("EventTag should be set"),
                                AeonAnimationTests::SetPropertyValue(*this, Notify, TEXT("EventTag"), EventTag)))
                {
                    Notify->Notify(Actor->GetSkeletalMeshComponentForTest(), nullptr, FAnimNotifyEventReference());
                    AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTagContainer, DelegateHandle);

                    return TestTrue(TEXT("Notify should dispatch the gameplay event to the mesh owner"), bReceivedEvent)
                        && TestTrue(TEXT("Notify should dispatch the configured gameplay event tag"),
                                    ReceivedTag == EventTag);
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
    }
}

#endif
