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

#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "Templates/SubclassOf.h"
#include "GroupAbilitySystem.generated.h"

#define UE_API AEON_API

class UAbilitySystemComponent;
class UGroupAbilitySystemComponentRegistry;
class UGameplayAbility;
class UGameplayEffect;
class UObject;

/**
 * A world subsystem that allows you to apply Abilities and Effects to groups of ASCs registered with the subsystem.
 *
 * Groups are defined using hierarchical GameplayTags. If an ASC is added to a group with a particular tag, then
 * they are also considered members of groups defined using parent tags.
 *
 * Abilities and effects will be applied or removed when they associated Apply or Remove methods are invoked.
 * Existing global abilities and effects are also applied when an ASC is registered and removed when the ASC
 * is deregistered.
 *
 * NOTE: This is **currently** supported on nodes that have authority. Support may be added for clients in the future.
 *
 * @note {This has been somewhat inspired by the ULyraGlobalAbilitySystem system in Lyra}
 */
UCLASS(MinimalAPI)
class UGroupAbilitySystem final : public UWorldSubsystem
{
    GENERATED_BODY()

    /** A map of effects applied globally. */
    UPROPERTY()
    TMap<FGameplayTag, TObjectPtr<UGroupAbilitySystemComponentRegistry>> Registry;

    /** Return true when the local world has network authority. */
    bool HasNetworkAuthority() const;

    /**
     * Find or create a group registry for the group with the specified tag.
     *
     * @param GroupTag the tag to identify the group
     * @return the registry
     */
    UGroupAbilitySystemComponentRegistry& FindOrCreateGroupRegistry(const FGameplayTag& GroupTag);

    /**
     * Find a group registry for the group with the specified tag.
     *
     * @param GroupTag the tag to identify the group
     * @return the registry if present, else null
     */
    UGroupAbilitySystemComponentRegistry* FindGroupRegistry(FGameplayTag GroupTag) const;

protected:
    virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

public:
    /**
     * Return true if the AbilitySystemComponent is registered with the specified group.
     *
     * @param  GroupTag The tag identifying the group.
     * @param  AbilitySystemComponent The AbilitySystemComponent to register.
     * @param bImplicit A flag indicating whether we are looking for explicit registrations or implicit registrations.
     *                  (Implicit registrations are those due to an explicit registration of a child group.)
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API bool IsAbilitySystemComponentRegistered(FGameplayTag GroupTag,
                                                   UAbilitySystemComponent* AbilitySystemComponent,
                                                   const bool bImplicit = false) const;

    /**
     * Register an ASC with the group and apply any active group abilities and/or effects.
     *
     * @param  GroupTag The tag identifying the group.
     * @param  AbilitySystemComponent The AbilitySystemComponent to register.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API void RegisterAbilitySystemComponent(FGameplayTag GroupTag, UAbilitySystemComponent* AbilitySystemComponent);

    /**
     * Remove an ASC from the group, deactivate any active group abilities and/or effects.
     *
     * @param  GroupTag The tag identifying the group.
     * @param  AbilitySystemComponent The AbilitySystemComponent to unregister.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API void UnregisterAbilitySystemComponent(FGameplayTag GroupTag,
                                                 UAbilitySystemComponent* AbilitySystemComponent);

    /**
     * Test if the specified ability has been added to the group.
     *
     * @param GroupTag The GameplayTag identifying the group. Must be valid.
     * @param Ability the Ability to test
     * @return true if the ability has been added to the group.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API bool IsAbilityAddedToGroup(FGameplayTag GroupTag, TSubclassOf<UGameplayAbility> Ability) const;

    /**
     * Apply the specified ability to the specified group.
     * If any ASCs are registered with the group, then apply the ability to the ASC.
     *
     * @param GroupTag The GameplayTag identifying the group. Must be valid.
     * @param Ability the Ability to add.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API void AddAbilityToGroup(FGameplayTag GroupTag, TSubclassOf<UGameplayAbility> Ability);

    /**
     * Remove the specified ability from the group.
     * If any ASCs are registered with the group, then remove the ability from the ASC.
     *
     * @param GroupTag The GameplayTag identifying the group. Must be valid.
     * @param Ability the Ability to remove
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API void RemoveAbilityFromGroup(FGameplayTag GroupTag, TSubclassOf<UGameplayAbility> Ability);

    /**
     * Test if the specified effect has been added to the group.
     *
     * @param GroupTag The GameplayTag identifying the group. Must be valid.
     * @param Effect the Effect to test
     * @return true if the effect has been added to the group.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API bool IsEffectAddedToGroup(FGameplayTag GroupTag, TSubclassOf<UGameplayEffect> Effect) const;

    /**
     * Add the specified effect to the group.
     * If any ASCs are registered with the group, then apply the effect to the ASC.
     *
     * @param GroupTag The GameplayTag identifying the group. Must be valid.
     * @param Effect the Effect to add
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API void AddEffectToGroup(FGameplayTag GroupTag, TSubclassOf<UGameplayEffect> Effect);

    /**
     * Remove the specified effect from the group.
     * If any ASCs are registered with the group, then remove the effect from the ASC.
     *
     * @param GroupTag The GameplayTag identifying the group. Must be valid.
     * @param Effect the Effect to remove
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Aeon")
    UE_API void RemoveEffectFromGroup(FGameplayTag GroupTag, TSubclassOf<UGameplayEffect> Effect);

    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
};

#undef UE_API
