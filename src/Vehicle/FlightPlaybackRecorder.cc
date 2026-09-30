#include "FlightPlaybackRecorder.h"

#include <QtCore/QTimer>
#include <QtCore/QVariantMap>

#include <algorithm>

#include "QGCLoggingCategory.h"
#include "QmlObjectListModel.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(FlightPlaybackRecorderLog, "Vehicle.FlightPlaybackRecorder")

FlightPlaybackRecorder::FlightPlaybackRecorder(QmlObjectListModel* vehicles, QObject* parent)
    : QObject(parent), _vehicles(vehicles), _sampleTimer(new QTimer(this))
{
    _sampleTimer->setInterval(kSampleIntervalMs);
    (void) connect(_sampleTimer, &QTimer::timeout, this, &FlightPlaybackRecorder::_sampleVehicles);
}

void FlightPlaybackRecorder::startRecording()
{
    if (_recording) {
        return;
    }

    setPlaybackActive(false);
    _clear();

    _recording = true;
    _elapsedTimer.start();
    _sampleVehicles();
    _sampleTimer->start();
    qCDebug(FlightPlaybackRecorderLog) << "Recording started";
    emit recordingChanged(_recording);
}

void FlightPlaybackRecorder::stopRecording()
{
    if (!_recording) {
        return;
    }

    _sampleTimer->stop();
    _recording = false;
    qCDebug(FlightPlaybackRecorderLog) << "Recording stopped"
                                       << "durationMs:" << _durationMs << "vehicles:" << _samples.count();
    emit recordingChanged(_recording);
}

QVariantList FlightPlaybackRecorder::samplesAt(qint64 elapsedMs) const
{
    QVariantList result;
    for (auto it = _samples.cbegin(); it != _samples.cend(); ++it) {
        const QList<Sample>& samples = it.value();
        const auto after = std::upper_bound(samples.cbegin(), samples.cend(), elapsedMs,
                                            [](qint64 ms, const Sample& sample) { return ms < sample.elapsedMs; });
        if (after == samples.cbegin()) {
            continue;
        }
        const Sample& sample = *std::prev(after);
        result.append(QVariantMap{
            {QStringLiteral("vehicleId"), it.key()},
            {QStringLiteral("coordinate"), QVariant::fromValue(sample.coordinate)},
            {QStringLiteral("heading"), sample.heading},
        });
    }
    return result;
}

void FlightPlaybackRecorder::setPlaybackActive(bool active)
{
    if (active && (_recording || !hasRecording())) {
        return;
    }
    if (_playbackActive != active) {
        _playbackActive = active;
        emit playbackActiveChanged(_playbackActive);
    }
}

void FlightPlaybackRecorder::setPlaybackPositionMs(qint64 positionMs)
{
    positionMs = std::clamp(positionMs, qint64(0), _durationMs);
    if (_playbackPositionMs != positionMs) {
        _playbackPositionMs = positionMs;
        emit playbackPositionMsChanged(_playbackPositionMs);
    }
}

void FlightPlaybackRecorder::_sampleVehicles()
{
    if (!_vehicles) {
        return;
    }

    const qint64 elapsedMs = _elapsedTimer.elapsed();
    for (int i = 0; i < _vehicles->count(); i++) {
        Vehicle* vehicle = _vehicles->value<Vehicle*>(i);
        if (!vehicle) {
            continue;
        }
        const Fact* headingFact = vehicle->heading();
        const double heading = headingFact ? headingFact->rawValue().toDouble() : qQNaN();
        _recordSample(vehicle->id(), elapsedMs, vehicle->coordinate(), heading);
    }
}

void FlightPlaybackRecorder::_recordSample(int vehicleId, qint64 elapsedMs, const QGeoCoordinate& coordinate,
                                           double heading)
{
    if (!coordinate.isValid()) {
        return;
    }

    const bool hadRecording = hasRecording();
    QList<Sample>& samples = _samples[vehicleId];
    if (!samples.isEmpty() && elapsedMs < samples.last().elapsedMs) {
        qCWarning(FlightPlaybackRecorderLog) << "Dropping out-of-order sample"
                                             << "vehicleId:" << vehicleId << "elapsedMs:" << elapsedMs;
        return;
    }

    samples.append(Sample{.elapsedMs = elapsedMs, .coordinate = coordinate, .heading = heading});
    if (!hadRecording || elapsedMs > _durationMs) {
        _durationMs = std::max(_durationMs, elapsedMs);
        emit durationChanged();
    }
}

void FlightPlaybackRecorder::_clear()
{
    const bool hadRecording = hasRecording();
    _samples.clear();
    _durationMs = 0;
    setPlaybackPositionMs(0);
    if (hadRecording) {
        emit durationChanged();
    }
}
