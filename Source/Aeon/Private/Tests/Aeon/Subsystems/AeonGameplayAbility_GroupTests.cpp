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

    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/Subsystems/AeonGameplayAbility_GroupTestTypes.h"

namespace AeonGameplayAbilityGroupTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestGroupTag, "Aeon.Test.Group.Primary");
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityGroupValidationAbstractClassAllowsMissingGroupTagTest,
                                 "Aeon.Validation.GameplayAbility_Group.AbstractClassAllowsMissingGroupTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonGameplayAbilityGroupValidationAbstractClassAllowsMissingGroupTagTest::RunTest(const FString&)
{
    const auto Ability = GetMutableDefault<UAeonTestAbstractGameplayAbility_Group>();
    return AeonTests::TestValidation(*this, Ability, EDataValidationResult::Valid);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityGroupValidationConcreteClassRequiresGroupTagTest,
                                 "Aeon.Validation.GameplayAbility_Group.ConcreteClassRequiresGroupTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonGameplayAbilityGroupValidationConcreteClassRequiresGroupTagTest::RunTest(const FString&)
{
    const auto Ability = AeonTests::NewTransientObject<UAeonTestConcreteGameplayAbility_Group>();
    return AeonTests::TestValidation(*this, Ability, EDataValidationResult::Invalid, TEXT("GroupTag is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGameplayAbilityGroupValidationConcreteClassValidGroupTagTest,
                                 "Aeon.Validation.GameplayAbility_Group.ConcreteClassValidGroupTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonGameplayAbilityGroupValidationConcreteClassValidGroupTagTest::RunTest(const FString&)
{
    const auto Ability = AeonTests::NewTransientObject<UAeonTestConcreteGameplayAbility_Group>();
    if (TestTrue(TEXT("Should set GroupTag"),
                 AeonTests::SetPropertyValue(*this,
                                             Ability,
                                             TEXT("GroupTag"),
                                             AeonGameplayAbilityGroupTests::TestGroupTag.GetTag())))
    {
        return AeonTests::TestValidation(*this, Ability, EDataValidationResult::Valid);
    }
    else
    {
        return false;
    }
}

#endif
