#pragma once

#include <embers/defines/types.hpp>
#include <embers/logger.hpp>

namespace embers {
template <typename T>
class Option;

template <typename T, typename E>
class Result {
  template <typename T1, typename E1>
  friend class Result;

  struct TagErr {};
  struct Mono {};

  static_assert(!std::is_same<T, void>::value, "T must not be void");
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  enum class State : u8 { kOk, kErr, kNotInit };
  union Container {
    T    ok_;
    E    err_;
    Mono dummy;
    constexpr Container() : dummy() {};
    ~Container() {}
  };

 private:
  constexpr Result(const T &ok);
  constexpr Result(T &&ok);
  constexpr Result(const E &err, TagErr);
  constexpr Result(E &&err, TagErr);

  constexpr T        move_ok() &&;
  constexpr E        move_err() &&;
  constexpr const T &copy_ok() const &;
  constexpr const E &copy_err() const &;

 public:
  Result() = delete;
  constexpr Result(const Result &result);
  constexpr Result(Result &&result);
  ~Result();

  // --- constructors ---

  constexpr static Result create_ok(const T &ok);
  constexpr static Result create_ok(T &&ok);
  constexpr static Result create_err(const E &err);
  constexpr static Result create_err(E &&err);

  // --- result checks ---

  constexpr bool is_ok() const;
  constexpr bool is_err() const;

  // --- result validation ---
  // -- oks --

  template <typename F>
  constexpr auto is_ok_and(F &&f) const & -> typename std::enable_if<
      std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
      bool>::type;

  template <typename F>
  constexpr auto is_ok_and(F &&f) && -> typename std::enable_if<
      std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
      bool>::type;

  // -- errs --
  template <typename F>
  constexpr auto is_err_and(F &&f) const & -> typename std::enable_if<
      std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
      bool>::type;

  template <typename F>
  constexpr auto is_err_and(F &&f) && -> typename std::enable_if<
      std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
      bool>::type;

  // --- conversion into options ---

  constexpr auto ok() const &;
  constexpr auto ok() &&;
  constexpr auto err() const &;
  constexpr auto err() &&;

  // --- maps ---

  template <typename F>
  constexpr auto map(F &&f) const & -> typename std::enable_if<
      std::is_invocable<F, T>::value,
      Result<typename std::invoke_result<F, T>::type, E>>::type;

  template <typename F>
  constexpr auto map(F &&f) && -> typename std::enable_if<
      std::is_invocable<F, T>::value,
      Result<typename std::invoke_result<F, T>::type, E>>::type;

  template <typename F>
  constexpr auto map_err(F &&f) const & -> typename std::enable_if<
      std::is_invocable<F, E>::value,
      Result<T, typename std::invoke_result<F, E>::type>>::type;

  template <typename F>
  constexpr auto map_err(F &&f) && -> typename std::enable_if<
      std::is_invocable<F, E>::value,
      Result<T, typename std::invoke_result<F, E>::type>>::type;

  // --- (map_or)s ---

  template <typename F>
  constexpr auto map_or(const typename std::invoke_result<F, T>::type &d, F &&f)
      const & -> typename std::enable_if<
          std::is_invocable<F, T>::value,
          typename std::invoke_result<F, T>::type>::type;

  template <typename F>
  constexpr auto map_or(typename std::invoke_result<F, T>::type &&d, F &&f)
      && -> typename std::enable_if<
          std::is_invocable<F, T>::value,
          typename std::invoke_result<F, T>::type>::type;

  template <typename F, typename D>
  constexpr auto map_or_else(D &&d, F &&f) const & -> typename std::enable_if<
      std::is_invocable<F, T>::value && std::is_invocable<D, E>::value &&
          std::is_same<
              typename std::invoke_result<F, T>::type,
              typename std::invoke_result<D, E>::type>::value,
      typename std::invoke_result<F, T>::type>::type;

  template <typename F, typename D>
  constexpr auto map_or_else(D &&d, F &&f) && -> typename std::enable_if<
      std::is_invocable<F, T>::value && std::is_invocable<D, E>::value &&
          std::is_same<
              typename std::invoke_result<F, T>::type,
              typename std::invoke_result<D, E>::type>::value,
      typename std::invoke_result<F, T>::type>::type;

  // --- inspect ---
  template <typename F>
  constexpr auto inspect(F &&f) const & -> typename std::
      enable_if<std::is_invocable<F, T>::value, const Result<T, E> &>::type;

