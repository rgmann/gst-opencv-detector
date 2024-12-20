/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2024 Robert Vaughan <robert.glissmann@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-opencvdetector
 *
 * FIXME:Describe opencvdetector here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! opencvdetector ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <opencv2/dnn/dnn.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

#include "gstopencv-utils.h"
#include "object_detector.h"
#include "detections_list_server.h"

#include "gstopencvdetector.h"

GST_DEBUG_CATEGORY_STATIC (gst_opencv_detector_debug);
#define GST_CAT_DEFAULT gst_opencv_detector_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_SILENT,
    PROP_CONFIGS_PATH,
    PROP_WEIGHTS_PATH,
    PROP_CLASS_NAMES_PATH,
    PROP_ANNOTATE,
    PROP_PORT,
    PROP_CONF_THRESHOLD,
    PROP_NMS_THRESHOLD
};

struct _GstOpencvDetector
{
    GstElement element;

    GstPad *sinkpad, *srcpad;

    // Settings
    gboolean silent;
    gchar* configs_path;
    gchar* weights_path;
    gchar* class_names_path;
    gboolean annotate;
    guint port;
    float conf_threshold;
    float nms_threshold;

    // std::unique_ptr<ObjectDetector> detector_;
    ObjectDetector* detector_;

    _GstOpencvDetector()
        : sinkpad(nullptr)
        , srcpad(nullptr)
        , silent(FALSE)
        , configs_path(nullptr)
        , weights_path(nullptr)
        , annotate(TRUE)
        , detector_(nullptr)
        // , detector_(std::make_unique<ObjectDetector>())
    {

    }

    ~_GstOpencvDetector()
    {
        g_free(configs_path);
        g_free(weights_path);
    }
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
        "video/x-raw, "
            "format=(string)BGR"
    )
);

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
        "video/x-raw, "
            "format=(string)BGR"
    )
);

