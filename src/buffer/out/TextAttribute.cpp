// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"
#include "TextAttribute.hpp"
#include "../../inc/conattrs.hpp"

bool TextAttribute::IsLegacy() const noexcept
{
    return _foreground.IsLegacy() && _background.IsLegacy();
}

// Arguments:
// - None
// Return Value:
// - color that should be displayed as the foreground color
COLORREF TextAttribute::CalculateRgbForeground(std::basic_string_view<COLORREF> colorTable,
                                               COLORREF defaultFgColor,
                                               COLORREF defaultBgColor) const
{
    return IsReverseVideo() ? _GetRgbBackground(colorTable, defaultBgColor) : _GetRgbForeground(colorTable, defaultFgColor);
}

// Routine Description:
// - Calculates rgb background color based off of current color table and active modification attributes
// Arguments:
// - None
// Return Value:
// - color that should be displayed as the background color
COLORREF TextAttribute::CalculateRgbBackground(std::basic_string_view<COLORREF> colorTable,
                                               COLORREF defaultFgColor,
                                               COLORREF defaultBgColor) const
{
    return IsReverseVideo() ? _GetRgbForeground(colorTable, defaultFgColor) : _GetRgbBackground(colorTable, defaultBgColor);
}

// Routine Description:
// - Makes this TextAttribute's foreground color the same as the other one.
// Arguments:
// - The TextAttribute to copy the foreground color from
// Return Value:
// - <none>
void TextAttribute::SetForegroundFrom(const TextAttribute& other) noexcept
{
    _foreground = other._foreground;
    WI_ClearAllFlags(_wAttrLegacy, FG_ATTRS);
    _wAttrLegacy |= (other._wAttrLegacy & FG_ATTRS);
}

// Routine Description:
// - Makes this TextAttribute's background color the same as the other one.
// Arguments:
// - The TextAttribute to copy the background color from
// Return Value:
// - <none>
void TextAttribute::SetBackgroundFrom(const TextAttribute& other) noexcept
{
    _background = other._background;
    WI_ClearAllFlags(_wAttrLegacy, BG_ATTRS);
    _wAttrLegacy |= (other._wAttrLegacy & BG_ATTRS);
}

// Routine Description:
// - gets rgb foreground color, possibly based off of current color table. Does not take active modification
// attributes into account
// Arguments:
// - None
// Return Value:
// - color that is stored as the foreground color
COLORREF TextAttribute::_GetRgbForeground(std::basic_string_view<COLORREF> colorTable,
                                          COLORREF defaultColor) const
{
    return _foreground.GetColor(colorTable, defaultColor, _isBold);
}

// Routine Description:
// - gets rgb background color, possibly based off of current color table. Does not take active modification
// attributes into account
// Arguments:
// - None
// Return Value:
// - color that is stored as the background color
COLORREF TextAttribute::_GetRgbBackground(std::basic_string_view<COLORREF> colorTable,
                                          COLORREF defaultColor) const
{
    return _background.GetColor(colorTable, defaultColor, false);
}

void TextAttribute::SetMetaAttributes(const WORD wMeta) noexcept
{
    WI_UpdateFlagsInMask(_wAttrLegacy, META_ATTRS, wMeta);
    WI_ClearAllFlags(_wAttrLegacy, COMMON_LVB_SBCSDBCS);
}

WORD TextAttribute::GetMetaAttributes() const noexcept
{
    WORD wMeta = _wAttrLegacy;
    WI_ClearAllFlags(wMeta, FG_ATTRS);
    WI_ClearAllFlags(wMeta, BG_ATTRS);
    WI_ClearAllFlags(wMeta, COMMON_LVB_SBCSDBCS);
    return wMeta;
}

void TextAttribute::SetForeground(const COLORREF rgbForeground)
{
    _foreground = TextColor(rgbForeground);
}

void TextAttribute::SetBackground(const COLORREF rgbBackground)
{
    _background = TextColor(rgbBackground);
}

void TextAttribute::SetFromLegacy(const WORD wLegacy) noexcept
{
    _wAttrLegacy = static_cast<WORD>(wLegacy & META_ATTRS);
    WI_ClearAllFlags(_wAttrLegacy, COMMON_LVB_SBCSDBCS);
    BYTE fgIndex = static_cast<BYTE>(wLegacy & FG_ATTRS);
    BYTE bgIndex = static_cast<BYTE>(wLegacy & BG_ATTRS) >> 4;
    _foreground = TextColor(fgIndex);
    _background = TextColor(bgIndex);
}

void TextAttribute::SetLegacyAttributes(const WORD attrs,
                                        const bool setForeground,
                                        const bool setBackground,
                                        const bool setMeta)
{
    if (setForeground)
    {
        BYTE fgIndex = (BYTE)(attrs & FG_ATTRS);
        _foreground = TextColor(fgIndex);
    }
    if (setBackground)
    {
        BYTE bgIndex = (BYTE)(attrs & BG_ATTRS) >> 4;
        _background = TextColor(bgIndex);
    }
    if (setMeta)
    {
        SetMetaAttributes(attrs);
    }
}

