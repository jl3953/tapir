#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace rss {

class Session;

using barrier_func_t = std::function<void(const Session &)>;

void RegisterRSSService(const std::string &name, barrier_func_t bf);
void UnregisterRSSService(const std::string &name);

void StartTransaction(Session &s, const std::string &name);
void EndTransaction(Session &s, const std::string &name);

class Session {
   public:
    Session();
    Session(Session &&other);

    ~Session();

    uint64_t id() const { return id_; }

   protected:
    void StartTransaction(const std::string &name);
    void EndTransaction(const std::string &name);

    friend void StartTransaction(Session &s, const std::string &name);
    friend void EndTransaction(Session &s, const std::string &name);

   private:
    enum RSSServiceState {
        NONE = 0,
        EXECUTING,
        EXECUTED
    };

    void UpdateLastService(const std::string &name);

    static std::atomic<std::uint64_t> next_id_;

    uint64_t id_;
    std::string last_service_;
    RSSServiceState current_state_;
};

class RSSRegistry {
   public:
    RSSRegistry();
    ~RSSRegistry();

   protected:
    class RSSService {
       public:
        RSSService(std::string name, barrier_func_t bf);
        ~RSSService();

        void invoke_barrier(const Session &session) const { bf_(session); }

       private:
        std::string name_;
        barrier_func_t bf_;
    };

    void RegisterRSSService(const std::string &name, barrier_func_t bf);
    void UnregisterRSSService(const std::string &name);

    RSSService &FindService(const std::string &name);

    friend Session;

    friend void RegisterRSSService(const std::string &name, barrier_func_t bf);
    friend void UnregisterRSSService(const std::string &name);

   private:
    std::unordered_map<std::string, RSSService> services_;
};

}  // namespace rss