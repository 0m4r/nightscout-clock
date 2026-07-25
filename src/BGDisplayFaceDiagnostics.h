#ifndef BGDISPLAYFACEDIAGNOSTICS_H
#define BGDISPLAYFACEDIAGNOSTICS_H

#include "BGDisplayFaceTextBase.h"

class BGDisplayFaceDiagnostics : public BGDisplayFaceTextBase {
public:
    void showReadings(const std::list<GlucoseReading>& readings, bool dataIsOld = false) const override;
    void showNoData() const override;
    bool needsFrequentRefresh() const override;
    unsigned long getFrequentRefreshIntervalMs() const override;

private:
    void showDateTimePage(const std::list<GlucoseReading>& readings, bool dataIsOld) const;
    int getScrollX(int contentWidth) const;
    const String& formatDateTime() const;

    // Cache for the date/time string so the 60fps scroll loop does not call the
    // relatively expensive (and potentially blocking) getLocalTime() on every
    // frame, and does not reallocate the String while the minute is unchanged.
    mutable unsigned long lastTimeCheckMillis = 0;
    mutable int cachedMinuteKey = -1;
    mutable String cachedDateTime;
};

#endif
