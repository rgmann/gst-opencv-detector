/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2024 Robert Vaughan <<user@hostname.org>>
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
 * SECTION:element-opencvserver
 *
 * FIXME:Describe opencvserver here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! opencvserver ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <opencv2/dnn/dnn.hpp>
#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "gstopencv-utils.h"

#include "gstopencvserver.h"

GST_DEBUG_CATEGORY_STATIC (gst_open_cvserver_debug);
#define GST_CAT_DEFAULT gst_open_cvserver_debug

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
    PROP_ANNOTATE
};

struct Detection {
    int class_id;
    std::string class_name;
    cv::Rect box;
    gfloat confidence;
};

struct GstOpenCVServerState {

    gboolean initialized_;

    GstOpenCVServer* server_;

    // Caps
    gint width;
    gint height;
    GstVideoFormat format;
    float conf_threshold;
    float nms_threshold;

    // Neural net
    std::unique_ptr<cv::dnn::DetectionModel> model_;

    std::vector<std::string> class_names_;

    GstOpenCVServerState() 
        : initialized_(FALSE)
        , server_(nullptr)
        , width(-1)
        , height(-1)
        , conf_threshold(0.45)
        , nms_threshold(0.2)
    {
    }

    gboolean is_initialized() const
    {
        return initialized_;
    }

    gboolean initialize(const gchar* config, const gchar* weights, const gchar* class_names)
    {
        gboolean success = FALSE;

        if (config && weights && class_names && (width > 0) && (height > 0))
        {
            g_print("Creating detection model.\n");

            success = parse_class_names(class_names);

            if (success)
            {
                model_ = std::make_unique<cv::dnn::DetectionModel>(
                    weights,
                    config
                );

                cv::Size input_size(320, 320);
                model_->setInputSize(input_size);

                // cv::Scalar input_scale = 1.0 / 127.5;
                model_->setInputScale(1.0 / 127.5);

                cv::Scalar input_mean = 127.5;
                model_->setInputMean(input_mean);

                model_->setInputSwapRB(true);

                initialized_ = TRUE;
            }
            else
            {
                success = false;
            }
        }

        return success;
    }

    gboolean parse_class_names(const gchar* filename)
    {
        std::ifstream file(filename);

        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                class_names_.push_back(line);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    gboolean get_objects(const cv::Mat& image, std::vector<Detection>& detections)
    {
        gboolean success = FALSE;

        std::vector<int> class_ids;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        (void)detections;

        if (is_initialized())
        {
            model_->detect(image, class_ids, confidences, boxes, conf_threshold, nms_threshold);

            for (std::size_t index = 0; index < class_ids.size(); ++index)
            {
                Detection detection;

                detection.class_id = class_ids[index];
                if (detection.class_id < static_cast<int>(class_names_.size()))
                {
                    detection.class_name = class_names_[static_cast<std::size_t>(detection.class_id)];
                }
                else
                {
                    GST_ELEMENT_WARNING(server_, RESOURCE, BUSY,
                        ("Invalid class_id=%d.", detection.class_id),
                        ("Class ID is invalid."));
                }

                detection.box = boxes[index];
                detection.confidence = confidences[index];

                detections.push_back(detection);
            }

            success = TRUE;
        }

        return success;
    }

    void annotate_image(const std::vector<Detection>& detections, cv::Mat& image)
    {
        static const int thickness = 2;
        cv::Scalar color(0, 255, 0);

        for (auto it = detections.begin(); it != detections.end(); ++it)
        {
            // cv2.rectangle(img,box,color=(0,255,0),thickness=2)
            // cv2.putText(img,classNames[classId-1].upper(),(box[0]+10,box[1]+30),
            // cv2.FONT_HERSHEY_COMPLEX,1,(0,255,0),2)
            // cv2.putText(img,str(round(confidence*100,2)),(box[0]+200,box[1]+30),
            // cv2.FONT_HERSHEY_COMPLEX,1,(0,255,0),2)
            cv::rectangle(image, it->box, color, thickness);

            cv::Point class_name_location(it->box.x + 10, it->box.y + 30);
            cv::putText(image, it->class_name, class_name_location, cv::FONT_HERSHEY_COMPLEX, 1, color);

            cv::Point confidence_location(it->box.x + 200, it->box.y + 30);
            cv::putText(image, std::to_string(it->confidence), confidence_location, cv::FONT_HERSHEY_COMPLEX, 1, color);
        }
    }
};

struct _GstOpenCVServer
{
    GstElement element;

    GstPad *sinkpad, *srcpad;

    // Settings
    gboolean silent;
    gchar* configs_path;
    gchar* weights_path;
    gchar* class_names_path;
    gboolean annotate;

    GstOpenCVServerState* state_;

    _GstOpenCVServer()
        : sinkpad(nullptr)
        , srcpad(nullptr)
        , silent(FALSE)
        , configs_path(nullptr)
        , weights_path(nullptr)
        , annotate(TRUE)
        , state_(nullptr)
    {

    }

