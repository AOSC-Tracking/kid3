import QtQuick 1.1
import Kid3App 1.0

Rectangle {
  id: root
  visible: true
  width: 640; height: 800

  FrameEditorObject {
    id: frameEditor
  }

  ScriptUtils {
    id: script
  }

  function centerOnRoot(item) {
    item.x = root.x + (root.width - item.width) / 2
    item.y = root.y + (root.height - item.height) / 2
  }

  FrameSelectDialog {
    id: frameSelectDialog
    onVisibleChanged: centerOnRoot(frameSelectDialog)

    onFrameSelected: frameEditor.onFrameSelectionFinished(name)

    Connections {
      target: frameEditor
      onFrameSelectionRequested: frameSelectDialog.open(frameNames)
    }
  }

  FrameEditDialog {
    id: frameEditDialog
    onVisibleChanged: centerOnRoot(frameEditDialog)
    onFrameEdited: frameEditor.onFrameEditFinished(frame)

    Connections {
      target: frameEditor
      onFrameEditRequested: frameEditDialog.open(frame)
    }
  }

  MessageDialog {
    property bool doNotRevert: false

    signal completed(bool ok)

    id: saveModifiedDialog
    onVisibleChanged: centerOnRoot(saveModifiedDialog)
    title: qsTr("Warning")
    text: qsTr("The current directory has been modified.\n" +
               "Do you want to save it?")
    onYes: {
      app.saveDirectory()
      completed(true)
    }
    onNo: {
      if (!doNotRevert) {
        app.deselectAllFiles()
        app.revertFileModifications()
      }
      completed(true)
    }
    onRejected: completed(false)

    // Open dialog if any file modified.
    // completed(ok) is signalled with false if canceled.
    function openIfModified() {
      if (app.modified && app.dirName) {
        open()
      } else {
        completed(true)
      }
    }
  }

  Text {
    id: title
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    text: app.dirName + (app.modified ? "[modified]" : "") + " - Kid3"
  }
  Item {
    id: leftSide

    anchors.left: parent.left
    anchors.top: title.bottom
    anchors.bottom: statusLabel.top
    width: fileButtonRow.width

    Row {
      id: fileButtonRow
      anchors.left: leftSide.left
      anchors.top: leftSide.top
      Button {
        id: parentDirButton
        text: ".."
        onClicked: {
          app.openDirectory(script.getIndexRoleData(fileModel.parentModelIndex(),
                                                    "filePath"))
        }
      }
      Button {
        text: "All"
        onClicked: app.selectAllFiles()
      }
      Button {
        text: "None"
        onClicked: app.deselectAllFiles()
      }
      Button {
        text: "Prev"
        onClicked: app.previousFile()
      }
      Button {
        text: "Next"
        onClicked: app.nextFile()
      }
    }

    ListView {
      id: fileList

      anchors.left: leftSide.left
      anchors.top: fileButtonRow.bottom
      anchors.bottom: leftSide.bottom
      anchors.right: leftSide.right
      clip: true

      model: CheckableListModel {
        id: fileModel
        sourceModel: app.fileProxyModel
        selectionModel: app.fileSelectionModel
        rootIndex: app.fileRootIndex
        onCurrentRowChanged: {
          fileList.currentIndex = row
        }
      }

      delegate: Item {
        id: fileDelegate
        width: parent.width
        height: fileText.height
        CheckField {
          id: checkField
          anchors.left: parent.left
          anchors.verticalCenter: parent.verticalCenter
          onClicked: {
            // QTBUG-7932, assigning is not possible
            fileModel.setDataValue(index, "checkState",
                                   checked ? Qt.Checked : Qt.Unchecked)
          }
        }
        Binding {
          // workaround for QTBUG-31627
          // should work with "checked: checkState === Qt.Checked"
          target: checkField
          property: "checked"
          value: checkState === Qt.Checked
        }
        Image {
          id: fileImage
          anchors.left: checkField.right
          anchors.verticalCenter: parent.verticalCenter
          width: 16
          source: "image://kid3/fileicon/" + iconId
        }
        Text {
          id: fileText
          anchors.left: fileImage.right
          anchors.right: parent.right
          text: fileName
          color: fileDelegate.ListView.isCurrentItem ? "red" : "black"
          MouseArea {
            anchors.fill: parent
            onClicked: {
              fileDelegate.ListView.view.currentIndex = index
              fileModel.currentRow = index
            }
            onDoubleClicked: {
              if (fileModel.hasModelChildren(index)) {
                app.openDirectory(filePath)
              }
            }
          }
        }
      }

      Connections {
        target: app
        onFileSelectionUpdateRequested: {
          // Force focus lost to store changes.
          frameTableV1.currentIndex = -1
          frameTableV2.currentIndex = -1
          app.frameModelsToTags()
          if (app.selectionInfo.singleFileSelected) {
            app.selectionInfo.fileName = fileNameEdit.text
          }
        }
        onSelectedFilesUpdated: app.tagsToFrameModels()
      }

    }
  }
  Item {
    id: rightSide
    anchors.left: leftSide.right
    anchors.right: parent.right
    anchors.top: title.bottom
    anchors.bottom: statusLabel.top

    Text {
      id: fileDetailsLabel
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      text: qsTr("File") + ": " + app.selectionInfo.detailInfo
    }

    Text {
      id: fileNameLabel
      anchors.top: fileDetailsLabel.bottom
      text: "Name:"
    }
    TextEdit {
      id: fileNameEdit
      anchors.top: fileDetailsLabel.bottom
      anchors.left: fileNameLabel.right
      anchors.right: parent.right
      enabled: app.selectionInfo.singleFileSelected
      text: app.selectionInfo.fileName
      color: app.selectionInfo.fileNameChanged ? "red" : "black"
    }
    CheckBox {
      id: checkBoxV1
      anchors.top: fileNameEdit.bottom
      text: qsTr("Tag 1") + ": " + app.selectionInfo.tagFormatV1
      // workaround for QTBUG-31627
      // should work with "checked: app.selectionInfo.hasTagV1" with Qt >= 5.3
      Binding {
        target: checkBoxV1
        property: "checked"
        value: app.selectionInfo.hasTagV1
      }
    }
    Item {
      id: sectionV1
      anchors.top: checkBoxV1.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      height: buttonsV1.height
      visible: checkBoxV1.checked
      enabled: app.selectionInfo.tag1Used

      ListView {
        id: frameTableV1
        clip: true
        anchors.top: sectionV1.top
        anchors.bottom: buttonsV1.bottom
        anchors.left: sectionV1.left
        anchors.right: buttonsV1.left
        model: app.frameModelV1
        delegate: FrameDelegate {
          width: frameTableV1.width
          isV1: true
        }
      }
      Column {
        id: buttonsV1
        anchors.top: frameTableV1.top
        anchors.right: sectionV1.right
        width: buttonsV2.width
        Button {
          width: parent.width
          text: "To Filename"
          onClicked: app.getFilenameFromTags(script.toTagVersion(1))
        }
        Button {
          width: parent.width
          text: "From Filename"
          onClicked: app.getTagsFromFilename(script.toTagVersion(1))
        }
        Button {
          width: parent.width
          text: "From Tag 2"
          onClicked: app.copyV2ToV1()
        }
        Button {
          width: parent.width
          text: "Copy"
          onClicked: app.copyTagsV1()
        }
        Button {
          width: parent.width
          text: "Paste"
          onClicked: app.pasteTagsV1()
        }
        Button {
          width: parent.width
          text: "Remove"
          onClicked: app.removeTagsV1()
        }
      }
    }
    CheckBox {
      id: checkBoxV2
      anchors.top: sectionV1.visible ? sectionV1.bottom : checkBoxV1.bottom
      text: qsTr("Tag 2") + ": " + app.selectionInfo.tagFormatV2
      // workaround for QTBUG-31627
      // should work with "checked: app.selectionInfo.hasTagV2" with Qt >= 5.3
      Binding {
        target: checkBoxV2
        property: "checked"
        value: app.selectionInfo.hasTagV2
      }
    }
    Item {
      id: sectionV2
      anchors.top: checkBoxV2.bottom
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      visible: checkBoxV2.checked

      ListView {
        id: frameTableV2
        clip: true
        anchors.top: sectionV2.top
        anchors.bottom: sectionV2.bottom
        anchors.left: sectionV2.left
        anchors.right: buttonsV2.left
        model: app.frameModelV2
        delegate: FrameDelegate {
          width: frameTableV2.width
        }
      }
      Column {
        id: buttonsV2
        anchors.top: frameTableV2.top
        anchors.right: sectionV2.right
        width: coverArtImage.width
        Button {
          width: parent.width
          text: "To Filename"
          onClicked: app.getFilenameFromTags(script.toTagVersion(2))
        }
        Button {
          width: parent.width
          text: "From Filename"
          onClicked: app.getTagsFromFilename(script.toTagVersion(2))
        }
        Button {
          width: parent.width
          text: "From Tag 1"
          onClicked: app.copyV1ToV2()
        }
        Button {
          width: parent.width
          text: "Copy"
          onClicked: app.copyTagsV2()
        }
        Button {
          width: parent.width
          text: "Paste"
          onClicked: app.pasteTagsV2()
        }
        Button {
          width: parent.width
          text: "Remove"
          onClicked: app.removeTagsV2()
        }
        Button {
          width: parent.width
          text: "Edit"
          onClicked: {
            app.frameList.selectByRow(frameTableV2.currentIndex)
            app.editFrame()
          }
        }
        Button {
          width: parent.width
          text: "Add"
          onClicked: {
            app.selectAndAddFrame()
          }
        }
        Button {
          width: parent.width
          text: "Delete"
          onClicked: {
            app.frameList.selectByRow(frameTableV2.currentIndex)
            app.deleteFrame()
          }
        }
        Image {
          id: coverArtImage
          width: 120
          sourceSize.width: 120
          sourceSize.height: 120
          source: app.coverArtImageId
          cache: false
        }
      }
    }
  }

  Text {
    id: statusLabel
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: controls.top
    text: "Ready."
  }
  Flow {
    id: controls
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    Button {
      text: qsTr("Save")
      onClicked: {
        var errorFiles = app.saveDirectory()
        if (errorFiles.length > 0) {
          console.debug("Save error:" + errorFiles)
        }
      }
    }
    Button {
      text: qsTr("Revert")
      onClicked: app.revertFileModifications()
    }
    Button {
      text: qsTr("Quit")
      onClicked: {
        saveModifiedDialog.doNotRevert = true
        saveModifiedDialog.completed.connect(quitIfCompleted)
        saveModifiedDialog.openIfModified()
      }

      function quitIfCompleted(ok) {
        saveModifiedDialog.completed.disconnect(quitIfCompleted)
        if (ok) {
          Qt.quit()
        }
      }
    }
    Button {
      text: qsTr("Apply Filename Format")
      onClicked: app.applyFilenameFormat()
    }
    Button {
      text: qsTr("Apply Tag Format")
      onClicked: app.applyId3Format()
    }
    Button {
      text: qsTr("Apply Text Encoding")
      onClicked: app.applyTextEncoding()
    }
    Button {
      text: qsTr("Convert ID3v2.3 to ID3v2.4")
      onClicked: app.convertToId3v24()
    }
    Button {
      text: qsTr("Convert ID3v2.4 to ID3v2.3")
      onClicked: app.convertToId3v23()
    }
  }

  Component.onCompleted: {
    app.frameEditor = frameEditor
    app.openDirectory("/home/urs/projects/kid3/test/testtags")
  }

  Connections {
    target: app

    onConfirmedOpenDirectoryRequested: {
      saveModifiedDialog.doNotRevert = false
      saveModifiedDialog.completed.connect(openIfCompleted)
      saveModifiedDialog.openIfModified()

      function openIfCompleted(ok) {
        saveModifiedDialog.completed.disconnect(openIfCompleted)
        if (ok) {
          app.openDirectory(paths)
        }
      }
    }
  }
}
