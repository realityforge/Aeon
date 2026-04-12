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
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AeonAbilityTask_PlayMontageAndWaitForEvent)

UAeonAbilityTask_PlayMontageAndWaitForEvent*
UAeonAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(UGameplayAbility* const OwningAbility,
                                                                        const FName TaskInstanceName,
                                                                        UAnimMontage* const MontageToPlay,
                                                                        const FGameplayTag EventTag,
                                                                        const float Rate,
                                                                        const FName StartSection,
                                                                        const bool bStopWhenAbilityEnds,
                                                                        const float AnimRootMotionTranslationScale,
                                                                        const bool bOnlyTriggerOnce,
                                                                        const bool bOnlyMatchExact)
{
    const auto Node = NewAbilityTask<UAeonAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName);
    Node->InitializeTask(MontageToPlay,
                         Rate,
                         StartSection,
                         bStopWhenAbilityEnds,
                         AnimRootMotionTranslationScale,
                         EventTag,
                         bOnlyTriggerOnce,
                         bOnlyMatchExact);
    return Node;
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::InitializeTask(UAnimMontage* const InMontageToPlay,
                                                                 const float InRate,
                                                                 const FName InStartSection,
                                                                 const bool bInStopWhenAbilityEnds,
                                                                 const float InAnimRootMotionTranslationScale,
                                                                 const FGameplayTag InEventTag,
                                                                 const bool bInOnlyTriggerOnce,
                                                                 const bool bInOnlyMatchExact)
{
    MontageToPlay = InMontageToPlay;
    EventTag = InEventTag;
    Rate = InRate;
    StartSection = InStartSection;
    bStopWhenAbilityEnds = bInStopWhenAbilityEnds;
    AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
    bOnlyTriggerOnce = bInOnlyTriggerOnce;
    bOnlyMatchExact = bInOnlyMatchExact;
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
    if (Ability && MontageToPlay)
    {
        if (const auto ASC = AbilitySystemComponent.Get())
        {
            if (bOnlyMatchExact)
            {
                auto& MulticastDelegate = ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag);
                OnGameplayEventHandle = MulticastDelegate.AddUObject(this, &ThisClass::OnGameplayEvent);
            }
            else
            {
                OnGameplayEventHandle = ASC->AddGameplayEventTagContainerDelegate(
                    FGameplayTagContainer(EventTag),
                    FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnGameplayEvent));
            }

            if (const auto AnimInstance = Ability->GetCurrentActorInfo()->GetAnimInstance())
            {
                const auto ActivationInfo = Ability->GetCurrentActivationInfo();
                if (ASC->PlayMontage(Ability, ActivationInfo, MontageToPlay, Rate, StartSection) > 0.f)
                {
                    // Playing a montage could potentially fire off a callback into game code which could kill this
                    // ability! Early out if we are pending kill.
                    if (ShouldBroadcastAbilityTaskDelegates())
                    {
                        OnGameplayAbilityCancelledHandle =
                            Ability->OnGameplayAbilityCancelled.AddUObject(this,
                                                                           &ThisClass::OnGameplayAbilityCancelled);

                        BlendingOutDelegate.BindUObject(this, &ThisClass::OnMontageBlendingOut);
                        AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

                        MontageEndedDelegate.BindUObject(this, &ThisClass::OnMontageEnded);
                        AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

                        if (const auto Character = Cast<ACharacter>(GetAvatarActor()))
                        {
                            const auto LocalRole = Character->GetLocalRole();
                            const auto ExecPolicy = Ability->GetNetExecutionPolicy();
                            if (ROLE_Authority == LocalRole
                                || (ROLE_AutonomousProxy == LocalRole
                                    && EGameplayAbilityNetExecutionPolicy::LocalPredicted == ExecPolicy))
                            {
                                Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
                            }
                        }
                        SetWaitingOnAvatar();
                    }
                    return;
                }
                else
                {
                    ABILITY_LOG(Warning,
                                TEXT("PlayMontageAndWaitForEvent called in Ability %s failed "
                                     "to play montage %s; Task Instance Name %s."),
                                *Ability->GetName(),
                                *GetNameSafe(MontageToPlay),
                                *InstanceName.ToString());
                }
            }
            else
            {
                ABILITY_LOG(Warning,
                            TEXT("PlayMontageAndWaitForEvent called in Ability %s failed "
                                 "play montage %s; Task Instance Name %s as to getAnimInstance() "
                                 "for current actor was null."),
                            *GetNameSafe(Ability),
                            *GetNameSafe(MontageToPlay),
                            *InstanceName.ToString());
            }
        }
        else
        {
            ABILITY_LOG(Warning,
                        TEXT("PlayMontageAndWaitForEvent called in Ability %s failed "
                             "play montage %s; Task Instance Name %s as Ability or Montage invalid."),
                        *GetNameSafe(Ability),
                        *GetNameSafe(MontageToPlay),
                        *InstanceName.ToString());
        }
    }
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnCancelled.Broadcast(FGameplayTag::EmptyTag, FGameplayEventData());
    }

    EndTask();
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnCancelled.Broadcast(FGameplayTag::EmptyTag, FGameplayEventData());
    }
    Super::ExternalCancel();
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(const bool bInOwnerFinished)
{
    // Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next
    // montage plays. (If we are destroyed, it will detect this and not do anything)

    // This delegate, however, should be cleared as it is a multicast
    if (Ability)
    {
        Ability->OnGameplayAbilityCancelled.Remove(OnGameplayAbilityCancelledHandle);
        if (bInOwnerFinished && bStopWhenAbilityEnds)
        {
            StopPlayingMontage();
        }
    }

    if (const auto ASC = AbilitySystemComponent.Get())
    {
        if (OnGameplayEventHandle.IsValid())
        {
            if (bOnlyMatchExact)
            {
                ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).Remove(OnGameplayEventHandle);
            }
            else
            {
                ASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(EventTag), OnGameplayEventHandle);
            }
            OnGameplayEventHandle.Reset();
        }
    }
    Super::OnDestroy(bInOwnerFinished);
}

