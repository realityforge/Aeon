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

#include "Abilities/Tasks/AbilityTask.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AeonAbilityTask_PlayMontageAndWaitForEvent.generated.h"

#define UE_API AEON_API

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UGameplayTask;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAeonGameplayEventSignature, FGameplayEventData, Payload);

UCLASS()
class UE_API UAeonAbilityTask_PlayMontageAndWaitForEvent : public UAbilityTask
{
    GENERATED_BODY()

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
    friend class UAeonTestPlayMontageAndWaitForEventTask;
#endif

public:
    UFUNCTION(BlueprintCallable,
              Category = "Aeon|AbilityTasks",
              meta = (DisplayName = "Play Montage And Wait For Event",
                      HidePin = "OwningAbility",
                      DefaultToSelf = "OwningAbility",
                      BlueprintInternalUseOnly = "true"))
    static UAeonAbilityTask_PlayMontageAndWaitForEvent*
    PlayMontageAndWaitForEvent(UGameplayAbility* OwningAbility,
                               FName TaskInstanceName,
                               UAnimMontage* MontageToPlay,
                               FGameplayTag EventTag,
                               float Rate = 1.f,
                               FName StartSection = NAME_None,
                               bool bStopWhenAbilityEnds = true,
                               float AnimRootMotionTranslationScale = 1.f,
                               float MontageStartTimeSeconds = 0.f,
                               bool bAllowInterruptAfterBlendOut = false,
                               AActor* OptionalExternalTarget = nullptr,
                               bool bOnlyTriggerOnce = false,
                               bool bOnlyMatchExact = true);

    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnCompleted;

    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnBlendOut;

    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnInterrupted;

    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnCancelled;

    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature EventReceived;

    virtual void ExternalCancel() override;
    virtual void OnDestroy(bool bInOwnerFinished) override;
    virtual FString GetDebugString() const override;

protected:
    void InitializeTask(FName InTaskInstanceName,
                        UAnimMontage* InMontageToPlay,
                        FGameplayTag InEventTag,
                        float InRate,
                        FName InStartSection,
                        bool bInStopWhenAbilityEnds,
                        float InAnimRootMotionTranslationScale,
                        float InMontageStartTimeSeconds,
                        bool bInAllowInterruptAfterBlendOut,
                        AActor* InOptionalExternalTarget,
                        bool bInOnlyTriggerOnce,
                        bool bInOnlyMatchExact);

    virtual UAbilityTask_PlayMontageAndWait* CreateMontageTask();
    virtual UAbilityTask_WaitGameplayEvent* CreateGameplayEventTask();
    virtual void ReadySubTaskForActivation(UGameplayTask* Task);

    virtual void Activate() override;

    UFUNCTION()
    void HandleMontageCompleted();

    UFUNCTION()
    void HandleMontageBlendOut();

    UFUNCTION()
    void HandleMontageInterrupted();

    UFUNCTION()
    void HandleMontageCancelled();

    UFUNCTION()
    void HandleGameplayEvent(FGameplayEventData Payload);

private:
    void CleanupSubTasks();
    void BroadcastCancelledAndEndTask();

    UPROPERTY()
    TObjectPtr<UAnimMontage> MontageToPlay{ nullptr };

    UPROPERTY()
    TObjectPtr<AActor> OptionalExternalTarget{ nullptr };

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask{ nullptr };

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> GameplayEventTask{ nullptr };

    FName TaskInstanceName{ NAME_None };
    FGameplayTag EventTag{ FGameplayTag::EmptyTag };
    float Rate{ 1.f };
    FName StartSection{ NAME_None };
    bool bStopWhenAbilityEnds{ true };
    float AnimRootMotionTranslationScale{ 1.f };
    float MontageStartTimeSeconds{ 0.f };
    bool bAllowInterruptAfterBlendOut{ false };
    bool bOnlyTriggerOnce{ false };
    bool bOnlyMatchExact{ true };
};

#undef UE_API
