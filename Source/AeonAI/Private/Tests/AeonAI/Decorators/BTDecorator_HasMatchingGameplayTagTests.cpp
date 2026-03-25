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

    #include "BehaviorTree/BlackboardComponent.h"
    #include "Misc/AutomationTest.h"
    #include "NativeGameplayTags.h"
    #include "Tests/AeonAI/AeonAIAutomationTestTypes.h"

namespace BTDecoratorHasMatchingGameplayTagTests
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC(TestConditionTag, "Aeon.Test.AI.Decorator.Matching");

    constexpr TCHAR TargetActorKeyName[] = TEXT("TargetActor");

    UBlackboardData* CreateBlackboardData()
    {
        const auto BlackboardData = AeonAITests::CreateBlackboardData();
        AeonAITests::AddObjectBlackboardKey(BlackboardData, TargetActorKeyName, AActor::StaticClass());
        return BlackboardData;
    }
} // namespace BTDecoratorHasMatchingGameplayTagTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTDecoratorHasMatchingGameplayTagReturnsTrueForMatchingTagTest,
                                 "Aeon.AI.Decorators.HasMatchingGameplayTag.ReturnsTrueForMatchingTag",
                                 AeonAITests::AutomationTestFlags)
bool FBTDecoratorHasMatchingGameplayTagReturnsTrueForMatchingTagTest::RunTest(const FString&)
{
    const FGameplayTag ConditionTag = BTDecoratorHasMatchingGameplayTagTests::TestConditionTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    if (AeonAITests::CreateAIContext(*this,
                                     World,
                                     Controller,
                                     Pawn,
                                     BehaviorTreeComponent,
                                     BTDecoratorHasMatchingGameplayTagTests::CreateBlackboardData(),
                                     TEXT("AeonAIDecoratorHasMatchingGameplayTagTestWorld")))
    {
        Pawn->GetAeonAbilitySystemComponent()->SetLooseGameplayTagCount(ConditionTag, 1);
        Controller->GetBlackboardComponent()->SetValueAsObject(
            BTDecoratorHasMatchingGameplayTagTests::TargetActorKeyName,
            Pawn);

        const auto Decorator = AeonAITests::NewTransientObject<UBTDecorator_HasMatchingGameplayTagTestNode>();
        Decorator->SetBlackboardKeyForTest(BTDecoratorHasMatchingGameplayTagTests::TargetActorKeyName);
        Decorator->SetGameplayTagForTest(ConditionTag);

        return TestTrue(TEXT("Decorator should match when the target actor ASC has the requested gameplay tag"),
                        Decorator->EvaluateConditionForTest(*BehaviorTreeComponent));
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTDecoratorHasMatchingGameplayTagReturnsFalseForMissingTargetOrTagTest,
                                 "Aeon.AI.Decorators.HasMatchingGameplayTag.ReturnsFalseForMissingTargetOrTag",
                                 AeonAITests::AutomationTestFlags)
bool FBTDecoratorHasMatchingGameplayTagReturnsFalseForMissingTargetOrTagTest::RunTest(const FString&)
{
    const FGameplayTag ConditionTag = BTDecoratorHasMatchingGameplayTagTests::TestConditionTag.GetTag();
    TUniquePtr<AeonAITests::FTestWorld> World;
    AAeonAITestAIController* Controller{ nullptr };
    AAeonAITestPawn* Pawn{ nullptr };
    UBehaviorTreeComponent* BehaviorTreeComponent{ nullptr };
    if (AeonAITests::CreateAIContext(*this,
                                     World,
                                     Controller,
                                     Pawn,
                                     BehaviorTreeComponent,
                                     BTDecoratorHasMatchingGameplayTagTests::CreateBlackboardData(),
                                     TEXT("AeonAIDecoratorHasMatchingGameplayTagMissingTargetTestWorld")))
    {
        const auto Decorator = AeonAITests::NewTransientObject<UBTDecorator_HasMatchingGameplayTagTestNode>();
        Decorator->SetBlackboardKeyForTest(BTDecoratorHasMatchingGameplayTagTests::TargetActorKeyName);
        Decorator->SetGameplayTagForTest(ConditionTag);

        const bool bMissingTargetMatches = Decorator->EvaluateConditionForTest(*BehaviorTreeComponent);
        Controller->GetBlackboardComponent()->SetValueAsObject(
            BTDecoratorHasMatchingGameplayTagTests::TargetActorKeyName,
            World->SpawnActor<AActor>());
        const bool bTargetWithoutASCMatches = Decorator->EvaluateConditionForTest(*BehaviorTreeComponent);

        return TestFalse(TEXT("Decorator should fail when the target actor key is unset"), bMissingTargetMatches)
            && TestFalse(TEXT("Decorator should fail when the target actor does not expose an ASC"),
                         bTargetWithoutASCMatches);
    }
    else
    {
        return false;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBTDecoratorHasMatchingGameplayTagDescriptionIncludesConfiguredKeyAndTagTest,
                                 "Aeon.AI.Decorators.HasMatchingGameplayTag.DescriptionIncludesConfiguredKeyAndTag",
                                 AeonAITests::AutomationTestFlags)
bool FBTDecoratorHasMatchingGameplayTagDescriptionIncludesConfiguredKeyAndTagTest::RunTest(const FString&)
{
    const FGameplayTag ConditionTag = BTDecoratorHasMatchingGameplayTagTests::TestConditionTag.GetTag();
    const auto Decorator = AeonAITests::NewTransientObject<UBTDecorator_HasMatchingGameplayTagTestNode>();
    if (TestNotNull(TEXT("Decorator should be created"), Decorator))
    {
        Decorator->SetBlackboardKeyForTest(BTDecoratorHasMatchingGameplayTagTests::TargetActorKeyName);
        Decorator->SetGameplayTagForTest(ConditionTag);

    #if WITH_EDITOR
        const auto Description = Decorator->GetDescriptionForTest();
        return TestTrue(TEXT("Decorator description should include the configured blackboard key"),
                        Description.Contains(BTDecoratorHasMatchingGameplayTagTests::TargetActorKeyName))
            && TestTrue(TEXT("Decorator description should include the configured gameplay tag"),
                        Description.Contains(ConditionTag.ToString()));
    #else
        return true;
    #endif
    }
    else
    {
        return false;
    }
}

#endif
