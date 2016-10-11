import QtQuick 2.5
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtMultimedia 5.4
import Plugin 1.0

ApplicationWindow {
    property string photoPath: ""

    property string photoPathTest: "/storage/emulated/0/DCIM/25.jpg"

    //property bool b_test: false//相機

    property bool b_test: true

    id: mainWindow
    width: 180
    height: 320

    visible: true

    StackView
    {
        id: stack
        anchors.fill: parent
        initialItem: cameraWindow
    }

    Plugin
    {
        id: plugin
    }

    Component
    {
        id: cameraWindow

        Rectangle
        {
            Rectangle
            {
                width: parent.width
                height: parent.height*0.8
                color: "#CFB689"

                Camera
                {
                    id: camera
                    position: Camera.FrontFace

                    imageCapture
                    {
                        onImageCaptured:
                        {
                            var imagePath = camera.imageCapture.capturedImagePath;
                            if(imagePath != "")
                            {
                                if(!b_test)
                                {
                                    mainWindow.photoPath = imagePath
                                    stack.push(finishCaptureWindow)
                                    console.log("use imagePath!!!" + b_test)
                                }
                            }
                            else
                            {
                                console.log("reCapture")
                                camera.imageCapture.capture()
                                imagePath = camera.imageCapture.capturedImagePath;
                                var startIdx = imagePath.lastIndexOf("_")
                                var endIdx = imagePath.lastIndexOf(".")
                                var oldImageName = imagePath.slice(startIdx+1, endIdx)
                                var oldImageNumber = parseInt(oldImageName)
                                var newImageNumber = oldImageNumber+1
                                var newImageName = oldImageName.replace(oldImageNumber.toString(), newImageNumber.toString());
                                imagePath = imagePath.replace(oldImageName, newImageName)
                            }
                        }
                    }
                }

                VideoOutput
                {
                    source: camera
                    anchors.fill: parent
                    focus : visible
                    autoOrientation: true
                }
            }
            Rectangle
            {
                y: parent.height*0.8
                width: parent.width
                height: parent.height*0.2
                color: "#CFB689"

                Text
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Click here to take a picture"
                    font.pointSize: 20
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        if(!mainWindow.b_test)
                            camera.imageCapture.capture()
                        else
                        {
                            mainWindow.photoPath = photoPathTest
                            stack.push(finishCaptureWindow)
                            console.log("use our test path")
                        }
                    }
                }
            }
        }
    }

    Component
    {
        id: finishCaptureWindow

        Rectangle
        {

            Rectangle
            {
                id: rectOfPhoto
                width: parent.width
                height: parent.height*0.8
                color: "#CFB689"

                Image
                {
                    id: imgShowPhoto
                    anchors.fill: parent
                    source: "file://" + mainWindow.photoPath
                    fillMode: Image.PreserveAspectFit
                    autoTransform: true
                }
            }

            Rectangle
            {
                id: rectOfYesOrNo
                y: parent.height*0.8
                width: parent.width
                height: parent.height*0.2
                color: "#CFB689"

                Image
                {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width*0.5
                    source: "../imageFiles/icon_recapture.png"
                    sourceSize.width: parent.height
                    sourceSize.height: parent.height
                    fillMode: Image.PreserveAspectFit

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            imgShowPhoto.source=""
                            stack.pop()
                        }
                    }
                }

                Image
                {
                    anchors.verticalCenter: parent.verticalCenter
                    x: parent.width*0.5
                    width: parent.width*0.5
                    source: "../imageFiles/ok.png"
                    sourceSize.width: parent.height
                    sourceSize.height: parent.height
                    fillMode: Image.PreserveAspectFit

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            if(!mainWindow.b_test)
                                plugin.findFeatures(mainWindow.photoPath)
                            else
                                //plugin.test_SetFeaturePos();
                                plugin.findFeatures(mainWindow.photoPath)
//                            plugin.findFeatures(mainWindow.photoPath)
                            stack.push(featureWindow)
                        }
                    }
                }
            }

            Row
            {
                id: loading
                visible: false
                anchors.centerIn: parent
                spacing: 60

                BusyIndicator
                {
                    scale: 2.0
                }

                Text
                {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Feature Tracking..."
                    font.pointSize: 20
                }
            }
        }
    }

    Component
    {
        id: featureWindow

        Rectangle
        {            
            property var featurePosition: plugin.getFeaturePos()
            property real scaleW: parent.width/imgSource.sourceSize.width
            property real scaleH: parent.height/imgSource.sourceSize.height
            property real diffX: (parent.width-scaleH*imgSource.sourceSize.width)/2
            property real diffY: (parent.height-scaleW*imgSource.sourceSize.height)/2
            color: "#CFB689"
            id: rectOfFeatureWindow            

            Component.onCompleted:
            {
                console.log("imgFeature initialize")
                if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                {
                    console.log("scaleW < scaleH")
                    imgFeature1.x = featurePosition[0]*rectOfFeatureWindow.scaleW - imgFeature1.sourceSize.height
                    imgFeature1.y = featurePosition[1]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature1.sourceSize.width/1.5
                    imgFeature2.x = featurePosition[2]*rectOfFeatureWindow.scaleW - imgFeature2.sourceSize.height
                    imgFeature2.y = featurePosition[3]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature2.sourceSize.width/1.5
                    imgFeature3.x = featurePosition[4]*rectOfFeatureWindow.scaleW - imgFeature3.sourceSize.height
                    imgFeature3.y = featurePosition[5]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature3.sourceSize.width/1.5
                    imgFeature4.x = featurePosition[6]*rectOfFeatureWindow.scaleW - imgFeature4.sourceSize.height
                    imgFeature4.y = featurePosition[7]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature4.sourceSize.width/1.5
                    imgFeature5.x = featurePosition[8]*rectOfFeatureWindow.scaleW - imgFeature5.sourceSize.height
                    imgFeature5.y = featurePosition[9]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature5.sourceSize.width/1.5
                    imgFeature6.x = featurePosition[10]*rectOfFeatureWindow.scaleW - imgFeature6.sourceSize.width/2
                    imgFeature6.y = featurePosition[11]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature6.sourceSize.width/6
                    imgFeature7.x = featurePosition[12]*rectOfFeatureWindow.scaleW - imgFeature7.sourceSize.width/2
                    imgFeature7.y = featurePosition[13]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature7.sourceSize.width/6
                    imgFeature8.x = featurePosition[14]*rectOfFeatureWindow.scaleW - imgFeature8.sourceSize.width/2
                    imgFeature8.y = featurePosition[15]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature8.sourceSize.width/6
                    imgFeature9.x = featurePosition[16]*rectOfFeatureWindow.scaleW - imgFeature9.sourceSize.width/2
                    imgFeature9.y = featurePosition[17]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature9.sourceSize.width/6
                    imgFeature10.x = featurePosition[18]*rectOfFeatureWindow.scaleW - imgFeature10.sourceSize.width/2
                    imgFeature10.y = featurePosition[19]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature10.sourceSize.width/6
                    imgFeature11.x = featurePosition[20]*rectOfFeatureWindow.scaleW + imgFeature11.sourceSize.width/3
                    imgFeature11.y = featurePosition[21]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature11.sourceSize.width/1.5
                    imgFeature12.x = featurePosition[22]*rectOfFeatureWindow.scaleW + imgFeature12.sourceSize.width/3
                    imgFeature12.y = featurePosition[23]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature12.sourceSize.width/1.5
                    imgFeature13.x = featurePosition[24]*rectOfFeatureWindow.scaleW + imgFeature13.sourceSize.width/3
                    imgFeature13.y = featurePosition[25]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature13.sourceSize.width/1.5
                    imgFeature14.x = featurePosition[26]*rectOfFeatureWindow.scaleW + imgFeature14.sourceSize.width/3
                    imgFeature14.y = featurePosition[27]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature14.sourceSize.width/1.5
                    imgFeature15.x = featurePosition[28]*rectOfFeatureWindow.scaleW + imgFeature15.sourceSize.width/3
                    imgFeature15.y = featurePosition[29]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature15.sourceSize.width/1.5
                }
                else
                {
                    console.log("scaleW > scaleH")
                    imgFeature1.x = featurePosition[0]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature1.sourceSize.height
                    imgFeature1.y = featurePosition[1]*rectOfFeatureWindow.scaleH - imgFeature1.sourceSize.width/1.5
                    imgFeature2.x = featurePosition[2]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature2.sourceSize.height
                    imgFeature2.y = featurePosition[3]*rectOfFeatureWindow.scaleH - imgFeature2.sourceSize.width/1.5
                    imgFeature3.x = featurePosition[4]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature3.sourceSize.height
                    imgFeature3.y = featurePosition[5]*rectOfFeatureWindow.scaleH - imgFeature3.sourceSize.width/1.5
                    imgFeature4.x = featurePosition[6]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature4.sourceSize.height
                    imgFeature4.y = featurePosition[7]*rectOfFeatureWindow.scaleH - imgFeature4.sourceSize.width/1.5
                    imgFeature5.x = featurePosition[8]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature5.sourceSize.height
                    imgFeature5.y = featurePosition[9]*rectOfFeatureWindow.scaleH - imgFeature5.sourceSize.width/1.5
                    imgFeature6.x = featurePosition[10]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature6.sourceSize.width/2
                    imgFeature6.y = featurePosition[11]*rectOfFeatureWindow.scaleH + imgFeature6.sourceSize.width/6
                    imgFeature7.x = featurePosition[12]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature7.sourceSize.width/2
                    imgFeature7.y = featurePosition[13]*rectOfFeatureWindow.scaleH + imgFeature7.sourceSize.width/6
                    imgFeature8.x = featurePosition[14]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature8.sourceSize.width/2
                    imgFeature8.y = featurePosition[15]*rectOfFeatureWindow.scaleH + imgFeature8.sourceSize.width/6
                    imgFeature9.x = featurePosition[16]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature9.sourceSize.width/2
                    imgFeature9.y = featurePosition[17]*rectOfFeatureWindow.scaleH + imgFeature9.sourceSize.width/6
                    imgFeature10.x = featurePosition[18]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature10.sourceSize.width/2
                    imgFeature10.y = featurePosition[19]*rectOfFeatureWindow.scaleH + imgFeature10.sourceSize.width/6
                    imgFeature11.x = featurePosition[20]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature11.sourceSize.width/3
                    imgFeature11.y = featurePosition[21]*rectOfFeatureWindow.scaleH - imgFeature11.sourceSize.width/1.5
                    imgFeature12.x = featurePosition[22]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature12.sourceSize.width/3
                    imgFeature12.y = featurePosition[23]*rectOfFeatureWindow.scaleH - imgFeature12.sourceSize.width/1.5
                    imgFeature13.x = featurePosition[24]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature13.sourceSize.width/3
                    imgFeature13.y = featurePosition[25]*rectOfFeatureWindow.scaleH - imgFeature13.sourceSize.width/1.5
                    imgFeature14.x = featurePosition[26]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature14.sourceSize.width/3
                    imgFeature14.y = featurePosition[27]*rectOfFeatureWindow.scaleH - imgFeature14.sourceSize.width/1.5
                    imgFeature15.x = featurePosition[28]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature15.sourceSize.width/3
                    imgFeature15.y = featurePosition[29]*rectOfFeatureWindow.scaleH - imgFeature15.sourceSize.width/1.5
                }
            }

            Image
            {
                id: imgSource
                width: parent.width
                height: parent.height
                fillMode: Image.PreserveAspectFit
                source: "file://" + mainWindow.photoPath
                autoTransform: true
            }

            Canvas
            {
                property var context: canvasOfDrawFeatures.getContext('2d')

                id: canvasOfDrawFeatures
                anchors.fill: parent

                onPaint:
                {      
                    if(featurePosition[0] !== -1)
                    {
                        canvasOfDrawFeatures.context.clearRect(0, 0, canvasOfDrawFeatures.width, canvasOfDrawFeatures.height)

                        for(var i=2; i<140; i+=2)
                        {
                            context.beginPath()
                            context.fillStyle = "#00FF00" //green
                            if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                                context.arc(featurePosition[i]*rectOfFeatureWindow.scaleW, featurePosition[i+1]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY, 5, 0, 2*Math.PI, true)
                            else
                                context.arc(featurePosition[i]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX, featurePosition[i+1]*rectOfFeatureWindow.scaleH, 5, 0, 2*Math.PI, true)
                            context.fill()
                            context.stroke()
                        }

                        for(var j=0; j<2; j+=2)
                        {
                            context.beginPath()
                            context.fillStyle = "red" //red
                            if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                                context.arc(featurePosition[j]*rectOfFeatureWindow.scaleW, featurePosition[j+1]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY, 5, 0, 2*Math.PI, true)
                            else
                                context.arc(featurePosition[j]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX, featurePosition[j+1]*rectOfFeatureWindow.scaleH, 5, 0, 2*Math.PI, true)
                            context.fill()
                            context.stroke()
                        }
                    }
                }
            }

            Canvas
            {
                property var context: canvasOfSelectFace.getContext('2d')
                property bool clear: false

                id: canvasOfSelectFace
                anchors.fill: parent

                onPaint:
                {
                    canvasOfSelectFace.context.clearRect(0, 0, canvasOfSelectFace.width, canvasOfSelectFace.height)                    
                    if(!clear)
                    {
                        context.beginPath()
                        context.strokeStyle = "red"
                        context.lineWidth = 3
                        context.rect(mouseAreaOfSelectFace.startP.x, mouseAreaOfSelectFace.startP.y, mouseAreaOfSelectFace.endP.x - mouseAreaOfSelectFace.startP.x, mouseAreaOfSelectFace.endP.y - mouseAreaOfSelectFace.startP.y)
                        context.stroke()
                    }
                    clear = false;
                }
            }

            MouseArea
            {
                property point startP: Qt.point(0, 0)
                property point endP: Qt.point(0, 0)

                id: mouseAreaOfSelectFace
                anchors.fill: parent
                enabled: false

                onPressed:
                {
                    startP = Qt.point(mouse.x, mouse.y)
                    btnTrackFace.visible = true
                }

                onPositionChanged:
                {
                    endP = Qt.point(mouse.x, mouse.y)
                    canvasOfSelectFace.requestPaint()
                }
            }

            Image
            {
                property int sw: 0

                id: btnShowFeatures
                source: "../imageFiles/icon_showFeatures.png"
                sourceSize.width: parent.width*0.2
                sourceSize.height: parent.width*0.2
                opacity: 0.5

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {                       
                        if(featurePosition[0] !== -1)
                        {
                            if(btnShowFeatures.sw)
                            {
                                btnShowFeatures.opacity = 0.5
                                btnShowFeatures.sw = 0
                                imgFeature1.visible = false
                                imgFeature2.visible = false
                                imgFeature3.visible = false
                                imgFeature4.visible = false
                                imgFeature5.visible = false
                                imgFeature6.visible = false
                                imgFeature7.visible = false
                                imgFeature8.visible = false
                                imgFeature9.visible = false
                                imgFeature10.visible = false
                                imgFeature11.visible = false
                                imgFeature12.visible = false
                                imgFeature13.visible = false
                                imgFeature14.visible = false
                                imgFeature15.visible = false
                            }
                            else
                            {
                                btnShowFeatures.opacity = 1
                                btnShowFeatures.sw = 1
                                btnSelectFace.opacity = 0.5
                                btnSelectFace.sw = 0
                                mouseAreaOfSelectFace.enabled = false
                                imgFeature1.visible = true
                                imgFeature2.visible = true
                                imgFeature3.visible = true
                                imgFeature4.visible = true
                                imgFeature5.visible = true
                                imgFeature6.visible = true
                                imgFeature7.visible = true
                                imgFeature8.visible = true
                                imgFeature9.visible = true
                                imgFeature10.visible = true
                                imgFeature11.visible = true
                                imgFeature12.visible = true
                                imgFeature13.visible = true
                                imgFeature14.visible = true
                                imgFeature15.visible = true
                            }
                        }
                    }
                }
            }

            Image
            {
                property int sw: 0

                id: btnSelectFace
                x: btnShowFeatures.sourceSize.width
                source: "../imageFiles/icon_rect.png"
                sourceSize.width: parent.width*0.2
                sourceSize.height: parent.width*0.2
                opacity: 0.5

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        if(btnSelectFace.sw)
                        {
                            btnSelectFace.opacity = 0.5
                            btnSelectFace.sw = 0
                            mouseAreaOfSelectFace.enabled = false
                        }
                        else
                        {
                            btnSelectFace.opacity = 1
                            btnSelectFace.sw = 1
                            btnShowFeatures.opacity = 0.5
                            btnShowFeatures.sw = 0
                            mouseAreaOfSelectFace.enabled = true
                            imgFeature1.visible = false
                            imgFeature2.visible = false
                            imgFeature3.visible = false
                            imgFeature4.visible = false
                            imgFeature5.visible = false
                            imgFeature6.visible = false
                            imgFeature7.visible = false
                            imgFeature8.visible = false
                            imgFeature9.visible = false
                            imgFeature10.visible = false
                            imgFeature11.visible = false
                            imgFeature12.visible = false
                            imgFeature13.visible = false
                            imgFeature14.visible = false
                            imgFeature15.visible = false
                        }
                    }
                }
            }

            Image
            {
                id: btnTrackFace
                x: parent.width  - btnTrackFace.sourceSize.width
                source: "../imageFiles/ok.png"
                sourceSize.width: parent.width*0.2
                sourceSize.height: parent.width*0.2
                visible: false

                MouseArea
                {
                    anchors.fill: parent

                    onClicked:
                    {
                        var sx, sy, ex, ey;
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            sx = mouseAreaOfSelectFace.startP.x / rectOfFeatureWindow.scaleW
                            sy = (mouseAreaOfSelectFace.startP.y - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                            ex = mouseAreaOfSelectFace.endP.x / rectOfFeatureWindow.scaleW
                            ey = (mouseAreaOfSelectFace.endP.y - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            sx = (mouseAreaOfSelectFace.startP.x - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            sy = mouseAreaOfSelectFace.startP.y / rectOfFeatureWindow.scaleH
                            ex = (mouseAreaOfSelectFace.endP.x / rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH;
                            ey = mouseAreaOfSelectFace.endP.y / rectOfFeatureWindow.scaleH
                        }

                        var width;

                        if((ex-sx) > (ey-sy))
                            width = ex-sx;
                        else
                            width = ey-sy;

                        canvasOfSelectFace.clear = true
                        plugin.setFaceRect(sx, sy, width, width)
                        featurePosition = plugin.getFeaturePos()
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            imgFeature1.x = featurePosition[0]*rectOfFeatureWindow.scaleW - imgFeature1.sourceSize.height
                            imgFeature1.y = featurePosition[1]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature1.sourceSize.width/1.5
                            imgFeature2.x = featurePosition[2]*rectOfFeatureWindow.scaleW - imgFeature2.sourceSize.height
                            imgFeature2.y = featurePosition[3]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature2.sourceSize.width/1.5
                            imgFeature3.x = featurePosition[4]*rectOfFeatureWindow.scaleW - imgFeature3.sourceSize.height
                            imgFeature3.y = featurePosition[5]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature3.sourceSize.width/1.5
                            imgFeature4.x = featurePosition[6]*rectOfFeatureWindow.scaleW - imgFeature4.sourceSize.height
                            imgFeature4.y = featurePosition[7]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature4.sourceSize.width/1.5
                            imgFeature5.x = featurePosition[8]*rectOfFeatureWindow.scaleW - imgFeature5.sourceSize.height
                            imgFeature5.y = featurePosition[9]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature5.sourceSize.width/1.5
                            imgFeature6.x = featurePosition[10]*rectOfFeatureWindow.scaleW - imgFeature6.sourceSize.width/2
                            imgFeature6.y = featurePosition[11]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature6.sourceSize.width/6
                            imgFeature7.x = featurePosition[12]*rectOfFeatureWindow.scaleW - imgFeature7.sourceSize.width/2
                            imgFeature7.y = featurePosition[13]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature7.sourceSize.width/6
                            imgFeature8.x = featurePosition[14]*rectOfFeatureWindow.scaleW - imgFeature8.sourceSize.width/2
                            imgFeature8.y = featurePosition[15]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature8.sourceSize.width/6
                            imgFeature9.x = featurePosition[16]*rectOfFeatureWindow.scaleW - imgFeature9.sourceSize.width/2
                            imgFeature9.y = featurePosition[17]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature9.sourceSize.width/6
                            imgFeature10.x = featurePosition[18]*rectOfFeatureWindow.scaleW - imgFeature10.sourceSize.width/2
                            imgFeature10.y = featurePosition[19]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY + imgFeature10.sourceSize.width/6
                            imgFeature11.x = featurePosition[20]*rectOfFeatureWindow.scaleW + imgFeature11.sourceSize.width/3
                            imgFeature11.y = featurePosition[21]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature11.sourceSize.width/1.5
                            imgFeature12.x = featurePosition[22]*rectOfFeatureWindow.scaleW + imgFeature12.sourceSize.width/3
                            imgFeature12.y = featurePosition[23]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature12.sourceSize.width/1.5
                            imgFeature13.x = featurePosition[24]*rectOfFeatureWindow.scaleW + imgFeature13.sourceSize.width/3
                            imgFeature13.y = featurePosition[25]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature13.sourceSize.width/1.5
                            imgFeature14.x = featurePosition[26]*rectOfFeatureWindow.scaleW + imgFeature14.sourceSize.width/3
                            imgFeature14.y = featurePosition[27]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature14.sourceSize.width/1.5
                            imgFeature15.x = featurePosition[28]*rectOfFeatureWindow.scaleW + imgFeature15.sourceSize.width/3
                            imgFeature15.y = featurePosition[29]*rectOfFeatureWindow.scaleW + rectOfFeatureWindow.diffY - imgFeature15.sourceSize.width/1.5
                        }
                        else
                        {
                            imgFeature1.x = featurePosition[0]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature1.sourceSize.height
                            imgFeature1.y = featurePosition[1]*rectOfFeatureWindow.scaleH - imgFeature1.sourceSize.width/1.5
                            imgFeature2.x = featurePosition[2]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature2.sourceSize.height
                            imgFeature2.y = featurePosition[3]*rectOfFeatureWindow.scaleH - imgFeature2.sourceSize.width/1.5
                            imgFeature3.x = featurePosition[4]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature3.sourceSize.height
                            imgFeature3.y = featurePosition[5]*rectOfFeatureWindow.scaleH - imgFeature3.sourceSize.width/1.5
                            imgFeature4.x = featurePosition[6]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature4.sourceSize.height
                            imgFeature4.y = featurePosition[7]*rectOfFeatureWindow.scaleH - imgFeature4.sourceSize.width/1.5
                            imgFeature5.x = featurePosition[8]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature5.sourceSize.height
                            imgFeature5.y = featurePosition[9]*rectOfFeatureWindow.scaleH - imgFeature5.sourceSize.width/1.5
                            imgFeature6.x = featurePosition[10]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature6.sourceSize.width/2
                            imgFeature6.y = featurePosition[11]*rectOfFeatureWindow.scaleH + imgFeature6.sourceSize.width/6
                            imgFeature7.x = featurePosition[12]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature7.sourceSize.width/2
                            imgFeature7.y = featurePosition[13]*rectOfFeatureWindow.scaleH + imgFeature7.sourceSize.width/6
                            imgFeature8.x = featurePosition[14]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature8.sourceSize.width/2
                            imgFeature8.y = featurePosition[15]*rectOfFeatureWindow.scaleH + imgFeature8.sourceSize.width/6
                            imgFeature9.x = featurePosition[16]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature9.sourceSize.width/2
                            imgFeature9.y = featurePosition[17]*rectOfFeatureWindow.scaleH + imgFeature9.sourceSize.width/6
                            imgFeature10.x = featurePosition[18]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX - imgFeature10.sourceSize.width/2
                            imgFeature10.y = featurePosition[19]*rectOfFeatureWindow.scaleH + imgFeature10.sourceSize.width/6
                            imgFeature11.x = featurePosition[20]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature11.sourceSize.width/3
                            imgFeature11.y = featurePosition[21]*rectOfFeatureWindow.scaleH - imgFeature11.sourceSize.width/1.5
                            imgFeature12.x = featurePosition[22]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature12.sourceSize.width/3
                            imgFeature12.y = featurePosition[23]*rectOfFeatureWindow.scaleH - imgFeature12.sourceSize.width/1.5
                            imgFeature13.x = featurePosition[24]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature13.sourceSize.width/3
                            imgFeature13.y = featurePosition[25]*rectOfFeatureWindow.scaleH - imgFeature13.sourceSize.width/1.5
                            imgFeature14.x = featurePosition[26]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature14.sourceSize.width/3
                            imgFeature14.y = featurePosition[27]*rectOfFeatureWindow.scaleH - imgFeature14.sourceSize.width/1.5
                            imgFeature15.x = featurePosition[28]*rectOfFeatureWindow.scaleH + rectOfFeatureWindow.diffX + imgFeature15.sourceSize.width/3
                            imgFeature15.y = featurePosition[29]*rectOfFeatureWindow.scaleH - imgFeature15.sourceSize.width/1.5
                        }
                        canvasOfSelectFace.requestPaint()
                        canvasOfDrawFeatures.requestPaint()
                        btnTrackFace.visible = false
                    }
                }
            }

            Image
            {
                id: btnNext
                x: parent.width - btnNext.sourceSize.width
                y: parent.height - btnNext.sourceSize.height
                source: "../imageFiles/icon_next.png"
                sourceSize.width: parent.width*0.2
                sourceSize.height: parent.width*0.2
                opacity: 0.5

                MouseArea
                {
                    anchors.fill: parent

                    onClicked:
                    {
                        btnNext.opacity = 1
                        for(var i=0; i< 30; i+=2)
                            plugin.setFeaturePos(i, featurePosition[i], featurePosition[i+1])
                        mainWindow.close()
                        plugin.switchToDeformWindow()
                    }
                }
            }

            Image
            {
                id: imgFeature1
                x: 0
                y: 0
                rotation: -90
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature1
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[0] = (imgFeature1.x + imgFeature1.sourceSize.height) / rectOfFeatureWindow.scaleW
                            featurePosition[1] = (imgFeature1.y + imgFeature1.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[0] = (imgFeature1.x + imgFeature1.sourceSize.height - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[1] = (imgFeature1.y + imgFeature1.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature2
                x: 0
                y: 0
                rotation: -90
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature2
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[2] = (imgFeature2.x + imgFeature2.sourceSize.height) / rectOfFeatureWindow.scaleW
                            featurePosition[3] = (imgFeature2.y + imgFeature2.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[2] = (imgFeature2.x + imgFeature2.sourceSize.height - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[3] = (imgFeature2.y + imgFeature2.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature3
                x: 0
                y: 0
                rotation: -90
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature3
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[4] = (imgFeature3.x + imgFeature3.sourceSize.height) / rectOfFeatureWindow.scaleW
                            featurePosition[5] = (imgFeature3.y + imgFeature3.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[4] = (imgFeature3.x + imgFeature3.sourceSize.height - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[5] = (imgFeature3.y + imgFeature3.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature4
                x: 0
                y: 0
                rotation: -90
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature4
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[6] = (imgFeature4.x + imgFeature4.sourceSize.height) / rectOfFeatureWindow.scaleW
                            featurePosition[7] = (imgFeature4.y + imgFeature4.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[6] = (imgFeature4.x + imgFeature4.sourceSize.height - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[7] = (imgFeature4.y + imgFeature4.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature5
                x: 0
                y: 0
                rotation: -90
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature5
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[8] = (imgFeature5.x + imgFeature5.sourceSize.height) / rectOfFeatureWindow.scaleW
                            featurePosition[9] = (imgFeature5.y + imgFeature5.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[8] = (imgFeature5.x + imgFeature5.sourceSize.height - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[9] = (imgFeature5.y + imgFeature5.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature6
                x: 0
                y: 0
                rotation: 180
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature6
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[10] = (imgFeature6.x + imgFeature6.sourceSize.width/2) / rectOfFeatureWindow.scaleW
                            featurePosition[11] = (imgFeature6.y - imgFeature6.sourceSize.width/6 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[10] = (imgFeature6.x + imgFeature6.sourceSize.width/2 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[11] = (imgFeature6.y - imgFeature6.sourceSize.width/6) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature7
                x: 0
                y: 0
                rotation: -180
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature7
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[12] = (imgFeature7.x + imgFeature7.sourceSize.width/2) / rectOfFeatureWindow.scaleW
                            featurePosition[13] = (imgFeature7.y - imgFeature7.sourceSize.width/6 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[12] = (imgFeature7.x + imgFeature7.sourceSize.width/2 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[13] = (imgFeature7.y - imgFeature7.sourceSize.width/6) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature8
                x: 0
                y: 0
                rotation: -180
                source: "../imageFiles/tack.png"
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature8
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[14] = (imgFeature8.x + imgFeature8.sourceSize.width/2) / rectOfFeatureWindow.scaleW
                            featurePosition[15] = (imgFeature8.y - imgFeature8.sourceSize.width/6 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[14] = (imgFeature8.x + imgFeature8.sourceSize.width/2 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[15] = (imgFeature8.y - imgFeature8.sourceSize.width/6) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature9
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: -180
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature9
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[16] = (imgFeature9.x + imgFeature9.sourceSize.width/2) / rectOfFeatureWindow.scaleW
                            featurePosition[17] = (imgFeature9.y - imgFeature9.sourceSize.width/6 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[16] = (imgFeature9.x + imgFeature9.sourceSize.width/2 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[17] = (imgFeature9.y - imgFeature9.sourceSize.width/6) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature10
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: -180
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature10
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[18] = (imgFeature10.x + imgFeature10.sourceSize.width/2) / rectOfFeatureWindow.scaleW
                            featurePosition[19] = (imgFeature10.y - imgFeature10.sourceSize.width/6 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[18] = (imgFeature10.x + imgFeature10.sourceSize.width/2 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[19] = (imgFeature10.y - imgFeature10.sourceSize.width/6) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature11
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: 90
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature11
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[20] = (imgFeature11.x - imgFeature11.sourceSize.width/3) / rectOfFeatureWindow.scaleW
                            featurePosition[21] = (imgFeature11.y + imgFeature11.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[20] = (imgFeature11.x - imgFeature11.sourceSize.width/3 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[21] = (imgFeature11.y + imgFeature11.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature12
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: 90
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature12
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[22] = (imgFeature12.x - imgFeature12.sourceSize.width/3) / rectOfFeatureWindow.scaleW
                            featurePosition[23] = (imgFeature12.y + imgFeature12.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[22] = (imgFeature12.x - imgFeature12.sourceSize.width/3 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[23] = (imgFeature12.y + imgFeature12.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature13
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: 90
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature13
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[24] = (imgFeature13.x - imgFeature13.sourceSize.width/3) / rectOfFeatureWindow.scaleW
                            featurePosition[25] = (imgFeature13.y + imgFeature13.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[24] = (imgFeature13.x - imgFeature13.sourceSize.width/3 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[25] = (imgFeature13.y + imgFeature13.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature14
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: 90
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature14
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[26] = (imgFeature14.x - imgFeature14.sourceSize.width/3) / rectOfFeatureWindow.scaleW
                            featurePosition[27] = (imgFeature14.y + imgFeature14.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[26] = (imgFeature14.x - imgFeature14.sourceSize.width/3 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[27] = (imgFeature14.y + imgFeature14.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }

            Image
            {
                id: imgFeature15
                x: 0
                y: 0
                source: "../imageFiles/tack.png"
                rotation: 90
                visible: false

                MouseArea
                {
                    anchors.fill: parent
                    drag.target: imgFeature15
                    drag.axis: Drag.XandYAxis
                    onPositionChanged:
                    {
                        if(rectOfFeatureWindow.scaleW < rectOfFeatureWindow.scaleH)
                        {
                            featurePosition[28] = (imgFeature15.x - imgFeature15.sourceSize.width/3) / rectOfFeatureWindow.scaleW
                            featurePosition[29] = (imgFeature15.y + imgFeature15.sourceSize.width/1.5 - rectOfFeatureWindow.diffY) / rectOfFeatureWindow.scaleW
                        }
                        else
                        {
                            featurePosition[28] = (imgFeature15.x - imgFeature15.sourceSize.width/3 - rectOfFeatureWindow.diffX) / rectOfFeatureWindow.scaleH
                            featurePosition[29] = (imgFeature15.y + imgFeature15.sourceSize.width/1.5) / rectOfFeatureWindow.scaleH
                        }
                        canvasOfDrawFeatures.requestPaint()
                    }
                }
            }
        }
    }
}
