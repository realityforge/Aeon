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

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "AeonAbilitySystemComponent.generated.h"

#define UE_API AEON_API

class UAeonAbilityTagRelationshipMapping;
class UAttributeSet;
class UAeonAbilitySet;
struct FAeonAbilitySetHandles;
class AAeonCharacterBase;

enum class EAeonPostInitState : uint8
{
    /** The set is registered and waiting for its one-shot post-init callback. */
    Pending,
    /** The callback is currently running, so removal or teardown can react without double-completing it. */
    Initializing,
    /** This registration lifetime has already consumed its post-init attempt. */
    Completed
};

/** The AbilitySystemComponent specialization used in Aeon */
UCLASS()
class UE_API UAeonAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    /**
     * Return whether Aeon's startup lifecycle has marked this Ability System ready.
     *
     * <p>This becomes true once startup configuration is complete and stays true for the remainder of the ASC instance
     * lifetime, including during teardown.</p>
     */
    bool IsAbilitySystemReady() const;

    /**
     * Return whether a ready Ability System still has Aeon-managed post-init work to drain.
     *
     * <p>This reports true while a mutation batch is open, while tracked post-init sets are pending, or while a
     * post-init callback is currently running. It reports false before readiness and after teardown begins.</p>
     */
    bool HasOutstandingPostInitWork() const;

    /**
     * Callback invoked when an AbilityInputAction has associated Input pressed.
     *
     * @param Tag The InputTag that identifies the ability.
     * @param bLogIfUnmatched A flag that controls whether a log message is emitted if no ability matches the Tag.
     * @see UAeonInputConfig for where AbilityInputAction are defined
     */
    void OnAbilityInputPressed(const FGameplayTag& Tag, bool bLogIfUnmatched = true);

    /**
     * Callback invoked when an AbilityInputAction has associated Input held.
     *
     * @param Tag The InputTag that identifies the ability.
     * @param bLogIfUnmatched A flag that controls whether a log message is emitted if no ability matches the Tag.
     * @see UAeonInputConfig for where AbilityInputAction are defined
     */
    void OnAbilityInputHeld(const FGameplayTag& Tag, bool bLogIfUnmatched = true);

    /**
     * Callback invoked when an AbilityInputAction has associated Input released.
     *
     * @param Tag The InputTag that identifies the ability.
     * @param bLogIfUnmatched A flag that controls whether a log message is emitted if no ability matches the Tag.
     * @see UAeonInputConfig for where AbilityInputAction are defined
     */
    void OnAbilityInputReleased(const FGameplayTag& Tag, bool bLogIfUnmatched = true);

#pragma region AbilityTagRelationship Support

private:
    /**
     * An optional structure used to explicitly define relationships between tags.
     *
     * So that we can define additional activation tag requirement tags, blocking and canceling tags.
     * See @link UAeonAbilityTagRelationshipMapping @endlink for further details.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Aeon|Ability")
    TObjectPtr<UAeonAbilityTagRelationshipMapping> TagRelationshipMapping;

public:
    /**
     * Set the tag relationship mapping.
     *
     * @param InTagRelationshipMapping The new tag relationship mapping. The parameter may be null.
     */
    void SetTagRelationshipMapping(UAeonAbilityTagRelationshipMapping* InTagRelationshipMapping);

    virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
                                                UGameplayAbility* RequestingAbility,
                                                bool bEnableBlockTags,
                                                const FGameplayTagContainer& BlockTags,
                                                bool bExecuteCancelTags,
                                                const FGameplayTagContainer& CancelTags) override;

    /**
     * Collect additional ActivationRequired and ActivationBlocked tags for the specified AbilityTags.
     *
     * @param  AbilityTags The Ability Tags to lookup.
     * @param  OutActivationRequiredTags The container in which to add additional ActivationRequired tags.
     * @param  OutActivationBlockedTags The container in which to add additional ActivationBlocked tags.
     * @param  OutSourceRequiredTags The container in which to add additional SourceRequired tags.
     * @param  OutSourceBlockedTags The container in which to add additional SourceBlocked tags.
     * @param  OutTargetRequiredTags The container in which to add additional TargetRequired tags.
     * @param  OutTargetBlockedTags The container in which to add additional TargetBlocked tags.
     */
    void GetAdditionalTagRequirements(const FGameplayTagContainer& AbilityTags,
                                      FGameplayTagContainer& OutActivationRequiredTags,
                                      FGameplayTagContainer& OutActivationBlockedTags,
                                      FGameplayTagContainer& OutSourceRequiredTags,
                                      FGameplayTagContainer& OutSourceBlockedTags,
                                      FGameplayTagContainer& OutTargetRequiredTags,
                                      FGameplayTagContainer& OutTargetBlockedTags) const;

#pragma endregion

private:
    friend class UAeonAbilitySet;
    friend struct FAeonAbilitySetHandles;

protected:
    friend class AAeonCharacterBase;

    /*
     * Internal Aeon lifecycle hooks for trusted callers only.
     *
     * <p>These methods form the ASC-side
     * state machine for automatic AttributeSet post-init.
     * Production code reaches them through Aeon character
     * startup/teardown and Aeon ability-set
     * grant or removal flows rather than by calling them directly.</p>
     */
    void MarkAbilitySystemReady();
    void BeginAeonMutationBatch();
    void EndAeonMutationBatch();

    void NotifyAttributeSetRegistered(UAttributeSet* AttributeSet);
    void NotifyAttributeSetRemoved(UAttributeSet* AttributeSet);

    void RunInitialReadyScanIfNeeded();
    void DrainPendingPostInitIfNeeded();
    void CancelPostInitForTeardown();

private:
    bool IsTrackedOptInSet(UAttributeSet* AttributeSet) const;
    bool IsCurrentlyRegisteredSet(const UAttributeSet* AttributeSet) const;
    void PurgeInvalidPostInitEntries();

    /** Sticky once startup completes; intentionally remains true during teardown for stable external queries. */
    UPROPERTY(Transient)
    bool bAbilitySystemReady{ false };

    /** Ensures the ready-time bookkeeping validation scan runs only once per ASC instance. */
    UPROPERTY(Transient)
    bool bHasCompletedInitialReadyScan{ false };

    /** Terminal flag that turns later lifecycle notifications into diagnosed no-ops. */
    UPROPERTY(Transient)
    bool bIsTearingDown{ false };

    /** Re-entrancy guard so nested grants schedule later work instead of recursively re-entering the drain loop. */
    UPROPERTY(Transient)
    bool bIsDrainingPendingPostInit{ false };

    /** Nesting depth for Aeon-owned structural ASC mutations; post-init drains after it reaches 0. */
    UPROPERTY(Transient)
    int32 AeonMutationBatchDepth{ 0 };

    /** Tracks only opt-in sets, keyed by instance so re-registering the same object creates a new lifetime. */
    TMap<TWeakObjectPtr<UAttributeSet>, EAeonPostInitState> TrackedPostInitStates;
};

#undef UE_API
