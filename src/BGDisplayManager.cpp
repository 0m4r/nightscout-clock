<<<<<<< HEAD
#include "BGDisplayManager.h"

<<<<<<< HEAD
#include <algorithm>
#include <list>

#include "BGSource.h"
#include "BGSourceManager.h"
#include "DisplayManager.h"
#include "ServerManager.h"
#include "SettingsManager.h"
#include "globals.h"

// The getter for the instantiated singleton instance
BGDisplayManager_& BGDisplayManager_::getInstance() {
    static BGDisplayManager_ instance;
    return instance;
}

// Initialize the global shared instance
BGDisplayManager_& bgDisplayManager = bgDisplayManager.getInstance();

void BGDisplayManager_::setup() {
    glucoseIntervals = GlucoseIntervals();
    /// TODO: Add urgent values to settings

    glucoseIntervals.addInterval(1, SettingsManager.settings.bg_low_urgent_limit, BG_LEVEL::URGENT_LOW);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_low_urgent_limit + 1, SettingsManager.settings.bg_low_warn_limit - 1,
        BG_LEVEL::WARNING_LOW);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_low_warn_limit, SettingsManager.settings.bg_high_warn_limit,
        BG_LEVEL::NORMAL);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_high_warn_limit, SettingsManager.settings.bg_high_urgent_limit - 1,
        BG_LEVEL::WARNING_HIGH);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_high_urgent_limit, 401, BG_LEVEL::URGENT_HIGH);

    faces.push_back(new BGDisplayFaceSimple());
    facesNames[0] = "Simple";
    faces.push_back(new BGDisplayFaceGraph());
    facesNames[1] = "Full graph";
    faces.push_back(new BGDisplayFaceGraphAndBG());
    facesNames[2] = "Graph and BG";
    faces.push_back(new BGDisplayFaceBigText());
    facesNames[3] = "Big text";
    faces.push_back(new BGDisplayFaceValueAndDiff());
    facesNames[4] = "Value and diff";
    faces.push_back(new BGDisplayFaceDiagnostics());
    facesNames[6] = "Diagnostics";
    faces.push_back(new BGDisplayFaceBatteryUptime());
    facesNames[7] = "Battery and uptime";
    faces.push_back(new BGDisplayFaceBigTextRainbow());
    facesNames[8] = "Rainbow big text";
    faces.push_back(new BGDisplayFaceSmiley());
    facesNames[9] = "Smiley";

    configureFaceCycle();

    if (faceCycleActive) {
        currentFaceIndex = faceCycleFaces.front();
    } else {
        currentFaceIndex = SettingsManager.settings.default_clockface;
    }

    if (currentFaceIndex < 0 || static_cast<size_t>(currentFaceIndex) >= faces.size()) {
        currentFaceIndex = 0;
    }

    currentFace = (faces[currentFaceIndex]);
}

void BGDisplayManager_::configureFaceCycle() {
    faceCycleFaces.clear();
    faceCycleActive = false;
    faceCycleTimerStarted = false;

    for (int faceId : SettingsManager.settings.face_cycle_faces) {
        if (faceId < 0 || static_cast<size_t>(faceId) >= faces.size()) {
            continue;
        }

        if (std::find(faceCycleFaces.begin(), faceCycleFaces.end(), faceId) == faceCycleFaces.end()) {
            faceCycleFaces.push_back(faceId);
        }
    }

    if (!SettingsManager.settings.face_cycle_enabled) {
        return;
    }

    if (faceCycleFaces.size() < 2) {
        DEBUG_PRINTF(
            "Clock face cycling disabled: at least two valid unique faces are required, found %u\n",
            static_cast<unsigned int>(faceCycleFaces.size()));
        return;
    }

    faceCycleActive = true;
}

std::map<int, String> BGDisplayManager_::getFaces() { return facesNames; }

int BGDisplayManager_::getCurrentFaceId() { return currentFaceIndex; }

GlucoseIntervals BGDisplayManager_::getGlucoseIntervals() { return glucoseIntervals; }

