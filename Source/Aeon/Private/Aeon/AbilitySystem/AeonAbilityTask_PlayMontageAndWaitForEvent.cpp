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
#include "Aeon/AbilitySystem/AeonAbilityTask_PlayMontageAndWaitForEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AeonAbilityTask_PlayMontageAndWaitForEvent)

namespace
{
    FGameplayEventData MakeEmptyPayload()
    {
        return FGameplayEventData();
    }
} // namespace

UAeonAbilityTask_PlayMontageAndWaitForEvent*
UAeonAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(UGameplayAbility* const OwningAbility,
                                                                        const FName TaskInstanceName,
                                                                        UAnimMontage* const MontageToPlay,
                                                                        const FGameplayTag EventTag,
                                                                        const float Rate,
                                                                        const FName StartSection,
                                                                        const bool bStopWhenAbilityEnds,
                                                                        const float AnimRootMotionTranslationScale,
                                                                        const float MontageStartTimeSeconds,
                                                                        const bool bAllowInterruptAfterBlendOut,
                                                                        AActor* const OptionalExternalTarget,
                                                                        const bool bOnlyTriggerOnce,
                                                                        const bool bOnlyMatchExact)
{
    const auto Node = NewAbilityTask<UAeonAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName);
    Node->InitializeTask(TaskInstanceName,
                         MontageToPlay,
                         EventTag,
                         Rate,
                         StartSection,
                         bStopWhenAbilityEnds,
                         AnimRootMotionTranslationScale,
                         MontageStartTimeSeconds,
                         bAllowInterruptAfterBlendOut,
                         OptionalExternalTarget,
                         bOnlyTriggerOnce,
                         bOnlyMatchExact);
    return Node;
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::InitializeTask(const FName InTaskInstanceName,
                                                                 UAnimMontage* const InMontageToPlay,
                                                                 const FGameplayTag InEventTag,
                                                                 const float InRate,
                                                                 const FName InStartSection,
                                                                 const bool bInStopWhenAbilityEnds,
                                                                 const float InAnimRootMotionTranslationScale,
                                                                 const float InMontageStartTimeSeconds,
                                                                 const bool bInAllowInterruptAfterBlendOut,
                                                                 AActor* const InOptionalExternalTarget,
                                                                 const bool bInOnlyTriggerOnce,
                                                                 const bool bInOnlyMatchExact)
{
    TaskInstanceName = InTaskInstanceName;
    MontageToPlay = InMontageToPlay;
    EventTag = InEventTag;
    Rate = InRate;
    StartSection = InStartSection;
    bStopWhenAbilityEnds = bInStopWhenAbilityEnds;
    AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
    MontageStartTimeSeconds = InMontageStartTimeSeconds;
    bAllowInterruptAfterBlendOut = bInAllowInterruptAfterBlendOut;
    OptionalExternalTarget = InOptionalExternalTarget;
    bOnlyTriggerOnce = bInOnlyTriggerOnce;
    bOnlyMatchExact = bInOnlyMatchExact;
}

UAbilityTask_PlayMontageAndWait* UAeonAbilityTask_PlayMontageAndWaitForEvent::CreateMontageTask()
{
    return UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(Ability,
                                                                          TaskInstanceName,
                                                                          MontageToPlay,
                                                                          Rate,
                                                                          StartSection,
                                                                          bStopWhenAbilityEnds,
                                                                          AnimRootMotionTranslationScale,
                                                                          MontageStartTimeSeconds,
                                                                          bAllowInterruptAfterBlendOut);
}

UAbilityTask_WaitGameplayEvent* UAeonAbilityTask_PlayMontageAndWaitForEvent::CreateGameplayEventTask()
{
    return UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(Ability,
                                                             EventTag,
                                                             OptionalExternalTarget.Get(),
                                                             bOnlyTriggerOnce,
                                                             bOnlyMatchExact);
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::ReadySubTaskForActivation(UGameplayTask* const Task)
{
    check(Task);
    Task->ReadyForActivation();
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
    if (!Ability || !MontageToPlay)
    {
        BroadcastCancelledAndEndTask();
        return;
    }

    GameplayEventTask = CreateGameplayEventTask();
    MontageTask = CreateMontageTask();

    if (GameplayEventTask && MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageCompleted);
        MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::HandleMontageBlendOut);
        MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageInterrupted);
        MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageCancelled);
        GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::HandleGameplayEvent);

        ReadySubTaskForActivation(GameplayEventTask);
        ReadySubTaskForActivation(MontageTask);
    }
    else
    {
        CleanupSubTasks();
        BroadcastCancelledAndEndTask();
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
    if (MontageTask)
    {
        MontageTask->ExternalCancel();
    }
    else
    {
        BroadcastCancelledAndEndTask();
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(const bool bInOwnerFinished)
{
    CleanupSubTasks();
    Super::OnDestroy(bInOwnerFinished);
}

FString UAeonAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
    return FString::Printf(TEXT("PlayMontageAndWaitForEvent. Montage: %s, EventTag: %s"),
                           *GetNameSafe(MontageToPlay),
                           *EventTag.ToString());
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::HandleMontageCompleted()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnCompleted.Broadcast(MakeEmptyPayload());
    }

    EndTask();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UAeonAbilityTask_PlayMontageAndWaitForEvent::HandleMontageBlendOut()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnBlendOut.Broadcast(MakeEmptyPayload());
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::HandleMontageInterrupted()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnInterrupted.Broadcast(MakeEmptyPayload());
    }

    EndTask();
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::HandleMontageCancelled()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnCancelled.Broadcast(MakeEmptyPayload());
    }

    EndTask();
}

// ReSharper disable once CppPassValueParameterByConstReference
// ReSharper disable once CppMemberFunctionMayBeConst
void UAeonAbilityTask_PlayMontageAndWaitForEvent::HandleGameplayEvent(const FGameplayEventData Payload)
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        EventReceived.Broadcast(Payload);
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::CleanupSubTasks()
{
    if (MontageTask)
    {
        MontageTask->OnCompleted.RemoveDynamic(this, &ThisClass::HandleMontageCompleted);
        MontageTask->OnBlendOut.RemoveDynamic(this, &ThisClass::HandleMontageBlendOut);
        MontageTask->OnInterrupted.RemoveDynamic(this, &ThisClass::HandleMontageInterrupted);
        MontageTask->OnCancelled.RemoveDynamic(this, &ThisClass::HandleMontageCancelled);

        const auto LocalMontageTask = MontageTask;
        MontageTask = nullptr;
        LocalMontageTask->EndTask();
    }

    if (GameplayEventTask)
    {
        GameplayEventTask->EventReceived.RemoveDynamic(this, &ThisClass::HandleGameplayEvent);

        const auto LocalGameplayEventTask = GameplayEventTask;
        GameplayEventTask = nullptr;
        LocalGameplayEventTask->EndTask();
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::BroadcastCancelledAndEndTask()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnCancelled.Broadcast(MakeEmptyPayload());
    }

    EndTask();
}
