#include "multiverse_client/multiverse_client_json.h"
#include "yaml-cpp/yaml.h"

#include <rtc/rtc.hpp>
#include <filesystem>
#include <fstream>
#include <thread>
#include <csignal>

namespace fs = std::filesystem;

struct MultiverseMetadata
{
    std::string host = "tcp://127.0.0.1";
    std::string server_port = "7000";
    std::string client_port = "7200";
    std::string world_name = "world";
    std::string simulation_name = "simulation";
    std::map<std::string, std::vector<std::string>> send_objects;
    std::map<std::string, std::vector<std::string>> receive_objects;
};

static MultiverseMetadata load_multiverse_metadata(const fs::path &yaml_path)
{
    MultiverseMetadata meta_data;

    YAML::Node root = YAML::LoadFile(yaml_path.string());

    if (root["world_name"])
        meta_data.world_name = root["world_name"].as<std::string>();

    if (root["simulation_name"])
        meta_data.simulation_name = root["simulation_name"].as<std::string>();

    if (root["host"])
        meta_data.host = root["host"].as<std::string>();

    if (root["server_port"])
        meta_data.server_port = root["server_port"].as<std::string>();

    if (root["client_port"])
        meta_data.client_port = root["client_port"].as<std::string>();

    if (root["send"])
    {
        for (const auto &it : root["send"])
        {
            const std::string obj = it.first.as<std::string>();
            const YAML::Node attrs = it.second;

            std::vector<std::string> vec;
            for (const auto &a : attrs)
                vec.push_back(a.as<std::string>());

            meta_data.send_objects[obj] = std::move(vec);
        }
    }

    if (root["receive"])
    {
        for (const auto &it : root["receive"])
        {
            const std::string obj = it.first.as<std::string>();
            const YAML::Node attrs = it.second;

            std::vector<std::string> vec;
            for (const auto &a : attrs)
                vec.push_back(a.as<std::string>());

            meta_data.receive_objects[obj] = std::move(vec);
        }
    }

    return meta_data;
}

std::atomic<bool> stop_requested{false};

void on_sigint(int)
{
    stop_requested.store(true);
}

class WebrtcConnector : public MultiverseClientJson
{
public:
    WebrtcConnector(const std::string &in_client_id, const fs::path &exe_dir, const fs::path &yaml_path)
        : client_id(std::move(in_client_id)),
          sdp_dir(fs::weakly_canonical(exe_dir / ".." / "SDP" / client_id)),
          offer_path(sdp_dir / "offer.txt"),
          answer_path(sdp_dir / "answer.txt")
    {
        std::error_code ec;
        fs::remove(answer_path, ec);
        if (ec)
        {
            printf("[WebrtcConnector] WARN: cannot remove answer: %s (%s)\n",
                   answer_path.c_str(), ec.message().c_str());
        }

        init_multiverse(yaml_path);

        init_webrtc();
    }

