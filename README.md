
# Gstreamer Computer Vision Detection Plugin (gst-opencv-detector)

This project implements a Gstreamer plugin wrapper for OpenCV object detection models. The pre-trained detection model is passed to the plugin as props and the plugin processes the input raw video stream frame-by-frame. Optionally, detections are annotated on the output frame. Detections are also published to subscribers via TCP.

## Motivation

This project was created specifically to enable OpenCV-based object detection on a Raspberry PI running Ubuntu with an attached Camera Module 3. As of time of writing, ROS2 cannot be natively installed on Raspbian (there are Docker workarounds) and Picamera2 cannot be installed natively on Ubuntu.

## Getting Started

Start by installing the dependencies:

 1. sudo apt-get install -y libopencv-dev
 2. sudo apt-get install -y libglib2.0-dev libgstreamer-plugins-base1.0-dev
 3. Clone, build, and install the `raspberrypi` fork of [libcamera](https://libcamera.org). See instructions [here](https://github.com/raspberrypi/libcamera).

Clone and build this project:

 1. `git clone `
 2. `cd gst-opencv-detector`
 3. `meson setup build`
 4. `ninja -C build`

## Settings

## Usage

### Dependencies


### Test Commands

For Default libcamera

See libcamera caps
`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/rpi_libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/usr/local/lib/aarch64-linux-gnu/gstreamer-1.0 gst-device-monitor-1.0 Video`

Stream video to video sink
`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/usr/local/lib/aarch64-linux-gnu/gstreamer-1.0:/home/rgmann/Development/gst-opencv-server/build/src gst-launch-1.0 libcamerasrc ! 'video/x-raw,width=1280,height=720' ! queue ! glimagesink`

Stream video to video sink
`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/usr/local/lib/aarch64-linux-gnu/gstreamer-1.0:/home/rgmann/Development/gst-opencv-server/build/src gst-launch-1.0 libcamerasrc auto-focus-mode=AfModeAuto ! 'video/x-raw,width=1280,height=720' ! queue ! glimagesink`

Stream video with opencv in the loop 
`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/usr/local/lib/aarch64-linux-gnu/gstreamer-1.0:/home/rgmann/Development/gst-opencv-server/build/src gst-launch-1.0 libcamerasrc auto-focus-mode=AfModeAuto ! 'video/x-raw,format=BGR,width=1280,height=720' ! queue ! open_cvserver c ! queue ! glimagesink`



For raspberrypi/libcamera

See libcamera caps
`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/rpi_libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/home/rgmann/Development/rpi_libcamera/build/src/gstreamer gst-device-monitor-1.0 Video`

`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/rpi_libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/home/rgmann/Development/rpi_libcamera/build/src/gstreamer gst-launch-1.0 libcamerasrc ! 'video/x-raw,width=1280,height=720' ! queue ! glimagesink`

`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/rpi_libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/home/rgmann/Development/rpi_libcamera/build/src/gstreamer:/home/rgmann/Development/gst-opencv-server/build/src gst-launch-1.0 libcamerasrc ! 'video/x-raw,format=BGR,width=1280,height=720' ! queue ! open_cvserver configs=/home/rgmann/Development/gst-opencv-server/config/ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt weights=/home/rgmann/Development/gst-opencv-server/config/frozen_inference_graph.pb classes=/home/rgmann/Development/gst-opencv-server/config/coco.names ! queue ! fakesink`

`sudo LIBCAMERA_IPA_MODULE_PATH=/home/rgmann/Development/rpi_libcamera/build/src/ipa/rpi/vc4:/usr/local/share/libcamera/ipa/rpi/vc4/ LIBCAMERA_LOG_LEVELS=*:DEBUG GST_PLUGIN_PATH=/home/rgmann/Development/rpi_libcamera/build/src/gstreamer:/home/rgmann/Development/gst-opencv-server/build/src gst-launch-1.0 libcamerasrc auto-focus-mode=AfModeContinuous ! 'video/x-raw,format=BGR,width=1280,height=720' ! queue ! open_cvserver configs=/home/rgmann/Development/gst-opencv-server/config/ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt weights=/home/rgmann/Development/gst-opencv-server/config/frozen_inference_graph.pb classes=/home/rgmann/Development/gst-opencv-server/config/coco.names ! queue ! glimagesink`
