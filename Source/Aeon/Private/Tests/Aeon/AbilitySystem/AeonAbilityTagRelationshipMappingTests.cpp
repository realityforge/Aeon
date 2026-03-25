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
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"

namespace AeonAbilityTagRelationshipMappingTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestPrimaryAbilityTag, "Aeon.Test.Ability.Primary");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestSecondaryAbilityTag, "Aeon.Test.Ability.Secondary");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestUnrelatedAbilityTag, "Aeon.Test.Ability.Unrelated");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestExistingBlockTag, "Aeon.Test.Ability.Block.Existing");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestExistingCancelTag, "Aeon.Test.Ability.Cancel.Existing");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestBlockTagA, "Aeon.Test.Ability.Block.A");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestBlockTagB, "Aeon.Test.Ability.Block.B");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestCancelTagA, "Aeon.Test.Ability.Cancel.A");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestCancelTagB, "Aeon.Test.Ability.Cancel.B");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestActivationRequiredTag, "Aeon.Test.Activation.Required");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestActivationBlockedTag, "Aeon.Test.Activation.Blocked");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestSourceRequiredTag, "Aeon.Test.Source.Required");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestSourceBlockedTag, "Aeon.Test.Source.Blocked");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestTargetRequiredTag, "Aeon.Test.Target.Required");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestTargetBlockedTag, "Aeon.Test.Target.Blocked");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestExistingRequirementTag, "Aeon.Test.Requirement.Existing");

    FAeonAbilityTagRelationship
    MakeRelationship(const FGameplayTag AbilityTag,
                     const FGameplayTagContainer& AbilityTagsToBlock = FGameplayTagContainer(),
                     const FGameplayTagContainer& AbilityTagsToCancel = FGameplayTagContainer(),
                     const FGameplayTagContainer& ActivationRequiredTags = FGameplayTagContainer(),
                     const FGameplayTagContainer& ActivationBlockedTags = FGameplayTagContainer(),
                     const FGameplayTagContainer& SourceRequiredTags = FGameplayTagContainer(),
                     const FGameplayTagContainer& SourceBlockedTags = FGameplayTagContainer(),
                     const FGameplayTagContainer& TargetRequiredTags = FGameplayTagContainer(),
                     const FGameplayTagContainer& TargetBlockedTags = FGameplayTagContainer())
    {
        FAeonAbilityTagRelationship Relationship;
        Relationship.AbilityTag = AbilityTag;
        Relationship.AbilityTagsToBlock = AbilityTagsToBlock;
        Relationship.AbilityTagsToCancel = AbilityTagsToCancel;
        Relationship.ActivationRequiredTags = ActivationRequiredTags;
        Relationship.ActivationBlockedTags = ActivationBlockedTags;
        Relationship.SourceRequiredTags = SourceRequiredTags;
        Relationship.SourceBlockedTags = SourceBlockedTags;
        Relationship.TargetRequiredTags = TargetRequiredTags;
        Relationship.TargetBlockedTags = TargetBlockedTags;
        return Relationship;
    }

    UAeonAbilityTagRelationshipMapping* CreateMapping(FAutomationTestBase& Test,
                                                      const TArray<FAeonAbilityTagRelationship>& Relationships)
    {
        const auto Mapping = AeonTests::NewTransientObject<UAeonAbilityTagRelationshipMapping>();
        Test.TestNotNull(TEXT("Tag relationship mapping should be created"), Mapping);
        Test.TestTrue(TEXT("Should set AbilityTagRelationships"),
                      AeonTests::SetPropertyValue(Test, Mapping, TEXT("AbilityTagRelationships"), Relationships));
        return Mapping;
    }
} // namespace AeonAbilityTagRelationshipMappingTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTagRelationshipMappingBlockAndCancelSingleMatchAppendsTest,
                                 "Aeon.AbilityTagRelationshipMapping.GetAbilityTagsToBlockAndCancel.SingleMatchAppends",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTagRelationshipMappingBlockAndCancelSingleMatchAppendsTest::RunTest(const FString&)
{
    TArray<FAeonAbilityTagRelationship> Relationships;
    Relationships.Add(AeonAbilityTagRelationshipMappingTests::MakeRelationship(
        AeonAbilityTagRelationshipMappingTests::TestPrimaryAbilityTag,
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestBlockTagA }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestCancelTagA })));
    Relationships.Add(AeonAbilityTagRelationshipMappingTests::MakeRelationship(
        AeonAbilityTagRelationshipMappingTests::TestUnrelatedAbilityTag,
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestBlockTagB }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestCancelTagB })));

    const auto Mapping = AeonAbilityTagRelationshipMappingTests::CreateMapping(*this, Relationships);
    auto OutBlockTags = AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestExistingBlockTag });
    auto OutCancelTags = AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestExistingCancelTag });

    Mapping->GetAbilityTagsToBlockAndCancel(
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestPrimaryAbilityTag }),
        OutBlockTags,
        OutCancelTags);

    return TestTrue(TEXT("Existing block tags should be preserved"),
                    OutBlockTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestExistingBlockTag))
        && TestTrue(TEXT("Matching relationship should append block tags"),
                    OutBlockTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestBlockTagA))
        && TestFalse(TEXT("Unrelated relationship should not append block tags"),
                     OutBlockTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestBlockTagB))
        && TestTrue(TEXT("Existing cancel tags should be preserved"),
                    OutCancelTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestExistingCancelTag))
        && TestTrue(TEXT("Matching relationship should append cancel tags"),
                    OutCancelTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestCancelTagA))
        && TestFalse(TEXT("Unrelated relationship should not append cancel tags"),
                     OutCancelTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestCancelTagB));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonAbilityTagRelationshipMappingBlockAndCancelMultipleMatchesMergeTest,
    "Aeon.AbilityTagRelationshipMapping.GetAbilityTagsToBlockAndCancel.MultipleMatchesMerge",
    AeonTests::AutomationTestFlags)
