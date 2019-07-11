#ifndef __FUSION_H__
#define __FUSION_H__
#include "monty.h"
#include "stdlib.h"
#include "cmath"
#include "mosektask.h"
#include "mosek.h"
#include "fusion_fwd.h"
namespace mosek
{
namespace fusion
{
enum class RelationKey
{
EqualsTo,
LessThan,
GreaterThan,
IsFree,
InRange
};
std::ostream & operator<<(std::ostream & os, RelationKey val);
enum class PSDKey
{
IsSymPSD,
IsTrilPSD
};
std::ostream & operator<<(std::ostream & os, PSDKey val);
enum class QConeKey
{
InQCone,
InRotatedQCone,
InPExpCone,
InPPowCone,
InDExpCone,
InDPowCone
};
std::ostream & operator<<(std::ostream & os, QConeKey val);
enum class ObjectiveSense
{
Undefined,
Minimize,
Maximize
};
std::ostream & operator<<(std::ostream & os, ObjectiveSense val);
enum class SolutionStatus
{
Undefined,
Unknown,
Optimal,
Feasible,
Certificate,
IllposedCert
};
std::ostream & operator<<(std::ostream & os, SolutionStatus val);
enum class AccSolutionStatus
{
Anything,
Optimal,
Feasible,
Certificate
};
std::ostream & operator<<(std::ostream & os, AccSolutionStatus val);
enum class ProblemStatus
{
Unknown,
PrimalAndDualFeasible,
PrimalFeasible,
DualFeasible,
PrimalInfeasible,
DualInfeasible,
PrimalAndDualInfeasible,
IllPosed,
PrimalInfeasibleOrUnbounded
};
std::ostream & operator<<(std::ostream & os, ProblemStatus val);
enum class SolutionType
{
Default,
Basic,
Interior,
Integer
};
std::ostream & operator<<(std::ostream & os, SolutionType val);
enum class StatusKey
{
Unknown,
Basic,
SuperBasic,
OnBound,
Infinity
};
std::ostream & operator<<(std::ostream & os, StatusKey val);
}
}
namespace mosek
{
namespace fusion
{
class /*interface*/ Expression : public virtual monty::RefCounted
{
public:
typedef monty::rc_ptr< Expression > t;

virtual void destroy() = 0;
virtual ~Expression() {};
virtual std::string toString() = 0;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,2 > > indexrows) = 0;
monty::rc_ptr< ::mosek::fusion::Expression > pick(std::shared_ptr< monty::ndarray< int,2 > > indexrows);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,1 > > indexes) = 0;
monty::rc_ptr< ::mosek::fusion::Expression > pick(std::shared_ptr< monty::ndarray< int,1 > > indexes);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(std::shared_ptr< monty::ndarray< int,1 > > indexes) = 0;
monty::rc_ptr< ::mosek::fusion::Expression > index(std::shared_ptr< monty::ndarray< int,1 > > indexes);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(int i) = 0;
monty::rc_ptr< ::mosek::fusion::Expression > index(int i);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta) = 0;
monty::rc_ptr< ::mosek::fusion::Expression > slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(int first,int last) = 0;
monty::rc_ptr< ::mosek::fusion::Expression > slice(int first,int last);
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) = 0;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() = 0;
monty::rc_ptr< ::mosek::fusion::FlatExpr > eval();
virtual int getND() = 0;
virtual int getDim(int d) = 0;
virtual long long getSize() = 0;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() = 0;
}; // interface Expression;

class /*interface*/ Variable : public virtual ::mosek::fusion::Expression
{
public:
typedef monty::rc_ptr< Variable > t;

virtual void destroy() = 0;
virtual ~Variable() {};
virtual std::string toString() = 0;
virtual int numInst() = 0;
virtual int inst(int spoffset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,int nioffset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) = 0;
virtual void inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) = 0;
virtual int getND() = 0;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() = 0;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel() = 0;
monty::rc_ptr< ::mosek::fusion::Model > getModel();
virtual long long getSize() = 0;
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > v) = 0;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1,int dim2) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(int dim0,int dim1,int dim2);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(int dim0,int dim1);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(int dim0);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(std::shared_ptr< monty::ndarray< int,1 > > shape);
virtual void set_values(std::shared_ptr< monty::ndarray< double,1 > > value,bool primal) = 0;
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() = 0;
virtual std::shared_ptr< monty::ndarray< double,1 > > level() = 0;
virtual void values(int offset,std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) = 0;
virtual void make_continuous() = 0;
virtual void make_integer() = 0;
virtual void makeContinuous() = 0;
virtual void makeInteger() = 0;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose() = 0;
monty::rc_ptr< ::mosek::fusion::Variable > transpose();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2,std::shared_ptr< monty::ndarray< int,1 > > i3) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2,std::shared_ptr< monty::ndarray< int,1 > > i3);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,2 > > midxs);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,2 > > indexrows);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,1 > > idxs);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,1 > > indexes);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag() = 0;
monty::rc_ptr< ::mosek::fusion::Variable > antidiag();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int index) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > antidiag(int index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag() = 0;
monty::rc_ptr< ::mosek::fusion::Variable > diag();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int index) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > diag(int index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > idx) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > index(std::shared_ptr< monty::ndarray< int,1 > > idx);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(std::shared_ptr< monty::ndarray< int,1 > > indexes);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i1,int i2,int i3) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > index(int i1,int i2,int i3);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i1,int i2) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > index(int i1,int i2);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i1) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > index(int i1);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(int i);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int first,int last) = 0;
monty::rc_ptr< ::mosek::fusion::Variable > slice(int first,int last);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(int first,int last);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr() = 0;
monty::rc_ptr< ::mosek::fusion::Expression > asExpr();
}; // interface Variable;

class /*interface*/ SymmetricVariable : public virtual ::mosek::fusion::Variable
{
public:
typedef monty::rc_ptr< SymmetricVariable > t;

virtual void destroy() = 0;
virtual ~SymmetricVariable() {};
}; // interface SymmetricVariable;

class FusionException : public ::monty::Exception
{
private:
std::string msg;
protected:
public:
typedef monty::rc_ptr< FusionException > t;

FusionException(const std::string &  msg_);
virtual /* override */ std::string toString() ;
}; // class FusionException;

class SolutionError : public ::mosek::fusion::FusionException
{
private:
protected:
public:
typedef monty::rc_ptr< SolutionError > t;

SolutionError();
SolutionError(const std::string &  msg);
}; // class SolutionError;

class UnimplementedError : public ::monty::RuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< UnimplementedError > t;

UnimplementedError(const std::string &  msg);
}; // class UnimplementedError;

class FatalError : public ::monty::RuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< FatalError > t;

FatalError(const std::string &  msg);
}; // class FatalError;

class UnexpectedError : public ::monty::RuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< UnexpectedError > t;

UnexpectedError(::mosek::fusion::FusionException e);
UnexpectedError(const std::string &  msg);
}; // class UnexpectedError;

class FusionRuntimeException : public ::monty::RuntimeException
{
private:
std::string msg;
protected:
public:
typedef monty::rc_ptr< FusionRuntimeException > t;

FusionRuntimeException(const std::string &  msg_);
virtual /* override */ std::string toString() ;
}; // class FusionRuntimeException;

class SparseFormatError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< SparseFormatError > t;

SparseFormatError(const std::string &  msg);
}; // class SparseFormatError;

class SliceError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< SliceError > t;

SliceError();
SliceError(const std::string &  msg);
}; // class SliceError;

class SetDefinitionError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< SetDefinitionError > t;

SetDefinitionError(const std::string &  msg);
}; // class SetDefinitionError;

class OptimizeError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< OptimizeError > t;

OptimizeError(const std::string &  msg);
}; // class OptimizeError;

class NameError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< NameError > t;

NameError(const std::string &  msg);
}; // class NameError;

class ModelError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< ModelError > t;

ModelError(const std::string &  msg);
}; // class ModelError;

class MatrixError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< MatrixError > t;

MatrixError(const std::string &  msg);
}; // class MatrixError;

class DimensionError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< DimensionError > t;

DimensionError(const std::string &  msg);
}; // class DimensionError;

class LengthError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< LengthError > t;

LengthError(const std::string &  msg);
}; // class LengthError;

class RangeError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< RangeError > t;

RangeError(const std::string &  msg);
}; // class RangeError;

class IndexError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< IndexError > t;

IndexError(const std::string &  msg);
}; // class IndexError;

class DomainError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< DomainError > t;

DomainError(const std::string &  msg);
}; // class DomainError;

class ValueConversionError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< ValueConversionError > t;

ValueConversionError(const std::string &  msg);
}; // class ValueConversionError;

class ParameterError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< ParameterError > t;

ParameterError(const std::string &  msg);
}; // class ParameterError;

class ExpressionError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< ExpressionError > t;

ExpressionError(const std::string &  msg);
}; // class ExpressionError;

class IOError : public ::mosek::fusion::FusionRuntimeException
{
private:
protected:
public:
typedef monty::rc_ptr< IOError > t;

IOError(const std::string &  msg);
}; // class IOError;

// mosek.fusion.BaseModel from file 'src/fusion/cxx/BaseModel.h'

class BaseModel : public monty::RefCounted
{
private:
  p_BaseModel * _impl;
protected:
  BaseModel(p_BaseModel * ptr);
public:
  friend class p_BaseModel;

  virtual void dispose();

  ~BaseModel();
};
// End of file 'src/fusion/cxx/BaseModel.h'
class Model : public ::mosek::fusion::BaseModel
{
Model(monty::rc_ptr< ::mosek::fusion::Model > m);
protected: Model(p_Model * _impl);
public:
Model(const Model &) = delete;
const Model & operator=(const Model &) = delete;
friend class p_Model;
virtual ~Model();
virtual void destroy();
typedef monty::rc_ptr< Model > t;

Model(const std::string &  name);
Model();
static void putlicensewait(bool wait);
static void putlicensepath(const std::string &  licfile);
static void putlicensecode(std::shared_ptr< monty::ndarray< int,1 > > code);
virtual /* override */ void dispose() ;
virtual MSKtask_t __mosek_2fusion_2Model__getTask() ;
MSKtask_t getTask();
virtual void getConstraintValues(bool primal,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs,std::shared_ptr< monty::ndarray< double,1 > > res,int offset) ;
virtual void getVariableDuals(bool lower,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs,std::shared_ptr< monty::ndarray< double,1 > > res,int offset) ;
virtual void getVariableValues(bool primal,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs,std::shared_ptr< monty::ndarray< double,1 > > res,int offset) ;
virtual void setVariableValues(bool primal,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs,std::shared_ptr< monty::ndarray< double,1 > > values) ;
virtual void flushNames() ;
virtual void writeTask(const std::string &  filename) ;
virtual long long getSolverLIntInfo(const std::string &  name) ;
virtual int getSolverIntInfo(const std::string &  name) ;
virtual double getSolverDoubleInfo(const std::string &  name) ;
virtual void setCallbackHandler(mosek::cbhandler_t  h) ;
virtual void setDataCallbackHandler(mosek::datacbhandler_t  h) ;
virtual void setLogHandler(mosek::msghandler_t  h) ;
virtual void setSolverParam(const std::string &  name,double floatval) ;
virtual void setSolverParam(const std::string &  name,int intval) ;
virtual void setSolverParam(const std::string &  name,const std::string &  strval) ;
virtual void breakSolver() ;
virtual void solve() ;
virtual void flushSolutions() ;
virtual mosek::fusion::SolutionStatus getDualSolutionStatus() ;
virtual mosek::fusion::ProblemStatus getProblemStatus() ;
virtual mosek::fusion::SolutionStatus getPrimalSolutionStatus() ;
virtual double dualObjValue() ;
virtual double primalObjValue() ;
virtual void selectedSolution(mosek::fusion::SolutionType soltype) ;
virtual mosek::fusion::AccSolutionStatus getAcceptedSolutionStatus() ;
virtual void acceptedSolutionStatus(mosek::fusion::AccSolutionStatus what) ;
virtual mosek::fusion::ProblemStatus getProblemStatus(mosek::fusion::SolutionType which) ;
virtual mosek::fusion::SolutionStatus getDualSolutionStatus(mosek::fusion::SolutionType which) ;
virtual mosek::fusion::SolutionStatus getPrimalSolutionStatus(mosek::fusion::SolutionType which) ;
virtual void updateObjective(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::Variable > x) ;
virtual void objective(double c) ;
virtual void objective(mosek::fusion::ObjectiveSense sense,double c) ;
virtual void objective(mosek::fusion::ObjectiveSense sense,monty::rc_ptr< ::mosek::fusion::Expression > expr) ;
virtual void objective(const std::string &  name,double c) ;
virtual void objective(const std::string &  name,mosek::fusion::ObjectiveSense sense,double c) ;
virtual void objective(const std::string &  name,mosek::fusion::ObjectiveSense sense,monty::rc_ptr< ::mosek::fusion::Expression > expr) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Constraint > constraint(const std::string &  name,monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int n,int m,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int n,int m,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int n,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int n,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int n,int m,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int n,int m,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int n,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int n,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::LinPSDDomain > lpsddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int n,int m,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int n,int m,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int n,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int n,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int n,int m,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int n,int m,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int n,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int n,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::PSDDomain > psddom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(std::shared_ptr< monty::ndarray< int,1 > > shp) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(std::shared_ptr< monty::ndarray< int,1 > > shp);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int size,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int size,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int size,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int size,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int size,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int size,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(int size) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(int size);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable() ;
monty::rc_ptr< ::mosek::fusion::Variable > variable();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shp);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int size,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int size,monty::rc_ptr< ::mosek::fusion::ConeDomain > qdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int size,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int size,monty::rc_ptr< ::mosek::fusion::RangeDomain > rdom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int size,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int size,monty::rc_ptr< ::mosek::fusion::LinearDomain > ldom);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name,int size) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name,int size);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__variable(const std::string &  name) ;
monty::rc_ptr< ::mosek::fusion::Variable > variable(const std::string &  name);
static std::string getVersion();
virtual bool hasConstraint(const std::string &  name) ;
virtual bool hasVariable(const std::string &  name) ;
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__getConstraint(int index) ;
monty::rc_ptr< ::mosek::fusion::Constraint > getConstraint(int index);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Model__getConstraint(const std::string &  name) ;
monty::rc_ptr< ::mosek::fusion::Constraint > getConstraint(const std::string &  name);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__getVariable(int index) ;
monty::rc_ptr< ::mosek::fusion::Variable > getVariable(int index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Model__getVariable(const std::string &  name) ;
monty::rc_ptr< ::mosek::fusion::Variable > getVariable(const std::string &  name);
virtual std::string getName() ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Model__clone() ;
monty::rc_ptr< ::mosek::fusion::Model > clone();
}; // class Model;

// mosek.fusion.Debug from file 'src/fusion/cxx/Debug.h'
class Debug : public monty::RefCounted
{    
  p_Debug * ptr;
public:  
  friend struct p_Debug;
  typedef monty::rc_ptr<Debug> t;

  virtual ~Debug();

  Debug();

  static t o();
  t p(const std::string & val);
  t p(      int val);
  t p(long long val);
  t p(double    val);
  t p(  bool    val);
  t lf();

  t p(const std::shared_ptr<monty::ndarray<double,1>>    & val);
  t p(const std::shared_ptr<monty::ndarray<int,1>>       & val);
  t p(const std::shared_ptr<monty::ndarray<long long,1>> & val);


  t __mosek_2fusion_2Debug__p(const std::string & val) { return p(val); }
  t __mosek_2fusion_2Debug__p(      int val) { return p(val); }
  t __mosek_2fusion_2Debug__p(long long val) { return p(val); }
  t __mosek_2fusion_2Debug__p(double    val) { return p(val); }
  t __mosek_2fusion_2Debug__p(  bool    val) { return p(val); }
    
  t __mosek_2fusion_2Debug__p(const std::shared_ptr<monty::ndarray<double,1>>    & val) { return p(val); }
  t __mosek_2fusion_2Debug__p(const std::shared_ptr<monty::ndarray<int,1>>       & val) { return p(val); }
  t __mosek_2fusion_2Debug__p(const std::shared_ptr<monty::ndarray<long long,1>> & val) { return p(val); }

