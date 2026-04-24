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

#include "ActiveGameplayEffectHandle.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CoreMinimal.h"
#include "AnimNotifyState_ApplyGameplayEffect.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
#if WITH_EDITOR
class FDataValidationContext;
enum class EDataValidationResult : uint8;
#endif

/**
 * Applies an infinite GameplayEffect to the mesh owner's AbilitySystemComponent for the duration of a notify state.
 */
UCLASS(DisplayName = "Apply Gameplay Effect")
class UAnimNotifyState_ApplyGameplayEffect : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UAnimNotifyState_ApplyGameplayEffect();

    virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
                             UAnimSequenceBase* Animation,
                             float TotalDuration,
                             const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
                           UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference) override;

protected:
    virtual FActiveGameplayEffectHandle ApplyEffect(UAbilitySystemComponent* AbilitySystemComponent,
                                                    const UGameplayEffect* Effect) const;

private:
    /**
     * The GameplayEffect class to apply.
     */
    UPROPERTY(EditAnywhere, Category = AnimNotify, meta = (AllowAbstract = false))
    TSubclassOf<UGameplayEffect> EffectClass{ nullptr };

    /**
     * The level of the GameplayEffect to apply. Defaults to 1.0.
     */
    UPROPERTY(EditAnywhere, Category = AnimNotify, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float EffectLevel{ 1.f };
};