    int run()
    {
        std::error_code ec;
        fs::remove(answer_path, ec);
        if (ec)
        {
            printf("[%s] WARN: cannot remove answer: %s (%s)\n",
                   client_id.c_str(), answer_path.c_str(), ec.message().c_str());
        }

        pre_run();

        printf("[%s] offer_path   = %s\n", client_id.c_str(), offer_path.c_str());
        printf("[%s] answer_path  = %s\n", client_id.c_str(), answer_path.c_str());

        waiting_for_signaling();

        printf("[%s] starting communication loop\n", client_id.c_str());

        std::signal(SIGINT, on_sigint);

        while (!stop_requested)
        {
            communicate();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        printf("[%s] stopping communication loop\n", client_id.c_str());
        disconnect();

        fs::remove(offer_path, ec);
        if (ec)
        {
            printf("[%s] WARN: cannot remove offer: %s (%s)\n",
                   client_id.c_str(), offer_path.c_str(), ec.message().c_str());
        }
        fs::remove(answer_path, ec);
        if (ec)
        {
            printf("[%s] WARN: cannot remove answer: %s (%s)\n",
                   client_id.c_str(), answer_path.c_str(), ec.message().c_str());
        }

        printf("[%s] done\n", client_id.c_str());
        return 0;
    }

    static fs::path resolve_exe_dir(const char *argv0)
    {
        fs::path p = fs::path(argv0);
        if (p.is_relative())
            p = fs::absolute(p);
        p = fs::weakly_canonical(p);
        if (!fs::exists(p))
            return fs::current_path();
        return p.parent_path();
    }

protected:
    double start_time;
    std::string client_id;

    std::shared_ptr<rtc::PeerConnection> pc_;

    std::string local_sdp;
    std::vector<std::string> local_cands;
    fs::path sdp_dir;
    fs::path offer_path;
    fs::path answer_path;

protected:
    void bind_data(rtc::message_variant &msg, std::shared_ptr<rtc::DataChannel> dc)
    {
        auto bin = std::get_if<rtc::binary>(&msg);
        if (!bin)
        {
            printf("[%s] WARN: unexpected data type received\n", client_id.c_str());
            return;
        }
        if (bin->size() != send_buffer.buffer_double.size * sizeof(double))
        {
            printf("[%s] WARN: unexpected data size received: %ld (expected %ld)\n",
                   client_id.c_str(), bin->size(), send_buffer.buffer_double.size * sizeof(double));
            return;
        }
        const double* send_data = reinterpret_cast<const double*>(bin->data());
        for (size_t i = 0; i < send_buffer.buffer_double.size; ++i)
        {
            send_buffer.buffer_double.data[i] = send_data[i];
        }
        const std::byte* receive_data = reinterpret_cast<const std::byte*>(receive_buffer.buffer_double.data);
        dc->send(receive_data, receive_buffer.buffer_double.size * sizeof(double));
    }

    virtual void pre_run() = 0;

    virtual void waiting_for_signaling() = 0;

    static void write_signaling(const fs::path &path,
                                const std::string &sdp,
                                const std::vector<std::string> &cands)
    {
        ensure_parent_dir(path);

        std::ofstream f(path, std::ios::out | std::ios::trunc);
        if (!f.good())
        {
            printf("ERROR: cannot write signaling to %s\n", path.c_str());
            return;
        }

        f << "SDP\n"
          << sdp << "\nEND_SDP\n";
        for (const auto &c : cands)
            f << "CANDIDATE\n"
              << c << "\n";
        f.flush();
    }

    static bool read_signaling(const fs::path &path,
                               std::string &sdp_out,
                               std::vector<std::string> &cands_out)
    {
        std::ifstream f(path);
        if (!f.good())
            return false;

        std::string line;
        if (!std::getline(f, line) || line != "SDP")
            return false;

        std::ostringstream sdp;
        while (std::getline(f, line))
        {
            if (line == "END_SDP")
                break;
            sdp << line << "\n";
        }
        sdp_out = sdp.str();
        if (sdp_out.empty())
            return false;

        cands_out.clear();
        while (std::getline(f, line))
        {
            if (line == "CANDIDATE")
            {
                std::string cand;
                if (std::getline(f, cand) && !cand.empty())
                    cands_out.push_back(cand);
            }
        }
        return true;
    }

private:
    MultiverseMetadata meta_data;

private:
    void init_webrtc()
    {
        rtc::InitLogger(rtc::LogLevel::Warning);

        rtc::Configuration cfg;
        cfg.iceServers.emplace_back("stun:stun.l.google.com:19302");
        pc_ = std::make_shared<rtc::PeerConnection>(cfg);
    }

    void init_multiverse(const fs::path &yaml_path)
    {
        meta_data = load_multiverse_metadata(yaml_path);
        host = meta_data.host;
        server_port = meta_data.server_port;
        client_port = meta_data.client_port;
        start_time = get_time_now();
        *world_time = get_time_now() - start_time;

        printf("Multiverse Server: %s:%s - Multiverse Client: %s:%s\n", host.c_str(), server_port.c_str(), host.c_str(), client_port.c_str());
    }

    static void ensure_parent_dir(const fs::path &file_path)
    {
        std::error_code ec;
        fs::create_directories(file_path.parent_path(), ec);
        if (ec)
        {
            printf("[WebrtcConnector] WARN: cannot create directory: %s (%s)\n",
                   file_path.parent_path().c_str(), ec.message().c_str());
        }
    }

private:
    void start_connect_to_server_thread() override
    {
        connect_to_server();
    }

    void wait_for_connect_to_server_thread_finish() override
    {
    }

    void start_meta_data_thread() override
    {
        send_and_receive_meta_data();
    }

    void wait_for_meta_data_thread_finish() override
    {
    }

    bool init_objects(bool) override
    {
        return meta_data.send_objects.size() > 0 || meta_data.receive_objects.size() > 0;
    }

    void bind_request_meta_data() override
    {
        request_meta_data_json.clear();
        request_meta_data_json["meta_data"] = Json::Value(Json::objectValue);
        request_meta_data_json["meta_data"]["world_name"] = meta_data.world_name;
        request_meta_data_json["meta_data"]["simulation_name"] = meta_data.simulation_name;
        request_meta_data_json["meta_data"]["time_unit"] = "s";
        request_meta_data_json["meta_data"]["length_unit"] = "m";
        request_meta_data_json["meta_data"]["angle_unit"] = "rad";
        request_meta_data_json["meta_data"]["handedness"] = "rhs";
        request_meta_data_json["meta_data"]["force_unit"] = "N";
        request_meta_data_json["send"] = Json::Value(Json::objectValue);
        for (const std::pair<const std::string, std::vector<std::string>> &obj : meta_data.send_objects)
        {
            for (const std::string &attribute : obj.second)
            {
                request_meta_data_json["send"][obj.first].append(attribute);
            }
        }
        request_meta_data_json["receive"] = Json::Value(Json::objectValue);
        for (const std::pair<const std::string, std::vector<std::string>> &obj : meta_data.receive_objects)
        {
            for (const std::string &attribute : obj.second)
            {
                request_meta_data_json["receive"][obj.first].append(attribute);
            }
        }

        request_meta_data_str = request_meta_data_json.toStyledString();

        printf("[WebrtcConnector] request meta data:\n%s\n", request_meta_data_str.c_str());
    }

    void bind_api_callbacks() override
    {
    }

    void bind_api_callbacks_response() override
    {
    }

    void bind_response_meta_data() override
    {
        printf("[WebrtcConnector] response meta data:\n%s\n", response_meta_data_str.c_str());
    }

    void init_send_and_receive_data() override
    {
    }

    void bind_send_data() override
    {
        *world_time = get_time_now() - start_time;
    }

    void bind_receive_data() override
    {
    }

    void clean_up() override
    {
    }

    void reset() override
    {
        start_time = get_time_now();
    }
};