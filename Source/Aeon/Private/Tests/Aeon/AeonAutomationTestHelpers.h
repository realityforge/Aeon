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
#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

    #include "AssetRegistry/AssetData.h"
    #include "Engine/Engine.h"
    #include "Engine/World.h"
    #include "GameFramework/Actor.h"
    #include "GameplayTagContainer.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/DataValidation.h"
    #include "UObject/UnrealType.h"

namespace AeonTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    class FTestWorld
    {
    public:
        explicit FTestWorld(const TCHAR* WorldName = TEXT("AeonAutomationTestWorld"))
        {
            if (GEngine)
            {
                World = UWorld::CreateWorld(
                    EWorldType::Game,
                    false,
                    MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), FName(WorldName)),
                    GetTransientPackage(),
                    true);
                if (World)
                {
                    World->SetShouldTick(false);
                    World->AddToRoot();

                    auto& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
                    WorldContext.SetCurrentWorld(World);
                }
            }
        }

        ~FTestWorld()
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

        bool IsValid() const { return nullptr != World; }

        UWorld* Get() const { return World; }

        template <typename TActor>
        TActor* SpawnActor(const FVector& Location = FVector::ZeroVector,
                           const FRotator& Rotation = FRotator::ZeroRotator) const
        {
            if (World)
            {
                FActorSpawnParameters SpawnParameters;
                SpawnParameters.ObjectFlags |= RF_Transient;
                SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                return World->SpawnActor<TActor>(TActor::StaticClass(), Location, Rotation, SpawnParameters);
            }
            else
            {
                return nullptr;
            }
        }

    private:
        UWorld* World{ nullptr };
    };

    inline FGameplayTagContainer MakeTagContainer(const std::initializer_list<FGameplayTag> Tags)
    {
        FGameplayTagContainer Container;
        for (const auto& Tag : Tags)
        {
            Container.AddTag(Tag);
        }

        return Container;
    }

    inline bool CreateTestWorld(FAutomationTestBase& Test,
                                TUniquePtr<FTestWorld>& OutWorld,
                                const TCHAR* WorldName = TEXT("AeonAutomationTestWorld"))
    {
        OutWorld = MakeUnique<FTestWorld>(WorldName);
        return Test.TestNotNull(TEXT("Automation test world should be created"), OutWorld.Get())
            && Test.TestTrue(TEXT("Automation test world should be valid"), OutWorld->IsValid());
    }

    template <typename TActor>
    TActor* SpawnActorInFreshTestWorld(FAutomationTestBase& Test,
                                       TUniquePtr<FTestWorld>& OutWorld,
                                       const TCHAR* ActorLabel,
                                       const TCHAR* WorldName = TEXT("AeonAutomationTestWorld"))
    {
        if (CreateTestWorld(Test, OutWorld, WorldName))
        {
            const auto Actor = OutWorld->SpawnActor<TActor>();
            Test.TestNotNull(FString::Printf(TEXT("%s should spawn"), ActorLabel), Actor);
            return Actor;
        }
        else
        {
            return nullptr;
        }
    }

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

    inline FDataValidationContext CreateValidationContext()
    {
        return FDataValidationContext(true, EDataValidationUsecase::Manual, TConstArrayView<FAssetData>{});
    }

    inline bool ValidationContextContainsIssue(const FDataValidationContext& Context, const FString& ExpectedFragment)
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

    inline bool TestValidation(FAutomationTestBase& Test,
                               const UObject* Object,
                               const EDataValidationResult ExpectedResult,
                               const TCHAR* ExpectedIssueFragment = nullptr)
    {
        auto Context = CreateValidationContext();
        const auto ActualResult = Object->IsDataValid(Context);
        const auto bResultMatches =
            Test.TestEqual(TEXT("Validation result should match expectation"), ActualResult, ExpectedResult);
        if (ExpectedIssueFragment)
        {
            return Test.TestTrue(FString::Printf(TEXT("Validation issues should contain '%s'"), ExpectedIssueFragment),
                                 ValidationContextContainsIssue(Context, ExpectedIssueFragment))
                && bResultMatches;
        }
        else
        {
            return bResultMatches;
        }
    }
} // namespace AeonTests

#endif
