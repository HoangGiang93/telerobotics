#include "webrtc_connector.hpp"

class TeleopServer : public WebrtcConnector
{
public:
    TeleopServer(std::string client_id, const fs::path &exe_dir, const fs::path &yaml_path)
        : WebrtcConnector(client_id, exe_dir, yaml_path),
          got_local_sdp(false)
    {
    }

    void pre_run() override
    {
        std::error_code ec;
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

        pc_->onLocalDescription([this](const rtc::Description &d)
                                {
            local_sdp = std::string(d);
            got_local_sdp = true;
            write_signaling(offer_path, local_sdp, local_cands);
            printf("[%s] wrote offer SDP: %s\n", client_id.c_str(), offer_path.c_str()); });

        pc_->onLocalCandidate([this](const rtc::Candidate &c)
                              {
            local_cands.push_back(std::string(c));
            if (got_local_sdp)
                write_signaling(offer_path, local_sdp, local_cands); });

        dc_ = pc_->createDataChannel("teleop");

        dc_->onOpen([this]()
                    {
            printf("[%s] DataChannel opened: %s\n", client_id.c_str(), dc_->label().c_str());
            connect();
            start_time = get_time_now();
            communicate(true);
            communicate();
            const std::byte *data = reinterpret_cast<const std::byte *>(receive_buffer.buffer_double.data);
            const size_t data_size = receive_buffer.buffer_double.size * sizeof(double);
            printf("[server] ready to send data at time %.6f s - size %ld\n", *world_time, receive_buffer.buffer_double.size);
            dc_->send(data, data_size); });

        dc_->onMessage([this](rtc::message_variant msg)
                       { bind_data(msg, dc_); });
    }

    void waiting_for_signaling() override
    {
        printf("[%s] waiting for %s ...\n", client_id.c_str(), answer_path.c_str());
        std::string remote_sdp;
        std::vector<std::string> remote_cands;

        while (true)
        {
            if (read_signaling(answer_path, remote_sdp, remote_cands))
            {
                try
                {
                    pc_->setRemoteDescription(rtc::Description(remote_sdp, "answer"));
                    for (auto &c : remote_cands)
                        pc_->addRemoteCandidate(rtc::Candidate(c));
                    printf("[%s] applied answer (cands=%ld)\n", client_id.c_str(), remote_cands.size());
                    break;
                }
                catch (const std::exception &e)
                {
                    printf("[%s] applying answer failed: %s\n", client_id.c_str(), e.what());
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

private:
    std::shared_ptr<rtc::DataChannel> dc_;

    std::atomic<bool> got_local_sdp;
};

int main(int argc, char **argv)
{
    const std::string client_id = (argc > 1) ? argv[1] : "client_1";
    const std::string yaml_path_str = (argc > 2) ? argv[2] : "server_meta_data.yaml";
    const fs::path exe_dir = TeleopServer::resolve_exe_dir((argc > 0 && argv && argv[0]) ? argv[0] : ".");
    fs::path yaml_path;
    if (fs::path(yaml_path_str).is_absolute())
    {
        yaml_path = fs::weakly_canonical(yaml_path_str);
    }
    else
    {
        yaml_path = fs::weakly_canonical(
            exe_dir / ".." / "configuration" / yaml_path_str);
    }
    TeleopServer server(client_id, exe_dir, yaml_path);
    return server.run();
}