  t __mosek_2fusion_2Debug__lf() { return lf(); }
};
// End of file 'src/fusion/cxx/Debug.h'
class Sort : public virtual monty::RefCounted
{
public: p_Sort * _impl;
protected: Sort(p_Sort * _impl);
public:
Sort(const Sort &) = delete;
const Sort & operator=(const Sort &) = delete;
friend class p_Sort;
virtual ~Sort();
virtual void destroy();
typedef monty::rc_ptr< Sort > t;

static void argTransposeSort(std::shared_ptr< monty::ndarray< long long,1 > > perm,std::shared_ptr< monty::ndarray< long long,1 > > ptrb,int m,int n,int p,std::shared_ptr< monty::ndarray< long long,1 > > val);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,std::shared_ptr< monty::ndarray< long long,1 > > vals2);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,std::shared_ptr< monty::ndarray< int,1 > > vals2);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,long long first,long long last);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,long long first,long long last);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,std::shared_ptr< monty::ndarray< long long,1 > > vals2,long long first,long long last);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,std::shared_ptr< monty::ndarray< int,1 > > vals2,long long first,long long last);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,long long first,long long last,bool check);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,long long first,long long last,bool check);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,std::shared_ptr< monty::ndarray< long long,1 > > vals2,long long first,long long last,bool check);
static void argsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,std::shared_ptr< monty::ndarray< int,1 > > vals2,long long first,long long last,bool check);
static void argbucketsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals,long long first,long long last,long long minv,long long maxv);
static void argbucketsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals,long long first,long long last,int minv,int maxv);
static void getminmax(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,std::shared_ptr< monty::ndarray< long long,1 > > vals2,long long first,long long last,std::shared_ptr< monty::ndarray< long long,1 > > res);
static void getminmax(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,std::shared_ptr< monty::ndarray< int,1 > > vals2,long long first,long long last,std::shared_ptr< monty::ndarray< int,1 > > res);
static bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,long long first,long long last,bool check);
static bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,long long first,long long last,bool check);
static bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,std::shared_ptr< monty::ndarray< long long,1 > > vals2,long long first,long long last,bool check);
static bool issorted(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,std::shared_ptr< monty::ndarray< int,1 > > vals2,long long first,long long last,bool check);
}; // class Sort;

class IndexCounter : public virtual monty::RefCounted
{
public: p_IndexCounter * _impl;
protected: IndexCounter(p_IndexCounter * _impl);
public:
IndexCounter(const IndexCounter &) = delete;
const IndexCounter & operator=(const IndexCounter &) = delete;
friend class p_IndexCounter;
virtual ~IndexCounter();
virtual void destroy();
typedef monty::rc_ptr< IndexCounter > t;

IndexCounter(std::shared_ptr< monty::ndarray< int,1 > > shape);
IndexCounter(long long start_,std::shared_ptr< monty::ndarray< int,1 > > dims_,std::shared_ptr< monty::ndarray< int,1 > > shape);
IndexCounter(long long start_,std::shared_ptr< monty::ndarray< int,1 > > dims_,std::shared_ptr< monty::ndarray< long long,1 > > strides_);
virtual bool atEnd() ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getIndex() ;
virtual long long next() ;
virtual long long get() ;
virtual void inc() ;
virtual void reset() ;
}; // class IndexCounter;

class CommonTools : public virtual monty::RefCounted
{
public: p_CommonTools * _impl;
protected: CommonTools(p_CommonTools * _impl);
public:
CommonTools(const CommonTools &) = delete;
const CommonTools & operator=(const CommonTools &) = delete;
friend class p_CommonTools;
virtual ~CommonTools();
virtual void destroy();
typedef monty::rc_ptr< CommonTools > t;

static std::shared_ptr< monty::ndarray< long long,1 > > resize(std::shared_ptr< monty::ndarray< long long,1 > > values,int newsize);
static std::shared_ptr< monty::ndarray< int,1 > > resize(std::shared_ptr< monty::ndarray< int,1 > > values,int newsize);
static std::shared_ptr< monty::ndarray< double,1 > > resize(std::shared_ptr< monty::ndarray< double,1 > > values,int newsize);
static int binarySearch(std::shared_ptr< monty::ndarray< int,1 > > values,int target);
static int binarySearch(std::shared_ptr< monty::ndarray< long long,1 > > values,long long target);
static int binarySearchR(std::shared_ptr< monty::ndarray< long long,1 > > values,long long target);
static int binarySearchL(std::shared_ptr< monty::ndarray< long long,1 > > values,long long target);
static void ndIncr(std::shared_ptr< monty::ndarray< int,1 > > ndidx,std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
static void transposeTriplets(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > val,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< long long,1 > >,1 > > tsubi_,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< long long,1 > >,1 > > tsubj_,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< double,1 > >,1 > > tval_,long long nelm,int dimi,int dimj);
static void transposeTriplets(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > val,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > tsubi_,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > tsubj_,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< double,1 > >,1 > > tval_,long long nelm,int dimi,int dimj);
static void tripletSort(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > val,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > tsubi_,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< int,1 > >,1 > > tsubj_,std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< double,1 > >,1 > > tval_,long long nelm,int dimi,int dimj);
static void argMSort(std::shared_ptr< monty::ndarray< int,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals);
static void argQsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< long long,1 > > vals1,std::shared_ptr< monty::ndarray< long long,1 > > vals2,long long first,long long last);
static void argQsort(std::shared_ptr< monty::ndarray< long long,1 > > idx,std::shared_ptr< monty::ndarray< int,1 > > vals1,std::shared_ptr< monty::ndarray< int,1 > > vals2,long long first,long long last);
}; // class CommonTools;

