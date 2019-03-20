#include <iostream>

#include <gstreamermm.h>
#include <glibmm/main.h>

Glib::RefPtr<Glib::MainLoop> loop;

static bool on_bus_call(const Glib::RefPtr<Gst::Bus> &bus, const Glib::RefPtr<Gst::Message> &message) {
    std::cout << "Got message of type " << Gst::Enums::get_name(message->get_message_type()) << std::endl;

    if (message->get_source()) {
        std::cout << "Source object: " << message->get_source()->get_name() << std::endl;
    }

    switch (message->get_message_type()) {
        case Gst::MESSAGE_EOS: {
            std::cout << "End of stream" << std::endl;
            loop->quit();
            break;
        }
        case Gst::MESSAGE_ERROR: {
            auto error_msg = Glib::RefPtr<Gst::MessageError>::cast_static(message);
            std::cout << "Error: " << error_msg->parse_error().what() << std::endl;
            std::cout << "Debug: " << error_msg->parse_debug() << std::endl;
            break;
        }
        case Gst::MESSAGE_STATE_CHANGED: {
            auto state_changed_msg = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(message);
            std::cout << "Old state: " << Gst::Enums::get_name(state_changed_msg->parse_old_state()) << std::endl;
            std::cout << "New state: " << Gst::Enums::get_name(state_changed_msg->parse_new_state()) << std::endl;
            break;
        }
        default:
            // unhandled messages
            break;
    }

    return true;
}

int main(int argc, char *argv[]) {
    // check args
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]<< " <v4l2 dev filename> <save location template>" << std::endl;
        return 1;
    }

    // init gstreamer
    Gst::init(argc, argv);

    // init loop
    loop = Glib::MainLoop::create();

    // create pipeline
    auto pipeline = Gst::Pipeline::create("camera-recorder");
    if (!pipeline) {
        std::cerr << "Cannot create pipeline element." << std::endl;
        return 1;
    }

    // region create elements
    // video source
    auto v4l2src = Gst::ElementFactory::create_element("v4l2src", "v4l2src");
    if (!v4l2src) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }
    v4l2src->property<Glib::ustring>("device", argv[1]);

    // video filter
    auto video_filter = Gst::ElementFactory::create_element("capsfilter", "video_filter");
    if (!video_filter) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }
    auto video_caps = Gst::Caps::create_simple("image/jpeg");
    video_caps->set_simple("width", 2592);
    video_caps->set_simple("height", 1944);
    video_filter->property("caps", video_caps);

    // jpeg decoder
    auto jpegdec = Gst::ElementFactory::create_element("jpegdec", "jpegdec");
    if (!jpegdec) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }

    // encode queue
    auto encode_queue = Gst::ElementFactory::create_element("queue", "encode_queue");
    if (!encode_queue) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }

    // h264 encoder
    auto x264enc = Gst::ElementFactory::create_element("x264enc", "x264enc");
    if (!x264enc) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }
    x264enc->property("key-int-max", 10);

    // h264 filter
    auto h264_filter = Gst::ElementFactory::create_element("capsfilter", "h264_filter");
    if (!h264_filter) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }
    auto encode_caps = Gst::Caps::create_simple("video/x-h264");
    encode_caps->set_simple("profile", "high");
    h264_filter->property("caps", encode_caps);

    // h264 parser
    auto h264parse = Gst::ElementFactory::create_element("h264parse", "h264parse");
    if (!h264parse) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }

    // sink
    auto splitmuxsink = Gst::ElementFactory::create_element("splitmuxsink", "splitmuxsink");
    if (!splitmuxsink) {
        std::cerr << "Cannot create stream element." << std::endl;
        return 1;
    }
    splitmuxsink->property<Glib::ustring>("location", argv[2]);
    splitmuxsink->property("max-size-time", 10000000000);
    splitmuxsink->property("send-keyframe-requests", true);
    // endregion

    // region set up pipeline
    // add elements
    try {
        pipeline->add(v4l2src)
                ->add(video_filter)
                ->add(jpegdec)
                ->add(encode_queue)
                ->add(x264enc)
                ->add(h264_filter)
                ->add(h264parse)
                ->add(splitmuxsink);
    } catch (const std::runtime_error &ex) {
        std::cerr << "Exception while adding: " << ex.what() << std::endl;
        return 1;
    }

    // link elements
    try {
        v4l2src->link(video_filter)
                ->link(jpegdec)
                ->link(encode_queue)
                ->link(x264enc)
                ->link(h264_filter)
                ->link(h264parse)
                ->link(splitmuxsink);
    } catch (const std::runtime_error &ex) {
        std::cerr << "Exception while linking: " << ex.what() << std::endl;
    }
    // endregion

    // add message handler
    auto bus = pipeline->get_bus();
    auto bus_watch_id = bus->add_watch(sigc::ptr_fun(on_bus_call));

    // start playing
    std::cout << "Now playing" << std::endl;
    auto start_result = pipeline->set_state(Gst::STATE_PLAYING);
    // TODO check result

    // run main loop
    std::cout << "Running..." << std::endl;
    loop->run();

    // clean up
    auto stop_result = pipeline->set_state(Gst::STATE_NULL);
    // TODO check result
    auto bus_result = bus->remove_watch(bus_watch_id);
    // TODO check resultZ

    return 0;
}
