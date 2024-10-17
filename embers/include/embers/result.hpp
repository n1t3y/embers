#pragma once

#include "defines/types.hpp"
#include "logger.hpp"

namespace embers {

namespace internal {
enum class ResultStatus : u8 {
  kNotInit = 0,
  kOk      = 1,
  kErr     = 2,
};
}

template <typename T, typename E>
class Result {
  template <typename T1, typename E1>
  friend class Result;

 public:
  struct Error {
    E value;
    explicit Error(const E &error) : value(error) {};
    explicit Error(E &&error) : value(std::move(error)) {};

    Error(const Error &error) : value(error.value) {};
    Error(Error &&error) : value(std::move(error.value)) {};
  };

 private:
  using Status = internal::ResultStatus;

  static_assert(!std::is_same<T, void>::value, "T must not be void");
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  union Container {
    T  ok_;
    E  err_;
    u8 dummy;
    constexpr Container() : dummy() {};
    ~Container() {}
  };

  Container container_;
  Status    status_ = Status::kNotInit;

  Result() = delete;

 public:
  constexpr Result(T &&ok) {
    new (&container_.ok_) T(std::move(ok));
    status_ = Status::kOk;
  }

  constexpr Result(Error &&err) {
    new (&container_.err_) E(std::move(err.value));
    status_ = Status::kErr;
  }

  constexpr Result(const T &ok) {
    new (&container_.ok_) T(ok);
    status_ = Status::kOk;
  }

  constexpr Result(const Error &err) {
    new (&container_.err_) E(err.value);
    status_ = Status::kErr;
  }

  constexpr Result(const Result &result) {
    switch (result.status_) {
      case Status::kOk:
        new (&container_.ok_) T(result.container_.ok_);
        break;
      case Status::kErr:
        new (&container_.err_) E(result.container_.err_);
        break;
      case Status::kNotInit:
      default:
        break;
    }
    status_ = result.status_;
  }

  constexpr Result(Result &&result) {
    switch (result.status_) {
      case Status::kOk:
        new (&container_.ok_) T(std::move(result.container_.ok_));
        break;
      case Status::kErr:
        new (&container_.err_) E(std::move(result.container_.err_));
        break;
      case Status::kNotInit:
      default:
        break;
    }
    status_        = result.status_;
    result.status_ = Status::kNotInit;
  }

 private:
  // Convert from Result <T1, E> into Result<T, E> (only if it is an error)
  template <typename T1>
  constexpr Result(const Result<T1, E> &result) {
    EMBERS_ASSERT(
        result.status_ != Status::kOk,
        "Attempting to convert Result between templates (to a different value "
        "types), but state isn't erroneous"
    );
    if (result.status_ == Status::kErr) {
      new (&container_.err_) E(result.container_.err_);
    }
    status_ = result.status_;
  }

  template <typename T1>
  constexpr Result(Result<T1, E> &&result) {
    EMBERS_ASSERT(
        result.status_ != Status::kOk,
        "Attempting to convert Result between templates (to a different value "
        "types), but state isn't erroneous"
    );
    if (result.status_ == Status::kErr) {
      new (&container_.err_) E(std::move(result.container_.err_));
    }

    status_        = result.status_;
    result.status_ = Status::kNotInit;
  }

  // Convert from Result <T, E1> into Result<T, E> (only if it is NOT an error)
  template <typename E1>
  constexpr Result(const Result<T, E1> &result) {
    EMBERS_ASSERT(
        result.status_ != Status::kErr,
        "Attempting to convert Result between templates (to a different error "
        "types), but state isn't valid"
    );
    if (result.status_ == Status::kOk) {
      new (&container_.ok_) T(result.container_.ok_);
    }
    status_ = result.status_;
  }

  template <typename E1>
  constexpr Result(Result<T, E1> &&result) {
    EMBERS_ASSERT(
        result.status_ != Status::kErr,
        "Attempting to convert Result between templates (to a different error "
        "types), but state isn't valid"
    );
    if (result.status_ == Status::kOk) {
      new (&container_.ok_) T(std::move(result.container_.ok_));
    }

    status_        = result.status_;
    result.status_ = Status::kNotInit;
  }