void BGDisplayManager_::setFace(int id) {
    if (id < 0 || static_cast<size_t>(id) >= faces.size()) {
        return;
    }

    currentFaceIndex = id;
    currentFace = (faces[currentFaceIndex]);
    lastRefreshEpoch = 0;
    resetFaceCycleTimer();
    runRenderCycle(RenderReason::FACE_CHANGE, ServerManager.getTimezonedTime());
}

void BGDisplayManager_::showNextFace() {
    if (!faceCycleActive) {
        int nextFaceIndex = currentFaceIndex + 1;
        if (static_cast<size_t>(nextFaceIndex) >= faces.size()) {
            nextFaceIndex = 0;
        }
        setFace(nextFaceIndex);
        return;
    }

    auto current = std::find(faceCycleFaces.begin(), faceCycleFaces.end(), currentFaceIndex);
    if (current == faceCycleFaces.end()) {
        setFace(faceCycleFaces.front());
        return;
    }

    current++;
    setFace(current == faceCycleFaces.end() ? faceCycleFaces.front() : *current);
}

void BGDisplayManager_::showPreviousFace() {
    if (!faceCycleActive) {
        int previousFaceIndex = currentFaceIndex - 1;
        if (previousFaceIndex < 0) {
            previousFaceIndex = static_cast<int>(faces.size()) - 1;
        }
        setFace(previousFaceIndex);
        return;
    }

    auto current = std::find(faceCycleFaces.begin(), faceCycleFaces.end(), currentFaceIndex);
    if (current == faceCycleFaces.end() || current == faceCycleFaces.begin()) {
        setFace(faceCycleFaces.back());
    } else {
        setFace(*--current);
    }
}

void BGDisplayManager_::resetFaceCycleTimer() {
    lastFaceCycleMillis = millis();
    faceCycleTimerStarted = true;
}

void BGDisplayManager_::updateFaceCycle() {
    if (!faceCycleActive) {
        return;
    }

    if (MATRIX_OFF) {
        faceCycleTimerStarted = false;
        return;
    }

    unsigned long currentMillis = millis();
    if (!faceCycleTimerStarted) {
        lastFaceCycleMillis = currentMillis;
        faceCycleTimerStarted = true;
        return;
    }

    unsigned long intervalMillis =
        static_cast<unsigned long>(SettingsManager.settings.face_cycle_interval_seconds) * 1000UL;
    if (currentMillis - lastFaceCycleMillis >= intervalMillis) {
        showNextFace();
    }
}

void BGDisplayManager_::tick() {
    updateFaceCycle();
    maybeRrefreshScreen();
}
=======
#include "BGDisplayManager.h"

#include <list>

#include "BGSource.h"
#include "BGSourceManager.h"
#include "DisplayManager.h"
#include "ServerManager.h"
#include "SettingsManager.h"
#include "globals.h"

// The getter for the instantiated singleton instance
BGDisplayManager_& BGDisplayManager_::getInstance() {
    static BGDisplayManager_ instance;
    return instance;
}

// Initialize the global shared instance
BGDisplayManager_& bgDisplayManager = bgDisplayManager.getInstance();

void BGDisplayManager_::setup() {
    glucoseIntervals = GlucoseIntervals();
    /// TODO: Add urgent values to settings

    glucoseIntervals.addInterval(1, SettingsManager.settings.bg_low_urgent_limit, BG_LEVEL::URGENT_LOW);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_low_urgent_limit + 1, SettingsManager.settings.bg_low_warn_limit - 1,
        BG_LEVEL::WARNING_LOW);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_low_warn_limit, SettingsManager.settings.bg_high_warn_limit,
        BG_LEVEL::NORMAL);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_high_warn_limit, SettingsManager.settings.bg_high_urgent_limit - 1,
        BG_LEVEL::WARNING_HIGH);
    glucoseIntervals.addInterval(
        SettingsManager.settings.bg_high_urgent_limit, 401, BG_LEVEL::URGENT_HIGH);

    faces.push_back(new BGDisplayFaceSimple());
    facesNames[0] = "Simple";
    faces.push_back(new BGDisplayFaceGraph());
    facesNames[1] = "Full graph";
    faces.push_back(new BGDisplayFaceGraphAndBG());
    facesNames[2] = "Graph and BG";
    faces.push_back(new BGDisplayFaceBigText());
    facesNames[3] = "Big text";
    faces.push_back(new BGDisplayFaceValueAndDiff());
    facesNames[4] = "Value and diff";
    faces.push_back(new BGDisplayFaceClock());
    facesNames[5] = "Clock and value";

    currentFaceIndex = SettingsManager.settings.default_clockface;
    if (currentFaceIndex >= faces.size()) {
        currentFaceIndex = 0;
    }

    currentFace = (faces[currentFaceIndex]);
}

