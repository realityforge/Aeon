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
    #include "Aeon/AbilitySystem/AeonAbilitySet.h"
    #include "GameplayEffect.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "ScalableFloat.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonAbilitySetTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestLooseGameplayTag, "Aeon.Test.AbilitySet.Loose");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityInputTag, "Aeon.Test.AbilitySet.Input");

    FScalableFloat MakeInvalidScalableFloat()
    {
        FScalableFloat Value(1.f);
        Value.Curve.RowName = TEXT("MissingCurveRow");
        return Value;
    }

    UAeonAbilitySet* CreateAbilitySet()
    {
        return AeonTests::NewTransientObject<UAeonAbilitySet>();
    }

    bool ConfigureValidAbilitySet(FAutomationTestBase& Test, UAeonAbilitySet* AbilitySet)
    {
        TArray<FAeonGameplayAbilityEntry> Abilities;
        FAeonGameplayAbilityEntry AbilityEntry;
        AbilityEntry.Ability = UAeonAutomationTestGameplayAbility::StaticClass();
        AbilityEntry.InputTags = AeonTests::MakeTagContainer({ TestAbilityInputTag });
        Abilities.Add(AbilityEntry);

        TArray<FAeonGameplayEffectEntry> Effects;
        FAeonGameplayEffectEntry EffectEntry;
        EffectEntry.Effect = UAeonAutomationTestGameplayEffect::StaticClass();
        Effects.Add(EffectEntry);

        TArray<FAeonAttributeSetEntry> AttributeSets;
        FAeonAttributeSetEntry AttributeSetEntry;
        AttributeSetEntry.AttributeSet = UAeonAutomationTestAttributeSet::StaticClass();
        AttributeSets.Add(AttributeSetEntry);

        TArray<FAeonAttributeInitializer> AttributeValues;
        FAeonAttributeInitializer AttributeValue;
        AttributeValue.Attribute = UAeonAutomationTestAttributeSet::GetResourceAttribute();
        AttributeValue.Value = FScalableFloat(25.f);
        AttributeValues.Add(AttributeValue);

        return Test.TestTrue(TEXT("Should set Tags"),
                             AeonTests::SetPropertyValue(Test,
                                                         AbilitySet,
                                                         TEXT("Tags"),
                                                         AeonTests::MakeTagContainer({ TestLooseGameplayTag })))
            && Test.TestTrue(TEXT("Should set Abilities"),
                             AeonTests::SetPropertyValue(Test, AbilitySet, TEXT("Abilities"), Abilities))
            && Test.TestTrue(TEXT("Should set Effects"),
                             AeonTests::SetPropertyValue(Test, AbilitySet, TEXT("Effects"), Effects))
            && Test.TestTrue(TEXT("Should set AttributeSets"),
                             AeonTests::SetPropertyValue(Test, AbilitySet, TEXT("AttributeSets"), AttributeSets))
            && Test.TestTrue(TEXT("Should set AttributeValues"),
                             AeonTests::SetPropertyValue(Test, AbilitySet, TEXT("AttributeValues"), AttributeValues));
    }

    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor =
                AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(Test,
                                                                                OutWorld,
                                                                                TEXT("Ability-system test actor"),
                                                                                TEXT("AeonAbilitySetTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }
} // namespace AeonAbilitySetTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationValidConfigTest,
                                 "Aeon.Validation.AbilitySet.ValidConfig",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationValidConfigTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    return AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet)
        ? AeonTests::TestValidation(*this, AbilitySet, EDataValidationResult::Valid)
        : false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationInvalidAbilityEntryTest,
                                 "Aeon.Validation.AbilitySet.InvalidAbilityEntry",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationInvalidAbilityEntryTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
    {
        TArray<FAeonGameplayAbilityEntry> Abilities;
        Abilities.AddDefaulted();
        if (TestTrue(TEXT("Should set invalid Abilities"),
                     AeonTests::SetPropertyValue(*this, AbilitySet, TEXT("Abilities"), Abilities)))
        {
            return AeonTests::TestValidation(*this,
                                             AbilitySet,
                                             EDataValidationResult::Invalid,
                                             TEXT("Abilities[0].Ability"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationInvalidEffectEntryTest,
                                 "Aeon.Validation.AbilitySet.InvalidEffectEntry",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationInvalidEffectEntryTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
    {
        TArray<FAeonGameplayEffectEntry> Effects;
        Effects.AddDefaulted();
        if (TestTrue(TEXT("Should set invalid Effects"),
                     AeonTests::SetPropertyValue(*this, AbilitySet, TEXT("Effects"), Effects)))
        {
            return AeonTests::TestValidation(*this,
                                             AbilitySet,
                                             EDataValidationResult::Invalid,
                                             TEXT("Effects[0].Effect"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationInvalidAttributeSetEntryTest,
                                 "Aeon.Validation.AbilitySet.InvalidAttributeSetEntry",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationInvalidAttributeSetEntryTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
    {
        TArray<FAeonAttributeSetEntry> AttributeSets;
        AttributeSets.AddDefaulted();
        if (TestTrue(TEXT("Should set invalid AttributeSets"),
                     AeonTests::SetPropertyValue(*this, AbilitySet, TEXT("AttributeSets"), AttributeSets)))
        {
            return AeonTests::TestValidation(*this,
                                             AbilitySet,
                                             EDataValidationResult::Invalid,
                                             TEXT("AttributeSets[0].AttributeSet"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationInvalidAttributeInitializerAttributeTest,
                                 "Aeon.Validation.AbilitySet.InvalidAttributeInitializerAttribute",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationInvalidAttributeInitializerAttributeTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
    {
        TArray<FAeonAttributeInitializer> AttributeValues;
        FAeonAttributeInitializer InvalidAttributeValue;
        InvalidAttributeValue.Value = FScalableFloat(25.f);
        AttributeValues.Add(InvalidAttributeValue);
        if (TestTrue(TEXT("Should set invalid AttributeValues"),
                     AeonTests::SetPropertyValue(*this, AbilitySet, TEXT("AttributeValues"), AttributeValues)))
        {
            return AeonTests::TestValidation(*this,
                                             AbilitySet,
                                             EDataValidationResult::Invalid,
                                             TEXT("AttributeValues[0].Attribute"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationInvalidAttributeInitializerValueTest,
                                 "Aeon.Validation.AbilitySet.InvalidAttributeInitializerValue",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationInvalidAttributeInitializerValueTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
    {
        TArray<FAeonAttributeInitializer> AttributeValues;
        FAeonAttributeInitializer InvalidAttributeValue;
        InvalidAttributeValue.Attribute = UAeonAutomationTestAttributeSet::GetResourceAttribute();
        InvalidAttributeValue.Value = AeonAbilitySetTests::MakeInvalidScalableFloat();
        AttributeValues.Add(InvalidAttributeValue);
        if (TestTrue(TEXT("Should set invalid AttributeValues"),
                     AeonTests::SetPropertyValue(*this, AbilitySet, TEXT("AttributeValues"), AttributeValues)))
        {
            return AeonTests::TestValidation(*this,
                                             AbilitySet,
                                             EDataValidationResult::Invalid,
                                             TEXT("AttributeValues[0].Value"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetValidationAttributeInitializerMissingAttributeSetTest,
                                 "Aeon.Validation.AbilitySet.AttributeInitializerMissingAttributeSet",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetValidationAttributeInitializerMissingAttributeSetTest::RunTest(const FString&)
{
    const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
    if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
    {
        const TArray<FAeonAttributeSetEntry> AttributeSets;
        if (TestTrue(TEXT("Should clear AttributeSets"),
                     AeonTests::SetPropertyValue(*this, AbilitySet, TEXT("AttributeSets"), AttributeSets)))
        {
            return AeonTests::TestValidation(*this,
                                             AbilitySet,
                                             EDataValidationResult::Invalid,
                                             TEXT("not defined in the AttributeSets property"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetGrantToAbilitySystemAppliesConfiguredGrantsTest,
                                 "Aeon.AbilitySet.GrantToAbilitySystem.AppliesConfiguredGrants",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetGrantToAbilitySystemAppliesConfiguredGrantsTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySetTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
        if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
        {
            FAeonAbilitySetHandles Handles;
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, &Handles, 0, Actor);

            const auto GrantedAbilitySpec =
                AbilitySystemComponent->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass());
            const auto ActiveEffects = AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery());

            return TestTrue(TEXT("Grant handles should be valid after a successful grant"), Handles.IsValid())
                && TestTrue(TEXT("Loose gameplay tag should be granted"),
                            AbilitySystemComponent->HasMatchingGameplayTag(AeonAbilitySetTests::TestLooseGameplayTag))
                && TestNotNull(TEXT("Gameplay ability should be granted"), GrantedAbilitySpec)
                && TestEqual(TEXT("A single active gameplay effect should be applied"), ActiveEffects.Num(), 1)
                && TestTrue(TEXT("Attribute set should be registered for its attribute"),
                            AbilitySystemComponent->HasAttributeSetForAttribute(
                                UAeonAutomationTestAttributeSet::GetResourceAttribute()))
                && TestEqual(TEXT("Attribute initializer should set the numeric base value"),
                             AbilitySystemComponent->GetNumericAttributeBase(
                                 UAeonAutomationTestAttributeSet::GetResourceAttribute()),
                             25.f);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetRemoveFromAbilitySystemComponentRemovesConfiguredGrantsTest,
                                 "Aeon.AbilitySet.RemoveFromAbilitySystemComponent.RemovesConfiguredGrants",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetRemoveFromAbilitySystemComponentRemovesConfiguredGrantsTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySetTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
        if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
        {
            FAeonAbilitySetHandles Handles;
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, &Handles, 0, Actor);
            Handles.RemoveFromAbilitySystemComponent();

            const auto ActiveEffects = AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery());
            const auto GrantedAbilitySpec =
                AbilitySystemComponent->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass());

            return TestFalse(TEXT("Grant handles should be invalid after removal"), Handles.IsValid())
                && TestFalse(TEXT("Loose gameplay tag should be removed"),
                             AbilitySystemComponent->HasMatchingGameplayTag(AeonAbilitySetTests::TestLooseGameplayTag))
                && TestNull(TEXT("Granted ability should be removed"), GrantedAbilitySpec)
                && TestEqual(TEXT("Active gameplay effects should be removed"), ActiveEffects.Num(), 0)
                && TestFalse(TEXT("Spawned attribute set should be removed"),
                             AbilitySystemComponent->HasAttributeSetForAttribute(
                                 UAeonAutomationTestAttributeSet::GetResourceAttribute()));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonAbilitySetRemoveFromAbilitySystemComponentIsIdempotentTest,
                                 "Aeon.AbilitySet.RemoveFromAbilitySystemComponent.IsIdempotent",
                                 AeonTests::AutomationTestFlags)
bool FAeonAbilitySetRemoveFromAbilitySystemComponentIsIdempotentTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = AeonAbilitySetTests::CreateAbilitySystemActor(*this, World))
    {
        const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
        const auto AbilitySet = AeonAbilitySetTests::CreateAbilitySet();
        if (AeonAbilitySetTests::ConfigureValidAbilitySet(*this, AbilitySet))
        {
            FAeonAbilitySetHandles Handles;
            AbilitySet->GrantToAbilitySystem(AbilitySystemComponent, &Handles, 0, Actor);
            Handles.RemoveFromAbilitySystemComponent();
            AddExpectedMessagePlain(
                TEXT("RemoveAbilitySetFromAbilitySystemComponent() invoked when AbilitySystemComponent is invalid."),
                ELogVerbosity::Warning,
                EAutomationExpectedMessageFlags::Contains,
                1);
            Handles.RemoveFromAbilitySystemComponent();

            return TestFalse(TEXT("Grant handles should remain invalid after repeated removal"), Handles.IsValid())
                && TestNull(TEXT("Granted ability should remain removed"),
                            AbilitySystemComponent->FindAbilitySpecFromClass(
                                UAeonAutomationTestGameplayAbility::StaticClass()))
                && TestEqual(TEXT("No active gameplay effects should remain after repeated removal"),
                             AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                             0);
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
