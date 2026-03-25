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

    #include "Aeon/Input/AeonInputConfig.h"
    #include "EnhancedInput/Public/InputAction.h"
    #include "EnhancedInput/Public/InputMappingContext.h"
    #include "GameplayTagContainer.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"

namespace AeonInputConfigTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestNativeInputTag, "Input.Test.Native");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityInputTag, "Input.Test.Ability");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestUnknownInputTag, "Input.Test.Unknown");

    UAeonInputConfig* CreateValidInputConfig(FAutomationTestBase& Test)
    {
        const auto InputConfig = AeonTests::NewTransientObject<UAeonInputConfig>();
        Test.TestNotNull(TEXT("Input config should be created"), InputConfig);

        TArray<FAeonNativeInputAction> NativeInputActions;
        FAeonNativeInputAction NativeInputAction;
        NativeInputAction.InputTag = TestNativeInputTag;
        NativeInputAction.InputAction = AeonTests::NewTransientObject<UInputAction>(InputConfig);
        NativeInputActions.Add(NativeInputAction);

        TArray<FAeonAbilityInputAction> AbilityInputActions;
        FAeonAbilityInputAction AbilityInputAction;
        AbilityInputAction.InputTag = TestAbilityInputTag;
        AbilityInputAction.InputAction = AeonTests::NewTransientObject<UInputAction>(InputConfig);
        AbilityInputActions.Add(AbilityInputAction);

        Test.TestTrue(TEXT("Should set DefaultMappingContext"),
                      AeonTests::SetPropertyValue(Test,
                                                  InputConfig,
                                                  TEXT("DefaultMappingContext"),
                                                  AeonTests::NewTransientObject<UInputMappingContext>()));
        Test.TestTrue(TEXT("Should set NativeInputActions"),
                      AeonTests::SetPropertyValue(Test, InputConfig, TEXT("NativeInputActions"), NativeInputActions));
        Test.TestTrue(TEXT("Should set AbilityInputActions"),
                      AeonTests::SetPropertyValue(Test, InputConfig, TEXT("AbilityInputActions"), AbilityInputActions));
        return InputConfig;
    }
} // namespace AeonInputConfigTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigFindNativeInputActionByTagMatchingTagTest,
                                 "Aeon.InputConfig.FindNativeInputActionByTag.MatchingTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigFindNativeInputActionByTagMatchingTagTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);
    const auto Result = InputConfig->FindNativeInputActionByTag(AeonInputConfigTests::TestNativeInputTag);
    return TestNotNull(TEXT("Matching tag should return the configured input action"), Result);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigFindNativeInputActionByTagUnknownTagTest,
                                 "Aeon.InputConfig.FindNativeInputActionByTag.UnknownTag",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigFindNativeInputActionByTagUnknownTagTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);
    return TestNull(TEXT("Unknown tags should not resolve to a native input action"),
                    InputConfig->FindNativeInputActionByTag(AeonInputConfigTests::TestUnknownInputTag));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationValidConfigTest,
                                 "Aeon.Validation.InputConfig.ValidConfig",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigValidationValidConfigTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);
    return AeonTests::TestValidation(*this, InputConfig, EDataValidationResult::Valid);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationNativeInputTagInvalidTest,
                                 "Aeon.Validation.InputConfig.NativeInputTagInvalid",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigValidationNativeInputTagInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonNativeInputAction> NativeInputActions;
    FAeonNativeInputAction NativeInputAction;
    NativeInputAction.InputAction = AeonTests::NewTransientObject<UInputAction>(InputConfig);
    NativeInputActions.Add(NativeInputAction);
    TestTrue(TEXT("Should set NativeInputActions"),
             AeonTests::SetPropertyValue(*this, InputConfig, TEXT("NativeInputActions"), NativeInputActions));

    return AeonTests::TestValidation(*this,
                                     InputConfig,
                                     EDataValidationResult::Invalid,
                                     TEXT("NativeInputActions[0].InputTag is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationNativeInputActionInvalidTest,
                                 "Aeon.Validation.InputConfig.NativeInputActionInvalid",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigValidationNativeInputActionInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonNativeInputAction> NativeInputActions;
    FAeonNativeInputAction NativeInputAction;
    NativeInputAction.InputTag = AeonInputConfigTests::TestNativeInputTag;
    NativeInputActions.Add(NativeInputAction);
    TestTrue(TEXT("Should set NativeInputActions"),
             AeonTests::SetPropertyValue(*this, InputConfig, TEXT("NativeInputActions"), NativeInputActions));

    return AeonTests::TestValidation(*this,
                                     InputConfig,
                                     EDataValidationResult::Invalid,
                                     TEXT("NativeInputActions[0].InputAction is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationAbilityInputTagInvalidTest,
                                 "Aeon.Validation.InputConfig.AbilityInputTagInvalid",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigValidationAbilityInputTagInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonAbilityInputAction> AbilityInputActions;
    FAeonAbilityInputAction AbilityInputAction;
    AbilityInputAction.InputAction = AeonTests::NewTransientObject<UInputAction>(InputConfig);
    AbilityInputActions.Add(AbilityInputAction);
    TestTrue(TEXT("Should set AbilityInputActions"),
             AeonTests::SetPropertyValue(*this, InputConfig, TEXT("AbilityInputActions"), AbilityInputActions));

    return AeonTests::TestValidation(*this,
                                     InputConfig,
                                     EDataValidationResult::Invalid,
                                     TEXT("AbilityInputActions[0].InputTag is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationAbilityInputActionInvalidTest,
                                 "Aeon.Validation.InputConfig.AbilityInputActionInvalid",
                                 AeonTests::AutomationTestFlags)
bool FAeonInputConfigValidationAbilityInputActionInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonAbilityInputAction> AbilityInputActions;
    FAeonAbilityInputAction AbilityInputAction;
    AbilityInputAction.InputTag = AeonInputConfigTests::TestAbilityInputTag;
    AbilityInputActions.Add(AbilityInputAction);
    TestTrue(TEXT("Should set AbilityInputActions"),
             AeonTests::SetPropertyValue(*this, InputConfig, TEXT("AbilityInputActions"), AbilityInputActions));

    return AeonTests::TestValidation(*this,
                                     InputConfig,
                                     EDataValidationResult::Invalid,
                                     TEXT("AbilityInputActions[0].InputAction is invalid"));
}

#endif
