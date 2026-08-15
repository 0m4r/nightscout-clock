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
