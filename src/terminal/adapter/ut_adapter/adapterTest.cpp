// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"
#include <wextestclass.h>
#include "..\..\inc\consoletaeftemplates.hpp"

#include "adaptDispatch.hpp"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace Microsoft
{
    namespace Console
    {
        namespace VirtualTerminal
        {
            class AdapterTest;
            class ConAdapterTestGetSet;
        };
    };
};

enum class CursorY
{
    TOP,
    BOTTOM,
    YCENTER
};

enum class CursorX
{
    LEFT,
    RIGHT,
    XCENTER
};

enum class CursorDirection : unsigned int
{
    UP = 0,
    DOWN = 1,
    RIGHT = 2,
    LEFT = 3,
    NEXTLINE = 4,
    PREVLINE = 5
};

enum class AbsolutePosition : unsigned int
{
    CursorHorizontal = 0,
    VerticalLine = 1,
};

using namespace Microsoft::Console::VirtualTerminal;

class TestGetSet final : public ConGetSet
{
public:
    BOOL GetConsoleScreenBufferInfoEx(_Out_ CONSOLE_SCREEN_BUFFER_INFOEX* const psbiex) const override
    {
        Log::Comment(L"GetConsoleScreenBufferInfoEx MOCK returning data...");

        if (_fGetConsoleScreenBufferInfoExResult)
        {
            psbiex->dwSize = _coordBufferSize;
            psbiex->srWindow = _srViewport;
            psbiex->dwCursorPosition = _coordCursorPos;
            psbiex->wAttributes = _attribute.GetLegacyAttributes();
        }

        return _fGetConsoleScreenBufferInfoExResult;
    }
    BOOL SetConsoleScreenBufferInfoEx(const CONSOLE_SCREEN_BUFFER_INFOEX* const psbiex) override
    {
        Log::Comment(L"SetConsoleScreenBufferInfoEx MOCK returning data...");

        if (_fSetConsoleScreenBufferInfoExResult)
        {
            VERIFY_ARE_EQUAL(_coordExpectedCursorPos, psbiex->dwCursorPosition);
            VERIFY_ARE_EQUAL(_coordExpectedScreenBufferSize, psbiex->dwSize);
            VERIFY_ARE_EQUAL(_srExpectedScreenBufferViewport, psbiex->srWindow);
            VERIFY_ARE_EQUAL(_expectedAttribute.GetLegacyAttributes(), psbiex->wAttributes);
        }
        return _fSetConsoleScreenBufferInfoExResult;
    }
    BOOL SetConsoleCursorPosition(const COORD dwCursorPosition) override
    {
        Log::Comment(L"SetConsoleCursorPosition MOCK called...");

        if (_fSetConsoleCursorPositionResult)
        {
            VERIFY_ARE_EQUAL(_coordExpectedCursorPos, dwCursorPosition);
            _coordCursorPos = dwCursorPosition;
        }

        return _fSetConsoleCursorPositionResult;
    }

    BOOL GetConsoleCursorInfo(_In_ CONSOLE_CURSOR_INFO* const pConsoleCursorInfo) const override
    {
        Log::Comment(L"GetConsoleCursorInfo MOCK called...");

        if (_fGetConsoleCursorInfoResult)
        {
            pConsoleCursorInfo->dwSize = _dwCursorSize;
            pConsoleCursorInfo->bVisible = _fCursorVisible;
        }

        return _fGetConsoleCursorInfoResult;
    }

    BOOL SetConsoleCursorInfo(const CONSOLE_CURSOR_INFO* const pConsoleCursorInfo) override
    {
        Log::Comment(L"SetConsoleCursorInfo MOCK called...");

        if (_fSetConsoleCursorInfoResult)
        {
            VERIFY_ARE_EQUAL(_dwExpectedCursorSize, pConsoleCursorInfo->dwSize);
            VERIFY_ARE_EQUAL(_fExpectedCursorVisible, pConsoleCursorInfo->bVisible);
        }

        return _fSetConsoleCursorInfoResult;
    }

    BOOL SetConsoleWindowInfo(const BOOL bAbsolute, const SMALL_RECT* const lpConsoleWindow) override
    {
        Log::Comment(L"SetConsoleWindowInfo MOCK called...");

        if (_fSetConsoleWindowInfoResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedWindowAbsolute, bAbsolute);
            VERIFY_ARE_EQUAL(_srExpectedConsoleWindow, *lpConsoleWindow);
            _srViewport = *lpConsoleWindow;
        }

