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
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "Aeon/AbilitySystem/AeonAbilityTagRelationshipMapping.h"
    #include "Aeon/AeonGameplayTags.h"
    #include "GameplayAbilitySpec.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonAbilitySystemComponentTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestInputTag, "Aeon.Test.AbilitySystemComponent.Input");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestMappedAbilityTag, "Aeon.Test.AbilitySystemComponent.Mapping.Ability");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestMappedBlockTag, "Aeon.Test.AbilitySystemComponent.Mapping.Block");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestMappedCancelTag, "Aeon.Test.AbilitySystemComponent.Mapping.Cancel");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestMappedRequiredTag, "Aeon.Test.AbilitySystemComponent.Mapping.Required");

    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor = AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(
                Test,
                OutWorld,
                TEXT("Ability-system test actor"),
                TEXT("AeonAbilitySystemComponentTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    FGameplayAbilitySpecHandle
    GrantInputTaggedAbility(UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent,
                            const FGameplayTag& InputTag,
                            const bool bToggle = false,
                            const bool bCancelOnRelease = false,
                            const FGameplayTagContainer* AssetTags = nullptr)
    {
        check(AbilitySystemComponent);
        UAeonAutomationTestGameplayAbility::ResetCounters();
        GetMutableDefault<UAeonAutomationTestGameplayAbility>()->SetAssetTagsForTest(
            AssetTags ? *AssetTags : FGameplayTagContainer());

        FGameplayAbilitySpec Spec(UAeonAutomationTestGameplayAbility::StaticClass());
        Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
        if (bToggle)
        {
            Spec.GetDynamicSpecSourceTags().AddTag(AeonGameplayTags::Input_Ability_Toggle);
        }
        if (bCancelOnRelease)
        {
            Spec.GetDynamicSpecSourceTags().AddTag(AeonGameplayTags::Input_Ability_CancelOnRelease);
        }

        return AbilitySystemComponent->GiveAbility(Spec);
    }

} // namespace AeonAbilitySystemComponentTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentInputPressedActivatesMatchingAbilityTest,
                                 "Aeon.AbilitySystemComponent.InputPressed.ActivatesMatchingAbility",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentInputPressedActivatesMatchingAbilityTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Handle = AeonAbilitySystemComponentTests::GrantInputTaggedAbility(
            AbilitySystemComponent,
            AeonAbilitySystemComponentTests::TestInputTag.GetTag());
        const auto AbilitySpecBefore = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
        if (TestNotNull(TEXT("Granted input ability should exist"), AbilitySpecBefore))
        {
            AbilitySystemComponent->OnAbilityInputPressed(AeonAbilitySystemComponentTests::TestInputTag.GetTag(),
                                                          false);

            const auto AbilitySpecAfter = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
            return TestNotNull(TEXT("Granted input ability should remain after input pressed"), AbilitySpecAfter)
                && TestTrue(TEXT("Input pressed should activate the matching ability"), AbilitySpecAfter->IsActive())
                && TestTrue(TEXT("Input pressed should mark the granted ability spec as pressed"),
                            AbilitySpecAfter->InputPressed)
                && TestEqual(TEXT("Input pressed should activate the counting test ability exactly once"),
                             UAeonAutomationTestGameplayAbility::ActivationCount,
                             1);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentInputHeldActivatesMatchingAbilityTest,
                                 "Aeon.AbilitySystemComponent.InputHeld.ActivatesMatchingAbility",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentInputHeldActivatesMatchingAbilityTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Handle = AeonAbilitySystemComponentTests::GrantInputTaggedAbility(
            AbilitySystemComponent,
            AeonAbilitySystemComponentTests::TestInputTag.GetTag());

        AbilitySystemComponent->OnAbilityInputHeld(AeonAbilitySystemComponentTests::TestInputTag.GetTag(), false);

        const auto AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
        return TestNotNull(TEXT("Granted input ability should remain after input held"), AbilitySpec)
            && TestTrue(TEXT("Input held should activate the matching ability"), AbilitySpec->IsActive())
            && TestEqual(TEXT("Input held should activate the counting test ability exactly once"),
                         UAeonAutomationTestGameplayAbility::ActivationCount,
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentToggleInputPressedCancelsActiveAbilityTest,
                                 "Aeon.AbilitySystemComponent.InputPressed.ToggleCancelsActiveAbility",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentToggleInputPressedCancelsActiveAbilityTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Handle = AeonAbilitySystemComponentTests::GrantInputTaggedAbility(
            AbilitySystemComponent,
            AeonAbilitySystemComponentTests::TestInputTag.GetTag(),
            true);
        if (TestTrue(TEXT("Toggle test ability should activate"), AbilitySystemComponent->TryActivateAbility(Handle)))
        {
            AbilitySystemComponent->OnAbilityInputPressed(AeonAbilitySystemComponentTests::TestInputTag.GetTag(),
                                                          false);

            const auto AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
            return TestNotNull(TEXT("Toggle ability spec should remain after cancellation"), AbilitySpec)
                && TestFalse(TEXT("Pressing a toggle input on an active ability should cancel it"),
                             AbilitySpec->IsActive())
                && TestEqual(TEXT("Toggle cancellation should end the counting ability once"),
                             UAeonAutomationTestGameplayAbility::EndCount,
                             1);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentInputReleasedCancelOnReleaseCancelsAbilityTest,
                                 "Aeon.AbilitySystemComponent.InputReleased.CancelOnReleaseCancelsAbility",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentInputReleasedCancelOnReleaseCancelsAbilityTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Handle = AeonAbilitySystemComponentTests::GrantInputTaggedAbility(
            AbilitySystemComponent,
            AeonAbilitySystemComponentTests::TestInputTag.GetTag(),
            false,
            true);
        if (TestTrue(TEXT("Cancel-on-release test ability should activate"),
                     AbilitySystemComponent->TryActivateAbility(Handle)))
        {
            AbilitySystemComponent->OnAbilityInputReleased(AeonAbilitySystemComponentTests::TestInputTag.GetTag(),
                                                           false);

            const auto AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
            return TestNotNull(TEXT("Cancel-on-release ability spec should remain after cancellation"), AbilitySpec)
                && TestFalse(TEXT("Releasing a cancel-on-release input should cancel the active ability"),
                             AbilitySpec->IsActive());
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonAbilitySystemComponentGetAdditionalTagRequirementsUsesMappingTest,
    "Aeon.AbilitySystemComponent.TagRelationshipMapping.GetAdditionalTagRequirementsUsesMapping",
    AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentGetAdditionalTagRequirementsUsesMappingTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Mapping = AeonTests::NewTransientObject<UAeonAbilityTagRelationshipMapping>();
        FAeonAbilityTagRelationship Relationship;
        Relationship.AbilityTag = AeonAbilitySystemComponentTests::TestMappedAbilityTag.GetTag();
        Relationship.ActivationRequiredTags.AddTag(AeonAbilitySystemComponentTests::TestMappedRequiredTag.GetTag());
        TArray<FAeonAbilityTagRelationship> Relationships;
        Relationships.Add(Relationship);
        if (TestTrue(TEXT("Tag relationship mapping should be configured"),
                     AeonTests::SetPropertyValue(*this, Mapping, TEXT("AbilityTagRelationships"), Relationships)))
        {
            AbilitySystemComponent->SetTagRelationshipMapping(Mapping);

            FGameplayTagContainer OutActivationRequiredTags;
            FGameplayTagContainer OutActivationBlockedTags;
            FGameplayTagContainer OutSourceRequiredTags;
            FGameplayTagContainer OutSourceBlockedTags;
            FGameplayTagContainer OutTargetRequiredTags;
            FGameplayTagContainer OutTargetBlockedTags;

            AbilitySystemComponent->GetAdditionalTagRequirements(
                AeonAbilitySystemComponentTests::TestMappedAbilityTag.GetTag().GetSingleTagContainer(),
                OutActivationRequiredTags,
                OutActivationBlockedTags,
                OutSourceRequiredTags,
                OutSourceBlockedTags,
                OutTargetRequiredTags,
                OutTargetBlockedTags);

            return TestTrue(
                TEXT("Tag relationship mapping should contribute the configured activation required tag"),
                OutActivationRequiredTags.HasTagExact(AeonAbilitySystemComponentTests::TestMappedRequiredTag.GetTag()));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonAbilitySystemComponentApplyAbilityBlockAndCancelTagsUsesMappingTest,
    "Aeon.AbilitySystemComponent.TagRelationshipMapping.ApplyAbilityBlockAndCancelTagsUsesMapping",
    AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentApplyAbilityBlockAndCancelTagsUsesMappingTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();

        const auto Mapping = AeonTests::NewTransientObject<UAeonAbilityTagRelationshipMapping>();
        FAeonAbilityTagRelationship Relationship;
        Relationship.AbilityTag = AeonAbilitySystemComponentTests::TestMappedAbilityTag.GetTag();
        Relationship.AbilityTagsToBlock.AddTag(AeonAbilitySystemComponentTests::TestMappedBlockTag.GetTag());
        Relationship.AbilityTagsToCancel.AddTag(AeonAbilitySystemComponentTests::TestMappedCancelTag.GetTag());
        TArray<FAeonAbilityTagRelationship> Relationships;
        Relationships.Add(Relationship);
        if (TestTrue(TEXT("Tag relationship mapping should be configured"),
                     AeonTests::SetPropertyValue(*this, Mapping, TEXT("AbilityTagRelationships"), Relationships)))
        {
            AbilitySystemComponent->SetTagRelationshipMapping(Mapping);

            FGameplayTagContainer CancelAbilityTags;
            CancelAbilityTags.AddTag(AeonAbilitySystemComponentTests::TestMappedCancelTag.GetTag());
            const auto CancelHandle = AeonAbilitySystemComponentTests::GrantInputTaggedAbility(
                AbilitySystemComponent,
                AeonAbilitySystemComponentTests::TestInputTag.GetTag(),
                false,
                false,
                &CancelAbilityTags);
            if (TestTrue(TEXT("Cancel-target ability should activate"),
                         AbilitySystemComponent->TryActivateAbility(CancelHandle)))
            {
                const FGameplayTagContainer EmptyTags;
                FGameplayTagContainer AbilityTags;
                AbilityTags.AddTag(AeonAbilitySystemComponentTests::TestMappedAbilityTag.GetTag());
                AbilitySystemComponent
                    ->ApplyAbilityBlockAndCancelTags(AbilityTags, nullptr, true, EmptyTags, true, EmptyTags);

                const auto CancelSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(CancelHandle);
                return TestTrue(TEXT("Mapping should apply the configured block tag to the ASC"),
                                AbilitySystemComponent->GetBlockedAbilityTags().HasTagExact(
                                    AeonAbilitySystemComponentTests::TestMappedBlockTag.GetTag()))
                    && TestNotNull(TEXT("Cancel-target ability spec should remain after cancellation"), CancelSpec)
                    && TestFalse(TEXT("Mapping should cancel active abilities with the configured cancel tag"),
                                 CancelSpec->IsActive());
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif
