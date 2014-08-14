/**
 * CMA-ES, Covariance Matrix Adaptation Evolution Strategy
 * Copyright (c) 2014 Inria
 * Author: Emmanuel Benazera <emmanuel.benazera@lri.fr>
 *
 * This file is part of libcmaes.
 *
 * libcmaes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libcmaes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcmaes.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "eo_matrix.h"
#include "genopheno.h"
#include "llogging.h"
#include <string>
#include <ctime>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace libcmaes
{
  /**
   * \brief Generic class for Evolution Strategy parameters.
   */
  template <class TGenoPheno=GenoPheno<NoBoundStrategy> >
  class Parameters
  {
  public:
    /**
     * \brief empty constructor.
     */
  Parameters():_dim(0),_lambda(0),_max_iter(0)
      {}

    /**
     * \brief constructor
     * @param dim problem dimensions
     * @param x0 initial search point
     * @param lambda number of offsprings sampled at each step
     * @param seed initial random seed, useful for reproducing results (if unspecified, automatically generated from current time)
     * @param gp genotype / phenotype object
     */
  Parameters(const int &dim, const double *x0, const int &lambda=-1,
	     const uint64_t &seed=0, const TGenoPheno &gp=GenoPheno<NoBoundStrategy>())
  :_dim(dim),_lambda(lambda),_seed(seed),_gp(gp)
  {
    if (_lambda == -1) // lambda is unspecified
      _lambda = 4 + floor(3.0*log(_dim));
    if (_seed == 0) // seed is not forced.
      _seed = static_cast<uint64_t>(time(nullptr));
    set_x0(x0);
  }
  
  ~Parameters()
  {
  }

  /**
   * \brief sets initial objective function parameter values to x0 across all dimensions
   * @param x0 initial value
   */
  void set_x0(const double &x0)
  {
    _x0min = _x0max = dVec::Constant(_dim,x0);
  }

  /**
   * \brief sets initial objective function parameter values to array x0
   * @param x0 array of initial parameter values
   */
  void set_x0(const double *x0)
  {
    _x0min = _x0max = dVec(_dim);
    for (int i=0;i<_dim;i++)
      _x0min(i) = _x0max(i) = x0[i];
  }

  /**
   * \brief sets bounds on initial objective function parameter values.
   *        Bounds are the same across all dimensions, and initial value is
   *        sampled uniformly within these bounds.
   * @param x0min lower bound
   * @param x0max upper bound
   */
  void set_x0(const double &x0min, const double &x0max)
  {
    _x0min = dVec::Constant(_dim,x0min);
    _x0max = dVec::Constant(_dim,x0max);
  }

  void set_x0(const dVec &x0)
  {
    _x0min = x0;
    _x0max = x0;
  }
  
  /**
   * \brief sets bounds on initial objective function parameter values.
   *        Initial value is sampled uniformly within these bounds.
   * @param x0min vector of initial lower bounds.
   * @param x0max vector of initial upper bounds.
   */
  void set_x0(const double *x0min, const double *x0max)
  {
    _x0min = dVec(_dim);
    _x0max = dVec(_dim);
    for (int i=0;i<_dim;i++)
      {
	_x0min(i) = x0min[i];
	_x0max(i) = x0max[i];
      }
  }
  
  /**
   * \brief sets bounds on initial objective function parameter values.
   *        Initial value is sampled uniformly within these bounds.
   * @param x0min vector of initial lower bounds.
   * @param x0max vector of initial upper bounds.
   */
  void set_x0(const dVec &x0min, const dVec &x0max)
  {
    _x0min = x0min;
    _x0max = x0max;
  }

  /**
   * \brief returns lower bound on x0 vector
   * @return lower bound on x0
   */
  dVec get_x0min() const
  {
    return _x0min;
  }

  /**
   * \brief returns upper bound on x0 vector
   * @return upper bound on x0
   */
  dVec get_x0max() const
  {
    return _x0max;
  }
  
  /**
   * \brief freezes a parameter to a given value during optimization.
   * @param index dimension index of the parameter to be frozen
   * @param value frozen value of the parameter
   */
  void set_fixed_p(const int &index, const double &value)
  {
    _fixed_p.insert(std::pair<int,double>(index,value));
  }

  /**
   * \brief unfreezes a parameter.
   * @param index dimenion index of the parameter to unfreeze
   */
  void unset_fixed_p(const int &index)
  {
    std::unordered_map<int,double>::iterator mit;
    if ((mit=_fixed_p.find(index))!=_fixed_p.end())
      _fixed_p.erase(mit);
  }
  
  /**
   * \brief sets the maximum number of iterations allowed for the optimization.
   * @param maxiter maximum number of allowed iterations
   */
  void set_max_iter(const int &maxiter)
  {
    _max_iter = maxiter;
  }

  /**
   * \brief returns maximum number of iterations
   * @return max number of iterations allowed
   */
  int get_max_iter() const
  {
    return _max_iter;
  }
  
  /**
   * \brief sets the maximum budget of objective function calls allowed for the optimization.
   * @param fevals maximum number of objective function evaluations
   */
  void set_max_fevals(const int &fevals)
  {
    _max_fevals = fevals;
  }

  /**
   * \brief returns maximum budget of objective function calls
   * @return max number of objective function evaluations
   */
  int get_max_fevals() const
  {
    return _max_fevals;
  }
  
  /**
   * \brief sets the objective function target value when known.
   * @param val objective function target value
   */
  void set_ftarget(const double &val)
  {
    _ftarget = val;
  }

  /**
   * \brief resets the objective function target value to its inactive state.
   */
  void reset_ftarget()
  {
    _ftarget = -std::numeric_limits<double>::infinity();
  }

  /**
   * \brief returns objective function target value.
   * @return objective function target value
   */
  double get_ftarget() const
  {
    return _ftarget;
  }

  /**
   * \brief sets random generator's seed, 0 is special value to generate random seed.
   * @param seed integer seed
   */
  void set_seed(const int &seed)
  {
    if (_seed == 0)
      _seed = static_cast<uint64_t>(time(nullptr));
    else _seed = seed;
  }

  /**
   * \brief returns random generator's seed.
   * @return integer seed
   */
  int get_seed() const
  {
    return _seed;
  }
  
  /**
   * \brief sets function tolerance as stopping criteria for TolHistFun: monitors the
   *        difference in function value over iterations and stops optimization when 
   *        below tolerance.
   * @param v value of the function tolerance.	    
   */
  void set_ftolerance(const double &v) { _ftolerance = v; }

  /**
   * \brief returns function tolerance
   * @return function tolerance
   */
  double get_ftolerance() const
  {
    return _ftolerance;
  }
  
  /**
   * \brief sets parameter tolerance as stopping criteria for TolX.
   * @param v value of the parameter tolerance.
   */
  void set_xtolerance(const double &v) { _xtol = v; }

  /**
   * \brief returns parameter tolerance
   * @return parameter tolerance
   */
  double get_xtolerance() const
  {
    return _xtol;
  }
  
  /**
   * \brief returns lambda, number of offsprings per generation
   * @return lambda
   */
  int lambda() const
  {
    return _lambda;
  }

  /**
   * \brief returns the problem's dimension
   * @return dimensions
   */
  int dim() const
  {
    return _dim;
  }

  /**
   * \brief sets the quiet mode (no output from the library) for the optimization at hand
   * @param quiet true / false
   */
  void set_quiet(const bool &quiet)
  {
    _quiet = quiet;
  }
  
  /**
   * \brief returns whether the quiet mode is on.
   * @return quiet mode
   */
  bool quiet() const
  {
    return _quiet;
  }

  /**
   * \brief sets the optimization algorithm.
   * @param algo from CMAES_DEFAULT, IPOP_CMAES, BIPOP_CMAES, aCMAES, aIPOP_CMAES, aBIPOP_CMAES, sepCMAES, sepIPOP_CMAES, sepBIPOP_CMAES, sepaCMAES, sepaIPOP_CMAES, sepaBIPOP_CMAES 
   */
  void set_algo(const int &algo)
  {
    _algo = algo;
  }

  /**
   * \brief sets the optimization algorithm.
   * @param algo as string from cmaes,ipop,bipop,acmaes,aipop,abipop,sepcmaes,sepipop,sepbipop,sepacmaes,sepaipop,sepabipop
   */
  void set_str_algo(const std::string &algo)
  {
    std::map<std::string,int>::const_iterator mit;
    if ((mit = Parameters<TGenoPheno>::_algos.find(algo))!=Parameters<TGenoPheno>::_algos.end())
      _algo = (*mit).second;
    else LOG(ERROR) << "unknown algorithm " << algo << std::endl;
  }
  
  /**
   * \brief returns which algorithm is set for the optimization at hand.
   * @return algorithm integer code
   */
  int get_algo() const
  {
    return _algo;
  }

  /**
   * \brief sets the genotype/phenotype transform object.
   * @param gp GenoPheno object
   */
  void set_gp(const TGenoPheno &gp)
  {
    _gp = gp;
  }

  /**
   * \brief returns the current genotype/phenotype transform object.
   * @return GenoPheno object
   */
  TGenoPheno get_gp() const
  {
    return _gp;
  }

  /**
   * \brief sets the output filename (activates the output to file).
   * @param fplot filename
   */
  void set_fplot(const std::string &fplot)
  {
    _fplot = fplot;
  }

  /**
   * \brief returns the current output filename.
   * @return output filename
   */
  std::string get_fplot() const
  {
    return _fplot;
  }
  
  /**
   * \brief activates the gradient injection scheme. 
   *        If no gradient function is defined, injects a numerical gradient solution instead
   * @param gradient true/false
   */
  void set_gradient(const bool &gradient)
  {
    _with_gradient = gradient;
  }

  /**
   * \brief returns whether the gradient injection scheme is activated.
   * @return with gradient
   */
  bool get_gradient() const
  {
    return _with_gradient;
  }

  /**
   * \brief activates computation of expected distance to minimum when optimization has completed
   * @param edm true / false
   */
  void set_edm(const bool &edm)
  {
    _with_edm = edm;
  }

  /**
   * \brief returns whether edm is activated.
   * @return edm
   */
  bool get_edm() const
  {
    return _with_edm;
  }

  /**
   * \brief activate / deactivate the parallel evaluation of objective function
   * @param mt true for activated, false otherwise
   */
  void set_mt_feval(const bool &mt)
  {
    _mt_feval = mt;
  }

  /**
   * \brief resturns whether the parallel evaluation of objective function is activated
   * @return activation status
   */
  bool get_mt_feval() const
  {
    return _mt_feval;
  }

  /**
   * \brief sets maximum history size, allows to keep memory requirements fixed.
   * @param m number of steps of candidate history that are kept into memory (for stopping criteria equalfunvals mostly).
   */
  void set_max_hist(const int &m)
  {
    _max_hist = m;
  }
  
  int _dim; /**< function space dimensions. */
  int _lambda = -1; /**< number of offsprings. */
  int _max_iter = -1; /**< max iterations. */
  int _max_fevals = -1; /**< max budget as number of function evaluations. */
  
  bool _quiet = false; /**< quiet all outputs. */
  std::string _fplot = ""; /**< plotting file, if specified. */
  dVec _x0min; /**< initial mean vector min bound value for all components. */
  dVec _x0max; /**< initial mean vector max bound value for all components. */
  double _ftarget = -std::numeric_limits<double>::infinity(); /**< optional objective function target value. */
  double _ftolerance = 1e-12; /**< tolerance of the best function values during the last 10+(30*dim/lambda) steps (TolHistFun). */ 
  double _xtol = 1e-12; /**< tolerance on parameters error. */
  
  uint64_t _seed = 0; /**< seed for random generator. */
  int _algo = 0; /**< selected algorithm. */

  bool _with_gradient=false; /**< whether to use injected gradient. */
  bool _with_edm=false; /**< whether to compute expected distance to minimum when optimization has completed. */
  
  std::unordered_map<int,double> _fixed_p; /**< fixed parameters and values. */
  
  TGenoPheno _gp; /**< genotype / phenotype object. */

  bool _mt_feval = false; /**< whether to force multithreaded (i.e. parallel) function evaluations. */ 
  int _max_hist = 100; /**< max size of the history, keeps memory requirements fixed. */

  static std::map<std::string,int> _algos;// = { {"cmaes",0}, {"ipop",1}};
  };

  template<class TGenoPheno>
    std::map<std::string,int> Parameters<TGenoPheno>::_algos = {{"cmaes",0},{"ipop",1},{"bipop",2},{"acmaes",3},{"aipop",4},{"abipop",5},{"sepcmaes",6},{"sepipop",7},{"sepbipop",8},{"sepacmaes",9},{"sepipop",10},{"sepbipop",11}};
  
}

#endif
