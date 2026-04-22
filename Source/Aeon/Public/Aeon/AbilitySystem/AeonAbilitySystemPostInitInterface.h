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

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AeonAbilitySystemPostInitInterface.generated.h"

#define UE_API AEON_API

UINTERFACE(MinimalAPI)
class UAeonAbilitySystemPostInitInterface : public UInterface
{
    GENERATED_BODY()
};

class UE_API IAeonAbilitySystemPostInitInterface
{
    GENERATED_BODY()

public:
    /**
     * Called exactly once per UAttributeSet instance per ASC-registration lifetime,
     * by UAeonAbilitySystemComponent, after Aeon has determined that the owning
     * Ability System is ready and the current Aeon mutation/grant batch has fully committed.
     *
     * Guarantees:
     * - GetOwningAbilitySystemComponent() is valid and is a UAeonAbilitySystemComponent.
     * - Owner/avatar access via AbilityActorInfo is valid for the duration of this call.
     * - All grant-time setup for the current batch is already visible, including initial
     *   attribute writes and immediate GameplayEffect consequences.
     * - All relevant AttributeSets from the current batch are already registered.
     * - No ordering is guaranteed relative to other AttributeSets' post-init hooks.
     *
     * Intended use:
     * - One-shot bootstrap/snapshot/wiring for this registration lifetime.
     * - Initial ASC->external or external->ASC synchronization, as documented by the set.
     * - Delegate binding and other non-structural wiring.
     *
     * Restrictions:
     * - Do not perform structural ASC mutation here (no grant/remove abilities/effects/sets).
     * - Do not treat this as ongoing synchronization.
     * - No automatic retry occurs if this implementation cannot complete.
     *
     * Implementations should be defensively idempotent where practical.
     */
    virtual void OnAbilitySystemPostInit() = 0;
};

#undef UE_API
