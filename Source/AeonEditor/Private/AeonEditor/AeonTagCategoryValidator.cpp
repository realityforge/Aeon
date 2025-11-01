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

#include "AeonTagCategoryValidator.h"
#include "AeonEditorLogging.h"
#include "AeonEditorMessageLog.h"
#include "AeonEditorSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameplayTagsSettings.h"
#include "Widgets/Notifications/SNotificationList.h"

const FGameplayTagCategoryRemap* FAeonTagCategoryValidator::FindRemap(const UGameplayTagsSettings* TagSettings,
                                                                      const FName CategoryName)
{
    for (auto& Remap : TagSettings->CategoryRemapping)
    {
        if (Remap.BaseCategory == CategoryName)
        {
            return &Remap;
        }
    }
    return nullptr;
}

void FAeonTagCategoryValidator::AddDefaultRemaps()
{
    TMap<FString, TArray<FString>> RemapsCreated;
    PerformScan(
        [&RemapsCreated](auto Settings, auto Name, auto Category) {
            FGameplayTagCategoryRemap Remap;
            Remap.BaseCategory = Name.ToString();
            Remap.RemapCategories.Append(Category->DefaultTargets);
            Settings->CategoryRemapping.Add(MoveTemp(Remap));
            RemapsCreated.Emplace(Name.ToString(), Category->DefaultTargets);
            // Persist the GameplayTags settings update to the project's default config file
            Settings->SaveConfig();
            Settings->TryUpdateDefaultConfigFile();
        },
        [](auto, auto, auto) {});

    if (!RemapsCreated.IsEmpty())
    {
        auto MessageLog = FAeonEditorMessageLog::GetMessageLog();
        // Message log info listing each created mapping and its targets
        MessageLog.Info()->AddToken(
            FTextToken::Create(FText::FromString(TEXT("Created default category remaps for all missing categories:"))));
        for (const auto& Missing : RemapsCreated)
        {
            MessageLog.Info()->AddToken(FTextToken::Create(FText::FromString(
                FString::Printf(TEXT("   %s -> [%s]"), *Missing.Key, *FString::Join(Missing.Value, TEXT(", "))))));
        }

        // Toast notification
        FNotificationInfo Info(
            NSLOCTEXT("Aeon", "FixAllTagRemapsDone", "Default values for tag category remaps created."));
        Info.ExpireDuration = 5.0f;
        Info.bFireAndForget = true;
        Info.bUseLargeFont = false;
        FSlateNotificationManager::Get().AddNotification(Info);
    }
}

void FAeonTagCategoryValidator::ScanGameplayTagCategoryRemapsForMissingCategories()
{
    bool bMissing{ false };
    PerformScan(
        [&bMissing](auto, auto Name, auto Category) {
            bMissing = true;
            FAeonEditorMessageLog::GetMessageLog().Error()->AddToken(
                FTextToken::Create(FText::FromString(FString::Printf(
                    TEXT("Missing remap for category '%s'. Category has description '%s' and defaults to [%s]"),
                    *FString(Name.ToString()),
                    *Category->Description.ToString(),
                    *FString::Join(Category->DefaultTargets, TEXT(", "))))));
        },
        [](auto, auto Name, auto Category) {
            FAeonEditorMessageLog::GetMessageLog().Error()->AddToken(FTextToken::Create(FText::FromString(FString::Printf(
                TEXT("Remap for category '%s' exists but is empty. Category has description '%s' and defaults to [%s]"),
                *FString(Name.ToString()),
                *Category->Description.ToString(),
                *FString::Join(Category->DefaultTargets, TEXT(", "))))));
        });

    // TODO: We should check that the remap entries are sorted alphabetically and allow the user to sort them via an
    // action if necessary

    if (bMissing)
    {
        const auto MessageLog = FAeonEditorMessageLog::GetMessageLog().Error();
        MessageLog->AddToken(
            FTextToken::Create(FText::FromString(TEXT("One or more Tag Category remaps are missing."))));
        MessageLog->AddToken(FActionToken::Create(
            FText::FromString(TEXT("Add Default Remaps For Missing Categories")),
            FText::FromString(TEXT("Create remap entries for all missing categories using their defaults.")),
            FSimpleDelegate::CreateLambda([] { AddDefaultRemaps(); }),
            true));
        FAeonEditorMessageLog::Open();
    }
}

void FAeonTagCategoryValidator::PerformScan(const FAeonMissingTagCategoryFn& MissingRemapFunction,
                                            const FAeonEmptyTagCategoryFn& EmptyRemapFunction)
{
    if (const auto Settings = GetMutableDefault<UAeonEditorSettings>())
    {
        if (const auto TagSettings = GetMutableDefault<UGameplayTagsSettings>())
        {
            if (0 == Settings->TagCategoryTables.Num())
            {
                // Default to a content asset provided by the plugin
                constexpr static auto Path =
                    TEXT("/Aeon/Aeon/Editor/TagCategories/DT_AeonTagCategoryMappings.DT_AeonTagCategoryMappings");
                Settings->TagCategoryTables.Add(TSoftObjectPtr<UDataTable>(FSoftObjectPath(Path)));
                // Persist the editor settings update to the project's default editor config file
                Settings->SaveConfig();
                Settings->TryUpdateDefaultConfigFile();
            }

            TSet<FName> Processed;

            auto Index{ 0 };
            for (const auto& TableReference : Settings->TagCategoryTables)
            {
                if (const auto Table = TableReference.LoadSynchronous())
                {
                    for (const auto& Pair : Table->GetRowMap())
                    {
                        const auto Name = Pair.Key;
                        if (!Processed.Contains(Name))
                        {
                            Processed.Add(Name);

                            if (const auto Row = reinterpret_cast<const FAeonTagCategoryRow*>(Pair.Value))
                            {
                                const auto Existing = FindRemap(TagSettings, Name);
                                if (!Existing)
                                {
                                    MissingRemapFunction(TagSettings, Name, Row);
                                }
                                else if (0 == Existing->RemapCategories.Num())
                                {
                                    EmptyRemapFunction(TagSettings, Name, Row);
                                }
                                else
                                {
                                    UE_LOGFMT(LogAeonEditor,
                                              Verbose,
                                              "Found existing remap for category '{Category}'",
                                              Name.ToString());
                                }
                            }
                        }
                    }
                }
                else
                {
                    UE_LOGFMT(LogAeonEditor,
                              Warning,
                              "GameplayTagsSettings: TagCategoryTables[{Index}] is null or not resolvable.",
                              Index);
                }
                Index++;
            }
        }
        else
        {
            UE_LOG(LogAeonEditor,
                   Warning,
                   TEXT("Failed to create default category remaps. Unable to retrieve GameplayTagsSettings."));
        }
    }
    else
    {
        UE_LOG(LogAeonEditor,
               Warning,
               TEXT("Failed to create default category remaps. Unable to retrieve AeonTagCategorySettings."));
    }
}
