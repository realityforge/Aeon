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

namespace AeonGameplayAbilityTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityTag, "Aeon.Test.GameplayAbility.Tag");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestActivationRequiredTag, "Aeon.Test.GameplayAbility.Required");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestMappedRequiredTag, "Aeon.Test.GameplayAbility.MappedRequired");

    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor =
                AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(Test,
                                                                                OutWorld,
                                                                                TEXT("Ability-system test actor"),
                                                                                TEXT("AeonGameplayAbilityTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    FGameplayTagContainer CreateActivateOnGivenTags()
    {
        FGameplayTagContainer Tags;
        Tags.AddTag(AeonGameplayTags::Aeon_Ability_Trait_ActivateOnGiven);
        return Tags;
    }
} // namespace AeonGameplayAbilityTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityDoesAbilitySatisfyTagRequirementsWithoutMappingTest,
                                 "Aeon.GameplayAbility.DoesAbilitySatisfyTagRequirements.WithoutMapping",
                                 AeonTests::AutomationTestFlags);
bool FAeonGameplayAbilityDoesAbilitySatisfyTagRequirementsWithoutMappingTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonGameplayAbilityTests::CreateAbilitySystemActor(*this, World))
    {
        const auto Ability = GetMutableDefault<UAeonAutomationTestGameplayAbility>();
        UAeonAutomationTestGameplayAbility::ResetMutableStateForTest();
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(AeonGameplayAbilityTests::TestAbilityTag.GetTag());
        FGameplayTagContainer RequiredTags;
        RequiredTags.AddTag(AeonGameplayAbilityTests::TestActivationRequiredTag.GetTag());
        Ability->SetAssetTagsForTest(AbilityTags);
        Ability->SetActivationRequiredTagsForTest(RequiredTags);

        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        AbilitySystemComponent->SetLooseGameplayTagCount(AeonGameplayAbilityTests::TestActivationRequiredTag.GetTag(),
                                                         1);

        return TestTrue(TEXT("Gameplay ability should satisfy its base activation required tags"),
                        Ability->DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityDoesAbilitySatisfyTagRequirementsWithMappingExpansionTest,
                                 "Aeon.GameplayAbility.DoesAbilitySatisfyTagRequirements.WithMappingExpansion",
                                 AeonTests::AutomationTestFlags);
bool FAeonGameplayAbilityDoesAbilitySatisfyTagRequirementsWithMappingExpansionTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonGameplayAbilityTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Mapping = AeonTests::NewTransientObject<UAeonAbilityTagRelationshipMapping>();
        UAeonAutomationTestGameplayAbility::ResetMutableStateForTest();
        FAeonAbilityTagRelationship Relationship;
        Relationship.AbilityTag = AeonGameplayAbilityTests::TestAbilityTag.GetTag();
        Relationship.ActivationRequiredTags.AddTag(AeonGameplayAbilityTests::TestMappedRequiredTag.GetTag());
        TArray<FAeonAbilityTagRelationship> Relationships;
        Relationships.Add(Relationship);
        if (!TestTrue(TEXT("Tag relationship mapping should be configured"),
                      AeonTests::SetPropertyValue(*this, Mapping, TEXT("AbilityTagRelationships"), Relationships)))
        {
            return false;
        }

        AbilitySystemComponent->SetTagRelationshipMapping(Mapping);
        const auto Ability = GetMutableDefault<UAeonAutomationTestGameplayAbility>();
        FGameplayTagContainer AbilityTags;
        AbilityTags.AddTag(AeonGameplayAbilityTests::TestAbilityTag.GetTag());
        Ability->SetAssetTagsForTest(AbilityTags);
        Ability->SetActivationRequiredTagsForTest(FGameplayTagContainer());

        const bool bWithoutMappedTag = Ability->DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent);
        AbilitySystemComponent->SetLooseGameplayTagCount(AeonGameplayAbilityTests::TestMappedRequiredTag.GetTag(), 1);
        const bool bWithMappedTag = Ability->DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent);

        return TestFalse(TEXT("Mapped required tags should prevent activation until the mapped tag is present"),
                         bWithoutMappedTag)
            && TestTrue(TEXT("Mapped required tags should allow activation once the mapped tag is present"),
                        bWithMappedTag);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityOnGiveAbilityActivateOnGivenActivatesAbilityTest,
                                 "Aeon.GameplayAbility.OnGiveAbility.ActivateOnGivenActivatesAbility",
                                 AeonTests::AutomationTestFlags);
bool FAeonGameplayAbilityOnGiveAbilityActivateOnGivenActivatesAbilityTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonGameplayAbilityTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestGameplayAbility::ResetCounters();
        const auto Ability = GetMutableDefault<UAeonAutomationTestGameplayAbility>();
        Ability->SetAssetTagsForTest(AeonGameplayAbilityTests::CreateActivateOnGivenTags());
        const FGameplayAbilitySpec Spec(UAeonAutomationTestGameplayAbility::StaticClass());
        const auto Handle = Actor->GetAeonAbilitySystemComponent()->GiveAbility(Spec);
        const auto AbilitySpec = Actor->GetAeonAbilitySystemComponent()->FindAbilitySpecFromHandle(Handle);
        const bool bResult = TestNotNull(TEXT("Activate-on-given ability spec should be granted"), AbilitySpec)
            && TestTrue(TEXT("Activate-on-given ability should become active when granted"), AbilitySpec->IsActive())
            && TestEqual(TEXT("Activate-on-given should activate the counting test ability once"),
                         UAeonAutomationTestGameplayAbility::ActivationCount,
                         1);
        UAeonAutomationTestGameplayAbility::ResetMutableStateForTest();
        return bResult;
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityEndAbilityClearsActivateOnGivenSpecTest,
                                 "Aeon.GameplayAbility.EndAbility.ClearsActivateOnGivenSpec",
                                 AeonTests::AutomationTestFlags);
bool FAeonGameplayAbilityEndAbilityClearsActivateOnGivenSpecTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonGameplayAbilityTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestGameplayAbility::ResetCounters();
        const auto Ability = GetMutableDefault<UAeonAutomationTestGameplayAbility>();
        Ability->SetAssetTagsForTest(AeonGameplayAbilityTests::CreateActivateOnGivenTags());
        const FGameplayAbilitySpec Spec(UAeonAutomationTestGameplayAbility::StaticClass());
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto Handle = AbilitySystemComponent->GiveAbility(Spec);
        if (TestTrue(TEXT("Activate-on-given ability should be active after being granted"),
                     AbilitySystemComponent->FindAbilitySpecFromHandle(Handle)
                         && AbilitySystemComponent->FindAbilitySpecFromHandle(Handle)->IsActive()))
        {
            AbilitySystemComponent->CancelAbilityHandle(Handle);
            const bool bResult =
                TestNull(TEXT("Activate-on-given ability should clear itself from the ASC when it ends"),
                         AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
                && TestEqual(TEXT("Cancelling the activate-on-given ability should end it once"),
                             UAeonAutomationTestGameplayAbility::EndCount,
                             1);
            UAeonAutomationTestGameplayAbility::ResetMutableStateForTest();
            return bResult;
        }
        else
        {
            UAeonAutomationTestGameplayAbility::ResetMutableStateForTest();
            return false;
        }
    }
    else
    {
        return false;
    }
}

#endif
