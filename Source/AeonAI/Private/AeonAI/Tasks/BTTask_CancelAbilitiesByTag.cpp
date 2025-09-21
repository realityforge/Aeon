#include "AeonAI/Tasks/BTTask_CancelAbilitiesByTag.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Aeon/Logging.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BTTask_CancelAbilitiesByTag)

UBTTask_CancelAbilitiesByTag::UBTTask_CancelAbilitiesByTag()
{
    NodeName = TEXT("Cancel Abilities By Tag");
}

FString UBTTask_CancelAbilitiesByTag::GetStaticDescription() const
{
    const auto FailFlag = bFailIfNoAbilitiesCancelled ? TEXT(", FailIfNone") : TEXT("");
    if (EAeonAbilityTagSource::FromProperty == AbilityTagSource)
    {
        return FString::Printf(TEXT("Cancel Abilities: [%s]%s"), *AbilityTag.ToString(), FailFlag);
    }
    else
    {
        return FString::Printf(TEXT("Cancel Abilities: BB Key [%s]%s"),
                               *AbilityTagKey.SelectedKeyName.ToString(),
                               FailFlag);
    }
}

void UBTTask_CancelAbilitiesByTag::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    if (EAeonAbilityTagSource::FromBlackboard == AbilityTagSource)
    {
        AbilityTagKey.ResolveSelectedKey(*Asset.GetBlackboardAsset());
    }
}

FGameplayTag UBTTask_CancelAbilitiesByTag::ResolveAbilityTag(UBehaviorTreeComponent& BehaviorTreeComponent)
{
    if (EAeonAbilityTagSource::FromProperty == AbilityTagSource)
    {
        return AbilityTag;
    }
    else if (const auto Blackboard = BehaviorTreeComponent.GetBlackboardComponent())
    {
        const FName TagName = Blackboard->GetValueAsName(AbilityTagKey.SelectedKeyName);
        if (TagName.IsNone())
        {
            UE_LOGFMT(LogAeon,
                      Warning,
                      "CancelAbilitiesByTag task failed: Blackboard key '{Key}' has no tag",
                      AbilityTagKey.SelectedKeyName);
            return FGameplayTag::EmptyTag;
        }
        else
        {
            return FGameplayTag::RequestGameplayTag(TagName);
        }
    }
    else
    {
        UE_LOGFMT(LogAeon, Error, "CancelAbilitiesByTag task failed: Blackboard component missing");
        return FGameplayTag::EmptyTag;
    }
}

EBTNodeResult::Type UBTTask_CancelAbilitiesByTag::ExecuteTask(UBehaviorTreeComponent& BehaviorTreeComponent,
                                                              uint8* NodeMemory)
{
    if (const auto AIController = BehaviorTreeComponent.GetAIOwner())
    {
        if (const auto Pawn = AIController->GetPawn())
        {
            if (const auto AbilitySystemComponent = Pawn->FindComponentByClass<UAbilitySystemComponent>())
            {
                const auto Tag = ResolveAbilityTag(BehaviorTreeComponent);
                if (Tag.IsValid())
                {
                    // Before cancel, record active abilities count
                    TArray<FGameplayAbilitySpec*> MatchingSpecs;
                    AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(
                        Tag.GetSingleTagContainer(),
                        MatchingSpecs);

                    auto CancelCount = 0;
                    for (const auto Spec : MatchingSpecs)
                    {
                        if (Spec && Spec->IsActive())
                        {
                            AbilitySystemComponent->CancelAbilityHandle(Spec->Handle);
                            CancelCount++;
                        }
                    }

                    if (CancelCount > 0)
                    {
                        UE_LOGFMT(LogAeon,
                                  VeryVerbose,
                                  "CancelAbilitiesByTag task succeeded: Cancelled {Count} active abilities "
                                  "with tag '{Tag}' on '{Pawn}'",
                                  CancelCount,
                                  Tag.GetTagName(),
                                  Pawn->GetActorNameOrLabel());
                        return EBTNodeResult::Succeeded;
                    }
                    else
                    {
                        UE_LOGFMT(LogAeon,
                                  Verbose,
                                  "CancelAbilitiesByTag task: No active abilities matched tag '{Tag}' on '{Pawn}'",
                                  Tag.GetTagName(),
                                  Pawn->GetActorNameOrLabel());
                        return bFailIfNoAbilitiesCancelled ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
                    }
                }
                else
                {
                    UE_LOGFMT(LogAeon,
                              Warning,
                              "CancelAbilitiesByTag task failed: Resolved tag is invalid for Pawn '{Pawn}'",
                              Pawn->GetActorNameOrLabel());
                    return EBTNodeResult::Failed;
                }
            }
            else
            {
                UE_LOGFMT(LogAeon,
                          Warning,
                          "CancelAbilitiesByTag task failed: no AbilitySystemComponent found on Pawn '{Pawn}'",
                          Pawn->GetActorNameOrLabel());
                return EBTNodeResult::Failed;
            }
        }
        else
        {
            UE_LOGFMT(LogAeon, Error, "CancelAbilitiesByTag task failed: AIController has no controlled Pawn");
            return EBTNodeResult::Failed;
        }
    }
    else
    {
        UE_LOGFMT(LogAeon, Error, "CancelAbilitiesByTag task failed: missing AIController");
        return EBTNodeResult::Failed;
    }
}
