/**
 * @file result.hpp
 * @author n1t3 (n1t3@tuta.io)
 * @brief Defines the Result template class which encapsulates a value or an
 * error.
 * @remark it supposed to be used inplace of C++ exceptions but it generates a
 * gross assembly during unwrapping: moves are not free and code generates a ton
 * of null result objects that are not optimized out
 * @version 1.0
 * @date 2024-11-09
 *
 * @copyright 2024 n1t3
 *
 */

#pragma once

#include <embers/defines/types.hpp>
#include <embers/logger.hpp>

namespace embers {
template <typename T>
class Option;

/**
 * @brief The Result class encapsulates either a value of type T or an error of
 * type E.
 *
 * @tparam T Type of the value that may be stored in the Result.
 * @tparam E Type of the error that may be stored in the Result.
 */
template <typename T, typename E>
class Result {
  template <typename T1, typename E1>
  friend class Result;

  // --- helper classes ---

  /// @brief A marker used for distinguishing error construction.
  struct TagErr {};

  /// @brief A marker class used for internal union initialization.
  struct Mono {};

  static_assert(!std::is_same<T, void>::value, "T must not be void");
  static_assert(!std::is_same<E, void>::value, "E must not be void");

  /// @brief Enumerates the possible states of a Result object.
  enum class State : u8 { kOk, kErr, kNotInit };

  /// @brief Union used to store either a value or an error.
  union Container {
    T    ok_;
    E    err_;
    Mono dummy;
    constexpr Container() : dummy() {};
    EMBERS_INLINE ~Container() {}
  };

 private:
  /**
   * @brief Constructor to create a successful Result with a value.
   * @param ok The value of type T that will be stored in the Result object.
   */
  constexpr Result(const T &ok);
  /**
   * @brief Constructor to create a successful Result with an rvalue reference
   * value.
   * @param ok The rvalue reference value of type T that will be moved into the
   * Result object.
   */
  constexpr Result(T &&ok);
  /**
   * @brief Constructor to create a failed Result with a value.
   * @param err The error value of type E that will be stored in the Result
   * object.
   * @param TagErr A tag used to distinguish this constructor from others.
   */
  constexpr Result(const E &err, TagErr);
  /**
   * @brief Constructor to create a failed Result with an rvalue reference
   * value.
   * @param err The rvalue reference error value of type E that will be moved
   * into the Result object.
   * @param TagErr A tag used to distinguish this constructor from others.
   */
  constexpr Result(E &&err, TagErr);

  /**
   * @brief Move the value of type T from a successful Result.
   *
   * This function moves the value stored in a successful Result object and
   * returns it. The state of the Result will be left uninitialized.
   *
   * @return The moved value of type T.
   */
  constexpr T move_ok() &&;
  /**
   * @brief Move the error of type E from a failed Result.
   *
   * This function moves the error value stored in a failed Result object and
   * returns it. The state of the Result will be left uninitialized.
   *
   * @return The moved error value of type E.
   */
  constexpr E move_err() &&;
  /**
   * @brief Return a reference to the stored value of type T.
   *
   * This function assumes that the Result object is in a success state (kOk).
   *
   * @return A const reference to the stored value of type T.
   */
  constexpr const T &ref_ok() const &;

  /**
   * @brief Return a reference to the stored error value of type E.
   *
   * This function assumes that the Result object is in an error state (kErr).
   *
   * @return A const reference to the stored error value of type E.
   */
  constexpr const E &ref_err() const &;

 public:
  /// @brief Deleted to prevent uninitilialized results
  Result() = delete;
  /**
   * @brief Copy constructor for Result.
   * @param result The other `Result` object to copy from.
   */
  constexpr Result(const Result &result);

  /**
   * @brief Move constructor for Result.
   * @param result The other `Result` object to move from.
   */
  constexpr Result(Result &&result);
  EMBERS_INLINE ~Result();

  // --- constructors ---

