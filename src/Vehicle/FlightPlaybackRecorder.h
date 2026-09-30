#pragma once

#include <QtCore/QElapsedTimer>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>
#include <QtQmlIntegration/QtQmlIntegration>

class QmlObjectListModel;
class QTimer;

Q_DECLARE_LOGGING_CATEGORY(FlightPlaybackRecorderLog)

/// Records time-indexed position/heading samples for all vehicles so a flight can be replayed as ghost vehicles
class FlightPlaybackRecorder : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(qint64 durationMs READ durationMs NOTIFY durationChanged)
    Q_PROPERTY(bool hasRecording READ hasRecording NOTIFY durationChanged)
    Q_PROPERTY(bool playbackActive READ playbackActive WRITE setPlaybackActive NOTIFY playbackActiveChanged)
    Q_PROPERTY(
        qint64 playbackPositionMs READ playbackPositionMs WRITE setPlaybackPositionMs NOTIFY playbackPositionMsChanged)

public:
    explicit FlightPlaybackRecorder(QmlObjectListModel* vehicles, QObject* parent = nullptr);

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

    /// Returns one { vehicleId, coordinate, heading } map per vehicle holding the latest sample at or before elapsedMs
    Q_INVOKABLE QVariantList samplesAt(qint64 elapsedMs) const;

    bool recording() const { return _recording; }

    qint64 durationMs() const { return _durationMs; }

    bool hasRecording() const { return !_samples.isEmpty(); }

    bool playbackActive() const { return _playbackActive; }

    qint64 playbackPositionMs() const { return _playbackPositionMs; }

    void setPlaybackActive(bool active);
    void setPlaybackPositionMs(qint64 positionMs);

    static constexpr int kSampleIntervalMs = 1000;

signals:
    void recordingChanged(bool recording);
    void durationChanged();
    void playbackActiveChanged(bool playbackActive);
    void playbackPositionMsChanged(qint64 playbackPositionMs);

private slots:
    void _sampleVehicles();
    void _recordSample(int vehicleId, qint64 elapsedMs, const QGeoCoordinate& coordinate, double heading);

private:
    struct Sample
    {
        qint64 elapsedMs;
        QGeoCoordinate coordinate;
        double heading;
    };

    void _clear();

    QmlObjectListModel* _vehicles = nullptr;
    QTimer* _sampleTimer = nullptr;
    QElapsedTimer _elapsedTimer;
    QHash<int, QList<Sample>> _samples;
    qint64 _durationMs = 0;
    bool _recording = false;
    bool _playbackActive = false;
    qint64 _playbackPositionMs = 0;
};
