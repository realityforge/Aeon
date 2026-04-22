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
#include "Aeon/AbilitySystem/AeonAbilitySystemComponent.h"
#include "Aeon/AbilitySystem/AeonAbilitySystemPostInitInterface.h"
#include "Aeon/AbilitySystem/AeonAbilityTagRelationshipMapping.h"
#include "Aeon/AbilitySystem/AeonGameplayAbility.h"
#include "Aeon/AeonGameplayTags.h"
#include "Aeon/Logging.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AeonAbilitySystemComponent)

namespace AeonAbilitySystemComponent
{
    bool IsOptInPostInitAttributeSet(UAttributeSet* const AttributeSet)
    {
        return IsValid(AttributeSet) && Cast<IAeonAbilitySystemPostInitInterface>(AttributeSet);
    }
} // namespace AeonAbilitySystemComponent

bool UAeonAbilitySystemComponent::IsAbilitySystemReady() const
{
    return bAbilitySystemReady;
}

bool UAeonAbilitySystemComponent::HasOutstandingPostInitWork() const
{
    if (bAbilitySystemReady && !bIsTearingDown)
    {
        if (AeonMutationBatchDepth > 0)
        {
            return true;
        }
        else
        {
            for (const auto& Entry : TrackedPostInitStates)
            {
                if (Entry.Key.IsValid() && EAeonPostInitState::Completed != Entry.Value)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void UAeonAbilitySystemComponent::MarkAbilitySystemReady()
{
    if (bIsTearingDown)
    {
        UE_LOGFMT(LogAeon,
                  Warning,
                  "MarkAbilitySystemReady() invoked after teardown started for AbilitySystemComponent {OwnerActor}",
                  GetNameSafe(GetOwnerActor()));
    }
    else if (bAbilitySystemReady)
    {
        UE_LOGFMT(LogAeon,
                  Warning,
                  "MarkAbilitySystemReady() invoked more than once for AbilitySystemComponent {OwnerActor}",
                  GetNameSafe(GetOwnerActor()));
    }
    else
    {
        bAbilitySystemReady = true;
        DrainPendingPostInitIfNeeded();
    }
}

void UAeonAbilitySystemComponent::BeginAeonMutationBatch()
{
    if (bIsTearingDown)
    {
        UE_LOGFMT(LogAeon,
                  Warning,
                  "BeginAeonMutationBatch() invoked after teardown started for AbilitySystemComponent {OwnerActor}",
                  GetNameSafe(GetOwnerActor()));
        return;
    }

    ++AeonMutationBatchDepth;
}

void UAeonAbilitySystemComponent::EndAeonMutationBatch()
{
    if (!bIsTearingDown)
    {
        if (AeonMutationBatchDepth > 0)
        {
            --AeonMutationBatchDepth;
            if (0 == AeonMutationBatchDepth)
            {
                DrainPendingPostInitIfNeeded();
            }
        }
        else
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "EndAeonMutationBatch() invoked without a matching begin for AbilitySystemComponent {OwnerActor}",
                      GetNameSafe(GetOwnerActor()));
            AeonMutationBatchDepth = 0;
        }
    }
}

void UAeonAbilitySystemComponent::NotifyAttributeSetRegistered(UAttributeSet* const AttributeSet)
{
    if (AeonAbilitySystemComponent::IsOptInPostInitAttributeSet(AttributeSet))
    {
        if (!bIsTearingDown)
        {
            TrackedPostInitStates.FindOrAdd(AttributeSet) = EAeonPostInitState::Pending;

            if (bAbilitySystemReady && 0 == AeonMutationBatchDepth)
            {
                DrainPendingPostInitIfNeeded();
            }
        }
        else
        {
            UE_LOGFMT(
                LogAeon,
                Warning,
                "NotifyAttributeSetRegistered() ignored opt-in AttributeSet {AttributeSet} after teardown started "
                "for AbilitySystemComponent {OwnerActor}",
                GetNameSafe(AttributeSet),
                GetNameSafe(GetOwnerActor()));
        }
    }
}

void UAeonAbilitySystemComponent::NotifyAttributeSetRemoved(UAttributeSet* const AttributeSet)
{
    if (AeonAbilitySystemComponent::IsOptInPostInitAttributeSet(AttributeSet))
    {
        TrackedPostInitStates.Remove(AttributeSet);
    }
}

void UAeonAbilitySystemComponent::RunInitialReadyScanIfNeeded()
{
    if (bAbilitySystemReady && !bHasCompletedInitialReadyScan)
    {
        // The first ready transition is our one chance to reconcile bookkeeping with the ASC's
        // actual registered-set list. After that we intentionally stay incremental so late runtime
        // grants do not pay for repeated full rescans.
        PurgeInvalidPostInitEntries();

        TArray<UAttributeSet*> RegisteredSetsSnapshot;
        RegisteredSetsSnapshot.Reserve(GetSpawnedAttributes().Num());
        for (auto const AttributeSet : GetSpawnedAttributes())
        {
            RegisteredSetsSnapshot.Add(AttributeSet);
        }

        for (auto const AttributeSet : RegisteredSetsSnapshot)
        {
            if (AeonAbilitySystemComponent::IsOptInPostInitAttributeSet(AttributeSet))
            {
                if (!TrackedPostInitStates.Contains(AttributeSet))
                {
                    // Aeon expects opt-in sets to arrive through Aeon-owned registration paths so
                    // that grant/remove lifecycle state stays coherent. We diagnose and skip these
                    // "manually attached" sets instead of trying to guess their lifetime.
                    UE_LOGFMT(
                        LogAeon,
                        Warning,
                        "Ready-time post-init scan found opt-in AttributeSet {AttributeSet} that was not registered "
                        "through Aeon bookkeeping on AbilitySystemComponent {OwnerActor}",
                        GetNameSafe(AttributeSet),
                        GetNameSafe(GetOwnerActor()));
                }
            }
        }

        bHasCompletedInitialReadyScan = true;
    }
}

void UAeonAbilitySystemComponent::DrainPendingPostInitIfNeeded()
{
    if (bAbilitySystemReady && !bIsTearingDown && AeonMutationBatchDepth <= 0 && !bIsDrainingPendingPostInit)
    {
        RunInitialReadyScanIfNeeded();
        bIsDrainingPendingPostInit = true;

        while (!bIsTearingDown)
        {
            PurgeInvalidPostInitEntries();

            // Post-init callbacks are allowed to add or remove sets synchronously. Iterating over
            // a snapshot keeps the current pass stable and lets later passes observe any re-entrant
            // changes without invalidating this traversal.
            TArray<UAttributeSet*> RegisteredSetsSnapshot;
            RegisteredSetsSnapshot.Reserve(GetSpawnedAttributes().Num());
            for (auto const AttributeSet : GetSpawnedAttributes())
            {
                RegisteredSetsSnapshot.Add(AttributeSet);
            }

            bool bRanPostInitThisPass = false;
            for (auto const AttributeSet : RegisteredSetsSnapshot)
            {
                if (AeonAbilitySystemComponent::IsOptInPostInitAttributeSet(AttributeSet))
                {
                    auto const CurrentState = TrackedPostInitStates.Find(AttributeSet);
                    if (CurrentState && EAeonPostInitState::Pending == *CurrentState)
                    {
                        // We move to Initializing before invoking user code so removal or teardown
                        // can distinguish "currently running" from "never attempted".
                        *CurrentState = EAeonPostInitState::Initializing;
                        bRanPostInitThisPass = true;

                        auto const PostInitInterface = Cast<IAeonAbilitySystemPostInitInterface>(AttributeSet);
                        check(PostInitInterface);
                        PostInitInterface->OnAbilitySystemPostInit();

                        if (!bIsTearingDown)
                        {
                            if (IsCurrentlyRegisteredSet(AttributeSet))
                            {
                                auto const PostCallState = TrackedPostInitStates.Find(AttributeSet);
                                if (PostCallState && EAeonPostInitState::Initializing == *PostCallState)
                                {
                                    *PostCallState = EAeonPostInitState::Completed;
                                }
                            }
                            else
                            {
                                TrackedPostInitStates.Remove(AttributeSet);
                            }
                        }
                        if (!bIsTearingDown && AeonMutationBatchDepth > 0)
                        {
                            // A callback opened a new trusted mutation batch (for example by
                            // granting another Aeon ability set). Stop this pass and let the
                            // outermost batch-close trigger a fresh drain once the new structure
                            // is fully committed.
                            break;
                        }
                    }
                }
            }
            if (!bRanPostInitThisPass || AeonMutationBatchDepth > 0)
            {
                break;
            }
        }

        bIsDrainingPendingPostInit = false;
    }
}

void UAeonAbilitySystemComponent::CancelPostInitForTeardown()
{
    if (!bIsTearingDown)
    {
        const bool bHadOutstandingWork = HasOutstandingPostInitWork();
        const bool bInterruptedDispatch = bIsDrainingPendingPostInit;

        // Teardown is terminal for this ASC instance. We intentionally stop accepting new work and
        // clear tracked state instead of trying to "finish" callbacks while owner/avatar teardown
        // is already underway.
        bIsTearingDown = true;
        bIsDrainingPendingPostInit = false;

        if (AeonMutationBatchDepth > 0 || bInterruptedDispatch)
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "CancelPostInitForTeardown() interrupted AbilitySystemComponent {OwnerActor} while "
                      "batch depth was {BatchDepth} and dispatch active was {DispatchActive}",
                      GetNameSafe(GetOwnerActor()),
                      AeonMutationBatchDepth,
                      bInterruptedDispatch);
        }
        else if (bHadOutstandingWork)
        {
            UE_LOGFMT(LogAeon,
                      Verbose,
                      "CancelPostInitForTeardown() cleared outstanding post-init work for AbilitySystemComponent "
                      "{OwnerActor}",
                      GetNameSafe(GetOwnerActor()));
        }
        AeonMutationBatchDepth = 0;
        TrackedPostInitStates.Reset();
    }
}