  /**
   * @brief Create a new Result object with a value of type T.
   * @param ok The value to encapsulate in the result object.
   * @return A newly created Result object that is in the "ok" state and
   * contains the given value.
   */
  constexpr static Result create_ok(const T &ok);
  /**
   * @brief Create a new Result object with a value of type T (r-value
   * reference).
   * @param ok The r-value reference value to encapsulate in the result object.
   * @return A newly created Result object that is in the "ok" state and
   * contains the given value.
   */
  constexpr static Result create_ok(T &&ok);
  /**
   * @brief Create a new Result object with an error of type E.
   * @param err The error to encapsulate in the result object.
   * @return A newly created Result object that is in the "err" state and
   * contains the given error.
   */
  constexpr static Result create_err(const E &err);
  /**
   * @brief Create a new Result object with an error of type E (r-value
   * reference).
   * @param err The r-value reference error to encapsulate in the result object.
   * @return A newly created Result object that is in the "err" state and
   * contains the given error.
   */
  constexpr static Result create_err(E &&err);

  // --- result checks ---

  /**
   * @brief Check if the Result object contains a value.
   * @return True if the result is in an "ok" state, false otherwise.
   */
  constexpr bool is_ok() const;
  /**
   * @brief Check if the Result object contains an error.
   * @return True if the result is in an "err" state, false otherwise.
   */
  constexpr bool is_err() const;

  // --- result validation ---
  // -- oks --

  /**
   * @brief Check if the Result object is in an "ok" state and satisfies a
   * condition.
   *
   * @tparam F The type of the function or functor to be invoked.
   * @param f A callable object that takes a parameter of type T and returns a
   * boolean.
   * @return True if the result is in an "ok" state and invoking `f` on the
   * contained value returns true, false otherwise.
   */
  template <typename F>
  constexpr auto is_ok_and(F &&f) const & ->
      typename std::enable_if<
          std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
          bool>::type;

  /**
   * @brief Check if the Result object is in an "ok" state and satisfies a
   * condition (move version).
   *
   * @tparam F The type of the function or functor to be invoked.
   * @param f A callable object that takes a parameter of type T and returns a
   * boolean.
   * @return True if the result is in an "ok" state and invoking `f` on the
   * contained value returns true, false otherwise.
   */
  template <typename F>
  constexpr auto is_ok_and(F &&f) && ->
      typename std::enable_if<
          std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
          bool>::type;

  // -- errs --
  /**
   * @brief Check if the Result object is in an "err" state and satisfies a
   * condition.
   *
   * @tparam F The type of the function or functor to be invoked.
   * @param f A callable object that takes a parameter of type E and returns a
   * boolean.
   * @return True if the result is in an "err" state and invoking `f` on the
   * contained error returns true, false otherwise.
   */
  template <typename F>
  constexpr auto is_err_and(F &&f) const & ->
      typename std::enable_if<
          std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
          bool>::type;

  /**
   * @brief Check if the Result object is in an "err" state and satisfies a
   * condition (move version).
   *
   * @tparam F The type of the function or functor to be invoked.
   * @param f A callable object that takes a parameter of type E and returns a
   * boolean.
   * @return True if the result is in an "err" state and invoking `f` on the
   * contained error returns true, false otherwise.
   */
  template <typename F>
  constexpr auto is_err_and(F &&f) && ->
      typename std::enable_if<
          std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
          bool>::type;

  // --- conversion into options ---

  // constexpr auto ok() const &;
  // constexpr auto ok() &&;
  // constexpr auto err() const &;
  // constexpr auto err() &&;

  // --- maps ---

  /**
   * @brief Apply a function to the contained value if the result is in an "ok"
   * state.
   *
   * @tparam F The type of the function or functor to be applied to the value.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return A new Result instance containing either the result of applying `f`
   * to the contained value (if in "ok" state) or the original error (if in
   * "err" state).
   */
  template <typename F>
  constexpr auto map(F &&f) const & ->
      typename std::enable_if<
          std::is_invocable<F, T>::value,
          Result<typename std::invoke_result<F, T>::type, E>>::type;

