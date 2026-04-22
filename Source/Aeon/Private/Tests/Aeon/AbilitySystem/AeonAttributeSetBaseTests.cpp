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

    #include "AbilitySystemComponent.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonAttributeSetBaseTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestThresholdTag, "Aeon.Test.AttributeSetBase.Threshold");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestReplicatedThresholdTag, "Aeon.Test.AttributeSetBase.ReplicatedThreshold");

    bool CreateOwnedAttributeSet(FAutomationTestBase& Test,
                                 TUniquePtr<AeonTests::FTestWorld>& OutWorld,
                                 AAeonAutomationTestActor*& OutActor,
                                 UAeonAutomationTestAbilitySystemComponent*& OutAbilitySystemComponent,
                                 UAeonAutomationTestAttributeSet*& OutAttributeSet)
    {
        OutActor =
            AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(Test,
                                                                            OutWorld,
                                                                            TEXT("Ability-system test actor"),
                                                                            TEXT("AeonAttributeSetBaseTestWorld"));
        if (OutActor)
        {
            OutAbilitySystemComponent = AeonTests::GetInitializedAbilitySystemComponent(Test, OutActor);
            if (OutAbilitySystemComponent)
            {
                OutAttributeSet =
                    AeonTests::CreateOwnedAttributeSet<UAeonAutomationTestAttributeSet>(Test,
                                                                                        OutAbilitySystemComponent);
                return nullptr != OutAttributeSet;
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
} // namespace AeonAttributeSetBaseTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAttributeSetBaseAddTagIfValueAboveThresholdAddsLooseTagTest,
                                 "Aeon.AttributeSetBase.AddTagIfValueAboveThreshold.AddsLooseTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonAttributeSetBaseAddTagIfValueAboveThresholdAddsLooseTagTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    AAeonAutomationTestActor* Actor{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UAeonAutomationTestAttributeSet* AttributeSet{ nullptr };
    if (AeonAttributeSetBaseTests::CreateOwnedAttributeSet(*this, World, Actor, AbilitySystemComponent, AttributeSet))
    {
        AttributeSet->AddTagIfValueAboveThresholdForTest(UAeonAutomationTestAttributeSet::GetResourceAttribute(),
                                                         AeonAttributeSetBaseTests::TestThresholdTag,
                                                         60.f,
                                                         50.f);

        return TestTrue(TEXT("Loose threshold tag should be added when the value is above the threshold"),
                        AbilitySystemComponent->HasMatchingGameplayTag(AeonAttributeSetBaseTests::TestThresholdTag))
            && TestEqual(TEXT("Replicated loose tag count should remain zero for non-replicated adds"),
                         AbilitySystemComponent->GetReplicatedLooseTagCountForTest(
                             AeonAttributeSetBaseTests::TestThresholdTag),
                         0);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAttributeSetBaseAddTagIfValueBelowThresholdRemovesLooseTagTest,
                                 "Aeon.AttributeSetBase.AddTagIfValueBelowThreshold.RemovesLooseTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonAttributeSetBaseAddTagIfValueBelowThresholdRemovesLooseTagTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    AAeonAutomationTestActor* Actor{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UAeonAutomationTestAttributeSet* AttributeSet{ nullptr };
    if (AeonAttributeSetBaseTests::CreateOwnedAttributeSet(*this, World, Actor, AbilitySystemComponent, AttributeSet))
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(AeonAttributeSetBaseTests::TestThresholdTag, 1);
        AttributeSet->AddTagIfValueBelowThresholdForTest(UAeonAutomationTestAttributeSet::GetResourceAttribute(),
                                                         AeonAttributeSetBaseTests::TestThresholdTag,
                                                         60.f,
                                                         50.f);

        return TestFalse(TEXT("Loose threshold tag should be removed when the value is no longer below the threshold"),
                         AbilitySystemComponent->HasMatchingGameplayTag(AeonAttributeSetBaseTests::TestThresholdTag));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAttributeSetBaseAddTagIfValueAboveThresholdUsesReplicatedTagPathTest,
                                 "Aeon.AttributeSetBase.AddTagIfValueAboveThreshold.UsesReplicatedTagPath",
                                 AeonTests::AutomationTestFlags)
bool FAeonAttributeSetBaseAddTagIfValueAboveThresholdUsesReplicatedTagPathTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    AAeonAutomationTestActor* Actor{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UAeonAutomationTestAttributeSet* AttributeSet{ nullptr };
    if (AeonAttributeSetBaseTests::CreateOwnedAttributeSet(*this, World, Actor, AbilitySystemComponent, AttributeSet))
    {
        AttributeSet->AddTagIfValueAboveThresholdForTest(UAeonAutomationTestAttributeSet::GetResourceAttribute(),
                                                         AeonAttributeSetBaseTests::TestReplicatedThresholdTag,
                                                         60.f,
                                                         50.f,
                                                         true);

        return TestTrue(TEXT("Replicated threshold tag should still be added to the ASC"),
                        AbilitySystemComponent->HasMatchingGameplayTag(
                            AeonAttributeSetBaseTests::TestReplicatedThresholdTag))
            && TestEqual(TEXT("Replicated threshold tag should contribute a single gameplay-tag count"),
                         AbilitySystemComponent->GetTagCount(AeonAttributeSetBaseTests::TestReplicatedThresholdTag),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAttributeSetBaseAddTagIfValueAboveThresholdRespectsToleranceTest,
                                 "Aeon.AttributeSetBase.AddTagIfValueAboveThreshold.RespectsTolerance",
                                 AeonTests::AutomationTestFlags)
bool FAeonAttributeSetBaseAddTagIfValueAboveThresholdRespectsToleranceTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    AAeonAutomationTestActor* Actor{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UAeonAutomationTestAttributeSet* AttributeSet{ nullptr };
    if (AeonAttributeSetBaseTests::CreateOwnedAttributeSet(*this, World, Actor, AbilitySystemComponent, AttributeSet))
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(AeonAttributeSetBaseTests::TestThresholdTag, 1);
        AttributeSet->AddTagIfValueAboveThresholdForTest(UAeonAutomationTestAttributeSet::GetResourceAttribute(),
                                                         AeonAttributeSetBaseTests::TestThresholdTag,
                                                         49.75f,
                                                         50.f,
                                                         false,
                                                         0.5f);

        return TestTrue(TEXT("The existing tag should remain when the value stays within the error tolerance"),
                        AbilitySystemComponent->HasMatchingGameplayTag(AeonAttributeSetBaseTests::TestThresholdTag))
            && TestEqual(TEXT("The tag count should remain stable when the value is within tolerance"),
                         AbilitySystemComponent->GetTagCount(AeonAttributeSetBaseTests::TestThresholdTag),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAttributeSetBaseAdjustAttributeAfterMaxValueChangesPreservesRatioTest,
                                 "Aeon.AttributeSetBase.AdjustAttributeAfterMaxValueChanges.PreservesRatio",
                                 AeonTests::AutomationTestFlags)
bool FAeonAttributeSetBaseAdjustAttributeAfterMaxValueChangesPreservesRatioTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    AAeonAutomationTestActor* Actor{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UAeonAutomationTestAttributeSet* AttributeSet{ nullptr };
    if (AeonAttributeSetBaseTests::CreateOwnedAttributeSet(*this, World, Actor, AbilitySystemComponent, AttributeSet))
    {
        AbilitySystemComponent->SetNumericAttributeBase(UAeonAutomationTestAttributeSet::GetResourceAttribute(), 40.f);
        AttributeSet->AdjustAttributeAfterMaxValueChangesForTest(
            UAeonAutomationTestAttributeSet::GetResourceAttribute(),
            100.f,
            150.f);

        return TestEqual(
            TEXT("Adjusted attribute base should preserve the same proportion to the max value"),
            AbilitySystemComponent->GetNumericAttributeBase(UAeonAutomationTestAttributeSet::GetResourceAttribute()),
            60.f);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAttributeSetBaseAdjustAttributeAfterMaxValueChangesNoOpWhenUnchangedTest,
                                 "Aeon.AttributeSetBase.AdjustAttributeAfterMaxValueChanges.NoOpWhenUnchanged",
                                 AeonTests::AutomationTestFlags)
bool FAeonAttributeSetBaseAdjustAttributeAfterMaxValueChangesNoOpWhenUnchangedTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    AAeonAutomationTestActor* Actor{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    UAeonAutomationTestAttributeSet* AttributeSet{ nullptr };
    if (AeonAttributeSetBaseTests::CreateOwnedAttributeSet(*this, World, Actor, AbilitySystemComponent, AttributeSet))
    {
        AbilitySystemComponent->SetNumericAttributeBase(UAeonAutomationTestAttributeSet::GetResourceAttribute(), 40.f);
        AttributeSet->AdjustAttributeAfterMaxValueChangesForTest(
            UAeonAutomationTestAttributeSet::GetResourceAttribute(),
            100.f,
            100.f);
        const float AfterEqualMaxValue =
            AbilitySystemComponent->GetNumericAttributeBase(UAeonAutomationTestAttributeSet::GetResourceAttribute());
        AttributeSet->AdjustAttributeAfterMaxValueChangesForTest(
            UAeonAutomationTestAttributeSet::GetResourceAttribute(),
            0.f,
            150.f);

        return TestEqual(TEXT("Equal max values should leave the base value unchanged"), AfterEqualMaxValue, 40.f)
            && TestEqual(TEXT("A zero old max value should leave the base value unchanged"),
                         AbilitySystemComponent->GetNumericAttributeBase(
                             UAeonAutomationTestAttributeSet::GetResourceAttribute()),
                         40.f);
    }
    else
    {
        return false;
    }
}

#endif
