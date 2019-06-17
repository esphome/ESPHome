#pragma once

#include "esphome/core/helpers.h"
#include "esphome/core/automation.h"
#include "api_pb2.h"
#include "api_server.h"

namespace esphome {
namespace api {

template<typename... Ts> class TemplatableKeyValuePair {
 public:
  template<typename T> TemplatableKeyValuePair(std::string key, T value) : key(std::move(key)), value(value) {}
  std::string key;
  TemplatableStringValue<Ts...> value;
};

template<typename... Ts> class HomeAssistantServiceCallAction : public Action<Ts...> {
 public:
  explicit HomeAssistantServiceCallAction(APIServer *parent) : parent_(parent) {}

  TEMPLATABLE_STRING_VALUE(service);
  template<typename T> void add_data(std::string key, T value) {
    this->data_.push_back(TemplatableKeyValuePair<Ts...>(key, value));
  }
  template<typename T> void add_data_template(std::string key, T value) {
    this->data_template_.push_back(TemplatableKeyValuePair<Ts...>(key, value));
  }
  template<typename T> void add_variable(std::string key, T value) {
    this->variables_.push_back(TemplatableKeyValuePair<Ts...>(key, value));
  }
  void play(Ts... x) override {
    ServiceCallResponse resp;
    resp.service = this->service_.value(x...);
    for (auto &it : this->data_) {
      ServiceCallMap kv;
      kv.key = it.key;
      kv.value = it.value.value(x...);
      resp.data.push_back(kv);
    }
    for (auto &it : this->data_template_) {
      ServiceCallMap kv;
      kv.key = it.key;
      kv.value = it.value.value(x...);
      resp.data_template.push_back(kv);
    }
    for (auto &it : this->variables_) {
      ServiceCallMap kv;
      kv.key = it.key;
      kv.value = it.value.value(x...);
      resp.variables.push_back(kv);
    }
    this->parent_->send_service_call(resp);
  }

 protected:
  APIServer *parent_;
  std::vector<TemplatableKeyValuePair<Ts...>> data_;
  std::vector<TemplatableKeyValuePair<Ts...>> data_template_;
  std::vector<TemplatableKeyValuePair<Ts...>> variables_;
};

}  // namespace api
}  // namespace esphome
