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
    #include "Misc/AutomationTest.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/Character/AeonCharacterBaseTestTypes.h"
    // ReSharper disable twice CppUnusedIncludeDirective
    #include "ActiveGameplayEffectHandle.h"
    #include "GameplayAbilitySpecHandle.h"

namespace AeonCharacterBaseTests
{
    AAeonTestConcreteCharacterBase* CreateConcreteCharacter(FAutomationTestBase& Test,
                                                            TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        return AeonTests::SpawnActorInFreshTestWorld<AAeonTestConcreteCharacterBase>(
            Test,
            OutWorld,
            TEXT("Concrete character"),
            TEXT("AeonCharacterBaseTestWorld"));
    }
} // namespace AeonCharacterBaseTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonCharacterBaseValidationAbstractClassAllowsMissingAbilitySetTest,
                                 "Aeon.Validation.CharacterBase.AbstractClassAllowsMissingAbilitySet",
                                 AeonTests::AutomationTestFlags)
bool FAeonCharacterBaseValidationAbstractClassAllowsMissingAbilitySetTest::RunTest(const FString&)
{
    const auto Character = GetMutableDefault<AAeonTestAbstractCharacterBase>();
    return AeonTests::TestValidation(*this, Character, EDataValidationResult::Valid);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonCharacterBaseValidationConcreteClassRequiresAbilitySetTest,
                                 "Aeon.Validation.CharacterBase.ConcreteClassRequiresAbilitySet",
                                 AeonTests::AutomationTestFlags)
bool FAeonCharacterBaseValidationConcreteClassRequiresAbilitySetTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Character = AeonCharacterBaseTests::CreateConcreteCharacter(*this, World))
    {
        return AeonTests::TestValidation(*this, Character, EDataValidationResult::Invalid, TEXT("AbilitySet"));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonCharacterBaseValidationConcreteClassValidAbilitySetTest,
                                 "Aeon.Validation.CharacterBase.ConcreteClassValidAbilitySet",
                                 AeonTests::AutomationTestFlags)
bool FAeonCharacterBaseValidationConcreteClassValidAbilitySetTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Character = AeonCharacterBaseTests::CreateConcreteCharacter(*this, World))
    {
        const TSoftObjectPtr<UAeonAbilitySet> AbilitySet(AeonTests::NewTransientObject<UAeonAbilitySet>(Character));
        if (TestTrue(TEXT("Should set AbilitySet"),
                     AeonTests::SetPropertyValue(*this, Character, TEXT("AbilitySet"), AbilitySet)))
        {
            return AeonTests::TestValidation(*this, Character, EDataValidationResult::Valid);
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
