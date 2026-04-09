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

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Aeon/AbilitySystem/AeonAbilityTask_PlayMontageAndWaitForEvent.h"
#include "AeonAbilityTask_PlayMontageAndWaitForEventTestTypes.generated.h"

UCLASS(NotBlueprintable)
class UAeonTestPlayMontageAndWaitForEventListener final : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void HandleCompleted(const FGameplayEventData Payload)
    {
        ++CompletedCount;
        CompletedPayloadTags.Add(Payload.EventTag);
    }

    UFUNCTION()
    void HandleBlendOut(const FGameplayEventData Payload)
    {
        ++BlendOutCount;
        BlendOutPayloadTags.Add(Payload.EventTag);
    }

    UFUNCTION()
    void HandleInterrupted(const FGameplayEventData Payload)
    {
        ++InterruptedCount;
        InterruptedPayloadTags.Add(Payload.EventTag);
    }

    UFUNCTION()
    void HandleCancelled(const FGameplayEventData Payload)
    {
        ++CancelledCount;
        CancelledPayloadTags.Add(Payload.EventTag);
    }

    // ReSharper disable once CppPassValueParameterByConstReference
    UFUNCTION()
    void HandleEventReceived(const FGameplayEventData GameplayEventData)
    {
        ++EventCount;
        ReceivedEventTags.Add(GameplayEventData.EventTag);
    }

    int32 CompletedCount{ 0 };
    int32 BlendOutCount{ 0 };
    int32 InterruptedCount{ 0 };
    int32 CancelledCount{ 0 };
    int32 EventCount{ 0 };
    TArray<FGameplayTag> CompletedPayloadTags;
    TArray<FGameplayTag> BlendOutPayloadTags;
    TArray<FGameplayTag> InterruptedPayloadTags;
    TArray<FGameplayTag> CancelledPayloadTags;
    TArray<FGameplayTag> ReceivedEventTags;
};

UCLASS()
class UAeonTestMontageTask final : public UAbilityTask_PlayMontageAndWait
{
    GENERATED_BODY()

public:
    virtual void Activate() override { bActivatedForTest = true; }

    virtual void ExternalCancel() override
    {
        ++ExternalCancelCountForTest;
        OnCancelled.Broadcast();
    }

    bool bActivatedForTest{ false };
    bool bDestroyedForTest{ false };
    int32 ExternalCancelCountForTest{ 0 };

    void BroadcastCompletedForTest() const { OnCompleted.Broadcast(); }
    void BroadcastBlendOutForTest() const { OnBlendOut.Broadcast(); }
    void BroadcastInterruptedForTest() const { OnInterrupted.Broadcast(); }

protected:
    virtual void OnDestroy(const bool bInOwnerFinished) override
    {
        bDestroyedForTest = true;
        Super::OnDestroy(bInOwnerFinished);
    }
};

UCLASS()
class UAeonTestGameplayEventTask final : public UAbilityTask_WaitGameplayEvent
{
    GENERATED_BODY()

public:
    virtual void Activate() override { bActivatedForTest = true; }

    virtual void OnDestroy(const bool bInOwnerFinished) override
    {
        bDestroyedForTest = true;
        Super::OnDestroy(bInOwnerFinished);
    }

    void BroadcastEventReceivedForTest(const FGameplayTag EventTag) const
    {
        FGameplayEventData Payload;
        Payload.EventTag = EventTag;
        EventReceived.Broadcast(Payload);
    }

    bool bActivatedForTest{ false };
    bool bDestroyedForTest{ false };
    FGameplayTag ConfiguredEventTagForTest{ FGameplayTag::EmptyTag };
    bool bOnlyTriggerOnceForTest{ false };
    bool bOnlyMatchExactForTest{ false };
};