        return _fSetConsoleWindowInfoResult;
    }

    BOOL PrivateSetCursorKeysMode(const bool fCursorKeysApplicationMode) override
    {
        Log::Comment(L"PrivateSetCursorKeysMode MOCK called...");

        if (_fPrivateSetCursorKeysModeResult)
        {
            VERIFY_ARE_EQUAL(_fCursorKeysApplicationMode, fCursorKeysApplicationMode);
        }

        return _fPrivateSetCursorKeysModeResult;
    }

    BOOL PrivateSetKeypadMode(const bool fKeypadApplicationMode) override
    {
        Log::Comment(L"PrivateSetKeypadMode MOCK called...");

        if (_fPrivateSetKeypadModeResult)
        {
            VERIFY_ARE_EQUAL(_fKeypadApplicationMode, fKeypadApplicationMode);
        }

        return _fPrivateSetKeypadModeResult;
    }

    BOOL PrivateShowCursor(const bool show) override
    {
        Log::Comment(L"PrivateShowCursor MOCK called...");

        if (_privateShowCursorResult)
        {
            VERIFY_ARE_EQUAL(_expectedShowCursor, show);
        }

        return _privateShowCursorResult;
    }

    BOOL PrivateAllowCursorBlinking(const bool fEnable) override
    {
        Log::Comment(L"PrivateAllowCursorBlinking MOCK called...");

        if (_fPrivateAllowCursorBlinkingResult)
        {
            VERIFY_ARE_EQUAL(_fEnable, fEnable);
        }

        return _fPrivateAllowCursorBlinkingResult;
    }

    BOOL SetConsoleTextAttribute(const WORD wAttr) override
    {
        Log::Comment(L"SetConsoleTextAttribute MOCK called...");

        if (_fSetConsoleTextAttributeResult)
        {
            VERIFY_ARE_EQUAL(_expectedAttribute.GetLegacyAttributes(), wAttr);
            _attribute.SetFromLegacy(wAttr);
            _fUsingRgbColor = false;
        }

        return _fSetConsoleTextAttributeResult;
    }

    BOOL PrivateSetLegacyAttributes(const WORD wAttr, const bool fForeground, const bool fBackground, const bool fMeta) override
    {
        Log::Comment(L"PrivateSetLegacyAttributes MOCK called...");
        if (_fPrivateSetLegacyAttributesResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedForeground, fForeground);
            VERIFY_ARE_EQUAL(_fExpectedBackground, fBackground);
            VERIFY_ARE_EQUAL(_fExpectedMeta, fMeta);

            _attribute.SetLegacyAttributes(wAttr, fForeground, fBackground, fMeta);

            VERIFY_ARE_EQUAL(_expectedAttribute.GetLegacyAttributes(), wAttr);

            _fExpectedForeground = _fExpectedBackground = _fExpectedMeta = false;
        }

        return _fPrivateSetLegacyAttributesResult;
    }

    BOOL SetConsoleXtermTextAttribute(const int iXtermTableEntry, const bool fIsForeground) override
    {
        Log::Comment(L"SetConsoleXtermTextAttribute MOCK called...");

        if (_fSetConsoleXtermTextAttributeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedIsForeground, fIsForeground);
            _fIsForeground = fIsForeground;
            VERIFY_ARE_EQUAL(_iExpectedXtermTableEntry, iXtermTableEntry);
            _iXtermTableEntry = iXtermTableEntry;
            // if the table entry is less than 16, keep using the legacy attr
            _fUsingRgbColor = iXtermTableEntry > 16;
            if (!_fUsingRgbColor)
            {
                //Convert the xterm index to the win index
                bool fRed = (iXtermTableEntry & 0x01) > 0;
                bool fGreen = (iXtermTableEntry & 0x02) > 0;
                bool fBlue = (iXtermTableEntry & 0x04) > 0;
                bool fBright = (iXtermTableEntry & 0x08) > 0;
                WORD iWinEntry = (fRed ? 0x4 : 0x0) | (fGreen ? 0x2 : 0x0) | (fBlue ? 0x1 : 0x0) | (fBright ? 0x8 : 0x0);
                WORD legacyAttr = _attribute.GetLegacyAttributes();
                legacyAttr = fIsForeground ? ((legacyAttr & 0xF0) | iWinEntry) : ((iWinEntry << 4) | (legacyAttr & 0x0F));
                _attribute.SetFromLegacy(legacyAttr);
            }
        }

        return _fSetConsoleXtermTextAttributeResult;
    }

    BOOL SetConsoleRGBTextAttribute(const COLORREF rgbColor, const bool fIsForeground) override
    {
        Log::Comment(L"SetConsoleRGBTextAttribute MOCK called...");
        if (_fSetConsoleRGBTextAttributeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedIsForeground, fIsForeground);
            _fIsForeground = fIsForeground;
            VERIFY_ARE_EQUAL(_ExpectedColor, rgbColor);
            _rgbColor = rgbColor;
            _fUsingRgbColor = true;
        }

        return _fSetConsoleRGBTextAttributeResult;
    }

    BOOL PrivateBoldText(const bool isBold) override
    {
        Log::Comment(L"PrivateBoldText MOCK called...");
        if (_fPrivateBoldTextResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedIsBold, isBold);
            if (isBold)
            {
                _attribute.Embolden();
            }
            else
            {
                _attribute.Debolden();
            }
            _fExpectedIsBold = false;
        }
        return !!_fPrivateBoldTextResult;
    }

    BOOL PrivateGetExtendedTextAttributes(ExtendedAttributes* const /*pAttrs*/)
    {
        Log::Comment(L"PrivateGetExtendedTextAttributes MOCK called...");
        return true;
    }

    BOOL PrivateSetExtendedTextAttributes(const ExtendedAttributes /*attrs*/)
    {
        Log::Comment(L"PrivateSetExtendedTextAttributes MOCK called...");
        return true;
    }

    BOOL PrivateGetTextAttributes(TextAttribute* const pAttributes) const
    {
        Log::Comment(L"PrivateGetTextAttributes MOCK called...");

        if (pAttributes != nullptr && _fPrivateGetTextAttributesResult)
        {
            *pAttributes = _attribute;
        }

        return _fPrivateGetTextAttributesResult;
    }

    BOOL PrivateSetTextAttributes(const TextAttribute& attributes)
    {
        Log::Comment(L"PrivateSetTextAttributes MOCK called...");
        if (_fPrivateSetTextAttributesResult)
        {
            VERIFY_ARE_EQUAL(_expectedAttribute, attributes);
            _attribute = attributes;
        }

        return _fPrivateSetTextAttributesResult;
    }

    BOOL PrivateWriteConsoleInputW(_Inout_ std::deque<std::unique_ptr<IInputEvent>>& events,
                                   _Out_ size_t& eventsWritten) override
    {
        Log::Comment(L"PrivateWriteConsoleInputW MOCK called...");

        if (_fPrivateWriteConsoleInputWResult)
        {
            // move all the input events we were given into local storage so we can test against them
            Log::Comment(NoThrowString().Format(L"Moving %zu input events into local storage...", events.size()));

            _events.clear();
            _events.swap(events);
            eventsWritten = _events.size();
        }

        return _fPrivateWriteConsoleInputWResult;
    }

    BOOL PrivatePrependConsoleInput(_Inout_ std::deque<std::unique_ptr<IInputEvent>>& events,
                                    _Out_ size_t& eventsWritten) override
    {
        Log::Comment(L"PrivatePrependConsoleInput MOCK called...");

        if (_fPrivatePrependConsoleInputResult)
        {
            // move all the input events we were given into local storage so we can test against them
            Log::Comment(NoThrowString().Format(L"Moving %zu input events into local storage...", events.size()));

            _events.clear();
            _events.swap(events);
            eventsWritten = _events.size();
        }

        return _fPrivatePrependConsoleInputResult;
    }

    BOOL PrivateWriteConsoleControlInput(_In_ KeyEvent key) override
    {
        Log::Comment(L"PrivateWriteConsoleControlInput MOCK called...");

        if (_fPrivateWriteConsoleControlInputResult)
        {
            VERIFY_ARE_EQUAL('C', key.GetVirtualKeyCode());
            VERIFY_ARE_EQUAL(0x3, key.GetCharData());
            VERIFY_ARE_EQUAL(true, key.IsCtrlPressed());
        }

        return _fPrivateWriteConsoleControlInputResult;
    }

    BOOL PrivateSetScrollingRegion(const SMALL_RECT* const psrScrollMargins) override
    {
        Log::Comment(L"PrivateSetScrollingRegion MOCK called...");

        if (_fPrivateSetScrollingRegionResult)
        {
            VERIFY_ARE_EQUAL(_srExpectedScrollRegion, *psrScrollMargins);
        }

        return _fPrivateSetScrollingRegionResult;
    }

    BOOL PrivateReverseLineFeed() override
    {
        Log::Comment(L"PrivateReverseLineFeed MOCK called...");
        // We made it through the adapter, woo! Return true.
        return TRUE;
    }

    BOOL MoveCursorVertically(const short lines) override
    {
        Log::Comment(L"MoveCursorVertically MOCK called...");
        if (_fMoveCursorVerticallyResult)
        {
            VERIFY_ARE_EQUAL(_expectedLines, lines);
            _coordCursorPos = { _coordCursorPos.X, _coordCursorPos.Y + lines };
        }
        return !!_fMoveCursorVerticallyResult;
    }

    BOOL SetConsoleTitleW(const std::wstring_view title)
    {
        Log::Comment(L"SetConsoleTitleW MOCK called...");

        if (_fSetConsoleTitleWResult)
        {
            VERIFY_ARE_EQUAL(_pwchExpectedWindowTitle, title.data());
            VERIFY_ARE_EQUAL(_sCchExpectedTitleLength, title.size());
        }
        return TRUE;
    }

    BOOL PrivateUseAlternateScreenBuffer() override
    {
        Log::Comment(L"PrivateUseAlternateScreenBuffer MOCK called...");
        return true;
    }

    BOOL PrivateUseMainScreenBuffer() override
    {
        Log::Comment(L"PrivateUseMainScreenBuffer MOCK called...");
        return true;
    }

    BOOL PrivateHorizontalTabSet() override
    {
        Log::Comment(L"PrivateHorizontalTabSet MOCK called...");
        // We made it through the adapter, woo! Return true.
        return TRUE;
    }

    BOOL PrivateForwardTab(const SHORT sNumTabs) override
    {
        Log::Comment(L"PrivateForwardTab MOCK called...");
        if (_fPrivateForwardTabResult)
        {
            VERIFY_ARE_EQUAL(_sExpectedNumTabs, sNumTabs);
        }
        return TRUE;
    }

    BOOL PrivateBackwardsTab(const SHORT sNumTabs) override
    {
        Log::Comment(L"PrivateBackwardsTab MOCK called...");
        if (_fPrivateBackwardsTabResult)
        {
            VERIFY_ARE_EQUAL(_sExpectedNumTabs, sNumTabs);
        }
        return TRUE;
    }

    BOOL PrivateTabClear(const bool fClearAll) override
    {
        Log::Comment(L"PrivateTabClear MOCK called...");
        if (_fPrivateTabClearResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedClearAll, fClearAll);
        }
        return TRUE;
    }

    BOOL PrivateSetDefaultTabStops() override
    {
        Log::Comment(L"PrivateSetDefaultTabStops MOCK called...");
        return TRUE;
    }

    BOOL PrivateEnableVT200MouseMode(const bool fEnabled) override
    {
        Log::Comment(L"PrivateEnableVT200MouseMode MOCK called...");
        if (_fPrivateEnableVT200MouseModeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedMouseEnabled, fEnabled);
        }
        return _fPrivateEnableVT200MouseModeResult;
    }

    BOOL PrivateEnableUTF8ExtendedMouseMode(const bool fEnabled) override
    {
        Log::Comment(L"PrivateEnableUTF8ExtendedMouseMode MOCK called...");
        if (_fPrivateEnableUTF8ExtendedMouseModeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedMouseEnabled, fEnabled);
        }
        return _fPrivateEnableUTF8ExtendedMouseModeResult;
    }

    BOOL PrivateEnableSGRExtendedMouseMode(const bool fEnabled) override
    {
        Log::Comment(L"PrivateEnableSGRExtendedMouseMode MOCK called...");
        if (_fPrivateEnableSGRExtendedMouseModeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedMouseEnabled, fEnabled);
        }
        return _fPrivateEnableSGRExtendedMouseModeResult;
    }

    BOOL PrivateEnableButtonEventMouseMode(const bool fEnabled) override
    {
        Log::Comment(L"PrivateEnableButtonEventMouseMode MOCK called...");
        if (_fPrivateEnableButtonEventMouseModeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedMouseEnabled, fEnabled);
        }
        return _fPrivateEnableButtonEventMouseModeResult;
    }

    BOOL PrivateEnableAnyEventMouseMode(const bool fEnabled) override
    {
        Log::Comment(L"PrivateEnableAnyEventMouseMode MOCK called...");
        if (_fPrivateEnableAnyEventMouseModeResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedMouseEnabled, fEnabled);
        }
        return _fPrivateEnableAnyEventMouseModeResult;
    }

    BOOL PrivateEnableAlternateScroll(const bool fEnabled) override
    {
        Log::Comment(L"PrivateEnableAlternateScroll MOCK called...");
        if (_fPrivateEnableAlternateScrollResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedAlternateScrollEnabled, fEnabled);
        }
        return _fPrivateEnableAlternateScrollResult;
    }

    BOOL PrivateEraseAll() override
    {
        Log::Comment(L"PrivateEraseAll MOCK called...");
        return TRUE;
    }

    BOOL SetCursorStyle(const CursorType cursorType) override
    {
        Log::Comment(L"SetCursorStyle MOCK called...");
        if (_fSetCursorStyleResult)
        {
            VERIFY_ARE_EQUAL(_ExpectedCursorStyle, cursorType);
        }
        return _fSetCursorStyleResult;
    }

    BOOL SetCursorColor(const COLORREF cursorColor) override
    {
        Log::Comment(L"SetCursorColor MOCK called...");
        if (_fSetCursorColorResult)
        {
            VERIFY_ARE_EQUAL(_ExpectedCursorColor, cursorColor);
        }
        return _fSetCursorColorResult;
    }

    BOOL PrivateGetConsoleScreenBufferLegacyAttributes(_Out_ WORD* const pwAttributes) override
    {
        Log::Comment(L"PrivateGetConsoleScreenBufferLegacyAttributes MOCK returning data...");

        if (pwAttributes != nullptr && _fPrivateGetConsoleScreenBufferLegacyAttributesResult)
        {
            *pwAttributes = _attribute.GetLegacyAttributes();
        }

        return _fPrivateGetConsoleScreenBufferLegacyAttributesResult;
    }

    BOOL PrivateRefreshWindow() override
    {
        Log::Comment(L"PrivateRefreshWindow MOCK called...");
        // We made it through the adapter, woo! Return true.
        return TRUE;
    }

    BOOL PrivateSuppressResizeRepaint() override
    {
        Log::Comment(L"PrivateSuppressResizeRepaint MOCK called...");
        VERIFY_IS_TRUE(false, L"AdaptDispatch should never be calling this function.");
        return FALSE;
    }

    BOOL GetConsoleOutputCP(_Out_ unsigned int* const puiOutputCP) override
    {
        Log::Comment(L"GetConsoleOutputCP MOCK called...");
        if (_fGetConsoleOutputCPResult)
        {
            *puiOutputCP = _uiExpectedOutputCP;
        }
        return _fGetConsoleOutputCPResult;
    }

    BOOL IsConsolePty(_Out_ bool* const isPty) const override
    {
        Log::Comment(L"IsConsolePty MOCK called...");
        if (_fIsConsolePtyResult)
        {
            *isPty = _fIsPty;
        }
        return _fIsConsolePtyResult;
    }

    BOOL DeleteLines(const unsigned int /*count*/) override
    {
        Log::Comment(L"DeleteLines MOCK called...");
        return TRUE;
    }

    BOOL InsertLines(const unsigned int /*count*/) override
    {
        Log::Comment(L"InsertLines MOCK called...");
        return TRUE;
    }

    BOOL PrivateSetDefaultAttributes(const bool fForeground,
                                     const bool fBackground) override
    {
        Log::Comment(L"PrivateSetDefaultAttributes MOCK called...");
        if (_fPrivateSetDefaultAttributesResult)
        {
            VERIFY_ARE_EQUAL(_fExpectedForeground, fForeground);
            VERIFY_ARE_EQUAL(_fExpectedBackground, fBackground);
            if (fForeground)
            {
                _attribute.SetDefaultForeground();
            }
            if (fBackground)
            {
                _attribute.SetDefaultBackground();
            }

            _fExpectedForeground = _fExpectedBackground = false;
        }
        return _fPrivateSetDefaultAttributesResult;
    }

    BOOL MoveToBottom() const override
    {
        Log::Comment(L"MoveToBottom MOCK called...");
        return _fMoveToBottomResult;
    }

    BOOL PrivateSetColorTableEntry(const short index, const COLORREF value) const noexcept override
    {
        Log::Comment(L"PrivateSetColorTableEntry MOCK called...");
        if (_fPrivateSetColorTableEntryResult)
        {
            VERIFY_ARE_EQUAL(_expectedColorTableIndex, index);
            VERIFY_ARE_EQUAL(_expectedColorValue, value);
        }

        return _fPrivateSetColorTableEntryResult;
    }

    BOOL PrivateSetDefaultForeground(const COLORREF value) const noexcept override
    {
        Log::Comment(L"PrivateSetDefaultForeground MOCK called...");
        if (_fPrivateSetDefaultForegroundResult)
        {
            VERIFY_ARE_EQUAL(_expectedDefaultForegroundColorValue, value);
        }

        return _fPrivateSetDefaultForegroundResult;
    }

    BOOL PrivateSetDefaultBackground(const COLORREF value) const noexcept override
    {
        Log::Comment(L"PrivateSetDefaultForeground MOCK called...");
        if (_fPrivateSetDefaultBackgroundResult)
        {
            VERIFY_ARE_EQUAL(_expectedDefaultBackgroundColorValue, value);
        }

        return _fPrivateSetDefaultBackgroundResult;
    }

    BOOL PrivateFillRegion(const COORD /*startPosition*/,
                           const size_t /*fillLength*/,
                           const wchar_t /*fillChar*/,
                           const bool /*standardFillAttrs*/) noexcept override
    {
        Log::Comment(L"PrivateFillRegion MOCK called...");

        return TRUE;
    }

    BOOL PrivateScrollRegion(const SMALL_RECT /*scrollRect*/,
                             const std::optional<SMALL_RECT> /*clipRect*/,
                             const COORD /*destinationOrigin*/,
                             const bool /*standardFillAttrs*/) noexcept override
    {
        Log::Comment(L"PrivateScrollRegion MOCK called...");

        return TRUE;
    }

    void PrepData()
    {
        PrepData(CursorDirection::UP); // if called like this, the cursor direction doesn't matter.
    }

    void PrepData(CursorDirection dir)
    {
        switch (dir)
        {
        case CursorDirection::UP:
            return PrepData(CursorX::LEFT, CursorY::TOP);
        case CursorDirection::DOWN:
            return PrepData(CursorX::LEFT, CursorY::BOTTOM);
        case CursorDirection::LEFT:
            return PrepData(CursorX::LEFT, CursorY::TOP);
        case CursorDirection::RIGHT:
            return PrepData(CursorX::RIGHT, CursorY::TOP);
        case CursorDirection::NEXTLINE:
            return PrepData(CursorX::LEFT, CursorY::BOTTOM);
        case CursorDirection::PREVLINE:
            return PrepData(CursorX::LEFT, CursorY::TOP);
        }
    }

    void PrepData(CursorX xact, CursorY yact)
    {
        Log::Comment(L"Resetting mock data state.");

        // APIs succeed by default
        _fSetConsoleCursorPositionResult = TRUE;
        _fGetConsoleScreenBufferInfoExResult = TRUE;
        _fGetConsoleCursorInfoResult = TRUE;
        _fSetConsoleCursorInfoResult = TRUE;
        _fSetConsoleTextAttributeResult = TRUE;
        _fPrivateWriteConsoleInputWResult = TRUE;
        _fPrivatePrependConsoleInputResult = TRUE;
        _fPrivateWriteConsoleControlInputResult = TRUE;
        _fSetConsoleWindowInfoResult = TRUE;
        _fPrivateGetConsoleScreenBufferLegacyAttributesResult = TRUE;
        _fPrivateGetTextAttributesResult = TRUE;
        _fMoveToBottomResult = true;

        _coordBufferSize.X = 100;
        _coordBufferSize.Y = 600;

        // Viewport sitting in the "middle" of the buffer somewhere (so all sides have excess buffer around them)
        _srViewport.Top = 20;
        _srViewport.Bottom = 49;
        _srViewport.Left = 30;
        _srViewport.Right = 59;

        // Call cursor positions seperately
        PrepCursor(xact, yact);

        _dwCursorSize = 33;
        _dwExpectedCursorSize = _dwCursorSize;

        _fCursorVisible = TRUE;
        _fExpectedCursorVisible = _fCursorVisible;

        // Attribute default is gray on black.
        _attribute = {};
        _attribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
        _expectedAttribute = _attribute;

        _expectedLines = 0;
    }

    void PrepCursor(CursorX xact, CursorY yact)
    {
        Log::Comment(L"Adjusting cursor within viewport... Expected will match actual when done.");

        switch (xact)
        {
        case CursorX::LEFT:
            Log::Comment(L"Cursor set to left edge of viewport.");
            _coordCursorPos.X = _srViewport.Left;
            break;
        case CursorX::RIGHT:
            Log::Comment(L"Cursor set to right edge of viewport.");
            _coordCursorPos.X = _srViewport.Right - 1;
            break;
        case CursorX::XCENTER:
            Log::Comment(L"Cursor set to centered X of viewport.");
            _coordCursorPos.X = _srViewport.Left + ((_srViewport.Right - _srViewport.Left) / 2);
            break;
        }

        switch (yact)
        {
        case CursorY::TOP:
            Log::Comment(L"Cursor set to top edge of viewport.");
            _coordCursorPos.Y = _srViewport.Top;
            break;
        case CursorY::BOTTOM:
            Log::Comment(L"Cursor set to bottom edge of viewport.");
            _coordCursorPos.Y = _srViewport.Bottom - 1;
            break;
        case CursorY::YCENTER:
            Log::Comment(L"Cursor set to centered Y of viewport.");
            _coordCursorPos.Y = _srViewport.Top + ((_srViewport.Bottom - _srViewport.Top) / 2);
            break;
        }

        _coordExpectedCursorPos = _coordCursorPos;
    }

    void ValidateInputEvent(_In_ PCWSTR pwszExpectedResponse)
    {
        size_t const cchResponse = wcslen(pwszExpectedResponse);
        size_t const eventCount = _events.size();

        VERIFY_ARE_EQUAL(cchResponse * 2, eventCount, L"We should receive TWO input records for every character in the expected string. Key down and key up.");

        for (size_t iInput = 0; iInput < eventCount; iInput++)
        {
            wchar_t const wch = pwszExpectedResponse[iInput / 2]; // the same portion of the string will be used twice. 0/2 = 0. 1/2 = 0. 2/2 = 1. 3/2 = 1. and so on.

            VERIFY_ARE_EQUAL(InputEventType::KeyEvent, _events[iInput]->EventType());

            const KeyEvent* const keyEvent = static_cast<const KeyEvent* const>(_events[iInput].get());

            // every even key is down. every odd key is up. DOWN = 0, UP = 1. DOWN = 2, UP = 3. and so on.
            VERIFY_ARE_EQUAL((bool)!(iInput % 2), keyEvent->IsKeyDown());
            VERIFY_ARE_EQUAL(0u, keyEvent->GetActiveModifierKeys());
            Log::Comment(NoThrowString().Format(L"Comparing '%c' with '%c'...", wch, keyEvent->GetCharData()));
            VERIFY_ARE_EQUAL(wch, keyEvent->GetCharData());
            VERIFY_ARE_EQUAL(1u, keyEvent->GetRepeatCount());
            VERIFY_ARE_EQUAL(0u, keyEvent->GetVirtualKeyCode());
            VERIFY_ARE_EQUAL(0u, keyEvent->GetVirtualScanCode());
        }
    }

    void _SetMarginsHelper(SMALL_RECT* rect, SHORT top, SHORT bottom)
    {
        rect->Top = top;
        rect->Bottom = bottom;
        //The rectangle is going to get converted from VT space to conhost space
        _srExpectedScrollRegion.Top = (top > 0) ? rect->Top - 1 : rect->Top;
        _srExpectedScrollRegion.Bottom = (bottom > 0) ? rect->Bottom - 1 : rect->Bottom;
    }

    ~TestGetSet()
    {
    }

    static const WCHAR s_wchErase = (WCHAR)0x20;
    static const WCHAR s_wchDefault = L'Z';
    static const WORD s_wAttrErase = FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
    static const WORD s_wDefaultAttribute = 0;
    static const WORD s_wDefaultFill = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED; // dark gray on black.

    std::deque<std::unique_ptr<IInputEvent>> _events;

    COORD _coordBufferSize = { 0, 0 };
    SMALL_RECT _srViewport = { 0, 0, 0, 0 };
    SMALL_RECT _srExpectedConsoleWindow = { 0, 0, 0, 0 };
    COORD _coordCursorPos = { 0, 0 };
    SMALL_RECT _srExpectedScrollRegion = { 0, 0, 0, 0 };

    DWORD _dwCursorSize = 0;
    BOOL _fCursorVisible = false;

    COORD _coordExpectedCursorPos = { 0, 0 };
    DWORD _dwExpectedCursorSize = 0;
    BOOL _fExpectedCursorVisible = false;

    TextAttribute _attribute;
    TextAttribute _expectedAttribute;
    int _iXtermTableEntry = 0;
    int _iExpectedXtermTableEntry = 0;
    COLORREF _rgbColor = 0;
    COLORREF _ExpectedColor = 0;
    bool _fIsForeground = false;
    bool _fExpectedIsForeground = false;
    bool _fUsingRgbColor = false;
    bool _fExpectedForeground = false;
    bool _fExpectedBackground = false;
    bool _fExpectedMeta = false;
    unsigned int _uiExpectedOutputCP = 0;
    bool _fIsPty = false;
    short _expectedLines = 0;
    bool _fPrivateBoldTextResult = false;
    bool _fExpectedIsBold = false;

    bool _privateShowCursorResult = false;
    bool _expectedShowCursor = false;

    BOOL _fGetConsoleScreenBufferInfoExResult = false;
    BOOL _fSetConsoleCursorPositionResult = false;
    BOOL _fGetConsoleCursorInfoResult = false;
    BOOL _fSetConsoleCursorInfoResult = false;
    BOOL _fSetConsoleTextAttributeResult = false;
    BOOL _fPrivateWriteConsoleInputWResult = false;
    BOOL _fPrivatePrependConsoleInputResult = false;
    BOOL _fPrivateWriteConsoleControlInputResult = false;

    BOOL _fSetConsoleWindowInfoResult = false;
    BOOL _fExpectedWindowAbsolute = false;
    BOOL _fSetConsoleScreenBufferInfoExResult = false;

    COORD _coordExpectedScreenBufferSize = { 0, 0 };
    SMALL_RECT _srExpectedScreenBufferViewport{ 0, 0, 0, 0 };
    BOOL _fPrivateSetCursorKeysModeResult = false;
    BOOL _fPrivateSetKeypadModeResult = false;
    bool _fCursorKeysApplicationMode = false;
    bool _fKeypadApplicationMode = false;
    BOOL _fPrivateAllowCursorBlinkingResult = false;
    bool _fEnable = false; // for cursor blinking
    BOOL _fPrivateSetScrollingRegionResult = false;
    BOOL _fPrivateReverseLineFeedResult = false;

    BOOL _fSetConsoleTitleWResult = false;
    wchar_t* _pwchExpectedWindowTitle = nullptr;
    unsigned short _sCchExpectedTitleLength = 0;
    BOOL _fPrivateHorizontalTabSetResult = false;
    BOOL _fPrivateForwardTabResult = false;
    BOOL _fPrivateBackwardsTabResult = false;
    SHORT _sExpectedNumTabs = 0;
    BOOL _fPrivateTabClearResult = false;
    bool _fExpectedClearAll = false;
    bool _fExpectedMouseEnabled = false;
    bool _fExpectedAlternateScrollEnabled = false;
    BOOL _fPrivateEnableVT200MouseModeResult = false;
    BOOL _fPrivateEnableUTF8ExtendedMouseModeResult = false;
    BOOL _fPrivateEnableSGRExtendedMouseModeResult = false;
    BOOL _fPrivateEnableButtonEventMouseModeResult = false;
    BOOL _fPrivateEnableAnyEventMouseModeResult = false;
    BOOL _fPrivateEnableAlternateScrollResult = false;
    BOOL _fSetConsoleXtermTextAttributeResult = false;
    BOOL _fSetConsoleRGBTextAttributeResult = false;
    BOOL _fPrivateSetLegacyAttributesResult = false;
    BOOL _fPrivateSetTextAttributesResult = false;
    BOOL _fPrivateGetConsoleScreenBufferLegacyAttributesResult = false;
    BOOL _fPrivateGetTextAttributesResult = false;
    BOOL _fSetCursorStyleResult = false;
    CursorType _ExpectedCursorStyle;
    BOOL _fSetCursorColorResult = false;
    COLORREF _ExpectedCursorColor = 0;
    BOOL _fGetConsoleOutputCPResult = false;
    BOOL _fIsConsolePtyResult = false;
    bool _fMoveCursorVerticallyResult = false;
    bool _fPrivateSetDefaultAttributesResult = false;
    bool _fMoveToBottomResult = false;

    bool _fPrivateSetColorTableEntryResult = false;
    short _expectedColorTableIndex = -1;
    COLORREF _expectedColorValue = INVALID_COLOR;

    bool _fPrivateSetDefaultForegroundResult = false;
    COLORREF _expectedDefaultForegroundColorValue = INVALID_COLOR;

    bool _fPrivateSetDefaultBackgroundResult = false;
    COLORREF _expectedDefaultBackgroundColorValue = INVALID_COLOR;

