#pragma once

#include "defines/types.hpp"
#include "logger.hpp"

namespace embers {

namespace internal::result {
enum class Status : u8 {
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
    explicit Error(const E &error) { value = error; };
    explicit Error(E &&error) { value = std::move(error); };

    Error(const Error &error) { value = error; };
    Error(Error &&error) { value = std::move(error); };
  };

 private:
  typedef T                                Valid;
  typedef E                                Invalid;
  typedef embers::internal::result::Status Status;

  static_assert(!std::is_same<T, void>::value, "T must not be void");
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  //   struct TagErr {};
  //   struct TagOk {};

  union Container {
    Valid   ok_;
    Invalid err_;
    u8      dummy;
    constexpr Container() : dummy(0) {};
    ~Container() {}
  };

  Status    status_ = Status::kNotInit;
  Container container_;

  Result() = delete;

 public:
  constexpr Result(T &&ok) {
    new (&container_.ok_) T(std::move(ok));  //  container_.ok_ = ok;
    status_ = Status::kOk;
  }
  constexpr Result(Error &&err) {
    new (&container_.err_) E(std::move(err.value));  //  container_.err_ = err;
    status_ = Status::kErr;
  }
  constexpr Result(const T &ok) {
    new (&container_.ok_) T(ok);  //  container_.ok_ = ok;
    status_ = Status::kOk;
  }
  constexpr Result(const Error &err) {
    new (&container_.err_) E(err.value);  //  container_.err_ = err;
    status_ = Status::kErr;
  }

  constexpr Result(const Result &result) {
    switch (result.status_) {
      case Status::kOk:
        new (&container_.ok_) T(result.container_.ok_);  //  container_.ok_ = result.container_.ok_;
        break;
      case Status::kErr:
        new (&container_.err_) E(result.container_.err_);  //  container_.err_ = result.container_.err_;
        break;
      case Status::kNotInit:
        break;
    }
    status_ = result.status_;
  }
  constexpr Result(Result &&result) {
    switch (result.status_) {
      case Status::kOk:
        new (&container_.ok_) T(std::move(result.container_.ok_));  //  container_.ok_ = result.container_.ok_;
        break;
      case Status::kErr:
        new (&container_.err_) E(std::move(result.container_.err_));  //  container_.err_ = result.container_.err_;
        break;
      case Status::kNotInit:
        break;
    }
    status_        = result.status_;
    result.status_ = Status::kNotInit;
  }

 private:
  // Convert from Result <T1, E> into Result<T, E> (only if it is an error)
  template <typename T1>
  constexpr Result(const Result<T1, E> &result) {
    EMBERS_ASSERT(result.status_ != Status::kOk, "Attempting to convert Result types with different value types");
    if (result.status_ == Status::kErr) {
      new (&container_.err_) E(result.container_.err_);  //  container_.err_ = result.container_.err_;
    }
    status_ = result.status_;
  }

  template <typename T1>
  constexpr Result(Result<T1, E> &&result) {
    EMBERS_ASSERT(result.status_ != Status::kOk, "Attempting to convert Result types with different value types");
    if (result.status_ == Status::kErr) {
      new (&container_.err_) E(std::move(result.container_.err_));  //  container_.err_ = result.container_.err_;
    }

    status_        = result.status_;
    result.status_ = Status::kNotInit;
  }

  // Convert from Result <T, E1> into Result<T, E> (only if it is NOT an error)
  template <typename E1>
  constexpr Result(const Result<T, E1> &result) {
    EMBERS_ASSERT(result.status_ != Status::kErr, "Attempting to convert Result types with different error types");
    if (result.status_ == Status::kOk) {
      new (&container_.ok_) T(result.container_.ok_);  //  container_.err_ = result.container_.err_;
    }
    status_ = result.status_;
  }

  template <typename E1>
  constexpr Result(Result<T, E1> &&result) {
    EMBERS_ASSERT(result.status_ != Status::kErr, "Attempting to convert Result types with different error types");
    if (result.status_ == Status::kOk) {
      new (&container_.ok_) T(std::move(result.container_.ok_));  //  container_.err_ = result.container_.err_;
    }

    status_        = result.status_;
    result.status_ = Status::kNotInit;
  }

