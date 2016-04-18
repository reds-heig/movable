/**
 * @file MOVABLE Line Search functions
 *
 * This is a customized version of the SQBlib library made by Carlos Becker
 * http://sites.google.com/site/carlosbecker
 *
 * MOVABLE project: MicrOscopic VisuAlization of BLood cElls for the Detection
 *                  of Malaria and CD4+
 *
 * Alberto Dassatti, Magali Fröhlich, Roberto Rigamonti
 * HES-SO
 * REDS institute, HEIG-VD (Yverdon-les-Bains)
 */

#ifndef LINESEARCH_HPP_
#define LINESEARCH_HPP_

#include <iostream>
#include <cassert>

#include <lbfgs.h>

#include "DataTypes.hpp"
#include "logging.hpp"

/**
 * class LineSearch - Line search procedures for regressed trees
 *
 * @labels    : true labels of the samples
 * @prevScores: cumulated scores for the samples -- that is, responses of the
 *              previous weak learners already weighted
 * @newScores : scores obtained by the newly-computed regression tree
 * @lossScale : scaling factor for the loss
 */
class LineSearch {
private:
	EVecD labels;
	EVecD prevScores;
	EVecD newScores;
	double lossScale;

public:
	/**
	 * LineSearch() - Initialize the class by referencing to original
	 *                vectors
	 *
	 * @labels    : true labels of the samples
	 * @prevScores: previous scores
	 * @newScores : newly-obtained scores
	 */
	LineSearch(const EVec &labels,
		   const EVec &prevScores,
		   const EVec &newScores) :
		lossScale(0.0)
	{
		this->labels = labels.cast< double >();
		this->prevScores = prevScores.cast< double >();
		this->newScores = newScores.cast< double >();
	}

	/**
	 * evaluate() - Evaluate both the function and the gradient at the
	 *              current point
	 *
	 * @instance: user data
	 * @x       : current value of optimized variable
	 * @g       : gradient vector
	 * @n       : dimension of the space (unused since set to 1)
	 * @step    : current step of line search (unused)
	 *
	 * Return: the value of the objective function as return value and the
	 *         gradient in the passed parameter
	 */
	static lbfgsfloatval_t
	evaluate(void *instance,
		 const lbfgsfloatval_t *x,
		 lbfgsfloatval_t *g,
		 const int /* n */,
		 const lbfgsfloatval_t /* step */)
	{
		LineSearch *LS = (LineSearch *)instance;

		lbfgsfloatval_t fx = 0;
		lbfgsfloatval_t alpha = x[0];

		/* Objective function's value */
		fx = (-LS->labels.array() *
		      (LS->prevScores + alpha * LS->newScores).array()).exp().sum();

		if (LS->lossScale == 0.0) {
			/* First evaluation -- give an initial value to the
			   scaling factor */
			LS->lossScale = 1.0 / (fx + std::numeric_limits< double >::epsilon());
			fx = 1.0;
			log_trace("Scaled lossScale for the 1st time to %f", LS->lossScale);
		} else {
			fx *= LS->lossScale;
		}

		/* Gradient's value */
		g[0] = LS->lossScale *
			(-LS->labels.array() * LS->newScores.array() *
			 (-LS->labels.array() * (LS->prevScores + alpha *
						 LS->newScores).array()).exp())
			.sum();

		return fx;
	}