#define gst_opencv_detector_parent_class parent_class
G_DEFINE_TYPE (GstOpencvDetector, gst_opencv_detector, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (opencv_detector, "opencv_detector", GST_RANK_NONE,
    GST_TYPE_OPENCVDETECTOR);

static void gst_opencv_detector_finalize(GObject *object);
static void gst_opencv_detector_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_opencv_detector_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_opencv_detector_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_opencv_detector_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the opencvdetector's class */
static void
gst_opencv_detector_class_init (GstOpencvDetectorClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;

    gobject_class->set_property = gst_opencv_detector_set_property;
    gobject_class->get_property = gst_opencv_detector_get_property;
    gobject_class->finalize = gst_opencv_detector_finalize;

    g_object_class_install_property( gobject_class, PROP_SILENT,
        g_param_spec_boolean(
            "silent",
            "Silent",
            "Produce verbose output ?",
            FALSE, G_PARAM_READWRITE));
    
    g_object_class_install_property( gobject_class, PROP_CONFIGS_PATH,
        g_param_spec_string (
            "configs",
            "Configs",
            "Path to OpenCV net configuration file",
            "", G_PARAM_READWRITE));

    g_object_class_install_property( gobject_class, PROP_WEIGHTS_PATH,
        g_param_spec_string(
            "weights",
            "Weights",
            "Path to OpenCV net weights file",
            "", G_PARAM_READWRITE));

    g_object_class_install_property( gobject_class, PROP_CLASS_NAMES_PATH,
        g_param_spec_string(
            "classes",
            "Classes",
            "Path to class names file",
            "", G_PARAM_READWRITE));

    g_object_class_install_property( gobject_class, PROP_ANNOTATE,
        g_param_spec_boolean(
            "annotate",
            "Annotate",
            "Enable or disable detected object annotation.",
            TRUE, G_PARAM_READWRITE));

    g_object_class_install_property( gobject_class, PROP_PORT,
        g_param_spec_int(
            "port",
            "Port",
            "Port that detection server will opened on",
            0, 65525,
            0, G_PARAM_READWRITE));

    g_object_class_install_property( gobject_class, PROP_CONF_THRESHOLD,
        g_param_spec_int(
            "confidence-threshold",
            "Confidence Threshold",
            "Confidence threshold for object detection",
            0.1, 1.0,
            0.6, G_PARAM_READWRITE));

    g_object_class_install_property( gobject_class, PROP_NMS_THRESHOLD,
        g_param_spec_int(
            "nms-threshold",
            "Non-Maximum Suppression Threshold",
            "Non-maximum suppression threshold",
            0.1, 1.0,
            0.2, G_PARAM_READWRITE));

    gst_element_class_set_details_simple (gstelement_class,
        "OpencvDetector",
        "FIXME:Generic",
        "FIXME:Generic Template Element", "Robert Vaughan <<user@hostname.org>>");

    gst_element_class_add_pad_template (gstelement_class,
        gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (gstelement_class,
        gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_opencv_detector_init (GstOpencvDetector * filter)
{
    ObjectDetector* detector = new ObjectDetector();

    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    gst_pad_set_event_function (filter->sinkpad,
        GST_DEBUG_FUNCPTR (gst_opencv_detector_sink_event));
    gst_pad_set_chain_function (filter->sinkpad,
        GST_DEBUG_FUNCPTR (gst_opencv_detector_chain));
    GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS (filter->srcpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    filter->silent = FALSE;
    filter->annotate = TRUE;

    filter->detector_ = detector;
}

static void
gst_opencv_detector_finalize(GObject *object)
{
    GObjectClass *klass = G_OBJECT_CLASS(gst_opencv_detector_parent_class);
    GstOpencvDetector *self = GST_OPENCVDETECTOR(object);
    (void)self;

    delete self->detector_;

    return klass->finalize(object);
}

static void
gst_opencv_detector_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstOpencvDetector *filter = GST_OPENCVDETECTOR (object);

    g_print("Setting OPENCV prop: prop_id=%d\n", prop_id);

    switch (prop_id) {
    case PROP_SILENT:
        filter->silent = g_value_get_boolean (value);
        break;
    case PROP_CONFIGS_PATH:
        {
            gchar* configs_path = g_value_dup_string(value);
            if (valid_file_path(configs_path))
            {
                g_free(filter->configs_path);
                filter->configs_path = configs_path;
            }
            else
            {
                g_print("Configuration file not found at '%s'.", configs_path);
                GST_ELEMENT_WARNING(filter, RESOURCE, BUSY,
                    ("Configuration file not found at '%s'.", configs_path),
                    ("File not found."));
                g_free(configs_path);
            }
        }
        break;
    case PROP_WEIGHTS_PATH:
        {
            gchar* weights_path = g_value_dup_string(value);
            if (valid_file_path(weights_path))
            {
                g_free(filter->weights_path);
                filter->weights_path = weights_path;
            }
            else
            {
                g_print("Weights file not found at '%s'.", weights_path);
                GST_ELEMENT_WARNING(filter, RESOURCE, BUSY,
                    ("Weights file not found at '%s'.", weights_path),
                    ("File not found."));
                g_free(weights_path);
            }
        }
        break;
    case PROP_CLASS_NAMES_PATH:
        {
            gchar* class_names_path = g_value_dup_string(value);
            if (valid_file_path(class_names_path))
            {
                g_free(filter->class_names_path);
                filter->class_names_path = class_names_path;
            }
            else
            {
                g_print("Class names file not found at '%s'.", class_names_path);
                GST_ELEMENT_WARNING(filter, RESOURCE, BUSY,
                    ("Class names file not found at '%s'.", class_names_path),
                    ("File not found."));
                g_free(class_names_path);
            }
        }
        break;
    case PROP_ANNOTATE:
        filter->annotate = g_value_get_boolean(value);
        break;
    case PROP_PORT:
        filter->port = g_value_get_int(value);
        break;
    case PROP_CONF_THRESHOLD:
        filter->conf_threshold = g_value_get_float(value);
        break;
    case PROP_NMS_THRESHOLD:
        filter->nms_threshold = g_value_get_float(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_opencv_detector_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstOpencvDetector *filter = GST_OPENCVDETECTOR (object);

    switch (prop_id) {
    case PROP_SILENT:
        g_value_set_boolean(value, filter->silent);
        break;
    case PROP_CONFIGS_PATH:
        g_value_set_string(value, filter->configs_path);
        break;
    case PROP_WEIGHTS_PATH:
        g_value_set_string(value, filter->weights_path);
        break;
    case PROP_CLASS_NAMES_PATH:
        g_value_set_string(value, filter->class_names_path);
        break;
    case PROP_ANNOTATE:
        g_value_set_boolean(value, filter->annotate);
        break;
    case PROP_PORT:
        g_value_set_int(value, filter->port);
        break;
    case PROP_CONF_THRESHOLD:
        g_value_set_float(value, filter->conf_threshold);
        break;
    case PROP_NMS_THRESHOLD:
        g_value_set_float(value, filter->nms_threshold);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_opencv_detector_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
    GstOpencvDetector *filter;
    gboolean ret;

    filter = GST_OPENCVDETECTOR (parent);

    GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
            GST_EVENT_TYPE_NAME (event), event);

    switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_CAPS:
        {
            GstCaps *caps;

            gst_event_parse_caps (event, &caps);
            /* do something with the caps */
            for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                GstStructure* s = gst_caps_get_structure(caps, i);

                // Caps are expected to appear in the following format:
                // video/x-raw,
                //     format=(string)NV21,
                //     width=(int)1280,
                //     height=(int)720,
                //     colorimetry=(string)bt709,
                //     framerate=(fraction)30/1;

                g_print("Received caps for %s\n", gst_structure_get_name(s));

                if (gst_structure_has_name(s, "video/x-raw"))
                {
                    gchar* serialized = gst_structure_to_string(s);
                    g_print("  Serialized: <begin>%s<end>\n", serialized);
                    g_free(serialized);

                    if (gst_structure_has_field(s, "format"))
                    {
                        const gchar* format_string = gst_structure_get_string(s, "format");
                        if (format_string) {
                            filter->detector_->format = gst_video_format_from_string(format_string);
                            g_print("   Format = %s\n", gst_structure_get_name(s));
                        }
                    }
                    else
                    {
                        g_print("   MISSING format");
                    }

                    if (gst_structure_has_field(s, "width"))
                    {
                        gint width = -1;
                        if (gst_structure_get_int(s, "width", &width))
                        {
                            filter->detector_->width = width;
                            g_print("   Width = %d\n", width);
                        } 
                    }
                    else
                    {
                        g_print("   MISSING width");
                    }

                    if (gst_structure_has_field(s, "height"))
                    {
                        gint height = -1;
                        if (gst_structure_get_int(s, "height", &height)) {
                            filter->detector_->height = height;
                            g_print("   Height = %d\n", height);
                        }
                    }
                    else
                    {
                        g_print("   MISSING height");
                    }

                    if (gst_structure_has_field_typed(s, "colorimetry", G_TYPE_STRING)) {
                        g_print("   Colorimetry = %s\n", gst_structure_get_string(s, "colorimetry"));
                    } else {
                        g_print("   MISSING colorimetry\n");
                    }
                }
            }

            /* and forward */
            ret = gst_pad_event_default (pad, parent, event);
            break;
        }
        default:
            ret = gst_pad_event_default (pad, parent, event);
            break;
    }
    return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_opencv_detector_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    GstOpencvDetector *filter;
    (void)pad;

    filter = GST_OPENCVDETECTOR (parent);

    // Attempt to initialize the filter state.
    if (!filter->detector_->is_initialized())
    {
        filter->detector_->initialize(filter->configs_path, filter->weights_path, filter->class_names_path);
    }

    if (filter->detector_->is_initialized())
    {
        ScopedBufferMap scoped_buffer(buf, filter->detector_->width, filter->detector_->height, filter->detector_->format);

        cv::Mat working_image = scoped_buffer.frame();

        DetectionList detection_list;
        auto start = std::chrono::high_resolution_clock::now();
        if (!filter->detector_->get_objects(working_image, detection_list, filter->annotate))
        {
            g_print("ERROR while attempting to get detections!\n");
        }
        else
        {
            g_print("FOUND %lu objects.\n", detection_list.detections.size());
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        g_print("Detection took %0.3f seconds.\n", elapsed.count());

        if ((detection_list.detections.size() > 0) && filter->annotate)
        {
            GstBuffer* annotated_frame = cv_mat_to_gst_buffer(working_image);
            gst_buffer_unref(buf);

            return gst_pad_push(filter->srcpad, annotated_frame);
        }
    }

    return gst_pad_push(filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
opencvdetector_init (GstPlugin * opencvdetector)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template opencvdetector' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_opencv_detector_debug, "opencvdetector",
      0, "Template opencvdetector");

  return GST_ELEMENT_REGISTER (opencv_detector, opencvdetector);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstopencvserver"
#endif

/* gstreamer looks for this structure to register opencvservers
 *
 * exchange the string 'Template opencvdetector' with your opencvdetector description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    opencvdetector,
    "opencv_detector",
    opencvdetector_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