UCLASS(NotBlueprintable)
class UAeonTestPlayMontageAndWaitForEventTask final : public UAeonAbilityTask_PlayMontageAndWaitForEvent
{
    GENERATED_BODY()

public:
    static UAeonTestPlayMontageAndWaitForEventTask* CreateForTest(UGameplayAbility* OwningAbility,
                                                                  UAnimMontage* MontageToPlay,
                                                                  const FGameplayTag EventTag,
                                                                  const bool bOnlyTriggerOnce = false,
                                                                  const bool bOnlyMatchExact = true)
    {
        const auto Task = NewAbilityTask<UAeonTestPlayMontageAndWaitForEventTask>(OwningAbility);
        Task->ExpectedEventTagForTest = EventTag;
        Task->bExpectedOnlyTriggerOnceForTest = bOnlyTriggerOnce;
        Task->bExpectedOnlyMatchExactForTest = bOnlyMatchExact;
        Task->InitializeTask(NAME_None,
                             MontageToPlay,
                             EventTag,
                             1.f,
                             NAME_None,
                             true,
                             1.f,
                             0.f,
                             false,
                             nullptr,
                             bOnlyTriggerOnce,
                             bOnlyMatchExact);
        return Task;
    }

    void SetForceMontageTaskNullForTest(const bool bValue) { bForceMontageTaskNullForTest = bValue; }
    void SetForceGameplayEventTaskNullForTest(const bool bValue) { bForceGameplayEventTaskNullForTest = bValue; }
    void ActivateForTest() { Activate(); }
    void ExternalCancelForTest() { ExternalCancel(); }

    const TArray<FString>& GetActivationOrderForTest() const { return ActivationOrderForTest; }
    UAeonTestMontageTask* GetMontageTaskForTest() const { return MontageTaskForTest; }
    UAeonTestGameplayEventTask* GetGameplayEventTaskForTest() const { return GameplayEventTaskForTest; }

protected:
    virtual UAbilityTask_PlayMontageAndWait* CreateMontageTask() override
    {
        if (bForceMontageTaskNullForTest)
        {
            MontageTaskForTest = nullptr;
            return nullptr;
        }

        MontageTaskForTest = NewAbilityTask<UAeonTestMontageTask>(Ability);
        return MontageTaskForTest;
    }

    virtual UAbilityTask_WaitGameplayEvent* CreateGameplayEventTask() override
    {
        if (bForceGameplayEventTaskNullForTest)
        {
            GameplayEventTaskForTest = nullptr;
            return nullptr;
        }

        GameplayEventTaskForTest = NewAbilityTask<UAeonTestGameplayEventTask>(Ability);
        GameplayEventTaskForTest->ConfiguredEventTagForTest = ExpectedEventTagForTest;
        GameplayEventTaskForTest->bOnlyTriggerOnceForTest = bExpectedOnlyTriggerOnceForTest;
        GameplayEventTaskForTest->bOnlyMatchExactForTest = bExpectedOnlyMatchExactForTest;
        return GameplayEventTaskForTest;
    }

    virtual void ReadySubTaskForActivation(UGameplayTask* const Task) override
    {
        if (Task == GameplayEventTaskForTest)
        {
            ActivationOrderForTest.Add(TEXT("GameplayEvent"));
        }
        else if (Task == MontageTaskForTest)
        {
            ActivationOrderForTest.Add(TEXT("Montage"));
        }

        if (Task)
        {
            Task->ReadyForActivation();
        }
    }

private:
    bool bForceMontageTaskNullForTest{ false };
    bool bForceGameplayEventTaskNullForTest{ false };
    FGameplayTag ExpectedEventTagForTest{ FGameplayTag::EmptyTag };
    bool bExpectedOnlyTriggerOnceForTest{ false };
    bool bExpectedOnlyMatchExactForTest{ true };
    TArray<FString> ActivationOrderForTest;

    UPROPERTY()
    TObjectPtr<UAeonTestMontageTask> MontageTaskForTest{ nullptr };

    UPROPERTY()
    TObjectPtr<UAeonTestGameplayEventTask> GameplayEventTaskForTest{ nullptr };
};
