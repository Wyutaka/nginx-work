//
// Created by cerussite on 3/1/20.
//

#pragma once

#include <memory>

#include <txpx2/context.hpp>

namespace txpx2 {

class Layer {
 private:
  std::unique_ptr<Layer> next_;

 public:
  explicit Layer(std::unique_ptr<Layer> next) : next_(std::move(next)) {}

 protected:
  void next(const Context& context) { next_->operator()(context); }

 public:
  virtual void operator()(const Context&) = 0;
};

}  // namespace txpx2