  /**
   * @brief Apply a function to the contained value if the result is in an "ok"
   * state (move version).
   *
   * @tparam F The type of the function or functor to be applied to the value.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return A new Result instance containing either the result of applying `f`
   * to the contained value (if in "ok" state) or the original error (if in
   * "err" state).
   */
  template <typename F>
  constexpr auto map(F &&f) && ->
      typename std::enable_if<
          std::is_invocable<F, T>::value,
          Result<typename std::invoke_result<F, T>::type, E>>::type;

  /**
   * @brief Apply a function to the contained error if the result is in an "err"
   * state.
   *
   * @tparam F The type of the function or functor to be applied to the error.
   * @param f A callable object that takes a parameter of type E and returns
   * some type U.
   * @return A new Result instance containing either the original value (if in
   * "ok" state) or the result of applying `f` to the contained error (if in
   * "err" state).
   */
  template <typename F>
  constexpr auto map_err(F &&f) const & ->
      typename std::enable_if<
          std::is_invocable<F, E>::value,
          Result<T, typename std::invoke_result<F, E>::type>>::type;

  /**
   * @brief Apply a function to the contained error if the result is in an "err"
   * state (move version).
   *
   * @tparam F The type of the function or functor to be applied to the error.
   * @param f A callable object that takes a parameter of type E and returns
   * some type U.
   * @return A new Result instance containing either the original value (if in
   * "ok" state) or the result of applying `f` to the contained error (if in
   * "err" state).
   */
  template <typename F>
  constexpr auto map_err(F &&f) && ->
      typename std::enable_if<
          std::is_invocable<F, E>::value,
          Result<T, typename std::invoke_result<F, E>::type>>::type;

  // --- (map_or)s ---

  /**
   * @brief Apply a function to the contained value if in "ok" state or return a
   * default value.
   *
   * @tparam F The type of the function or functor to be applied to the value.
   * @param d A default value to return if the Result is in an "err" state.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The result of applying `f` to the contained value (if in "ok"
   * state) or the provided default value `d`.
   */
  template <typename F>
  constexpr auto map_or(const typename std::invoke_result<F, T>::type &d, F &&f)
      const & -> typename std::enable_if<
                  std::is_invocable<F, T>::value,
                  typename std::invoke_result<F, T>::type>::type;

  /**
   * @brief Apply a function to the contained value if in "ok" state or return a
   * default value (move version).
   *
   * @tparam F The type of the function or functor to be applied to the value.
   * @param d A default value to return if the Result is in an "err" state.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The result of applying `f` to the contained value (if in "ok"
   * state) or the provided default value `d`.
   */
  template <typename F>
  constexpr auto map_or(typename std::invoke_result<F, T>::type &&d, F &&f)
      && -> typename std::enable_if<
             std::is_invocable<F, T>::value,
             typename std::invoke_result<F, T>::type>::type;

  /**
   * @brief Apply a function to the contained value if in "ok" state or apply
   * another callable to handle error.
   *
   * @tparam F The type of the function or functor to be applied to the value if
   * "ok".
   * @tparam D The type of the function or functor to be applied to the error if
   * "err".
   * @param d A callable object that takes a parameter of type E and returns
   * some type U.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The result of applying `f` to the contained value (if in "ok"
   * state) or the result of applying `d` to handle the error (if in "err"
   * state).
   */
  template <typename F, typename D>
  constexpr auto map_or_else(D &&d, F &&f) const & ->
      typename std::enable_if<
          std::is_invocable<F, T>::value && std::is_invocable<D, E>::value &&
              std::is_same<
                  typename std::invoke_result<F, T>::type,
                  typename std::invoke_result<D, E>::type>::value,
          typename std::invoke_result<F, T>::type>::type;

  /**
   * @brief Apply a function to the contained value if in "ok" state or apply
   * another callable to handle error (move version).
   *
   * @tparam F The type of the function or functor to be applied to the value if
   * "ok".
   * @tparam D The type of the function or functor to be applied to the error if
   * "err".
   * @param d A callable object that takes a parameter of type E and returns
   * some type U.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The result of applying `f` to the contained value (if in "ok"
   * state) or the result of applying `d` to handle the error (if in "err"
   * state).
   */
  template <typename F, typename D>
  constexpr auto map_or_else(D &&d, F &&f) && ->
      typename std::enable_if<
          std::is_invocable<F, T>::value && std::is_invocable<D, E>::value &&
              std::is_same<
                  typename std::invoke_result<F, T>::type,
                  typename std::invoke_result<D, E>::type>::value,
          typename std::invoke_result<F, T>::type>::type;

