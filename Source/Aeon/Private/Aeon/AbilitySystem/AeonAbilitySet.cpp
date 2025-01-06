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
#include "Aeon/AbilitySystem/AeonAbilitySet.h"
#include "ActiveGameplayEffectHandle.h"
#include "Aeon/AbilitySystem/AeonAbilitySystemComponent.h"
#include "Aeon/AbilitySystem/AeonGameplayAbility.h"
#include "Aeon/Logging.h"
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AeonAbilitySet)

void FAeonAbilitySetHandles::RemoveFromAbilitySystemComponent()
{
    if (AbilitySystemComponent)
    {
        if (AbilitySystemComponent->IsOwnerActorAuthoritative())
        {
            for (const auto& Handle : AbilitySpecHandles)
            {
                if (Handle.IsValid())
                {
                    AbilitySystemComponent->ClearAbility(Handle);
                }
            }

            for (const auto& Handle : EffectHandles)
            {
                if (Handle.IsValid())
                {
                    AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
                }
            }

            for (const auto AttributeSet : AttributeSets)
            {
                AbilitySystemComponent->RemoveSpawnedAttribute(AttributeSet);
            }

            AbilitySpecHandles.Reset();
            EffectHandles.Reset();
            AttributeSets.Reset();
            AbilitySystemComponent = nullptr;
        }
        else
        {
            AEON_WARNING_ALOG("RemoveAbilitySetFromAbilitySystemComponent() must be invoked when "
                              "OwnerActor is Authoritative");
        }
    }
    else
    {
        AEON_WARNING_ALOG("RemoveAbilitySetFromAbilitySystemComponent() invoked when AbilitySystemComponent "
                          "is invalid. This is likely a result of invoking it multiple times. Please guard "
                          "call with IsValid() or avoid calling when handles are invalid.");
    }
}

bool FAeonAbilitySetHandles::IsValid() const
{
    return nullptr != AbilitySystemComponent;
}

UAeonAbilitySet::UAeonAbilitySet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UAeonAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* AbilitySystemComponent,
                                          FAeonAbilitySetHandles* OutGrantedHandles,
                                          int32 LevelDelta,
                                          UObject* SourceObject) const
{
    checkf(AbilitySystemComponent, TEXT("AbilitySystemComponent must not be null"));
    if (AbilitySystemComponent->IsOwnerActorAuthoritative())
    {
        if (OutGrantedHandles)
        {
            OutGrantedHandles->AbilitySystemComponent = AbilitySystemComponent;
        }

        for (int32 Index = 0; Index < Abilities.Num(); ++Index)
        {
            // ReSharper disable once CppUseStructuredBinding
            if (const auto& Ability = Abilities[Index]; IsValid(Ability.Ability))
            {
                const auto CDO = Ability.Ability->GetDefaultObject<UAeonGameplayAbility>();
                FGameplayAbilitySpec AbilitySpec(CDO, Ability.Level + LevelDelta);
                AbilitySpec.SourceObject = SourceObject;
                if (Ability.InputTag.IsValid())
                {
                    // Only add tag if it is valid
                    AbilitySpec.GetDynamicSpecSourceTags().AddTag(Ability.InputTag);
                }

                // ReSharper disable once CppTooWideScopeInitStatement
                const auto Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
                if (OutGrantedHandles && Handle.IsValid())
                {
                    OutGrantedHandles->AbilitySpecHandles.Add(Handle);
                }
            }
            else
            {
                AEON_ERROR_ALOG("AbilitySet '%s' has invalid value at Abilities[%d]", *GetNameSafe(this), Index);
            }
        }

        for (int32 Index = 0; Index < Effects.Num(); ++Index)
        {
            // ReSharper disable once CppUseStructuredBinding
            if (const auto& Effect = Effects[Index]; IsValid(Effect.Effect))
            {
                const auto CDO = Effect.Effect->GetDefaultObject<UGameplayEffect>();
                auto EffectContext = AbilitySystemComponent->MakeEffectContext();
                const float EffectLevel = Effect.Level + LevelDelta;
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto Handle = AbilitySystemComponent->ApplyGameplayEffectToSelf(CDO, EffectLevel, EffectContext);
                if (OutGrantedHandles && Handle.IsValid())
                {
                    OutGrantedHandles->EffectHandles.Add(Handle);
                }
            }
            else
            {
                AEON_ERROR_ALOG("AbilitySet '%s' has invalid value at Effects[%d]", *GetNameSafe(this), Index);
            }
        }

        for (int32 Index = 0; Index < AttributeSets.Num(); ++Index)
        {
            if (const auto& Entry = AttributeSets[Index]; IsValid(Entry.AttributeSet))
            {
                const auto Outer = AbilitySystemComponent->GetOwner();
                const auto AttributeSet = NewObject<UAttributeSet>(Outer, Entry.AttributeSet);
                AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
                if (OutGrantedHandles)
                {
                    OutGrantedHandles->AttributeSets.Add(AttributeSet);
                }
            }
            else
            {
                AEON_ERROR_ALOG("AbilitySet '%s' has invalid value at  AttributeSets[%d]", *GetNameSafe(this), Index);
            }
        }
    }
    else
    {
        AEON_WARNING_ALOG("GiveToAbilitySystem() must be invoked when OwnerActor is Authoritative");
    }
}

#if WITH_EDITOR
EDataValidationResult UAeonAbilitySet::IsDataValid(FDataValidationContext& Context) const
{
    auto Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

    for (int32 Index = 0; Index < Abilities.Num(); ++Index)
    {
        // ReSharper disable once CppUseStructuredBinding
        if (const auto& Ability = Abilities[Index]; !IsValid(Ability.Ability))
        {
            Context.AddError(
                FText::FromString(FString::Printf(TEXT("Abilities[%d].Ability references an invalid value"), Index)));
            Result = EDataValidationResult::Invalid;
        }
    }

    for (int32 Index = 0; Index < Effects.Num(); ++Index)
    {
        // ReSharper disable once CppUseStructuredBinding
        if (const auto& Effect = Effects[Index]; !IsValid(Effect.Effect))
        {
            Context.AddError(
                FText::FromString(FString::Printf(TEXT("Effects[%d].Effect references an invalid value"), Index)));
            Result = EDataValidationResult::Invalid;
        }
    }

    for (int32 Index = 0; Index < AttributeSets.Num(); ++Index)
    {
        if (const auto& Entry = AttributeSets[Index]; !IsValid(Entry.AttributeSet))
        {
            Context.AddError(FText::FromString(
                FString::Printf(TEXT("AttributeSets[%d].AttributeSet references an invalid value"), Index)));
            Result = EDataValidationResult::Invalid;
        }
    }

    return Result;
}
#endif
