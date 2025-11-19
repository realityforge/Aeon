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
#include "AeonAI/Decorators/BTDecorator_HasMatchingGameplayTag.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aeon/Logging.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BTDecorator_HasMatchingGameplayTag)

UBTDecorator_HasMatchingGameplayTag::UBTDecorator_HasMatchingGameplayTag()
{
    NodeName = "Has Matching Gameplay Tag";

    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, BlackboardKey), AActor::StaticClass());
    INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

void UBTDecorator_HasMatchingGameplayTag::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    // Make sure the key selector is resolved against the blackboard asset
    if (const auto BBAsset = GetBlackboardAsset())
    {
        BlackboardKey.ResolveSelectedKey(*BBAsset);
    }
}

bool UBTDecorator_HasMatchingGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
                                                                     uint8* NodeMemory) const
{
    if (GameplayTag.IsValid())
    {
        if (const auto BlackboardComponent = OwnerComp.GetBlackboardComponent())
        {
            if (const auto Target = Cast<AActor>(BlackboardComponent->GetValueAsObject(BlackboardKey.SelectedKeyName)))
            {
                if (const auto AbilitySystemComponent =
                        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
                {
                    return AbilitySystemComponent->HasMatchingGameplayTag(GameplayTag);
                }
            }
        }
    }
    else
    {
        UE_LOGFMT(LogAeon,
                  Warning,
                  "{Self}: GameplayTag has not been set so will always fail to match.",
                  *GetNameSafe(this));
    }

    return false;
}

#if WITH_EDITOR
FName UBTDecorator_HasMatchingGameplayTag::GetNodeIconName() const
{
    return FName("BTEditor.Graph.BTNode.Decorator.CompareBBEntries.Icon");
}

FString UBTDecorator_HasMatchingGameplayTag::GetStaticDescription() const
{
    const auto KeyDesc =
        BlackboardKey.SelectedKeyName.IsNone() ? TEXT("None") : BlackboardKey.SelectedKeyName.ToString();
    const auto TagDesc = GameplayTag.IsValid() ? GameplayTag.ToString() : TEXT("None");

    return FString::Printf(TEXT("Actor Key: %s\nTag: %s"), *KeyDesc, *TagDesc);
}
#endif
