#include "webrtc_connector.hpp"

class TeleopClient : public WebrtcConnector
{
public:
    TeleopClient(std::string client_id, const fs::path &exe_dir, const fs::path &yaml_path)
        : WebrtcConnector(client_id, exe_dir, yaml_path)
    {
    }

    void pre_run() override
    {
        pc_->onLocalDescription([this](const rtc::Description &d)
                                {
            local_sdp = std::string(d);
            write_signaling(answer_path, local_sdp, local_cands);
            printf("[%s] wrote answer SDP: %s\n", client_id.c_str(), answer_path.c_str()); });

        pc_->onLocalCandidate([this](const rtc::Candidate &c)
                              {
            local_cands.push_back(std::string(c));
            if (!local_sdp.empty())
                write_signaling(answer_path, local_sdp, local_cands); });

        pc_->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc)
                           {
            printf("[%s] DataChannel opened: %s\n", client_id.c_str(), dc->label().c_str());
            connect();
            start_time = get_time_now();
            communicate(true);
            communicate();
            dc->onMessage([this, dc](rtc::message_variant msg) 
            { bind_data(msg, dc); }); });
    }

    void waiting_for_signaling() override
    {
        printf("[%s] waiting for %s ...\n", client_id.c_str(), offer_path.c_str());
        std::string remote_sdp;
        std::vector<std::string> remote_cands;

        while (true)
        {
            if (read_signaling(offer_path, remote_sdp, remote_cands))
            {
                try
                {
                    pc_->setRemoteDescription(rtc::Description(remote_sdp, "offer"));
                    for (auto &c : remote_cands)
                        pc_->addRemoteCandidate(rtc::Candidate(c));
                    printf("[%s] applied offer (cands=%ld)\n", client_id.c_str(), remote_cands.size());
                    break;
                }
                catch (const std::exception &e)
                {
                    printf("[%s] applying offer failed: %s\n", client_id.c_str(), e.what());
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

int main(int argc, char **argv)
{
    const std::string client_id = (argc > 1) ? argv[1] : "client_1";
    const std::string yaml_path_str = (argc > 2) ? argv[2] : "client_meta_data.yaml";
    const fs::path exe_dir =
        TeleopClient::resolve_exe_dir((argc > 0 && argv && argv[0]) ? argv[0] : ".");
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
    TeleopClient client(client_id, exe_dir, yaml_path);
    return client.run();
}