bool UAeonAbilitySystemComponent::IsTrackedOptInSet(UAttributeSet* const AttributeSet) const
{
    return AeonAbilitySystemComponent::IsOptInPostInitAttributeSet(AttributeSet)
        && TrackedPostInitStates.Contains(AttributeSet);
}

bool UAeonAbilitySystemComponent::IsCurrentlyRegisteredSet(const UAttributeSet* const AttributeSet) const
{
    if (IsValid(AttributeSet))
    {
        for (const auto RegisteredSet : GetSpawnedAttributes())
        {
            if (RegisteredSet == AttributeSet)
            {
                return true;
            }
        }
    }
    return false;
}

void UAeonAbilitySystemComponent::PurgeInvalidPostInitEntries()
{
    for (auto It = TrackedPostInitStates.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

void UAeonAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& Tag, const bool bLogIfUnmatched)
{
    if (ensure(Tag.IsValid()))
    {
        bool bMatched = false;
        for (auto& AbilitySpec : GetActivatableAbilities())
        {
            if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Tag))
            {
                // ReSharper disable once CppTooWideScopeInitStatement
                const bool bAbilitySpecIsActive = AbilitySpec.IsActive();
                if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(AeonGameplayTags::Input_Ability_Toggle)
                    && bAbilitySpecIsActive)
                {
                    CancelAbilityHandle(AbilitySpec.Handle);
                }
                else
                {
                    AbilitySpecInputPressed(AbilitySpec);
                    if (!bAbilitySpecIsActive)
                    {
                        TryActivateAbility(AbilitySpec.Handle);
                    }
                }
                bMatched = true;
            }
        }
        if (bLogIfUnmatched && !bMatched)
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "UAeonAbilitySystemComponent::OnAbilityInputPressed: "
                      "Unable to activate any ability with tag {Tag}",
                      Tag.GetTagName());
        }
    }
    else
    {
        UE_LOGFMT(LogAeon, Warning, "UAeonAbilitySystemComponent::OnAbilityInputPressed: Invalid tag parameter");
    }
}