// Method Description:
// - Sets the foreground and/or background to a particular index in the 256color
//      table. If either parameter is nullptr, it's ignored.
//   This method can be used to set the colors to indexes in the range [0, 255],
//      as opposed to SetLegacyAttributes, which clamps them to [0,15]
// Arguments:
// - foreground: nullptr if we should ignore this attr, else a pointer to a byte
//      value to use as an index into the 256-color table.
// - background: nullptr if we should ignore this attr, else a pointer to a byte
//      value to use as an index into the 256-color table.
// Return Value:
// - <none>
void TextAttribute::SetIndexedAttributes(const std::optional<const BYTE> foreground,
                                         const std::optional<const BYTE> background) noexcept
{
    if (foreground)
    {
        BYTE fgIndex = (*foreground) & 0xFF;
        _foreground = TextColor(fgIndex);
    }
    if (background)
    {
        BYTE bgIndex = (*background) & 0xFF;
        _background = TextColor(bgIndex);
    }
}

void TextAttribute::SetColor(const COLORREF rgbColor, const bool fIsForeground)
{
    if (fIsForeground)
    {
        SetForeground(rgbColor);
    }
    else
    {
        SetBackground(rgbColor);
    }
}

bool TextAttribute::IsBold() const noexcept
{
    return _isBold;
}

bool TextAttribute::IsUnderline() const noexcept
{
    return IsBottomHorizontalDisplayed();
}

bool TextAttribute::IsReverseVideo() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_REVERSE_VIDEO);
}

bool TextAttribute::IsLeadingByte() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_LEADING_BYTE);
}

bool TextAttribute::IsTrailingByte() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_LEADING_BYTE);
}

bool TextAttribute::IsTopHorizontalDisplayed() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_GRID_HORIZONTAL);
}

bool TextAttribute::IsBottomHorizontalDisplayed() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_UNDERSCORE);
}

bool TextAttribute::IsLeftVerticalDisplayed() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_GRID_LVERTICAL);
}

bool TextAttribute::IsRightVerticalDisplayed() const noexcept
{
    return WI_IsFlagSet(_wAttrLegacy, COMMON_LVB_GRID_RVERTICAL);
}

void TextAttribute::SetLeftVerticalDisplayed(bool isDisplayed) noexcept
{
    WI_UpdateFlag(_wAttrLegacy, COMMON_LVB_GRID_LVERTICAL, isDisplayed);
}

void TextAttribute::SetRightVerticalDisplayed(bool isDisplayed) noexcept
{
    WI_UpdateFlag(_wAttrLegacy, COMMON_LVB_GRID_RVERTICAL, isDisplayed);
}

void TextAttribute::SetBottomHorizontalDisplayed(bool isDisplayed) noexcept
{
    WI_UpdateFlag(_wAttrLegacy, COMMON_LVB_UNDERSCORE, isDisplayed);
}

void TextAttribute::Embolden() noexcept
{
    _SetBoldness(true);
}

void TextAttribute::Debolden() noexcept
{
    _SetBoldness(false);
}

void TextAttribute::EnableUnderline() noexcept
{
    SetBottomHorizontalDisplayed(true);
}

void TextAttribute::DisableUnderline() noexcept
{
    SetBottomHorizontalDisplayed(false);
}

// Routine Description:
// - swaps foreground and background color
void TextAttribute::Invert() noexcept
{
    WI_ToggleFlag(_wAttrLegacy, COMMON_LVB_REVERSE_VIDEO);
}

void TextAttribute::_SetBoldness(const bool isBold) noexcept
{
    _isBold = isBold;
}

void TextAttribute::SetDefaultForeground() noexcept
{
    _foreground = TextColor();
}

void TextAttribute::SetDefaultBackground() noexcept
{
    _background = TextColor();
}

// Method Description:
// - Returns true if this attribute indicates its foreground is the "default"
//      foreground. Its _rgbForeground will contain the actual value of the
//      default foreground. If the default colors are ever changed, this method
//      should be used to identify attributes with the default fg value, and
//      update them accordingly.
// Arguments:
// - <none>
// Return Value:
// - true iff this attribute indicates it's the "default" foreground color.
bool TextAttribute::ForegroundIsDefault() const noexcept
{
    return _foreground.IsDefault();
}

// Method Description:
// - Returns true if this attribute indicates its background is the "default"
//      background. Its _rgbBackground will contain the actual value of the
//      default background. If the default colors are ever changed, this method
//      should be used to identify attributes with the default bg value, and
//      update them accordingly.
// Arguments:
// - <none>
// Return Value:
// - true iff this attribute indicates it's the "default" background color.
bool TextAttribute::BackgroundIsDefault() const noexcept
{
    return _background.IsDefault();
}