  // --- inspect ---

  /**
   * @brief Applies a function to the contained value if in "ok" state.
   *
   * @tparam F The type of the function or functor to apply.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The const reference to this Result instance.
   */
  template <typename F>
  constexpr auto inspect(F &&f) const & ->
      typename std::
          enable_if<std::is_invocable<F, T>::value, const Result<T, E> &>::type;

  /**
   * @brief Applies a function to the contained value if in "ok" state (move
   * version).
   *
   * @tparam F The type of the function or functor to apply.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The rvalue reference to this Result instance.
   */
  template <typename F>
  constexpr auto inspect(F &&f) && ->
      typename std::enable_if<std::is_invocable<F, T>::value, Result<T, E> &&>::
          type;

  /**
   * @brief Applies a function to the contained value if in "ok" state.
   *
   * @tparam F The type of the function or functor to apply.
   * @param f A callable object that takes a parameter of type T and returns
   * some type U.
   * @return The reference to this Result instance.
   */
  template <typename F>
  constexpr auto inspect(F &&f) & ->
      typename std::enable_if<std::is_invocable<F, T>::value, Result<T, E> &>::
          type;

  /**
   * @brief Applies a function to the error if in "err" state.
   *
   * @tparam F The type of the function or functor to apply.
   * @param f A callable object that takes a parameter of type E and returns
   * some type U.
   * @return The const reference to this Result instance.
   */
  template <typename F>
  constexpr auto inspect_err(F &&f) const & ->
      typename std::
          enable_if<std::is_invocable<F, E>::value, const Result<T, E> &>::type;

  /**
   * @brief Applies a function to the error if in "err" state (move version).
   *
   * @tparam F The type of the function or functor to apply.
   * @param f A callable object that takes a parameter of type E and returns
   * some type U.
   * @return The rvalue reference to this Result instance.
   */
  template <typename F>
  constexpr auto inspect_err(F &&f) && ->
      typename std::enable_if<std::is_invocable<F, E>::value, Result<T, E> &&>::
          type;

  /**
   * @brief Applies a function to the error if in "err" state.
   *
   * @tparam F The type of the function or functor to apply.
   * @param f A callable object that takes a parameter of type E and returns
   * some type U.
   * @return The reference to this Result instance.
   */
  template <typename F>
  constexpr auto inspect_err(F &&f) & ->
      typename std::enable_if<std::is_invocable<F, E>::value, Result<T, E> &>::
          type;

  // unwraps

  /**
   * @brief Unwraps the contained value if in "ok" state.
   * @return The contained value of type T.
   */
  constexpr T unwrap() const &;
  /**
   * @brief Unwraps the contained value if in "ok" state (move version).
   * @return The contained value of type T.
   */
  constexpr T unwrap() &&;
  /**
   * @brief Unwraps the contained value if in "err" state.
   * @return The contained value of type E.
   */
  constexpr E unwrap_err() const &;
  /**
   * @brief Unwraps the contained value if in "err" state (move version).
   * @return The contained value of type E.
   */
  constexpr E unwrap_err() &&;
  /**
   * @brief Unwraps the contained value if in "ok" state, otherwise returns a
   * default value.
   * @return The contained value of type T.
   */
  constexpr T unwrap_or(const T &other) const &;
  /**
   * @brief Unwraps the contained value if in "ok" state, otherwise returns a
   * default value (move version).
   * @return The contained value of type T.
   */
  constexpr T unwrap_or(T &&other) &&;

  /**
   * @brief Returns the contained value if in "ok" state or applies a function
   * to the error.
   *
   * @param f A callable object that takes the error of type E as input and
   * returns a value of type T.
   * @return The contained value of type T or the result of applying `f` to the
   * error if in an "err" state.
   */
  template <typename F>
  constexpr auto unwrap_or_else(F &&f) const & ->
      typename std::enable_if<
          std::is_invocable<F, E>::value &&
              std::is_same<typename std::invoke_result<F, E>::type, T>::value,
          T>;

