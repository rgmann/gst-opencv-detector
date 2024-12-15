
### Dependencies

sudo apt-get install libopencv-dev

`git clone https://github.com/raspberrypi/libcamera.git rpi_libcamera`


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
