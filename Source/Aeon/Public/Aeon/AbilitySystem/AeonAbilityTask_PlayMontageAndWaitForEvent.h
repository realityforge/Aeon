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

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAeonGameplayEventSignature,
                                             FGameplayTag,
                                             EventTag,
                                             const FGameplayEventData&,
                                             Payload);

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
                               bool bOnlyTriggerOnce = false,
                               bool bOnlyMatchExact = true);

    /** Broadcast when the montage completes without interruption. */
    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnCompleted;

    /** Broadcast when the montage begins blending out without interruption. */
    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnBlendOut;

    /** Broadcast when the montage or owning ability is interrupted after playback has started. */
    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnInterrupted;

    /** Broadcast when the task is explicitly cancelled or cannot begin playback. */
    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnCancelled;

    /** Broadcast when a matching gameplay event is received while the task is active. */
    UPROPERTY(BlueprintAssignable)
    FAeonGameplayEventSignature OnEventReceived;

    virtual void ExternalCancel() override;
    virtual void OnDestroy(bool bInOwnerFinished) override;
    virtual FString GetDebugString() const override;

protected:
    void InitializeTask(UAnimMontage* InMontageToPlay,
                        float InRate,
                        FName InStartSection,
                        bool bInStopWhenAbilityEnds,
                        float InAnimRootMotionTranslationScale,
                        FGameplayTag InEventTag,
                        bool bInOnlyTriggerOnce,
                        bool bInOnlyMatchExact);

    virtual void Activate() override;

private:
    /** The Montage to play. */
    UPROPERTY()
    TObjectPtr<UAnimMontage> MontageToPlay{ nullptr };

    /** The Event Tag to listen for. */
    UPROPERTY()
    FGameplayTag EventTag{ FGameplayTag::EmptyTag };

    /** Playback rate */
    UPROPERTY()
    float Rate{ 1.f };

    /** Section to start montage from */
    UPROPERTY()
    FName StartSection{ NAME_None };

    /** True if the montage should be aborted if ability ends */
    UPROPERTY()
    bool bStopWhenAbilityEnds{ true };

    /** Modifies how root motion movement to apply */
    UPROPERTY()
    float AnimRootMotionTranslationScale{ 1.f };

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    FOnMontageEnded MontageEndedDelegate;
    FDelegateHandle OnGameplayAbilityCancelledHandle;

    bool bOnlyTriggerOnce{ false };
    bool bOnlyMatchExact{ true };

    void ResetAnimRootMotionTranslationScale() const;

    /** If the ability is playing a montage, stop it and return true else return false. */
    bool StopPlayingMontage();

    void OnGameplayEvent(const FGameplayEventData* Payload);
    void OnGameplayEvent(FGameplayTag InEventTag, const FGameplayEventData* Payload);

    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void OnGameplayAbilityCancelled();

    FDelegateHandle OnGameplayEventHandle;
};

#undef UE_API
