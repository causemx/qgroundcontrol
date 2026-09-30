import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.FactControls
import QGroundControl.Controls
import QGroundControl.FlyView
import QGroundControl.FlightMap

Rectangle {
    id:             topRightPanel
    width:          contentWidth
    height:         Math.max(contentHeight, minimumHeight)
    color:          qgcPal.toolbarBackground
    radius:         ScreenTools.defaultFontPixelHeight / 2
    visible:        !QGroundControl.videoManager.fullScreen && _multipleVehicles && _settingEnableMVPanel
    clip:           true

    property bool _settingEnableMVPanel:    QGroundControl.settingsManager.appSettings.enableMultiVehiclePanel.value
    property bool  _multipleVehicles:       QGroundControl.multiVehicleManager.vehicles.count > 1
    property var   vehicles:                QGroundControl.multiVehicleManager.vehicles
    property var   selectedVehicles:        QGroundControl.multiVehicleManager.selectedVehicles
    property real  contentWidth:            Math.max(
                                                multiVehicleList.implicitWidth,
                                                swipeViewContainer.implicitWidth
                                            ) + ScreenTools.defaultFontPixelHeight
    property real  contentHeight:           Math.min(
                                                maximumHeight,
                                                topRightPanelColumnLayout.implicitHeight + topRightPanelColumnLayout.anchors.margins * 2
                                            )
    property real  minimumHeight:           swipeViewContainer.implicitHeight
    property real  maximumHeight

    QGCPalette { id: qgcPal }

    DeadMouseArea {
        anchors.fill:       parent
    }

    ColumnLayout {
        id:                 topRightPanelColumnLayout
        anchors.fill:       parent
        anchors.margins:    topRightPanel.color.a ? ScreenTools.defaultFontPixelHeight / 2 : 0
        spacing:            ScreenTools.defaultFontPixelWidth * 0.75 // _layoutMargin

        MultiVehicleList {
            id:                    multiVehicleList
            Layout.fillWidth:      true
            Layout.fillHeight:     true

            Rectangle {
                anchors.fill: parent
                visible:      topRightPanel.height === maximumHeight

                Rectangle {
                    anchors.left:       parent.left
                    anchors.right:      parent.right
                    anchors.top:        parent.top
                    anchors.margins:    0
                    height:             1
                    color:              QGroundControl.globalPalette.groupBorder
                }

                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.00; color: topRightPanel.color }
                    GradientStop { position: 0.05; color: "transparent" }

                    GradientStop { position: 0.95; color: "transparent" }
                    GradientStop { position: 1.00; color: topRightPanel.color }
                }

                Rectangle {
                    anchors.left:       parent.left
                    anchors.right:      parent.right
                    anchors.bottom:     parent.bottom
                    anchors.margins:    0
                    height:             1
                    color:              QGroundControl.globalPalette.groupBorder
                }
            }

        }

        Rectangle {
            id:                     swipeViewContainer
            Layout.fillWidth:       true
            implicitHeight:         swipePages.implicitHeight
            implicitWidth:          swipePages.implicitWidth
            color:                  "transparent"

            QGCSwipeView {
                id:                swipePages
                anchors.fill:      parent
                spacing:           ScreenTools.defaultFontPixelHeight
                implicitHeight:    Math.max(buttonsPage.implicitHeight, trailsPlaybackPage.implicitHeight, photoVideoPage.implicitHeight)
                implicitWidth:     Math.max(buttonsPage.implicitWidth, trailsPlaybackPage.implicitWidth, photoVideoPage.implicitWidth)

                MvPanelPage {
                    id:                buttonsPage
                    implicitHeight:    buttonsColumnLayout.implicitHeight + ScreenTools.defaultFontPixelHeight * 2
                    implicitWidth:     buttonsColumnLayout.implicitWidth + ScreenTools.defaultFontPixelHeight * 2

                    ColumnLayout {
                        id:                     buttonsColumnLayout
                        anchors.right:          parent.right
                        anchors.left:           parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing:                ScreenTools.defaultFontPixelHeight / 2
                        implicitHeight:         Math.max(selectionRowLayout.height, actionRowLayout.height) + ScreenTools.defaultFontPixelHeight * 2
                        implicitWidth:          Math.max(selectionRowLayout.width, actionRowLayout.width) + ScreenTools.defaultFontPixelHeight * 4

                        QGCLabel {
                            text: {
                                let ids = Array.from({length: selectedVehicles.count}, (_, i) =>
                                    selectedVehicles.get(i).id
                                ).sort((a, b) => a - b)
                                .join(", ")
                                return qsTr("Vehicles Selected: ") + (ids ? ids : "-")
                            }
                            Layout.alignment:   Qt.AlignHCenter
                        }

                        RowLayout {
                            id:                 selectionRowLayout
                            Layout.alignment:   Qt.AlignHCenter

                            QGCButton {
                                text:                  qsTr("Select All")
                                enabled:               multiVehicleList.selectedVehicles && multiVehicleList.selectedVehicles.count !== QGroundControl.multiVehicleManager.vehicles.count
                                onClicked:             multiVehicleList.selectAll()
                            }

                            QGCButton {
                                text:                  qsTr("Deselect All")
                                enabled:               multiVehicleList.selectedVehicles && multiVehicleList.selectedVehicles.count > 0
                                onClicked:             multiVehicleList.deselectAll()
                            }

                        }


                        QGCLabel {
                            text:              qsTr("Multi Vehicle Actions")
                            Layout.alignment:  Qt.AlignHCenter
                        }

                        RowLayout {
                            id:                actionRowLayout
                            Layout.alignment:  Qt.AlignHCenter

                            QGCButton {
                                text:                  qsTr("Arm")
                                enabled:               multiVehicleList.armAvailable()
                                onClicked:             _guidedController.confirmAction(_guidedController.actionMVArm)
                                Layout.preferredWidth: ScreenTools.defaultFontPixelHeight * 2.75
                                leftPadding:           0
                                rightPadding:          0
                            }

                            QGCButton {
                                text:                  qsTr("Disarm")
                                enabled:               multiVehicleList.disarmAvailable()
                                onClicked:             _guidedController.confirmAction(_guidedController.actionMVDisarm)
                                Layout.preferredWidth: ScreenTools.defaultFontPixelHeight * 2.75
                                leftPadding:           0
                                rightPadding:          0
                            }

                            QGCButton {
                                text:                  qsTr("Start")
                                enabled:               multiVehicleList.startAvailable()
                                onClicked:             _guidedController.confirmAction(_guidedController.actionMVStartMission)
                                Layout.preferredWidth: ScreenTools.defaultFontPixelHeight * 2.75
                                leftPadding:           0
                                rightPadding:          0
                            }

                            QGCButton {
                                text:                  qsTr("Pause")
                                enabled:               multiVehicleList.pauseAvailable()
                                onClicked:             _guidedController.confirmAction(_guidedController.actionMVPause)
                                Layout.preferredWidth: ScreenTools.defaultFontPixelHeight * 2.75
                                leftPadding:           0
                                rightPadding:          0
                            }

                            QGCButton {
                                text:                  qsTr("Clear Trails")
                                enabled:               QGroundControl.multiVehicleManager.vehicles.count > 0
                                onClicked:             multiVehicleList.clearTrails()
                            }
                        }
                    }
                } // Page 1

                MvPanelPage {
                    id:                 trailsPlaybackPage
                    implicitHeight:     playbackColumnLayout.implicitHeight + ScreenTools.defaultFontPixelHeight * 2
                    implicitWidth:      playbackColumnLayout.implicitWidth + ScreenTools.defaultFontPixelHeight * 2

                    property var            _recorder:          QGroundControl.multiVehicleManager.flightPlaybackRecorder
                    readonly property int   _stepMs:            1000
                    readonly property int   _tickIntervalMs:    5000

                    function _formatTime(ms) {
                        var totalSeconds = Math.floor(ms / 1000)
                        var seconds = totalSeconds % 60
                        return Math.floor(totalSeconds / 60) + ":" + (seconds < 10 ? "0" : "") + seconds
                    }

                    ColumnLayout {
                        id:                     playbackColumnLayout
                        anchors.right:          parent.right
                        anchors.left:           parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        spacing:                ScreenTools.defaultFontPixelHeight / 2

                        QGCLabel {
                            text:              qsTr("Flight Playback")
                            Layout.alignment:  Qt.AlignHCenter
                        }

                        RowLayout {
                            Layout.alignment:  Qt.AlignHCenter

                            QGCButton {
                                text:       qsTr("Record")
                                iconSource: "/res/PlaybackRecord.svg"
                                enabled:    !trailsPlaybackPage._recorder.recording && !trailsPlaybackPage._recorder.playbackActive
                                onClicked:  trailsPlaybackPage._recorder.startRecording()
                            }

                            QGCButton {
                                text:       qsTr("Stop")
                                iconSource: "/res/PlaybackStop.svg"
                                enabled:    trailsPlaybackPage._recorder.recording
                                onClicked:  trailsPlaybackPage._recorder.stopRecording()
                            }

                            QGCButton {
                                text:       qsTr("Play")
                                iconSource: "/res/PlaybackPlay.svg"
                                checked:    trailsPlaybackPage._recorder.playbackActive
                                enabled:    !trailsPlaybackPage._recorder.recording && trailsPlaybackPage._recorder.hasRecording
                                onClicked:  trailsPlaybackPage._recorder.playbackActive = !trailsPlaybackPage._recorder.playbackActive
                            }
                        }

                        QGCSlider {
                            id:                 playbackSlider
                            Layout.fillWidth:   true
                            from:               0
                            to:                 trailsPlaybackPage._recorder.durationMs
                            stepSize:           trailsPlaybackPage._stepMs
                            snapMode:           Slider.SnapAlways
                            value:              trailsPlaybackPage._recorder.playbackPositionMs
                            enabled:            trailsPlaybackPage._recorder.playbackActive
                            onMoved:            trailsPlaybackPage._recorder.playbackPositionMs = value
                        }

                        Item {
                            id:                     tickRow
                            Layout.fillWidth:       true
                            Layout.preferredHeight: ScreenTools.defaultFontPixelHeight / 2

                            Repeater {
                                model: playbackSlider.to > 0 ? Math.floor(playbackSlider.to / trailsPlaybackPage._tickIntervalMs) + 1 : 0

                                Rectangle {
                                    width:  1
                                    height: tickRow.height
                                    color:  qgcPal.text
                                    x:      playbackSlider.leftPadding + playbackSlider.handle.width / 2
                                            + (index * trailsPlaybackPage._tickIntervalMs / playbackSlider.to)
                                              * (playbackSlider.availableWidth - playbackSlider.handle.width)
                                            - width / 2
                                }
                            }
                        }

                        QGCLabel {
                            text:              trailsPlaybackPage._formatTime(trailsPlaybackPage._recorder.playbackPositionMs)
                                               + " / " + trailsPlaybackPage._formatTime(trailsPlaybackPage._recorder.durationMs)
                            Layout.alignment:  Qt.AlignHCenter
                        }
                    }
                } // Page 2

                MvPanelPage {

                    id:                  photoVideoPage
                    implicitHeight:      photoVideoControlLoader.implicitHeight + ScreenTools.defaultFontPixelHeight * 2
                    implicitWidth:       photoVideoControlLoader.implicitWidth + ScreenTools.defaultFontPixelHeight * 2

                    // We use a Loader to load the photoVideoControlComponent only when the active vehicle is not null
                    // This make it easier to implement PhotoVideoControl without having to check for the mavlink camera
                    // to be null all over the place

                    Loader {
                        id:                         photoVideoControlLoader
                        anchors.horizontalCenter:   parent.horizontalCenter
                        sourceComponent:            globals.activeVehicle ? photoVideoControlComponent : undefined

                        property real rightEdgeCenterInset: visible ? parent.width - x : 0

                        Component {
                            id: photoVideoControlComponent

                            PhotoVideoControl {
                            }
                        }
                    }
                } // Page 3
            } // QGCSwipeView

            QGCPageIndicator {
                id:                       pageIndicator
                count:                    swipePages.count
                currentIndex:             swipePages.currentIndex
                anchors.bottom:           parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.margins:          ScreenTools.defaultFontPixelHeight / 4

                delegate: Rectangle {
                    height:    ScreenTools.defaultFontPixelHeight  / 2
                    width:     height
                    radius:    width / 2
                    color:     model.index === pageIndicator.currentIndex ? qgcPal.text : qgcPal.button
                    opacity:   model.index === pageIndicator.currentIndex ? 0.9 : 0.3
                }
            }
        }
    }
}
