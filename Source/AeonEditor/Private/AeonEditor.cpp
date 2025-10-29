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
#include "AeonEditor.h"
#include "AeonEditor/AeonEditorMessageLog.h"
#include "AeonEditor/AeonEditorSettings.h"
#include "AeonEditor/AeonTagCategoryValidator.h"
#include "Engine/DataTable.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "FAeonEditorModule"

void FAeonEditorModule::StartupModule()
{
    FAeonEditorMessageLog::Initialize();
    FAeonTagCategoryValidator::ScanGameplayTagCategoryRemapsForMissingCategories();

    // Sort FAeonTagCategoryRow.DefaultTargets prior to saving any matching DataTable
    PreSaveHandle = FCoreUObjectDelegates::OnObjectPreSave.AddRaw(this, &FAeonEditorModule::HandlePreObjectSave);
}

void FAeonEditorModule::ShutdownModule()
{
    FAeonEditorMessageLog::Shutdown();

    if (PreSaveHandle.IsValid())
    {
        FCoreUObjectDelegates::OnObjectPreSave.Remove(PreSaveHandle);
        PreSaveHandle.Reset();
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FAeonEditorModule::HandlePreObjectSave(UObject* Object, FObjectPreSaveContext /*SaveContext*/)
{
    // Only operate on DataTables with FAeonTagCategoryRow rows
    if (const auto Table = Cast<UDataTable>(Object))
    {
        if (FAeonTagCategoryRow::StaticStruct() == Table->GetRowStruct())
        {
            for (const auto& Pair : Table->GetRowMap())
            {
                if (const auto Row = reinterpret_cast<FAeonTagCategoryRow*>(Pair.Value))
                {
                    Row->DefaultTargets.Sort(
                        [](auto& A, auto& B) { return A.Compare(B, ESearchCase::CaseSensitive) < 0; });
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAeonEditorModule, AeonEditor)
