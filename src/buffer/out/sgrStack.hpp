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

#include "TextAttribute.hpp"
#include "..\..\terminal\adapter\DispatchTypes.hpp"

namespace Microsoft::Console::VirtualTerminal
{
    class SgrStack
    {
    public:
        SgrStack() noexcept :
            _numSgrPushes{ 0 },
            _validAttributes{ 0 },
            _storedSgrAttributes{ 0 }
        {
        }

        // Method Description:
        // - Saves the specified text attributes onto an internal stack.
        // Arguments:
        // - currentAttributes - The attributes to save onto the stack.
        // - options - If none supplied, the full attributes are saved. Else only the
        //   specified parts of currentAttributes are saved.
        // Return Value:
        // - <none>
        void Push(const TextAttribute& currentAttributes,
                  const gsl::span<const DispatchTypes::GraphicsOptions> options) noexcept
        {
            uint32_t validParts = 0;

            if (options.size() == 0)
            {
                // We save all current attributes.
                validParts = UINT32_MAX;
            }
            else
            {
                // Each option is encoded as a bit in validParts. Options that aren't
                // supported are ignored. So if you try to save only unsuppported aspects
                // of the current text attributes, validParts will be zero, and you'll do
                // what is effectively an "empty" push (the subsequent pop will not change
                // the current attributes).
                for (auto option : options)
                {
                    validParts |= _GraphicsOptionToFlag(option);
                }
            }

            if (_numSgrPushes < _countof(_storedSgrAttributes))
            {
// Must disable 26482 "Only index into arrays using constant expressions" because we are
// implementing a stack, and that's the whole point.
// We also disable the warning for using gsl::at, because doing that yields another: "No
// array to pointer decay".
#pragma warning(push)
#pragma warning(disable : 26482 26446)
                _storedSgrAttributes[_numSgrPushes] = currentAttributes;
                _validAttributes[_numSgrPushes] = validParts;
#pragma warning(pop)
            }

            if (_numSgrPushes < c_MaxBalancedPushes)
            {
                _numSgrPushes++;
            }
        }

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
        const TextAttribute Pop(const TextAttribute& currentAttributes) noexcept
        {
            if (_numSgrPushes > 0)
            {
                _numSgrPushes--;

                if (_numSgrPushes < _countof(_storedSgrAttributes))
                {
// Must disable 26482 "Only index into arrays using constant expressions" because we are
// implementing a stack, and that's the whole point.
// We also disable the warning for using gsl::at, because doing that yields another: "No
// array to pointer decay".
#pragma warning(push)
#pragma warning(disable : 26482 26446)
                    const uint32_t validParts = _validAttributes[_numSgrPushes];

                    if (validParts == UINT32_MAX)
                    {
                        return _storedSgrAttributes[_numSgrPushes];
                    }
                    else
                    {
                        return _CombineWithCurrentAttributes(currentAttributes,
                                                             _storedSgrAttributes[_numSgrPushes],
                                                             validParts);
                    }
#pragma warning(pop)
                }
            }

            return currentAttributes;
        }

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
        static constexpr uint32_t _GraphicsOptionToFlag(DispatchTypes::GraphicsOptions option)
        {
            int iOption = static_cast<int>(option);

            if (iOption < (sizeof(uint32_t) * 8))
            {
                iOption = 1 << iOption;
            }
            // else it's a bad parameter; we'll just ignore it

            return iOption;
        }

        TextAttribute _CombineWithCurrentAttributes(const TextAttribute& currentAttributes,
                                                    const TextAttribute& savedAttribute,
                                                    uint32_t validParts) noexcept // of savedAttribute
        {
            TextAttribute result = currentAttributes;

            // From xterm documentation:
            //
            //  CSI # {
            //  CSI Ps ; Ps # {
            //            Push video attributes onto stack (XTPUSHSGR), xterm.  The
            //            optional parameters correspond to the SGR encoding for video
            //            attributes, except for colors (which do not have a unique SGR
            //            code):
            //              Ps = 1  -> Bold.
            //              Ps = 2  -> Faint.
            //              Ps = 3  -> Italicized.
            //              Ps = 4  -> Underlined.
            //              Ps = 5  -> Blink.
            //              Ps = 7  -> Inverse.
            //              Ps = 8  -> Invisible.
            //              Ps = 9  -> Crossed-out characters.
            //              Ps = 1 0  -> Foreground color.
            //              Ps = 1 1  -> Background color.
            //              Ps = 2 1  -> Doubly-underlined.
            //
            //  (some closing braces for people with editors that get thrown off without them: }})
            //
            // Attributes that are not currently supported are simply ignored.

            if (_GraphicsOptionToFlag(DispatchTypes::GraphicsOptions::BoldBright) & validParts)
            {
                if (savedAttribute.IsBold())
                {
                    result.Embolden();
                }
                else
                {
                    result.Debolden();
                }
            }

            if (_GraphicsOptionToFlag(DispatchTypes::GraphicsOptions::Underline) & validParts)
            {
                if (savedAttribute.IsUnderline())
                {
                    result.EnableUnderline();
                }
                else
                {
                    result.DisableUnderline();
                }
            }

            if (_GraphicsOptionToFlag(DispatchTypes::GraphicsOptions::Negative) & validParts)
            {
                if (savedAttribute.IsReverseVideo())
                {
                    if (!result.IsReverseVideo())
                    {
                        result.Invert();
                    }
                }
                else
                {
                    if (result.IsReverseVideo())
                    {
                        result.Invert();
                    }
                }
            }

            if (_GraphicsOptionToFlag(DispatchTypes::GraphicsOptions::SaveForegroundColor) & validParts)
            {
                result.SetForegroundFrom(savedAttribute);
            }

            if (_GraphicsOptionToFlag(DispatchTypes::GraphicsOptions::SaveBackgroundColor) & validParts)
            {
                result.SetBackgroundFrom(savedAttribute);
            }

            return result;
        }

        int _numSgrPushes; // used as an index into the following arrays
        TextAttribute _storedSgrAttributes[c_MaxStoredSgrPushes];
        uint32_t _validAttributes[c_MaxStoredSgrPushes]; // flags that indicate which portions of the attributes are valid
    };
}
