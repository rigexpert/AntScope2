// Drives the REAL, unmodified HidAnalyzer::update() (analyzer/hid_analyzer.cpp)
// against the mock hidapi backend in mock_hid.cpp, to validate the client-side
// logic of the fix without any real hardware: previously, only the first
// firmware-write chunk's response was ever read, so the read queue filled
// with stale OK responses and the final integrity check (BL_CMD_CHECK)
// silently read one of those instead of the device's real answer - meaning
// firmware verification always appeared to pass. This validates the state
// machine (per-chunk error handling, short final chunk, boot-mode
// re-detection after RESET), not real-world device/timing behavior - that
// was separately validated on real hardware (see the PR description).

#include <QCoreApplication>
#include <QBuffer>
#include <QByteArray>
#include <QMessageBox>
#include <cstdio>

#include "analyzer/hid_analyzer.h"

// hid_analyzer.cpp references these as extern; normally defined in main.cpp,
// which this test does not link against.
bool g_usbOnly = false;

int g_showMessageBox(QWidget*, QMessageBox::Icon, QString title, QString text,
                      QMessageBox::StandardButtons, QMessageBox::StandardButton)
{
    printf("  [message box suppressed] %s: %s\n", qPrintable(title), qPrintable(text));
    return 0;
}

extern "C" {
    void mock_hid_reset_state();
    void mock_hid_set_fail_after_chunk(int n);
    void mock_hid_set_check_should_fail(bool fail);
    bool mock_hid_reset_was_sent();
    int  mock_hid_chunk_count();
    bool mock_hid_check_received();
    bool mock_hid_start_received();
    bool mock_hid_unread_response_overwritten();
    int  mock_hid_last_chunk_declared_length();
    hid_device* mock_hid_make_normal_mode_device();
}

namespace {

int g_failures = 0;

void expect(bool cond, const char* what)
{
    if (cond) {
        printf("  PASS: %s\n", what);
    } else {
        printf("  FAIL: %s\n", what);
        g_failures++;
    }
}

QByteArray makeFakeFirmware(int sizeBytes)
{
    QByteArray data;
    data.resize(sizeBytes);
    for (int i = 0; i < sizeBytes; i++) {
        data[i] = static_cast<char>(i & 0xFF);
    }
    return data;
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // --- Scenario 1: clean successful update ---
    printf("Scenario 1: successful update\n");
    {
        mock_hid_reset_state();
        HidAnalyzer analyzer;
        analyzer.setTestHidDevice(mock_hid_make_normal_mode_device(), "450001370");

        QByteArray fw = makeFakeFirmware(48 * 10); // 10 clean chunks
        QBuffer buf(&fw);
        buf.open(QIODevice::ReadOnly);

        bool ok = analyzer.update(&buf);

        expect(ok, "update() returns true");
        expect(mock_hid_reset_was_sent(), "RESET was sent");
        expect(mock_hid_chunk_count() == 10, "all 10 chunks were written");
        expect(mock_hid_check_received(), "BL_CMD_CHECK was sent");
        expect(mock_hid_start_received(), "BL_CMD_START was sent after a passing CHECK");
        expect(!mock_hid_unread_response_overwritten(),
               "no response was ever left unread before the next write (the bug this fixes)");
    }

    printf("\n");

    // --- Scenario 2: a chunk fails partway through (simulated BL_CMD_ERROR) ---
    printf("Scenario 2: BL_CMD_ERROR on chunk 5 of 10\n");
    {
        mock_hid_reset_state();
        mock_hid_set_fail_after_chunk(5);
        HidAnalyzer analyzer;
        analyzer.setTestHidDevice(mock_hid_make_normal_mode_device(), "450001370");

        QByteArray fw = makeFakeFirmware(48 * 10);
        QBuffer buf(&fw);
        buf.open(QIODevice::ReadOnly);

        bool ok = analyzer.update(&buf);

        expect(!ok, "update() returns false");
        expect(mock_hid_chunk_count() == 5, "stopped at chunk 5, did not send the remaining 5");
        expect(!mock_hid_check_received(), "BL_CMD_CHECK was never sent after a chunk error");
        expect(!mock_hid_start_received(), "BL_CMD_START was never sent after a chunk error");
    }

    printf("\n");

    // --- Scenario 3: CHECK itself fails (device reports bad checksum) ---
    printf("Scenario 3: BL_CMD_CHECK reports failure\n");
    {
        mock_hid_reset_state();
        mock_hid_set_check_should_fail(true);
        HidAnalyzer analyzer;
        analyzer.setTestHidDevice(mock_hid_make_normal_mode_device(), "450001370");

        QByteArray fw = makeFakeFirmware(48 * 10);
        QBuffer buf(&fw);
        buf.open(QIODevice::ReadOnly);

        bool ok = analyzer.update(&buf);

        expect(!ok, "update() returns false");
        expect(mock_hid_chunk_count() == 10, "all chunks were still written");
        expect(mock_hid_check_received(), "BL_CMD_CHECK was sent");
        expect(!mock_hid_start_received(),
               "BL_CMD_START was NOT sent after a failing CHECK "
               "(this is the exact defect the fix closes: a stale queued OK "
               "previously made this check always pass)");
    }

    printf("\n");

    // --- Scenario 4: firmware size is not a multiple of 48 (the real
    //     Stick230 recovery image: 227216-byte payload, 227216 % 48 == 32) ---
    // Uses a scaled-down size with the same remainder shape (3 full chunks
    // + one 32-byte tail) so the test stays fast; the chunking arithmetic
    // in update() doesn't care about absolute size.
    printf("Scenario 4: firmware size not a multiple of 48 (short final chunk)\n");
    {
        mock_hid_reset_state();
        HidAnalyzer analyzer;
        analyzer.setTestHidDevice(mock_hid_make_normal_mode_device(), "450001370");

        const int kFullChunks = 3;
        const int kTailBytes = 32;
        QByteArray fw = makeFakeFirmware(48 * kFullChunks + kTailBytes);
        QBuffer buf(&fw);
        buf.open(QIODevice::ReadOnly);

        bool ok = analyzer.update(&buf);

        expect(ok, "update() returns true");
        expect(mock_hid_chunk_count() == kFullChunks + 1,
               "sent 3 full 48-byte chunks plus one short tail chunk");
        expect(mock_hid_last_chunk_declared_length() == kTailBytes,
               "the final chunk declared its true length (32), not 48 "
               "(this is the exact path the Stick230 recovery image exercises "
               "that the Stick500 image, an exact multiple of 48, never did)");
        expect(mock_hid_check_received() && mock_hid_start_received(),
               "CHECK and START both sent after a short final chunk");
    }

    printf("\n%s\n", g_failures == 0 ? "ALL SCENARIOS PASSED" : "SOME SCENARIOS FAILED");
    return g_failures == 0 ? 0 : 1;
}
