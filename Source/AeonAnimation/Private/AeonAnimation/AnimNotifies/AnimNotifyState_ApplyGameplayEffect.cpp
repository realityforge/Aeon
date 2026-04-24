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
#include "AeonAnimation/AnimNotifies/AnimNotifyState_ApplyGameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aeon/Logging.h"
#include "Animation/AnimNotifyQueue.h"
#include "GameplayEffect.h"
#include "Logging/StructuredLog.h"
#if WITH_EDITOR
    #include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotifyState_ApplyGameplayEffect)

namespace AeonApplyGameplayEffectNotify
{
    struct FExecutionKey
    {
        TObjectKey<const USkeletalMeshComponent> MeshComp;
        TObjectKey<const UAnimNotifyState_ApplyGameplayEffect> NotifyInstance;

        bool operator==(const FExecutionKey& Other) const
        {
            return MeshComp == Other.MeshComp && NotifyInstance == Other.NotifyInstance;
        }
    };

    uint32 GetTypeHash(const FExecutionKey& Key)
    {
        auto Hash = GetTypeHash(Key.MeshComp);
        Hash = HashCombine(Hash, GetTypeHash(Key.NotifyInstance));
        return Hash;
    }

    TMap<FExecutionKey, TArray<FActiveGameplayEffectHandle>>& GetAppliedEffectsByExecutionKey()
    {
        static TMap<FExecutionKey, TArray<FActiveGameplayEffectHandle>> AppliedEffectsByExecutionKey;
        return AppliedEffectsByExecutionKey;
    }

    FExecutionKey BuildExecutionKey(const UAnimNotifyState_ApplyGameplayEffect* const Notify,
                                    const USkeletalMeshComponent* const MeshComp,
                                    const FAnimNotifyEventReference& EventReference)
    {
        FExecutionKey Key;
        Key.MeshComp = MeshComp;
        Key.NotifyInstance = Notify;
        return Key;
    }

    void StoreAppliedEffectHandle(const FExecutionKey& Key, const FActiveGameplayEffectHandle Handle)
    {
        if (Handle.IsValid())
        {
            GetAppliedEffectsByExecutionKey().FindOrAdd(Key).Add(Handle);
        }
    }
    void StoreAppliedEffectHandle(const UAnimNotifyState_ApplyGameplayEffect* const Notify,
                                  const USkeletalMeshComponent* const MeshComp,
                                  const FAnimNotifyEventReference& EventReference,
                                  const FActiveGameplayEffectHandle Handle)
    {
        StoreAppliedEffectHandle(BuildExecutionKey(Notify, MeshComp, EventReference), Handle);
    }

    FActiveGameplayEffectHandle TakeAppliedEffectHandle(const FExecutionKey& Key)
    {
        auto& AppliedEffectsByExecutionKey = GetAppliedEffectsByExecutionKey();
        if (auto* const Handles = AppliedEffectsByExecutionKey.Find(Key))
        {
            while (!Handles->IsEmpty())
            {
                const auto Handle = Handles->Pop(EAllowShrinking::No);
                if (Handles->IsEmpty())
                {
                    AppliedEffectsByExecutionKey.Remove(Key);
                }

                if (Handle.IsValid())
                {
                    return Handle;
                }
            }

            AppliedEffectsByExecutionKey.Remove(Key);
        }

        return FActiveGameplayEffectHandle();
    }

    FActiveGameplayEffectHandle TakeAppliedEffectHandle(const UAnimNotifyState_ApplyGameplayEffect* const Notify,
                                                        const USkeletalMeshComponent* const MeshComp,
                                                        const FAnimNotifyEventReference& EventReference)
    {
        return TakeAppliedEffectHandle(BuildExecutionKey(Notify, MeshComp, EventReference));
    }

    FString GetFriendlyEffectName(const TSubclassOf<UGameplayEffect> EffectClass)
    {
        if (!EffectClass)
        {
            return FString();
        }

        if (const auto GeneratedBy = EffectClass->ClassGeneratedBy)
        {
            return GeneratedBy->GetName();
        }

        const FString DisplayName = EffectClass->GetDisplayNameText().ToString();
        return !DisplayName.IsEmpty() ? DisplayName : EffectClass->GetName();
    }

    FString FormatEffectLevel(const float EffectLevel)
    {
        return FString::Printf(TEXT("%g"), static_cast<double>(EffectLevel));
    }

#if WITH_EDITOR
    EDataValidationResult ValidateRequiredInfiniteEffectClass(const UObject* const Owner,
                                                              const TSubclassOf<UGameplayEffect> EffectClass,
                                                              const TCHAR* const PropertyName,
                                                              FDataValidationContext& Context)
    {
        if (EffectClass)
        {
            if (const auto Effect = EffectClass->GetDefaultObject<UGameplayEffect>())
            {
                if (EGameplayEffectDurationType::Infinite == Effect->DurationPolicy)
                {
                    return EDataValidationResult::Valid;
                }
                else
                {
                    Context.AddError(FText::FromString(FString::Printf(
                        TEXT("Object %s property %s must reference a GameplayEffect with an infinite policy"),
                        *GetNameSafe(Owner),
                        PropertyName)));
                    return EDataValidationResult::Invalid;
                }
            }
            else
            {
                Context.AddError(FText::FromString(FString::Printf(TEXT("Object %s has an invalid %s default object"),
                                                                   *GetNameSafe(Owner),
                                                                   PropertyName)));
                return EDataValidationResult::Invalid;
            }
        }
        else
        {
            Context.AddError(FText::FromString(FString::Printf(TEXT("Object %s is missing the required property %s"),
                                                               *GetNameSafe(Owner),
                                                               PropertyName)));
            return EDataValidationResult::Invalid;
        }
    }
#endif
} // namespace AeonApplyGameplayEffectNotify