  /**
   * @brief Returns the contained value if in "ok" state or applies a function
   * to the error (move version).
   *
   * @param f A callable object that takes the error of type E as input and
   * returns a value of type T.
   * @return The contained value of type T or the result of applying `f` to the
   * error if in an "err" state.
   */
  template <typename F>
  constexpr auto unwrap_or_else(F &&f) && ->
      typename std::enable_if<
          std::is_invocable<F, E>::value &&
              std::is_same<typename std::invoke_result<F, E>::type, T>::value,
          T>;

  template <typename U>
  constexpr Result<U, E> also(const Result<U, E> &other) const &;

  template <typename U>
  constexpr Result<U, E> also(Result<U, E> &&other) &&;

  template <typename U>
  constexpr Result<T, U> otherwise(const Result<T, U> &other) const &;

  template <typename U>
  constexpr Result<T, U> otherwise(Result<T, U> &&other) &&;

  template <typename F>
  constexpr auto then(F &&f)
      const & -> std::enable_if<
                  std::is_invocable<F, T>::value,
                  Result<typename std::invoke_result<F, T>::type, E>>;

  template <typename F>
  constexpr auto then(F &&f)
      && -> std::enable_if<
             std::is_invocable<F, T>::value,
             Result<typename std::invoke_result<F, T>::type, E>>;

  template <typename F>
  constexpr auto alternatively(F &&f)
      const & -> std::enable_if<
                  std::is_invocable<F, E>::value,
                  Result<T, typename std::invoke_result<F, E>::type>>;

  template <typename F>
  constexpr auto alternatively(F &&f)
      && -> std::enable_if<
             std::is_invocable<F, E>::value,
             Result<T, typename std::invoke_result<F, E>::type>>;

  // operators

  template <typename T1, typename E1>
  constexpr bool operator==(const Result<T1, E1> &rhs) const {
    if (std::is_same_v<T, T1> && (is_ok() && rhs.is_ok())) {
      return ref_ok() == rhs.ref_ok();
    }
    if (std::is_same_v<E, E1> && (is_err() && rhs.is_err())) {
      return ref_err() == rhs.ref_err();
    }
    return false;
  }

  template <typename T1, typename E1>
  constexpr bool operator!=(const Result<T1, E1> &rhs) const {
    return !(*this == rhs);
  }

