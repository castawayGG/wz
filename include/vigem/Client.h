/*
 * ViGEmClient SDK Header (bundled)
 * Original: https://github.com/nefarius/ViGEmClient
 * License: MIT
 *
 * This header provides the client-side API for communicating with the
 * ViGEmBus kernel driver to create virtual HID devices.
 *
 * NOTE: The ViGEmBus driver does NOT read or modify game process memory.
 * It operates purely as a virtual USB bus controller (PnP device).
 */

#pragma once

#ifndef VIGEM_CLIENT_H
#define VIGEM_CLIENT_H

#include <Windows.h>
#include <winioctl.h>

//
// ViGEm error codes
//
typedef enum _VIGEM_ERROR
{
    VIGEM_ERROR_NONE                        = 0x20000000,
    VIGEM_ERROR_BUS_NOT_FOUND               = 0xE0000001,
    VIGEM_ERROR_NO_FREE_SLOT                = 0xE0000002,
    VIGEM_ERROR_INVALID_TARGET              = 0xE0000003,
    VIGEM_ERROR_REMOVAL_FAILED              = 0xE0000004,
    VIGEM_ERROR_ALREADY_CONNECTED           = 0xE0000005,
    VIGEM_ERROR_TARGET_UNINITIALIZED        = 0xE0000006,
    VIGEM_ERROR_TARGET_NOT_PLUGGED_IN       = 0xE0000007,
    VIGEM_ERROR_BUS_VERSION_MISMATCH        = 0xE0000008,
    VIGEM_ERROR_BUS_ACCESS_FAILED           = 0xE0000009,
    VIGEM_ERROR_CALLBACK_ALREADY_REGISTERED = 0xE0000010,
    VIGEM_ERROR_CALLBACK_NOT_FOUND          = 0xE0000011,
    VIGEM_ERROR_BUS_ALREADY_CONNECTED       = 0xE0000012,
    VIGEM_ERROR_BUS_INVALID_HANDLE          = 0xE0000013,
    VIGEM_ERROR_XUSB_USERINDEX_OUT_OF_RANGE = 0xE0000014,
    VIGEM_ERROR_INVALID_PARAMETER           = 0xE0000015,
    VIGEM_ERROR_NOT_SUPPORTED               = 0xE0000016,
    VIGEM_ERROR_WINAPI                      = 0xE0000017,
    VIGEM_ERROR_TIMED_OUT                   = 0xE0000018,
} VIGEM_ERROR;

#define VIGEM_SUCCESS(Error) ((VIGEM_ERROR)(Error) == VIGEM_ERROR_NONE)

//
// Xbox 360 button/axis bitmasks (XUSB_REPORT.wButtons)
//
#define XUSB_GAMEPAD_DPAD_UP            0x0001
#define XUSB_GAMEPAD_DPAD_DOWN          0x0002
#define XUSB_GAMEPAD_DPAD_LEFT          0x0004
#define XUSB_GAMEPAD_DPAD_RIGHT         0x0008
#define XUSB_GAMEPAD_START              0x0010
#define XUSB_GAMEPAD_BACK               0x0020
#define XUSB_GAMEPAD_LEFT_THUMB         0x0040
#define XUSB_GAMEPAD_RIGHT_THUMB        0x0080
#define XUSB_GAMEPAD_LEFT_SHOULDER      0x0100
#define XUSB_GAMEPAD_RIGHT_SHOULDER     0x0200
#define XUSB_GAMEPAD_GUIDE              0x0400
#define XUSB_GAMEPAD_A                  0x1000
#define XUSB_GAMEPAD_B                  0x2000
#define XUSB_GAMEPAD_X                  0x4000
#define XUSB_GAMEPAD_Y                  0x8000

//
// Xbox 360 controller report structure
//
#pragma pack(push, 1)
typedef struct _XUSB_REPORT
{
    USHORT wButtons;     // Button bitmask
    BYTE   bLeftTrigger; // LT: 0-255
    BYTE   bRightTrigger;// RT: 0-255
    SHORT  sThumbLX;     // Left stick X: -32768 to 32767
    SHORT  sThumbLY;     // Left stick Y: -32768 to 32767
    SHORT  sThumbRX;     // Right stick X: -32768 to 32767
    SHORT  sThumbRY;     // Right stick Y: -32768 to 32767
} XUSB_REPORT, *PXUSB_REPORT;
#pragma pack(pop)

//
// Opaque handle types
//
typedef struct _VIGEM_CLIENT_T *PVIGEM_CLIENT;
typedef struct _VIGEM_TARGET_T *PVIGEM_TARGET;

//
// Vibration notification callback
//
typedef VOID(CALLBACK *PFN_VIGEM_X360_NOTIFICATION)(
    PVIGEM_CLIENT Client,
    PVIGEM_TARGET Target,
    UCHAR LargeMotor,
    UCHAR SmallMotor,
    UCHAR LedNumber,
    LPVOID UserData
);

#ifdef __cplusplus
extern "C" {
#endif

//
// Client lifecycle
//
PVIGEM_CLIENT vigem_alloc(void);
void          vigem_free(PVIGEM_CLIENT vigem);
VIGEM_ERROR   vigem_connect(PVIGEM_CLIENT vigem);
void          vigem_disconnect(PVIGEM_CLIENT vigem);

//
// Target (virtual device) lifecycle
//
PVIGEM_TARGET vigem_target_x360_alloc(void);
void          vigem_target_free(PVIGEM_TARGET target);

VIGEM_ERROR vigem_target_add(PVIGEM_CLIENT vigem, PVIGEM_TARGET target);
VIGEM_ERROR vigem_target_remove(PVIGEM_CLIENT vigem, PVIGEM_TARGET target);

//
// Xbox 360 report update
//
VIGEM_ERROR vigem_target_x360_update(
    PVIGEM_CLIENT vigem,
    PVIGEM_TARGET target,
    XUSB_REPORT report
);

//
// Vibration feedback registration
//
VIGEM_ERROR vigem_target_x360_register_notification(
    PVIGEM_CLIENT                     vigem,
    PVIGEM_TARGET                     target,
    PFN_VIGEM_X360_NOTIFICATION       notification,
    LPVOID                            userData
);

void vigem_target_x360_unregister_notification(PVIGEM_TARGET target);

//
// Target state queries
//
BOOL vigem_target_is_attached(PVIGEM_TARGET target);
ULONG vigem_target_get_index(PVIGEM_TARGET target);

#ifdef __cplusplus
}
#endif

//
// Inline helper: zero-initialize an XUSB_REPORT
//
inline void XUSB_REPORT_INIT(PXUSB_REPORT report)
{
    RtlZeroMemory(report, sizeof(XUSB_REPORT));
}

#endif // VIGEM_CLIENT_H
