#pragma once

#include "defines/types.hpp"
#include "logger.hpp"

namespace embers {

template <typename T>
constexpr bool is_lvalue(T &) {
  return true;
}

template <typename T>
constexpr bool is_lvalue(T &&) {
  return false;
}

template <typename T>
class Option;

namespace internal {
enum class ResultStatus : u8 {
  kNotInit = 0,
  kOk      = 1,
  kErr     = 2,
};
}

template <typename T, typename E>
class Result {
  // to convert result between instances
  template <typename T1, typename E1>
  friend class Result;

  static_assert(!std::is_same<T, void>::value, "T must not be void");
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  // === Helper structs ===
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

  union Container {
    T  ok_;
    E  err_;
    u8 dummy;
    constexpr Container() : dummy() {};
    ~Container() {}
  };

  // === Fields ===

  Container container_;
  Status    status_ = Status::kNotInit;

  // === Constructors & Deconstructor ===
 public:
  Result() = delete;

  // ___ copy __
  constexpr Result(const T &ok) {
    new (&container_.ok_) T(ok);
    status_ = Status::kOk;
  }

  constexpr Result(const Error &err) {
    new (&container_.err_) E(err.value);
    status_ = Status::kErr;
  }

  constexpr static Result create_ok(const T &ok) { return Result(ok); };

  constexpr static Result create_err(const E &err) {
    return Result(Error(err));
  };

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

  // ___ move ___
  constexpr Result(T &&ok) {
    new (&container_.ok_) T(std::move(ok));
    status_ = Status::kOk;
  }

  constexpr Result(Error &&err) {
    new (&container_.err_) E(std::move(err.value));
    status_ = Status::kErr;
  }

  constexpr static Result create_ok(T &&ok) { return Result(std::move(ok)); };

  constexpr static Result create_err(E &&err) {
    return Result(Error(std::move(err)));
  };

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

  // ___ deconstructor ___
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

