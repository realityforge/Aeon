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

    #include "Aeon/Subsystems/GroupAbilitySystem.h"
    #include "GameplayEffectTypes.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/Aeon/AeonAutomationTestHelpers.h"
    #include "Tests/Aeon/AeonAutomationTestTypes.h"

namespace GroupAbilitySystemTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestParentGroupTag, "Aeon.Test.Groups.Parent");
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestChildGroupTag, "Aeon.Test.Groups.Parent.Child");

    class FClientPieWorld
    {
    public:
        explicit FClientPieWorld(const TCHAR* WorldName = TEXT("GroupAbilitySystemClientWorld"))
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

    AAeonAutomationTestActor* CreateAbilitySystemActor(FAutomationTestBase& Test,
                                                       TUniquePtr<AeonTests::FTestWorld>& OutWorld)
    {
        if (const auto Actor =
                AeonTests::SpawnActorInFreshTestWorld<AAeonAutomationTestActor>(Test,
                                                                                OutWorld,
                                                                                TEXT("Ability-system test actor"),
                                                                                TEXT("GroupAbilitySystemTestWorld")))
        {
            return AeonTests::GetInitializedAbilitySystemComponent(Test, Actor) ? Actor : nullptr;
        }
        else
        {
            return nullptr;
        }
    }
} // namespace GroupAbilitySystemTests

struct FGroupAbilitySystemTestAccess
{
    static bool ShouldCreateSubsystem(const UGroupAbilitySystem& Subsystem, UObject* Outer)
    {
        return Subsystem.ShouldCreateSubsystem(Outer);
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemShouldCreateSubsystemForAuthorityWorldAndRejectClientPieWorldTest,
    "Aeon.Subsystems.GroupAbilitySystem.ShouldCreateSubsystemForAuthorityWorldAndRejectClientPieWorld",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemShouldCreateSubsystemForAuthorityWorldAndRejectClientPieWorldTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> AuthorityWorld;
    if (AeonTests::CreateTestWorld(*this, AuthorityWorld, TEXT("GroupAbilitySystemAuthorityWorld")))
    {
        const auto SubsystemCDO = GetMutableDefault<UGroupAbilitySystem>();
        GroupAbilitySystemTests::FClientPieWorld ClientWorld(TEXT("GroupAbilitySystemClientWorld"));
        return TestTrue(TEXT("Group ability subsystem should be creatable for an authority world"),
                        FGroupAbilitySystemTestAccess::ShouldCreateSubsystem(*SubsystemCDO, AuthorityWorld->Get()))
            && TestFalse(TEXT("Group ability subsystem should not be creatable for a client PIE world"),
                         FGroupAbilitySystemTestAccess::ShouldCreateSubsystem(*SubsystemCDO, ClientWorld.Get()));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGroupAbilitySystemParentGroupAbilityAppliesToChildRegisteredComponentTest,
    "Aeon.Subsystems.GroupAbilitySystem.ParentGroupAbilityAppliesToChildRegisteredComponent",
    AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemParentGroupAbilityAppliesToChildRegisteredComponentTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemTests::CreateAbilitySystemActor(*this, World))
    {
        const auto GroupAbilitySystem = World->Get()->GetSubsystem<UGroupAbilitySystem>();
        if (TestNotNull(TEXT("Group ability subsystem should be created"), GroupAbilitySystem))
        {
            GroupAbilitySystem->RegisterAbilitySystemComponent(GroupAbilitySystemTests::TestChildGroupTag.GetTag(),
                                                               Actor->GetAeonAbilitySystemComponent());
            GroupAbilitySystem->AddAbilityToGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                  UAeonAutomationTestGameplayAbility::StaticClass());

            return TestTrue(TEXT("Registered child-group ASC should be implicitly present in the parent group"),
                            GroupAbilitySystem->IsAbilitySystemComponentRegistered(
                                GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                Actor->GetAeonAbilitySystemComponent(),
                                true))
                && TestTrue(
                       TEXT("Parent group should report the ability as added"),
                       GroupAbilitySystem->IsAbilityAddedToGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                                 UAeonAutomationTestGameplayAbility::StaticClass()))
                && TestNotNull(TEXT("Adding an ability to the parent group should grant it to child-group members"),
                               Actor->GetAeonAbilitySystemComponent()->FindAbilitySpecFromClass(
                                   UAeonAutomationTestGameplayAbility::StaticClass()));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGroupAbilitySystemRegisterAfterAddingEffectAppliesExistingEffectTest,
                                 "Aeon.Subsystems.GroupAbilitySystem.RegisterAfterAddingEffectAppliesExistingEffect",
                                 AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemRegisterAfterAddingEffectAppliesExistingEffectTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemTests::CreateAbilitySystemActor(*this, World))
    {
        const auto GroupAbilitySystem = World->Get()->GetSubsystem<UGroupAbilitySystem>();
        if (TestNotNull(TEXT("Group ability subsystem should be created"), GroupAbilitySystem))
        {
            GroupAbilitySystem->AddEffectToGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                 UAeonAutomationTestGameplayEffect::StaticClass());
            GroupAbilitySystem->RegisterAbilitySystemComponent(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                               Actor->GetAeonAbilitySystemComponent());

            return TestTrue(
                       TEXT("Parent group should report the effect as added"),
                       GroupAbilitySystem->IsEffectAddedToGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                                UAeonAutomationTestGameplayEffect::StaticClass()))
                && TestEqual(TEXT("Registering after adding a group effect should apply it immediately"),
                             Actor->GetAeonAbilitySystemComponent()->GetActiveEffects(FGameplayEffectQuery()).Num(),
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGroupAbilitySystemRemoveAbilityAndEffectClearsGrantedEntriesTest,
                                 "Aeon.Subsystems.GroupAbilitySystem.RemoveAbilityAndEffectClearsGrantedEntries",
                                 AeonTests::AutomationTestFlags)
bool FGroupAbilitySystemRemoveAbilityAndEffectClearsGrantedEntriesTest::RunTest(const FString&)
{
    TUniquePtr<AeonTests::FTestWorld> World;
    if (const auto Actor = GroupAbilitySystemTests::CreateAbilitySystemActor(*this, World))
    {
        const auto GroupAbilitySystem = World->Get()->GetSubsystem<UGroupAbilitySystem>();
        if (TestNotNull(TEXT("Group ability subsystem should be created"), GroupAbilitySystem))
        {
            GroupAbilitySystem->RegisterAbilitySystemComponent(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                               Actor->GetAeonAbilitySystemComponent());
            GroupAbilitySystem->AddAbilityToGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                  UAeonAutomationTestGameplayAbility::StaticClass());
            GroupAbilitySystem->AddEffectToGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                 UAeonAutomationTestGameplayEffect::StaticClass());
            GroupAbilitySystem->RemoveAbilityFromGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                       UAeonAutomationTestGameplayAbility::StaticClass());
            GroupAbilitySystem->RemoveEffectFromGroup(GroupAbilitySystemTests::TestParentGroupTag.GetTag(),
                                                      UAeonAutomationTestGameplayEffect::StaticClass());

            return TestNull(TEXT("Removing the group ability should clear the granted spec from members"),
                            Actor->GetAeonAbilitySystemComponent()->FindAbilitySpecFromClass(
                                UAeonAutomationTestGameplayAbility::StaticClass()))
                && TestEqual(TEXT("Removing the group effect should clear the applied effect from members"),
                             Actor->GetAeonAbilitySystemComponent()->GetActiveEffects(FGameplayEffectQuery()).Num(),
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
