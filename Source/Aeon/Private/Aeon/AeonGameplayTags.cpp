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
#include "Aeon/AeonGameplayTags.h"

namespace AeonGameplayTags
{
    // --------------------------------------------------- //
    // Input Tags
    // --------------------------------------------------- //

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_CancelOnRelease,
                                   "Input.Ability.CancelOnRelease",
                                   "If present in an abilities InputTags then cancel "
                                   "ability once the input is released")
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Ability_Toggle,
                                   "Input.Ability.Toggle",
                                   "If present in an abilities InputTags then pressing "
                                   "the input will toggle activation state")

} // namespace AeonGameplayTags
