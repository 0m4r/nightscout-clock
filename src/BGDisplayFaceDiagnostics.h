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
    String formatDateTime() const;
};

#endif
