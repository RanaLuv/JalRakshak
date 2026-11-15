#include "alert_classes.hpp"
#include <sstream>
#include <new>

namespace jalrakshak {

static const char *levelText(RiskLevel level) {
    switch (level) {
        case RISK_WATCH: return "WATCH";
        case RISK_WARNING: return "WARNING";
        case RISK_CRITICAL: return "CRITICAL";
        default: return "NORMAL";
    }
}

std::string Alert::message() const {
    std::ostringstream out;
    out << "[JALRAKSHAK] " << levelText(data_.level)
        << " alert at location " << data_.location_id
        << ". Confidence: " << static_cast<int>(data_.confidence * 100.0)
        << "%. Follow local emergency instructions.";
    return out.str();
}

std::string FloodAlert::message() const {
    return std::string("[FLOOD] ") + Alert::message();
}

std::string LandslideAlert::message() const {
    return std::string("[LANDSLIDE] ") + Alert::message();
}

AlertManager::AlertManager(int capacity)
    : heap_(ah_create(capacity)), history_(history_create()) {
    if (!heap_ || !history_) {
        ah_free(heap_);
        history_free(history_);
        heap_ = nullptr;
        history_ = nullptr;
        throw std::bad_alloc();
    }
}

AlertManager::~AlertManager() {
    ah_free(heap_);
    history_free(history_);
}

Status AlertManager::enqueue(const ::Alert& alert) {
    return ah_push(heap_, alert) ? STATUS_OK : STATUS_FULL;
}

::Alert AlertManager::next() {
    ::Alert result;
    if (!ah_pop_max(heap_, &result)) throw EmptyQueueException();
    if (history_add(history_, &result) != STATUS_OK)
        throw std::runtime_error("Could not add handled alert to history");
    return result;
}

Status AlertManager::saveHistory(const std::string& path) const {
    return history_save_to(history_, path.c_str());
}

Status AlertManager::loadHistory(const std::string& path) {
    return history_load_from(history_, path.c_str());
}

int AlertManager::pending() const {
    return heap_ ? heap_->size : 0;
}

int AlertManager::historyCount() const {
    return history_ ? history_->count : 0;
}

} 
