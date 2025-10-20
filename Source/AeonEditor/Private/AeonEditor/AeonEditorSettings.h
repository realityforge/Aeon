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

#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "AeonEditorSettings.generated.h"

/**
 * Row describing a mapping category to validate, a description for UI, and default target categories.
 * The row name is treated as the category key (e.g., InputTagCategory, AbilityTagCategory, etc.).
 */
USTRUCT(BlueprintType)
struct FAeonTagCategoryRow : public FTableRowBase
{
    GENERATED_BODY()

    /** A friendly description for the category. */
    UPROPERTY(EditAnywhere, Category = "Category")
    FText Description;

    /** Default target category names used to initialize GameplayTags Category Remapping if missing. */
    UPROPERTY(EditAnywhere, Category = "Category")
    TArray<FString> DefaultTargets;
};

/**
 * Project settings that define which DataTables provide Category Remapping validation/migration rules.
 */
UCLASS(Config = Editor, DefaultConfig, DisplayName = "Aeon Settings")
class UAeonEditorSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** DataTables that contain FAeonTagCategoryRow rows. If none are specified, a plugin default is used. */
    UPROPERTY(Config, EditAnywhere, Category = "Tag Categories")
    TArray<TSoftObjectPtr<UDataTable>> TagCategoryTables;
};
