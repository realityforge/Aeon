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

    #include "Aeon/AeonFunctionLibrary.h"
    #include "GameplayAbilitySpec.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonFunctionLibraryTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityTag, "Aeon.Test.FunctionLibrary.Ability");

    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor =
                AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(Test,
                                                                                OutWorld,
                                                                                TEXT("Ability-system test actor"),
                                                                                TEXT("AeonFunctionLibraryTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    bool GrantTaggedAbility(FAutomationTestBase& Test,
                            UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent,
                            FGameplayAbilitySpecHandle& OutHandle)
    {
        const FGameplayTag AbilityTag = AeonFunctionLibraryTests::TestAbilityTag.GetTag();
        if (Test.TestNotNull(TEXT("Ability-system test ASC should be valid"), AbilitySystemComponent))
        {
            UAeonAutomationTestGameplayAbility::ResetCounters();
            FGameplayTagContainer AbilityTagContainer;
            AbilityTagContainer.AddTag(AbilityTag);
            GetMutableDefault<UAeonAutomationTestGameplayAbility>()->SetAssetTagsForTest(AbilityTagContainer);

            FGameplayAbilitySpec Spec(UAeonAutomationTestGameplayAbility::StaticClass());
            Spec.GetDynamicSpecSourceTags().AddTag(AbilityTag);
            OutHandle = AbilitySystemComponent->GiveAbility(Spec);
            return Test.TestTrue(TEXT("Tagged test ability should be granted"), OutHandle.IsValid());
        }
        else
        {
            return false;
        }
    }
} // namespace AeonFunctionLibraryTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonFunctionLibraryTryActivateRandomSingleAbilityByTagActivatesMatchingAbilityTest,
                                 "Aeon.FunctionLibrary.TryActivateRandomSingleAbilityByTag.ActivatesMatchingAbility",
                                 AeonTests::AutomationTestFlags)
bool FAeonFunctionLibraryTryActivateRandomSingleAbilityByTagActivatesMatchingAbilityTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = AeonFunctionLibraryTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonFunctionLibraryTests::CreateAbilitySystemActor(*this, World))
    {
        FGameplayAbilitySpecHandle Handle;
        if (AeonFunctionLibraryTests::GrantTaggedAbility(*this, Actor->GetAeonAbilitySystemComponent(), Handle))
        {
            FGameplayAbilitySpec* ActivatedSpec{ nullptr };
            const bool bActivated =
                UAeonFunctionLibrary::TryActivateRandomSingleAbilityByTag(Actor->GetAeonAbilitySystemComponent(),
                                                                          AbilityTag,
                                                                          &ActivatedSpec);

            return TestTrue(TEXT("Function library should activate a matching ability"), bActivated)
                && TestNotNull(TEXT("Function library should return the activated spec"), ActivatedSpec)
                && TestEqual(TEXT("Function library should activate exactly one test ability instance"),
                             UAeonAutomationTestGameplayAbility::ActivationCount,
                             1)
                && TestEqual(TEXT("Function library should activate the granted spec handle"),
                             ActivatedSpec ? ActivatedSpec->Handle : FGameplayAbilitySpecHandle(),
                             Handle);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonFunctionLibraryTryActivateRandomSingleAbilityByTagReturnsFalseForNoMatchTest,
                                 "Aeon.FunctionLibrary.TryActivateRandomSingleAbilityByTag.ReturnsFalseForNoMatch",
                                 AeonTests::AutomationTestFlags)
bool FAeonFunctionLibraryTryActivateRandomSingleAbilityByTagReturnsFalseForNoMatchTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = AeonFunctionLibraryTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonFunctionLibraryTests::CreateAbilitySystemActor(*this, World))
    {
        FGameplayAbilitySpec* ActivatedSpec{ reinterpret_cast<FGameplayAbilitySpec*>(0x1) };
        const bool bActivated =
            UAeonFunctionLibrary::TryActivateRandomSingleAbilityByTag(Actor->GetAeonAbilitySystemComponent(),
                                                                      AbilityTag,
                                                                      &ActivatedSpec);

        return TestFalse(TEXT("Function library should fail when no matching ability exists"), bActivated)
            && TestNull(TEXT("Function library should clear the output ability spec when no match exists"),
                        ActivatedSpec);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonFunctionLibraryTryActivateRandomSingleAbilityByTagReturnsFalseForActiveAbilityTest,
    "Aeon.FunctionLibrary.TryActivateRandomSingleAbilityByTag.ReturnsFalseForActiveAbility",
    AeonTests::AutomationTestFlags)
bool FAeonFunctionLibraryTryActivateRandomSingleAbilityByTagReturnsFalseForActiveAbilityTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = AeonFunctionLibraryTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonFunctionLibraryTests::CreateAbilitySystemActor(*this, World))
    {
        FGameplayAbilitySpecHandle Handle;
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        if (AeonFunctionLibraryTests::GrantTaggedAbility(*this, AbilitySystemComponent, Handle)
            && TestTrue(TEXT("Tagged test ability should activate"),
                        AbilitySystemComponent->TryActivateAbility(Handle)))
        {
            FGameplayAbilitySpec* ActivatedSpec{ reinterpret_cast<FGameplayAbilitySpec*>(0x1) };
            const bool bActivated = UAeonFunctionLibrary::TryActivateRandomSingleAbilityByTag(AbilitySystemComponent,
                                                                                              AbilityTag,
                                                                                              &ActivatedSpec);

            return TestFalse(TEXT("Function library should not reactivate an already active matching ability"),
                             bActivated)
                && TestNull(TEXT("Output ability spec should remain null when activation does not occur"),
                            ActivatedSpec);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonFunctionLibraryCancelActiveAbilitiesByTagCancelsOnlyActiveAbilitiesTest,
                                 "Aeon.FunctionLibrary.CancelActiveAbilitiesByTag.CancelsOnlyActiveAbilities",
                                 AeonTests::AutomationTestFlags)
bool FAeonFunctionLibraryCancelActiveAbilitiesByTagCancelsOnlyActiveAbilitiesTest::RunTest(const FString&)
{
    const FGameplayTag AbilityTag = AeonFunctionLibraryTests::TestAbilityTag.GetTag();
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonFunctionLibraryTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        FGameplayAbilitySpecHandle ActiveHandle;
        FGameplayAbilitySpecHandle InactiveHandle;
        if (AeonFunctionLibraryTests::GrantTaggedAbility(*this, AbilitySystemComponent, ActiveHandle)
            && AeonFunctionLibraryTests::GrantTaggedAbility(*this, AbilitySystemComponent, InactiveHandle)
            && TestTrue(TEXT("Tagged test ability should activate"),
                        AbilitySystemComponent->TryActivateAbility(ActiveHandle)))
        {
            UAeonFunctionLibrary::CancelActiveAbilitiesByTag(AbilitySystemComponent, AbilityTag);

            const auto ActiveSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(ActiveHandle);
            const auto InactiveSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(InactiveHandle);
            return TestNotNull(TEXT("Active ability spec should remain after cancellation"), ActiveSpec)
                && TestNotNull(TEXT("Inactive ability spec should remain after cancellation"), InactiveSpec)
                && TestFalse(TEXT("Active matching ability should be cancelled"), ActiveSpec->IsActive())
                && TestFalse(TEXT("Inactive matching ability should remain inactive"), InactiveSpec->IsActive());
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
