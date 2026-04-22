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

    #include "Aeon/AbilitySystem/AeonAbilitySet.h"
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

    UAeonAbilitySet* CreatePostInitAbilitySet(FAutomationTestBase& Test,
                                              UObject* Outer,
                                              const TArray<TSubclassOf<UAttributeSet>>& AttributeSetClasses,
                                              const TOptional<float> InitialResourceValue = TOptional<float>())
    {
        const auto AbilitySet = AeonTests::NewTransientObject<UAeonAbilitySet>(Outer);
        if (!Test.TestNotNull(TEXT("Post-init ability set should be created"), AbilitySet))
        {
            return nullptr;
        }

        TArray<FAeonAttributeSetEntry> AttributeSets;
        for (const auto AttributeSetClass : AttributeSetClasses)
        {
            FAeonAttributeSetEntry Entry;
            Entry.AttributeSet = AttributeSetClass;
            AttributeSets.Add(Entry);
        }

        if (!Test.TestTrue(TEXT("Post-init ability set should define attribute sets"),
                           AeonTests::SetPropertyValue(Test, AbilitySet, TEXT("AttributeSets"), AttributeSets)))
        {
            return nullptr;
        }

        if (InitialResourceValue.IsSet())
        {
            TArray<FAeonAttributeInitializer> AttributeValues;
            FAeonAttributeInitializer AttributeValue;
            AttributeValue.Attribute = UAeonAutomationTestAttributeSet::GetResourceAttribute();
            AttributeValue.Value = FScalableFloat(InitialResourceValue.GetValue());
            AttributeValues.Add(AttributeValue);

            if (!Test.TestTrue(TEXT("Post-init ability set should define attribute initializers"),
                               AeonTests::SetPropertyValue(Test, AbilitySet, TEXT("AttributeValues"), AttributeValues)))
            {
                return nullptr;
            }
        }

        return AbilitySet;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentPreReadyTrackedSetDoesNotDispatchTest,
                                 "Aeon.AbilitySystemComponent.PostInit.PreReadyTrackedSetDoesNotDispatch",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentPreReadyTrackedSetDoesNotDispatchTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() },
            25.f);
        if (TestNotNull(TEXT("Post-init ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            FAeonAbilitySetHandles Handles;
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, &Handles, 0, Actor);

            return TestFalse(TEXT("Ability system should not be ready before the ready transition"),
                             AbilitySystemComponent->IsAbilitySystemReady())
                && TestFalse(TEXT("Outstanding post-init work should stay hidden until ready"),
                             AbilitySystemComponent->HasOutstandingPostInitWork())
                && TestEqual(TEXT("Pre-ready grant should not dispatch post-init"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentReadyTransitionDispatchesTrackedSetTest,
                                 "Aeon.AbilitySystemComponent.PostInit.ReadyTransitionDispatchesTrackedSet",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentReadyTransitionDispatchesTrackedSetTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() },
            25.f);
        if (TestNotNull(TEXT("Post-init ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            return TestTrue(TEXT("Ready transition should mark the ASC ready"),
                            AbilitySystemComponent->IsAbilitySystemReady())
                && TestFalse(TEXT("Outstanding post-init work should be drained after ready transition"),
                             AbilitySystemComponent->HasOutstandingPostInitWork())
                && TestEqual(TEXT("Ready transition should dispatch tracked post-init sets exactly once"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             1)
                && TestEqual(TEXT("Grant-time attribute initializers should be visible during post-init"),
                             UAeonAutomationTestPostInitAttributeSet::ObservedResourceValues[0],
                             25.f);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentReadyTransitionIgnoresNonOptInSetTest,
                                 "Aeon.AbilitySystemComponent.PostInit.ReadyTransitionIgnoresNonOptInSet",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentReadyTransitionIgnoresNonOptInSetTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestAttributeSet::StaticClass() },
            10.f);
        if (TestNotNull(TEXT("Non-opt-in ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            return TestEqual(TEXT("Non-opt-in sets should not dispatch post-init"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0)
                && TestFalse(TEXT("Non-opt-in sets should not leave outstanding post-init work"),
                             AbilitySystemComponent->HasOutstandingPostInitWork());
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentReadyTransitionUsesRegisteredSetOrderTest,
                                 "Aeon.AbilitySystemComponent.PostInit.ReadyTransitionUsesRegisteredSetOrder",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentReadyTransitionUsesRegisteredSetOrderTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass(),
              UAeonAutomationTestPostInitAttributeSet::StaticClass() });
        if (TestNotNull(TEXT("Ordered post-init ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);

            TArray<FString> RegisteredNames;
            for (const UAttributeSet* const AttributeSet : AbilitySystemComponent->GetSpawnedAttributes())
            {
                if (const auto PostInitSet = Cast<UAeonAutomationTestPostInitAttributeSet>(AttributeSet))
                {
                    RegisteredNames.Add(PostInitSet->GetName());
                }
            }

            AbilitySystemComponent->MarkAbilitySystemReadyForTest();
            return TestEqual(TEXT("Two tracked post-init sets should be registered"), RegisteredNames.Num(), 2)
                && TestEqual(TEXT("Two post-init invocations should occur"),
                             UAeonAutomationTestPostInitAttributeSet::InvocationOrder.Num(),
                             2)
                && TestEqual(TEXT("Dispatch order should follow registered-set enumeration"),
                             UAeonAutomationTestPostInitAttributeSet::InvocationOrder[0],
                             RegisteredNames[0])
                && TestEqual(TEXT("Dispatch order should follow registered-set enumeration"),
                             UAeonAutomationTestPostInitAttributeSet::InvocationOrder[1],
                             RegisteredNames[1]);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentRuntimeGrantWaitsForOuterBatchTest,
                                 "Aeon.AbilitySystemComponent.PostInit.RuntimeGrantWaitsForOuterBatch",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentRuntimeGrantWaitsForOuterBatchTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        AbilitySystemComponent->MarkAbilitySystemReadyForTest();

        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() },
            30.f);
        if (TestNotNull(TEXT("Runtime post-init ability set should be available"), AbilitySet))
        {
            AbilitySystemComponent->BeginAeonMutationBatchForTest();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);

            const bool bBlockedWhileOuterBatchOpen =
                TestTrue(TEXT("Ready ASC should report outstanding work while the outer batch is open"),
                         AbilitySystemComponent->HasOutstandingPostInitWork())
                && TestEqual(TEXT("Runtime grant should not dispatch until the outer batch closes"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0);

            AbilitySystemComponent->EndAeonMutationBatchForTest();
            return bBlockedWhileOuterBatchOpen
                && TestEqual(TEXT("Closing the outermost batch should dispatch post-init"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             1)
                && TestFalse(TEXT("Outstanding work should be drained after the outermost batch closes"),
                             AbilitySystemComponent->HasOutstandingPostInitWork());
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentRepeatedReadyMarkIsNoOpTest,
                                 "Aeon.AbilitySystemComponent.PostInit.RepeatedReadyMarkIsNoOp",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentRepeatedReadyMarkIsNoOpTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() });
        if (TestNotNull(TEXT("Repeated-ready ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            AddExpectedMessagePlain(TEXT("MarkAbilitySystemReady() invoked more than once"),
                                    ELogVerbosity::Warning,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            return TestEqual(TEXT("Repeated ready marks should not redispatch post-init"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             1);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentReentrantNestedGrantDrainsLaterTest,
                                 "Aeon.AbilitySystemComponent.PostInit.ReentrantNestedGrantDrainsLater",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentReentrantNestedGrantDrainsLaterTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        UAeonAutomationTestPostInitAttributeSet::Behavior =
            UAeonAutomationTestPostInitAttributeSet::EBehavior::GrantNestedAbilitySet;
        UAeonAutomationTestPostInitAttributeSet::NestedAbilitySet =
            AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
                *this,
                Actor,
                { UAeonAutomationTestPostInitAttributeSet::StaticClass() },
                40.f);

        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() },
            20.f);
        if (TestNotNull(TEXT("Nested post-init ability set should be available"), AbilitySet)
            && TestNotNull(TEXT("Nested grant ability set should be available"),
                           UAeonAutomationTestPostInitAttributeSet::NestedAbilitySet.Get()))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            return TestEqual(TEXT("Re-entrant nested grant should eventually dispatch twice"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             2)
                && TestEqual(TEXT("Initial and nested post-init values should both be observed"),
                             UAeonAutomationTestPostInitAttributeSet::ObservedResourceValues.Num(),
                             2)
                && TestEqual(TEXT("Initial and nested grants should produce two invocation records"),
                             UAeonAutomationTestPostInitAttributeSet::InvocationOrder.Num(),
                             2)
                && TestNotEqual(TEXT("Initial and nested callbacks should run on distinct set instances"),
                                UAeonAutomationTestPostInitAttributeSet::InvocationOrder[0],
                                UAeonAutomationTestPostInitAttributeSet::InvocationOrder[1]);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentRemovalBeforeDispatchSkipsSetTest,
                                 "Aeon.AbilitySystemComponent.PostInit.RemovalBeforeDispatchSkipsSet",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentRemovalBeforeDispatchSkipsSetTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() });
        if (TestNotNull(TEXT("Removal-before-dispatch ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            FAeonAbilitySetHandles Handles;
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, &Handles, 0, Actor);
            Handles.RemoveFromAbilitySystemComponent();
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            return TestEqual(TEXT("Removing the set before ready should skip post-init"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentRemovalDuringDispatchAllowsNewLifetimeTest,
                                 "Aeon.AbilitySystemComponent.PostInit.RemovalDuringDispatchAllowsNewLifetime",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentRemovalDuringDispatchAllowsNewLifetimeTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        UAeonAutomationTestPostInitAttributeSet::Behavior =
            UAeonAutomationTestPostInitAttributeSet::EBehavior::RemoveSelf;

        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() });
        if (TestNotNull(TEXT("Removal-during-dispatch ability set should be available"), AbilitySet))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);
            const auto AttributeSet = const_cast<UAeonAutomationTestPostInitAttributeSet*>(
                AbilitySystemComponent->GetSet<UAeonAutomationTestPostInitAttributeSet>());
            if (TestNotNull(TEXT("Tracked post-init attribute set should be registered"), AttributeSet))
            {
                AbilitySystemComponent->MarkAbilitySystemReadyForTest();
                const bool bRemovalSucceeded =
                    TestEqual(TEXT("Removal during dispatch should still invoke post-init once"),
                              UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                              1)
                    && TestFalse(TEXT("Removing the set during dispatch should unregister it"),
                                 AbilitySystemComponent->HasAttributeSetForAttribute(
                                     UAeonAutomationTestAttributeSet::GetResourceAttribute()));

                UAeonAutomationTestPostInitAttributeSet::Behavior =
                    UAeonAutomationTestPostInitAttributeSet::EBehavior::None;
                AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
                AbilitySystemComponent->NotifyAttributeSetRegisteredForTest(AttributeSet);

                return bRemovalSucceeded
                    && TestEqual(TEXT("Re-registering the same instance should create a new post-init lifetime"),
                                 UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                                 2);
            }
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentTeardownCancelsOutstandingWorkTest,
                                 "Aeon.AbilitySystemComponent.PostInit.TeardownCancelsOutstandingWork",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentTeardownCancelsOutstandingWorkTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        AbilitySystemComponent->MarkAbilitySystemReadyForTest();
        AbilitySystemComponent->BeginAeonMutationBatchForTest();

        const auto AbilitySet = AeonAbilitySystemComponentTests::CreatePostInitAbilitySet(
            *this,
            Actor,
            { UAeonAutomationTestPostInitAttributeSet::StaticClass() });
        if (TestNotNull(TEXT("Teardown ability set should be available"), AbilitySet))
        {
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, nullptr, 0, Actor);
            const bool bPendingBeforeTeardown =
                TestTrue(TEXT("Ready ASC should report outstanding work while a batch is open"),
                         AbilitySystemComponent->HasOutstandingPostInitWork())
                && TestEqual(TEXT("Open batch should prevent post-init dispatch before teardown"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0);

            AddExpectedMessagePlain(TEXT("CancelPostInitForTeardown() interrupted AbilitySystemComponent"),
                                    ELogVerbosity::Warning,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);
            AbilitySystemComponent->CancelPostInitForTeardownForTest();
            AbilitySystemComponent->EndAeonMutationBatchForTest();

            return bPendingBeforeTeardown
                && TestTrue(TEXT("Teardown should keep the ASC ready flag sticky"),
                            AbilitySystemComponent->IsAbilitySystemReady())
                && TestFalse(TEXT("Teardown should clear outstanding post-init work"),
                             AbilitySystemComponent->HasOutstandingPostInitWork())
                && TestEqual(TEXT("Teardown should cancel pending post-init dispatch"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0);
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySystemComponentReadyScanDiagnosesUntrackedOptInSetTest,
                                 "Aeon.AbilitySystemComponent.PostInit.ReadyScanDiagnosesUntrackedOptInSet",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySystemComponentReadyScanDiagnosesUntrackedOptInSetTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySystemComponentTests::CreateAbilitySystemActor(*this, World))
    {
        UAeonAutomationTestPostInitAttributeSet::ResetPostInitState();
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto AttributeSet =
            AeonTests::CreateOwnedAttributeSet<UAeonAutomationTestPostInitAttributeSet>(*this, AbilitySystemComponent);
        if (TestNotNull(TEXT("Untracked opt-in set should be created"), AttributeSet))
        {
            AddExpectedMessagePlain(TEXT("Ready-time post-init scan found opt-in AttributeSet"),
                                    ELogVerbosity::Warning,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);
            AbilitySystemComponent->MarkAbilitySystemReadyForTest();

            return TestEqual(TEXT("Untracked ready-time sets should be skipped"),
                             UAeonAutomationTestPostInitAttributeSet::PostInitCallCount,
                             0);
        }
    }

    return false;
}

#endif
