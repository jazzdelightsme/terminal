// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"
#include "inc/sgrStack.hpp"

namespace Microsoft::Console::VirtualTerminal
{
    SgrStack::SgrStack() noexcept :
        _nextPushIndex{ 0 },
        _numSavedAttrs{ 0 }
    {
    }

    void SgrStack::Push(const TextAttribute& currentAttributes,
                        const gsl::span<const DispatchTypes::SgrSaveRestoreStackOptions> options) noexcept
    {
        AttrBitset validParts;

        if (options.size() == 0)
        {
            // We save all current attributes.
            validParts.set(); // (sets all bits)
        }
        else
        {
            // Each option is encoded as a bit in validParts. Options that aren't
            // supported are ignored. So if you try to save only unsupported aspects of
            // the current text attributes, validParts end up as zero, and you'll do what
            // is effectively an "empty" push (the subsequent pop will not change the
            // current attributes), which is the correct behavior.
            for (auto option : options)
            {
                const size_t optionAsIndex = static_cast<size_t>(option);

                // Options must be specified singly; not in combination. Values that are
                // out of range will be ignored.
                if (optionAsIndex < validParts.size())
                {
                    try
                    {
                        validParts.set(optionAsIndex);
                    }
                    catch (std::out_of_range&)
                    {
                        // We should not be able to reach here: we already checked
                        // optionAsIndex against the size of the bitset.
                        RaiseFailFastException(nullptr, nullptr, 0);
                    }
                }
            }
        }

        if (_numSavedAttrs < gsl::narrow<int>(_storedSgrAttributes.size()))
        {
            _numSavedAttrs++;
        }

        _storedSgrAttributes.at(_nextPushIndex) = { currentAttributes, validParts };
        _nextPushIndex = (_nextPushIndex + 1) % gsl::narrow<int>(_storedSgrAttributes.size());
    }

    const TextAttribute SgrStack::Pop(const TextAttribute& currentAttributes) noexcept
    {
        if (_numSavedAttrs > 0)
        {
            _numSavedAttrs--;

            if (_nextPushIndex == 0)
            {
                _nextPushIndex = gsl::narrow<int>(_storedSgrAttributes.size() - 1);
            }
            else
            {
                _nextPushIndex--;
            }

            SavedSgrAttributes& restoreMe = _storedSgrAttributes.at(_nextPushIndex);

            if (restoreMe.ValidParts.all())
            {
                return restoreMe.TextAttributes;
            }
            else
            {
                return _CombineWithCurrentAttributes(currentAttributes,
                                                     restoreMe.TextAttributes,
                                                     restoreMe.ValidParts);
            }
        }

        return currentAttributes;
    }

    TextAttribute SgrStack::_CombineWithCurrentAttributes(const TextAttribute& currentAttributes,
                                                          const TextAttribute& savedAttribute,
                                                          const AttrBitset validParts) noexcept // of savedAttribute
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
        // Note that not all of these attributes are actually supported by
        // renderers/conhost, despite setters/getters on TextAttribute.

        try
        {
            // Boldness = 1,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Boldness)))
            {
                result.SetBold(savedAttribute.IsBold());
            }

            // Faintness = 2,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Faintness)))
            {
                result.SetFaint(savedAttribute.IsFaint());
            }

            // Italics = 3,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Italics)))
            {
                result.SetItalic(savedAttribute.IsItalic());
            }

            // Underline = 4,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Underline)))
            {
                result.SetUnderlined(savedAttribute.IsUnderlined());
            }

            // Blink = 5,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Blink)))
            {
                result.SetBlinking(savedAttribute.IsBlinking());
            }

            // Negative = 7,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Negative)))
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

            // Invisible = 8,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::Invisible)))
            {
                result.SetInvisible(savedAttribute.IsInvisible());
            }

            // CrossedOut = 9,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::CrossedOut)))
            {
                result.SetCrossedOut(savedAttribute.IsCrossedOut());
            }

            // SaveForegroundColor = 10,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::SaveForegroundColor)))
            {
                result.SetForegroundFrom(savedAttribute);
            }

            // SaveBackgroundColor = 11,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::SaveBackgroundColor)))
            {
                result.SetBackgroundFrom(savedAttribute);
            }

            // DoublyUnderlined = 21,
            if (validParts.test(static_cast<size_t>(DispatchTypes::SgrSaveRestoreStackOptions::DoublyUnderlined)))
            {
                result.SetDoublyUnderlined(savedAttribute.IsDoublyUnderlined());
            }
        }
        catch (std::out_of_range&)
        {
            // We should not be able to reach here: all values passed to bitset::test are
            // constants, clearly in range of the bitset.
            RaiseFailFastException(nullptr, nullptr, 0);
        }

        return result;
    }

}