bool FAeonAbilityTagRelationshipMappingBlockAndCancelMultipleMatchesMergeTest::RunTest(const FString&)
{
    TArray<FAeonAbilityTagRelationship> Relationships;
    Relationships.Add(AeonAbilityTagRelationshipMappingTests::MakeRelationship(
        AeonAbilityTagRelationshipMappingTests::TestPrimaryAbilityTag,
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestBlockTagA }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestCancelTagA })));
    Relationships.Add(AeonAbilityTagRelationshipMappingTests::MakeRelationship(
        AeonAbilityTagRelationshipMappingTests::TestSecondaryAbilityTag,
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestBlockTagB }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestCancelTagB })));

    const auto Mapping = AeonAbilityTagRelationshipMappingTests::CreateMapping(*this, Relationships);
    FGameplayTagContainer OutBlockTags;
    FGameplayTagContainer OutCancelTags;

    Mapping->GetAbilityTagsToBlockAndCancel(
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestPrimaryAbilityTag,
                                      AeonAbilityTagRelationshipMappingTests::TestSecondaryAbilityTag }),
        OutBlockTags,
        OutCancelTags);

    return TestTrue(TEXT("First matching relationship should contribute block tags"),
                    OutBlockTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestBlockTagA))
        && TestTrue(TEXT("Second matching relationship should contribute block tags"),
                    OutBlockTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestBlockTagB))
        && TestTrue(TEXT("First matching relationship should contribute cancel tags"),
                    OutCancelTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestCancelTagA))
        && TestTrue(TEXT("Second matching relationship should contribute cancel tags"),
                    OutCancelTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestCancelTagB));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilityTagRelationshipMappingAdditionalRequirementsMergeTest,
                                 "Aeon.AbilityTagRelationshipMapping.GetAdditionalTagRequirements.MultipleMatchesMerge",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilityTagRelationshipMappingAdditionalRequirementsMergeTest::RunTest(const FString&)
{
    TArray<FAeonAbilityTagRelationship> Relationships;
    Relationships.Add(AeonAbilityTagRelationshipMappingTests::MakeRelationship(
        AeonAbilityTagRelationshipMappingTests::TestPrimaryAbilityTag,
        FGameplayTagContainer(),
        FGameplayTagContainer(),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestActivationRequiredTag }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestActivationBlockedTag }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestSourceRequiredTag }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestSourceBlockedTag }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestTargetRequiredTag }),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestTargetBlockedTag })));
    Relationships.Add(AeonAbilityTagRelationshipMappingTests::MakeRelationship(
        AeonAbilityTagRelationshipMappingTests::TestUnrelatedAbilityTag,
        FGameplayTagContainer(),
        FGameplayTagContainer(),
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestBlockTagB })));

    const auto Mapping = AeonAbilityTagRelationshipMappingTests::CreateMapping(*this, Relationships);
    auto OutActivationRequiredTags =
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestExistingRequirementTag });
    FGameplayTagContainer OutActivationBlockedTags;
    FGameplayTagContainer OutSourceRequiredTags;
    FGameplayTagContainer OutSourceBlockedTags;
    FGameplayTagContainer OutTargetRequiredTags;
    FGameplayTagContainer OutTargetBlockedTags;

    Mapping->GetAdditionalTagRequirements(
        AeonTests::MakeTagContainer({ AeonAbilityTagRelationshipMappingTests::TestPrimaryAbilityTag,
                                      AeonAbilityTagRelationshipMappingTests::TestSecondaryAbilityTag }),
        OutActivationRequiredTags,
        OutActivationBlockedTags,
        OutSourceRequiredTags,
        OutSourceBlockedTags,
        OutTargetRequiredTags,
        OutTargetBlockedTags);

    return TestTrue(TEXT("Existing activation required tags should be preserved"),
                    OutActivationRequiredTags.HasTagExact(
                        AeonAbilityTagRelationshipMappingTests::TestExistingRequirementTag))
        && TestTrue(
               TEXT("Matching relationship should append activation required tags"),
               OutActivationRequiredTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestActivationRequiredTag))
        && TestTrue(
               TEXT("Matching relationship should append activation blocked tags"),
               OutActivationBlockedTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestActivationBlockedTag))
        && TestTrue(TEXT("Matching relationship should append source required tags"),
                    OutSourceRequiredTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestSourceRequiredTag))
        && TestTrue(TEXT("Matching relationship should append source blocked tags"),
                    OutSourceBlockedTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestSourceBlockedTag))
        && TestTrue(TEXT("Matching relationship should append target required tags"),
                    OutTargetRequiredTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestTargetRequiredTag))
        && TestTrue(TEXT("Matching relationship should append target blocked tags"),
                    OutTargetBlockedTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestTargetBlockedTag))
        && TestFalse(TEXT("Unrelated relationships should not add activation-required tags"),
                     OutActivationRequiredTags.HasTagExact(AeonAbilityTagRelationshipMappingTests::TestBlockTagB));
}

#endif
