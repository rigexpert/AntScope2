// Software-simulated RigExpert Stick 500 DFU bootloader, implementing the
// same function signatures as analyzer/usbhid/hidapi/hidapi.h, so that the
// REAL, unmodified HidAnalyzer::update() and HidAnalyzer::waitForBootDevice()
// (analyzer/hid_analyzer.cpp) can be exercised without any real hardware or
// USB stack involved.
//
// The simulated protocol (command bytes, chunk size, ACK value) matches what
// was independently verified against a real, successful update session
// captured from the genuine vendor Windows client. This is a model of that
// protocol, not the real bootloader: it validates that update()'s
// client-side logic (state machine, error handling, chunking, boot-mode
// re-detection) is internally correct. It cannot and does not validate
// real-world timing/pacing tolerance.

#include "hidapi.h"
#include <cstring>
#include <cwchar>
#include <cstdlib>

namespace {

const unsigned short kBootVid = 0x0483;
const unsigned short kBootPid = 0xA1DA;
const wchar_t* kSerial = L"450001370"; // matches the real capture

enum {
    BL_CMD_GET_ID = 1, BL_CMD_ERASE = 2, BL_CMD_WRITE = 3, BL_CMD_DATA = 4,
    BL_CMD_CHECK = 5, BL_CMD_START = 6, BL_CMD_OK = 7, BL_CMD_ERROR = 8
};

bool g_resetSent = false;
int g_enumeratePolls = 0;
const int kPollsBeforeBootAvailable = 3; // simulates re-enumeration delay

bool g_hasPendingResponse = false;
unsigned char g_pendingResponse[65];
bool g_unreadResponseOverwritten = false; // client wrote again without reading

int g_chunkCount = 0;
int g_failAfterChunk = -1; // -1 = never fail a chunk
bool g_checkReceived = false;
bool g_checkShouldFail = false;
bool g_startReceived = false;
int g_lastChunkDeclaredLength = 0; // data[2] of the most recent WRITE/DATA frame

struct MockDevice { bool isBoot; };

} // namespace

// ---- test-control API (not part of real hidapi; declared for the test driver) ----
extern "C" {
    void mock_hid_reset_state()
    {
        g_resetSent = false;
        g_enumeratePolls = 0;
        g_hasPendingResponse = false;
        g_unreadResponseOverwritten = false;
        g_chunkCount = 0;
        g_failAfterChunk = -1;
        g_checkReceived = false;
        g_checkShouldFail = false;
        g_startReceived = false;
        g_lastChunkDeclaredLength = 0;
    }
    void mock_hid_set_fail_after_chunk(int n) { g_failAfterChunk = n; }
    void mock_hid_set_check_should_fail(bool fail) { g_checkShouldFail = fail; }
    bool mock_hid_reset_was_sent() { return g_resetSent; }
    int  mock_hid_chunk_count() { return g_chunkCount; }
    bool mock_hid_check_received() { return g_checkReceived; }
    bool mock_hid_start_received() { return g_startReceived; }
    bool mock_hid_unread_response_overwritten() { return g_unreadResponseOverwritten; }
    int  mock_hid_last_chunk_declared_length() { return g_lastChunkDeclaredLength; }
    // A real, safely hid_close()-able handle for the test driver to inject
    // as the "already connected in normal mode" starting state, instead of
    // a raw non-heap pointer that would crash hid_close()'s delete later.
    hid_device* mock_hid_make_normal_mode_device()
    {
        MockDevice* dev = new MockDevice();
        dev->isBoot = false;
        return reinterpret_cast<hid_device*>(dev);
    }
}

// ---- hidapi surface ----

struct hid_device_info* HID_API_EXPORT HID_API_CALL hid_enumerate(unsigned short vid, unsigned short pid)
{
    if (vid != kBootVid || pid != kBootPid)
        return nullptr;
    if (!g_resetSent)
        return nullptr;

    g_enumeratePolls++;
    if (g_enumeratePolls < kPollsBeforeBootAvailable)
        return nullptr;