  template <typename F>
  constexpr auto inspect(F &&f) && -> typename std::
      enable_if<std::is_invocable<F, T>::value, Result<T, E>>::type;

  template <typename F>
  constexpr auto inspect(F &&f) & -> typename std::
      enable_if<std::is_invocable<F, T>::value, Result<T, E> &>::type;

  // template <typename F>

  template <typename F>
  constexpr auto inspect_err(F &&f) const & -> typename std::
      enable_if<std::is_invocable<F, E>::value, const Result<T, E> &>::type;

  template <typename F>
  constexpr auto inspect_err(F &&f) && -> typename std::
      enable_if<std::is_invocable<F, E>::value, Result<T, E>>::type;

  template <typename F>
  constexpr auto inspect_err(F &&f) & -> typename std::
      enable_if<std::is_invocable<F, E>::value, Result<T, E> &>::type;

  constexpr T unwrap() const &;
  constexpr T unwrap() &&;
  constexpr E unwrap_err() const &;
  constexpr E unwrap_err() &&;
  constexpr T unwrap_or(const T &other) const &;
  constexpr T unwrap_or(T &&other) &&;

  template <typename F>
  constexpr auto unwrap_or_else(F &&f) const & -> typename std::enable_if<
      std::is_invocable<F, E>::value &&
          std::is_same<typename std::invoke_result<F, E>::type, T>::value,
      T>;

  template <typename F>
  constexpr auto unwrap_or_else(F &&f) && -> typename std::enable_if<
      std::is_invocable<F, E>::value &&
          std::is_same<typename std::invoke_result<F, E>::type, T>::value,
      T>;

 private:
 public:  // todo
  Container container_;
  State     state_ = State::kNotInit;
};

}  // namespace embers

// ---------------------------- implementation ----------------------------

