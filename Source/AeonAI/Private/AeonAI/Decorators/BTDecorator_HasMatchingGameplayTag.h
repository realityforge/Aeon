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

#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BTDecorator_HasMatchingGameplayTag.generated.h"

UCLASS()
class UBTDecorator_HasMatchingGameplayTag : public UBTDecorator_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTDecorator_HasMatchingGameplayTag();

    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
    /** Gameplay Tag to check on the target Actor */
    UPROPERTY(EditAnywhere, Category = "Condition")
    FGameplayTag GameplayTag{ FGameplayTag::EmptyTag };

    /** Core condition check */
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

#if WITH_EDITOR
    virtual FName GetNodeIconName() const override;
    virtual FString GetStaticDescription() const override;
#endif
};