 private:
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
EMBERS_INLINE Result<T, E>::~Result() {
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
  EMBERS_ASSERT(
      state_ == State::kOk,
      "Calling move_ok on result with non ok state"
  );
  T arg  = std::move(container_.ok_);
  state_ = State::kNotInit;
  return arg;  // copy elision, no move
}

template <typename T, typename E>
constexpr E Result<T, E>::move_err() && {
  EMBERS_ASSERT(
      state_ == State::kErr,
      "Calling move_err on result with non err state"
  );
  E arg  = std::move(container_.err_);
  state_ = State::kNotInit;
  return arg;  // copy elision, no move
}

template <typename T, typename E>
constexpr const T &Result<T, E>::ref_ok() const & {
  EMBERS_ASSERT(
      state_ == State::kOk,
      "Calling ref_ok on result with non ok state"
  );
  return container_.ok_;  // copy elision, no move
}

template <typename T, typename E>
constexpr const E &Result<T, E>::ref_err() const & {
  EMBERS_ASSERT(
      state_ == State::kErr,
      "Calling ref_err on result with non err state"
  );
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
  return is_ok() && std::invoke(std::forward<F>(f), ref_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_ok_and(F &&f) && ->
    typename std::enable_if<
        std::is_same<typename std::invoke_result<F, T>::type, bool>::value,
        bool>::type {
  return is_ok() && std::invoke(std::forward<F>(f), std::move(*this).move_ok());
};

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_err_and(F &&f) const & ->
    typename std::enable_if<
        std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
        bool>::type {
  return is_err() && std::invoke(std::forward<F>(f), ref_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::is_err_and(F &&f) && ->
    typename std::enable_if<
        std::is_same<typename std::invoke_result<F, E>::type, bool>::value,
        bool>::type {
  return is_err() &&
         std::invoke(std::forward<F>(f), std::move(*this).move_err());
};

// template <typename T, typename E>
// constexpr auto Result<T, E>::ok() const & {
//   return is_ok() ? Option<T>(ref_ok()) : Option<T>();
// }

// template <typename T, typename E>
// constexpr auto Result<T, E>::ok() && {
//   return is_ok() ? Option<T>(std::move(*this).move_ok()) : Option<T>();
// }

// template <typename T, typename E>
// constexpr auto Result<T, E>::err() const & {
//   return is_err() ? Option<E>(ref_err()) : Option<E>();
// }

// template <typename T, typename E>
// constexpr auto Result<T, E>::err() && {
//   return is_err() ? Option<E>(std::move(*this).move_err()) : Option<E>();
// }

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map(F &&f) const & ->
    typename std::enable_if<
        std::is_invocable<F, T>::value,
        Result<typename std::invoke_result<F, T>::type, E>>::type {
  using ReturnType = Result<typename std::invoke_result<F, T>::type, E>;
  return is_ok()  //
             ? ReturnType::create_ok(std::invoke(std::forward<F>(f), ref_ok()))
             : ReturnType::create_err(ref_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map(F &&f) && ->
    typename std::enable_if<
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
constexpr auto Result<T, E>::map_err(F &&f) const & ->
    typename std::enable_if<
        std::is_invocable<F, E>::value,
        Result<T, typename std::invoke_result<F, E>::type>>::type {
  using ReturnType = Result<T, typename std::invoke_result<F, E>::type>;
  return is_err()  //
             ? ReturnType::create_err(std::invoke(std::forward<F>(f), ref_err())
               )
             : ReturnType::create_ok(ref_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::map_err(F &&f) && ->
    typename std::enable_if<
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
             ? std::invoke(std::forward<F>(f), ref_ok())
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
             ? std::invoke(std::forward<F>(f), ref_ok())
             : std::invoke(std::forward<D>(d), ref_err());
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
constexpr auto Result<T, E>::inspect(F &&f) const & ->
    typename std::
        enable_if<std::is_invocable<F, T>::value, const Result<T, E> &>::type {
  if (is_ok()) {
    std::invoke(std::forward<F>(f), ref_ok());
  }
  return *this;
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect(F &&f) && ->
    typename std::enable_if<std::is_invocable<F, T>::value, Result<T, E> &&>::
        type {
  if (is_ok()) {
    std::invoke(std::forward<F>(f), ref_ok());
  }
  return std::move(*this);
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect(F &&f) & ->
    typename std::enable_if<std::is_invocable<F, T>::value, Result<T, E> &>::
        type {
  if (is_ok()) {
    std::invoke(std::forward<F>(f), ref_ok());
  }
  return *this;
}

// dfkgopsdkplgk

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect_err(F &&f) const & ->
    typename std::
        enable_if<std::is_invocable<F, E>::value, const Result<T, E> &>::type {
  if (is_err()) {
    std::invoke(std::forward<F>(f), ref_err());
  }
  return *this;
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect_err(F &&f) && ->
    typename std::enable_if<std::is_invocable<F, E>::value, Result<T, E> &&>::
        type {
  if (is_err()) {
    std::invoke(std::forward<F>(f), ref_err());
  }
  return std::move(*this);
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::inspect_err(F &&f) & ->
    typename std::enable_if<std::is_invocable<F, E>::value, Result<T, E> &>::
        type {
  if (is_err()) {
    std::invoke(std::forward<F>(f), ref_err());
  }
  return *this;
}

template <typename T, typename E>
constexpr T Result<T, E>::unwrap() const & {
  return ref_ok();
}
template <typename T, typename E>
constexpr T Result<T, E>::unwrap() && {
  return std::move(*this).move_ok();
}
template <typename T, typename E>
constexpr E Result<T, E>::unwrap_err() const & {
  return ref_err();
}
template <typename T, typename E>
constexpr E Result<T, E>::unwrap_err() && {
  return std::move(*this).move_err();
}
template <typename T, typename E>
constexpr T Result<T, E>::unwrap_or(const T &other) const & {
  return is_ok() ? ref_ok() : other;
}
template <typename T, typename E>
constexpr T Result<T, E>::unwrap_or(T &&other) && {
  return is_ok() ? std::move(*this).move_ok() : std::move(other);
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::unwrap_or_else(F &&f) const & ->
    typename std::enable_if<
        std::is_invocable<F, E>::value &&
            std::is_same<typename std::invoke_result<F, E>::type, T>::value,
        T> {
  return is_ok() ? ref_ok()
                 : std::invoke(std::forward<F>(f), std::move(*this).copy_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::unwrap_or_else(F &&f) && ->
    typename std::enable_if<
        std::is_invocable<F, E>::value &&
            std::is_same<typename std::invoke_result<F, E>::type, T>::value,
        T> {
  return is_ok() ? move_ok()
                 : std::invoke(std::forward<F>(f), std::move(*this).move_err());
}

template <typename T, typename E>
template <typename U>
constexpr Result<U, E> Result<T, E>::also(const Result<U, E> &other) const & {
  return is_ok() ? other : Result<U, E>::create_err(ref_err());
}

template <typename T, typename E>
template <typename U>
constexpr Result<U, E> Result<T, E>::also(Result<U, E> &&other) && {
  return is_ok() ? std::move(other)
                 : Result<U, E>::create_err(std::move(*this).move_err());
}

template <typename T, typename E>
template <typename U>
constexpr Result<T, U> Result<T, E>::otherwise(const Result<T, U> &other
) const & {
  return is_err() ? other : Result<T, U>::create_ok(ref_ok());
}

template <typename T, typename E>
template <typename U>
constexpr Result<T, U> Result<T, E>::otherwise(Result<T, U> &&other) && {
  return is_err() ? std::move(other)
                  : Result<T, U>::create_ok(std::move(*this).move_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::then(F &&f)
    const & -> std::enable_if<
                std::is_invocable<F, T>::value,
                Result<typename std::invoke_result<F, T>::type, E>> {
  using ReturnType = Result<typename std::invoke_result<F, T>::type, E>;
  return is_ok()
             ? ReturnType::create_ok(std::invoke(std::forward<F>(f), ref_ok()))
             : ReturnType::create_err(ref_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::then(F &&f)
    && -> std::enable_if<
           std::is_invocable<F, T>::value,
           Result<typename std::invoke_result<F, T>::type, E>> {
  using ReturnType = Result<typename std::invoke_result<F, T>::type, E>;
  return is_ok()
             ? ReturnType::create_ok(
                   std::invoke(std::forward<F>(f), std::move(*this).move_ok())
               )
             : ReturnType::create_err(std::move(*this).move_err());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::alternatively(F &&f)
    const & -> std::enable_if<
                std::is_invocable<F, E>::value,
                Result<T, typename std::invoke_result<F, E>::type>> {
  using ReturnType = Result<T, typename std::invoke_result<F, E>::type>;
  return is_err()
             ? ReturnType::create_err(std::invoke(std::forward<F>(f), ref_err())
               )
             : ReturnType::create_ok(ref_ok());
}

template <typename T, typename E>
template <typename F>
constexpr auto Result<T, E>::alternatively(F &&f)
    && -> std::enable_if<
           std::is_invocable<F, E>::value,
           Result<T, typename std::invoke_result<F, E>::type>> {
  using ReturnType = Result<T, typename std::invoke_result<F, E>::type>;
  return is_err()
             ? ReturnType::create_err(
                   std::invoke(std::forward<F>(f), std::move(*this).move_err())
               )
             : ReturnType::create_ok(std::move(*this).move_ok());
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
      return format_to(ctx.out(), "Ok({:?})", result.unwrap());
    }
    if (result.is_err()) {
      return format_to(ctx.out(), "Err({:?})", result.unwrap_err());
    }
    return format_to(ctx.out(), "NotInit");
  }
};