std::map<int, String> BGDisplayManager_::getFaces() { return facesNames; }

int BGDisplayManager_::getCurrentFaceId() { return currentFaceIndex; }

GlucoseIntervals BGDisplayManager_::getGlucoseIntervals() { return glucoseIntervals; }

void BGDisplayManager_::setFace(int id) {
    if (id < faces.size()) {
        currentFaceIndex = id;
        currentFace = (faces[currentFaceIndex]);
        DisplayManager.clearMatrix();
        lastRefreshEpoch = 0;
        tick();
    }
}

void BGDisplayManager_::tick() { maybeRrefreshScreen(); }

void BGDisplayManager_::renderCurrentFace(bool dataIsOld) {
    DisplayManager.clearMatrix();
    if (displayedReadings.size() > 0) {
        currentFace->showReadings(displayedReadings, dataIsOld);
    } else {
        currentFace->showNoData();
    }
    DisplayManager.update();
    lastRenderedDataWasOld = dataIsOld;
    lastRefreshEpoch = ServerManager.getUtcEpoch();
}

bool BGDisplayManager_::shouldUseClockPartialRefresh(bool force, bool dataIsOld) const {
    return !force && displayedReadings.size() > 0 && currentFaceIndex == 5 &&
           dataIsOld == lastRenderedDataWasOld;
}

void BGDisplayManager_::refreshClockFaceTimeAndTimer() {
    if (currentFaceIndex != 5 || clockFace == nullptr || displayedReadings.size() == 0) {
        renderCurrentFace(lastRenderedDataWasOld);
        return;
    }

    DisplayManager.clearMatrixPart(0, 0, 16, 7);

    switch (SettingsManager.settings.time_format) {
        case TIME_FORMAT::HOURS_12:
            DisplayManager.clearMatrixPart(0, 7, 16, 1);
            DisplayManager.clearMatrixPart(18, 7, 13, 1);
            break;
        default:
            DisplayManager.clearMatrixPart(0, 7, 31, 1);
            break;
    }

    clockFace->showClock();

    switch (SettingsManager.settings.time_format) {
        case TIME_FORMAT::HOURS_12:
            BGDisplayManager_::drawTimerBlocks(displayedReadings.back(), MATRIX_WIDTH - 17, 18, 7);
            break;
        default:
            BGDisplayManager_::drawTimerBlocks(displayedReadings.back(), MATRIX_WIDTH, 0, 7);
            break;
    }

    DisplayManager.update();
    lastRefreshEpoch = ServerManager.getUtcEpoch();
}

void BGDisplayManager_::maybeRrefreshScreen(bool force) {
    auto currentEpoch = ServerManager.getUtcEpoch();
    tm timeInfo = ServerManager.getTimezonedTime();

    auto lastReading = bgDisplayManager.getLastDisplayedGlucoseReading();

    if (bgSourceManager.hasNewData(lastReading == NULL ? 0 : lastReading->epoch)) {
        DEBUG_PRINTLN("We have new data");
        bgDisplayManager.showData(bgSourceManager.getInstance().getGlucoseData());
    } else {
        // We refresh the display every minue trying to match the exact :00 second
        if (force || timeInfo.tm_sec == 0 && currentEpoch > lastRefreshEpoch ||
            currentEpoch - lastRefreshEpoch > 60) {
            if (displayedReadings.size() > 0) {
                bool dataIsOld = displayedReadings.back().getSecondsAgo() >
                                 60 * SettingsManager.settings.bg_data_too_old_threshold_minutes;
                if (shouldUseClockPartialRefresh(force, dataIsOld)) {
                    refreshClockFaceTimeAndTimer();
                } else {
                    renderCurrentFace(dataIsOld);
                }
            } else {
                renderCurrentFace(false);
            }
        }
    }
}

