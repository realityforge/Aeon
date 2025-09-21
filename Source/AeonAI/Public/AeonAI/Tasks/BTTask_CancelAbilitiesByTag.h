#pragma once

#include "BTTask_ActivateAbilityByTag.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BTTask_CancelAbilitiesByTag.generated.h"

class UAbilitySystemComponent;

/**
 * Behavior tree task that cancels all currently active gameplay abilities
 * on the controlled pawn matching a tag, either from a property or a blackboard key.
 */
UCLASS()
class AEONAI_API UBTTask_CancelAbilitiesByTag : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_CancelAbilitiesByTag();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& BehaviorTreeComponent, uint8* NodeMemory) override;
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual FString GetStaticDescription() const override;

protected:
    FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& BehaviorTreeComponent);

    /** Determines whether to use the Blackboard or Property gameplay tag */
    UPROPERTY(EditAnywhere, Category = "Ability")
    EAeonAbilityTagSource AbilityTagSource{ EAeonAbilityTagSource::FromProperty };

    /** The gameplay tag used if AbilityTagSource is FromProperty */
    UPROPERTY(EditAnywhere,
              Category = "Ability",
              meta = (EditCondition = "AbilityTagSource == EAeonAbilityTagSource::FromProperty", EditConditionHides))
    FGameplayTag AbilityTag{ FGameplayTag::EmptyTag };

    /** The blackboard key that should return a gameplay tag */
    UPROPERTY(EditAnywhere,
              Category = "Blackboard",
              meta = (EditCondition = "AbilityTagSource == EAeonAbilityTagSource::FromBlackboard", EditConditionHides))
    FBlackboardKeySelector AbilityTagKey;

    /** If true, task fails when no abilities were canceled */
    UPROPERTY(EditAnywhere, Category = "Ability")
    bool bFailIfNoAbilitiesCancelled{ false };
};
