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

    #include "AeonAnimation/AnimNotifies/AnimNotifyState_ApplyGameplayEffect.h"
    #include "Engine/TargetPoint.h"
    #include "GameplayEffect.h"
    #include "Misc/AutomationTest.h"
    #include "Tests/AeonAnimation/AeonAnimationAutomationTestHelpers.h"
    #include "Tests/AeonAnimation/AeonAnimationAutomationTestTypes.h"

namespace AnimNotifyStateApplyGameplayEffectTests
{
    constexpr auto AutomationTestFlags = AeonAnimationTests::AutomationTestFlags;
    constexpr float FloatTolerance = KINDA_SMALL_NUMBER;
    constexpr float ExpectedEffectValue = 0.35f;
    constexpr TCHAR EffectClassPropertyName[] = TEXT("EffectClass");
    constexpr TCHAR EffectLevelPropertyName[] = TEXT("EffectLevel");
    constexpr TCHAR BlueprintEffectName[] = TEXT("BP_AeonAnimationTestInfiniteGameplayEffect");

    bool GrantTestAttributeSet(FAutomationTestBase& Test, AAeonAnimationTestActor* const Actor)
    {
        const auto AbilitySystemComponent = Actor ? Actor->GetAeonAbilitySystemComponent() : nullptr;
        if (Test.TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
        {
            const auto AttributeSet =
                AeonAnimationTests::NewTransientObject<UAeonAnimationTestGameplayEffectAttributeSet>(Actor);
            if (Test.TestNotNull(TEXT("Test attribute set should be created"), AttributeSet))
            {
                AttributeSet->InitEffectValue(1.f);
                AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
                return Test.TestNotNull(TEXT("Test attribute set should be registered"),
                                        AbilitySystemComponent->GetSet<UAeonAnimationTestGameplayEffectAttributeSet>());
            }
        }
        return false;
    }

    AAeonAnimationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                      TUniquePtr<AeonAnimationTests::FTestWorld>& OutWorld)
    {
        OutWorld = MakeUnique<AeonAnimationTests::FTestWorld>(TEXT("AeonAnimationApplyGameplayEffectWorld"));
        if (Test.TestTrue(TEXT("Automation test world should be valid"), OutWorld && OutWorld->IsValid()))
        {
            const auto Actor = OutWorld->SpawnActor<AAeonAnimationTestActor>();
            if (Test.TestNotNull(TEXT("Animation test actor should spawn"), Actor))
            {
                const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
                if (Test.TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
                {
                    AbilitySystemComponent->InitAbilityActorInfo(Actor, Actor);
                    if (GrantTestAttributeSet(Test, Actor))
                    {
                        return Actor;
                    }
                }
            }
        }

        return nullptr;
    }

    USkeletalMeshComponent* CreateMeshWithoutAbilitySystem(FAutomationTestBase& Test,
                                                           TUniquePtr<AeonAnimationTests::FTestWorld>& OutWorld)
    {
        OutWorld = MakeUnique<AeonAnimationTests::FTestWorld>(TEXT("AeonAnimationApplyGameplayEffectNoASCWorld"));
        if (Test.TestTrue(TEXT("Automation test world should be valid"), OutWorld && OutWorld->IsValid()))
        {
            const auto Actor = OutWorld->SpawnActor<ATargetPoint>();
            if (Test.TestNotNull(TEXT("Owning actor should spawn"), Actor))
            {
                const auto MeshComp = NewObject<USkeletalMeshComponent>(Actor, NAME_None, RF_Transient);
                if (Test.TestNotNull(TEXT("Mesh component should be created"), MeshComp))
                {
                    Actor->SetRootComponent(MeshComp);
                    Actor->AddOwnedComponent(MeshComp);
                    MeshComp->RegisterComponent();
                    return MeshComp;
                }
            }
        }

        return nullptr;
    }

    template <typename TNotify = UAnimNotifyState_ApplyGameplayEffect>
    TNotify* CreateConfiguredNotify(FAutomationTestBase& Test)
    {
        const auto Notify = AeonAnimationTests::NewTransientObject<TNotify>();
        if (Test.TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify))
        {
            Test.TestTrue(TEXT("Apply-gameplay-effect notify should be configured with an infinite gameplay effect"),
                          AeonAnimationTests::SetPropertyValue(
                              Test,
                              Notify,
                              EffectClassPropertyName,
                              TSubclassOf<UGameplayEffect>(UAeonAnimationTestInfiniteGameplayEffect::StaticClass())));
        }
        return Notify;
    }

