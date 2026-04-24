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
    #include "Engine/Blueprint.h"
    #include "Engine/Engine.h"
    #include "Engine/World.h"
    #include "GameFramework/Actor.h"
    #include "HAL/FileManager.h"
    #include "Kismet2/KismetEditorUtilities.h"
    #include "Misc/AutomationTest.h"
    #include "Misc/DataValidation.h"
    #include "Misc/Guid.h"
    #include "Misc/Paths.h"
    #include "Subsystems/EditorAssetSubsystem.h"
    #include "UObject/Package.h"
    #include "UObject/UnrealType.h"

namespace AeonAnimationTests
{
    constexpr auto AutomationTestFlags =
        EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

    class FTestWorld
    {
    public:
        explicit FTestWorld(const TCHAR* WorldName = TEXT("AeonAnimationAutomationTestWorld"))
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

    template <typename TValue, typename TObject>
    TValue GetPropertyValue(FAutomationTestBase& Test, TObject* Object, const TCHAR* PropertyName)
    {
        const auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
        if (Test.TestNotNull(FString::Printf(TEXT("Property %s should exist"), PropertyName), Property))
        {
            if (const auto ValuePtr = Property->template ContainerPtrToValuePtr<TValue>(Object))
            {
                return *ValuePtr;
            }

            Test.AddError(FString::Printf(TEXT("Property %s should be readable"), PropertyName));
        }

        return TValue();
    }

    inline const TCHAR* GetAeonAnimationTestMountRoot()
    {
        return TEXT("/Game/Developers/Tests/AeonAnimation");
    }

    inline FString GetAeonAnimationTestContentRoot()
    {
        return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Developers/Tests/AeonAnimation"));
    }

    inline bool IsAeonAnimationTestPackagePath(const FString& PackagePath)
    {
        return PackagePath.StartsWith(GetAeonAnimationTestMountRoot(), ESearchCase::CaseSensitive);
    }

    inline void CleanupAeonAnimationTestContentRoot()
    {
        if (nullptr != GEditor)
        {
            if (auto const Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
            {
                Subsystem->DeleteDirectory(GetAeonAnimationTestMountRoot());
            }
        }

        IFileManager::Get().DeleteDirectory(*GetAeonAnimationTestContentRoot(), false, true);
    }

    class FAeonAnimationTestAssetCleanupRegistrar
    {
    public:
        FAeonAnimationTestAssetCleanupRegistrar()
        {
            auto& Framework = FAutomationTestFramework::Get();
            OnTestStartHandle =
                Framework.OnTestStartEvent.AddStatic(&FAeonAnimationTestAssetCleanupRegistrar::HandleTestStart);
            OnTestEndHandle =
                Framework.OnTestEndEvent.AddStatic(&FAeonAnimationTestAssetCleanupRegistrar::HandleTestEnd);
        }

        ~FAeonAnimationTestAssetCleanupRegistrar()
        {
            auto& Framework = FAutomationTestFramework::Get();
            Framework.OnTestStartEvent.Remove(OnTestStartHandle);
            Framework.OnTestEndEvent.Remove(OnTestEndHandle);
            CleanupAeonAnimationTestContentRoot();
        }

    private:
        static void HandleTestStart(FAutomationTestBase*) { CleanupAeonAnimationTestContentRoot(); }

        static void HandleTestEnd(FAutomationTestBase*) { CleanupAeonAnimationTestContentRoot(); }

        FDelegateHandle OnTestStartHandle;
        FDelegateHandle OnTestEndHandle;
    };

    inline void EnsureTestAssetCleanupRegistered()
    {
        static FAeonAnimationTestAssetCleanupRegistrar Registrar;
    }

    inline FString NewUniqueTestPackageName(const TCHAR* const Prefix)
    {
        const FString BasePrefix = Prefix ? Prefix : TEXT("AeonAnimationTestObject");
        return FString::Printf(TEXT("%s/%s_%s"),
                               GetAeonAnimationTestMountRoot(),
                               *BasePrefix,
                               *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    }

    inline void EnsurePackageDirectoryExists(const FString& PackageName)
    {
        if (IsAeonAnimationTestPackagePath(PackageName))
        {
            EnsureTestAssetCleanupRegistered();
            constexpr TCHAR GameRoot[] = TEXT("/Game/");
            const auto RelativePath = PackageName.RightChop(UE_ARRAY_COUNT(GameRoot) - 1);
            const auto DirectoryPath = FPaths::GetPath(FPaths::Combine(FPaths::ProjectContentDir(), RelativePath));
            if (!DirectoryPath.IsEmpty())
            {
                IFileManager::Get().MakeDirectory(*DirectoryPath, true);
            }
        }
    }

    inline UPackage* NewTransientPackage(const FString& PackageName)
    {
        EnsurePackageDirectoryExists(PackageName);
        const auto Package = CreatePackage(*PackageName);
        if (Package)
        {
            Package->SetFlags(RF_Transient);
        }

        return Package;
    }

    inline UBlueprint* NewBlueprint(UClass* const ParentClass,
                                    const FString& PackageName,
                                    const TCHAR* const ObjectName,
                                    const EBlueprintType BlueprintType = BPTYPE_Normal)
    {
        const auto Package = NewTransientPackage(PackageName);
        return Package ? FKismetEditorUtilities::CreateBlueprint(ParentClass,
                                                                 Package,
                                                                 FName(ObjectName),
                                                                 BlueprintType,
                                                                 UBlueprint::StaticClass(),
                                                                 UBlueprintGeneratedClass::StaticClass(),
                                                                 NAME_None)
                       : nullptr;
    }

    inline void CompileBlueprint(UBlueprint* const Blueprint)
    {
        if (Blueprint)
        {
            FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
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

        return bResultMatches;
    }
} // namespace AeonAnimationTests

#endif