private:
    HANDLE _hCon;
};

class DummyAdapter : public AdaptDefaults
{
    void Print(const wchar_t /*wch*/) override
    {
    }

    void PrintString(_In_reads_(_Param_(2)) const wchar_t* const /*rgwch*/, const size_t /*cch*/) override
    {
    }

    void Execute(const wchar_t /*wch*/) override
    {
    }
};

class AdapterTest
{
public:
    TEST_CLASS(AdapterTest);

    TEST_METHOD_SETUP(SetupMethods)
    {
        bool fSuccess = true;

        _testGetSet = new TestGetSet;
        fSuccess = _testGetSet != nullptr;
        if (fSuccess)
        {
            // give AdaptDispatch ownership of _testGetSet
            _pDispatch = new AdaptDispatch(_testGetSet, new DummyAdapter);
            fSuccess = _pDispatch != nullptr;
        }
        return fSuccess;
    }

    TEST_METHOD_CLEANUP(CleanupMethods)
    {
        delete _pDispatch;
        _testGetSet = nullptr;
        return true;
    }

    TEST_METHOD(CursorMovementTest)
    {
        BEGIN_TEST_METHOD_PROPERTIES()
            TEST_METHOD_PROPERTY(L"Data:uiDirection", L"{0, 1, 2, 3, 4, 5}") // These values align with the CursorDirection enum class to try all the directions.
        END_TEST_METHOD_PROPERTIES()

        Log::Comment(L"Starting test...");

        // Used to switch between the various function options.
        typedef bool (AdaptDispatch::*CursorMoveFunc)(unsigned int);
        CursorMoveFunc moveFunc = nullptr;

        // Modify variables based on directionality of this test
        CursorDirection direction;
        unsigned int dir;
        VERIFY_SUCCEEDED_RETURN(TestData::TryGetValue(L"uiDirection", dir));
        direction = (CursorDirection)dir;

        switch (direction)
        {
        case CursorDirection::UP:
            Log::Comment(L"Testing up direction.");
            moveFunc = &AdaptDispatch::CursorUp;
            break;
        case CursorDirection::DOWN:
            Log::Comment(L"Testing down direction.");
            moveFunc = &AdaptDispatch::CursorDown;
            break;
        case CursorDirection::RIGHT:
            Log::Comment(L"Testing right direction.");
            moveFunc = &AdaptDispatch::CursorForward;
            break;
        case CursorDirection::LEFT:
            Log::Comment(L"Testing left direction.");
            moveFunc = &AdaptDispatch::CursorBackward;
            break;
        case CursorDirection::NEXTLINE:
            Log::Comment(L"Testing next line direction.");
            moveFunc = &AdaptDispatch::CursorNextLine;
            break;
        case CursorDirection::PREVLINE:
            Log::Comment(L"Testing prev line direction.");
            moveFunc = &AdaptDispatch::CursorPrevLine;
            break;
        }

        if (moveFunc == nullptr)
        {
            VERIFY_FAIL();
            return;
        }

        // success cases
        // place cursor in top left. moving up is expected to go nowhere (it should get bounded by the viewport)
        Log::Comment(L"Test 1: Cursor doesn't move when placed in corner of viewport.");
        _testGetSet->PrepData(direction);

        switch (direction)
        {
        case CursorDirection::UP:
            Log::Comment(L"Testing up direction.");
            _testGetSet->_expectedLines = -1;
            _testGetSet->_fMoveCursorVerticallyResult = true;
            break;
        case CursorDirection::DOWN:
            Log::Comment(L"Testing down direction.");
            _testGetSet->_expectedLines = 1;
            _testGetSet->_fMoveCursorVerticallyResult = true;
            break;
        default:
            _testGetSet->_expectedLines = 0;
            _testGetSet->_fMoveCursorVerticallyResult = false;
            break;
        }

        VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(1));