 public:
  //   constexpr Result(const T &ok) : Result(ok) {};
  //   constexpr Result(const Error &err) : Result(err) {};
  //   constexpr Result(T &&ok) : Result(std::move(ok)) {};
  //   constexpr Result(Error &&err) : Result(std::move(err)) {};

  // constructors if T is E (like Result<int, int>)
  constexpr static Result create_ok(const T &ok) { return Result(ok); };

  constexpr static Result create_err(const E &err) { return Result(Error(err)); };

  constexpr static Result create_ok(T &&ok) { return Result(std::move(ok)); };

  constexpr static Result create_err(E &&err) { return Result(Error(std::move(err))); };

  ~Result() {
    switch (status_) {
      case Status::kOk:
        container_.ok_.Valid::~Valid();
        break;
      case Status::kErr:
        container_.err_.Invalid::~Invalid();
        break;
      default:
        break;
    }
  };

 public:
  constexpr const T &value() const & {
    EMBERS_ASSERT(status_ == Status::kOk, "Unwrapping value of non ok result");
    return container_.ok_;
  }
  constexpr T &&value() && {
    EMBERS_ASSERT(status_ == Status::kOk, "Unwrapping value of non ok result");
    return std::move(container_.ok_);
  }
  constexpr const E &error() const & {
    EMBERS_ASSERT(status_ == Status::kErr, "Unwrapping error of non error result");
    return container_.err_;
  }
  constexpr const E &&error() && {
    EMBERS_ASSERT(status_ == Status::kErr, "Unwrapping error of non error result");
    return std::move(container_.err_);
  }

 public:
  constexpr const T &value_or(const T &other) const & {
    if (status_ == Status::kOk) {
      return container_.ok_;
    }
    return other;
  }
  constexpr T &&value_or(T &&other) && {
    if (status_ == Status::kOk) {
      return std::move(container_.ok_);
    }
    return std::move(other);
  }

  constexpr const T &error_or(const E &other) const & {
    if (status_ == Status::kErr) {
      return container_.err_;
    }
    return other;
  }
  constexpr T &&error_or(E &&other) && {
    if (status_ == Status::kErr) {
      return std::move(container_.err_);
    }
    return std::move(other);
  }

 public:
  constexpr bool has_value() const { return status_ == Status::kOk; }
  template <class F>
  constexpr bool has_value_and(F &&f) const & {
    if (status_ != Status::kOk) {
      return false;
    }
    return std::invoke(std::forward<F>(f), container_.ok_);
  }
  template <class F>
  constexpr bool has_value_and(F &&f) && {
    if (status_ != Status::kOk) {
      return false;
    }
    return std::invoke(std::forward<F>(f), std::move(container_.ok_));
  }

 public:
  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f) const & {
    if (status_ != Status::kOk) {
      return *this;
    }
    return Result<typename std::invoke_result<F, T>::type, E>(std::invoke(std::forward<F>(f), (container_.ok_)));
  }
  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f) && {
    if (status_ != Status::kOk) {
      return *this;
    }
    return Result<typename std::invoke_result<F, T>::type, E>(std::invoke(std::forward<F>(f), std::move(container_.ok_))
    );
  }

  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> map_err(F &&f) const & {
    if (status_ != Status::kErr) {
      return *this;
    }
    return Result<T, typename std::invoke_result<F, E>::type>(Error(std::invoke(std::forward<F>(f), (container_.err_)))
    );
  }
  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> map_err(F &&f) && {
    using ForeignResult = Result<T, typename std::invoke_result<F, E>::type>;
    using ForeignError  = typename ForeignResult::Error;
    if (status_ != Status::kErr) {
      return *this;
    }
    return ForeignResult(ForeignError(std::invoke(std::forward<F>(f), std::move(container_.err_))));
  }

 public:
  template <class F>
  constexpr auto map_or(F &&f, const T &value) const & {
    if (status_ != Status::kOk) {
      return value;
    }
    return Result(std::invoke(std::forward<F>(f), (container_.ok_)));
  }
  template <class F>
  constexpr auto map_or(F &&f) && {
    if (status_ != Status::kOk) {
      return value;
    }
    return Result(std::invoke(std::forward<F>(f), std::move(container_.ok_)));
  }
};

}  // namespace embers