 public:
  constexpr static Result create_ok(const T &ok) { return Result(ok); };

  constexpr static Result create_err(const E &err) {
    return Result(Error(err));
  };

  constexpr static Result create_ok(T &&ok) { return Result(std::move(ok)); };

  constexpr static Result create_err(E &&err) {
    return Result(Error(std::move(err)));
  };

  ~Result() {
    switch (status_) {
      case Status::kOk:
        container_.ok_.T::~T();
        break;
      case Status::kErr:
        container_.err_.E::~E();
        break;
      case Status::kNotInit:
      default:
        break;
    }
  };

 public:
  constexpr const T &value() const & {
    EMBERS_ASSERT(status_ == Status::kOk, "Unwrapping value of invalid result");
    return container_.ok_;
  }

  constexpr T &&value() && {
    EMBERS_ASSERT(status_ == Status::kOk, "Unwrapping value of invalid result");
    return std::move(container_.ok_);
  }

  constexpr const E &error() const & {
    EMBERS_ASSERT(status_ == Status::kErr, "Unwrapping error of valid result");
    return container_.err_;
  }

  constexpr const E &&error() && {
    EMBERS_ASSERT(status_ == Status::kErr, "Unwrapping error of valid result");
    return std::move(container_.err_);
  }

 public:
  constexpr const T &value_or(const T &other) const & {
    if (status_ != Status::kOk) {
      return other;
    }
    return container_.ok_;
  }

  constexpr T &&value_or(T &&other) && {
    if (status_ != Status::kOk) {
      return std::move(other);
    }
    return std::move(container_.ok_);
  }

  constexpr const T &error_or(const E &other) const & {
    if (status_ != Status::kErr) {
      return other;
    }
    return container_.err_;
  }

  constexpr T &&error_or(E &&other) && {
    if (status_ != Status::kErr) {
      return std::move(other);
    }
    return std::move(container_.err_);
  }

 public:
  constexpr bool has_value() const { return status_ == Status::kOk; }

  template <class F>
  constexpr bool has_value_and(F &&f) const & {
    static_assert(
        std::is_same<std::invoke_result<F, T>::type, bool>::value,
        __FUNCTION__ " must be called with a (template) F argument that returns bool"
    );
    if (status_ != Status::kOk) {
      return false;
    }
    return std::invoke(std::forward<F>(f), container_.ok_);
  }

  template <class F>
  constexpr bool has_value_and(F &&f) && {
    static_assert(
        std::is_same<std::invoke_result<F, T>::type, bool>::value,
        __FUNCTION__ " must be called with a (template) F argument that returns bool"
    );
    if (status_ != Status::kOk) {
      return false;
    }
    return std::invoke(std::forward<F>(f), std::move(container_.ok_));
  }

 public:
  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f
  ) const & {
    // (n1t3): i'm not what contraints are valid for this case
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    if (status_ != Status::kOk) {
      return *this;
    }
    return MappedResult(  //
        std::invoke(std::forward<F>(f), (container_.ok_))
    );
  }

  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f) && {
    // (n1t3): i'm not what contraints are valid for this case
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    if (status_ != Status::kOk) {
      return *this;
    }
    return MappedResult(  //
        std::invoke(std::forward<F>(f), std::move(container_.ok_))
    );
  }

  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> map_err(F &&f
  ) const & {
    // (n1t3): i'm not what contraints are valid for this case
    using MappedResult = Result<T, typename std::invoke_result<F, E>::type>;
    using MappedError  = MappedResult::Error;
    if (status_ != Status::kErr) {
      return *this;
    }
    return MappedResult(  //
        MappedError(      //
            std::invoke(std::forward<F>(f), (container_.err_))
        )
    );
  }

  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> map_err(F &&f
  ) && {
    // (n1t3): i'm not what contraints are valid for this case
    using MappedResult = Result<T, typename std::invoke_result<F, E>::type>;
    using MappedError  = MappedResult::Error;
    if (status_ != Status::kErr) {
      return *this;
    }
    return MappedResult(  //
        MappedError(      //
            std::invoke(std::forward<F>(f), std::move(container_.err_))
        )
    );
  }
};

}  // namespace embers
