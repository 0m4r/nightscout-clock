#include "BGDisplayFaceDiagnostics.h"

#include "ServerManager.h"
#include "globals.h"

namespace {
constexpr int CONTENT_BASELINE_Y = 7;
constexpr int STATUS_ITEM_SPACING = 2;
constexpr int ARROW_SPACING = 1;
constexpr int ARROW_WIDTH = 5;
constexpr int DATETIME_SCROLL_LEFT_PADDING = 2;
constexpr unsigned long DATETIME_SCROLL_FRAME_MS = 16;
constexpr float DATETIME_SCROLL_STEP_PIXELS = 0.18f;
const char* WEEKDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
}  // namespace

void BGDisplayFaceDiagnostics::showReadings(
    const std::list<GlucoseReading>& readings, bool dataIsOld) const {
    showDateTimePage(readings, dataIsOld);
}

void BGDisplayFaceDiagnostics::showNoData() const {
    String dateTimeText = formatDateTime();
    String noDataText = SettingsManager.settings.bg_units == BG_UNIT::MMOLL ? "--.-" : "---";
    int dateTimeWidth = (int)DisplayManager.getTextWidth(dateTimeText.c_str(), 2);
    int noDataWidth = (int)DisplayManager.getTextWidth(noDataText.c_str(), 2);
    int contentWidth = dateTimeWidth + STATUS_ITEM_SPACING + noDataWidth;
    int scrollX = getScrollX(contentWidth);

    DisplayManager.clearMatrix(false);

    DisplayManager.setTextColor(COLOR_CYAN);
    DisplayManager.printText(scrollX, CONTENT_BASELINE_Y, dateTimeText.c_str(), TEXT_ALIGNMENT::LEFT, 2, false);

    DisplayManager.setTextColor(BG_COLOR_OLD);
    DisplayManager.printText(
        scrollX + dateTimeWidth + STATUS_ITEM_SPACING, CONTENT_BASELINE_Y, noDataText.c_str(),
        TEXT_ALIGNMENT::LEFT, 2, false);

    DisplayManager.update();
}

bool BGDisplayFaceDiagnostics::needsFrequentRefresh() const { return true; }

unsigned long BGDisplayFaceDiagnostics::getFrequentRefreshIntervalMs() const {
    return DATETIME_SCROLL_FRAME_MS;
}

void BGDisplayFaceDiagnostics::showDateTimePage(
    const std::list<GlucoseReading>& readings, bool dataIsOld) const {
    auto lastReading = readings.back();
    String dateTimeText = formatDateTime();
    String printableReading = getPrintableReading(lastReading.sgv);
    int dateTimeWidth = (int)DisplayManager.getTextWidth(dateTimeText.c_str(), 2);
    int readingWidth = (int)DisplayManager.getTextWidth(printableReading.c_str(), 2);
    int readingX = dateTimeWidth + STATUS_ITEM_SPACING;
    int arrowX = readingX + readingWidth + ARROW_SPACING;
    int contentWidth = arrowX + ARROW_WIDTH;
    int scrollX = getScrollX(contentWidth);

    DisplayManager.clearMatrix(false);

    DisplayManager.setTextColor(COLOR_CYAN);
    DisplayManager.printText(scrollX, CONTENT_BASELINE_Y, dateTimeText.c_str(), TEXT_ALIGNMENT::LEFT, 2, false);

    showReading(
        lastReading, scrollX + readingX, CONTENT_BASELINE_Y, TEXT_ALIGNMENT::LEFT, FONT_TYPE::MEDIUM,
        dataIsOld, false);
    showTrendArrow(lastReading, scrollX + arrowX, 2, dataIsOld, true, false);

    DisplayManager.update();
}

int BGDisplayFaceDiagnostics::getScrollX(int contentWidth) const {
    if (contentWidth <= MATRIX_WIDTH) {
        return max(0, (MATRIX_WIDTH - contentWidth) / 2);
    }

    float travel = contentWidth + MATRIX_WIDTH + DATETIME_SCROLL_LEFT_PADDING;
    float position = MATRIX_WIDTH - fmodf(
                                        (millis() / (float)DATETIME_SCROLL_FRAME_MS) *
                                            DATETIME_SCROLL_STEP_PIXELS,
                                        travel);

    return (int)position;
}

String BGDisplayFaceDiagnostics::formatDateTime() const {
    tm timeinfo = ServerManager.getTimezonedTime();
    char dateTimeBuffer[20];

    snprintf(
        dateTimeBuffer, sizeof(dateTimeBuffer), "%s %02d/%02d %02d:%02d",
        WEEKDAY_NAMES[timeinfo.tm_wday], timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_hour,
        timeinfo.tm_min);

    return String(dateTimeBuffer);
}