    ~_GstOpenCVServer()
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
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_open_cvserver_parent_class parent_class
G_DEFINE_TYPE (GstOpenCVServer, gst_open_cvserver, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (open_cvserver, "open_cvserver", GST_RANK_NONE,
    GST_TYPE_OPENCVSERVER);

static void gst_open_cvserver_finalize(GObject *object);
static void gst_open_cvserver_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_open_cvserver_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_open_cvserver_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_open_cvserver_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

static gboolean valid_file_path(const gchar* path)
{
    if (!path) return FALSE;

    std::error_code error;
    std::filesystem::file_status status = std::filesystem::status(path, error);

    if (!error)
    {
        return std::filesystem::is_regular_file(status) ? TRUE : FALSE;
    }

    return FALSE;
}

/* GObject vmethod implementations */

/* initialize the opencvserver's class */
static void
gst_open_cvserver_class_init (GstOpenCVServerClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;

    gobject_class->set_property = gst_open_cvserver_set_property;
    gobject_class->get_property = gst_open_cvserver_get_property;
    gobject_class->finalize = gst_open_cvserver_finalize;

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

    gst_element_class_set_details_simple (gstelement_class,
        "OpenCVServer",
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
gst_open_cvserver_init (GstOpenCVServer * filter)
{
    GstOpenCVServerState* state = new GstOpenCVServerState();

    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    gst_pad_set_event_function (filter->sinkpad,
        GST_DEBUG_FUNCPTR (gst_open_cvserver_sink_event));
    gst_pad_set_chain_function (filter->sinkpad,
        GST_DEBUG_FUNCPTR (gst_open_cvserver_chain));
    GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS (filter->srcpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    filter->silent = FALSE;
    filter->annotate = TRUE;

    g_print("OPENCV INIT\n");

    state->server_ = filter;
    filter->state_ = state;
}

static void
gst_open_cvserver_finalize(GObject *object)
{
    GObjectClass *klass = G_OBJECT_CLASS(gst_open_cvserver_parent_class);
    GstOpenCVServer *self = GST_OPENCVSERVER(object);

    delete self->state_;
    self->state_ = nullptr;

    return klass->finalize(object);
}

static void
gst_open_cvserver_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstOpenCVServer *filter = GST_OPENCVSERVER (object);

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
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_open_cvserver_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstOpenCVServer *filter = GST_OPENCVSERVER (object);

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
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_open_cvserver_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
    GstOpenCVServer *filter;
    gboolean ret;

    filter = GST_OPENCVSERVER (parent);

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
                // gint width, height;
                // guint delta;

                //video/x-raw, format=(string)NV21, width=(int)1280, height=(int)720, colorimetry=(string)bt709, framerate=(fraction)30/1;

                g_print("Received caps for %s\n", gst_structure_get_name(s));
                gchar* serialized = gst_structure_to_string(s);
                g_print("  Serialized: <begin>%s<end>\n", serialized);
                g_free(serialized);

                const gchar* format_string = gst_structure_get_string(s, "format");
                if (format_string) {
                    g_print("   Format = %s", gst_structure_get_name(s));
                } else {
                    g_print("   MISSING format");
                }
                
                gint width = -1;
                if (gst_structure_get_int(s, "width", &width)) {
                    filter->state_->width = width;
                    g_print("   Width = %d", width);
                } else {
                    g_print("   MISSING width");
                }

                gint height = -1;
                if (gst_structure_get_int(s, "height", &height)) {
                    filter->state_->height = height;
                    g_print("   Height = %d", height);
                } else {
                    g_print("   MISSING height");
                }

                if (gst_structure_has_field_typed(s, "colorimetry", G_TYPE_STRING)) {
                    g_print("   Colorimetry = %s", gst_structure_get_string(s, "colorimetry"));
                } else {
                    g_print("   MISSING colorimetry");
                }

                // if (gst_structure_has_field_typed(s, "width", G_TYPE_INT) &&
                // gst_structure_has_field_typed(s, "height", G_TYPE_INT)) {
                //     gst_structure_get_int(s, "width", &width);
                //     gst_structure_get_int(s, "height", &height);
                // }
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
gst_open_cvserver_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    GstOpenCVServer *filter;
    (void)pad;

    filter = GST_OPENCVSERVER (parent);

    // Attempt to initialize the filter state.
    if (!filter->state_->is_initialized())
    {
        filter->state_->initialize(filter->configs_path, filter->weights_path, filter->class_names_path);
    }

    if (filter->state_->is_initialized())
    {
        // cv::Mat image = gst_buffer_to_cv_mat(buf, filter->state_->width, filter->state_->width, 3);

        // std::vector<Detection> detections;
        // if (!filter->state_->get_objects(image, detections))
        // {
        //     g_print("ERROR while attempting to get detections!\n");
        // }
        // else
        // {
        //     g_print("FOUND %lu objects.\n", detections.size());
        // }
        GstMapInfo map;
        gst_buffer_map(buf, &map, GST_MAP_READ);

        {// Create OpenCV Mat
            cv::Mat frame(cv::Size(filter->state_->width, filter->state_->height), CV_8UC3, map.data, cv::Mat::AUTO_STEP);

            std::vector<Detection> detections;
            if (!filter->state_->get_objects(frame, detections))
            {
                g_print("ERROR while attempting to get detections!\n");
            }
            else
            {
                g_print("FOUND %lu objects.\n", detections.size());
            }
        }

        // Unmap the buffer
        gst_buffer_unmap(buf, &map);
    }

    // if (filter->silent == FALSE)
    //     g_print ("I'm plugged, therefore I'm in: buffer size is %lu\n");

    /* just push out the incoming buffer without touching it */
    return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
opencvserver_init (GstPlugin * opencvserver)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template opencvserver' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_open_cvserver_debug, "opencvserver",
      0, "Template opencvserver");

  return GST_ELEMENT_REGISTER (open_cvserver, opencvserver);
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
 * exchange the string 'Template opencvserver' with your opencvserver description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    opencvserver,
    "open_cvserver",
    opencvserver_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
