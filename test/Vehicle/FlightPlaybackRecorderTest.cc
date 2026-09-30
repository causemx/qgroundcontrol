#include "FlightPlaybackRecorderTest.h"

#include <QtCore/QMetaObject>
#include <QtCore/QVariantMap>
#include <QtPositioning/QGeoCoordinate>
#include <QtTest/QSignalSpy>

#include "FlightPlaybackRecorder.h"
#include "QmlObjectListModel.h"

namespace {

const QGeoCoordinate kStart(47.397742, 8.545594, 100.0);

QGeoCoordinate coordinateAt(int step)
{
    return kStart.atDistanceAndAzimuth(10.0 * step, 90);
}

// Drives the private slot directly so samples get deterministic timestamps
void recordSample(FlightPlaybackRecorder& recorder, int vehicleId, qint64 elapsedMs, const QGeoCoordinate& coordinate)
{
    QVERIFY(QMetaObject::invokeMethod(&recorder, "_recordSample", Q_ARG(int, vehicleId), Q_ARG(qint64, elapsedMs),
                                      Q_ARG(QGeoCoordinate, coordinate), Q_ARG(double, 45.0)));
}

}  // namespace

void FlightPlaybackRecorderTest::_startStopRecording()
{
    QmlObjectListModel vehicles;
    FlightPlaybackRecorder recorder(&vehicles);
    QSignalSpy recordingSpy(&recorder, &FlightPlaybackRecorder::recordingChanged);

    recorder.startRecording();
    recorder.startRecording();
    QVERIFY(recorder.recording());
    QCOMPARE(recordingSpy.count(), 1);

    recorder.stopRecording();
    recorder.stopRecording();
    QVERIFY(!recorder.recording());
    QCOMPARE(recordingSpy.count(), 2);
}

void FlightPlaybackRecorderTest::_samplesAt_data()
{
    QTest::addColumn<qint64>("elapsedMs");
    QTest::addColumn<int>("expectedVehicle1Step");  // -1: vehicle 1 absent
    QTest::addColumn<bool>("expectVehicle2");

    QTest::newRow("beforeFirstSample") << qint64(-1) << -1 << false;
    QTest::newRow("exactFirstSample") << qint64(0) << 0 << false;
    QTest::newRow("betweenSamples") << qint64(1500) << 1 << false;
    QTest::newRow("vehicleJoinedMidRecording") << qint64(2000) << 2 << true;
    QTest::newRow("afterEnd") << qint64(60000) << 2 << true;
}

void FlightPlaybackRecorderTest::_samplesAt()
{
    QFETCH(qint64, elapsedMs);
    QFETCH(int, expectedVehicle1Step);
    QFETCH(bool, expectVehicle2);

    QmlObjectListModel vehicles;
    FlightPlaybackRecorder recorder(&vehicles);
    recordSample(recorder, 1, 0, coordinateAt(0));
    recordSample(recorder, 1, 1000, coordinateAt(1));
    recordSample(recorder, 1, 2000, coordinateAt(2));
    recordSample(recorder, 2, 2000, kStart);
    recordSample(recorder, 3, 2000, QGeoCoordinate());
    QCOMPARE(recorder.durationMs(), qint64(2000));

    int vehicle1Step = -1;
    bool hasVehicle2 = false;
    for (const QVariant& entry : recorder.samplesAt(elapsedMs)) {
        const QVariantMap sample = entry.toMap();
        const int vehicleId = sample.value(QStringLiteral("vehicleId")).toInt();
        QVERIFY2(vehicleId != 3, "Invalid coordinates must not be recorded");
        if (vehicleId == 1) {
            const QGeoCoordinate coordinate = sample.value(QStringLiteral("coordinate")).value<QGeoCoordinate>();
            for (int step = 0; step <= 2; step++) {
                if (coordinate == coordinateAt(step)) {
                    vehicle1Step = step;
                }
            }
            QCOMPARE(sample.value(QStringLiteral("heading")).toDouble(), 45.0);
        } else if (vehicleId == 2) {
            hasVehicle2 = true;
        }
    }
    QCOMPARE(vehicle1Step, expectedVehicle1Step);
    QCOMPARE(hasVehicle2, expectVehicle2);
}

void FlightPlaybackRecorderTest::_startRecordingClears()
{
    QmlObjectListModel vehicles;
    FlightPlaybackRecorder recorder(&vehicles);
    recordSample(recorder, 1, 0, coordinateAt(0));
    recordSample(recorder, 1, 3000, coordinateAt(1));
    recorder.setPlaybackActive(true);
    recorder.setPlaybackPositionMs(2000);
    QVERIFY(recorder.playbackActive());

    recorder.startRecording();

    QVERIFY(!recorder.hasRecording());
    QVERIFY(!recorder.playbackActive());
    QCOMPARE(recorder.durationMs(), qint64(0));
    QCOMPARE(recorder.playbackPositionMs(), qint64(0));
    QVERIFY(recorder.samplesAt(3000).isEmpty());
}

void FlightPlaybackRecorderTest::_playbackRequiresRecording()
{
    QmlObjectListModel vehicles;
    FlightPlaybackRecorder recorder(&vehicles);

    recorder.setPlaybackActive(true);
    QVERIFY(!recorder.playbackActive());

    recordSample(recorder, 1, 0, coordinateAt(0));
    recordSample(recorder, 1, 4000, coordinateAt(1));
    recorder.setPlaybackPositionMs(10000);
    QCOMPARE(recorder.playbackPositionMs(), qint64(4000));

    recorder.startRecording();
    recordSample(recorder, 1, 0, coordinateAt(0));
    recorder.setPlaybackActive(true);
    QVERIFY2(!recorder.playbackActive(), "Playback must stay off while recording");
}

UT_REGISTER_TEST(FlightPlaybackRecorderTest, TestLabel::Unit, TestLabel::Vehicle)