    struct hid_device_info* info = new struct hid_device_info();
    memset(info, 0, sizeof(*info));
    info->vendor_id = kBootVid;
    info->product_id = kBootPid;
    info->serial_number = wcsdup(kSerial);
    info->next = nullptr;
    return info;
}

void HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info* devs)
{
    while (devs != nullptr) {
        struct hid_device_info* next = devs->next;
        free(devs->serial_number);
        delete devs;
        devs = next;
    }
}

HID_API_EXPORT hid_device* HID_API_CALL hid_open(unsigned short vid, unsigned short pid, const wchar_t* serial)
{
    if (vid != kBootVid || pid != kBootPid)
        return nullptr;
    if (!g_resetSent || g_enumeratePolls < kPollsBeforeBootAvailable)
        return nullptr;
    if (serial != nullptr && wcscmp(serial, kSerial) != 0)
        return nullptr;

    MockDevice* dev = new MockDevice();
    dev->isBoot = true;
    return reinterpret_cast<hid_device*>(dev);
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device* device)
{
    delete reinterpret_cast<MockDevice*>(device);
}

int HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device*, int)
{
    return 0;
}

int HID_API_EXPORT HID_API_CALL hid_write(hid_device* device, const unsigned char* data, size_t length)
{
    if (device == nullptr || data == nullptr)
        return -1;

    // RESET: buff[0]=1, "RESET" ascii at buff[1..5] (see
    // HidAnalyzer::update()). This is what triggers the simulated
    // device to start "re-enumerating" as the boot-mode device.
    if (length >= 6 && data[0] == 1 && memcmp(&data[1], "RESET", 5) == 0) {
        g_resetSent = true;
        return static_cast<int>(length);
    }

    if (length < 3)
        return -1;

    unsigned char cmd = data[1];

    if (cmd == BL_CMD_WRITE || cmd == BL_CMD_DATA) {
        if (g_hasPendingResponse)
            g_unreadResponseOverwritten = true; // caller didn't read the last one
        g_chunkCount++;
        g_lastChunkDeclaredLength = data[2];
        memset(g_pendingResponse, 0, sizeof(g_pendingResponse));
        g_pendingResponse[0] = (g_failAfterChunk >= 0 && g_chunkCount == g_failAfterChunk)
                                    ? BL_CMD_ERROR : BL_CMD_OK;
        g_hasPendingResponse = true;
        return static_cast<int>(length);
    }
    if (cmd == BL_CMD_CHECK) {
        if (g_hasPendingResponse)
            g_unreadResponseOverwritten = true;
        g_checkReceived = true;
        memset(g_pendingResponse, 0, sizeof(g_pendingResponse));
        g_pendingResponse[0] = g_checkShouldFail ? BL_CMD_ERROR : BL_CMD_OK;
        g_hasPendingResponse = true;
        return static_cast<int>(length);
    }
    if (cmd == BL_CMD_START) {
        g_startReceived = true;
        return static_cast<int>(length);
    }

    return static_cast<int>(length);
}

int HID_API_EXPORT HID_API_CALL hid_read_timeout(hid_device* device, unsigned char* data, size_t length, int)
{
    if (device == nullptr)
        return -1;
    if (!g_hasPendingResponse)
        return 0; // matches real hidapi: 0 = no data ready yet, keep polling

    memset(data, 0, length);
    size_t n = length < sizeof(g_pendingResponse) ? length : sizeof(g_pendingResponse);
    memcpy(data, g_pendingResponse, n);
    g_hasPendingResponse = false;
    return static_cast<int>(n);
}

int HID_API_EXPORT HID_API_CALL hid_read(hid_device* device, unsigned char* data, size_t length)
{
    // Non-blocking variant: same behavior as hid_read_timeout() here since
    // this mock always has the response ready synchronously (produced
    // inside hid_write() above) or not at all.
    return hid_read_timeout(device, data, length, 0);
}

HID_API_EXPORT const wchar_t* HID_API_CALL hid_error(hid_device*)
{
    return L"(mock hidapi backend - no error string)";
}