void UAeonAbilitySystemComponent::OnAbilityInputHeld(const FGameplayTag& Tag, const bool bLogIfUnmatched)
{
    if (ensure(Tag.IsValid()))
    {
        bool bMatched = false;
        for (auto& AbilitySpec : GetActivatableAbilities())
        {
            if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Tag))
            {
                AbilitySpecInputPressed(AbilitySpec);
                if (!AbilitySpec.IsActive())
                {
                    TryActivateAbility(AbilitySpec.Handle);
                }
                bMatched = true;
            }
        }
        if (bLogIfUnmatched && !bMatched)
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "UAeonAbilitySystemComponent::OnAbilityInputHeld: "
                      "Unable to activate any ability with tag {Tag}",
                      Tag.GetTagName());
        }
    }
    else
    {
        UE_LOGFMT(LogAeon, Warning, "UAeonAbilitySystemComponent::OnAbilityInputHeld: Invalid tag parameter");
    }
}

void UAeonAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& Tag, const bool bLogIfUnmatched)
{
    if (ensure(Tag.IsValid()))
    {
        bool bMatched = false;
        for (auto& AbilitySpec : GetActivatableAbilities())
        {
            if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Tag) && AbilitySpec.IsActive())
            {
                AbilitySpecInputReleased(AbilitySpec);

                if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(AeonGameplayTags::Input_Ability_CancelOnRelease))
                {
                    CancelAbilityHandle(AbilitySpec.Handle);
                }
                bMatched = true;
            }
        }
        if (bLogIfUnmatched && !bMatched)
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "UAeonAbilitySystemComponent::OnAbilityInputReleased: "
                      "Unable to release any ability with tag {Tag}",
                      Tag.GetTagName());
        }
    }
    else
    {
        UE_LOGFMT(LogAeon, Warning, "UAeonAbilitySystemComponent::OnAbilityInputReleased: Invalid tag parameter");
    }
}

