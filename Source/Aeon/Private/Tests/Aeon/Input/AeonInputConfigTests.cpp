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
    #include "Misc/DataValidation.h"
    #include "NativeGameplayTags.h"
    #include "UObject/UnrealType.h"

namespace AeonInputConfigTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestNativeInputTag, "Input.Test.Native");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestAbilityInputTag, "Input.Test.Ability");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestUnknownInputTag, "Input.Test.Unknown");

    template <typename TObject>
    TObject* NewTransientObject(UObject* Outer = GetTransientPackage())
    {
        return NewObject<TObject>(Outer, NAME_None, RF_Transient);
    }

    template <typename TObject, typename TValue>
    bool SetPropertyValue(FAutomationTestBase& Test, TObject* Object, const TCHAR* PropertyName, const TValue& Value)
    {
        const auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
        if (Test.TestNotNull(FString::Printf(TEXT("Property %s should exist"), PropertyName), Property))
        {
            auto ValuePtr = Property->template ContainerPtrToValuePtr<TValue>(Object);
            if (Test.TestNotNull(FString::Printf(TEXT("Property %s should be writable"), PropertyName), ValuePtr))
            {
                *ValuePtr = Value;
                return true;
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

    FDataValidationContext CreateValidationContext()
    {
        return FDataValidationContext(true, EDataValidationUsecase::Manual, TConstArrayView<FAssetData>{});
    }

    bool ValidationContextContainsIssue(const FDataValidationContext& Context, const FString& ExpectedFragment)
    {
        for (const auto& Issue : Context.GetIssues())
        {
            if (Issue.Message.ToString().Contains(ExpectedFragment))
            {
                return true;
            }
        }

        return false;
    }

    bool TestValidation(FAutomationTestBase& Test,
                        const UObject* Object,
                        const EDataValidationResult ExpectedResult,
                        const TCHAR* ExpectedIssueFragment = nullptr)
    {
        auto Context = CreateValidationContext();
        const auto ActualResult = Object->IsDataValid(Context);
        const auto bResultMatches =
            Test.TestEqual(TEXT("Validation result should match expectation"), ActualResult, ExpectedResult);
        if (!ExpectedIssueFragment)
        {
            return bResultMatches;
        }
        else
        {
            return Test.TestTrue(FString::Printf(TEXT("Validation issues should contain '%s'"), ExpectedIssueFragment),
                                 ValidationContextContainsIssue(Context, ExpectedIssueFragment))
                && bResultMatches;
        }
    }

    UAeonInputConfig* CreateValidInputConfig(FAutomationTestBase& Test)
    {
        const auto InputConfig = NewTransientObject<UAeonInputConfig>();
        Test.TestNotNull(TEXT("Input config should be created"), InputConfig);

        TArray<FAeonNativeInputAction> NativeInputActions;
        FAeonNativeInputAction NativeInputAction;
        NativeInputAction.InputTag = TestNativeInputTag;
        NativeInputAction.InputAction = NewTransientObject<UInputAction>(InputConfig);
        NativeInputActions.Add(NativeInputAction);

        TArray<FAeonAbilityInputAction> AbilityInputActions;
        FAeonAbilityInputAction AbilityInputAction;
        AbilityInputAction.InputTag = TestAbilityInputTag;
        AbilityInputAction.InputAction = NewTransientObject<UInputAction>(InputConfig);
        AbilityInputActions.Add(AbilityInputAction);

        Test.TestTrue(TEXT("Should set DefaultMappingContext"),
                      SetPropertyValue(Test,
                                       InputConfig,
                                       TEXT("DefaultMappingContext"),
                                       NewTransientObject<UInputMappingContext>()));
        Test.TestTrue(TEXT("Should set NativeInputActions"),
                      SetPropertyValue(Test, InputConfig, TEXT("NativeInputActions"), NativeInputActions));
        Test.TestTrue(TEXT("Should set AbilityInputActions"),
                      SetPropertyValue(Test, InputConfig, TEXT("AbilityInputActions"), AbilityInputActions));
        return InputConfig;
    }
} // namespace AeonInputConfigTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigFindNativeInputActionByTagMatchingTagTest,
                                 "Aeon.InputConfig.FindNativeInputActionByTag.MatchingTag",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigFindNativeInputActionByTagMatchingTagTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);
    const auto* Result = InputConfig->FindNativeInputActionByTag(AeonInputConfigTests::TestNativeInputTag);
    return TestNotNull(TEXT("Matching tag should return the configured input action"), Result);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigFindNativeInputActionByTagUnknownTagTest,
                                 "Aeon.InputConfig.FindNativeInputActionByTag.UnknownTag",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigFindNativeInputActionByTagUnknownTagTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);
    return TestNull(TEXT("Unknown tags should not resolve to a native input action"),
                    InputConfig->FindNativeInputActionByTag(AeonInputConfigTests::TestUnknownInputTag));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationValidConfigTest,
                                 "Aeon.Validation.InputConfig.ValidConfig",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigValidationValidConfigTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);
    return AeonInputConfigTests::TestValidation(*this, InputConfig, EDataValidationResult::Valid);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationNativeInputTagInvalidTest,
                                 "Aeon.Validation.InputConfig.NativeInputTagInvalid",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigValidationNativeInputTagInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonNativeInputAction> NativeInputActions;
    FAeonNativeInputAction NativeInputAction;
    NativeInputAction.InputAction = AeonInputConfigTests::NewTransientObject<UInputAction>(InputConfig);
    NativeInputActions.Add(NativeInputAction);
    TestTrue(
        TEXT("Should set NativeInputActions"),
        AeonInputConfigTests::SetPropertyValue(*this, InputConfig, TEXT("NativeInputActions"), NativeInputActions));

    return AeonInputConfigTests::TestValidation(*this,
                                                InputConfig,
                                                EDataValidationResult::Invalid,
                                                TEXT("NativeInputActions[0].InputTag is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationNativeInputActionInvalidTest,
                                 "Aeon.Validation.InputConfig.NativeInputActionInvalid",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigValidationNativeInputActionInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonNativeInputAction> NativeInputActions;
    FAeonNativeInputAction NativeInputAction;
    NativeInputAction.InputTag = AeonInputConfigTests::TestNativeInputTag;
    NativeInputActions.Add(NativeInputAction);
    TestTrue(
        TEXT("Should set NativeInputActions"),
        AeonInputConfigTests::SetPropertyValue(*this, InputConfig, TEXT("NativeInputActions"), NativeInputActions));

    return AeonInputConfigTests::TestValidation(*this,
                                                InputConfig,
                                                EDataValidationResult::Invalid,
                                                TEXT("NativeInputActions[0].InputAction is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationAbilityInputTagInvalidTest,
                                 "Aeon.Validation.InputConfig.AbilityInputTagInvalid",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigValidationAbilityInputTagInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonAbilityInputAction> AbilityInputActions;
    FAeonAbilityInputAction AbilityInputAction;
    AbilityInputAction.InputAction = AeonInputConfigTests::NewTransientObject<UInputAction>(InputConfig);
    AbilityInputActions.Add(AbilityInputAction);
    TestTrue(
        TEXT("Should set AbilityInputActions"),
        AeonInputConfigTests::SetPropertyValue(*this, InputConfig, TEXT("AbilityInputActions"), AbilityInputActions));

    return AeonInputConfigTests::TestValidation(*this,
                                                InputConfig,
                                                EDataValidationResult::Invalid,
                                                TEXT("AbilityInputActions[0].InputTag is invalid"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonInputConfigValidationAbilityInputActionInvalidTest,
                                 "Aeon.Validation.InputConfig.AbilityInputActionInvalid",
                                 AeonInputConfigTests::AutomationTestFlags)
bool FAeonInputConfigValidationAbilityInputActionInvalidTest::RunTest(const FString&)
{
    const auto InputConfig = AeonInputConfigTests::CreateValidInputConfig(*this);

    TArray<FAeonAbilityInputAction> AbilityInputActions;
    FAeonAbilityInputAction AbilityInputAction;
    AbilityInputAction.InputTag = AeonInputConfigTests::TestAbilityInputTag;
    AbilityInputActions.Add(AbilityInputAction);
    TestTrue(
        TEXT("Should set AbilityInputActions"),
        AeonInputConfigTests::SetPropertyValue(*this, InputConfig, TEXT("AbilityInputActions"), AbilityInputActions));

    return AeonInputConfigTests::TestValidation(*this,
                                                InputConfig,
                                                EDataValidationResult::Invalid,
                                                TEXT("AbilityInputActions[0].InputAction is invalid"));
}

#endif