  // === Conversion constructors ===
  // they are only used by intenal methods such as map to convert types
  // and require checks before running
 private:
  // --- Result <T1, E> -> Result<T, E> (only if it is an error) ---
  template <typename T1>
  explicit constexpr Result(const Result<T1, E> &result) {
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
  explicit constexpr Result(Result<T1, E> &&result) {
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

  // --- Result <T, E1> -> Result<T, E> (only if it is NOT an error) ---
  template <typename E1>
  explicit constexpr Result(const Result<T, E1> &result) {
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
  explicit constexpr Result(Result<T, E1> &&result) {
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
  // === Other methods ===
  // --- Result Checking ---
  constexpr bool is_ok() const & { return status_ == Status::kOk; }
  constexpr bool is_err() const & { return status_ == Status::kErr; }

  // --- Result Validation ---
  // __ copy __
  template <class F>
  constexpr bool is_ok_and(F &&f) const & {
    static_assert(
        std::is_invocable<F, T>::value,
        "is_ok_and must be called with a (template) F argument that is "
        "invokable with T"
    );
    static_assert(
        std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
        "is_ok_and must be called with a (template) F argument that returns "
        "bool"
    );
    return is_ok() && std::invoke(std::forward<F>(f), container_.ok_);
  }

  template <class F>
  constexpr bool is_err_and(F &&f) const & {
    static_assert(
        std::is_invocable<F, E>::value,
        "is_err_and must be called with a (template) F argument that is "
        "invokable with E"
    );
    static_assert(
        std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
        "is_err_and must be called with a (template) F argument that returns "
        "bool and accepts E"
    );
    return is_err() && std::invoke(std::forward<F>(f), container_.err_);
  }

  // __ move __
  template <class F>
  constexpr bool is_ok_and(F &&f) && {
    static_assert(
        std::is_invocable<F, T>::value,
        "is_ok_and must be called with a (template) F argument that is "
        "invokable with T"
    );
    static_assert(
        std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
        "is_ok_and must be called with a (template) F argument that returns "
        "bool"
    );
    return is_ok() &&
           std::invoke(std::forward<F>(f), std::move(container_.ok_));
  }

  template <class F>
  constexpr bool is_err_and(F &&f) && {
    static_assert(
        std::is_invocable<F, E>::value,
        "is_err_and must be called with a (template) F argument that is "
        "invokable with E"
    );
    static_assert(
        std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
        "is_err_and must be called with a (template) F argument that returns "
        "bool and accepts E"
    );
    return is_err() &&
           std::invoke(std::forward<F>(f), std::move(container_.err_));
  }

  // --- Convertion into Option<T>/Option<E> ---
  // ___ copy ___
  constexpr Option<T> ok() const & {
    return is_ok() ? Option(container_.ok_) : Option<T>();
  }
  constexpr Option<E> err() const & {
    return is_err() ? Option(container_.err_) : Option<E>();
  }
  // ___ move ___
  constexpr Option<T> &&ok() && {
    auto option = is_ok() ? Option(std::move(container_.ok_)) : Option<T>();
    return std::move(option);
  }
  constexpr Option<E> &&err() && {
    auto option = is_err() ? Option(std::move(container_.err_)) : Option<E>();
    return std::move(option);
  }

  // --- Maps ---
  // ___ copy ___
  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> map(F &&f
  ) const & {
    static_assert(
        std::is_invocable<F, T>::value,
        "map must be called with a (template) F argument that is invokable "
        "with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "map must be called with a (template) F argument that doesn't "
        "return void"
    );
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    auto result =
        is_ok()
            ? MappedResult(std::invoke(std::forward<F>(f), (container_.ok_)))
            : MappedResult(*this);
    return result;
  }

  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> map_err(F &&f
  ) const & {
    static_assert(
        std::is_invocable<F, E>::value,
        "map_err must be called with a (template) F argument that is invokable "
        "with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, E>::type, void>::value,
        "map_err must be called with a (template) F argument that "
        "doesn't return void"
    );
    using MappedResult = Result<T, typename std::invoke_result<F, E>::type>;
    using MappedError  = typename MappedResult::Error;

    auto result =
        is_err()             //
            ? MappedResult(  //
                  MappedError(std::invoke(std::forward<F>(f), (container_.err_))
                  )
              )
            : MappedResult(*this);
    return result;
  }

  // ___ move ___

  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> &&map(F &&f) && {
    static_assert(
        std::is_invocable<F, T>::value,
        "map must be called with a (template) F argument that is invokable "
        "with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "map must be called with a (template) F argument that doesn't "
        "return void"
    );
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    auto result =
        is_ok() ? MappedResult(
                      std::invoke(std::forward<F>(f), std::move(container_.ok_))
                  )
                : MappedResult(std::move(*this));

    return std::move(result);
  }

  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> map_err(F &&f
  ) && {
    static_assert(
        std::is_invocable<F, E>::value,
        "map_err must be called with a (template) F argument that is invokable "
        "with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, E>::type, void>::value,
        "map_err must be called with a (template) F argument that "
        "doesn't return void"
    );
    using MappedResult = Result<T, typename std::invoke_result<F, E>::type>;
    using MappedError  = typename MappedResult::Error;

    auto result = is_err()             //
                      ? MappedResult(  //
                            MappedError(std::invoke(
                                std::forward<F>(f),
                                (std::move(container_.err_))
                            ))
                        )
                      : MappedResult(std::move(*this));
    return std::move(result);
  }

  // --- Map or ---
  // ___ copy ___
  template <class F>
  constexpr typename std::invoke_result<F, T>::type map_or(
      typename std::invoke_result<F, T>::type d, F &&f
  ) const & {
    static_assert(
        std::is_invocable<F, T>::value,
        "map_or must be called with a (template) F argument that is invokable "
        "with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "map_or must be called with a (template) F argument that doesn't "
        "return void"
    );
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    auto result =
        is_ok() ? std::invoke(std::forward<F>(f), (container_.ok_)) : d;
    return result;
  }

  // ___ move ___

  template <class F>
  constexpr typename std::invoke_result<F, T>::type &&map_or(
      typename std::invoke_result<F, T>::type &&d, F &&f
  ) && {
    static_assert(
        std::is_invocable<F, T>::value,
        "map_or must be called with a (template) F argument that is invokable "
        "with T "
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "map_or must be called with a (template) F argument that doesn't "
        "return void"
    );
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    auto result =
        is_ok() ? std::invoke(std::forward<F>(f), std::move(container_.ok_))
                : std::move(d);

    return std::move(result);
  }

  // --- Map or else ---
  // ___ copy ___
  template <class F, class D>
  constexpr typename std::invoke_result<F, T>::type map_or_else(D &&d, F &&f)
      const & {
    static_assert(
        std::is_invocable<F, T>::value,
        "map_or_else must be called with a (template) F argument that is "
        "invokable with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "map_or_else must be called with a (template) F argument that doesn't "
        "return void"
    );
    static_assert(
        std::is_invocable<D, E>::value,
        "map_or_else must be called with a (template) D argument that is "
        "invokable with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<D, E>::type, void>::value,
        "map_or_else must be called with a (template) D argument that doesn't "
        "return void"
    );
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    auto result = is_ok() ? std::invoke(std::forward<F>(f), (container_.ok_))
                          : std::invoke(std::forward<D>(d), (container_.err_));
    return result;
  }

  // ___ move ___

  template <class F, class D>
  constexpr typename std::invoke_result<F, T>::type &&map_or_else(
      D &&d, F &&f
  ) && {
    static_assert(
        std::is_invocable<F, T>::value,
        "map_or_else must be called with a (template) F argument that is "
        "invokable with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "map_or_else must be called with a (template) F argument that doesn't "
        "return void"
    );
    static_assert(
        std::is_invocable<D, E>::value,
        "map_or_else must be called with a (template) D argument that is "
        "invokable with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<D, E>::type, void>::value,
        "map_or_else must be called with a (template) D argument that doesn't "
        "return void"
    );
    using MappedResult = Result<typename std::invoke_result<F, T>::type, E>;
    auto result =
        is_ok() ? std::invoke(std::forward<F>(f), (std::move(container_.ok_)))
                : std::invoke(std::forward<D>(d), (std::move(container_.err_)));
    return std::move(result);
  }

  // --- Inspect ---
  // ___ copy ___
  template <class F>
  constexpr Result inspect(F &&f) const {
    static_assert(
        std::is_invocable<F, const T>::value,
        "inspect must be called with a (template) F argument that is "
        "invokable with const T"
    );
    static_assert(
        std::is_same<typename std::invoke_result<F, const T>::type, void>::
            value,
        "inspect must be called with a (template) F argument that returns "
        "void"
    );
    if (is_ok()) {
      std::invoke(std::forward<F>(f), (const T)(container_.ok_));
    }
    return *this;
  }

  template <class F>
  constexpr Result inspect_err(F &&f) const {
    static_assert(
        std::is_invocable<F, const E>::value,
        "inspect_err must be called with a (template) F argument that is "
        "invokable with const E"
    );
    static_assert(
        std::is_same<typename std::invoke_result<F, const E>::type, void>::
            value,
        "inspect_err must be called with a (template) F argument that returns "
        "void"
    );
    if (is_err()) {
      std::invoke(std::forward<F>(f), (const E)(container_.err_));
    }
    return *this;
  }

  // --- Unwraps ---
 public:
  constexpr T unwrap() const & {
    EMBERS_ASSERT(status_ == Status::kOk, "Unwrapping value of invalid result");
    return container_.ok_;
  }

  constexpr T &&unwrap() && {
    EMBERS_ASSERT(status_ == Status::kOk, "Unwrapping value of invalid result");
    return std::move(container_.ok_);
  }

  constexpr E unwrap_err() const & {
    EMBERS_ASSERT(status_ == Status::kErr, "Unwrapping error of valid result");
    return container_.err_;
  }

  constexpr E &&unwrap_err() && {
    EMBERS_ASSERT(status_ == Status::kErr, "Unwrapping error of valid result");
    return std::move(container_.err_);
  }

  constexpr T unwrap_or(const T &other) const & {
    return is_ok() ? container_.ok_ : other;
  }

  constexpr T &&unwrap_or(T &&other) && {
    auto result = is_ok() ? std::move(container_.ok_) : std::move(other);
    return std::move(result);
  }

  template <class F>
  constexpr T unwrap_or_else(F &&f) const & {
    static_assert(
        std::is_invocable<F, E>::value,
        "unwrap_or_else must be called with a (template) F argument that is "
        "invokable with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "unwrap_or_else must be called with a (template) F argument that "
        "doesn't return void"
    );
    return is_ok() ? container_.ok_
                   : std::invoke(std::forward<F>(f), (container_.err_));
    ;
  }

  template <class F>
  constexpr T &&unwrap_or_else(F &&f) && {
    static_assert(
        std::is_invocable<F, E>::value,
        "unwrap_or_else must be called with a (template) F argument that is "
        "invokable with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "unwrap_or_else must be called with a (template) F argument that "
        "doesn't return void"
    );
    auto result = is_ok() ? std::move(container_.ok_)
                          : std::invoke(std::forward<F>(f), (container_.err_));
    return std::move(result);
  }

  // --- And & Or ---
  template <typename U>
  constexpr Result<U, E> also(const Result<U, E> &other) const & {
    return is_ok() ? other : Result<U, E>(*this);
  }

  template <typename F>
  constexpr Result<T, F> otherwise(const Result<T, F> &other) const & {
    return is_err() ? other : Result<T, F>(*this);
  }

  // ___ move ___
  template <typename U>
  constexpr Result<U, E> &&also(Result<U, E> &&other) && {
    auto result = is_ok() ? std::move(other) : Result<U, E>(std::move(*this));
    return std::move(result);
  }

  template <typename F>
  constexpr Result<T, F> &&otherwise(Result<T, F> &&other) && {
    auto result = is_err() ? std::move(other) : Result<T, F>(std::move(*this));
    return std::move(result);
  }

  // --- Then & Else ---
  template <class F>
  constexpr Result<typename std::invoke_result<F, T>::type, E> then(  //
      F &&f
  ) const & {
    static_assert(
        std::is_invocable<F, T>::value,
        "then must be called with a (template) F argument that is "
        "invokable with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "then must be called with a (template) F argument that doesn't "
        "return void"
    );
    return  //
        is_ok()
            ? Result<U, E>(std::invoke(std::forward<F>(f), (container_.ok_)))
            : Result<U, E>(*this);
  }

  template <class F>
  constexpr Result<T, typename std::invoke_result<F, E>::type> alternatively(
      F &&f
  ) const & {
    static_assert(
        std::is_invocable<F, E>::value,
        "alternatively must be called with a (template) F argument that is "
        "invokable with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, E>::type, void>::value,
        "alternatively must be called with a (template) F argument that "
        "doesn't "
        "return void"
    );
    return  //
        is_err()
            ? Result<T, G>(std::invoke(std::forward<F>(f), (container_.err_)))
            : Result<T, G>(*this);
  }

  // ___ move ___
  template <typename U, class F>
  constexpr Result<U, E> &&then(F &&f) && {
    static_assert(
        std::is_invocable<F, T>::value,
        "then must be called with a (template) F argument that is "
        "invokable with T"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, T>::type, void>::value,
        "then must be called with a (template) F argument that doesn't "
        "return void"
    );
    auto result =
        is_ok()
            ? Result<U, E>(
                  std::invoke(std::forward<F>(f), (std::move(container_.ok_)))
              )
            : Result<U, E>(std::move(*this));
    return std::move(result);
  }

  template <typename G, class F>
  constexpr Result<T, G> &&alternatively(F &&f) && {
    static_assert(
        std::is_invocable<F, E>::value,
        "alternatively must be called with a (template) F argument that is "
        "invokable with E"
    );
    static_assert(
        !std::is_same<typename std::invoke_result<F, E>::type, void>::value,
        "alternatively must be called with a (template) F argument that "
        "doesn't "
        "return void"
    );
    auto result =
        is_err()
            ? Result<T, G>(
                  std::invoke(std::forward<F>(f), (std::move(container_.err_)))
              )
            : Result<T, G>(std::move(*this));
    return std::move(result);
  }
};
}  // namespace embers