#pragma region AbilityTagRelationship Support

void UAeonAbilitySystemComponent::SetTagRelationshipMapping(
    UAeonAbilityTagRelationshipMapping* InTagRelationshipMapping)
{
    UE_LOGFMT(LogAeonTagRelationship,
              Log,
              "TagRelationshipMapping changed to {Mapping} for AeonAbilitySystemComponent {OwnerActor}",
              GetNameSafe(InTagRelationshipMapping),
              GetNameSafe(GetOwnerActor()));
    TagRelationshipMapping = InTagRelationshipMapping;
}

static FString ToTagDeltaDiffString(const FGameplayTagContainer& AllTags, const FGameplayTagContainer& OriginalTags)
{
    FGameplayTagContainer Tags(AllTags);
    Tags.RemoveTags(OriginalTags);
    return Tags.ToString();
}

void UAeonAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
                                                                 UGameplayAbility* RequestingAbility,
                                                                 const bool bEnableBlockTags,
                                                                 const FGameplayTagContainer& BlockTags,
                                                                 const bool bExecuteCancelTags,
                                                                 const FGameplayTagContainer& CancelTags)
{
    if (TagRelationshipMapping)
    {
        auto AllBlockTags = BlockTags;
        auto AllCancelTags = CancelTags;
        TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, AllBlockTags, AllCancelTags);

        UE_LOGFMT(LogAeonTagRelationship,
                  Verbose,
                  "ApplyAbilityBlockAndCancelTags for ability defined "
                  "by tags {Tags} added {BlockTags} block tags and {CancelTags} cancel tags for actor '{OwnerActor}'",
                  AbilityTags.ToString(),
                  ToTagDeltaDiffString(AllBlockTags, BlockTags),
                  ToTagDeltaDiffString(AllCancelTags, CancelTags),
                  GetNameSafe(GetOwnerActor()));

        Super::ApplyAbilityBlockAndCancelTags(AbilityTags,
                                              RequestingAbility,
                                              bEnableBlockTags,
                                              AllBlockTags,
                                              bExecuteCancelTags,
                                              AllCancelTags);
    }
    else
    {
        Super::ApplyAbilityBlockAndCancelTags(AbilityTags,
                                              RequestingAbility,
                                              bEnableBlockTags,
                                              BlockTags,
                                              bExecuteCancelTags,
                                              CancelTags);
    }
}

void UAeonAbilitySystemComponent::GetAdditionalTagRequirements(const FGameplayTagContainer& AbilityTags,
                                                               FGameplayTagContainer& OutActivationRequiredTags,
                                                               FGameplayTagContainer& OutActivationBlockedTags,
                                                               FGameplayTagContainer& OutSourceRequiredTags,
                                                               FGameplayTagContainer& OutSourceBlockedTags,
                                                               FGameplayTagContainer& OutTargetRequiredTags,
                                                               FGameplayTagContainer& OutTargetBlockedTags) const
{
    if (TagRelationshipMapping)
    {
        TagRelationshipMapping->GetAdditionalTagRequirements(AbilityTags,
                                                             OutActivationRequiredTags,
                                                             OutActivationBlockedTags,
                                                             OutSourceRequiredTags,
                                                             OutSourceBlockedTags,
                                                             OutTargetRequiredTags,
                                                             OutTargetBlockedTags);
        UE_LOGFMT(LogAeonTagRelationship,
                  Verbose,
                  "GetAdditionalTagRequirements for ability defined by tags {AbilityTags} for actor '{OwnerActor}': "
                  "ActivationRequiredTags={ActivationRequiredTags} "
                  "ActivationBlockedTags={ActivationBlockedTags} "
                  "SourceRequiredTags={SourceRequiredTags} "
                  "SourceBlockedTags={SourceBlockedTags} "
                  "TargetRequiredTags={TargetRequiredTags} "
                  "TargetBlockedTags={TargetBlockedTags}",
                  AbilityTags.ToString(),
                  GetNameSafe(GetOwnerActor()),
                  OutActivationRequiredTags.ToString(),
                  OutActivationBlockedTags.ToString(),
                  OutSourceRequiredTags.ToString(),
                  OutSourceBlockedTags.ToString(),
                  OutTargetRequiredTags.ToString(),
                  OutTargetBlockedTags.ToString());
    }
}

#pragma endregion