void BGDisplayManager_::showData(std::list<GlucoseReading> glucoseReadings) {
    if (glucoseReadings.size() == 0) {
        displayedReadings.clear();
        renderCurrentFace(false);
        return;
    }

    displayedReadings = glucoseReadings;
    bool dataIsOld = displayedReadings.back().getSecondsAgo() >
                     60 * SettingsManager.settings.bg_data_too_old_threshold_minutes;
    renderCurrentFace(dataIsOld);
}

// We draw the horizontal blocks equal to the number of minutes since last reading
// maximum numer of lines is 5
// Depending on the face we draw the lines in different places having different sizes
// The idea is to fit that maximum of 5 lines in the available space
// We can draw lines in 3 colors:
// - dark green if reading is less than 6 minutes old
// - dark orange if reading is between 6 and old_data_threshold_minutes threshold
// - gray if reading is older than old_data_threshold_minutes threshold
// @param lastReading - the last reading to draw the lines for
// @param width - the width of the available space in pixels
// @param yPosition - the y position of the lines
// @param xPosition - the x position of the lines
void BGDisplayManager_::drawTimerBlocks(
    GlucoseReading lastReading, int width, int xPosition, int yPosition) {
    const int MAX_BLOXCS = 5;  // maximum number of blocks to draw

    int blocksCount = lastReading.getSecondsAgo() / 60;
    if (blocksCount > MAX_BLOXCS) {
        blocksCount = MAX_BLOXCS;  // we draw maximum 5 lines
    }
    if (blocksCount <= 0) {
#ifdef DEBUG_DISPLAY
        DEBUG_PRINTLN("No blocks to draw, not drawing timer blocks");
#endif
        return;
    }

    // minimal block size is 1 pixel, size between blocks is 1 pixel, so we get width, subtract spaces
    // between lines and divide by the maximum number of lines
    int blockSize = blockSize = (width - 4) / MAX_BLOXCS;
    if (blockSize < 1) {
#ifdef DEBUG_DISPLAY
        DEBUG_PRINTLN("Block size is less than 1, not drawing timer blocks");
#endif
        return;
    }

    // Now let's alter xPosition to center the blocks in the available space
    xPosition += (width - (blockSize * MAX_BLOXCS + (MAX_BLOXCS - 1))) / 2;

    uint16_t color = COLOR_GREEN;
    if (lastReading.getSecondsAgo() >= 60 * SettingsManager.settings.bg_data_too_old_threshold_minutes) {
        color = COLOR_GRAY;  // old data
    } else if (lastReading.getSecondsAgo() >= (MAX_BLOXCS + 1) * 60) {
        color = COLOR_YELLOW;  // warning data
    }
#ifdef DEBUG_DISPLAY
    DEBUG_PRINTF(
        "Drawing %d blocks of size %d at position (%d, %d) with color %04X", blocksCount, blockSize,
        xPosition, yPosition, color);
#endif

    for (int i = 0; i < blocksCount; i++) {
        int x = xPosition + i * (blockSize + 1);  // +1 for the space between blocks
        for (int j = 0; j < blockSize; j++) {
            DisplayManager.drawPixel(x + j, yPosition, color, false);
        }
    }
}

GlucoseReading* BGDisplayManager_::getLastDisplayedGlucoseReading() {
    if (displayedReadings.size() > 0) {
        return &displayedReadings.back();
    } else {
        return NULL;
    }
}
>>>>>>> b6fca79 (Commit-based screen refresh)