class SolutionStruct : public virtual monty::RefCounted
{
public: p_SolutionStruct * _impl;
protected: SolutionStruct(p_SolutionStruct * _impl);
public:
SolutionStruct(const SolutionStruct &) = delete;
const SolutionStruct & operator=(const SolutionStruct &) = delete;
friend class p_SolutionStruct;
virtual ~SolutionStruct();
virtual void destroy();
typedef monty::rc_ptr< SolutionStruct > t;
std::shared_ptr< monty::ndarray< double,1 > > get_yx();
void set_yx(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_snx();
void set_snx(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_sux();
void set_sux(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_slx();
void set_slx(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_bars();
void set_bars(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_barx();
void set_barx(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_y();
void set_y(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_suc();
void set_suc(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_slc();
void set_slc(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_xx();
void set_xx(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_xc();
void set_xc(std::shared_ptr< monty::ndarray< double,1 > > val);
double get_dobj();
void set_dobj(double val);
double get_pobj();
void set_pobj(double val);
mosek::fusion::ProblemStatus get_probstatus();
void set_probstatus(mosek::fusion::ProblemStatus val);
mosek::fusion::SolutionStatus get_dstatus();
void set_dstatus(mosek::fusion::SolutionStatus val);
mosek::fusion::SolutionStatus get_pstatus();
void set_pstatus(mosek::fusion::SolutionStatus val);
int get_sol_numbarvar();
void set_sol_numbarvar(int val);
int get_sol_numcone();
void set_sol_numcone(int val);
int get_sol_numvar();
void set_sol_numvar(int val);
int get_sol_numcon();
void set_sol_numcon(int val);

SolutionStruct(int numvar,int numcon,int numcone,int numbarvar);
SolutionStruct(monty::rc_ptr< ::mosek::fusion::SolutionStruct > that);
virtual monty::rc_ptr< ::mosek::fusion::SolutionStruct > __mosek_2fusion_2SolutionStruct__clone() ;
monty::rc_ptr< ::mosek::fusion::SolutionStruct > clone();
virtual void resize(int numvar,int numcon,int numcone,int numbarvar) ;
virtual bool isDualAcceptable(mosek::fusion::AccSolutionStatus acceptable_sol) ;
virtual bool isPrimalAcceptable(mosek::fusion::AccSolutionStatus acceptable_sol) ;
}; // class SolutionStruct;

class ConNZStruct : public virtual monty::RefCounted
{
public: p_ConNZStruct * _impl;
protected: ConNZStruct(p_ConNZStruct * _impl);
public:
ConNZStruct(const ConNZStruct &) = delete;
const ConNZStruct & operator=(const ConNZStruct &) = delete;
friend class p_ConNZStruct;
virtual ~ConNZStruct();
virtual void destroy();
typedef monty::rc_ptr< ConNZStruct > t;
std::shared_ptr< monty::ndarray< int,1 > > get_barmidx();
void set_barmidx(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_barsubj();
void set_barsubj(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_barsubi();
void set_barsubi(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_bfix();
void set_bfix(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_cof();
void set_cof(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_subj();
void set_subj(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< long long,1 > > get_ptrb();
void set_ptrb(std::shared_ptr< monty::ndarray< long long,1 > > val);

ConNZStruct(std::shared_ptr< monty::ndarray< long long,1 > > ptrb_,std::shared_ptr< monty::ndarray< int,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > cof_,std::shared_ptr< monty::ndarray< double,1 > > bfix_,std::shared_ptr< monty::ndarray< int,1 > > barsubi_,std::shared_ptr< monty::ndarray< int,1 > > barsubj_,std::shared_ptr< monty::ndarray< int,1 > > barmidx_);
}; // class ConNZStruct;

class BaseVariable : public virtual ::mosek::fusion::Variable
{
public: p_BaseVariable * _impl;
protected: BaseVariable(p_BaseVariable * _impl);
public:
BaseVariable(const BaseVariable &) = delete;
const BaseVariable & operator=(const BaseVariable &) = delete;
friend class p_BaseVariable;
virtual ~BaseVariable();
virtual void destroy();
typedef monty::rc_ptr< BaseVariable > t;

BaseVariable(monty::rc_ptr< ::mosek::fusion::Model > m,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs);
virtual /* override */ std::string toString() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2BaseVariable__eval() ;
monty::rc_ptr< ::mosek::fusion::FlatExpr > eval();
/* override: mosek.fusion.Expression.eval*/
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval();
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(int dim0,int dim1,int dim2) ;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(int dim0,int dim1,int dim2);
/* override: mosek.fusion.Variable.reshape*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1,int dim2);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(int dim0,int dim1) ;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(int dim0,int dim1);
/* override: mosek.fusion.Variable.reshape*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(int dim0) ;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(int dim0);
/* override: mosek.fusion.Variable.reshape*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape) ;
monty::rc_ptr< ::mosek::fusion::Variable > reshape(std::shared_ptr< monty::ndarray< int,1 > > shape);
/* override: mosek.fusion.Variable.reshape*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape);
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > v) ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2BaseVariable__getModel() ;
monty::rc_ptr< ::mosek::fusion::Model > getModel();
/* override: mosek.fusion.Variable.getModel*/
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel();
virtual int getND() ;
virtual int getDim(int d) ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
virtual long long getSize() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > level() ;
virtual void makeContinuous() ;
virtual void makeInteger() ;
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__transpose() ;
monty::rc_ptr< ::mosek::fusion::Variable > transpose();
/* override: mosek.fusion.Variable.transpose*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int i0,int i1,int i2) ;
monty::rc_ptr< ::mosek::fusion::Variable > index(int i0,int i1,int i2);
/* override: mosek.fusion.Variable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1,int i2);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int i0,int i1) ;
monty::rc_ptr< ::mosek::fusion::Variable > index(int i0,int i1);
/* override: mosek.fusion.Variable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(std::shared_ptr< monty::ndarray< int,1 > > index) ;
monty::rc_ptr< ::mosek::fusion::Variable > index(std::shared_ptr< monty::ndarray< int,1 > > index);
/* override: mosek.fusion.Variable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int index) ;
monty::rc_ptr< ::mosek::fusion::Variable > index(int index);
/* override: mosek.fusion.Variable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2) ;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2);
/* override: mosek.fusion.Variable.pick*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1) ;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1);
/* override: mosek.fusion.Variable.pick*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs) ;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,2 > > midxs);
/* override: mosek.fusion.Variable.pick*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs) ;
monty::rc_ptr< ::mosek::fusion::Variable > pick(std::shared_ptr< monty::ndarray< int,1 > > idxs);
/* override: mosek.fusion.Variable.pick*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__antidiag(int index) ;
monty::rc_ptr< ::mosek::fusion::Variable > antidiag(int index);
/* override: mosek.fusion.Variable.antidiag*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__antidiag() ;
monty::rc_ptr< ::mosek::fusion::Variable > antidiag();
/* override: mosek.fusion.Variable.antidiag*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__diag(int index) ;
monty::rc_ptr< ::mosek::fusion::Variable > diag(int index);
/* override: mosek.fusion.Variable.diag*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int index);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__diag() ;
monty::rc_ptr< ::mosek::fusion::Variable > diag();
/* override: mosek.fusion.Variable.diag*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag();
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last) ;
monty::rc_ptr< ::mosek::fusion::Variable > slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
/* override: mosek.fusion.Variable.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(int first,int last) ;
monty::rc_ptr< ::mosek::fusion::Variable > slice(int first,int last);
/* override: mosek.fusion.Variable.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int first,int last);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseVariable__asExpr() ;
monty::rc_ptr< ::mosek::fusion::Expression > asExpr();
/* override: mosek.fusion.Variable.asExpr*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr();
virtual int inst(int spoffset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,int nioffset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) ;
virtual int numInst() ;
virtual void inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > nindex) ;
virtual void set_values(std::shared_ptr< monty::ndarray< double,1 > > values,bool primal) ;
virtual void values(int offset,std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) ;
virtual void make_continuous() ;
virtual void make_integer() ;
}; // class BaseVariable;

class SliceVariable : public ::mosek::fusion::BaseVariable
{
SliceVariable(monty::rc_ptr< ::mosek::fusion::Model > m,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs);
protected: SliceVariable(p_SliceVariable * _impl);
public:
SliceVariable(const SliceVariable &) = delete;
const SliceVariable & operator=(const SliceVariable &) = delete;
friend class p_SliceVariable;
virtual ~SliceVariable();
virtual void destroy();
typedef monty::rc_ptr< SliceVariable > t;

}; // class SliceVariable;

class ModelVariable : public ::mosek::fusion::BaseVariable
{
protected: ModelVariable(p_ModelVariable * _impl);
public:
ModelVariable(const ModelVariable &) = delete;
const ModelVariable & operator=(const ModelVariable &) = delete;
friend class p_ModelVariable;
virtual ~ModelVariable();
virtual void destroy();
typedef monty::rc_ptr< ModelVariable > t;

virtual void elementName(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb) ;
}; // class ModelVariable;

class SymRangedVariable : public ::mosek::fusion::ModelVariable, public virtual ::mosek::fusion::SymmetricVariable
{
SymRangedVariable(monty::rc_ptr< ::mosek::fusion::SymRangedVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
SymRangedVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,long long varid,int dim,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: SymRangedVariable(p_SymRangedVariable * _impl);
public:
SymRangedVariable(const SymRangedVariable &) = delete;
const SymRangedVariable & operator=(const SymRangedVariable &) = delete;
friend class p_SymRangedVariable;
virtual ~SymRangedVariable();
virtual void destroy();
typedef monty::rc_ptr< SymRangedVariable > t;

public:
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr() { return ::mosek::fusion::BaseVariable::asExpr(); }
virtual void make_continuous() { ::mosek::fusion::BaseVariable::make_continuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int first,int last) { return ::mosek::fusion::BaseVariable::slice(first,last); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs) { return ::mosek::fusion::BaseVariable::pick(midxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs) { return ::mosek::fusion::BaseVariable::pick(idxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag() { return ::mosek::fusion::BaseVariable::antidiag(); }
virtual void makeContinuous() { ::mosek::fusion::BaseVariable::makeContinuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1,int dim2) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1,dim2); }
virtual void set_values(std::shared_ptr< monty::ndarray< double,1 > > values,bool primal) { ::mosek::fusion::BaseVariable::set_values(values,primal); };
virtual int getND() { return ::mosek::fusion::BaseVariable::getND(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2) { return ::mosek::fusion::BaseVariable::pick(i0,i1,i2); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int index) { return ::mosek::fusion::BaseVariable::index(index); }
virtual void makeInteger() { ::mosek::fusion::BaseVariable::makeInteger(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1,int i2) { return ::mosek::fusion::BaseVariable::index(i0,i1,i2); }
virtual int getDim(int d) { return ::mosek::fusion::BaseVariable::getDim(d); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag() { return ::mosek::fusion::BaseVariable::diag(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1) { return ::mosek::fusion::BaseVariable::index(i0,i1); }
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() { return ::mosek::fusion::BaseVariable::getShape(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose() { return ::mosek::fusion::BaseVariable::transpose(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > index) { return ::mosek::fusion::BaseVariable::index(index); }
virtual long long getSize() { return ::mosek::fusion::BaseVariable::getSize(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1) { return ::mosek::fusion::BaseVariable::pick(i0,i1); }
virtual std::shared_ptr< monty::ndarray< double,1 > > level() { return ::mosek::fusion::BaseVariable::level(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape) { return ::mosek::fusion::BaseVariable::reshape(shape); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0) { return ::mosek::fusion::BaseVariable::reshape(dim0); }
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel() { return ::mosek::fusion::BaseVariable::getModel(); }
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) { ::mosek::fusion::BaseVariable::eval(rs,ws,xs); };
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > v) { ::mosek::fusion::BaseVariable::setLevel(v); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int index) { return ::mosek::fusion::BaseVariable::diag(index); }
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() { return ::mosek::fusion::BaseVariable::dual(); };
virtual int inst(int spoffset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,int nioffset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) { return ::mosek::fusion::BaseVariable::inst(spoffset,sparsity,nioffset,nativeidxs); };
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() { return ::mosek::fusion::BaseVariable::eval(); }
virtual int numInst() { return ::mosek::fusion::BaseVariable::numInst(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last) { return ::mosek::fusion::BaseVariable::slice(first,last); }
virtual std::string toString() { return ::mosek::fusion::BaseVariable::toString(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int index) { return ::mosek::fusion::BaseVariable::antidiag(index); }
virtual void make_integer() { ::mosek::fusion::BaseVariable::make_integer(); };
virtual void inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > nindex) { ::mosek::fusion::BaseVariable::inst(offset,nindex); };
virtual void values(int offset,std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) { ::mosek::fusion::BaseVariable::values(offset,target,primal); };
}; // class SymRangedVariable;

class RangedVariable : public ::mosek::fusion::ModelVariable
{
RangedVariable(monty::rc_ptr< ::mosek::fusion::RangedVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
RangedVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,long long varid,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: RangedVariable(p_RangedVariable * _impl);
public:
RangedVariable(const RangedVariable &) = delete;
const RangedVariable & operator=(const RangedVariable &) = delete;
friend class p_RangedVariable;
virtual ~RangedVariable();
virtual void destroy();
typedef monty::rc_ptr< RangedVariable > t;

virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2RangedVariable__elementDesc(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb) ;
monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > elementDesc(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb);
}; // class RangedVariable;

class LinearPSDVariable : public ::mosek::fusion::ModelVariable
{
LinearPSDVariable(monty::rc_ptr< ::mosek::fusion::LinearPSDVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
LinearPSDVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,int varid,std::shared_ptr< monty::ndarray< int,1 > > shape,int conedim,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs);
protected: LinearPSDVariable(p_LinearPSDVariable * _impl);
public:
LinearPSDVariable(const LinearPSDVariable &) = delete;
const LinearPSDVariable & operator=(const LinearPSDVariable &) = delete;
friend class p_LinearPSDVariable;
virtual ~LinearPSDVariable();
virtual void destroy();
typedef monty::rc_ptr< LinearPSDVariable > t;

virtual /* override */ std::string toString() ;
virtual void make_continuous(std::shared_ptr< monty::ndarray< long long,1 > > idxs) ;
virtual void make_integer(std::shared_ptr< monty::ndarray< long long,1 > > idxs) ;
}; // class LinearPSDVariable;

class PSDVariable : public ::mosek::fusion::ModelVariable, public virtual ::mosek::fusion::SymmetricVariable
{
PSDVariable(monty::rc_ptr< ::mosek::fusion::PSDVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
PSDVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,int varid,std::shared_ptr< monty::ndarray< int,1 > > shape,int conedim1,int conedim2,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs);
protected: PSDVariable(p_PSDVariable * _impl);
public:
PSDVariable(const PSDVariable &) = delete;
const PSDVariable & operator=(const PSDVariable &) = delete;
friend class p_PSDVariable;
virtual ~PSDVariable();
virtual void destroy();
typedef monty::rc_ptr< PSDVariable > t;

virtual /* override */ std::string toString() ;
virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2PSDVariable__elementDesc(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb) ;
monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > elementDesc(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb);
public:
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr() { return ::mosek::fusion::BaseVariable::asExpr(); }
virtual void make_continuous() { ::mosek::fusion::BaseVariable::make_continuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int first,int last) { return ::mosek::fusion::BaseVariable::slice(first,last); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs) { return ::mosek::fusion::BaseVariable::pick(midxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs) { return ::mosek::fusion::BaseVariable::pick(idxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag() { return ::mosek::fusion::BaseVariable::antidiag(); }
virtual void makeContinuous() { ::mosek::fusion::BaseVariable::makeContinuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1,int dim2) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1,dim2); }
virtual void set_values(std::shared_ptr< monty::ndarray< double,1 > > values,bool primal) { ::mosek::fusion::BaseVariable::set_values(values,primal); };
virtual int getND() { return ::mosek::fusion::BaseVariable::getND(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2) { return ::mosek::fusion::BaseVariable::pick(i0,i1,i2); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int index) { return ::mosek::fusion::BaseVariable::index(index); }
virtual void makeInteger() { ::mosek::fusion::BaseVariable::makeInteger(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1,int i2) { return ::mosek::fusion::BaseVariable::index(i0,i1,i2); }
virtual int getDim(int d) { return ::mosek::fusion::BaseVariable::getDim(d); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag() { return ::mosek::fusion::BaseVariable::diag(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1) { return ::mosek::fusion::BaseVariable::index(i0,i1); }
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() { return ::mosek::fusion::BaseVariable::getShape(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose() { return ::mosek::fusion::BaseVariable::transpose(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > index) { return ::mosek::fusion::BaseVariable::index(index); }
virtual long long getSize() { return ::mosek::fusion::BaseVariable::getSize(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1) { return ::mosek::fusion::BaseVariable::pick(i0,i1); }
virtual std::shared_ptr< monty::ndarray< double,1 > > level() { return ::mosek::fusion::BaseVariable::level(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape) { return ::mosek::fusion::BaseVariable::reshape(shape); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0) { return ::mosek::fusion::BaseVariable::reshape(dim0); }
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel() { return ::mosek::fusion::BaseVariable::getModel(); }
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) { ::mosek::fusion::BaseVariable::eval(rs,ws,xs); };
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > v) { ::mosek::fusion::BaseVariable::setLevel(v); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int index) { return ::mosek::fusion::BaseVariable::diag(index); }
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() { return ::mosek::fusion::BaseVariable::dual(); };
virtual int inst(int spoffset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,int nioffset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) { return ::mosek::fusion::BaseVariable::inst(spoffset,sparsity,nioffset,nativeidxs); };
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() { return ::mosek::fusion::BaseVariable::eval(); }
virtual int numInst() { return ::mosek::fusion::BaseVariable::numInst(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last) { return ::mosek::fusion::BaseVariable::slice(first,last); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int index) { return ::mosek::fusion::BaseVariable::antidiag(index); }
virtual void make_integer() { ::mosek::fusion::BaseVariable::make_integer(); };
virtual void inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > nindex) { ::mosek::fusion::BaseVariable::inst(offset,nindex); };
virtual void values(int offset,std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) { ::mosek::fusion::BaseVariable::values(offset,target,primal); };
}; // class PSDVariable;

class SymLinearVariable : public ::mosek::fusion::ModelVariable, public virtual ::mosek::fusion::SymmetricVariable
{
SymLinearVariable(monty::rc_ptr< ::mosek::fusion::SymLinearVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
SymLinearVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,long long varid,int dim,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: SymLinearVariable(p_SymLinearVariable * _impl);
public:
SymLinearVariable(const SymLinearVariable &) = delete;
const SymLinearVariable & operator=(const SymLinearVariable &) = delete;
friend class p_SymLinearVariable;
virtual ~SymLinearVariable();
virtual void destroy();
typedef monty::rc_ptr< SymLinearVariable > t;

public:
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr() { return ::mosek::fusion::BaseVariable::asExpr(); }
virtual void make_continuous() { ::mosek::fusion::BaseVariable::make_continuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int first,int last) { return ::mosek::fusion::BaseVariable::slice(first,last); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs) { return ::mosek::fusion::BaseVariable::pick(midxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs) { return ::mosek::fusion::BaseVariable::pick(idxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag() { return ::mosek::fusion::BaseVariable::antidiag(); }
virtual void makeContinuous() { ::mosek::fusion::BaseVariable::makeContinuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1,int dim2) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1,dim2); }
virtual void set_values(std::shared_ptr< monty::ndarray< double,1 > > values,bool primal) { ::mosek::fusion::BaseVariable::set_values(values,primal); };
virtual int getND() { return ::mosek::fusion::BaseVariable::getND(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2) { return ::mosek::fusion::BaseVariable::pick(i0,i1,i2); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int index) { return ::mosek::fusion::BaseVariable::index(index); }
virtual void makeInteger() { ::mosek::fusion::BaseVariable::makeInteger(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1,int i2) { return ::mosek::fusion::BaseVariable::index(i0,i1,i2); }
virtual int getDim(int d) { return ::mosek::fusion::BaseVariable::getDim(d); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag() { return ::mosek::fusion::BaseVariable::diag(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1) { return ::mosek::fusion::BaseVariable::index(i0,i1); }
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() { return ::mosek::fusion::BaseVariable::getShape(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose() { return ::mosek::fusion::BaseVariable::transpose(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > index) { return ::mosek::fusion::BaseVariable::index(index); }
virtual long long getSize() { return ::mosek::fusion::BaseVariable::getSize(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1) { return ::mosek::fusion::BaseVariable::pick(i0,i1); }
virtual std::shared_ptr< monty::ndarray< double,1 > > level() { return ::mosek::fusion::BaseVariable::level(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape) { return ::mosek::fusion::BaseVariable::reshape(shape); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0) { return ::mosek::fusion::BaseVariable::reshape(dim0); }
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel() { return ::mosek::fusion::BaseVariable::getModel(); }
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) { ::mosek::fusion::BaseVariable::eval(rs,ws,xs); };
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > v) { ::mosek::fusion::BaseVariable::setLevel(v); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int index) { return ::mosek::fusion::BaseVariable::diag(index); }
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() { return ::mosek::fusion::BaseVariable::dual(); };
virtual int inst(int spoffset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,int nioffset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) { return ::mosek::fusion::BaseVariable::inst(spoffset,sparsity,nioffset,nativeidxs); };
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() { return ::mosek::fusion::BaseVariable::eval(); }
virtual int numInst() { return ::mosek::fusion::BaseVariable::numInst(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last) { return ::mosek::fusion::BaseVariable::slice(first,last); }
virtual std::string toString() { return ::mosek::fusion::BaseVariable::toString(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int index) { return ::mosek::fusion::BaseVariable::antidiag(index); }
virtual void make_integer() { ::mosek::fusion::BaseVariable::make_integer(); };
virtual void inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > nindex) { ::mosek::fusion::BaseVariable::inst(offset,nindex); };
virtual void values(int offset,std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) { ::mosek::fusion::BaseVariable::values(offset,target,primal); };
}; // class SymLinearVariable;

class LinearVariable : public ::mosek::fusion::ModelVariable
{
LinearVariable(monty::rc_ptr< ::mosek::fusion::LinearVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
LinearVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,long long varid,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: LinearVariable(p_LinearVariable * _impl);
public:
LinearVariable(const LinearVariable &) = delete;
const LinearVariable & operator=(const LinearVariable &) = delete;
friend class p_LinearVariable;
virtual ~LinearVariable();
virtual void destroy();
typedef monty::rc_ptr< LinearVariable > t;

virtual /* override */ std::string toString() ;
}; // class LinearVariable;

class ConicVariable : public ::mosek::fusion::ModelVariable
{
ConicVariable(monty::rc_ptr< ::mosek::fusion::ConicVariable > v,monty::rc_ptr< ::mosek::fusion::Model > m);
ConicVariable(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,int varid,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: ConicVariable(p_ConicVariable * _impl);
public:
ConicVariable(const ConicVariable &) = delete;
const ConicVariable & operator=(const ConicVariable &) = delete;
friend class p_ConicVariable;
virtual ~ConicVariable();
virtual void destroy();
typedef monty::rc_ptr< ConicVariable > t;

virtual /* override */ std::string toString() ;
}; // class ConicVariable;

class NilVariable : public ::mosek::fusion::BaseVariable, public virtual ::mosek::fusion::SymmetricVariable
{
NilVariable(std::shared_ptr< monty::ndarray< int,1 > > shape);
NilVariable();
protected: NilVariable(p_NilVariable * _impl);
public:
NilVariable(const NilVariable &) = delete;
const NilVariable & operator=(const NilVariable &) = delete;
friend class p_NilVariable;
virtual ~NilVariable();
virtual void destroy();
typedef monty::rc_ptr< NilVariable > t;

virtual monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > __mosek_2fusion_2NilVariable__elementDesc(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb) ;
monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > elementDesc(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb);
virtual void elementName(long long index,monty::rc_ptr< ::mosek::fusion::Utils::StringBuffer > sb) ;
virtual /* override */ int numInst() ;
virtual int inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) ;
virtual /* override */ void inst(int offset,std::shared_ptr< monty::ndarray< long long,1 > > nindex) ;
virtual /* override */ void set_values(std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) ;
virtual /* override */ void values(int offset,std::shared_ptr< monty::ndarray< double,1 > > target,bool primal) ;
virtual /* override */ void make_continuous() ;
virtual /* override */ void make_integer() ;
virtual /* override */ std::string toString() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__index(std::shared_ptr< monty::ndarray< int,1 > > first) ;
monty::rc_ptr< ::mosek::fusion::Variable > index(std::shared_ptr< monty::ndarray< int,1 > > first);
/* override: mosek.fusion.BaseVariable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(std::shared_ptr< monty::ndarray< int,1 > > first);
/* override: mosek.fusion.Variable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(std::shared_ptr< monty::ndarray< int,1 > > first);
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__index(int first) ;
monty::rc_ptr< ::mosek::fusion::Variable > index(int first);
/* override: mosek.fusion.BaseVariable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__index(int first);
/* override: mosek.fusion.Variable.index*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int first);
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last) ;
monty::rc_ptr< ::mosek::fusion::Variable > slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
/* override: mosek.fusion.BaseVariable.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
/* override: mosek.fusion.Variable.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2NilVariable__slice(int first,int last) ;
monty::rc_ptr< ::mosek::fusion::Variable > slice(int first,int last);
/* override: mosek.fusion.BaseVariable.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2BaseVariable__slice(int first,int last);
/* override: mosek.fusion.Variable.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__slice(int first,int last);
public:
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Variable__asExpr() { return ::mosek::fusion::BaseVariable::asExpr(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,2 > > midxs) { return ::mosek::fusion::BaseVariable::pick(midxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > idxs) { return ::mosek::fusion::BaseVariable::pick(idxs); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag() { return ::mosek::fusion::BaseVariable::antidiag(); }
virtual void makeContinuous() { ::mosek::fusion::BaseVariable::makeContinuous(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1,int dim2) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1,dim2); }
virtual int getND() { return ::mosek::fusion::BaseVariable::getND(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1,std::shared_ptr< monty::ndarray< int,1 > > i2) { return ::mosek::fusion::BaseVariable::pick(i0,i1,i2); }
virtual void makeInteger() { ::mosek::fusion::BaseVariable::makeInteger(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1,int i2) { return ::mosek::fusion::BaseVariable::index(i0,i1,i2); }
virtual int getDim(int d) { return ::mosek::fusion::BaseVariable::getDim(d); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag() { return ::mosek::fusion::BaseVariable::diag(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__index(int i0,int i1) { return ::mosek::fusion::BaseVariable::index(i0,i1); }
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() { return ::mosek::fusion::BaseVariable::getShape(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__transpose() { return ::mosek::fusion::BaseVariable::transpose(); }
virtual long long getSize() { return ::mosek::fusion::BaseVariable::getSize(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__pick(std::shared_ptr< monty::ndarray< int,1 > > i0,std::shared_ptr< monty::ndarray< int,1 > > i1) { return ::mosek::fusion::BaseVariable::pick(i0,i1); }
virtual std::shared_ptr< monty::ndarray< double,1 > > level() { return ::mosek::fusion::BaseVariable::level(); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0,int dim1) { return ::mosek::fusion::BaseVariable::reshape(dim0,dim1); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(std::shared_ptr< monty::ndarray< int,1 > > shape) { return ::mosek::fusion::BaseVariable::reshape(shape); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__reshape(int dim0) { return ::mosek::fusion::BaseVariable::reshape(dim0); }
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Variable__getModel() { return ::mosek::fusion::BaseVariable::getModel(); }
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) { ::mosek::fusion::BaseVariable::eval(rs,ws,xs); };
virtual void setLevel(std::shared_ptr< monty::ndarray< double,1 > > v) { ::mosek::fusion::BaseVariable::setLevel(v); };
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__diag(int index) { return ::mosek::fusion::BaseVariable::diag(index); }
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() { return ::mosek::fusion::BaseVariable::dual(); };
virtual int inst(int spoffset,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,int nioffset,std::shared_ptr< monty::ndarray< long long,1 > > nativeidxs) { return ::mosek::fusion::BaseVariable::inst(spoffset,sparsity,nioffset,nativeidxs); };
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval() { return ::mosek::fusion::BaseVariable::eval(); }
virtual monty::rc_ptr< ::mosek::fusion::Variable > __mosek_2fusion_2Variable__antidiag(int index) { return ::mosek::fusion::BaseVariable::antidiag(index); }
}; // class NilVariable;

class Var : public virtual monty::RefCounted
{
public: p_Var * _impl;
protected: Var(p_Var * _impl);
public:
Var(const Var &) = delete;
const Var & operator=(const Var &) = delete;
friend class p_Var;
virtual ~Var();
virtual void destroy();
typedef monty::rc_ptr< Var > t;

static monty::rc_ptr< ::mosek::fusion::Variable > empty(std::shared_ptr< monty::ndarray< int,1 > > shape);
static monty::rc_ptr< ::mosek::fusion::Variable > compress(monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Variable > reshape(monty::rc_ptr< ::mosek::fusion::Variable > v,int d1);
static monty::rc_ptr< ::mosek::fusion::Variable > reshape(monty::rc_ptr< ::mosek::fusion::Variable > v,int d1,int d2);
static monty::rc_ptr< ::mosek::fusion::Variable > flatten(monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Variable > reshape(monty::rc_ptr< ::mosek::fusion::Variable > v,std::shared_ptr< monty::ndarray< int,1 > > shape);
static monty::rc_ptr< ::mosek::fusion::Variable > hrepeat(monty::rc_ptr< ::mosek::fusion::Variable > v,int n);
static monty::rc_ptr< ::mosek::fusion::Variable > vrepeat(monty::rc_ptr< ::mosek::fusion::Variable > v,int n);
static monty::rc_ptr< ::mosek::fusion::Variable > repeat(monty::rc_ptr< ::mosek::fusion::Variable > v,int n);
static monty::rc_ptr< ::mosek::fusion::Variable > repeat(monty::rc_ptr< ::mosek::fusion::Variable > v,int dim,int n);
static monty::rc_ptr< ::mosek::fusion::Variable > stack(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > >,1 > > vlist);
static monty::rc_ptr< ::mosek::fusion::Variable > vstack(monty::rc_ptr< ::mosek::fusion::Variable > v1,monty::rc_ptr< ::mosek::fusion::Variable > v2,monty::rc_ptr< ::mosek::fusion::Variable > v3);
static monty::rc_ptr< ::mosek::fusion::Variable > vstack(monty::rc_ptr< ::mosek::fusion::Variable > v1,monty::rc_ptr< ::mosek::fusion::Variable > v2);
static monty::rc_ptr< ::mosek::fusion::Variable > vstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > v);
static monty::rc_ptr< ::mosek::fusion::Variable > hstack(monty::rc_ptr< ::mosek::fusion::Variable > v1,monty::rc_ptr< ::mosek::fusion::Variable > v2,monty::rc_ptr< ::mosek::fusion::Variable > v3);
static monty::rc_ptr< ::mosek::fusion::Variable > hstack(monty::rc_ptr< ::mosek::fusion::Variable > v1,monty::rc_ptr< ::mosek::fusion::Variable > v2);
static monty::rc_ptr< ::mosek::fusion::Variable > hstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > v);
static monty::rc_ptr< ::mosek::fusion::Variable > stack(monty::rc_ptr< ::mosek::fusion::Variable > v1,monty::rc_ptr< ::mosek::fusion::Variable > v2,monty::rc_ptr< ::mosek::fusion::Variable > v3,int dim);
static monty::rc_ptr< ::mosek::fusion::Variable > stack(monty::rc_ptr< ::mosek::fusion::Variable > v1,monty::rc_ptr< ::mosek::fusion::Variable > v2,int dim);
static monty::rc_ptr< ::mosek::fusion::Variable > stack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > v,int dim);
static monty::rc_ptr< ::mosek::fusion::Variable > promote(monty::rc_ptr< ::mosek::fusion::Variable > v,int nd);
}; // class Var;

class ConstraintCache : public virtual monty::RefCounted
{
public: p_ConstraintCache * _impl;
ConstraintCache(monty::rc_ptr< ::mosek::fusion::ConstraintCache > cc);
protected: ConstraintCache(p_ConstraintCache * _impl);
public:
ConstraintCache(const ConstraintCache &) = delete;
const ConstraintCache & operator=(const ConstraintCache &) = delete;
friend class p_ConstraintCache;
virtual ~ConstraintCache();
virtual void destroy();
typedef monty::rc_ptr< ConstraintCache > t;
std::shared_ptr< monty::ndarray< int,1 > > get_barmatidx();
void set_barmatidx(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_barsubj();
void set_barsubj(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_barsubi();
void set_barsubi(std::shared_ptr< monty::ndarray< int,1 > > val);
long long get_nbarnz();
void set_nbarnz(long long val);
long long get_nunordered();
void set_nunordered(long long val);
std::shared_ptr< monty::ndarray< int,1 > > get_buffer_subi();
void set_buffer_subi(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_buffer_subj();
void set_buffer_subj(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_buffer_cof();
void set_buffer_cof(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_bfix();
void set_bfix(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_cof();
void set_cof(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_subi();
void set_subi(std::shared_ptr< monty::ndarray< int,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_subj();
void set_subj(std::shared_ptr< monty::ndarray< int,1 > > val);
long long get_nnz();
void set_nnz(long long val);
int get_nrows();
void set_nrows(int val);

ConstraintCache(std::shared_ptr< monty::ndarray< long long,1 > > ptrb_,std::shared_ptr< monty::ndarray< double,1 > > cof_,std::shared_ptr< monty::ndarray< int,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > bfix_,std::shared_ptr< monty::ndarray< int,1 > > barsubi_,std::shared_ptr< monty::ndarray< int,1 > > barsubj_,std::shared_ptr< monty::ndarray< int,1 > > barmatidx_);
virtual void add(std::shared_ptr< monty::ndarray< long long,1 > > ptrb_,std::shared_ptr< monty::ndarray< int,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > cof_,std::shared_ptr< monty::ndarray< double,1 > > bfix_) ;
virtual long long flush(std::shared_ptr< monty::ndarray< int,1 > > ressubi,std::shared_ptr< monty::ndarray< int,1 > > ressubj,std::shared_ptr< monty::ndarray< double,1 > > rescof,std::shared_ptr< monty::ndarray< double,1 > > resbfix) ;
virtual long long numUnsorted() ;
}; // class ConstraintCache;

class Constraint : public virtual monty::RefCounted
{
public: p_Constraint * _impl;
protected: Constraint(p_Constraint * _impl);
public:
Constraint(const Constraint &) = delete;
const Constraint & operator=(const Constraint &) = delete;
friend class p_Constraint;
virtual ~Constraint();
virtual void destroy();
typedef monty::rc_ptr< Constraint > t;

Constraint(monty::rc_ptr< ::mosek::fusion::Constraint > c,monty::rc_ptr< ::mosek::fusion::Model > m);
Constraint(monty::rc_ptr< ::mosek::fusion::Model > model,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
virtual /* override */ std::string toString() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > dual() ;
virtual std::shared_ptr< monty::ndarray< double,1 > > level() ;
virtual void update(std::shared_ptr< monty::ndarray< double,1 > > bfix) ;
virtual void update(monty::rc_ptr< ::mosek::fusion::Expression > expr) ;
virtual void update(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::Variable > x,bool bfixupdate) ;
virtual void update(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::Variable > x) ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Constraint__get_model() ;
monty::rc_ptr< ::mosek::fusion::Model > get_model();
virtual int get_nd() ;
virtual long long size() ;
static monty::rc_ptr< ::mosek::fusion::Constraint > stack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > clist,int dim);
static monty::rc_ptr< ::mosek::fusion::Constraint > stack(monty::rc_ptr< ::mosek::fusion::Constraint > v1,monty::rc_ptr< ::mosek::fusion::Constraint > v2,monty::rc_ptr< ::mosek::fusion::Constraint > v3,int dim);
static monty::rc_ptr< ::mosek::fusion::Constraint > stack(monty::rc_ptr< ::mosek::fusion::Constraint > v1,monty::rc_ptr< ::mosek::fusion::Constraint > v2,int dim);
static monty::rc_ptr< ::mosek::fusion::Constraint > hstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > clist);
static monty::rc_ptr< ::mosek::fusion::Constraint > vstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Constraint >,1 > > clist);
static monty::rc_ptr< ::mosek::fusion::Constraint > hstack(monty::rc_ptr< ::mosek::fusion::Constraint > v1,monty::rc_ptr< ::mosek::fusion::Constraint > v2,monty::rc_ptr< ::mosek::fusion::Constraint > v3);
static monty::rc_ptr< ::mosek::fusion::Constraint > vstack(monty::rc_ptr< ::mosek::fusion::Constraint > v1,monty::rc_ptr< ::mosek::fusion::Constraint > v2,monty::rc_ptr< ::mosek::fusion::Constraint > v3);
static monty::rc_ptr< ::mosek::fusion::Constraint > hstack(monty::rc_ptr< ::mosek::fusion::Constraint > v1,monty::rc_ptr< ::mosek::fusion::Constraint > v2);
static monty::rc_ptr< ::mosek::fusion::Constraint > vstack(monty::rc_ptr< ::mosek::fusion::Constraint > v1,monty::rc_ptr< ::mosek::fusion::Constraint > v2);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__index(std::shared_ptr< monty::ndarray< int,1 > > idxa) ;
monty::rc_ptr< ::mosek::fusion::Constraint > index(std::shared_ptr< monty::ndarray< int,1 > > idxa);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__index(int idx) ;
monty::rc_ptr< ::mosek::fusion::Constraint > index(int idx);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta) ;
monty::rc_ptr< ::mosek::fusion::Constraint > slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta);
virtual monty::rc_ptr< ::mosek::fusion::Constraint > __mosek_2fusion_2Constraint__slice(int first,int last) ;
monty::rc_ptr< ::mosek::fusion::Constraint > slice(int first,int last);
virtual int getND() ;
virtual int getSize() ;
virtual monty::rc_ptr< ::mosek::fusion::Model > __mosek_2fusion_2Constraint__getModel() ;
monty::rc_ptr< ::mosek::fusion::Model > getModel();
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
}; // class Constraint;

class SliceConstraint : public ::mosek::fusion::Constraint
{
SliceConstraint(monty::rc_ptr< ::mosek::fusion::Model > model,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: SliceConstraint(p_SliceConstraint * _impl);
public:
SliceConstraint(const SliceConstraint &) = delete;
const SliceConstraint & operator=(const SliceConstraint &) = delete;
friend class p_SliceConstraint;
virtual ~SliceConstraint();
virtual void destroy();
typedef monty::rc_ptr< SliceConstraint > t;

virtual /* override */ std::string toString() ;
}; // class SliceConstraint;

class ModelConstraint : public ::mosek::fusion::Constraint
{
protected: ModelConstraint(p_ModelConstraint * _impl);
public:
ModelConstraint(const ModelConstraint &) = delete;
const ModelConstraint & operator=(const ModelConstraint &) = delete;
friend class p_ModelConstraint;
virtual ~ModelConstraint();
virtual void destroy();
typedef monty::rc_ptr< ModelConstraint > t;

virtual /* override */ std::string toString() ;
}; // class ModelConstraint;

class LinearPSDConstraint : public ::mosek::fusion::ModelConstraint
{
LinearPSDConstraint(monty::rc_ptr< ::mosek::fusion::LinearPSDConstraint > c,monty::rc_ptr< ::mosek::fusion::Model > m);
LinearPSDConstraint(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,int conid,std::shared_ptr< monty::ndarray< int,1 > > shape,int conedim,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs,std::shared_ptr< monty::ndarray< long long,1 > > slackidxs);
protected: LinearPSDConstraint(p_LinearPSDConstraint * _impl);
public:
LinearPSDConstraint(const LinearPSDConstraint &) = delete;
const LinearPSDConstraint & operator=(const LinearPSDConstraint &) = delete;
friend class p_LinearPSDConstraint;
virtual ~LinearPSDConstraint();
virtual void destroy();
typedef monty::rc_ptr< LinearPSDConstraint > t;

}; // class LinearPSDConstraint;

class PSDConstraint : public ::mosek::fusion::ModelConstraint
{
PSDConstraint(monty::rc_ptr< ::mosek::fusion::PSDConstraint > c,monty::rc_ptr< ::mosek::fusion::Model > m);
PSDConstraint(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,int conid,std::shared_ptr< monty::ndarray< int,1 > > shape,int conedim0,int conedim1,std::shared_ptr< monty::ndarray< long long,1 > > slackidxs,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: PSDConstraint(p_PSDConstraint * _impl);
public:
PSDConstraint(const PSDConstraint &) = delete;
const PSDConstraint & operator=(const PSDConstraint &) = delete;
friend class p_PSDConstraint;
virtual ~PSDConstraint();
virtual void destroy();
typedef monty::rc_ptr< PSDConstraint > t;

virtual /* override */ std::string toString() ;
}; // class PSDConstraint;

class RangedConstraint : public ::mosek::fusion::ModelConstraint
{
RangedConstraint(monty::rc_ptr< ::mosek::fusion::RangedConstraint > c,monty::rc_ptr< ::mosek::fusion::Model > m);
RangedConstraint(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs);
protected: RangedConstraint(p_RangedConstraint * _impl);
public:
RangedConstraint(const RangedConstraint &) = delete;
const RangedConstraint & operator=(const RangedConstraint &) = delete;
friend class p_RangedConstraint;
virtual ~RangedConstraint();
virtual void destroy();
typedef monty::rc_ptr< RangedConstraint > t;

}; // class RangedConstraint;

class ConicConstraint : public ::mosek::fusion::ModelConstraint
{
ConicConstraint(monty::rc_ptr< ::mosek::fusion::ConicConstraint > c,monty::rc_ptr< ::mosek::fusion::Model > m);
ConicConstraint(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,monty::rc_ptr< ::mosek::fusion::ConeDomain > dom,std::shared_ptr< monty::ndarray< int,1 > > shape,int conid,std::shared_ptr< monty::ndarray< int,1 > > nativeidxs,std::shared_ptr< monty::ndarray< int,1 > > nativeslack);
protected: ConicConstraint(p_ConicConstraint * _impl);
public:
ConicConstraint(const ConicConstraint &) = delete;
const ConicConstraint & operator=(const ConicConstraint &) = delete;
friend class p_ConicConstraint;
virtual ~ConicConstraint();
virtual void destroy();
typedef monty::rc_ptr< ConicConstraint > t;

virtual /* override */ std::string toString() ;
}; // class ConicConstraint;

class LinearConstraint : public ::mosek::fusion::ModelConstraint
{
LinearConstraint(monty::rc_ptr< ::mosek::fusion::LinearConstraint > c,monty::rc_ptr< ::mosek::fusion::Model > m);
LinearConstraint(monty::rc_ptr< ::mosek::fusion::Model > model,const std::string &  name,int conid,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > nidxs);
protected: LinearConstraint(p_LinearConstraint * _impl);
public:
LinearConstraint(const LinearConstraint &) = delete;
const LinearConstraint & operator=(const LinearConstraint &) = delete;
friend class p_LinearConstraint;
virtual ~LinearConstraint();
virtual void destroy();
typedef monty::rc_ptr< LinearConstraint > t;

virtual /* override */ std::string toString() ;
}; // class LinearConstraint;

class Set : public virtual monty::RefCounted
{
public: p_Set * _impl;
protected: Set(p_Set * _impl);
public:
Set(const Set &) = delete;
const Set & operator=(const Set &) = delete;
friend class p_Set;
virtual ~Set();
virtual void destroy();
typedef monty::rc_ptr< Set > t;

static long long size(std::shared_ptr< monty::ndarray< int,1 > > shape);
static bool match(std::shared_ptr< monty::ndarray< int,1 > > s1,std::shared_ptr< monty::ndarray< int,1 > > s2);
static long long linearidx(std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > key);
static std::shared_ptr< monty::ndarray< int,1 > > idxtokey(std::shared_ptr< monty::ndarray< int,1 > > shape,long long idx);
static void idxtokey(std::shared_ptr< monty::ndarray< int,1 > > shape,long long idx,std::shared_ptr< monty::ndarray< int,1 > > dest);
static std::string indexToString(std::shared_ptr< monty::ndarray< int,1 > > shape,long long key);
static std::string keyToString(std::shared_ptr< monty::ndarray< int,1 > > key);
static void indexToKey(std::shared_ptr< monty::ndarray< int,1 > > shape,long long key,std::shared_ptr< monty::ndarray< int,1 > > res);
static std::shared_ptr< monty::ndarray< long long,1 > > strides(std::shared_ptr< monty::ndarray< int,1 > > shape);
static std::shared_ptr< monty::ndarray< int,1 > > make(std::shared_ptr< monty::ndarray< int,1 > > set1,std::shared_ptr< monty::ndarray< int,1 > > set2);
static std::shared_ptr< monty::ndarray< int,1 > > make(std::shared_ptr< monty::ndarray< int,1 > > sizes);
static std::shared_ptr< monty::ndarray< int,1 > > make(int s1,int s2,int s3);
static std::shared_ptr< monty::ndarray< int,1 > > make(int s1,int s2);
static std::shared_ptr< monty::ndarray< int,1 > > make(int sz);
static std::shared_ptr< monty::ndarray< int,1 > > scalar();
static std::shared_ptr< monty::ndarray< int,1 > > make(std::shared_ptr< monty::ndarray< std::string,1 > > names);
}; // class Set;

class ConeDomain : public virtual monty::RefCounted
{
public: p_ConeDomain * _impl;
ConeDomain(mosek::fusion::QConeKey k,double alpha,std::shared_ptr< monty::ndarray< int,1 > > d);
ConeDomain(mosek::fusion::QConeKey k,std::shared_ptr< monty::ndarray< int,1 > > d);
protected: ConeDomain(p_ConeDomain * _impl);
public:
ConeDomain(const ConeDomain &) = delete;
const ConeDomain & operator=(const ConeDomain &) = delete;
friend class p_ConeDomain;
virtual ~ConeDomain();
virtual void destroy();
typedef monty::rc_ptr< ConeDomain > t;

virtual monty::rc_ptr< ::mosek::fusion::ConeDomain > __mosek_2fusion_2ConeDomain__integral() ;
monty::rc_ptr< ::mosek::fusion::ConeDomain > integral();
virtual bool axisIsSet() ;
virtual int getAxis() ;
virtual monty::rc_ptr< ::mosek::fusion::ConeDomain > __mosek_2fusion_2ConeDomain__axis(int a) ;
monty::rc_ptr< ::mosek::fusion::ConeDomain > axis(int a);
}; // class ConeDomain;

class LinPSDDomain : public virtual monty::RefCounted
{
public: p_LinPSDDomain * _impl;
LinPSDDomain(std::shared_ptr< monty::ndarray< int,1 > > shp,int conedim);
LinPSDDomain(std::shared_ptr< monty::ndarray< int,1 > > shp);
LinPSDDomain();
protected: LinPSDDomain(p_LinPSDDomain * _impl);
public:
LinPSDDomain(const LinPSDDomain &) = delete;
const LinPSDDomain & operator=(const LinPSDDomain &) = delete;
friend class p_LinPSDDomain;
virtual ~LinPSDDomain();
virtual void destroy();
typedef monty::rc_ptr< LinPSDDomain > t;

}; // class LinPSDDomain;

class PSDDomain : public virtual monty::RefCounted
{
public: p_PSDDomain * _impl;
PSDDomain(mosek::fusion::PSDKey k,std::shared_ptr< monty::ndarray< int,1 > > shp,int conedim1,int conedim2);
PSDDomain(mosek::fusion::PSDKey k,std::shared_ptr< monty::ndarray< int,1 > > shp);
PSDDomain(mosek::fusion::PSDKey k);
protected: PSDDomain(p_PSDDomain * _impl);
public:
PSDDomain(const PSDDomain &) = delete;
const PSDDomain & operator=(const PSDDomain &) = delete;
friend class p_PSDDomain;
virtual ~PSDDomain();
virtual void destroy();
typedef monty::rc_ptr< PSDDomain > t;

virtual monty::rc_ptr< ::mosek::fusion::PSDDomain > __mosek_2fusion_2PSDDomain__axis(int conedim1,int conedim2) ;
monty::rc_ptr< ::mosek::fusion::PSDDomain > axis(int conedim1,int conedim2);
}; // class PSDDomain;

class RangeDomain : public virtual monty::RefCounted
{
public: p_RangeDomain * _impl;
RangeDomain(bool scalable,std::shared_ptr< monty::ndarray< double,1 > > lb,std::shared_ptr< monty::ndarray< double,1 > > ub,std::shared_ptr< monty::ndarray< int,1 > > dims);
RangeDomain(bool scalable,std::shared_ptr< monty::ndarray< double,1 > > lb,std::shared_ptr< monty::ndarray< double,1 > > ub,std::shared_ptr< monty::ndarray< int,1 > > dims,std::shared_ptr< monty::ndarray< int,2 > > sp);
RangeDomain(bool scalable,std::shared_ptr< monty::ndarray< double,1 > > lb,std::shared_ptr< monty::ndarray< double,1 > > ub,std::shared_ptr< monty::ndarray< int,1 > > dims,std::shared_ptr< monty::ndarray< int,2 > > sp,int steal);
RangeDomain(monty::rc_ptr< ::mosek::fusion::RangeDomain > other);
protected: RangeDomain(p_RangeDomain * _impl);
public:
RangeDomain(const RangeDomain &) = delete;
const RangeDomain & operator=(const RangeDomain &) = delete;
friend class p_RangeDomain;
virtual ~RangeDomain();
virtual void destroy();
typedef monty::rc_ptr< RangeDomain > t;

virtual monty::rc_ptr< ::mosek::fusion::SymmetricRangeDomain > __mosek_2fusion_2RangeDomain__symmetric() ;
monty::rc_ptr< ::mosek::fusion::SymmetricRangeDomain > symmetric();
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__sparse(std::shared_ptr< monty::ndarray< int,2 > > sparsity) ;
monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse(std::shared_ptr< monty::ndarray< int,2 > > sparsity);
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__sparse(std::shared_ptr< monty::ndarray< int,1 > > sparsity) ;
monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse(std::shared_ptr< monty::ndarray< int,1 > > sparsity);
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__sparse() ;
monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse();
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__integral() ;
monty::rc_ptr< ::mosek::fusion::RangeDomain > integral();
virtual monty::rc_ptr< ::mosek::fusion::RangeDomain > __mosek_2fusion_2RangeDomain__withShape(std::shared_ptr< monty::ndarray< int,1 > > shp) ;
monty::rc_ptr< ::mosek::fusion::RangeDomain > withShape(std::shared_ptr< monty::ndarray< int,1 > > shp);
}; // class RangeDomain;

class SymmetricRangeDomain : public ::mosek::fusion::RangeDomain
{
SymmetricRangeDomain(monty::rc_ptr< ::mosek::fusion::RangeDomain > other);
protected: SymmetricRangeDomain(p_SymmetricRangeDomain * _impl);
public:
SymmetricRangeDomain(const SymmetricRangeDomain &) = delete;
const SymmetricRangeDomain & operator=(const SymmetricRangeDomain &) = delete;
friend class p_SymmetricRangeDomain;
virtual ~SymmetricRangeDomain();
virtual void destroy();
typedef monty::rc_ptr< SymmetricRangeDomain > t;

}; // class SymmetricRangeDomain;

class SymmetricLinearDomain : public virtual monty::RefCounted
{
public: p_SymmetricLinearDomain * _impl;
SymmetricLinearDomain(monty::rc_ptr< ::mosek::fusion::LinearDomain > other);
protected: SymmetricLinearDomain(p_SymmetricLinearDomain * _impl);
public:
SymmetricLinearDomain(const SymmetricLinearDomain &) = delete;
const SymmetricLinearDomain & operator=(const SymmetricLinearDomain &) = delete;
friend class p_SymmetricLinearDomain;
virtual ~SymmetricLinearDomain();
virtual void destroy();
typedef monty::rc_ptr< SymmetricLinearDomain > t;

virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2SymmetricLinearDomain__sparse(std::shared_ptr< monty::ndarray< int,2 > > sparsity) ;
monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > sparse(std::shared_ptr< monty::ndarray< int,2 > > sparsity);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2SymmetricLinearDomain__sparse(std::shared_ptr< monty::ndarray< int,1 > > sparsity) ;
monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > sparse(std::shared_ptr< monty::ndarray< int,1 > > sparsity);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2SymmetricLinearDomain__integral() ;
monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > integral();
}; // class SymmetricLinearDomain;

class LinearDomain : public virtual monty::RefCounted
{
public: p_LinearDomain * _impl;
LinearDomain(mosek::fusion::RelationKey k,bool scalable,std::shared_ptr< monty::ndarray< double,1 > > rhs,std::shared_ptr< monty::ndarray< int,1 > > dims);
LinearDomain(mosek::fusion::RelationKey k,bool scalable,std::shared_ptr< monty::ndarray< double,1 > > rhs,std::shared_ptr< monty::ndarray< int,1 > > dims,std::shared_ptr< monty::ndarray< int,2 > > sp,int steal);
LinearDomain(monty::rc_ptr< ::mosek::fusion::LinearDomain > other);
protected: LinearDomain(p_LinearDomain * _impl);
public:
LinearDomain(const LinearDomain &) = delete;
const LinearDomain & operator=(const LinearDomain &) = delete;
friend class p_LinearDomain;
virtual ~LinearDomain();
virtual void destroy();
typedef monty::rc_ptr< LinearDomain > t;

virtual monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > __mosek_2fusion_2LinearDomain__symmetric() ;
monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > symmetric();
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__sparse(std::shared_ptr< monty::ndarray< int,2 > > sparsity) ;
monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse(std::shared_ptr< monty::ndarray< int,2 > > sparsity);
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__sparse(std::shared_ptr< monty::ndarray< int,1 > > sparsity) ;
monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse(std::shared_ptr< monty::ndarray< int,1 > > sparsity);
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__sparse() ;
monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse();
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__integral() ;
monty::rc_ptr< ::mosek::fusion::LinearDomain > integral();
virtual monty::rc_ptr< ::mosek::fusion::LinearDomain > __mosek_2fusion_2LinearDomain__withShape(std::shared_ptr< monty::ndarray< int,1 > > shp) ;
monty::rc_ptr< ::mosek::fusion::LinearDomain > withShape(std::shared_ptr< monty::ndarray< int,1 > > shp);
}; // class LinearDomain;

class Domain : public virtual monty::RefCounted
{
public: p_Domain * _impl;
protected: Domain(p_Domain * _impl);
public:
Domain(const Domain &) = delete;
const Domain & operator=(const Domain &) = delete;
friend class p_Domain;
virtual ~Domain();
virtual void destroy();
typedef monty::rc_ptr< Domain > t;

static monty::rc_ptr< ::mosek::fusion::SymmetricRangeDomain > symmetric(monty::rc_ptr< ::mosek::fusion::RangeDomain > rd);
static monty::rc_ptr< ::mosek::fusion::SymmetricLinearDomain > symmetric(monty::rc_ptr< ::mosek::fusion::LinearDomain > ld);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse(monty::rc_ptr< ::mosek::fusion::RangeDomain > rd,std::shared_ptr< monty::ndarray< int,2 > > sparsity);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > sparse(monty::rc_ptr< ::mosek::fusion::RangeDomain > rd,std::shared_ptr< monty::ndarray< int,1 > > sparsity);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse(monty::rc_ptr< ::mosek::fusion::LinearDomain > ld,std::shared_ptr< monty::ndarray< int,2 > > sparsity);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > sparse(monty::rc_ptr< ::mosek::fusion::LinearDomain > ld,std::shared_ptr< monty::ndarray< int,1 > > sparsity);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > integral(monty::rc_ptr< ::mosek::fusion::RangeDomain > rd);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > integral(monty::rc_ptr< ::mosek::fusion::LinearDomain > ld);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > integral(monty::rc_ptr< ::mosek::fusion::ConeDomain > c);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > axis(monty::rc_ptr< ::mosek::fusion::ConeDomain > c,int a);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inDPowerCone(double alpha,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inDPowerCone(double alpha,int m);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inDPowerCone(double alpha);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inPPowerCone(double alpha,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inPPowerCone(double alpha,int m);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inPPowerCone(double alpha);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inDExpCone(std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inDExpCone(int m);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inDExpCone();
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inPExpCone(std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inPExpCone(int m);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inPExpCone();
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone(std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone(int m,int n);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone(int n);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inRotatedQCone();
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone(std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone(int m,int n);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone(int n);
static monty::rc_ptr< ::mosek::fusion::ConeDomain > inQCone();
static monty::rc_ptr< ::mosek::fusion::LinPSDDomain > isLinPSD(int n,int m);
static monty::rc_ptr< ::mosek::fusion::LinPSDDomain > isLinPSD(int n);
static monty::rc_ptr< ::mosek::fusion::LinPSDDomain > isLinPSD();
static monty::rc_ptr< ::mosek::fusion::PSDDomain > isTrilPSD(int n,int m);
static monty::rc_ptr< ::mosek::fusion::PSDDomain > isTrilPSD(int n);
static monty::rc_ptr< ::mosek::fusion::PSDDomain > isTrilPSD();
static monty::rc_ptr< ::mosek::fusion::PSDDomain > inPSDCone(int n,int m);
static monty::rc_ptr< ::mosek::fusion::PSDDomain > inPSDCone(int n);
static monty::rc_ptr< ::mosek::fusion::PSDDomain > inPSDCone();
static monty::rc_ptr< ::mosek::fusion::RangeDomain > binary();
static monty::rc_ptr< ::mosek::fusion::RangeDomain > binary(std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > binary(int m,int n);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > binary(int n);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > lba,std::shared_ptr< monty::ndarray< double,1 > > uba,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > lba,double ub,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double lb,std::shared_ptr< monty::ndarray< double,1 > > uba,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double lb,double ub,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > lba,std::shared_ptr< monty::ndarray< double,1 > > uba);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(std::shared_ptr< monty::ndarray< double,1 > > lba,double ub);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double lb,std::shared_ptr< monty::ndarray< double,1 > > uba);
static monty::rc_ptr< ::mosek::fusion::RangeDomain > inRange(double lb,double ub);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(std::shared_ptr< monty::ndarray< double,1 > > a1,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(std::shared_ptr< monty::ndarray< double,2 > > a2);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(std::shared_ptr< monty::ndarray< double,1 > > a1);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double b,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double b,int m,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double b,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > greaterThan(double b);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(std::shared_ptr< monty::ndarray< double,1 > > a1,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(std::shared_ptr< monty::ndarray< double,2 > > a2);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(std::shared_ptr< monty::ndarray< double,1 > > a1);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double b,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double b,int m,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double b,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > lessThan(double b);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(std::shared_ptr< monty::ndarray< double,1 > > a1,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(std::shared_ptr< monty::ndarray< double,2 > > a2);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(std::shared_ptr< monty::ndarray< double,1 > > a1);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double b,std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double b,int m,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double b,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > equalsTo(double b);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded(std::shared_ptr< monty::ndarray< int,1 > > dims);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded(int m,int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded(int n);
static monty::rc_ptr< ::mosek::fusion::LinearDomain > unbounded();
}; // class Domain;

class BaseExpression : public virtual ::mosek::fusion::Expression
{
public: p_BaseExpression * _impl;
protected: BaseExpression(p_BaseExpression * _impl);
public:
BaseExpression(const BaseExpression &) = delete;
const BaseExpression & operator=(const BaseExpression &) = delete;
friend class p_BaseExpression;
virtual ~BaseExpression();
virtual void destroy();
typedef monty::rc_ptr< BaseExpression > t;

BaseExpression(std::shared_ptr< monty::ndarray< int,1 > > shape);
virtual /* override */ std::string toString() ;
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2BaseExpression__eval() ;
monty::rc_ptr< ::mosek::fusion::FlatExpr > eval();
/* override: mosek.fusion.Expression.eval*/
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expression__eval();
static void storeexpr(monty::rc_ptr< ::mosek::fusion::WorkStack > s,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< int,1 > > ptr,std::shared_ptr< monty::ndarray< long long,1 > > sp,std::shared_ptr< monty::ndarray< long long,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > cof,std::shared_ptr< monty::ndarray< double,1 > > bfix);
virtual void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs)  = 0;
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__pick(std::shared_ptr< monty::ndarray< int,2 > > indexrows) ;
monty::rc_ptr< ::mosek::fusion::Expression > pick(std::shared_ptr< monty::ndarray< int,2 > > indexrows);
/* override: mosek.fusion.Expression.pick*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,2 > > indexrows);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__pick(std::shared_ptr< monty::ndarray< int,1 > > indexes) ;
monty::rc_ptr< ::mosek::fusion::Expression > pick(std::shared_ptr< monty::ndarray< int,1 > > indexes);
/* override: mosek.fusion.Expression.pick*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__pick(std::shared_ptr< monty::ndarray< int,1 > > indexes);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__index(std::shared_ptr< monty::ndarray< int,1 > > indexes) ;
monty::rc_ptr< ::mosek::fusion::Expression > index(std::shared_ptr< monty::ndarray< int,1 > > indexes);
/* override: mosek.fusion.Expression.index*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(std::shared_ptr< monty::ndarray< int,1 > > indexes);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__index(int i) ;
monty::rc_ptr< ::mosek::fusion::Expression > index(int i);
/* override: mosek.fusion.Expression.index*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__index(int i);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta) ;
monty::rc_ptr< ::mosek::fusion::Expression > slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta);
/* override: mosek.fusion.Expression.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(std::shared_ptr< monty::ndarray< int,1 > > firsta,std::shared_ptr< monty::ndarray< int,1 > > lasta);
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2BaseExpression__slice(int first,int last) ;
monty::rc_ptr< ::mosek::fusion::Expression > slice(int first,int last);
/* override: mosek.fusion.Expression.slice*/
virtual monty::rc_ptr< ::mosek::fusion::Expression > __mosek_2fusion_2Expression__slice(int first,int last);
virtual long long getSize() ;
virtual int getND() ;
virtual int getDim(int d) ;
virtual std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
}; // class BaseExpression;

class ExprConst : public ::mosek::fusion::BaseExpression
{
protected: ExprConst(p_ExprConst * _impl);
public:
ExprConst(const ExprConst &) = delete;
const ExprConst & operator=(const ExprConst &) = delete;
friend class p_ExprConst;
virtual ~ExprConst();
virtual void destroy();
typedef monty::rc_ptr< ExprConst > t;

ExprConst(std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,std::shared_ptr< monty::ndarray< double,1 > > bfix);
ExprConst(std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > sparsity,double bfix);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprConst;

class ExprPick : public ::mosek::fusion::BaseExpression
{
protected: ExprPick(p_ExprPick * _impl);
public:
ExprPick(const ExprPick &) = delete;
const ExprPick & operator=(const ExprPick &) = delete;
friend class p_ExprPick;
virtual ~ExprPick();
virtual void destroy();
typedef monty::rc_ptr< ExprPick > t;

ExprPick(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< int,2 > > idxs);
ExprPick(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< long long,1 > > idxs);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprPick;

class ExprSlice : public ::mosek::fusion::BaseExpression
{
protected: ExprSlice(p_ExprSlice * _impl);
public:
ExprSlice(const ExprSlice &) = delete;
const ExprSlice & operator=(const ExprSlice &) = delete;
friend class p_ExprSlice;
virtual ~ExprSlice();
virtual void destroy();
typedef monty::rc_ptr< ExprSlice > t;

ExprSlice(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< int,1 > > first,std::shared_ptr< monty::ndarray< int,1 > > last);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprSlice;

class ExprPermuteDims : public ::mosek::fusion::BaseExpression
{
ExprPermuteDims(std::shared_ptr< monty::ndarray< int,1 > > perm,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprPermuteDims(p_ExprPermuteDims * _impl);
public:
ExprPermuteDims(const ExprPermuteDims &) = delete;
const ExprPermuteDims & operator=(const ExprPermuteDims &) = delete;
friend class p_ExprPermuteDims;
virtual ~ExprPermuteDims();
virtual void destroy();
typedef monty::rc_ptr< ExprPermuteDims > t;

ExprPermuteDims(std::shared_ptr< monty::ndarray< int,1 > > perm,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
}; // class ExprPermuteDims;

class ExprTranspose : public ::mosek::fusion::BaseExpression
{
protected: ExprTranspose(p_ExprTranspose * _impl);
public:
ExprTranspose(const ExprTranspose &) = delete;
const ExprTranspose & operator=(const ExprTranspose &) = delete;
friend class p_ExprTranspose;
virtual ~ExprTranspose();
virtual void destroy();
typedef monty::rc_ptr< ExprTranspose > t;

ExprTranspose(monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprTranspose;

class ExprStack : public ::mosek::fusion::BaseExpression
{
protected: ExprStack(p_ExprStack * _impl);
public:
ExprStack(const ExprStack &) = delete;
const ExprStack & operator=(const ExprStack &) = delete;
friend class p_ExprStack;
virtual ~ExprStack();
virtual void destroy();
typedef monty::rc_ptr< ExprStack > t;

ExprStack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > exprs,int dim);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprStack;

class ExprInner : public ::mosek::fusion::BaseExpression
{
protected: ExprInner(p_ExprInner * _impl);
public:
ExprInner(const ExprInner &) = delete;
const ExprInner & operator=(const ExprInner &) = delete;
friend class p_ExprInner;
virtual ~ExprInner();
virtual void destroy();
typedef monty::rc_ptr< ExprInner > t;

ExprInner(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< long long,1 > > vsub,std::shared_ptr< monty::ndarray< double,1 > > vcof);
ExprInner(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< double,1 > > vcof);
ExprInner(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< int,2 > > vsub,std::shared_ptr< monty::ndarray< double,1 > > vcof);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprInner;

class ExprMulDiagRight : public ::mosek::fusion::BaseExpression
{
ExprMulDiagRight(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprMulDiagRight(p_ExprMulDiagRight * _impl);
public:
ExprMulDiagRight(const ExprMulDiagRight &) = delete;
const ExprMulDiagRight & operator=(const ExprMulDiagRight &) = delete;
friend class p_ExprMulDiagRight;
virtual ~ExprMulDiagRight();
virtual void destroy();
typedef monty::rc_ptr< ExprMulDiagRight > t;

ExprMulDiagRight(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulDiagRight;

class ExprMulDiagLeft : public ::mosek::fusion::BaseExpression
{
ExprMulDiagLeft(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprMulDiagLeft(p_ExprMulDiagLeft * _impl);
public:
ExprMulDiagLeft(const ExprMulDiagLeft &) = delete;
const ExprMulDiagLeft & operator=(const ExprMulDiagLeft &) = delete;
friend class p_ExprMulDiagLeft;
virtual ~ExprMulDiagLeft();
virtual void destroy();
typedef monty::rc_ptr< ExprMulDiagLeft > t;

ExprMulDiagLeft(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulDiagLeft;

class ExprMulElement : public ::mosek::fusion::BaseExpression
{
ExprMulElement(std::shared_ptr< monty::ndarray< double,1 > > mcof,std::shared_ptr< monty::ndarray< long long,1 > > msp,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprMulElement(p_ExprMulElement * _impl);
public:
ExprMulElement(const ExprMulElement &) = delete;
const ExprMulElement & operator=(const ExprMulElement &) = delete;
friend class p_ExprMulElement;
virtual ~ExprMulElement();
virtual void destroy();
typedef monty::rc_ptr< ExprMulElement > t;

ExprMulElement(std::shared_ptr< monty::ndarray< double,1 > > mcof,std::shared_ptr< monty::ndarray< long long,1 > > msp,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulElement;

class ExprMulScalarConst : public ::mosek::fusion::BaseExpression
{
protected: ExprMulScalarConst(p_ExprMulScalarConst * _impl);
public:
ExprMulScalarConst(const ExprMulScalarConst &) = delete;
const ExprMulScalarConst & operator=(const ExprMulScalarConst &) = delete;
friend class p_ExprMulScalarConst;
virtual ~ExprMulScalarConst();
virtual void destroy();
typedef monty::rc_ptr< ExprMulScalarConst > t;

ExprMulScalarConst(double c,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulScalarConst;

class ExprScalarMul : public ::mosek::fusion::BaseExpression
{
ExprScalarMul(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprScalarMul(p_ExprScalarMul * _impl);
public:
ExprScalarMul(const ExprScalarMul &) = delete;
const ExprScalarMul & operator=(const ExprScalarMul &) = delete;
friend class p_ExprScalarMul;
virtual ~ExprScalarMul();
virtual void destroy();
typedef monty::rc_ptr< ExprScalarMul > t;

ExprScalarMul(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprScalarMul;

class ExprMulRight : public ::mosek::fusion::BaseExpression
{
ExprMulRight(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprMulRight(p_ExprMulRight * _impl);
public:
ExprMulRight(const ExprMulRight &) = delete;
const ExprMulRight & operator=(const ExprMulRight &) = delete;
friend class p_ExprMulRight;
virtual ~ExprMulRight();
virtual void destroy();
typedef monty::rc_ptr< ExprMulRight > t;

ExprMulRight(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulRight;

class ExprMulLeft : public ::mosek::fusion::BaseExpression
{
ExprMulLeft(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr,int validated);
protected: ExprMulLeft(p_ExprMulLeft * _impl);
public:
ExprMulLeft(const ExprMulLeft &) = delete;
const ExprMulLeft & operator=(const ExprMulLeft &) = delete;
friend class p_ExprMulLeft;
virtual ~ExprMulLeft();
virtual void destroy();
typedef monty::rc_ptr< ExprMulLeft > t;

ExprMulLeft(int mdim0,int mdim1,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mval,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulLeft;

class ExprMulVar : public ::mosek::fusion::BaseExpression
{
ExprMulVar(bool left,int mdimi,int mdimj,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mcof,monty::rc_ptr< ::mosek::fusion::Variable > x,int unchecked_);
protected: ExprMulVar(p_ExprMulVar * _impl);
public:
ExprMulVar(const ExprMulVar &) = delete;
const ExprMulVar & operator=(const ExprMulVar &) = delete;
friend class p_ExprMulVar;
virtual ~ExprMulVar();
virtual void destroy();
typedef monty::rc_ptr< ExprMulVar > t;

ExprMulVar(bool left,int mdimi,int mdimj,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mcof,monty::rc_ptr< ::mosek::fusion::Variable > x);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual void eval_right(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual void eval_left(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulVar;

class ExprMulScalarVar : public ::mosek::fusion::BaseExpression
{
ExprMulScalarVar(int mdimi,int mdimj,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mcof,monty::rc_ptr< ::mosek::fusion::Variable > x,int unchecked_);
protected: ExprMulScalarVar(p_ExprMulScalarVar * _impl);
public:
ExprMulScalarVar(const ExprMulScalarVar &) = delete;
const ExprMulScalarVar & operator=(const ExprMulScalarVar &) = delete;
friend class p_ExprMulScalarVar;
virtual ~ExprMulScalarVar();
virtual void destroy();
typedef monty::rc_ptr< ExprMulScalarVar > t;

ExprMulScalarVar(int mdimi,int mdimj,std::shared_ptr< monty::ndarray< int,1 > > msubi,std::shared_ptr< monty::ndarray< int,1 > > msubj,std::shared_ptr< monty::ndarray< double,1 > > mcof,monty::rc_ptr< ::mosek::fusion::Variable > x);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulScalarVar;

class ExprMulVarScalarConst : public ::mosek::fusion::BaseExpression
{
protected: ExprMulVarScalarConst(p_ExprMulVarScalarConst * _impl);
public:
ExprMulVarScalarConst(const ExprMulVarScalarConst &) = delete;
const ExprMulVarScalarConst & operator=(const ExprMulVarScalarConst &) = delete;
friend class p_ExprMulVarScalarConst;
virtual ~ExprMulVarScalarConst();
virtual void destroy();
typedef monty::rc_ptr< ExprMulVarScalarConst > t;

ExprMulVarScalarConst(monty::rc_ptr< ::mosek::fusion::Variable > x,double c);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprMulVarScalarConst;

class ExprAdd : public ::mosek::fusion::BaseExpression
{
protected: ExprAdd(p_ExprAdd * _impl);
public:
ExprAdd(const ExprAdd &) = delete;
const ExprAdd & operator=(const ExprAdd &) = delete;
friend class p_ExprAdd;
virtual ~ExprAdd();
virtual void destroy();
typedef monty::rc_ptr< ExprAdd > t;

ExprAdd(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double m1,double m2);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprAdd;

class ExprWSum : public ::mosek::fusion::BaseExpression
{
protected: ExprWSum(p_ExprWSum * _impl);
public:
ExprWSum(const ExprWSum &) = delete;
const ExprWSum & operator=(const ExprWSum &) = delete;
friend class p_ExprWSum;
virtual ~ExprWSum();
virtual void destroy();
typedef monty::rc_ptr< ExprWSum > t;

ExprWSum(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > es,std::shared_ptr< monty::ndarray< double,1 > > w);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprWSum;

class ExprSumReduce : public ::mosek::fusion::BaseExpression
{
protected: ExprSumReduce(p_ExprSumReduce * _impl);
public:
ExprSumReduce(const ExprSumReduce &) = delete;
const ExprSumReduce & operator=(const ExprSumReduce &) = delete;
friend class p_ExprSumReduce;
virtual ~ExprSumReduce();
virtual void destroy();
typedef monty::rc_ptr< ExprSumReduce > t;

ExprSumReduce(int dim,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprSumReduce;

class ExprDenseTril : public ::mosek::fusion::BaseExpression
{
protected: ExprDenseTril(p_ExprDenseTril * _impl);
public:
ExprDenseTril(const ExprDenseTril &) = delete;
const ExprDenseTril & operator=(const ExprDenseTril &) = delete;
friend class p_ExprDenseTril;
virtual ~ExprDenseTril();
virtual void destroy();
typedef monty::rc_ptr< ExprDenseTril > t;

ExprDenseTril(int dim0,int dim1,monty::rc_ptr< ::mosek::fusion::Expression > expr,int unchecked_);
ExprDenseTril(int dim0_,int dim1_,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
}; // class ExprDenseTril;

class ExprDense : public ::mosek::fusion::BaseExpression
{
protected: ExprDense(p_ExprDense * _impl);
public:
ExprDense(const ExprDense &) = delete;
const ExprDense & operator=(const ExprDense &) = delete;
friend class p_ExprDense;
virtual ~ExprDense();
virtual void destroy();
typedef monty::rc_ptr< ExprDense > t;

ExprDense(monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprDense;

class ExprSymmetrize : public ::mosek::fusion::BaseExpression
{
protected: ExprSymmetrize(p_ExprSymmetrize * _impl);
public:
ExprSymmetrize(const ExprSymmetrize &) = delete;
const ExprSymmetrize & operator=(const ExprSymmetrize &) = delete;
friend class p_ExprSymmetrize;
virtual ~ExprSymmetrize();
virtual void destroy();
typedef monty::rc_ptr< ExprSymmetrize > t;

ExprSymmetrize(int dim0,int dim1,monty::rc_ptr< ::mosek::fusion::Expression > expr,int unchecked_);
ExprSymmetrize(int dim0_,int dim1_,monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
}; // class ExprSymmetrize;

class ExprCompress : public ::mosek::fusion::BaseExpression
{
protected: ExprCompress(p_ExprCompress * _impl);
public:
ExprCompress(const ExprCompress &) = delete;
const ExprCompress & operator=(const ExprCompress &) = delete;
friend class p_ExprCompress;
virtual ~ExprCompress();
virtual void destroy();
typedef monty::rc_ptr< ExprCompress > t;

ExprCompress(monty::rc_ptr< ::mosek::fusion::Expression > expr);
ExprCompress(monty::rc_ptr< ::mosek::fusion::Expression > expr,double epsilon);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
static void arg_sort(monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs,int perm,int nelem,int nnz,int ptr,int nidxs);
static void merge_sort(int origperm1,int origperm2,int nelem,int nnz,int ptr_base,int nidxs_base,std::shared_ptr< monty::ndarray< int,1 > > wi32,std::shared_ptr< monty::ndarray< long long,1 > > wi64);
virtual /* override */ std::string toString() ;
}; // class ExprCompress;

class ExprCondense : public ::mosek::fusion::BaseExpression
{
protected: ExprCondense(p_ExprCondense * _impl);
public:
ExprCondense(const ExprCondense &) = delete;
const ExprCondense & operator=(const ExprCondense &) = delete;
friend class p_ExprCondense;
virtual ~ExprCondense();
virtual void destroy();
typedef monty::rc_ptr< ExprCondense > t;

ExprCondense(monty::rc_ptr< ::mosek::fusion::Expression > expr);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
}; // class ExprCondense;

class ExprFromVar : public ::mosek::fusion::BaseExpression
{
protected: ExprFromVar(p_ExprFromVar * _impl);
public:
ExprFromVar(const ExprFromVar &) = delete;
const ExprFromVar & operator=(const ExprFromVar &) = delete;
friend class p_ExprFromVar;
virtual ~ExprFromVar();
virtual void destroy();
typedef monty::rc_ptr< ExprFromVar > t;

ExprFromVar(monty::rc_ptr< ::mosek::fusion::Variable > x);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprFromVar;

class ExprReshape : public ::mosek::fusion::BaseExpression
{
protected: ExprReshape(p_ExprReshape * _impl);
public:
ExprReshape(const ExprReshape &) = delete;
const ExprReshape & operator=(const ExprReshape &) = delete;
friend class p_ExprReshape;
virtual ~ExprReshape();
virtual void destroy();
typedef monty::rc_ptr< ExprReshape > t;

ExprReshape(std::shared_ptr< monty::ndarray< int,1 > > shape,monty::rc_ptr< ::mosek::fusion::Expression > e);
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
virtual /* override */ std::string toString() ;
}; // class ExprReshape;

class Expr : public ::mosek::fusion::BaseExpression
{
Expr(std::shared_ptr< monty::ndarray< long long,1 > > ptrb,std::shared_ptr< monty::ndarray< long long,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > cof,std::shared_ptr< monty::ndarray< double,1 > > bfix,std::shared_ptr< monty::ndarray< int,1 > > shp,std::shared_ptr< monty::ndarray< long long,1 > > inst,int unchecked_);
Expr(monty::rc_ptr< ::mosek::fusion::Expression > e);
protected: Expr(p_Expr * _impl);
public:
Expr(const Expr &) = delete;
const Expr & operator=(const Expr &) = delete;
friend class p_Expr;
virtual ~Expr();
virtual void destroy();
typedef monty::rc_ptr< Expr > t;

Expr(std::shared_ptr< monty::ndarray< long long,1 > > ptrb,std::shared_ptr< monty::ndarray< long long,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > cof,std::shared_ptr< monty::ndarray< double,1 > > bfix,std::shared_ptr< monty::ndarray< int,1 > > shape,std::shared_ptr< monty::ndarray< long long,1 > > inst);
static monty::rc_ptr< ::mosek::fusion::Expression > condense(monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > flatten(monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > reshape(monty::rc_ptr< ::mosek::fusion::Expression > e,int dimi,int dimj);
static monty::rc_ptr< ::mosek::fusion::Expression > reshape(monty::rc_ptr< ::mosek::fusion::Expression > e,int size);
static monty::rc_ptr< ::mosek::fusion::Expression > reshape(monty::rc_ptr< ::mosek::fusion::Expression > e,std::shared_ptr< monty::ndarray< int,1 > > newshape);
virtual long long size() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2Expr__eval() ;
monty::rc_ptr< ::mosek::fusion::FlatExpr > eval();
/* override: mosek.fusion.BaseExpression.eval*/
virtual monty::rc_ptr< ::mosek::fusion::FlatExpr > __mosek_2fusion_2BaseExpression__eval();
static monty::rc_ptr< ::mosek::fusion::Expression > zeros(std::shared_ptr< monty::ndarray< int,1 > > shp);
static monty::rc_ptr< ::mosek::fusion::Expression > zeros(int size);
static monty::rc_ptr< ::mosek::fusion::Expression > ones();
static monty::rc_ptr< ::mosek::fusion::Expression > ones(std::shared_ptr< monty::ndarray< int,1 > > shp,std::shared_ptr< monty::ndarray< int,2 > > sparsity);
static monty::rc_ptr< ::mosek::fusion::Expression > ones(std::shared_ptr< monty::ndarray< int,1 > > shp);
static monty::rc_ptr< ::mosek::fusion::Expression > ones(int size);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(monty::rc_ptr< ::mosek::fusion::NDSparseArray > nda);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(double val);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< int,1 > > shp,std::shared_ptr< monty::ndarray< int,2 > > sparsity,double val);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< int,1 > > shp,std::shared_ptr< monty::ndarray< int,2 > > sparsity,std::shared_ptr< monty::ndarray< double,1 > > val);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< int,1 > > shp,double val);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(int size,double val);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< double,2 > > vals2);
static monty::rc_ptr< ::mosek::fusion::Expression > constTerm(std::shared_ptr< monty::ndarray< double,1 > > vals1);
virtual long long numNonzeros() ;
static monty::rc_ptr< ::mosek::fusion::Expression > sum(monty::rc_ptr< ::mosek::fusion::Expression > expr,int dim);
static monty::rc_ptr< ::mosek::fusion::Expression > sum(monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > neg(monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Variable > v,monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Matrix > mx,monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Matrix > mx,monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Variable > v,std::shared_ptr< monty::ndarray< double,2 > > a);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< double,2 > > a);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(std::shared_ptr< monty::ndarray< double,2 > > a,monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Expression > mulDiag(std::shared_ptr< monty::ndarray< double,2 > > a,monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > e,double c);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(double c,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > e,std::shared_ptr< monty::ndarray< double,1 > > a);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(std::shared_ptr< monty::ndarray< double,1 > > a,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > e,std::shared_ptr< monty::ndarray< double,2 > > a);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(std::shared_ptr< monty::ndarray< double,2 > > a,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Expression > e,monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Matrix > mx,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Variable > v,monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::Expression > mul(monty::rc_ptr< ::mosek::fusion::Matrix > mx,monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > e,monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > e,std::shared_ptr< monty::ndarray< double,2 > > c2);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > e,monty::rc_ptr< ::mosek::fusion::NDSparseArray > nda);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Expression > e,std::shared_ptr< monty::ndarray< double,1 > > c1);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::Matrix > m,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(monty::rc_ptr< ::mosek::fusion::NDSparseArray > nda,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(std::shared_ptr< monty::ndarray< double,2 > > c2,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > dot(std::shared_ptr< monty::ndarray< double,1 > > c1,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > outer(std::shared_ptr< monty::ndarray< double,1 > > a,monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Expression > e,std::shared_ptr< monty::ndarray< double,1 > > a);
static monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Matrix > m,monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Variable > v,monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::Expression > outer(std::shared_ptr< monty::ndarray< double,1 > > a,monty::rc_ptr< ::mosek::fusion::Variable > v);
static monty::rc_ptr< ::mosek::fusion::Expression > outer(monty::rc_ptr< ::mosek::fusion::Variable > v,std::shared_ptr< monty::ndarray< double,1 > > a);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > >,1 > > exprs);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(double a1,double a2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(double a1,double a2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > vstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > exprs);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(double a1,double a2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > hstack(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > exprs);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2,monty::rc_ptr< ::mosek::fusion::Expression > e3);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2,double a3);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,double a1,double a2,monty::rc_ptr< ::mosek::fusion::Expression > e1);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,double a1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,monty::rc_ptr< ::mosek::fusion::Expression > e1,double a2);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > stack(int dim,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > exprs);
static monty::rc_ptr< ::mosek::fusion::Expression > repeat(monty::rc_ptr< ::mosek::fusion::Expression > e,int n,int d);
static monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Expression >,1 > > exps);
static monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > vs);
static monty::rc_ptr< ::mosek::fusion::Expression > transpose(monty::rc_ptr< ::mosek::fusion::Expression > e);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Matrix > m,monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::NDSparseArray > spm,monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(std::shared_ptr< monty::ndarray< double,2 > > a2,monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(std::shared_ptr< monty::ndarray< double,1 > > a1,monty::rc_ptr< ::mosek::fusion::Expression > expr);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< double,2 > > a2);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > expr,std::shared_ptr< monty::ndarray< double,1 > > a1);
static monty::rc_ptr< ::mosek::fusion::Expression > mulElm(monty::rc_ptr< ::mosek::fusion::Expression > expr,monty::rc_ptr< ::mosek::fusion::NDSparseArray > spm);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::NDSparseArray > n,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::NDSparseArray > n);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Matrix > m,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(double c,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > e1,double c);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(std::shared_ptr< monty::ndarray< double,2 > > a2,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(std::shared_ptr< monty::ndarray< double,1 > > a1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > e1,std::shared_ptr< monty::ndarray< double,2 > > a2);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > e1,std::shared_ptr< monty::ndarray< double,1 > > a1);
static monty::rc_ptr< ::mosek::fusion::Expression > sub(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::NDSparseArray > n,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::NDSparseArray > n);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Matrix > m,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::Expression > add(double c,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > e1,double c);
static monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< double,2 > > a2,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(std::shared_ptr< monty::ndarray< double,1 > > a1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > e1,std::shared_ptr< monty::ndarray< double,2 > > a2);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > e1,std::shared_ptr< monty::ndarray< double,1 > > a1);
static monty::rc_ptr< ::mosek::fusion::Expression > add(monty::rc_ptr< ::mosek::fusion::Expression > e1,monty::rc_ptr< ::mosek::fusion::Expression > e2);
virtual /* override */ int getND() ;
virtual /* override */ std::shared_ptr< monty::ndarray< int,1 > > getShape() ;
virtual /* override */ void eval(monty::rc_ptr< ::mosek::fusion::WorkStack > rs,monty::rc_ptr< ::mosek::fusion::WorkStack > ws,monty::rc_ptr< ::mosek::fusion::WorkStack > xs) ;
}; // class Expr;

class WorkStack : public virtual monty::RefCounted
{
public: p_WorkStack * _impl;
protected: WorkStack(p_WorkStack * _impl);
public:
WorkStack(const WorkStack &) = delete;
const WorkStack & operator=(const WorkStack &) = delete;
friend class p_WorkStack;
virtual ~WorkStack();
virtual void destroy();
typedef monty::rc_ptr< WorkStack > t;
int get_cof_base();
void set_cof_base(int val);
int get_bfix_base();
void set_bfix_base(int val);
int get_nidxs_base();
void set_nidxs_base(int val);
int get_sp_base();
void set_sp_base(int val);
int get_shape_base();
void set_shape_base(int val);
int get_ptr_base();
void set_ptr_base(int val);
bool get_hassp();
void set_hassp(bool val);
int get_nelem();
void set_nelem(int val);
int get_nnz();
void set_nnz(int val);
int get_nd();
void set_nd(int val);
int get_pf64();
void set_pf64(int val);
int get_pi64();
void set_pi64(int val);
int get_pi32();
void set_pi32(int val);
std::shared_ptr< monty::ndarray< double,1 > > get_f64();
void set_f64(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< long long,1 > > get_i64();
void set_i64(std::shared_ptr< monty::ndarray< long long,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_i32();
void set_i32(std::shared_ptr< monty::ndarray< int,1 > > val);

WorkStack();
virtual bool peek_hassp() ;
virtual int peek_nnz() ;
virtual int peek_nelem() ;
virtual int peek_dim(int i) ;
virtual int peek_nd() ;
virtual void alloc_expr(int nd,int nelem,int nnz,bool hassp) ;
virtual void move_expr(monty::rc_ptr< ::mosek::fusion::WorkStack > to) ;
virtual void peek_expr() ;
virtual void pop_expr() ;
virtual void ensure_sparsity() ;
virtual void clear() ;
virtual int allocf64(int n) ;
virtual int alloci64(int n) ;
virtual int alloci32(int n) ;
virtual void pushf64(double v) ;
virtual void pushi64(long long v) ;
virtual void pushi32(int v) ;
virtual void ensuref64(int n) ;
virtual void ensurei64(int n) ;
virtual void ensurei32(int n) ;
virtual int popf64(int n) ;
virtual int popi64(int n) ;
virtual int popi32(int n) ;
virtual void popf64(int n,std::shared_ptr< monty::ndarray< double,1 > > r,int ofs) ;
virtual void popi64(int n,std::shared_ptr< monty::ndarray< long long,1 > > r,int ofs) ;
virtual void popi32(int n,std::shared_ptr< monty::ndarray< int,1 > > r,int ofs) ;
virtual double popf64() ;
virtual long long popi64() ;
virtual int popi32() ;
virtual double peekf64() ;
virtual long long peeki64() ;
virtual int peeki32() ;
virtual double peekf64(int i) ;
virtual long long peeki64(int i) ;
virtual int peeki32(int i) ;
}; // class WorkStack;

class SymmetricExpr : public virtual monty::RefCounted
{
public: p_SymmetricExpr * _impl;
SymmetricExpr(int n,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::SymmetricMatrix >,1 > > Ms,std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Variable >,1 > > xs,monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > b);
protected: SymmetricExpr(p_SymmetricExpr * _impl);
public:
SymmetricExpr(const SymmetricExpr &) = delete;
const SymmetricExpr & operator=(const SymmetricExpr &) = delete;
friend class p_SymmetricExpr;
virtual ~SymmetricExpr();
virtual void destroy();
typedef monty::rc_ptr< SymmetricExpr > t;

virtual /* override */ std::string toString() ;
}; // class SymmetricExpr;

class FlatExpr : public virtual monty::RefCounted
{
public: p_FlatExpr * _impl;
protected: FlatExpr(p_FlatExpr * _impl);
public:
FlatExpr(const FlatExpr &) = delete;
const FlatExpr & operator=(const FlatExpr &) = delete;
friend class p_FlatExpr;
virtual ~FlatExpr();
virtual void destroy();
typedef monty::rc_ptr< FlatExpr > t;
std::shared_ptr< monty::ndarray< long long,1 > > get_inst();
void set_inst(std::shared_ptr< monty::ndarray< long long,1 > > val);
std::shared_ptr< monty::ndarray< int,1 > > get_shape();
void set_shape(std::shared_ptr< monty::ndarray< int,1 > > val);
long long get_nnz();
void set_nnz(long long val);
std::shared_ptr< monty::ndarray< double,1 > > get_cof();
void set_cof(std::shared_ptr< monty::ndarray< double,1 > > val);
std::shared_ptr< monty::ndarray< long long,1 > > get_subj();
void set_subj(std::shared_ptr< monty::ndarray< long long,1 > > val);
std::shared_ptr< monty::ndarray< long long,1 > > get_ptrb();
void set_ptrb(std::shared_ptr< monty::ndarray< long long,1 > > val);
std::shared_ptr< monty::ndarray< double,1 > > get_bfix();
void set_bfix(std::shared_ptr< monty::ndarray< double,1 > > val);

FlatExpr(monty::rc_ptr< ::mosek::fusion::FlatExpr > e);
FlatExpr(std::shared_ptr< monty::ndarray< double,1 > > bfix_,std::shared_ptr< monty::ndarray< long long,1 > > ptrb_,std::shared_ptr< monty::ndarray< long long,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > cof_,std::shared_ptr< monty::ndarray< int,1 > > shape_,std::shared_ptr< monty::ndarray< long long,1 > > inst_);
virtual /* override */ std::string toString() ;
virtual int size() ;
}; // class FlatExpr;

class SymmetricMatrix : public virtual monty::RefCounted
{
public: p_SymmetricMatrix * _impl;
SymmetricMatrix(int dim0,int dim1,std::shared_ptr< monty::ndarray< int,1 > > usubi,std::shared_ptr< monty::ndarray< int,1 > > usubj,std::shared_ptr< monty::ndarray< double,1 > > uval,std::shared_ptr< monty::ndarray< int,1 > > vsubi,std::shared_ptr< monty::ndarray< int,1 > > vsubj,std::shared_ptr< monty::ndarray< double,1 > > vval,double scale);
protected: SymmetricMatrix(p_SymmetricMatrix * _impl);
public:
SymmetricMatrix(const SymmetricMatrix &) = delete;
const SymmetricMatrix & operator=(const SymmetricMatrix &) = delete;
friend class p_SymmetricMatrix;
virtual ~SymmetricMatrix();
virtual void destroy();
typedef monty::rc_ptr< SymmetricMatrix > t;

static monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > rankOne(int n,std::shared_ptr< monty::ndarray< int,1 > > sub,std::shared_ptr< monty::ndarray< double,1 > > v);
static monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > rankOne(std::shared_ptr< monty::ndarray< double,1 > > v);
static monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > antiDiag(std::shared_ptr< monty::ndarray< double,1 > > vals);
static monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > diag(std::shared_ptr< monty::ndarray< double,1 > > vals);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > __mosek_2fusion_2SymmetricMatrix__add(monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > m) ;
monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > add(monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > m);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > __mosek_2fusion_2SymmetricMatrix__sub(monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > m) ;
monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > sub(monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > m);
virtual monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > __mosek_2fusion_2SymmetricMatrix__mul(double v) ;
monty::rc_ptr< ::mosek::fusion::SymmetricMatrix > mul(double v);
virtual int getdim() ;
}; // class SymmetricMatrix;

class NDSparseArray : public virtual monty::RefCounted
{
public: p_NDSparseArray * _impl;
NDSparseArray(std::shared_ptr< monty::ndarray< int,1 > > dims_,std::shared_ptr< monty::ndarray< int,2 > > sub,std::shared_ptr< monty::ndarray< double,1 > > cof_);
NDSparseArray(std::shared_ptr< monty::ndarray< int,1 > > dims_,std::shared_ptr< monty::ndarray< long long,1 > > inst_,std::shared_ptr< monty::ndarray< double,1 > > cof_);
NDSparseArray(monty::rc_ptr< ::mosek::fusion::Matrix > m);
protected: NDSparseArray(p_NDSparseArray * _impl);
public:
NDSparseArray(const NDSparseArray &) = delete;
const NDSparseArray & operator=(const NDSparseArray &) = delete;
friend class p_NDSparseArray;
virtual ~NDSparseArray();
virtual void destroy();
typedef monty::rc_ptr< NDSparseArray > t;

static monty::rc_ptr< ::mosek::fusion::NDSparseArray > make(monty::rc_ptr< ::mosek::fusion::Matrix > m);
static monty::rc_ptr< ::mosek::fusion::NDSparseArray > make(std::shared_ptr< monty::ndarray< int,1 > > dims,std::shared_ptr< monty::ndarray< long long,1 > > inst,std::shared_ptr< monty::ndarray< double,1 > > cof);
static monty::rc_ptr< ::mosek::fusion::NDSparseArray > make(std::shared_ptr< monty::ndarray< int,1 > > dims,std::shared_ptr< monty::ndarray< int,2 > > sub,std::shared_ptr< monty::ndarray< double,1 > > cof);
}; // class NDSparseArray;

class Matrix : public virtual monty::RefCounted
{
public: p_Matrix * _impl;
protected: Matrix(p_Matrix * _impl);
public:
Matrix(const Matrix &) = delete;
const Matrix & operator=(const Matrix &) = delete;
friend class p_Matrix;
virtual ~Matrix();
virtual void destroy();
typedef monty::rc_ptr< Matrix > t;

virtual /* override */ std::string toString() ;
static monty::rc_ptr< ::mosek::fusion::Matrix > diag(int num,monty::rc_ptr< ::mosek::fusion::Matrix > mv);
static monty::rc_ptr< ::mosek::fusion::Matrix > diag(std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Matrix >,1 > > md);
static monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(int n,double val,int k);
static monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(int n,double val);
static monty::rc_ptr< ::mosek::fusion::Matrix > diag(int n,double val,int k);
static monty::rc_ptr< ::mosek::fusion::Matrix > diag(int n,double val);
static monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(std::shared_ptr< monty::ndarray< double,1 > > d,int k);
static monty::rc_ptr< ::mosek::fusion::Matrix > antidiag(std::shared_ptr< monty::ndarray< double,1 > > d);
static monty::rc_ptr< ::mosek::fusion::Matrix > diag(std::shared_ptr< monty::ndarray< double,1 > > d,int k);
static monty::rc_ptr< ::mosek::fusion::Matrix > diag(std::shared_ptr< monty::ndarray< double,1 > > d);
static monty::rc_ptr< ::mosek::fusion::Matrix > ones(int n,int m);
static monty::rc_ptr< ::mosek::fusion::Matrix > eye(int n);
static monty::rc_ptr< ::mosek::fusion::Matrix > dense(monty::rc_ptr< ::mosek::fusion::Matrix > other);
static monty::rc_ptr< ::mosek::fusion::Matrix > dense(int dimi,int dimj,double value);
static monty::rc_ptr< ::mosek::fusion::Matrix > dense(int dimi,int dimj,std::shared_ptr< monty::ndarray< double,1 > > data);
static monty::rc_ptr< ::mosek::fusion::Matrix > dense(std::shared_ptr< monty::ndarray< double,2 > > data);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(monty::rc_ptr< ::mosek::fusion::Matrix > mx);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< std::shared_ptr< monty::ndarray< monty::rc_ptr< ::mosek::fusion::Matrix >,1 > >,1 > > blocks);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< double,2 > > data);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(int nrow,int ncol);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(int nrow,int ncol,std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,double val);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,double val);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > val);
static monty::rc_ptr< ::mosek::fusion::Matrix > sparse(int nrow,int ncol,std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > val);
virtual double get(int i,int j)  = 0;
virtual bool isSparse()  = 0;
virtual std::shared_ptr< monty::ndarray< double,1 > > getDataAsArray()  = 0;
virtual void getDataAsTriplets(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > val)  = 0;
virtual monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2Matrix__transpose()  = 0;
monty::rc_ptr< ::mosek::fusion::Matrix > transpose();
virtual long long numNonzeros()  = 0;
virtual int numColumns() ;
virtual int numRows() ;
}; // class Matrix;

class DenseMatrix : public ::mosek::fusion::Matrix
{
DenseMatrix(int dimi_,int dimj_,std::shared_ptr< monty::ndarray< double,1 > > cof);
DenseMatrix(monty::rc_ptr< ::mosek::fusion::Matrix > m_);
DenseMatrix(std::shared_ptr< monty::ndarray< double,2 > > d);
DenseMatrix(int dimi_,int dimj_,double value_);
protected: DenseMatrix(p_DenseMatrix * _impl);
public:
DenseMatrix(const DenseMatrix &) = delete;
const DenseMatrix & operator=(const DenseMatrix &) = delete;
friend class p_DenseMatrix;
virtual ~DenseMatrix();
virtual void destroy();
typedef monty::rc_ptr< DenseMatrix > t;

virtual /* override */ std::string toString() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2DenseMatrix__transpose() ;
monty::rc_ptr< ::mosek::fusion::Matrix > transpose();
/* override: mosek.fusion.Matrix.transpose*/
virtual monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2Matrix__transpose();
virtual /* override */ bool isSparse() ;
virtual /* override */ std::shared_ptr< monty::ndarray< double,1 > > getDataAsArray() ;
virtual /* override */ void getDataAsTriplets(std::shared_ptr< monty::ndarray< int,1 > > subi,std::shared_ptr< monty::ndarray< int,1 > > subj,std::shared_ptr< monty::ndarray< double,1 > > cof) ;
virtual /* override */ double get(int i,int j) ;
virtual /* override */ long long numNonzeros() ;
}; // class DenseMatrix;

class SparseMatrix : public ::mosek::fusion::Matrix
{
SparseMatrix(int dimi_,int dimj_,std::shared_ptr< monty::ndarray< int,1 > > subi_,std::shared_ptr< monty::ndarray< int,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > val_,long long nelm);
SparseMatrix(int dimi_,int dimj_,std::shared_ptr< monty::ndarray< int,1 > > subi_,std::shared_ptr< monty::ndarray< int,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > val_);
protected: SparseMatrix(p_SparseMatrix * _impl);
public:
SparseMatrix(const SparseMatrix &) = delete;
const SparseMatrix & operator=(const SparseMatrix &) = delete;
friend class p_SparseMatrix;
virtual ~SparseMatrix();
virtual void destroy();
typedef monty::rc_ptr< SparseMatrix > t;

virtual /* override */ std::string toString() ;
virtual /* override */ long long numNonzeros() ;
virtual /* override */ monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2SparseMatrix__transpose() ;
monty::rc_ptr< ::mosek::fusion::Matrix > transpose();
/* override: mosek.fusion.Matrix.transpose*/
virtual monty::rc_ptr< ::mosek::fusion::Matrix > __mosek_2fusion_2Matrix__transpose();
virtual /* override */ bool isSparse() ;
virtual /* override */ std::shared_ptr< monty::ndarray< double,1 > > getDataAsArray() ;
virtual /* override */ void getDataAsTriplets(std::shared_ptr< monty::ndarray< int,1 > > subi_,std::shared_ptr< monty::ndarray< int,1 > > subj_,std::shared_ptr< monty::ndarray< double,1 > > cof_) ;
virtual /* override */ double get(int i,int j) ;
}; // class SparseMatrix;

class LinkedBlocks : public virtual monty::RefCounted
{
public: p_LinkedBlocks * _impl;
protected: LinkedBlocks(p_LinkedBlocks * _impl);
public:
LinkedBlocks(const LinkedBlocks &) = delete;
const LinkedBlocks & operator=(const LinkedBlocks &) = delete;
friend class p_LinkedBlocks;
virtual ~LinkedBlocks();
virtual void destroy();
typedef monty::rc_ptr< LinkedBlocks > t;

LinkedBlocks();
LinkedBlocks(int n);
LinkedBlocks(monty::rc_ptr< ::mosek::fusion::LinkedBlocks > other);
virtual void free(int bkey) ;
virtual int alloc(int size) ;
virtual int maxidx(int bkey) ;
virtual void get(int bkey,std::shared_ptr< monty::ndarray< int,1 > > target,int offset) ;
virtual int numblocks() ;
virtual int blocksize(int bkey) ;
virtual int capacity() ;
virtual bool validate() ;
}; // class LinkedBlocks;

class LinkedInts : public virtual monty::RefCounted
{
public: p_LinkedInts * _impl;
protected: LinkedInts(p_LinkedInts * _impl);
public:
LinkedInts(const LinkedInts &) = delete;
const LinkedInts & operator=(const LinkedInts &) = delete;
friend class p_LinkedInts;
virtual ~LinkedInts();
virtual void destroy();
typedef monty::rc_ptr< LinkedInts > t;

LinkedInts(int cap_);
LinkedInts();
LinkedInts(monty::rc_ptr< ::mosek::fusion::LinkedInts > other);
virtual void free(int i,int num) ;
virtual int alloc() ;
virtual int alloc(int n) ;
virtual void alloc(int num,std::shared_ptr< monty::ndarray< int,1 > > target,int offset) ;
virtual void get(int i,int num,std::shared_ptr< monty::ndarray< int,1 > > target,int offset) ;
virtual int maxidx(int i,int num) ;
virtual int capacity() ;
virtual bool validate() ;
}; // class LinkedInts;

class Parameters : public virtual monty::RefCounted
{
public: p_Parameters * _impl;
protected: Parameters(p_Parameters * _impl);
public:
Parameters(const Parameters &) = delete;
const Parameters & operator=(const Parameters &) = delete;
friend class p_Parameters;
virtual ~Parameters();
virtual void destroy();
typedef monty::rc_ptr< Parameters > t;

static void setParameter(monty::rc_ptr< ::mosek::fusion::Model > M,const std::string &  name,double value);
static void setParameter(monty::rc_ptr< ::mosek::fusion::Model > M,const std::string &  name,int value);
static void setParameter(monty::rc_ptr< ::mosek::fusion::Model > M,const std::string &  name,const std::string &  value);
}; // class Parameters;

}
}
namespace mosek
{
namespace fusion
{
namespace Utils
{
// class mosek.fusion.Utils.StringIntMap
// mosek.fusion.Utils.IntMap from file 'src/fusion/cxx/IntMap.h'
// namespace mosek::fusion::Utils
class IntMap : public monty::RefCounted
{
  std::unique_ptr<p_IntMap> _impl;
public:
  friend class p_IntMap;
  typedef monty::rc_ptr<IntMap> t;

  IntMap();
  bool hasItem (long long key);
  int  getItem (long long key);
  void setItem (long long key, int val);
  std::shared_ptr<monty::ndarray<long long,1>> keys();
  std::shared_ptr<monty::ndarray<int,1>>       values();

  t clone();
  t __mosek_2fusion_2Utils_2IntMap__clone();
};

class StringIntMap : public monty::RefCounted
{
  std::unique_ptr<p_StringIntMap> _impl;
public:
  friend class p_StringIntMap;
  typedef monty::rc_ptr<StringIntMap> t;

  StringIntMap();
  bool hasItem (const std::string & key);
  int  getItem (const std::string & key);
  void setItem (const std::string & key, int val);
  std::shared_ptr<monty::ndarray<std::string,1>> keys();
  std::shared_ptr<monty::ndarray<int,1>>       values();

  t clone();
  t __mosek_2fusion_2Utils_2StringIntMap__clone();
};

// End of file 'src/fusion/cxx/IntMap.h'
// mosek.fusion.Utils.StringBuffer from file 'src/fusion/cxx/StringBuffer.h'
// namespace mosek::fusion::Utils
class StringBuffer : public monty::RefCounted
{
private:
  std::unique_ptr<p_StringBuffer> _impl;
public:
  friend class p_StringBuffer;

  typedef monty::rc_ptr<StringBuffer> t;

  StringBuffer();
  t clear ();
  t a (int                 value);
  t a (long long           value);
  t a (double              value);
  t a (const std::string & value);
  t a (bool                value);
  t a (std::shared_ptr<monty::ndarray<std::string,1>> value);
  t a (std::shared_ptr<monty::ndarray<int,1>>         value);
  t a (std::shared_ptr<monty::ndarray<long long,1>>   value);
  t a (std::shared_ptr<monty::ndarray<double,1>>      value);
  t lf ();

  t __mosek_2fusion_2Utils_2StringBuffer__clear ();
  t __mosek_2fusion_2Utils_2StringBuffer__a (int                 value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (long long           value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (double              value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (const std::string & value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (bool                value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<std::string,1>> value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<int,1>>         value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<long long,1>>   value);
  t __mosek_2fusion_2Utils_2StringBuffer__a (std::shared_ptr<monty::ndarray<double,1>>      value);
  t __mosek_2fusion_2Utils_2StringBuffer__lf ();


  std::string toString () const;
};
// End of file 'src/fusion/cxx/StringBuffer.h'
// mosek.fusion.Utils.Tools from file 'src/fusion/cxx/Tools.h'
namespace Tools
{
  template<typename T, int N>
  void
  arraycopy
  ( const std::shared_ptr<monty::ndarray<T,N>> & src,
    int                                          srcoffset,
    const std::shared_ptr<monty::ndarray<T,N>> & tgt,
    int                                          tgtoffset,
    int                                          size)
  {
    std::copy(src->flat_begin()+srcoffset, src->flat_begin()+srcoffset+size, tgt->flat_begin()+tgtoffset);
  }

  template<typename T, int N>
  void
  arraycopy
  ( const std::shared_ptr<monty::ndarray<T,N>> & src,
    long long                                    srcoffset,
    const std::shared_ptr<monty::ndarray<T,N>> & tgt,
    long long                                    tgtoffset,
    long long                                    size)
  {
    std::copy(src->flat_begin()+srcoffset, src->flat_begin()+srcoffset+size, tgt->flat_begin()+tgtoffset);
  }

  template<typename T, int N>
  std::shared_ptr<monty::ndarray<T,N>> 
  arraycopy (const std::shared_ptr<monty::ndarray<T,N>> & a) 
  { 
    return std::shared_ptr<monty::ndarray<T,N>>(new monty::ndarray<T,N>(a->shape, a->flat_begin(), a->flat_end()));
  }

  template<typename T> 
  std::shared_ptr<monty::ndarray<T,1>> range(T last) 
  {     
    return std::shared_ptr<monty::ndarray<T,1>>(new monty::ndarray<T,1>(monty::shape(last), monty::iterable(monty::range_t<T>(0,last))));
  }
  
  template<typename T> 
  std::shared_ptr<monty::ndarray<T,1>> range(T first, T last)
  { 
    return std::shared_ptr<monty::ndarray<T,1>>(new monty::ndarray<T,1>(monty::shape(last-first), monty::iterable(monty::range_t<T>(first,last))));
  }
  
  template<typename T> 
  std::shared_ptr<monty::ndarray<T,1>> range(T first, T last, T step)
  { 
    size_t num = last > first && step > 0 ? (last - first - 1) / step + 1 : 0;
    if (num > 0)
      return std::shared_ptr<monty::ndarray<T,1>>(new monty::ndarray<T,1>(monty::shape(num), monty::iterable(monty::range_t<T>(first,last,step))));
    else
      return std::shared_ptr<monty::ndarray<T,1>>(new monty::ndarray<T,1>(monty::shape(0)));
  }

  static std::shared_ptr<monty::ndarray<double,1>> zeros(int num)            { return std::shared_ptr<monty::ndarray<double,1>>(new monty::ndarray<double,1>(monty::shape(num),0.0));       }
  static std::shared_ptr<monty::ndarray<double,2>> zeros(int dimi, int dimj) { return std::shared_ptr<monty::ndarray<double,2>>(new monty::ndarray<double,2>(monty::shape(dimi,dimj),0.0)); }
  static std::shared_ptr<monty::ndarray<double,1>> ones (int num)            { return std::shared_ptr<monty::ndarray<double,1>>(new monty::ndarray<double,1>(monty::shape(num),1.0));       }

  template<typename T>
  std::shared_ptr<monty::ndarray<T,1>> makevector(T v, int num) { return std::shared_ptr<monty::ndarray<T,1>>(new monty::ndarray<T,1>(monty::shape(num),v)); }

  template<typename T>
  std::shared_ptr<monty::ndarray<T,1>> repeatrange(T first, T last, T num)
  {
    return std::shared_ptr<monty::ndarray<T,1>>(new monty::ndarray<T,1>(monty::shape((last-first)*num),[=](ptrdiff_t i) { return (T)(i%num+first); }));
  }
  
  template<typename T>
  std::string stringvalue (T val) 
  {
    std::stringstream os; os << val;
    return os.str(); 
  }

  static int    toInt(const std::string & v)    { return atoi(v.c_str()); } 
  static double toDouble(const std::string & v) { return atof(v.c_str()); }
  static double sqrt(double v) { return std::sqrt(v); }


  template<typename T>
  void sort (const std::shared_ptr<monty::ndarray<T,1>> & vals,int first,int last)
  {
      std::sort(vals->flat_begin()+first, vals->flat_begin()+last, [&](T lhs, T rhs){ return lhs < rhs; });
  }

#if (defined(_WIN32) && _WIN32) || (defined(_WIN64) && _WIN64)
  static int randInt(int max) 
  {
    long long lo = rand(), hi = rand();
    return (int)(((double)(((hi << 31) | lo)) / (double)(LLONG_MAX)) * max);
  }
#else
  static int randInt(int max) { return (int)(((double)(random()) / (double)(RAND_MAX)) * max); }
#endif





  template<typename T>
  static void argsort(const std::shared_ptr<monty::ndarray<long long,1>> & perm, 
                      const std::shared_ptr<monty::ndarray<T,1>> & v,
                      long long first,
                      long long last)
  {
    std::sort(perm->begin()+first,perm->begin()+last,[&v](long long lhs, long long rhs){ return (*v)[lhs] < (*v)[rhs]; });
  }
  
  template<typename T>
  static void argsort(const std::shared_ptr<monty::ndarray<long long,1>> & perm, 
                      const std::shared_ptr<monty::ndarray<T,1>> & v0,
                      const std::shared_ptr<monty::ndarray<T,1>> & v1,
                      long long first,
                      long long last)
  {
    std::sort(perm->begin()+first,perm->begin()+last,[&v0,&v1](long long lhs, long long rhs){ return (*v0)[lhs] < (*v0)[rhs] || ((*v0)[lhs] == (*v0)[rhs] && (*v1)[lhs] < (*v1)[rhs]); });
  }


  template <typename T>
  static void bucketsort(const std::shared_ptr<monty::ndarray<long long,1>> & perm,
                         long long first, 
                         long long last,
                         const std::shared_ptr<monty::ndarray<T,1>> & v,
                         T minval,
                         T maxval)
  {
    T N = maxval-minval+1;
    long long M = last-first;
    std::vector<ptrdiff_t> ptrb(N+1);
    std::vector<long long> nperm(M);
    for (ptrdiff_t i = first; i < last; ++i) ++ptrb[(*v)[(*perm)[i]]-minval+1];
    for (ptrdiff_t i = 1; i < N; ++i) ptrb[i] += ptrb[i-1];
    for (ptrdiff_t i = first; i < last; ++i) nperm[ptrb[(*v)[(*perm)[i]]-minval]++] = (*perm)[i];

    std::copy(nperm.begin(),nperm.end(),perm->begin()+first);
  }
}

// End of file 'src/fusion/cxx/Tools.h'
}
}
}
#endif
