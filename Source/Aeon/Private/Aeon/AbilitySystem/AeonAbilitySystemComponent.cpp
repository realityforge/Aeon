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
#include "Aeon/AbilitySystem/AeonAbilityTagRelationshipMapping.h"

#pragma region AbilityTagRelationship Support

// --------------------------------------------------- //
// AbilityTagRelationship Support
// --------------------------------------------------- //

void UAeonAbilitySystemComponent::SetTagRelationshipMapping(
    UAeonAbilityTagRelationshipMapping* InTagRelationshipMapping)
{
    TagRelationshipMapping = InTagRelationshipMapping;
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
        FGameplayTagContainer AllBlockTags = BlockTags;
        FGameplayTagContainer AllCancelTags = CancelTags;
        TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, AllBlockTags, AllCancelTags);
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

void UAeonAbilitySystemComponent::GetRequiredAndBlockedActivationTags(
    const FGameplayTagContainer& AbilityTags,
    FGameplayTagContainer& OutActivationRequiredTags,
    FGameplayTagContainer& OutActivationBlockedTags) const
{
    if (TagRelationshipMapping)
    {
        TagRelationshipMapping->GetActivationRequiredAndBlockedTags(AbilityTags,
                                                                    OutActivationRequiredTags,
                                                                    OutActivationBlockedTags);
    }
}

// --------------------------------------------------- //

#pragma endregion