UAnimNotifyState_ApplyGameplayEffect::UAnimNotifyState_ApplyGameplayEffect()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(49, 149, 255);
    bShouldFireInEditor = false;
#endif
}

FString UAnimNotifyState_ApplyGameplayEffect::GetNotifyName_Implementation() const
{
    const auto FriendlyEffectName = AeonApplyGameplayEffectNotify::GetFriendlyEffectName(EffectClass);
    if (FriendlyEffectName.IsEmpty())
    {
        return TEXT("Apply Gameplay Effect");
    }
    else
    {
        if (FMath::IsNearlyEqual(EffectLevel, 1.f))
        {
            return FString::Printf(TEXT("\u2192 %s"), *FriendlyEffectName);
        }
        else
        {
            return FString::Printf(TEXT("\u2192 %s (Lv %s)"),
                                   *FriendlyEffectName,
                                   *AeonApplyGameplayEffectNotify::FormatEffectLevel(EffectLevel));
        }
    }
}

void UAnimNotifyState_ApplyGameplayEffect::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                       UAnimSequenceBase* Animation,
                                                       const float TotalDuration,
                                                       const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (MeshComp)
    {
        if (EffectClass)
        {
            if (const auto ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
            {
                if (const auto Effect = EffectClass->GetDefaultObject<UGameplayEffect>())
                {
                    const auto Handle = ApplyEffect(ASC, Effect);
                    if (Handle.IsValid())
                    {
                        AeonApplyGameplayEffectNotify::StoreAppliedEffectHandle(this, MeshComp, EventReference, Handle);
                    }
                    else
                    {
                        UE_LOGFMT(
                            LogAeon,
                            Warning,
                            "AnimNotifyState_ApplyGameplayEffect failed to apply effect {Effect} for owner {Owner}",
                            *GetNameSafe(EffectClass),
                            *GetNameSafe(MeshComp->GetOwner()));
                    }
                }
                else
                {
                    UE_LOGFMT(LogAeon,
                              Warning,
                              "AnimNotifyState_ApplyGameplayEffect has an invalid EffectClass "
                              "default object {Effect} for owner {Owner}",
                              *GetNameSafe(EffectClass),
                              *GetNameSafe(MeshComp->GetOwner()));
                }
            }
            else
            {
                UE_LOGFMT(
                    LogAeon,
                    Warning,
                    "AnimNotifyState_ApplyGameplayEffect failed to resolve an ability system component for owner {Owner}",
                    *GetNameSafe(MeshComp->GetOwner()));
            }
        }
        else
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "AnimNotifyState_ApplyGameplayEffect has no EffectClass configured for owner {Owner}",
                      *GetNameSafe(MeshComp->GetOwner()));
        }
    }
}

void UAnimNotifyState_ApplyGameplayEffect::NotifyEnd(USkeletalMeshComponent* MeshComp,
                                                     UAnimSequenceBase* Animation,
                                                     const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (MeshComp)
    {
        // Runtime state cannot live on the notify UObject because notify-state instances are asset-owned and reused
        // across executions. We key by mesh + notify UObject because Unreal rewrites the FAnimNotifyEvent pointer as a
        // state moves through ActiveAnimNotifyEventReference, so begin/end do not reliably share the same GetNotify()
        // pointer even when they refer to the same authored notify state. Branching-point states remain unsupported
        // here because their execution model can still make begin/end pairing too sparse to reason about safely.
        const auto Handle = AeonApplyGameplayEffectNotify::TakeAppliedEffectHandle(this, MeshComp, EventReference);
        if (Handle.IsValid())
        {
            if (const auto ASC = Handle.GetOwningAbilitySystemComponent())
            {
                ASC->RemoveActiveGameplayEffect(Handle);
            }
        }
    }
}

FActiveGameplayEffectHandle
UAnimNotifyState_ApplyGameplayEffect::ApplyEffect(UAbilitySystemComponent* AbilitySystemComponent,
                                                  const UGameplayEffect* Effect) const
{
    check(AbilitySystemComponent);
    check(Effect);
    return AbilitySystemComponent->ApplyGameplayEffectToSelf(Effect,
                                                             EffectLevel,
                                                             AbilitySystemComponent->MakeEffectContext());
}

#if WITH_EDITOR
EDataValidationResult UAnimNotifyState_ApplyGameplayEffect::IsDataValid(FDataValidationContext& Context) const
{
    return CombineDataValidationResults(
        Super::IsDataValid(Context),
        AeonApplyGameplayEffectNotify::ValidateRequiredInfiniteEffectClass(this,
                                                                           EffectClass,
                                                                           TEXT("EffectClass"),
                                                                           Context));
}
#endif
