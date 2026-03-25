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

    #include "Aeon/Subsystems/GroupAbilitySystemComponentRegistry.h"
    #include "Misc/AutomationTest.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace GroupAbilitySystemComponentRegistryTests
{
    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor = AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(
                Test,
                OutWorld,
                TEXT("Ability-system test actor"),
                TEXT("GroupAbilitySystemComponentRegistryTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }

    UGroupAbilitySystemComponentRegistry* CreateRegistry(UObject* Outer = GetTransientPackage())
    {
        return NewObject<UGroupAbilitySystemComponentRegistry>(Outer, NAME_None, RF_Transient);
    }
} // namespace GroupAbilitySystemComponentRegistryTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemComponentRegistryChildRegistrationPropagatesImplicitlyToParentTest,
    "Aeon.Subsystems.GroupAbilitySystemComponentRegistry.ChildRegistrationPropagatesImplicitlyToParent",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemComponentRegistryChildRegistrationPropagatesImplicitlyToParentTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemComponentRegistryTests::CreateAbilitySystemActor(*this, World))
    {
        const auto ParentRegistry = GroupAbilitySystemComponentRegistryTests::CreateRegistry();
        const auto ChildRegistry = GroupAbilitySystemComponentRegistryTests::CreateRegistry();
        ChildRegistry->SetParent(ParentRegistry);
        ChildRegistry->RegisterAbilitySystemComponent(Actor->GetAeonAbilitySystemComponent());

        return TestTrue(TEXT("Child registry should contain the explicitly registered ASC"),
                        ChildRegistry->IsAbilitySystemComponentRegistered(Actor->GetAeonAbilitySystemComponent()))
            && TestTrue(
                   TEXT("Parent registry should contain the ASC as an implicit registration"),
                   ParentRegistry->IsAbilitySystemComponentRegistered(Actor->GetAeonAbilitySystemComponent(), true));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemComponentRegistryAddAbilityAndEffectAppliesToRegisteredComponentsTest,
    "Aeon.Subsystems.GroupAbilitySystemComponentRegistry.AddAbilityAndEffectAppliesToRegisteredComponents",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemComponentRegistryAddAbilityAndEffectAppliesToRegisteredComponentsTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemComponentRegistryTests::CreateAbilitySystemActor(*this, World))
    {
        const auto Registry = GroupAbilitySystemComponentRegistryTests::CreateRegistry();
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        Registry->RegisterAbilitySystemComponent(AbilitySystemComponent);
        Registry->AddAbility(UAeonAutomationTestGameplayAbility::StaticClass());
        Registry->AddEffect(UAeonAutomationTestGameplayEffect::StaticClass());

        return TestTrue(TEXT("Ability registry should track the granted gameplay ability"),
                        Registry->IsAbilityPresent(UAeonAutomationTestGameplayAbility::StaticClass()))
            && TestNotNull(
                   TEXT("Registered ASC should receive the added gameplay ability"),
                   AbilitySystemComponent->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass()))
            && TestTrue(TEXT("Effect registry should track the applied gameplay effect"),
                        Registry->IsEffectPresent(UAeonAutomationTestGameplayEffect::StaticClass()))
            && TestEqual(TEXT("Registered ASC should receive the added gameplay effect"),
                         AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemComponentRegistryRegisterAfterAddingAbilityAndEffectAppliesExistingEntriesTest,
    "Aeon.Subsystems.GroupAbilitySystemComponentRegistry.RegisterAfterAddingAbilityAndEffectAppliesExistingEntries",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemComponentRegistryRegisterAfterAddingAbilityAndEffectAppliesExistingEntriesTest::RunTest(
    const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemComponentRegistryTests::CreateAbilitySystemActor(*this, World))
    {
        const auto Registry = GroupAbilitySystemComponentRegistryTests::CreateRegistry();
        Registry->AddAbility(UAeonAutomationTestGameplayAbility::StaticClass());
        Registry->AddEffect(UAeonAutomationTestGameplayEffect::StaticClass());

        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        Registry->RegisterAbilitySystemComponent(AbilitySystemComponent);

        return TestNotNull(
                   TEXT("ASC should receive pre-existing group abilities on registration"),
                   AbilitySystemComponent->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass()))
            && TestEqual(TEXT("ASC should receive pre-existing group effects on registration"),
                         AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemComponentRegistryRemoveAndUnregisterRemovesAppliedEntriesTest,
    "Aeon.Subsystems.GroupAbilitySystemComponentRegistry.RemoveAndUnregisterRemovesAppliedEntries",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemComponentRegistryRemoveAndUnregisterRemovesAppliedEntriesTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemComponentRegistryTests::CreateAbilitySystemActor(*this, World))
    {
        const auto Registry = GroupAbilitySystemComponentRegistryTests::CreateRegistry();
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        Registry->RegisterAbilitySystemComponent(AbilitySystemComponent);
        Registry->AddAbility(UAeonAutomationTestGameplayAbility::StaticClass());
        Registry->AddEffect(UAeonAutomationTestGameplayEffect::StaticClass());
        Registry->RemoveAbility(UAeonAutomationTestGameplayAbility::StaticClass());
        Registry->RemoveEffect(UAeonAutomationTestGameplayEffect::StaticClass());
        Registry->UnregisterAbilitySystemComponent(AbilitySystemComponent);

        return TestNull(
                   TEXT("Removing the grouped ability should clear it from the ASC"),
                   AbilitySystemComponent->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass()))
            && TestEqual(TEXT("Removing the grouped effect should clear it from the ASC"),
                         AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                         0)
            && TestFalse(TEXT("Unregistering should remove the explicit ASC registration"),
                         Registry->IsAbilitySystemComponentRegistered(AbilitySystemComponent));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemComponentRegistryDuplicateRegistrationDoesNotDuplicateGrantedEntriesTest,
    "Aeon.Subsystems.GroupAbilitySystemComponentRegistry.DuplicateRegistrationDoesNotDuplicateGrantedEntries",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemComponentRegistryDuplicateRegistrationDoesNotDuplicateGrantedEntriesTest::RunTest(
    const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemComponentRegistryTests::CreateAbilitySystemActor(*this, World))
    {
        AddExpectedError(TEXT("already explicitly registered"), EAutomationExpectedErrorFlags::Contains, 1);

        const auto Registry = GroupAbilitySystemComponentRegistryTests::CreateRegistry();
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        Registry->AddAbility(UAeonAutomationTestGameplayAbility::StaticClass());
        Registry->AddEffect(UAeonAutomationTestGameplayEffect::StaticClass());
        Registry->RegisterAbilitySystemComponent(AbilitySystemComponent);
        Registry->RegisterAbilitySystemComponent(AbilitySystemComponent);

        return TestEqual(TEXT("Duplicate registration should not duplicate granted ability specs"),
                         AbilitySystemComponent->GetActivatableAbilities().Num(),
                         1)
            && TestEqual(TEXT("Duplicate registration should not duplicate applied effects"),
                         AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                         1);
    }
    else
    {
        return false;
    }
}

#endif