FString UAeonAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
    const UAnimMontage* PlayingMontage{ nullptr };
    if (Ability)
    {
        if (const auto AnimInstance = Ability->GetCurrentActorInfo()->GetAnimInstance())
        {
            PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? ToRawPtr(MontageToPlay)
                                                                           : AnimInstance->GetCurrentActiveMontage();
        }
    }

    return FString::Printf(TEXT("PlayMontageAndWaitForEvent. Montage: %s (Currently Playing %s), EventTag: %s"),
                           *GetNameSafe(MontageToPlay),
                           *GetNameSafe(PlayingMontage),
                           *EventTag.ToString());
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnGameplayAbilityCancelled()
{
    if (StopPlayingMontage())
    {
        // Let the BP handle the interrupt as well
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnInterrupted.Broadcast(FGameplayTag::EmptyTag, FGameplayEventData());
        }
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnMontageEnded(UAnimMontage*, const bool bInterrupted)
{
    if (!bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCompleted.Broadcast(FGameplayTag::EmptyTag, FGameplayEventData());
        }
    }

    EndTask();
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::ResetAnimRootMotionTranslationScale() const
{
    if (const auto Character = Cast<ACharacter>(GetAvatarActor()))
    {
        const auto LocalRole = Character->GetLocalRole();
        if (ROLE_Authority == LocalRole
            || (ROLE_AutonomousProxy == LocalRole
                && EGameplayAbilityNetExecutionPolicy::LocalPredicted == Ability->GetNetExecutionPolicy()))
        {
            Character->SetAnimRootMotionTranslationScale(1.f);
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
bool UAeonAbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
{
    if (Ability)
    {
        if (const auto ActorInfo = Ability->GetCurrentActorInfo())
        {
            if (const auto AnimInstance = ActorInfo->GetAnimInstance())
            {
                // Check if the montage is still playing
                // The ability would have been interrupted, in which case we should automatically stop the montage
                if (const auto ASC = AbilitySystemComponent.Get())
                {
                    if (ASC->GetAnimatingAbility() == Ability && ASC->GetCurrentMontage() == MontageToPlay)
                    {
                        ResetAnimRootMotionTranslationScale();

                        // Unbind delegates so they don't get called as well
                        if (const auto MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
                        {
                            MontageInstance->OnMontageBlendingOutStarted.Unbind();
                            MontageInstance->OnMontageEnded.Unbind();
                        }
                        ASC->CurrentMontageStop();
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, const bool bInterrupted)
{
    if (Montage == MontageToPlay && Ability && Ability->GetCurrentMontage() == MontageToPlay)
    {
        ResetAnimRootMotionTranslationScale();
        if (bInterrupted)
        {
            if (const auto ASC = AbilitySystemComponent.Get())
            {
                ASC->ClearAnimatingAbility(Ability);
            }
        }
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        if (bInterrupted)
        {
            OnInterrupted.Broadcast(FGameplayTag::EmptyTag, FGameplayEventData());
        }
        else
        {
            OnBlendOut.Broadcast(FGameplayTag::EmptyTag, FGameplayEventData());
        }
    }
}

void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(const FGameplayEventData* Payload)
{
    OnGameplayEvent(EventTag, Payload);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UAeonAbilityTask_PlayMontageAndWaitForEvent::OnGameplayEvent(const FGameplayTag InEventTag,
                                                                  const FGameplayEventData* Payload)
{
    if (bOnlyTriggerOnce && OnGameplayEventHandle.IsValid())
    {
        if (const auto ASC = AbilitySystemComponent.Get())
        {
            if (bOnlyMatchExact)
            {
                ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).Remove(OnGameplayEventHandle);
            }
            else
            {
                ASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(EventTag), OnGameplayEventHandle);
            }
        }
        OnGameplayEventHandle.Reset();
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        const FGameplayEventData SafePayload = Payload ? *Payload : FGameplayEventData();
        OnEventReceived.Broadcast(InEventTag, SafePayload);
    }
}
