/*******************************************************************************
 *
 * \file
 * \brief Implementation of an abstract domain keeping track of exceptions
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2018 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#pragma once

#include <sstream>

#include <ikos/core/domain/exception/abstract_domain.hpp>

namespace ikos {
namespace core {
namespace exception {

/// \brief Exception abstract domain
///
/// This is basically implemented as a triple (normal, propagated, caught) of
/// abstract domains:
///   * **normal** represents the normal execution flow state;
///   * **caught_exceptions** represents the state of uncaught exceptions;
///   * **propagated_exceptions** represents the state of caught exceptions
///     that are propagated through the control flow graph.
template < typename UnderlyingDomain >
class ExceptionDomain final
    : public exception::AbstractDomain< UnderlyingDomain,
                                        ExceptionDomain< UnderlyingDomain > > {
private:
  /// \brief Represents the normal execution flow state
  UnderlyingDomain _normal;

  /// \brief Represents the state of uncaught exceptions
  UnderlyingDomain _caught_exceptions;

  /// \brief Represents the state of caught exceptions that are propagated
  /// through the control flow graph
  UnderlyingDomain _propagated_exceptions;

private:
  struct TopTag {};
  struct TopNoExceptionsTag {};
  struct BottomTag {};

  /// \brief Create the top abstract value
  explicit ExceptionDomain(TopTag)
      : _normal(UnderlyingDomain::top()),
        _caught_exceptions(UnderlyingDomain::top()),
        _propagated_exceptions(UnderlyingDomain::top()) {}

  /// \brief Create the top abstract value with no pending exceptions
  explicit ExceptionDomain(TopNoExceptionsTag)
      : _normal(UnderlyingDomain::top()),
        _caught_exceptions(UnderlyingDomain::bottom()),
        _propagated_exceptions(UnderlyingDomain::bottom()) {}

  /// \brief Create the bottom abstract value
  explicit ExceptionDomain(BottomTag)
      : _normal(UnderlyingDomain::bottom()),
        _caught_exceptions(UnderlyingDomain::bottom()),
        _propagated_exceptions(UnderlyingDomain::bottom()) {}

public:
  /// \brief Create the top abstract value
  ExceptionDomain() : ExceptionDomain(TopTag{}) {}

  /// \brief Create an abstract value with the given underlying domains
  ///
  /// \param normal Represents the normal execution flow state
  /// \param caught_exceptions Represents the state of uncaught exceptions
  /// \param propagated_exceptions Represents the state of caught exceptions
  /// that are propagated through the control flow graph.
  ExceptionDomain(UnderlyingDomain normal,
                  UnderlyingDomain caught_exceptions,
                  UnderlyingDomain propagated_exceptions)
      : _normal(std::move(normal)),
        _caught_exceptions(std::move(caught_exceptions)),
        _propagated_exceptions(std::move(propagated_exceptions)) {}

  /// \brief Copy constructor
  ExceptionDomain(const ExceptionDomain&) = default;

  /// \brief Move constructor
  ExceptionDomain(ExceptionDomain&&) = default;

  /// \brief Copy assignment operator
  ExceptionDomain& operator=(const ExceptionDomain&) = default;

  /// \brief Move assignment operator
  ExceptionDomain& operator=(ExceptionDomain&&) = default;

  /// \brief Destructor
  ~ExceptionDomain() override = default;

  /// \brief Create the top abstract value
  static ExceptionDomain top() { return ExceptionDomain(TopTag{}); }

  /// \brief Create the top abstract value with no pending exceptions
  static ExceptionDomain top_no_exceptions() {
    return ExceptionDomain(TopNoExceptionsTag{});
  }

  /// \brief Create the bottom abstract value
  static ExceptionDomain bottom() { return ExceptionDomain(BottomTag{}); }

  bool is_bottom() const override {
    return this->_normal.is_bottom() && this->_caught_exceptions.is_bottom() &&
           this->_propagated_exceptions.is_bottom();
  }

  bool is_top() const override {
    return this->_normal.is_top() && this->_caught_exceptions.is_top() &&
           this->_propagated_exceptions.is_top();
  }

  void set_to_bottom() override {
    this->_normal.set_to_bottom();
    this->_caught_exceptions.set_to_bottom();
    this->_propagated_exceptions.set_to_bottom();
  }

  void set_to_top() override {
    this->_normal.set_to_top();
    this->_caught_exceptions.set_to_top();
    this->_propagated_exceptions.set_to_top();
  }

  bool leq(const ExceptionDomain& other) const override {
    return this->_normal.leq(other._normal) &&
           this->_caught_exceptions.leq(other._caught_exceptions) &&
           this->_propagated_exceptions.leq(other._propagated_exceptions);
  }

  bool equals(const ExceptionDomain& other) const override {
    return this->_normal.equals(other._normal) &&
           this->_caught_exceptions.equals(other._caught_exceptions) &&
           this->_propagated_exceptions.equals(other._propagated_exceptions);
  }

  void join_with(const ExceptionDomain& other) override {
    this->_normal.join_with(other._normal);
    this->_caught_exceptions.join_with(other._caught_exceptions);
    this->_propagated_exceptions.join_with(other._propagated_exceptions);
  }

  void join_loop_with(const ExceptionDomain& other) override {
    this->_normal.join_loop_with(other._normal);
    this->_caught_exceptions.join_loop_with(other._caught_exceptions);
    this->_propagated_exceptions.join_loop_with(other._propagated_exceptions);
  }

  void join_iter_with(const ExceptionDomain& other) override {
    this->_normal.join_iter_with(other._normal);
    this->_caught_exceptions.join_iter_with(other._caught_exceptions);
    this->_propagated_exceptions.join_iter_with(other._propagated_exceptions);
  }

  void widen_with(const ExceptionDomain& other) override {
    this->_normal.widen_with(other._normal);
    this->_caught_exceptions.widen_with(other._caught_exceptions);
    this->_propagated_exceptions.widen_with(other._propagated_exceptions);
  }

  /// \brief Perform the widening of two abstract values with a threshold
  template < typename Threshold >
  void widen_threshold_with(const ExceptionDomain& other,
                            const Threshold& threshold) {
    this->_normal.widen_threshold_with(other._normal, threshold);
    this->_caught_exceptions.widen_threshold_with(other._caught_exceptions,
                                                  threshold);
    this->_propagated_exceptions
        .widen_threshold_with(other._propagated_exceptions, threshold);
  }

  void meet_with(const ExceptionDomain& other) override {
    this->_normal.meet_with(other._normal);
    this->_caught_exceptions.meet_with(other._caught_exceptions);
    this->_propagated_exceptions.meet_with(other._propagated_exceptions);
  }

  void narrow_with(const ExceptionDomain& other) override {
    this->_normal.narrow_with(other._normal);
    this->_caught_exceptions.narrow_with(other._caught_exceptions);
    this->_propagated_exceptions.narrow_with(other._propagated_exceptions);
  }

  /*
   * Implement exception::AbstractDomain
   */

  UnderlyingDomain& normal() override { return this->_normal; }

  const UnderlyingDomain& normal() const override { return this->_normal; }

  UnderlyingDomain& caught_exceptions() override {
    return this->_caught_exceptions;
  }

  const UnderlyingDomain& caught_exceptions() const override {
    return this->_caught_exceptions;
  }

  UnderlyingDomain propagated_exceptions() override {
    return this->_propagated_exceptions;
  }

  const UnderlyingDomain& propagated_exceptions() const override {
    return this->_propagated_exceptions;
  }

  bool is_normal_flow_bottom() const override {
    return this->_normal.is_bottom();
  }

  bool is_normal_flow_top() const override { return this->_normal.is_top(); }

  void set_normal_flow_to_bottom() override { this->_normal.set_to_bottom(); }

  void set_normal_flow_to_top() override { this->_normal.set_to_top(); }

  bool is_caught_exceptions_bottom() const override {
    return this->_caught_exceptions.is_bottom();
  }

  bool is_caught_exceptions_top() const override {
    return this->_caught_exceptions.is_top();
  }

  void set_caught_exceptions_to_bottom() override {
    this->_caught_exceptions.set_to_bottom();
  }

  void set_caught_exceptions_to_top() override {
    this->_caught_exceptions.set_to_top();
  }

  bool is_propagated_exceptions_bottom() const override {
    return this->_propagated_exceptions.is_bottom();
  }

  bool is_propagated_exceptions_top() const override {
    return this->_propagated_exceptions.is_top();
  }

  void set_propagated_exceptions_to_bottom() override {
    this->_propagated_exceptions.set_to_bottom();
  }

  void set_propagated_exceptions_to_top() override {
    this->_propagated_exceptions.set_to_top();
  }

  void merge_propagated_in_caught_exceptions() override {
    this->_caught_exceptions.join_with(this->_propagated_exceptions);
    this->_propagated_exceptions.set_to_bottom();
  }

  void merge_caught_in_propagated_exceptions() override {
    this->_propagated_exceptions.join_with(this->_caught_exceptions);
    this->_caught_exceptions.set_to_bottom();
  }

  void enter_normal() override { this->_caught_exceptions.set_to_bottom(); }

  void enter_catch() override {
    std::swap(this->_normal, this->_caught_exceptions);
    this->_caught_exceptions.set_to_bottom();
    this->_propagated_exceptions.set_to_bottom();
  }

  void ignore_exceptions() override {
    this->_caught_exceptions.set_to_bottom();
    this->_propagated_exceptions.set_to_bottom();
  }

  void throw_exception() override {
    this->_caught_exceptions.join_with(this->_normal);
    this->_normal.set_to_bottom();
  }

  void resume_exception() override {
    this->_caught_exceptions.join_with(this->_normal);
    this->_normal.set_to_bottom();
  }

  void dump(std::ostream& o) const override {
    o << "(normal=";
    this->_normal.dump(o);
    o << ", caught_exceptions=";
    this->_caught_exceptions.dump(o);
    o << ", propagated_exceptions=";
    this->_propagated_exceptions.dump(o);
    o << ")";
  }

  static std::string name() {
    return "exception domain of " + UnderlyingDomain::name();
  }

}; // end class ExceptionDomain

} // end namespace exception
} // end namespace core
} // end namespace ikos
