#pragma once

#include "defines/types.hpp"
#include "logger.hpp"

namespace embers {

template <typename T>
class Option {
  template <typename T1>
  friend class Option;

 private:
  static_assert(!std::is_same<T, void>::value, "T must not be void");

  union Container {
    T  some_;
    u8 dummy;
    constexpr Container() : dummy() {};
    ~Container() {}
  };

  Container container_;
  bool      has_value_ = false;

 public:
  constexpr Option(T &&some) {
    new (&container_.some_) T(std::move(some));
    has_value_ = true;
  }

  constexpr Option() : has_value_(false) {}

  constexpr Option(const T &some) {
    new (&container_.some_) T(some);
    has_value_ = true;
  }

  constexpr Option(const Option &option) {
    if (has_value_) {
      new (&container_.some_) T(option.container_.some_);
    }
    has_value_ = option.has_value_;
  }

  constexpr Option(Option &&option) {
    if (has_value_) {
      new (&container_.some_) T(std::move(option.container_.some_));
    }
    has_value_        = option.has_value_;
    option.has_value_ = false;
  }

 private:
  // Convert from Option <T1> into Option<T> (only if it is a empty)
  template <typename T1>
  constexpr Option(const Option<T1> &option) {
    EMBERS_ASSERT(
        !option.has_value_,
        "Attempting to convert Option between templates (to a different value "
        "types), but it isn't empty"
    );
    status_ = option.status_;
  }

  template <typename T1>
  constexpr Option(Option<T1, E> &&option) {
    EMBERS_ASSERT(
        !option.has_value_,
        "Attempting to convert Option between templates (to a different value "
        "types), but it isn't empty"
    );
    status_        = result.status_;
    option.status_ = false;
  }

 public:
  constexpr static Option create_some(const T &some) { return Option(some); };

  constexpr static Option create_none() { return Option(); };

  constexpr static Option create_some(T &&some) {
    return Option(std::move(some));
  };

  ~Option() {
    if (has_value_) {
      container_.some_.T::~T();
    }
  };

 public:
  constexpr const T &value() const & {
    EMBERS_ASSERT(has_value_, "Unwrapping value of empty option");
    return container_.some_;
  }

  constexpr T &&value() && {
    EMBERS_ASSERT(has_value_, "Unwrapping value of empty option");
    return std::move(container_.some_);
  }

 public:
  constexpr const T &value_or(const T &other) const & {
    if (status_ != Status::kOk) {
      return other;
    }
    return container_.some_;
  }

  constexpr T &&value_or(T &&other) && {
    if (!has_value_) {
      return std::move(other);
    }
    return std::move(container_.some_);
  }

 public:
  constexpr bool has_value() const { return has_value_; }

  template <class F>
  constexpr bool has_value_and(F &&f) const & {
    static_assert(
        std::is_same<std::invoke_result<F, T>::type, bool>::value,
        __FUNCTION__ " must be called with a (template) F argument that returns bool"
    );
    if (!has_value_) {
      return false;
    }
    return std::invoke(std::forward<F>(f), container_.some_);
  }

  template <class F>
  constexpr bool has_value_and(F &&f) && {
    static_assert(
        std::is_same<std::invoke_result<F, T>::type, bool>::value,
        __FUNCTION__ " must be called with a (template) F argument that returns bool"
    );
    if (!has_value_) {
      return false;
    }
    return std::invoke(std::forward<F>(f), std::move(container_.some_));
  }

 public:
  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f
  ) const & {
    // (n1t3): i'm not what contraints are valid for this case
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    if (!has_value_) {
      return *this;
    }
    return MappedResult(  //
        std::invoke(std::forward<F>(f), (container_.some_))
    );
  }

  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f) && {
    // (n1t3): i'm not what contraints are valid for this case
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    if (!has_value_) {
      return *this;
    }
    return MappedResult(  //
        std::invoke(std::forward<F>(f), std::move(container_.some_))
    );
  }
};

}  // namespace embers
