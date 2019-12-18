/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- sgrStack.hpp

Abstract:
- Encapsulates logic for the XTPUSHSGR / XTPOPSGR VT control sequences, which save and
  restore text attributes on a stack.

--*/

#pragma once

#include "..\..\buffer\out\TextAttribute.hpp"
#include "..\..\terminal\adapter\DispatchTypes.hpp"
#include <bitset>

namespace Microsoft::Console::VirtualTerminal
{
    class SgrStack
    {
    public:
        SgrStack() noexcept;

        // Method Description:
        // - Saves the specified text attributes onto an internal stack.
        // Arguments:
        // - currentAttributes - The attributes to save onto the stack.
        // - options - If none supplied, the full attributes are saved. Else only the
        //   specified parts of currentAttributes are saved.
        // Return Value:
        // - <none>
        void Push(const TextAttribute& currentAttributes,
                  const gsl::span<const DispatchTypes::SgrSaveRestoreStackOptions> options) noexcept;

        // Method Description:
        // - Restores text attributes by removing from the top of the internal stack,
        //   combining them with the supplied currentAttributes, if appropriate.
        // Arguments:
        // - currentAttributes - The current text attributes. If only a portion of
        //   attributes were saved on the internal stack, then those attributes will be
        //   combined with the currentAttributes passed in to form the return value.
        // Return Value:
        // - The TextAttribute that has been removed from the top of the stack, possibly
        //   combined with currentAttributes.
        const TextAttribute Pop(const TextAttribute& currentAttributes) noexcept;

        // Xterm allows the save stack to go ten deep, so we'll follow suit. Pushes after
        // ten deep will still remain "balanced"--once you pop back down below ten, you'll
        // restore the appropriate text attributes. However, if you get more than a
        // hundred pushes deep, we'll stop counting. Why unbalance somebody doing so many
        // pushes? Putting a bound on it allows us to provide "reset" functionality: at
        // any given point, you can execute 101 pops and know that you've taken the stack
        // (push count) to zero. (Then you reset text attributes, and your state is
        // clean.)
        static constexpr int c_MaxStoredSgrPushes = 10;
        static constexpr int c_MaxBalancedPushes = 100;

    private:
        // Note the +1 in the size of the bitset: this is because we use the
        // SgrSaveRestoreStackOptions enumerands as bitset flags, so they are naturally
        // one-based (and we don't offset them, so the lowest bit in the bitset is
        // actually not used).
        typedef std::bitset<static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Max) + 1> AttrBitset;

        TextAttribute _CombineWithCurrentAttributes(const TextAttribute& currentAttributes,
                                                    const TextAttribute& savedAttribute,
                                                    const AttrBitset validParts) noexcept; // valid parts of savedAttribute

        int _numSgrPushes; // used as an index into the following arrays
        std::array<TextAttribute, c_MaxStoredSgrPushes> _storedSgrAttributes;
        std::array<AttrBitset, c_MaxStoredSgrPushes> _validAttributes; // flags that indicate which portions of the attributes are valid
    };
}
