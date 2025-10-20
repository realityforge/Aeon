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
#include "AeonEditorMessageLog.h"
#include "Aeon/Logging.h"
#include "Logging/StructuredLog.h"
#include "MessageLogModule.h"

const FName FAeonEditorMessageLog::MessageLogName{ TEXT("AeonEditor") };
static const FName MessageLogModuleName = FName(TEXT("MessageLog"));

void FAeonEditorMessageLog::Initialize()
{
    // create a MessageLog category to use in plugin
    auto& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(MessageLogModuleName);
    FMessageLogInitializationOptions InitOptions;
    InitOptions.bShowPages = true;
    InitOptions.bAllowClear = true;
    InitOptions.bShowFilters = true;
    InitOptions.MaxPageCount = 4;
    MessageLogModule.RegisterLogListing(GetMessageLogName(),
                                        NSLOCTEXT("AeonEditor", "AeonEditorLogLabel", "Aeon Editor"),
                                        InitOptions);
    UE_LOGFMT(LogAeon, VeryVerbose, "FAeonEditorMessageLog::Shutdown(): Registered MessageLog.");
}

void FAeonEditorMessageLog::Shutdown()
{
    // Deregister MessageLog created in Startup
    if (FModuleManager::Get().IsModuleLoaded(MessageLogModuleName))
    {
        FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>(MessageLogModuleName);
        if (MessageLogModule.IsRegisteredLogListing(GetMessageLogName()))
        {
            UE_LOGFMT(LogAeon, VeryVerbose, "FAeonEditorMessageLog::Shutdown(): Deregistering MessageLog.");
            MessageLogModule.UnregisterLogListing(GetMessageLogName());
        }
        else
        {
            UE_LOGFMT(LogAeon,
                      Verbose,
                      "FAeonEditorMessageLog::Shutdown(): Skipping deregister of MessageLog as not registered.");
        }
    }
}

FMessageLog FAeonEditorMessageLog::GetMessageLog()
{
    return FMessageLog(GetMessageLogName());
}

void FAeonEditorMessageLog::Open()
{
    GetMessageLog().Open();
}