	/**
	 * getErrDescr() - Get a textual description of an error ID
	 *
	 * @errID: error identified returned by the library
	 *
	 * Return: Textual description of the error
	 */
        static const char *
	getErrDescr(int errID)
        {
            switch(errID)
            {
	    case LBFGS_ALREADY_MINIMIZED:
		    return "The initial variables already minimize the objective function.";
	    case LBFGSERR_UNKNOWNERROR:
		    return "Unknown error";
	    case LBFGSERR_LOGICERROR:
		    return "Logic error.";
	    case LBFGSERR_OUTOFMEMORY:
		    return "Insufficient memory.";
	    case LBFGSERR_CANCELED:
		    return "The minimization process has been canceled.";
	    case LBFGSERR_INVALID_N:
		    return "Invalid number of variables specified.";
	    case LBFGSERR_INVALID_N_SSE:
		    return "Invalid number of variables (for SSE) specified.";
	    case LBFGSERR_INVALID_X_SSE:
		    return "The array x must be aligned to 16 (for SSE).";
	    case LBFGSERR_INVALID_EPSILON:
		    return "Invalid parameter lbfgs_parameter_t::epsilon specified.";
	    case LBFGSERR_INVALID_TESTPERIOD:
		    return "Invalid parameter lbfgs_parameter_t::past specified.";
	    case LBFGSERR_INVALID_DELTA:
		    return "Invalid parameter lbfgs_parameter_t::delta specified.";
	    case LBFGSERR_INVALID_LINESEARCH:
		    return "Invalid parameter lbfgs_parameter_t::linesearch specified.";
	    case LBFGSERR_INVALID_MINSTEP:
		    return "Invalid parameter lbfgs_parameter_t::max_step specified.";
	    case LBFGSERR_INVALID_MAXSTEP:
		    return "Invalid parameter lbfgs_parameter_t::max_step specified.";
	    case LBFGSERR_INVALID_FTOL:
		    return "Invalid parameter lbfgs_parameter_t::ftol specified.";
	    case LBFGSERR_INVALID_WOLFE:
		    return "Invalid parameter lbfgs_parameter_t::wolfe specified.";
	    case LBFGSERR_INVALID_GTOL:
		    return "Invalid parameter lbfgs_parameter_t::gtol specified.";
	    case LBFGSERR_INVALID_XTOL:
		    return "Invalid parameter lbfgs_parameter_t::xtol specified.";
	    case LBFGSERR_INVALID_MAXLINESEARCH:
		    return "Invalid parameter lbfgs_parameter_t::max_linesearch specified.";
	    case LBFGSERR_INVALID_ORTHANTWISE:
		    return "Invalid parameter lbfgs_parameter_t::orthantwise_c specified.";
	    case LBFGSERR_INVALID_ORTHANTWISE_START:
		    return "Invalid parameter lbfgs_parameter_t::orthantwise_start specified.";
	    case LBFGSERR_INVALID_ORTHANTWISE_END:
		    return "Invalid parameter lbfgs_parameter_t::orthantwise_end specified.";
	    case LBFGSERR_OUTOFINTERVAL:
		    return "The line-search step went out of the interval of uncertainty.";
	    case LBFGSERR_INCORRECT_TMINMAX:
		    return "A logic error occurred; alternatively, the interval of uncertainty became too small.";
	    case LBFGSERR_ROUNDING_ERROR:
		    return "A rounding error occurred; alternatively, no line-search step satisfies the sufficient decrease and curvature conditions.";
	    case LBFGSERR_MINIMUMSTEP:
		    return "The line-search step became smaller than lbfgs_parameter_t::min_step.";
	    case LBFGSERR_MAXIMUMSTEP:
		    return "The line-search step became larger than lbfgs_parameter_t::max_step.";
	    case LBFGSERR_MAXIMUMLINESEARCH:
		    return "The line-search routine reaches the maximum number of evaluations.";
	    case LBFGSERR_MAXIMUMITERATION:
		    return "The algorithm routine reaches the maximum number of iterations.";
	    case LBFGSERR_WIDTHTOOSMALL:
		    return "Relative width of the interval of uncertainty is at most lbfgs_parameter_t::xtol.";
	    case LBFGSERR_INVALIDPARAMETERS:
		    return "A logic error (negative line-search step) occurred.";
	    case LBFGSERR_INCREASEGRADIENT:
		    return "The current search direction increases the objective function value.";
	    default:
		    return "UNKNOWN";
            }

	    /* Should never get here... */
            return "UNKNOWN";
        }

	/**
	 * progress() - Display the state of the current iteration of the L-BFGS
	 *              algorithm
	 *
	 * @instance: user data
	 * @x       : current value
	 * @g       : gradient value
	 * @fx      : value of the objective function
	 * @xnorm   : Euclidean norm of the variable
	 * @gnorm   : Euclidean norm of the gradient
	 * @step    : line-search step used for this iteration
	 * @n       : Dimensionality of the optimized variable
	 * @k       : Iteration count
	 * @ls      : Number of evaluations called for this iteration
	 *
	 * Return: 0 (a non-zero value would cancel the optimization process!)
	 */
	static int
	progress(void * /* instance */,
		 const lbfgsfloatval_t * /* x */,
		 const lbfgsfloatval_t * /* g */,
		 const lbfgsfloatval_t /* fx */,
		 const lbfgsfloatval_t /* xnorm */,
		 const lbfgsfloatval_t /* gnorm */,
		 const lbfgsfloatval_t /* step */,
		 int /* n */,
		 int /* k */,
		 int /* ls */)
	{
		// log_info("Iteration %d - step = %f:\n\tfx = %f\tx = %f\n"
		// 	  "\tx_norm = %f\tg_norm = %f",
		// 	  k, step, fx, x[0], xnorm, gnorm);
		return 0;
	}

	/**
	 * run() - Start the optimization to compute the alpha value
	 *
	 * Return: alpha value for the vectors set in the constructor
	 */
	double
	run()
	{
		lbfgsfloatval_t fx;
		lbfgsfloatval_t *x = lbfgs_malloc(1);
		x[0] = 0.0;
		double val;

		lbfgs_parameter_t param;
		lbfgs_parameter_init(&param);

		/* Run the optimization */
		int rc = lbfgs(1, x, &fx, evaluate, progress,
			       (void *)this, &param);
		if (rc != LBFGS_SUCCESS) {
			log_info("L-BFGS line search ended with rc = %d, "
				 "error description: %s", rc, getErrDescr(rc));
		}

		val = x[0];
		if (std::isnan(val) || std::isinf(val)) {
			log_info("L-BFGS optimization result is NaN/inf, "
				 "setting to 0");
			val = 0;
		}

		lbfgs_free(x);

		return val;
	}

};

#endif /* LINESEARCH_HPP_ */