namespace embers {

template <typename T, typename E>
constexpr Result<T, E>::Result(const T &ok) {
  new (&container_.ok_) T(ok);
  state_ = State::kOk;
}

template <typename T, typename E>
constexpr Result<T, E>::Result(T &&ok) {
  new (&container_.ok_) T(std::move(ok));
  state_ = State::kOk;
}

template <typename T, typename E>
constexpr Result<T, E>::Result(const E &err, TagErr) {
  new (&container_.err_) E(err);
  state_ = State::kErr;
}

template <typename T, typename E>
constexpr Result<T, E>::Result(E &&err, TagErr) {
  new (&container_.err_) E(std::move(err));
  state_ = State::kErr;
}

template <typename T, typename E>
constexpr Result<T, E>::Result(const Result &result) {
  switch (result.state_) {
    case State::kOk:
      new (&container_.ok_) T(result.container_.ok_);
      break;
    case State::kErr:
      new (&container_.err_) E(result.container_.err_);
      break;
  }
  state_ = result.state_;
}

template <typename T, typename E>
constexpr Result<T, E>::Result(Result &&result) {
  switch (result.state_) {
    case State::kOk:
      new (&container_.ok_) T(std::move(result.container_.ok_));
      break;
    case State::kErr:
      new (&container_.err_) E(std::move(result.container_.err_));
      break;
    case State::kNotInit:
    default:
      break;
  }
  state_        = result.state_;
  result.state_ = State::kNotInit;
}

template <typename T, typename E>
Result<T, E>::~Result() {
  switch (state_) {
    case State::kOk:
      container_.ok_.T::~T();
      break;
    case State::kErr:
      container_.err_.E::~E();
      break;
    case State::kNotInit:
    default:
      break;
  }
}

template <typename T, typename E>
constexpr T Result<T, E>::move_ok() && {
  T arg  = std::move(container_.ok_);
  state_ = State::kNotInit;
  return arg;  // copy elision, no move
}

template <typename T, typename E>
constexpr E Result<T, E>::move_err() && {
  E arg  = std::move(container_.err_);
  state_ = State::kNotInit;
  return arg;  // copy elision, no move
}

template <typename T, typename E>
constexpr const T &Result<T, E>::copy_ok() const & {
  return container_.ok_;  // copy elision, no move
}

template <typename T, typename E>
constexpr const E &Result<T, E>::copy_err() const & {
  return container_.err_;  // copy elision, no move
}

template <typename T, typename E>
constexpr Result<T, E> Result<T, E>::create_ok(const T &ok) {
  return Result(ok);
}

template <typename T, typename E>
constexpr Result<T, E> Result<T, E>::create_ok(T &&ok) {
  return Result(std::move(ok));
}

template <typename T, typename E>
constexpr Result<T, E> Result<T, E>::create_err(const E &err) {
  return Result(err, {});
}

template <typename T, typename E>
constexpr Result<T, E> Result<T, E>::create_err(E &&err) {
  return Result(std::move(err), {});
}

template <typename T, typename E>
constexpr bool Result<T, E>::is_ok() const {
  return state_ == State::kOk;
}

template <typename T, typename E>
constexpr bool Result<T, E>::is_err() const {
  return state_ == State::kErr;
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_ok_and(F &&f) const & ->
    typename std::enable_if<
        std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
        bool>::type {
  return is_ok() &&  //
         std::invoke(std::forward<F>(f), copy_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_ok_and(F &&f) && -> typename std::enable_if<
    std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
    bool>::type {
  return is_ok() &&  //
         std::invoke(std::forward<F>(f), std::move(*this).move_ok());
};

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_err_and(F &&f) const & ->
    typename std::enable_if<
        std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
        bool>::type {
  return is_err() &&  //
         std::invoke(std::forward<F>(f), copy_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_err_and(F &&f) && -> typename std::enable_if<
    std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
    bool>::type {
  return is_err() &&  //
         std::invoke(std::forward<F>(f), std::move(*this).move_err());
};

template <typename T, typename E>
constexpr auto Result<T, E>::ok() const & {
  return is_ok() ? Option<T>(copy_ok()) : Option<T>();
}

template <typename T, typename E>
constexpr auto Result<T, E>::ok() && {
  return is_ok() ? Option<T>(std::move(*this).move_ok()) : Option<T>();
}

template <typename T, typename E>
constexpr auto Result<T, E>::err() const & {
  return is_err() ? Option<E>(copy_err()) : Option<E>();
}

template <typename T, typename E>
constexpr auto Result<T, E>::err() && {
  return is_err() ? Option<E>(std::move(*this).move_err()) : Option<E>();
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map(F &&f) const & -> typename std::enable_if<
    std::is_invocable<F, T>::value,
    Result<typename std::invoke_result<F, T>::type, E>>::type {
  using ReturnType = Result<typename std::invoke_result<F, T>::type, E>;
  return is_ok()  //
             ? ReturnType::create_ok(std::invoke(std::forward<F>(f), copy_ok()))
             : ReturnType::create_err(copy_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map(F &&f) && -> typename std::enable_if<
    std::is_invocable<F, T>::value,
    Result<typename std::invoke_result<F, T>::type, E>>::type {
  using ReturnType = Result<typename std::invoke_result<F, T>::type, E>;
  return is_ok()  //
             ? ReturnType::create_ok(
                   std::invoke(std::forward<F>(f), std::move(*this).move_ok())
               )
             : ReturnType::create_err(std::move(*this).move_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map_err(F &&f) const & -> typename std::enable_if<
    std::is_invocable<F, E>::value,
    Result<T, typename std::invoke_result<F, E>::type>>::type {
  using ReturnType = Result<T, typename std::invoke_result<F, E>::type>;
  return is_err()  //
             ? ReturnType::create_err(
                   std::invoke(std::forward<F>(f), copy_err())
               )
             : ReturnType::create_ok(copy_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map_err(F &&f) && -> typename std::enable_if<
    std::is_invocable<F, E>::value,
    Result<T, typename std::invoke_result<F, E>::type>>::type {
  using ReturnType = Result<T, typename std::invoke_result<F, E>::type>;
  return is_err()  //
             ? ReturnType::create_err(
                   std::invoke(std::forward<F>(f), std::move(*this).move_err())
               )
             : ReturnType::create_ok(std::move(*this).move_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map_or(
    const typename std::invoke_result<F, T>::type &d, F &&f
) const & ->
    typename std::enable_if<
        std::is_invocable<F, T>::value,
        typename std::invoke_result<F, T>::type>::type {
  return is_ok()  //
             ? std::invoke(std::forward<F>(f), copy_ok())
             : d;
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map_or(
    typename std::invoke_result<F, T>::type &&d, F &&f
) && ->
    typename std::enable_if<
        std::is_invocable<F, T>::value,
        typename std::invoke_result<F, T>::type>::type {
  return is_ok()  //
             ? std::invoke(std::forward<F>(f), std::move(*this).move_ok())
             : std::move(d);
}

template <typename T, typename E>
template <typename F, typename D>
constexpr auto Result<T, E>::map_or_else(D &&d, F &&f) const & ->
    typename std::enable_if<
        std::is_invocable<F, T>::value && std::is_invocable<D, E>::value &&
            std::is_same<
                typename std::invoke_result<F, T>::type,
                typename std::invoke_result<D, E>::type>::value,
        typename std::invoke_result<F, T>::type>::type {
  return is_ok()  //
             ? std::invoke(std::forward<F>(f), copy_ok())
             : std::invoke(std::forward<D>(d), copy_err());
}

template <typename T, typename E>
template <typename F, typename D>
constexpr auto Result<T, E>::map_or_else(D &&d, F &&f) && ->
    typename std::enable_if<
        std::is_invocable<F, T>::value && std::is_invocable<D, E>::value &&
            std::is_same<
                typename std::invoke_result<F, T>::type,
                typename std::invoke_result<D, E>::type>::value,
        typename std::invoke_result<F, T>::type>::type {
  return is_ok()  //
             ? std::invoke(std::forward<F>(f), std::move(*this).move_ok())
             : std::invoke(std::forward<D>(d), std::move(*this).move_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect(F &&f) const & -> typename std::
    enable_if<std::is_invocable<F, T>::value, const Result<T, E> &>::type {
  if (is_ok()) {
    std::invoke(std::forward<F>(f), copy_ok());
  }
  return *this;
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect(F &&f) && -> typename std::
    enable_if<std::is_invocable<F, T>::value, Result<T, E>>::type {
  if (is_ok()) {
    std::invoke(std::forward<F>(f), copy_ok());
  }
  return Result<T, E>(std::move(*this));
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect(F &&f) & -> typename std::
    enable_if<std::is_invocable<F, T>::value, Result<T, E> &>::type {
  if (is_ok()) {
    std::invoke(std::forward<F>(f), copy_ok());
  }
  return *this;
}

// dfkgopsdkplgk

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect_err(F &&f) const & -> typename std::
    enable_if<std::is_invocable<F, E>::value, const Result<T, E> &>::type {
  if (is_err()) {
    std::invoke(std::forward<F>(f), copy_err());
  }
  return *this;
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect_err(F &&f) && -> typename std::
    enable_if<std::is_invocable<F, E>::value, Result<T, E>>::type {
  if (is_err()) {
    std::invoke(std::forward<F>(f), copy_err());
  }
  return Result<T, E>(std::move(*this));
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect_err(F &&f) & -> typename std::
    enable_if<std::is_invocable<F, E>::value, Result<T, E> &>::type {
  if (is_err()) {
    std::invoke(std::forward<F>(f), copy_err());
  }
  return *this;
}

template <typename T, typename E>
constexpr T Result<T, E>::unwrap() const & {
  return copy_ok();
}
template <typename T, typename E>
constexpr T Result<T, E>::unwrap() && {
  return std::move(*this).move_ok();
}
template <typename T, typename E>
constexpr E Result<T, E>::unwrap_err() const & {
  return copy_err();
}
template <typename T, typename E>
constexpr E Result<T, E>::unwrap_err() && {
  return std::move(*this).move_err();
}
template <typename T, typename E>
constexpr T Result<T, E>::unwrap_or(const T &other) const & {
  return is_ok() ? copy_ok() : copy_err();
}
template <typename T, typename E>
constexpr T Result<T, E>::unwrap_or(T &&other) && {
  return is_ok() ? std::move(*this).move_ok() : std::move(*this).move_err();
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::unwrap_or_else(F &&f) const & ->
    typename std::enable_if<
        std::is_invocable<F, E>::value &&
            std::is_same<typename std::invoke_result<F, E>::type, T>::value,
        T> {
  return is_ok() ? copy_ok() : copy_err();
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::unwrap_or_else(F &&f) && ->
    typename std::enable_if<
        std::is_invocable<F, E>::value &&
            std::is_same<typename std::invoke_result<F, E>::type, T>::value,
        T> {
  return is_ok() ? std::move(*this).move_ok() : std::move(*this).move_err();
}

}  // namespace embers

template <typename T, typename E>
class fmt::formatter<embers::Result<T, E>> {
  using Result = embers::Result<T, E>;

 public:
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(Result const &result, Context &ctx) const {
    if (result.is_ok()) {
      return format_to(ctx.out(), "Ok({:?})", result.container_.ok_);  // todo
    }
    if (result.is_err()) {
      return format_to(ctx.out(), "Err({:?})", result.container_.err_);  // todo
    }
    return format_to(ctx.out(), "NotInit");
  }
};
