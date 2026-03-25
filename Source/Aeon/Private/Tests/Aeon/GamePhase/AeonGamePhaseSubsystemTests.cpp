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

    #include "Aeon/GamePhase/AeonGamePhaseSubsystem.h"
    #include "GameplayAbilitySpec.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace AeonGamePhaseSubsystemTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestPhaseTag, "Aeon.Test.GamePhase.Active");

    class FClientPieWorld
    {
    public:
        explicit FClientPieWorld(const TCHAR* WorldName = TEXT("AeonGamePhaseSubsystemClientWorld"))
        {
            if (GEngine)
            {
                World = UWorld::CreateWorld(
                    EWorldType::PIE,
                    false,
                    MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), FName(WorldName)),
                    GetTransientPackage(),
                    true);
                if (World)
                {
                    World->SetShouldTick(false);
                    World->SetPlayInEditorInitialNetMode(NM_Client);
                    World->AddToRoot();

                    auto& WorldContext = GEngine->CreateNewWorldContext(EWorldType::PIE);
                    WorldContext.SetCurrentWorld(World);
                }
            }
        }

        ~FClientPieWorld()
        {
            if (World)
            {
                World->RemoveFromRoot();
                if (GEngine)
                {
                    GEngine->DestroyWorldContext(World);
                }
                World->DestroyWorld(false);
            }
        }

        UWorld* Get() const { return World; }

    private:
        UWorld* World{ nullptr };
    };

    bool CreateSubsystemContext(FAutomationTestBase& Test,
                                TUniquePtr<AeonTests::FTestWorld>& OutWorld,
                                UAeonGamePhaseSubsystem*& OutSubsystem,
                                AAeonAutomationTestGameState*& OutGameState,
                                UAeonAutomationTestAbilitySystemComponent*& OutAbilitySystemComponent)
    {
        if (AeonTests::CreateTestWorld(Test, OutWorld, TEXT("AeonGamePhaseSubsystemTestWorld")))
        {
            OutSubsystem = OutWorld->Get()->GetSubsystem<UAeonGamePhaseSubsystem>();
            OutGameState = OutWorld->SpawnActor<AAeonAutomationTestGameState>();
            if (Test.TestNotNull(TEXT("Game phase subsystem should be created"), OutSubsystem)
                && Test.TestNotNull(TEXT("Automation test game state should spawn"), OutGameState))
            {
                OutWorld->Get()->SetGameState(OutGameState);
                OutAbilitySystemComponent = OutGameState->GetAeonAbilitySystemComponent();
                if (Test.TestNotNull(TEXT("Automation test game state should expose an Aeon ASC"),
                                     OutAbilitySystemComponent))
                {
                    OutAbilitySystemComponent->InitAbilityActorInfo(OutGameState, OutGameState);
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
        else
        {
            return false;
        }
    }
} // namespace AeonGamePhaseSubsystemTests

struct FAeonGamePhaseSubsystemTestAccess
{
    static bool ShouldCreateSubsystem(const UAeonGamePhaseSubsystem& Subsystem, UObject* Outer)
    {
        return Subsystem.ShouldCreateSubsystem(Outer);
    }

    static void ActivatePhaseAbility(UAeonGamePhaseSubsystem& Subsystem,
                                     UAbilitySystemComponent* AbilitySystemComponent,
                                     const FGameplayTag& PhaseTag,
                                     const TSubclassOf<UGameplayAbility>& Ability)
    {
        Subsystem.ActivatePhaseAbility(AbilitySystemComponent, PhaseTag, Ability);
    }

    static void ForcePhaseActive(UAeonGamePhaseSubsystem& Subsystem, const FGameplayTag& PhaseTag)
    {
        Subsystem.ActivePhases.Add(PhaseTag, FGameplayAbilitySpecHandle());
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAeonGamePhaseSubsystemShouldCreateSubsystemForAuthorityWorldAndRejectClientPieWorldTest,
    "Aeon.GamePhase.Subsystem.ShouldCreateSubsystemForAuthorityWorldAndRejectClientPieWorld",
    AeonTests::AutomationTestFlags)
bool FAeonGamePhaseSubsystemShouldCreateSubsystemForAuthorityWorldAndRejectClientPieWorldTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> AuthorityWorld;
    if (AeonTests::CreateTestWorld(*this, AuthorityWorld, TEXT("AeonGamePhaseSubsystemAuthorityWorld")))
    {
        const auto SubsystemCDO = GetMutableDefault<UAeonGamePhaseSubsystem>();
        AeonGamePhaseSubsystemTests::FClientPieWorld ClientWorld(TEXT("AeonGamePhaseSubsystemClientWorld"));
        return TestTrue(TEXT("Game phase subsystem should be creatable for an authority world"),
                        FAeonGamePhaseSubsystemTestAccess::ShouldCreateSubsystem(*SubsystemCDO, AuthorityWorld->Get()))
            && TestFalse(TEXT("Game phase subsystem should not be creatable for a client PIE world"),
                         FAeonGamePhaseSubsystemTestAccess::ShouldCreateSubsystem(*SubsystemCDO, ClientWorld.Get()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGamePhaseSubsystemRegistersPhaseAbilityAndRejectsDuplicatesTest,
                                 "Aeon.GamePhase.Subsystem.RegistersPhaseAbilityAndRejectsDuplicates",
                                 AeonTests::AutomationTestFlags)
bool FAeonGamePhaseSubsystemRegistersPhaseAbilityAndRejectsDuplicatesTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    UAeonGamePhaseSubsystem* Subsystem{ nullptr };
    AAeonAutomationTestGameState* GameState{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    if (AeonGamePhaseSubsystemTests::CreateSubsystemContext(*this, World, Subsystem, GameState, AbilitySystemComponent))
    {
        AddExpectedError(TEXT("RegisterPhaseAbility invoked for PhaseTag"), EAutomationExpectedErrorFlags::Contains, 1);

        Subsystem->RegisterPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag(),
                                        UAeonAutomationTestGameplayAbility::StaticClass());
        Subsystem->RegisterPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag(),
                                        UAeonAutomationTestGameplayAbility::StaticClass());

        return TestTrue(TEXT("RegisterPhaseAbility should record the initial phase ability"),
                        Subsystem->IsPhaseAbilityRegistered(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag()))
            && TestTrue(TEXT("Duplicate registration should preserve the original phase ability"),
                        Subsystem->GetPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag())
                            && *Subsystem->GetPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag())
                                == UAeonAutomationTestGameplayAbility::StaticClass());
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGamePhaseSubsystemActivatePhaseAbilityMarksPhaseActiveTest,
                                 "Aeon.GamePhase.Subsystem.ActivatePhaseAbilityMarksPhaseActive",
                                 AeonTests::AutomationTestFlags)
bool FAeonGamePhaseSubsystemActivatePhaseAbilityMarksPhaseActiveTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    UAeonGamePhaseSubsystem* Subsystem{ nullptr };
    AAeonAutomationTestGameState* GameState{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    if (AeonGamePhaseSubsystemTests::CreateSubsystemContext(*this, World, Subsystem, GameState, AbilitySystemComponent))
    {
        UAeonAutomationTestGameplayAbility::ResetCounters();
        FAeonGamePhaseSubsystemTestAccess::ActivatePhaseAbility(*Subsystem,
                                                                AbilitySystemComponent,
                                                                AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag(),
                                                                UAeonAutomationTestGameplayAbility::StaticClass());

        return TestTrue(TEXT("ActivatePhaseAbility should mark the phase as active"),
                        Subsystem->IsPhaseActive(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag()))
            && TestEqual(TEXT("ActivatePhaseAbility should activate the configured gameplay ability once"),
                         UAeonAutomationTestGameplayAbility::ActivationCount,
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGamePhaseSubsystemRegisterPhaseAbilityActivatesWhenPhaseAlreadyActiveTest,
                                 "Aeon.GamePhase.Subsystem.RegisterPhaseAbilityActivatesWhenPhaseAlreadyActive",
                                 AeonTests::AutomationTestFlags)
bool FAeonGamePhaseSubsystemRegisterPhaseAbilityActivatesWhenPhaseAlreadyActiveTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    UAeonGamePhaseSubsystem* Subsystem{ nullptr };
    AAeonAutomationTestGameState* GameState{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    if (AeonGamePhaseSubsystemTests::CreateSubsystemContext(*this, World, Subsystem, GameState, AbilitySystemComponent))
    {
        UAeonAutomationTestGameplayAbility::ResetCounters();
        FAeonGamePhaseSubsystemTestAccess::ForcePhaseActive(*Subsystem,
                                                            AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag());
        Subsystem->RegisterPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag(),
                                        UAeonAutomationTestGameplayAbility::StaticClass());

        return TestTrue(
                   TEXT("Registering a phase ability for an already active phase should activate it"),
                   AbilitySystemComponent->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass())
                       && AbilitySystemComponent
                              ->FindAbilitySpecFromClass(UAeonAutomationTestGameplayAbility::StaticClass())
                              ->IsActive())
            && TestEqual(TEXT("Registering an active phase should activate the gameplay ability once"),
                         UAeonAutomationTestGameplayAbility::ActivationCount,
                         1);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAeonGamePhaseSubsystemUnregisterPhaseAbilityCancelsActiveAbilityTest,
                                 "Aeon.GamePhase.Subsystem.UnregisterPhaseAbilityCancelsActiveAbility",
                                 AeonTests::AutomationTestFlags)
bool FAeonGamePhaseSubsystemUnregisterPhaseAbilityCancelsActiveAbilityTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    UAeonGamePhaseSubsystem* Subsystem{ nullptr };
    AAeonAutomationTestGameState* GameState{ nullptr };
    UAeonAutomationTestAbilitySystemComponent* AbilitySystemComponent{ nullptr };
    if (AeonGamePhaseSubsystemTests::CreateSubsystemContext(*this, World, Subsystem, GameState, AbilitySystemComponent))
    {
        UAeonAutomationTestGameplayAbility::ResetCounters();
        Subsystem->RegisterPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag(),
                                        UAeonAutomationTestGameplayAbility::StaticClass());
        FAeonGamePhaseSubsystemTestAccess::ActivatePhaseAbility(*Subsystem,
                                                                AbilitySystemComponent,
                                                                AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag(),
                                                                UAeonAutomationTestGameplayAbility::StaticClass());

        Subsystem->UnregisterPhaseAbility(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag());

        return TestFalse(TEXT("UnregisterPhaseAbility should remove the phase registration"),
                         Subsystem->IsPhaseAbilityRegistered(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag()))
            && TestFalse(TEXT("UnregisterPhaseAbility should clear the phase active state"),
                         Subsystem->IsPhaseActive(AeonGamePhaseSubsystemTests::TestPhaseTag.GetTag()))
            && TestEqual(TEXT("UnregisterPhaseAbility should end the active gameplay ability once"),
                         UAeonAutomationTestGameplayAbility::EndCount,
                         1);
    }
    else
    {
        return false;
    }
}

#endif