    float GetEffectValue(const AAeonAnimationTestActor* const Actor)
    {
        const auto AbilitySystemComponent = Actor ? Actor->GetAeonAbilitySystemComponent() : nullptr;
        const auto AttributeSet = AbilitySystemComponent
            ? AbilitySystemComponent->GetSet<UAeonAnimationTestGameplayEffectAttributeSet>()
            : nullptr;
        return AttributeSet ? AttributeSet->GetEffectValue() : 0.f;
    }
} // namespace AnimNotifyStateApplyGameplayEffectTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifyStateApplyGameplayEffectValidationMissingEffectClassTest,
                                 "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Validation.MissingEffectClass",
                                 AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectValidationMissingEffectClassTest::RunTest(const FString&)
{
    const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotifyState_ApplyGameplayEffect>();
    return TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify)
        && AeonAnimationTests::TestValidation(*this, Notify, EDataValidationResult::Invalid, TEXT("EffectClass"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifyStateApplyGameplayEffectDefaultsEffectLevelToOneTest,
                                 "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Defaults.EffectLevelToOne",
                                 AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectDefaultsEffectLevelToOneTest::RunTest(const FString&)
{
    const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotifyState_ApplyGameplayEffect>();
    return TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify)
        && TestEqual(TEXT("EffectLevel should default to 1"),
                     AeonAnimationTests::GetPropertyValue<float>(
                         *this,
                         Notify,
                         AnimNotifyStateApplyGameplayEffectTests::EffectLevelPropertyName),
                     1.f);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifyStateApplyGameplayEffectValidationEffectMustBeInfiniteTest,
                                 "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Validation.EffectMustBeInfinite",
                                 AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectValidationEffectMustBeInfiniteTest::RunTest(const FString&)
{
    const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotifyState_ApplyGameplayEffect>();
    return TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify)
        && TestTrue(TEXT("Apply-gameplay-effect notify should accept the duration gameplay effect for validation"),
                    AeonAnimationTests::SetPropertyValue(
                        *this,
                        Notify,
                        AnimNotifyStateApplyGameplayEffectTests::EffectClassPropertyName,
                        TSubclassOf<UGameplayEffect>(UAeonAnimationTestDurationGameplayEffect::StaticClass())))
        && AeonAnimationTests::TestValidation(*this, Notify, EDataValidationResult::Invalid, TEXT("infinite policy"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifyStateApplyGameplayEffectValidationValidWhenConfiguredTest,
                                 "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Validation.ValidWhenConfigured",
                                 AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectValidationValidWhenConfiguredTest::RunTest(const FString&)
{
    const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this);
    return Notify ? AeonAnimationTests::TestValidation(*this, Notify, EDataValidationResult::Valid) : false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectNotifyNameFallsBackWhenUnconfiguredTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.NotifyName.FallsBackWhenUnconfigured",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectNotifyNameFallsBackWhenUnconfiguredTest::RunTest(const FString&)
{
    const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotifyState_ApplyGameplayEffect>();
    return TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify)
        && TestEqual(TEXT("Unconfigured notify should fall back to the display label"),
                     Notify->GetNotifyName_Implementation(),
                     FString(TEXT("Apply Gameplay Effect")));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifyStateApplyGameplayEffectNotifyNameUsesNativeDisplayNameTest,
                                 "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.NotifyName.UsesNativeDisplayName",
                                 AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectNotifyNameUsesNativeDisplayNameTest::RunTest(const FString&)
{
    if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
    {
        const auto ExpectedName =
            FString::Printf(TEXT("\u2192 %s"),
                            *UAeonAnimationTestInfiniteGameplayEffect::StaticClass()->GetDisplayNameText().ToString());
        return TestEqual(TEXT("Notify name should use the native effect display name"),
                         Notify->GetNotifyName_Implementation(),
                         ExpectedName);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectNotifyNameUsesBlueprintAssetNameWithoutGeneratedSuffixTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.NotifyName.UsesBlueprintAssetNameWithoutGeneratedSuffix",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectNotifyNameUsesBlueprintAssetNameWithoutGeneratedSuffixTest::RunTest(
    const FString&)
{
    const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotifyState_ApplyGameplayEffect>();
    if (TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify))
    {
        const auto PackageName =
            AeonAnimationTests::NewUniqueTestPackageName(TEXT("AnimNotifyStateApplyGameplayEffectNotifyName"));
        const auto Blueprint =
            AeonAnimationTests::NewBlueprint(UAeonAnimationTestInfiniteGameplayEffect::StaticClass(),
                                             PackageName,
                                             AnimNotifyStateApplyGameplayEffectTests::BlueprintEffectName);
        if (TestNotNull(TEXT("Gameplay effect blueprint should be created"), Blueprint))
        {
            AeonAnimationTests::CompileBlueprint(Blueprint);
            const auto GeneratedClass = Blueprint->GeneratedClass.Get();
            if (TestNotNull(TEXT("Gameplay effect blueprint should generate a class"), GeneratedClass))
            {
                const TSubclassOf<UGameplayEffect> BlueprintEffectClass(GeneratedClass);
                if (TestTrue(TEXT("Apply-gameplay-effect notify should accept the blueprint gameplay effect"),
                             AeonAnimationTests::SetPropertyValue(
                                 *this,
                                 Notify,
                                 AnimNotifyStateApplyGameplayEffectTests::EffectClassPropertyName,
                                 BlueprintEffectClass)))
                {
                    const auto NotifyName = Notify->GetNotifyName_Implementation();
                    return TestTrue(TEXT("Notify name should use the Blueprint asset name"),
                                    NotifyName.Contains(AnimNotifyStateApplyGameplayEffectTests::BlueprintEffectName))
                        && TestFalse(TEXT("Notify name should not expose the generated-class suffix"),
                                     NotifyName.Contains(TEXT("_C")));
                }
            }
        }
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectNotifyNameIncludesLevelWhenNotDefaultTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.NotifyName.IncludesLevelWhenNotDefault",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectNotifyNameIncludesLevelWhenNotDefaultTest::RunTest(const FString&)
{
    if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
    {
        if (TestTrue(
                TEXT("Apply-gameplay-effect notify should accept a non-default level"),
                AeonAnimationTests::SetPropertyValue(*this,
                                                     Notify,
                                                     AnimNotifyStateApplyGameplayEffectTests::EffectLevelPropertyName,
                                                     2.f)))
        {
            return TestTrue(TEXT("Notify name should append the authored level when it is not the default"),
                            Notify->GetNotifyName_Implementation().Contains(TEXT("(Lv 2)")));
        }
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectRuntimeNotifyBeginAppliesInfiniteEffectTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.NotifyBeginAppliesInfiniteEffect",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeNotifyBeginAppliesInfiniteEffectTest::RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto Actor = AnimNotifyStateApplyGameplayEffectTests::CreateAbilitySystemActor(*this, World))
    {
        if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            if (TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(),
                                    nullptr,
                                    0.5f,
                                    FAnimNotifyEventReference());

                return TestEqual(TEXT("Notify begin should apply one active gameplay effect"),
                                 AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                                 1)
                    && TestTrue(TEXT("Notify begin should apply the configured modifier"),
                                FMath::IsNearlyEqual(AnimNotifyStateApplyGameplayEffectTests::GetEffectValue(Actor),
                                                     AnimNotifyStateApplyGameplayEffectTests::ExpectedEffectValue,
                                                     AnimNotifyStateApplyGameplayEffectTests::FloatTolerance));
            }
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectRuntimeNotifyEndRemovesAppliedEffectTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.NotifyEndRemovesAppliedEffect",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeNotifyEndRemovesAppliedEffectTest::RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto Actor = AnimNotifyStateApplyGameplayEffectTests::CreateAbilitySystemActor(*this, World))
    {
        if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            if (TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(),
                                    nullptr,
                                    0.5f,
                                    FAnimNotifyEventReference());
                Notify->NotifyEnd(Actor->GetSkeletalMeshComponentForTest(), nullptr, FAnimNotifyEventReference());

                return TestEqual(TEXT("Notify end should remove the applied gameplay effect"),
                                 AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                                 0)
                    && TestTrue(TEXT("Notify end should restore the base attribute value"),
                                FMath::IsNearlyEqual(AnimNotifyStateApplyGameplayEffectTests::GetEffectValue(Actor),
                                                     1.f,
                                                     AnimNotifyStateApplyGameplayEffectTests::FloatTolerance));
            }
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectRuntimeNotifyEndRemovesAppliedEffectWhenEndReferenceUsesDifferentNotifyPointerTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.NotifyEndRemovesAppliedEffectWhenEndReferenceUsesDifferentNotifyPointer",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeNotifyEndRemovesAppliedEffectWhenEndReferenceUsesDifferentNotifyPointerTest::
    RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto Actor = AnimNotifyStateApplyGameplayEffectTests::CreateAbilitySystemActor(*this, World))
    {
        if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            if (TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                FAnimNotifyEvent BeginNotifyEvent;
                BeginNotifyEvent.NotifyStateClass = Notify;

                FAnimNotifyEvent EndNotifyEvent;
                EndNotifyEvent.NotifyStateClass = Notify;

                const FAnimNotifyEventReference BeginReference(&BeginNotifyEvent, Notify);
                const FAnimNotifyEventReference EndReference(&EndNotifyEvent, Notify);

                Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(), nullptr, 0.5f, BeginReference);
                Notify->NotifyEnd(Actor->GetSkeletalMeshComponentForTest(), nullptr, EndReference);

                return TestEqual(
                           TEXT(
                               "Notify end should remove the applied gameplay effect even if the end reference uses a different notify pointer"),
                           AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                           0)
                    && TestTrue(
                           TEXT("Notify end should restore the base attribute value after the mismatched-pointer path"),
                           FMath::IsNearlyEqual(AnimNotifyStateApplyGameplayEffectTests::GetEffectValue(Actor),
                                                1.f,
                                                AnimNotifyStateApplyGameplayEffectTests::FloatTolerance));
            }
        }
    }

    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectRuntimeOverlappingExecutionsKeepHandlesIsolatedTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.OverlappingExecutionsKeepHandlesIsolated",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeOverlappingExecutionsKeepHandlesIsolatedTest::RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto Actor = AnimNotifyStateApplyGameplayEffectTests::CreateAbilitySystemActor(*this, World))
    {
        if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
        {
            const auto AbilitySystemComponent = Actor->GetAeonAbilitySystemComponent();
            if (TestNotNull(TEXT("Animation test actor should expose an Aeon ASC"), AbilitySystemComponent))
            {
                Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(),
                                    nullptr,
                                    0.5f,
                                    FAnimNotifyEventReference());
                Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(),
                                    nullptr,
                                    0.5f,
                                    FAnimNotifyEventReference());

                const bool bTwoEffectsApplied =
                    TestEqual(TEXT("Overlapping executions should apply two gameplay effects"),
                              AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                              2);

                Notify->NotifyEnd(Actor->GetSkeletalMeshComponentForTest(), nullptr, FAnimNotifyEventReference());

                const bool bOneEffectRemains =
                    TestEqual(TEXT("Ending one overlapping execution should leave one gameplay effect active"),
                              AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                              1);

                Notify->NotifyEnd(Actor->GetSkeletalMeshComponentForTest(), nullptr, FAnimNotifyEventReference());

                return bTwoEffectsApplied && bOneEffectRemains
                    && TestEqual(TEXT("Ending all overlapping executions should remove every gameplay effect"),
                                 AbilitySystemComponent->GetActiveEffects(FGameplayEffectQuery()).Num(),
                                 0);
            }
        }
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectRuntimeMissingAbilitySystemLogsAndNoOpsTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.MissingAbilitySystemLogsAndNoOps",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeMissingAbilitySystemLogsAndNoOpsTest::RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto MeshComp = AnimNotifyStateApplyGameplayEffectTests::CreateMeshWithoutAbilitySystem(*this, World))
    {
        if (const auto Notify = AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify(*this))
        {
            AddExpectedMessagePlain(TEXT("failed to resolve an ability system component"),
                                    ELogVerbosity::Warning,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);

            Notify->NotifyBegin(MeshComp, nullptr, 0.5f, FAnimNotifyEventReference());
            Notify->NotifyEnd(MeshComp, nullptr, FAnimNotifyEventReference());
            return true;
        }
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAnimNotifyStateApplyGameplayEffectRuntimeNullEffectClassLogsAndNoOpsTest,
    "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.NullEffectClassLogsAndNoOps",
    AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeNullEffectClassLogsAndNoOpsTest::RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto Actor = AnimNotifyStateApplyGameplayEffectTests::CreateAbilitySystemActor(*this, World))
    {
        const auto Notify = AeonAnimationTests::NewTransientObject<UAnimNotifyState_ApplyGameplayEffect>();
        if (TestNotNull(TEXT("Apply-gameplay-effect notify should be created"), Notify))
        {
            AddExpectedMessagePlain(TEXT("has no EffectClass configured"),
                                    ELogVerbosity::Warning,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);

            Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(), nullptr, 0.5f, FAnimNotifyEventReference());

            return TestEqual(TEXT("Null effect class should not apply a gameplay effect"),
                             Actor->GetAeonAbilitySystemComponent()->GetActiveEffects(FGameplayEffectQuery()).Num(),
                             0)
                && TestTrue(TEXT("Null effect class should leave the base attribute value unchanged"),
                            FMath::IsNearlyEqual(AnimNotifyStateApplyGameplayEffectTests::GetEffectValue(Actor),
                                                 1.f,
                                                 AnimNotifyStateApplyGameplayEffectTests::FloatTolerance));
        }
    }
    return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnimNotifyStateApplyGameplayEffectRuntimeApplyFailureLogsAndNoOpsTest,
                                 "Aeon.Animation.AnimNotifyState.ApplyGameplayEffect.Runtime.ApplyFailureLogsAndNoOps",
                                 AnimNotifyStateApplyGameplayEffectTests::AutomationTestFlags)
bool FAnimNotifyStateApplyGameplayEffectRuntimeApplyFailureLogsAndNoOpsTest::RunTest(const FString&)
{
    TUniquePtr<AeonAnimationTests::FTestWorld> World;
    if (const auto Actor = AnimNotifyStateApplyGameplayEffectTests::CreateAbilitySystemActor(*this, World))
    {
        if (const auto Notify =
                AnimNotifyStateApplyGameplayEffectTests::CreateConfiguredNotify<UAeonAnimationTestApplyFailureNotify>(
                    *this))
        {
            AddExpectedMessagePlain(TEXT("failed to apply effect"),
                                    ELogVerbosity::Warning,
                                    EAutomationExpectedMessageFlags::Contains,
                                    1);

            Notify->NotifyBegin(Actor->GetSkeletalMeshComponentForTest(), nullptr, 0.5f, FAnimNotifyEventReference());
            Notify->NotifyEnd(Actor->GetSkeletalMeshComponentForTest(), nullptr, FAnimNotifyEventReference());

            return TestEqual(TEXT("Apply failure should not leave a gameplay effect active"),
                             Actor->GetAeonAbilitySystemComponent()->GetActiveEffects(FGameplayEffectQuery()).Num(),
                             0)
                && TestTrue(TEXT("Apply failure should leave the base attribute value unchanged"),
                            FMath::IsNearlyEqual(AnimNotifyStateApplyGameplayEffectTests::GetEffectValue(Actor),
                                                 1.f,
                                                 AnimNotifyStateApplyGameplayEffectTests::FloatTolerance));
        }
    }

    return false;
}

#endif