        Log::Comment(L"Test 1b: Cursor moves to left of line with next/prev line command when cursor can't move higher/lower.");

        bool fDoTest1b = false;

        switch (direction)
        {
        case CursorDirection::NEXTLINE:
            _testGetSet->PrepData(CursorX::RIGHT, CursorY::BOTTOM);
            fDoTest1b = true;
            break;
        case CursorDirection::PREVLINE:
            _testGetSet->PrepData(CursorX::RIGHT, CursorY::TOP);
            fDoTest1b = true;
            break;
        }

        if (fDoTest1b)
        {
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
            VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(1));
        }
        else
        {
            Log::Comment(L"Test not applicable to direction selected. Skipping.");
        }

        // place cursor lower, move up 1.
        Log::Comment(L"Test 2: Cursor moves 1 in the correct direction from viewport.");
        _testGetSet->PrepData(CursorX::XCENTER, CursorY::YCENTER);

        switch (direction)
        {
        case CursorDirection::UP:
            _testGetSet->_coordExpectedCursorPos.Y--;
            _testGetSet->_expectedLines = -1;
            _testGetSet->_fMoveCursorVerticallyResult = true;
            break;
        case CursorDirection::DOWN:
            _testGetSet->_coordExpectedCursorPos.Y++;
            _testGetSet->_expectedLines = 1;
            _testGetSet->_fMoveCursorVerticallyResult = true;
            break;
        case CursorDirection::RIGHT:
            _testGetSet->_coordExpectedCursorPos.X++;
            break;
        case CursorDirection::LEFT:
            _testGetSet->_coordExpectedCursorPos.X--;
            break;
        case CursorDirection::NEXTLINE:
            _testGetSet->_coordExpectedCursorPos.Y++;
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
            break;
        case CursorDirection::PREVLINE:
            _testGetSet->_coordExpectedCursorPos.Y--;
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
            break;
        }

        VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(1));

        // place cursor and move it up too far. It should get bounded by the viewport.
        Log::Comment(L"Test 3: Cursor moves and gets stuck at viewport when started away from edges and moved beyond edges.");
        _testGetSet->PrepData(CursorX::XCENTER, CursorY::YCENTER);

        // Bottom and right viewports are -1 because those two sides are specified to be 1 outside the viewable area.

        switch (direction)
        {
        case CursorDirection::UP:
            _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Top;
            _testGetSet->_expectedLines = -100;
            _testGetSet->_fMoveCursorVerticallyResult = true;
            break;
        case CursorDirection::DOWN:
            _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Bottom - 1;
            _testGetSet->_expectedLines = 100;
            _testGetSet->_fMoveCursorVerticallyResult = true;
            break;
        case CursorDirection::RIGHT:
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Right - 1;
            break;
        case CursorDirection::LEFT:
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
            break;
        case CursorDirection::NEXTLINE:
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
            _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Bottom - 1;
            break;
        case CursorDirection::PREVLINE:
            _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
            _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Top;
            break;
        }

        VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(100));

        // error cases
        // give too large an up distance, cursor move should fail, cursor should stay the same.
        Log::Comment(L"Test 4: When given invalid (massive) move distance that doesn't fit in a short, call fails and cursor doesn't move.");
        _testGetSet->PrepData(CursorX::XCENTER, CursorY::YCENTER);

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(UINT_MAX));
        VERIFY_ARE_EQUAL(_testGetSet->_coordExpectedCursorPos, _testGetSet->_coordCursorPos);

        // cause short underflow. cursor move should fail. cursor should stay the same.
        Log::Comment(L"Test 5: When an over/underflow occurs in cursor math, call fails and cursor doesn't move.");
        _testGetSet->PrepData(direction);

        switch (direction)
        {
        case CursorDirection::UP:
        case CursorDirection::PREVLINE:
            _testGetSet->_coordCursorPos.Y = -10;
            break;
        case CursorDirection::DOWN:
        case CursorDirection::NEXTLINE:
            _testGetSet->_coordCursorPos.Y = 10;
            break;
        case CursorDirection::RIGHT:
            _testGetSet->_coordCursorPos.X = 10;
            break;
        case CursorDirection::LEFT:
            _testGetSet->_coordCursorPos.X = -10;
            break;
        }

        _testGetSet->_coordExpectedCursorPos = _testGetSet->_coordCursorPos;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(SHRT_MAX + 1));
        VERIFY_ARE_EQUAL(_testGetSet->_coordExpectedCursorPos, _testGetSet->_coordCursorPos);

        // SetConsoleCursorPosition throws failure. Parameters are otherwise normal.
        Log::Comment(L"Test 6: When SetConsoleCursorPosition throws a failure, call fails and cursor doesn't move.");
        _testGetSet->PrepData(direction);
        _testGetSet->_fSetConsoleCursorPositionResult = FALSE;
        _testGetSet->_fMoveCursorVerticallyResult = false;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(0));
        VERIFY_ARE_EQUAL(_testGetSet->_coordExpectedCursorPos, _testGetSet->_coordCursorPos);

        // GetConsoleScreenBufferInfo throws failure. Parameters are otherwise normal.
        Log::Comment(L"Test 7: When GetConsoleScreenBufferInfo throws a failure, call fails and cursor doesn't move.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);
        _testGetSet->_fGetConsoleScreenBufferInfoExResult = FALSE;
        _testGetSet->_fMoveCursorVerticallyResult = true;
        Log::Comment(NoThrowString().Format(
            L"Cursor Up and Down don't need GetConsoleScreenBufferInfoEx, so they will succeed"));
        if (direction == CursorDirection::UP || direction == CursorDirection::DOWN)
        {
            VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(0));
        }
        else
        {
            VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(0));
        }
        VERIFY_ARE_EQUAL(_testGetSet->_coordExpectedCursorPos, _testGetSet->_coordCursorPos);
    }

    TEST_METHOD(CursorPositionTest)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: Place cursor within the viewport. Start from top left, move to middle.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        short sCol = (_testGetSet->_srViewport.Right - _testGetSet->_srViewport.Left) / 2;
        short sRow = (_testGetSet->_srViewport.Bottom - _testGetSet->_srViewport.Top) / 2;

        _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left + (sCol - 1);
        _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Top + (sRow - 1);

        VERIFY_IS_TRUE(_pDispatch->CursorPosition(sRow, sCol));

        Log::Comment(L"Test 2: Move to 0, 0 (which is 1,1 in VT speak)");
        _testGetSet->PrepData(CursorX::RIGHT, CursorY::BOTTOM);

        _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Left;
        _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Top;

        VERIFY_IS_TRUE(_pDispatch->CursorPosition(1, 1));

        Log::Comment(L"Test 3: Move beyond rectangle (down/right too far). Should be bounded back in.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        sCol = (_testGetSet->_srViewport.Right - _testGetSet->_srViewport.Left) * 2;
        sRow = (_testGetSet->_srViewport.Bottom - _testGetSet->_srViewport.Top) * 2;

        _testGetSet->_coordExpectedCursorPos.X = _testGetSet->_srViewport.Right - 1;
        _testGetSet->_coordExpectedCursorPos.Y = _testGetSet->_srViewport.Bottom - 1;

        VERIFY_IS_TRUE(_pDispatch->CursorPosition(sRow, sCol));

        Log::Comment(L"Test 4: Values too large for short. Cursor shouldn't move. Return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        VERIFY_IS_FALSE(_pDispatch->CursorPosition(UINT_MAX, UINT_MAX));

        Log::Comment(L"Test 5: Overflow during addition. Cursor shouldn't move. Return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        _testGetSet->_srViewport.Left = SHRT_MAX;
        _testGetSet->_srViewport.Top = SHRT_MAX;

        VERIFY_IS_FALSE(_pDispatch->CursorPosition(5, 5));

        Log::Comment(L"Test 6: GetConsoleInfo API returns false. No move, return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        _testGetSet->_fGetConsoleScreenBufferInfoExResult = FALSE;

        VERIFY_IS_FALSE(_pDispatch->CursorPosition(1, 1));

        Log::Comment(L"Test 7: SetCursor API returns false. No move, return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        _testGetSet->_fSetConsoleCursorPositionResult = FALSE;

        VERIFY_IS_FALSE(_pDispatch->CursorPosition(1, 1));

        Log::Comment(L"Test 8: Move to 0,0. Cursor shouldn't move. Return false. 1,1 is the top left corner in VT100 speak. 0,0 isn't a position. The parser will give 1 for a 0 input.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        VERIFY_IS_FALSE(_pDispatch->CursorPosition(0, 0));
    }

    TEST_METHOD(CursorSingleDimensionMoveTest)
    {
        BEGIN_TEST_METHOD_PROPERTIES()
            TEST_METHOD_PROPERTY(L"Data:uiDirection", L"{0, 1}") // These values align with the CursorDirection enum class to try all the directions.
        END_TEST_METHOD_PROPERTIES()

        Log::Comment(L"Starting test...");

        //// Used to switch between the various function options.
        typedef bool (AdaptDispatch::*CursorMoveFunc)(unsigned int);
        CursorMoveFunc moveFunc = nullptr;
        SHORT* psViewportEnd = nullptr;
        SHORT* psViewportStart = nullptr;
        SHORT* psCursorExpected = nullptr;

        // Modify variables based on directionality of this test
        AbsolutePosition direction;
        unsigned int dir;
        VERIFY_SUCCEEDED_RETURN(TestData::TryGetValue(L"uiDirection", dir));
        direction = (AbsolutePosition)dir;

        switch (direction)
        {
        case AbsolutePosition::CursorHorizontal:
            Log::Comment(L"Testing cursor horizontal movement.");
            psViewportEnd = &_testGetSet->_srViewport.Right;
            psViewportStart = &_testGetSet->_srViewport.Left;
            psCursorExpected = &_testGetSet->_coordExpectedCursorPos.X;
            moveFunc = &AdaptDispatch::CursorHorizontalPositionAbsolute;
            break;
        case AbsolutePosition::VerticalLine:
            Log::Comment(L"Testing vertical line movement.");
            psViewportEnd = &_testGetSet->_srViewport.Bottom;
            psViewportStart = &_testGetSet->_srViewport.Top;
            psCursorExpected = &_testGetSet->_coordExpectedCursorPos.Y;
            moveFunc = &AdaptDispatch::VerticalLinePositionAbsolute;
            break;
        }

        if (moveFunc == nullptr || psViewportEnd == nullptr || psViewportStart == nullptr || psCursorExpected == nullptr)
        {
            VERIFY_FAIL();
            return;
        }

        Log::Comment(L"Test 1: Place cursor within the viewport. Start from top left, move to middle.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        short sVal = (*psViewportEnd - *psViewportStart) / 2;

        *psCursorExpected = *psViewportStart + (sVal - 1);

        VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 2: Move to 0 (which is 1 in VT speak)");
        _testGetSet->PrepData(CursorX::RIGHT, CursorY::BOTTOM);

        *psCursorExpected = *psViewportStart;
        sVal = 1;

        VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 3: Move beyond rectangle (down/right too far). Should be bounded back in.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        sVal = (*psViewportEnd - *psViewportStart) * 2;

        *psCursorExpected = *psViewportEnd - 1;

        VERIFY_IS_TRUE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 4: Values too large for short. Cursor shouldn't move. Return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        sVal = SHORT_MAX;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 5: Overflow during addition. Cursor shouldn't move. Return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        _testGetSet->_srViewport.Left = SHRT_MAX;

        sVal = 5;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 6: GetConsoleInfo API returns false. No move, return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        _testGetSet->_fGetConsoleScreenBufferInfoExResult = FALSE;

        sVal = 1;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 7: SetCursor API returns false. No move, return false.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        _testGetSet->_fSetConsoleCursorPositionResult = FALSE;

        sVal = 1;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(sVal));

        Log::Comment(L"Test 8: Move to 0. Cursor shouldn't move. Return false. 1 is the left edge in VT100 speak. 0 isn't a position. The parser will give 1 for a 0 input.");
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);

        sVal = 0;

        VERIFY_IS_FALSE((_pDispatch->*(moveFunc))(sVal));
    }

    TEST_METHOD(CursorSaveRestoreTest)
    {
        Log::Comment(L"Starting test...");

        COORD coordExpected = { 0 };

        Log::Comment(L"Test 1: Restore with no saved data should move to top-left corner, the null/default position.");

        // Move cursor to top left and save off expected position.
        _testGetSet->PrepData(CursorX::LEFT, CursorY::TOP);
        coordExpected = _testGetSet->_coordExpectedCursorPos;

        // Then move cursor to the middle and reset the expected to the top left.
        _testGetSet->PrepData(CursorX::XCENTER, CursorY::YCENTER);
        _testGetSet->_coordExpectedCursorPos = coordExpected;
        _testGetSet->_fPrivateSetTextAttributesResult = true;
        _testGetSet->_expectedAttribute = {};

        VERIFY_IS_TRUE(_pDispatch->CursorRestoreState(), L"By default, restore to top left corner (0,0 offset from viewport).");

        Log::Comment(L"Test 2: Place cursor in center. Save. Move cursor to corner. Restore. Should come back to center.");
        _testGetSet->PrepData(CursorX::XCENTER, CursorY::YCENTER);
        VERIFY_IS_TRUE(_pDispatch->CursorSaveState(), L"Succeed at saving position.");

        Log::Comment(L"Backup expected cursor (in the middle). Move cursor to corner. Then re-set expected cursor to middle.");
        // save expected cursor position
        coordExpected = _testGetSet->_coordExpectedCursorPos;

        // adjust cursor to corner
        _testGetSet->PrepData(CursorX::LEFT, CursorY::BOTTOM);

        // restore expected cursor position to center.
        _testGetSet->_coordExpectedCursorPos = coordExpected;

        VERIFY_IS_TRUE(_pDispatch->CursorRestoreState(), L"Restoring to corner should succeed. API call inside will test that cursor matched expected position.");
    }

    TEST_METHOD(CursorHideShowTest)
    {
        BEGIN_TEST_METHOD_PROPERTIES()
            TEST_METHOD_PROPERTY(L"Data:fStartingVis", L"{TRUE, FALSE}")
            TEST_METHOD_PROPERTY(L"Data:fEndingVis", L"{TRUE, FALSE}")
        END_TEST_METHOD_PROPERTIES()

        Log::Comment(L"Starting test...");

        // Modify variables based on permutations of this test.
        bool fStart;
        bool fEnd;
        VERIFY_SUCCEEDED_RETURN(TestData::TryGetValue(L"fStartingVis", fStart));
        VERIFY_SUCCEEDED_RETURN(TestData::TryGetValue(L"fEndingVis", fEnd));

        Log::Comment(L"Test 1: Verify successful API call modifies visibility state.");
        _testGetSet->PrepData();
        _testGetSet->_fCursorVisible = fStart;
        _testGetSet->_privateShowCursorResult = true;
        _testGetSet->_expectedShowCursor = fEnd;
        VERIFY_IS_TRUE(_pDispatch->CursorVisibility(fEnd));

        Log::Comment(L"Test 3: When we fail to set updated cursor information, the dispatch should fail.");
        _testGetSet->PrepData();
        _testGetSet->_privateShowCursorResult = false;
        VERIFY_IS_FALSE(_pDispatch->CursorVisibility(fEnd));
    }

    TEST_METHOD(GraphicsBaseTests)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: Send no options.");

        _testGetSet->PrepData();

        DispatchTypes::GraphicsOptions rgOptions[16];
        size_t cOptions = 0;

        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Test 2: Gracefully fail when getting buffer information fails.");

        _testGetSet->PrepData();
        _testGetSet->_fPrivateGetConsoleScreenBufferLegacyAttributesResult = FALSE;

        VERIFY_IS_FALSE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Test 3: Gracefully fail when setting attribute data fails.");

        _testGetSet->PrepData();
        _testGetSet->_fSetConsoleTextAttributeResult = FALSE;
        // Need at least one option in order for the call to be able to fail.
        rgOptions[0] = (DispatchTypes::GraphicsOptions)0;
        cOptions = 1;
        VERIFY_IS_FALSE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
    }

    TEST_METHOD(GraphicsSingleTests)
    {
        BEGIN_TEST_METHOD_PROPERTIES()
            TEST_METHOD_PROPERTY(L"Data:uiGraphicsOptions", L"{0, 1, 4, 7, 24, 27, 30, 31, 32, 33, 34, 35, 36, 37, 39, 40, 41, 42, 43, 44, 45, 46, 47, 49, 90, 91, 92, 93, 94, 95, 96, 97, 100, 101, 102, 103, 104, 105, 106, 107}") // corresponds to options in DispatchTypes::GraphicsOptions
        END_TEST_METHOD_PROPERTIES()

        Log::Comment(L"Starting test...");
        _testGetSet->PrepData();

        // Modify variables based on type of this test
        DispatchTypes::GraphicsOptions graphicsOption;
        unsigned int uiGraphicsOption;
        VERIFY_SUCCEEDED_RETURN(TestData::TryGetValue(L"uiGraphicsOptions", uiGraphicsOption));
        graphicsOption = (DispatchTypes::GraphicsOptions)uiGraphicsOption;

        DispatchTypes::GraphicsOptions rgOptions[16];
        size_t cOptions = 1;
        rgOptions[0] = graphicsOption;
        WORD expectedLegacy = 0;

        _testGetSet->_fPrivateSetLegacyAttributesResult = TRUE;

        switch (graphicsOption)
        {
        case DispatchTypes::GraphicsOptions::Off:
            Log::Comment(L"Testing graphics 'Off/Reset'");
            _testGetSet->_attribute.SetFromLegacy((WORD)~_testGetSet->s_wDefaultFill);
            _testGetSet->_expectedAttribute.SetFromLegacy(0);
            _testGetSet->_fPrivateSetDefaultAttributesResult = true;
            _testGetSet->_fExpectedForeground = true;
            _testGetSet->_fExpectedBackground = true;
            _testGetSet->_fExpectedMeta = true;
            _testGetSet->_fPrivateBoldTextResult = true;
            _testGetSet->_fExpectedIsBold = false;

            break;
        case DispatchTypes::GraphicsOptions::BoldBright:
            Log::Comment(L"Testing graphics 'Bold/Bright'");
            _testGetSet->_attribute.SetFromLegacy(0);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY);
            _testGetSet->_fExpectedForeground = true;
            _testGetSet->_fPrivateBoldTextResult = true;
            _testGetSet->_fExpectedIsBold = true;
            break;
        case DispatchTypes::GraphicsOptions::Underline:
            Log::Comment(L"Testing graphics 'Underline'");
            _testGetSet->_attribute.SetFromLegacy(0);
            _testGetSet->_expectedAttribute.SetFromLegacy(COMMON_LVB_UNDERSCORE);
            _testGetSet->_fExpectedMeta = true;
            break;
        case DispatchTypes::GraphicsOptions::Negative:
            Log::Comment(L"Testing graphics 'Negative'");
            _testGetSet->_attribute.SetFromLegacy(0);
            _testGetSet->_expectedAttribute.SetFromLegacy(COMMON_LVB_REVERSE_VIDEO);
            _testGetSet->_fExpectedMeta = true;
            break;
        case DispatchTypes::GraphicsOptions::NoUnderline:
            Log::Comment(L"Testing graphics 'No Underline'");
            _testGetSet->_attribute.SetFromLegacy(COMMON_LVB_UNDERSCORE);
            _testGetSet->_expectedAttribute.SetFromLegacy(0);
            _testGetSet->_fExpectedMeta = true;
            break;
        case DispatchTypes::GraphicsOptions::Positive:
            Log::Comment(L"Testing graphics 'Positive'");
            _testGetSet->_attribute.SetFromLegacy(COMMON_LVB_REVERSE_VIDEO);
            _testGetSet->_expectedAttribute.SetFromLegacy(0);
            _testGetSet->_fExpectedMeta = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundBlack:
            Log::Comment(L"Testing graphics 'Foreground Color Black'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(0);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundBlue:
            Log::Comment(L"Testing graphics 'Foreground Color Blue'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundGreen:
            Log::Comment(L"Testing graphics 'Foreground Color Green'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundCyan:
            Log::Comment(L"Testing graphics 'Foreground Color Cyan'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_GREEN);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundRed:
            Log::Comment(L"Testing graphics 'Foreground Color Red'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundMagenta:
            Log::Comment(L"Testing graphics 'Foreground Color Magenta'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundYellow:
            Log::Comment(L"Testing graphics 'Foreground Color Yellow'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundWhite:
            Log::Comment(L"Testing graphics 'Foreground Color White'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::ForegroundDefault:
            Log::Comment(L"Testing graphics 'Foreground Color Default'");
            _testGetSet->_fPrivateSetDefaultAttributesResult = true;
            _testGetSet->_attribute.SetFromLegacy((WORD)~_testGetSet->s_wDefaultAttribute); // set the current attribute to the opposite of default so we can ensure all relevant bits flip.
            // To get expected value, take what we started with and change ONLY the background series of bits to what the Default says.
            expectedLegacy = _testGetSet->_attribute.GetLegacyAttributes(); // expect = starting
            expectedLegacy &= ~(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY); // turn off all bits related to the background
            expectedLegacy |= (_testGetSet->s_wDefaultFill & (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)); // reapply ONLY background bits from the default attribute.
            _testGetSet->_expectedAttribute.SetFromLegacy(expectedLegacy);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundBlack:
            Log::Comment(L"Testing graphics 'Background Color Black'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(0);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundBlue:
            Log::Comment(L"Testing graphics 'Background Color Blue'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_BLUE);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundGreen:
            Log::Comment(L"Testing graphics 'Background Color Green'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_GREEN);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundCyan:
            Log::Comment(L"Testing graphics 'Background Color Cyan'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_BLUE | BACKGROUND_GREEN);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundRed:
            Log::Comment(L"Testing graphics 'Background Color Red'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundMagenta:
            Log::Comment(L"Testing graphics 'Background Color Magenta'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_GREEN | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_BLUE | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundYellow:
            Log::Comment(L"Testing graphics 'Background Color Yellow'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_BLUE | BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_GREEN | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundWhite:
            Log::Comment(L"Testing graphics 'Background Color White'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_INTENSITY);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BackgroundDefault:
            Log::Comment(L"Testing graphics 'Background Color Default'");
            _testGetSet->_fPrivateSetDefaultAttributesResult = true;
            _testGetSet->_attribute.SetFromLegacy((WORD)~_testGetSet->s_wDefaultAttribute); // set the current attribute to the opposite of default so we can ensure all relevant bits flip.
            // To get expected value, take what we started with and change ONLY the background series of bits to what the Default says.
            expectedLegacy = _testGetSet->_attribute.GetLegacyAttributes(); // expect = starting
            expectedLegacy &= ~(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY); // turn off all bits related to the background
            expectedLegacy |= (_testGetSet->s_wDefaultFill & (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)); // reapply ONLY background bits from the default attribute.
            _testGetSet->_expectedAttribute.SetFromLegacy(expectedLegacy);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundBlack:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Black'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundBlue:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Blue'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_GREEN);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_BLUE);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundGreen:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Green'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_BLUE);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundCyan:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Cyan'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_RED);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundRed:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Red'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_GREEN);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundMagenta:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Magenta'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_GREEN);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundYellow:
            Log::Comment(L"Testing graphics 'Bright Foreground Color Yellow'");
            _testGetSet->_attribute.SetFromLegacy(FOREGROUND_BLUE);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightForegroundWhite:
            Log::Comment(L"Testing graphics 'Bright Foreground Color White'");
            _testGetSet->_attribute.SetFromLegacy(0);
            _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            _testGetSet->_fExpectedForeground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundBlack:
            Log::Comment(L"Testing graphics 'Bright Background Color Black'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundBlue:
            Log::Comment(L"Testing graphics 'Bright Background Color Blue'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_GREEN);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_BLUE);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundGreen:
            Log::Comment(L"Testing graphics 'Bright Background Color Green'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED | BACKGROUND_BLUE);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_GREEN);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundCyan:
            Log::Comment(L"Testing graphics 'Bright Background Color Cyan'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_RED);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundRed:
            Log::Comment(L"Testing graphics 'Bright Background Color Red'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_BLUE | BACKGROUND_GREEN);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundMagenta:
            Log::Comment(L"Testing graphics 'Bright Background Color Magenta'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_GREEN);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundYellow:
            Log::Comment(L"Testing graphics 'Bright Background Color Yellow'");
            _testGetSet->_attribute.SetFromLegacy(BACKGROUND_BLUE);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        case DispatchTypes::GraphicsOptions::BrightBackgroundWhite:
            Log::Comment(L"Testing graphics 'Bright Background Color White'");
            _testGetSet->_attribute.SetFromLegacy(0);
            _testGetSet->_expectedAttribute.SetFromLegacy(BACKGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
            _testGetSet->_fExpectedBackground = true;
            break;
        default:
            VERIFY_FAIL(L"Test not implemented yet!");
            break;
        }

        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
    }

    TEST_METHOD(GraphicsPushPopTests)
    {
        Log::Comment(L"Starting test...");

        _testGetSet->PrepData(); // default color from here is gray on black, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

        _testGetSet->_fPrivateSetLegacyAttributesResult = TRUE;

        DispatchTypes::GraphicsOptions rgOptions[16];
        DispatchTypes::SgrSaveRestoreStackOptions rgStackOptions[16];
        size_t cOptions = 1;

        Log::Comment(L"Test 1: Basic push and pop");

        rgOptions[0] = DispatchTypes::GraphicsOptions::Off;
        _testGetSet->_fPrivateSetDefaultAttributesResult = true;
        _testGetSet->_expectedAttribute.SetFromLegacy(0);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = true;
        _testGetSet->_fExpectedMeta = true;
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        cOptions = 0;
        VERIFY_IS_TRUE(_pDispatch->PushGraphicsRendition(rgStackOptions, cOptions));

        _testGetSet->_fPrivateGetTextAttributesResult = true;
        _testGetSet->_fPrivateSetTextAttributesResult = true;
        _testGetSet->_expectedAttribute.SetDefaultBackground();
        _testGetSet->_expectedAttribute.SetDefaultForeground();
        VERIFY_IS_TRUE(_pDispatch->PopGraphicsRendition());

        Log::Comment(L"Test 2: Push, change color, pop");

        VERIFY_IS_TRUE(_pDispatch->PushGraphicsRendition(rgStackOptions, cOptions));

        cOptions = 1;
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundCyan;
        _testGetSet->_expectedAttribute.SetFromLegacy(3);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = false;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        cOptions = 0;
        _testGetSet->_fPrivateGetTextAttributesResult = true;
        _testGetSet->_fPrivateSetTextAttributesResult = true;
        _testGetSet->_expectedAttribute.SetDefaultBackground();
        _testGetSet->_expectedAttribute.SetDefaultForeground();
        VERIFY_IS_TRUE(_pDispatch->PopGraphicsRendition());

        Log::Comment(L"Test 3: two pushes (nested) and pops");

        // First push:
        VERIFY_IS_TRUE(_pDispatch->PushGraphicsRendition(rgStackOptions, cOptions));

        cOptions = 1;
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundRed;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_RED);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = false;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        // Second push:
        cOptions = 0;
        VERIFY_IS_TRUE(_pDispatch->PushGraphicsRendition(rgStackOptions, cOptions));

        cOptions = 1;
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundGreen;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = false;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        // First pop:
        cOptions = 0;
        _testGetSet->_fPrivateGetTextAttributesResult = true;
        _testGetSet->_fPrivateSetTextAttributesResult = true;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_RED);
        // Q: The verification of the original SGR doesn't have to set the expected
        //    background to default; why do we need it for the pop?
        // A: The pop code path restores the entire TextAttribute, not just a legacy WORD,
        //    so the full TextAttribute is compared against the full expected
        //    TextAttribute (as opposed to just a WORD for the SGR path).
        _testGetSet->_expectedAttribute.SetDefaultBackground();
        VERIFY_IS_TRUE(_pDispatch->PopGraphicsRendition());

        // Second pop:
        cOptions = 0;
        _testGetSet->_fPrivateGetTextAttributesResult = true;
        _testGetSet->_fPrivateSetTextAttributesResult = true;
        _testGetSet->_expectedAttribute.SetDefaultBackground();
        _testGetSet->_expectedAttribute.SetDefaultForeground();
        VERIFY_IS_TRUE(_pDispatch->PopGraphicsRendition());

        Log::Comment(L"Test 4: Save and restore partial attributes");

        cOptions = 1;
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundGreen;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = false;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        cOptions = 1;
        rgOptions[0] = DispatchTypes::GraphicsOptions::BoldBright;
        // N.B. _expectedAttribute will not be checked for the BoldBright SGR.
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        rgOptions[0] = DispatchTypes::GraphicsOptions::BackgroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE);
        _testGetSet->_fExpectedForeground = false;
        _testGetSet->_fExpectedBackground = true;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        // Push, specifying that we only want to save the background, the boldness, and double-underline-ness:
        cOptions = 3;
        rgStackOptions[0] = DispatchTypes::SgrSaveRestoreStackOptions::Boldness;
        rgStackOptions[1] = DispatchTypes::SgrSaveRestoreStackOptions::SaveBackgroundColor;
        rgStackOptions[2] = DispatchTypes::SgrSaveRestoreStackOptions::DoublyUnderlined;
        VERIFY_IS_TRUE(_pDispatch->PushGraphicsRendition(rgStackOptions, cOptions));

        // Now change everything...
        cOptions = 2;
        rgOptions[0] = DispatchTypes::GraphicsOptions::BackgroundGreen;
        rgOptions[1] = DispatchTypes::GraphicsOptions::DoublyUnderlined;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_GREEN);
        _testGetSet->_expectedAttribute.SetDoublyUnderlined(true);
        _testGetSet->_fExpectedForeground = false;
        _testGetSet->_fExpectedBackground = true;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        cOptions = 1;
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundRed;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_RED | BACKGROUND_GREEN);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = false;
        _testGetSet->_fExpectedMeta = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        rgOptions[0] = DispatchTypes::GraphicsOptions::UnBold;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        // And then restore...
        cOptions = 0;
        _testGetSet->_fPrivateGetTextAttributesResult = true;
        _testGetSet->_fPrivateSetTextAttributesResult = true;
        // Q: Why don't we set FOREGROUND_INTENSITY here?
        // A: That flag is folded in on the fly when you call GetLegacyAttributes based on
        //    the _isBold member, but it isn't actually stored in the legacy WORD.
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_RED | BACKGROUND_BLUE);
        _testGetSet->_expectedAttribute.Embolden();
        _testGetSet->_expectedAttribute.SetDoublyUnderlined(false);
        VERIFY_IS_TRUE(_pDispatch->PopGraphicsRendition());
    }

    TEST_METHOD(GraphicsPersistBrightnessTests)
    {
        Log::Comment(L"Starting test...");

        _testGetSet->PrepData(); // default color from here is gray on black, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

        _testGetSet->_fPrivateSetLegacyAttributesResult = TRUE;

        DispatchTypes::GraphicsOptions rgOptions[16];
        size_t cOptions = 1;

        Log::Comment(L"Test 1: Basic brightness test");
        Log::Comment(L"Reseting graphics options");
        rgOptions[0] = DispatchTypes::GraphicsOptions::Off;
        _testGetSet->_fPrivateSetDefaultAttributesResult = true;
        _testGetSet->_expectedAttribute.SetFromLegacy(0);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = true;
        _testGetSet->_fExpectedMeta = true;
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Testing graphics 'Foreground Color Blue'");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Enabling brightness");
        rgOptions[0] = DispatchTypes::GraphicsOptions::BoldBright;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Green, with brightness'");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundGreen;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(WI_IsFlagSet(_testGetSet->_attribute.GetLegacyAttributes(), FOREGROUND_GREEN));
        VERIFY_IS_TRUE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Test 2: Disable brightness, use a bright color, next normal call remains not bright");
        Log::Comment(L"Reseting graphics options");
        rgOptions[0] = DispatchTypes::GraphicsOptions::Off;
        _testGetSet->_fPrivateSetDefaultAttributesResult = true;
        _testGetSet->_expectedAttribute.SetFromLegacy(0);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = true;
        _testGetSet->_fExpectedMeta = true;
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(WI_IsFlagClear(_testGetSet->_attribute.GetLegacyAttributes(), FOREGROUND_INTENSITY));
        VERIFY_IS_FALSE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Bright Blue'");
        rgOptions[0] = DispatchTypes::GraphicsOptions::BrightForegroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_FALSE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Blue', brightness of 9x series doesn't persist");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_FALSE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Test 3: Enable brightness, use a bright color, brightness persists to next normal call");
        Log::Comment(L"Reseting graphics options");
        rgOptions[0] = DispatchTypes::GraphicsOptions::Off;
        _testGetSet->_fPrivateSetDefaultAttributesResult = true;
        _testGetSet->_expectedAttribute.SetFromLegacy(0);
        _testGetSet->_fExpectedForeground = true;
        _testGetSet->_fExpectedBackground = true;
        _testGetSet->_fExpectedMeta = true;
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_FALSE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Blue'");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_FALSE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Enabling brightness");
        rgOptions[0] = DispatchTypes::GraphicsOptions::BoldBright;
        _testGetSet->_fPrivateBoldTextResult = true;
        _testGetSet->_fExpectedIsBold = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Bright Blue'");
        rgOptions[0] = DispatchTypes::GraphicsOptions::BrightForegroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Blue, with brightness', brightness of 9x series doesn't affect brightness");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundBlue;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_BLUE);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(_testGetSet->_attribute.IsBold());

        Log::Comment(L"Testing graphics 'Foreground Color Green, with brightness'");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundGreen;
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN);
        _testGetSet->_fExpectedForeground = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
        VERIFY_IS_TRUE(_testGetSet->_attribute.IsBold());
    }

    TEST_METHOD(DeviceStatusReportTests)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: Verify failure when using bad status.");
        _testGetSet->PrepData();
        VERIFY_IS_FALSE(_pDispatch->DeviceStatusReport((DispatchTypes::AnsiStatusType)-1));
    }

    TEST_METHOD(DeviceStatus_CursorPositionReportTests)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: Verify normal cursor response position.");
        _testGetSet->PrepData(CursorX::XCENTER, CursorY::YCENTER);

        // start with the cursor position in the buffer.
        COORD coordCursorExpected = _testGetSet->_coordCursorPos;

        // to get to VT, we have to adjust it to its position relative to the viewport.
        coordCursorExpected.X -= _testGetSet->_srViewport.Left;
        coordCursorExpected.Y -= _testGetSet->_srViewport.Top;

        // Then note that VT is 1,1 based for the top left, so add 1. (The rest of the console uses 0,0 for array index bases.)
        coordCursorExpected.X++;
        coordCursorExpected.Y++;

        VERIFY_IS_TRUE(_pDispatch->DeviceStatusReport(DispatchTypes::AnsiStatusType::CPR_CursorPositionReport));

        wchar_t pwszBuffer[50];

        swprintf_s(pwszBuffer, ARRAYSIZE(pwszBuffer), L"\x1b[%d;%dR", coordCursorExpected.Y, coordCursorExpected.X);
        _testGetSet->ValidateInputEvent(pwszBuffer);
    }

    TEST_METHOD(DeviceAttributesTests)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: Verify normal response.");
        _testGetSet->PrepData();
        VERIFY_IS_TRUE(_pDispatch->DeviceAttributes());

        PCWSTR pwszExpectedResponse = L"\x1b[?1;0c";
        _testGetSet->ValidateInputEvent(pwszExpectedResponse);

        Log::Comment(L"Test 2: Verify failure when WriteConsoleInput doesn't work.");
        _testGetSet->PrepData();
        _testGetSet->_fPrivatePrependConsoleInputResult = FALSE;

        VERIFY_IS_FALSE(_pDispatch->DeviceAttributes());
    }

    TEST_METHOD(CursorKeysModeTest)
    {
        Log::Comment(L"Starting test...");

        // success cases
        // set numeric mode = true
        Log::Comment(L"Test 1: application mode = false");
        _testGetSet->_fPrivateSetCursorKeysModeResult = TRUE;
        _testGetSet->_fCursorKeysApplicationMode = false;

        VERIFY_IS_TRUE(_pDispatch->SetCursorKeysMode(false));

        // set numeric mode = false
        Log::Comment(L"Test 2: application mode = true");
        _testGetSet->_fPrivateSetCursorKeysModeResult = TRUE;
        _testGetSet->_fCursorKeysApplicationMode = true;

        VERIFY_IS_TRUE(_pDispatch->SetCursorKeysMode(true));
    }

    TEST_METHOD(KeypadModeTest)
    {
        Log::Comment(L"Starting test...");

        // success cases
        // set numeric mode = true
        Log::Comment(L"Test 1: application mode = false");
        _testGetSet->_fPrivateSetKeypadModeResult = TRUE;
        _testGetSet->_fKeypadApplicationMode = false;

        VERIFY_IS_TRUE(_pDispatch->SetKeypadMode(false));

        // set numeric mode = false
        Log::Comment(L"Test 2: application mode = true");
        _testGetSet->_fPrivateSetKeypadModeResult = TRUE;
        _testGetSet->_fKeypadApplicationMode = true;

        VERIFY_IS_TRUE(_pDispatch->SetKeypadMode(true));
    }

    TEST_METHOD(AllowBlinkingTest)
    {
        Log::Comment(L"Starting test...");

        // success cases
        // set numeric mode = true
        Log::Comment(L"Test 1: enable blinking = true");
        _testGetSet->_fPrivateAllowCursorBlinkingResult = TRUE;
        _testGetSet->_fEnable = true;

        VERIFY_IS_TRUE(_pDispatch->EnableCursorBlinking(true));

        // set numeric mode = false
        Log::Comment(L"Test 2: enable blinking = false");
        _testGetSet->_fPrivateAllowCursorBlinkingResult = TRUE;
        _testGetSet->_fEnable = false;

        VERIFY_IS_TRUE(_pDispatch->EnableCursorBlinking(false));
    }

    TEST_METHOD(ScrollMarginsTest)
    {
        Log::Comment(L"Starting test...");

        SMALL_RECT srTestMargins = { 0 };
        _testGetSet->_srViewport.Right = 8;
        _testGetSet->_srViewport.Bottom = 8;
        _testGetSet->_fGetConsoleScreenBufferInfoExResult = TRUE;
        SHORT sScreenHeight = _testGetSet->_srViewport.Bottom - _testGetSet->_srViewport.Top;

        Log::Comment(L"Test 1: Verify having both values is valid.");
        _testGetSet->_SetMarginsHelper(&srTestMargins, 2, 6);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        _testGetSet->_fSetConsoleCursorPositionResult = true;
        _testGetSet->_fMoveToBottomResult = true;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 2: Verify having only top is valid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, 7, 0);
        _testGetSet->_srExpectedScrollRegion.Bottom = _testGetSet->_srViewport.Bottom - 1; // We expect the bottom to be the bottom of the viewport, exclusive.
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 3: Verify having only bottom is valid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, 0, 7);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 4: Verify having no values is valid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, 0, 0);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 5: Verify having both values, but bad bounds is invalid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, 7, 3);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_FALSE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 6: Verify setting margins to (0, height) clears them");
        // First set,
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        _testGetSet->_SetMarginsHelper(&srTestMargins, 2, 6);
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));
        // Then clear
        _testGetSet->_SetMarginsHelper(&srTestMargins, 0, sScreenHeight);
        _testGetSet->_srExpectedScrollRegion.Top = 0;
        _testGetSet->_srExpectedScrollRegion.Bottom = 0;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 7: Verify setting margins to (1, height) clears them");
        // First set,
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        _testGetSet->_SetMarginsHelper(&srTestMargins, 2, 6);
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));
        // Then clear
        _testGetSet->_SetMarginsHelper(&srTestMargins, 1, sScreenHeight);
        _testGetSet->_srExpectedScrollRegion.Top = 0;
        _testGetSet->_srExpectedScrollRegion.Bottom = 0;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 8: Verify setting margins to (1, 0) clears them");
        // First set,
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        _testGetSet->_SetMarginsHelper(&srTestMargins, 2, 6);
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));
        // Then clear
        _testGetSet->_SetMarginsHelper(&srTestMargins, 1, 0);
        _testGetSet->_srExpectedScrollRegion.Top = 0;
        _testGetSet->_srExpectedScrollRegion.Bottom = 0;
        VERIFY_IS_TRUE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 9: Verify having top and bottom margin the same is invalid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, 4, 4);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_FALSE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 10: Verify having top margin out of bounds is invalid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, sScreenHeight + 1, sScreenHeight + 10);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_FALSE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));

        Log::Comment(L"Test 11: Verify having bottom margin out of bounds is invalid.");

        _testGetSet->_SetMarginsHelper(&srTestMargins, 1, sScreenHeight + 1);
        _testGetSet->_fPrivateSetScrollingRegionResult = TRUE;
        VERIFY_IS_FALSE(_pDispatch->SetTopBottomScrollingMargins(srTestMargins.Top, srTestMargins.Bottom));
    }

    TEST_METHOD(TabSetClearTests)
    {
        Log::Comment(L"Starting test...");

        _testGetSet->_fPrivateHorizontalTabSetResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->HorizontalTabSet());

        _testGetSet->_sExpectedNumTabs = 16;

        _testGetSet->_fPrivateForwardTabResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->ForwardTab(16));

        _testGetSet->_fPrivateBackwardsTabResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->BackwardsTab(16));

        _testGetSet->_fPrivateTabClearResult = TRUE;
        _testGetSet->_fExpectedClearAll = true;
        VERIFY_IS_TRUE(_pDispatch->TabClear(DispatchTypes::TabClearType::ClearAllColumns));

        _testGetSet->_fExpectedClearAll = false;
        VERIFY_IS_TRUE(_pDispatch->TabClear(DispatchTypes::TabClearType::ClearCurrentColumn));
    }

    TEST_METHOD(SetConsoleTitleTest)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: set title to be non-null");
        _testGetSet->_fSetConsoleTitleWResult = TRUE;
        wchar_t* pwchTestString = L"Foo bar";
        _testGetSet->_pwchExpectedWindowTitle = pwchTestString;
        _testGetSet->_sCchExpectedTitleLength = 8;

        VERIFY_IS_TRUE(_pDispatch->SetWindowTitle({ pwchTestString, 8 }));

        Log::Comment(L"Test 2: set title to be null");
        _testGetSet->_fSetConsoleTitleWResult = FALSE;
        _testGetSet->_pwchExpectedWindowTitle = nullptr;

        VERIFY_IS_TRUE(_pDispatch->SetWindowTitle({}));
    }

    TEST_METHOD(TestMouseModes)
    {
        Log::Comment(L"Starting test...");

        Log::Comment(L"Test 1: Test Default Mouse Mode");
        _testGetSet->_fExpectedMouseEnabled = true;
        _testGetSet->_fPrivateEnableVT200MouseModeResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->EnableVT200MouseMode(true));
        _testGetSet->_fExpectedMouseEnabled = false;
        VERIFY_IS_TRUE(_pDispatch->EnableVT200MouseMode(false));

        Log::Comment(L"Test 2: Test UTF-8 Extended Mouse Mode");
        _testGetSet->_fExpectedMouseEnabled = true;
        _testGetSet->_fPrivateEnableUTF8ExtendedMouseModeResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->EnableUTF8ExtendedMouseMode(true));
        _testGetSet->_fExpectedMouseEnabled = false;
        VERIFY_IS_TRUE(_pDispatch->EnableUTF8ExtendedMouseMode(false));

        Log::Comment(L"Test 3: Test SGR Extended Mouse Mode");
        _testGetSet->_fExpectedMouseEnabled = true;
        _testGetSet->_fPrivateEnableSGRExtendedMouseModeResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->EnableSGRExtendedMouseMode(true));
        _testGetSet->_fExpectedMouseEnabled = false;
        VERIFY_IS_TRUE(_pDispatch->EnableSGRExtendedMouseMode(false));

        Log::Comment(L"Test 4: Test Button-Event Mouse Mode");
        _testGetSet->_fExpectedMouseEnabled = true;
        _testGetSet->_fPrivateEnableButtonEventMouseModeResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->EnableButtonEventMouseMode(true));
        _testGetSet->_fExpectedMouseEnabled = false;
        VERIFY_IS_TRUE(_pDispatch->EnableButtonEventMouseMode(false));

        Log::Comment(L"Test 5: Test Any-Event Mouse Mode");
        _testGetSet->_fExpectedMouseEnabled = true;
        _testGetSet->_fPrivateEnableAnyEventMouseModeResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->EnableAnyEventMouseMode(true));
        _testGetSet->_fExpectedMouseEnabled = false;
        VERIFY_IS_TRUE(_pDispatch->EnableAnyEventMouseMode(false));

        Log::Comment(L"Test 6: Test Alt Scroll Mouse Mode");
        _testGetSet->_fExpectedAlternateScrollEnabled = true;
        _testGetSet->_fPrivateEnableAlternateScrollResult = TRUE;
        VERIFY_IS_TRUE(_pDispatch->EnableAlternateScroll(true));
        _testGetSet->_fExpectedAlternateScrollEnabled = false;
        VERIFY_IS_TRUE(_pDispatch->EnableAlternateScroll(false));
    }

    TEST_METHOD(Xterm256ColorTest)
    {
        Log::Comment(L"Starting test...");

        _testGetSet->PrepData(); // default color from here is gray on black, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

        DispatchTypes::GraphicsOptions rgOptions[16];
        size_t cOptions = 3;

        _testGetSet->_fSetConsoleXtermTextAttributeResult = true;

        Log::Comment(L"Test 1: Change Foreground");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundExtended;
        rgOptions[1] = DispatchTypes::GraphicsOptions::BlinkOrXterm256Index;
        rgOptions[2] = (DispatchTypes::GraphicsOptions)2; // Green
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN);
        _testGetSet->_iExpectedXtermTableEntry = 2;
        _testGetSet->_fExpectedIsForeground = true;
        _testGetSet->_fUsingRgbColor = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Test 2: Change Background");
        rgOptions[0] = DispatchTypes::GraphicsOptions::BackgroundExtended;
        rgOptions[1] = DispatchTypes::GraphicsOptions::BlinkOrXterm256Index;
        rgOptions[2] = (DispatchTypes::GraphicsOptions)9; // Bright Red
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
        _testGetSet->_iExpectedXtermTableEntry = 9;
        _testGetSet->_fExpectedIsForeground = false;
        _testGetSet->_fUsingRgbColor = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Test 3: Change Foreground to RGB color");
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundExtended;
        rgOptions[1] = DispatchTypes::GraphicsOptions::BlinkOrXterm256Index;
        rgOptions[2] = (DispatchTypes::GraphicsOptions)42; // Arbitrary Color
        _testGetSet->_iExpectedXtermTableEntry = 42;
        _testGetSet->_fExpectedIsForeground = true;
        _testGetSet->_fUsingRgbColor = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Test 4: Change Background to RGB color");
        rgOptions[0] = DispatchTypes::GraphicsOptions::BackgroundExtended;
        rgOptions[1] = DispatchTypes::GraphicsOptions::BlinkOrXterm256Index;
        rgOptions[2] = (DispatchTypes::GraphicsOptions)142; // Arbitrary Color
        _testGetSet->_iExpectedXtermTableEntry = 142;
        _testGetSet->_fExpectedIsForeground = false;
        _testGetSet->_fUsingRgbColor = true;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));

        Log::Comment(L"Test 5: Change Foreground to Legacy Attr while BG is RGB color");
        // Unfortunately this test isn't all that good, because the adapterTest adapter isn't smart enough
        //   to have its own color table and translate the pre-existing RGB BG into a legacy BG.
        // Fortunately, the ft_api:RgbColorTests IS smart enough to test that.
        rgOptions[0] = DispatchTypes::GraphicsOptions::ForegroundExtended;
        rgOptions[1] = DispatchTypes::GraphicsOptions::BlinkOrXterm256Index;
        rgOptions[2] = (DispatchTypes::GraphicsOptions)9; // Bright Red
        _testGetSet->_expectedAttribute.SetFromLegacy(FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_INTENSITY);
        _testGetSet->_iExpectedXtermTableEntry = 9;
        _testGetSet->_fExpectedIsForeground = true;
        _testGetSet->_fUsingRgbColor = false;
        VERIFY_IS_TRUE(_pDispatch->SetGraphicsRendition(rgOptions, cOptions));
    }

    TEST_METHOD(SetColorTableValue)
    {
        _testGetSet->PrepData();

        _testGetSet->_fPrivateSetColorTableEntryResult = true;
        const auto testColor = RGB(1, 2, 3);
        _testGetSet->_expectedColorValue = testColor;

        _testGetSet->_expectedColorTableIndex = 0; // Windows DARK_BLACK
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(0, testColor));

        _testGetSet->_expectedColorTableIndex = 4; // Windows DARK_RED
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(1, testColor));

        _testGetSet->_expectedColorTableIndex = 2; // Windows DARK_GREEN
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(2, testColor));

        _testGetSet->_expectedColorTableIndex = 6; // Windows DARK_YELLOW
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(3, testColor));

        _testGetSet->_expectedColorTableIndex = 1; // Windows DARK_BLUE
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(4, testColor));

        _testGetSet->_expectedColorTableIndex = 5; // Windows DARK_MAGENTA
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(5, testColor));

        _testGetSet->_expectedColorTableIndex = 3; // Windows DARK_CYAN
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(6, testColor));

        _testGetSet->_expectedColorTableIndex = 7; // Windows DARK_WHITE
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(7, testColor));

        _testGetSet->_expectedColorTableIndex = 8; // Windows BRIGHT_BLACK
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(8, testColor));

        _testGetSet->_expectedColorTableIndex = 12; // Windows BRIGHT_RED
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(9, testColor));

        _testGetSet->_expectedColorTableIndex = 10; // Windows BRIGHT_GREEN
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(10, testColor));

        _testGetSet->_expectedColorTableIndex = 14; // Windows BRIGHT_YELLOW
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(11, testColor));

        _testGetSet->_expectedColorTableIndex = 9; // Windows BRIGHT_BLUE
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(12, testColor));

        _testGetSet->_expectedColorTableIndex = 13; // Windows BRIGHT_MAGENTA
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(13, testColor));

        _testGetSet->_expectedColorTableIndex = 11; // Windows BRIGHT_CYAN
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(14, testColor));

        _testGetSet->_expectedColorTableIndex = 15; // Windows BRIGHT_WHITE
        VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(15, testColor));

        for (short i = 16; i < 256; i++)
        {
            _testGetSet->_expectedColorTableIndex = i;
            VERIFY_IS_TRUE(_pDispatch->SetColorTableEntry(i, testColor));
        }

        // Test in pty mode - we should fail, but PrivateSetColorTableEntry should still be called
        _testGetSet->_fIsPty = true;
        _testGetSet->_fIsConsolePtyResult = true;

        _testGetSet->_expectedColorTableIndex = 15; // Windows BRIGHT_WHITE
        VERIFY_IS_FALSE(_pDispatch->SetColorTableEntry(15, testColor));
    }

private:
    TestGetSet* _testGetSet; // non-ownership pointer
    AdaptDispatch* _pDispatch;
};
