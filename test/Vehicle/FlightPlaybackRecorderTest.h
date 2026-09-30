#pragma once

#include "UnitTest.h"

class FlightPlaybackRecorderTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _startStopRecording();
    void _samplesAt_data();
    void _samplesAt();
    void _startRecordingClears();
    void _playbackRequiresRecording();
};
