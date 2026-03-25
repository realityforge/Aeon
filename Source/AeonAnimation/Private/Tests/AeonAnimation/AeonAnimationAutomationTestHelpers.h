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

    #include "Engine/Engine.h"
    #include "Engine/World.h"
    #include "GameFramework/Actor.h"
    #include "Misc/AutomationTest.h"
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
} // namespace AeonAnimationTests

#endif
