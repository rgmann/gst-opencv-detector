
# gst-opencv-detector - Gstreamer Computer Vision Plugin

<img align="cener" alt="Project Status: Alpha" src="https://img.shields.io/badge/Status-Alpha-red">

Welcome to the `gst-opencv-detector` project!  

`gst-opencv-detector` is a Gstreamer plugin wrapper for OpenCV object detection. Given a pre-trained detection model, the plugin receives raw video frames on its _sink_ pad, uses the model to identify detections within the frame, and then transmits the detections to all subscribed clients (connected via TCP sockets). The plugin pushes unmodified frames to the _source_ pad so that downstream Gstreamer plugins can do additional processing. If enabled, the plugin can also annotate the frame to identify the objects that have been detected.

## Motivation

As of time of writing, ROS2 cannot be natively installed on Raspbian (Docker-based workarounds do exist) and Picamera2 cannot be installed natively on Ubuntu. This project was created specifically to enable OpenCV-based object detection on a Raspberry PI running Ubuntu with an attached Camera Module 3.

## Getting Started

Start by installing the dependencies:

 1. sudo apt-get install -y libopencv-dev
 2. sudo apt-get install -y libglib2.0-dev libgstreamer-plugins-base1.0-dev
 3. Clone, build, and install the `raspberrypi` fork of [libcamera](https://libcamera.org). See instructions [here](https://github.com/raspberrypi/libcamera).

Clone and build this project:

 1. `git clone https://github.com/rgmann/gst-opencv-detector.git`
 2. `cd gst-opencv-detector`
 3. `meson setup build`
 4. `ninja -C build`

## Settings

`config=<path to config file>` (REQUIRED)  
Path to OpenCV net configuration file.

`weights=<path to weights file>` (REQUIRED)  
Path to OpenCV net weights file.

`classes=<path to class name file>` (REQUIRED)  
Path to class names file.

`annotate=<TRUE|FALSE>` (default=FALSE)  
Enable or disable detected object annotation.

`confidence-threshold=<[0.0, 1.0]>` (default=0.5)  
Inference confidence threshold.

`nms-threshold=<[0.0, 1.0]>` (default=0.1)  
Non-maximum suppression threshold

`port=<port number>` (default=0)  
TCP port number used to publish the detection list. If a port number is not specified, then the detections server is not started.

`max-subscribers` (default=1)
Maximum number of clients that may subscribe to the detections server at once.


## Usage

The following example shows how to launch a Gstreamer pipeline that captures frames from a connected PI Camera 3 module, perform object detection with annotation enabled, and display the annotated result in a GL image window on the device itself. The example assumes you have an evironment variable called `GST_OPENCV_DETECTOR` set to point to the root of this project.

```
gst-launch-1.0 libcamerasrc ! 'video/x-raw,format=BGR,width=1280,height=720' ! queue ! opencv_detector configs=${GST_OPENCV_DETECTOR}/config/ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt weights=${GST_OPENCV_DETECTOR}/config/frozen_inference_graph.pb classes=${GST_OPENCV_DETECTOR}/config/coco.names port=5050 ! queue ! glimagesink
```

For a headless configuration, you would run the following:

```
gst-launch-1.0 libcamerasrc ! 'video/x-raw,format=BGR,width=1280,height=720' ! queue ! opencv_detector configs=${GST_OPENCV_DETECTOR}/config/ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt weights=${GST_OPENCV_DETECTOR}/config/frozen_inference_graph.pb classes=${GST_OPENCV_DETECTOR}/config/coco.names port=5050 ! queue ! fakesink
```

If you want to see the annotated image for debugging purposes, but don't have a display directly connected to the device, you stream the frame and start a client elsewhere to receive and display the annotated frames.

## Detections Server

The `gst-opencv-detector` plugin is bundled with a server that publishes detected objects to all connected clients on the port specified in the plugin settings. Detections are published as flatbuffers `DetectionList` packets with a string-encoded packet size preamble. The flatbuffers schema can be found in src/schema, with header/extension code for the schema getting generated at build time. Once you have build the project, generated flatbuffers code can be found in the build/src/generated directory.

At any time, the detection server will allow up to `max-subscribers` TCP clients to connect and subscribe to receive `DetectionList` packets.

For the complete definition of the `DetectionList` packet, please see the [schema](src/schema/detections_list.fbs).

### How do I subscribe to detections in Python?

Please refer to the [detections_client.py](examples/detections_client.py) example.

### How do I subscribe to detections in C++?

Please refer to the [ros2_bridge.cpp](utils/ros2_bridge.cpp) example.


