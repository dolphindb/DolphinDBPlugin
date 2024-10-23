 
#ifndef _GUROBI_CPP_H
#define _GUROBI_CPP_H
 
// Copyright (C) 2023, Gurobi Optimization, LLC
// All Rights Reserved
#include <iostream>
#include <vector>
#include <string>

extern "C" {
#include "gurobi_c.h"
}

#define GRB_ERROR_NOT_IN_MODEL            20001
#define GRB_ERROR_FAILED_TO_CREATE_MODEL  20002
#define GRB_ERROR_INTERNAL                20003

enum GRB_DoubleParam {
  GRB_DoubleParam_Cutoff,
  GRB_DoubleParam_IterationLimit,
  GRB_DoubleParam_MemLimit,
  GRB_DoubleParam_SoftMemLimit,
  GRB_DoubleParam_NodeLimit,
  GRB_DoubleParam_TimeLimit,
  GRB_DoubleParam_WorkLimit,
  GRB_DoubleParam_FeasibilityTol,
  GRB_DoubleParam_IntFeasTol,
  GRB_DoubleParam_MarkowitzTol,
  GRB_DoubleParam_MIPGap,
  GRB_DoubleParam_MIPGapAbs,
  GRB_DoubleParam_OptimalityTol,
  GRB_DoubleParam_PerturbValue,
  GRB_DoubleParam_Heuristics,
  GRB_DoubleParam_ObjScale,
  GRB_DoubleParam_NodefileStart,
  GRB_DoubleParam_BarConvTol,
  GRB_DoubleParam_BarQCPConvTol,
  GRB_DoubleParam_PSDTol,
  GRB_DoubleParam_ImproveStartGap,
  GRB_DoubleParam_ImproveStartNodes,
  GRB_DoubleParam_ImproveStartTime,
  GRB_DoubleParam_FeasRelaxBigM,
  GRB_DoubleParam_TuneTimeLimit,
  GRB_DoubleParam_TuneCleanup,
  GRB_DoubleParam_TuneTargetMIPGap,
  GRB_DoubleParam_TuneTargetTime,
  GRB_DoubleParam_PreSOS1BigM,
  GRB_DoubleParam_PreSOS2BigM,
  GRB_DoubleParam_PoolGap,
  GRB_DoubleParam_PoolGapAbs,
  GRB_DoubleParam_BestObjStop,
  GRB_DoubleParam_BestBdStop,
  GRB_DoubleParam_CSQueueTimeout,
  GRB_DoubleParam_FuncPieceError,
  GRB_DoubleParam_FuncPieceLength,
  GRB_DoubleParam_FuncPieceRatio,
  GRB_DoubleParam_FuncMaxVal,
  GRB_DoubleParam_NoRelHeurTime,
  GRB_DoubleParam_NoRelHeurWork,
  GRB_DoubleParam_WLSTokenRefresh
};

enum GRB_IntParam {
  GRB_IntParam_SolutionLimit,
  GRB_IntParam_Method,
  GRB_IntParam_ScaleFlag,
  GRB_IntParam_SimplexPricing,
  GRB_IntParam_Quad,
  GRB_IntParam_NormAdjust,
  GRB_IntParam_Sifting,
  GRB_IntParam_SiftMethod,
  GRB_IntParam_NetworkAlg,
  GRB_IntParam_LPWarmStart,
  GRB_IntParam_SubMIPNodes,
  GRB_IntParam_VarBranch,
  GRB_IntParam_Cuts,
  GRB_IntParam_CliqueCuts,
  GRB_IntParam_CoverCuts,
  GRB_IntParam_FlowCoverCuts,
  GRB_IntParam_FlowPathCuts,
  GRB_IntParam_GUBCoverCuts,
  GRB_IntParam_ImpliedCuts,
  GRB_IntParam_ProjImpliedCuts,
  GRB_IntParam_MIPSepCuts,
  GRB_IntParam_MIRCuts,
  GRB_IntParam_StrongCGCuts,
  GRB_IntParam_ModKCuts,
  GRB_IntParam_ZeroHalfCuts,
  GRB_IntParam_NetworkCuts,
  GRB_IntParam_SubMIPCuts,
  GRB_IntParam_InfProofCuts,
  GRB_IntParam_RelaxLiftCuts,
  GRB_IntParam_RLTCuts,
  GRB_IntParam_BQPCuts,
  GRB_IntParam_PSDCuts,
  GRB_IntParam_LiftProjectCuts,
  GRB_IntParam_CutAggPasses,
  GRB_IntParam_CutPasses,
  GRB_IntParam_GomoryPasses,
  GRB_IntParam_NodeMethod,
  GRB_IntParam_Presolve,
  GRB_IntParam_Aggregate,
  GRB_IntParam_IISMethod,
  GRB_IntParam_PreCrush,
  GRB_IntParam_PreDepRow,
  GRB_IntParam_PrePasses,
  GRB_IntParam_DisplayInterval,
  GRB_IntParam_OutputFlag,
  GRB_IntParam_Threads,
  GRB_IntParam_BarIterLimit,
  GRB_IntParam_Crossover,
  GRB_IntParam_CrossoverBasis,
  GRB_IntParam_BarCorrectors,
  GRB_IntParam_BarOrder,
  GRB_IntParam_PumpPasses,
  GRB_IntParam_RINS,
  GRB_IntParam_Symmetry,
  GRB_IntParam_MIPFocus,
  GRB_IntParam_NumericFocus,
  GRB_IntParam_AggFill,
  GRB_IntParam_PreDual,
  GRB_IntParam_SolutionNumber,
  GRB_IntParam_MinRelNodes,
  GRB_IntParam_ZeroObjNodes,
  GRB_IntParam_BranchDir,
  GRB_IntParam_DegenMoves,
  GRB_IntParam_InfUnbdInfo,
  GRB_IntParam_DualReductions,
  GRB_IntParam_BarHomogeneous,
  GRB_IntParam_PreQLinearize,
  GRB_IntParam_MIQCPMethod,
  GRB_IntParam_NonConvex,
  GRB_IntParam_QCPDual,
  GRB_IntParam_LogToConsole,
  GRB_IntParam_PreSOS1Encoding,
  GRB_IntParam_PreSOS2Encoding,
  GRB_IntParam_PreSparsify,
  GRB_IntParam_PreMIQCPForm,
  GRB_IntParam_Seed,
  GRB_IntParam_ConcurrentMIP,
  GRB_IntParam_ConcurrentJobs,
  GRB_IntParam_DistributedMIPJobs,
  GRB_IntParam_LazyConstraints,
  GRB_IntParam_TuneResults,
  GRB_IntParam_TuneTrials,
  GRB_IntParam_TuneOutput,
  GRB_IntParam_TuneJobs,
  GRB_IntParam_TuneCriterion,
  GRB_IntParam_TuneMetric,
  GRB_IntParam_Disconnected,
  GRB_IntParam_UpdateMode,
  GRB_IntParam_Record,
  GRB_IntParam_ObjNumber,
  GRB_IntParam_MultiObjMethod,
  GRB_IntParam_MultiObjPre,
  GRB_IntParam_PoolSolutions,
  GRB_IntParam_PoolSearchMode,
  GRB_IntParam_ScenarioNumber,
  GRB_IntParam_StartNumber,
  GRB_IntParam_StartNodeLimit,
  GRB_IntParam_IgnoreNames,
  GRB_IntParam_PartitionPlace,
  GRB_IntParam_CSPriority,
  GRB_IntParam_CSTLSInsecure,
  GRB_IntParam_CSIdleTimeout,
  GRB_IntParam_ServerTimeout,
  GRB_IntParam_TSPort,
  GRB_IntParam_JSONSolDetail,
  GRB_IntParam_CSBatchMode,
  GRB_IntParam_FuncPieces,
  GRB_IntParam_CSClientLog,
  GRB_IntParam_IntegralityFocus,
  GRB_IntParam_NLPHeur,
  GRB_IntParam_WLSTokenDuration,
  GRB_IntParam_LicenseID,
  GRB_IntParam_OBBT
};

enum GRB_StringParam {
  GRB_StringParam_LogFile,
  GRB_StringParam_NodefileDir,
  GRB_StringParam_ResultFile,
  GRB_StringParam_WorkerPool,
  GRB_StringParam_WorkerPassword,
  GRB_StringParam_ComputeServer,
  GRB_StringParam_ServerPassword,
  GRB_StringParam_CSRouter,
  GRB_StringParam_CSGroup,
  GRB_StringParam_TokenServer,
  GRB_StringParam_CloudAccessID,
  GRB_StringParam_CloudSecretKey,
  GRB_StringParam_CloudPool,
  GRB_StringParam_CloudHost,
  GRB_StringParam_JobID,
  GRB_StringParam_CSManager,
  GRB_StringParam_CSAuthToken,
  GRB_StringParam_CSAPIAccessID,
  GRB_StringParam_CSAPISecret,
  GRB_StringParam_UserName,
  GRB_StringParam_CSAppName,
  GRB_StringParam_SolFiles,
  GRB_StringParam_WLSAccessID,
  GRB_StringParam_WLSSecret,
  GRB_StringParam_WLSToken,
  GRB_StringParam_Dummy
};

enum GRB_IntAttr {
  GRB_IntAttr_NumConstrs,
  GRB_IntAttr_NumVars,
  GRB_IntAttr_NumSOS,
  GRB_IntAttr_NumQConstrs,
  GRB_IntAttr_NumGenConstrs,
  GRB_IntAttr_NumNZs,
  GRB_IntAttr_NumQNZs,
  GRB_IntAttr_NumQCNZs,
  GRB_IntAttr_NumIntVars,
  GRB_IntAttr_NumBinVars,
  GRB_IntAttr_NumPWLObjVars,
  GRB_IntAttr_ModelSense,
  GRB_IntAttr_IsMIP,
  GRB_IntAttr_IsQP,
  GRB_IntAttr_IsQCP,
  GRB_IntAttr_IsMultiObj,
  GRB_IntAttr_Status,
  GRB_IntAttr_ConcurrentWinMethod,
  GRB_IntAttr_SolCount,
  GRB_IntAttr_BarIterCount,
  GRB_IntAttr_VBasis,
  GRB_IntAttr_CBasis,
  GRB_IntAttr_PWLObjCvx,
  GRB_IntAttr_BranchPriority,
  GRB_IntAttr_VarPreStat,
  GRB_IntAttr_BoundVioIndex,
  GRB_IntAttr_BoundSVioIndex,
  GRB_IntAttr_ConstrVioIndex,
  GRB_IntAttr_ConstrSVioIndex,
  GRB_IntAttr_ConstrResidualIndex,
  GRB_IntAttr_ConstrSResidualIndex,
  GRB_IntAttr_DualVioIndex,
  GRB_IntAttr_DualSVioIndex,
  GRB_IntAttr_DualResidualIndex,
  GRB_IntAttr_DualSResidualIndex,
  GRB_IntAttr_ComplVioIndex,
  GRB_IntAttr_IntVioIndex,
  GRB_IntAttr_IISMinimal,
  GRB_IntAttr_IISLB,
  GRB_IntAttr_IISUB,
  GRB_IntAttr_IISConstr,
  GRB_IntAttr_IISSOS,
  GRB_IntAttr_IISQConstr,
  GRB_IntAttr_IISGenConstr,
  GRB_IntAttr_IISLBForce,
  GRB_IntAttr_IISUBForce,
  GRB_IntAttr_IISConstrForce,
  GRB_IntAttr_IISSOSForce,
  GRB_IntAttr_IISQConstrForce,
  GRB_IntAttr_IISGenConstrForce,
  GRB_IntAttr_TuneResultCount,
  GRB_IntAttr_Lazy,
  GRB_IntAttr_VarHintPri,
  GRB_IntAttr_ObjNPriority,
  GRB_IntAttr_NumObj,
  GRB_IntAttr_GenConstrType,
  GRB_IntAttr_NumStart,
  GRB_IntAttr_Partition,
  GRB_IntAttr_LicenseExpiration,
  GRB_IntAttr_NumScenarios,
  GRB_IntAttr_FuncPieces,
  GRB_IntAttr_BatchErrorCode,
  GRB_IntAttr_BatchStatus,
  GRB_IntAttr_Fingerprint,
  GRB_IntAttr_PoolIgnore
};

enum GRB_CharAttr {
  GRB_CharAttr_VType,
  GRB_CharAttr_Sense,
  GRB_CharAttr_QCSense
};

enum GRB_DoubleAttr {
  GRB_DoubleAttr_Runtime,
  GRB_DoubleAttr_Work,
  GRB_DoubleAttr_ObjCon,
  GRB_DoubleAttr_LB,
  GRB_DoubleAttr_UB,
  GRB_DoubleAttr_Obj,
  GRB_DoubleAttr_Start,
  GRB_DoubleAttr_PreFixVal,
  GRB_DoubleAttr_RHS,
  GRB_DoubleAttr_QCRHS,
  GRB_DoubleAttr_MaxCoeff,
  GRB_DoubleAttr_MinCoeff,
  GRB_DoubleAttr_MaxBound,
  GRB_DoubleAttr_MinBound,
  GRB_DoubleAttr_MaxObjCoeff,
  GRB_DoubleAttr_MinObjCoeff,
  GRB_DoubleAttr_MaxRHS,
  GRB_DoubleAttr_MinRHS,
  GRB_DoubleAttr_MaxQCRHS,
  GRB_DoubleAttr_MinQCRHS,
  GRB_DoubleAttr_MaxQCCoeff,
  GRB_DoubleAttr_MinQCCoeff,
  GRB_DoubleAttr_MaxQCLCoeff,
  GRB_DoubleAttr_MinQCLCoeff,
  GRB_DoubleAttr_MaxQObjCoeff,
  GRB_DoubleAttr_MinQObjCoeff,
  GRB_DoubleAttr_ObjVal,
  GRB_DoubleAttr_ObjBound,
  GRB_DoubleAttr_ObjBoundC,
  GRB_DoubleAttr_MIPGap,
  GRB_DoubleAttr_IterCount,
  GRB_DoubleAttr_NodeCount,
  GRB_DoubleAttr_X,
  GRB_DoubleAttr_RC,
  GRB_DoubleAttr_Pi,
  GRB_DoubleAttr_QCPi,
  GRB_DoubleAttr_Slack,
  GRB_DoubleAttr_QCSlack,
  GRB_DoubleAttr_MaxVio,
  GRB_DoubleAttr_BoundVio,
  GRB_DoubleAttr_BoundSVio,
  GRB_DoubleAttr_BoundVioSum,
  GRB_DoubleAttr_BoundSVioSum,
  GRB_DoubleAttr_ConstrVio,
  GRB_DoubleAttr_ConstrSVio,
  GRB_DoubleAttr_ConstrVioSum,
  GRB_DoubleAttr_ConstrSVioSum,
  GRB_DoubleAttr_ConstrResidual,
  GRB_DoubleAttr_ConstrSResidual,
  GRB_DoubleAttr_ConstrResidualSum,
  GRB_DoubleAttr_ConstrSResidualSum,
  GRB_DoubleAttr_DualVio,
  GRB_DoubleAttr_DualSVio,
  GRB_DoubleAttr_DualVioSum,
  GRB_DoubleAttr_DualSVioSum,
  GRB_DoubleAttr_DualResidual,
  GRB_DoubleAttr_DualSResidual,
  GRB_DoubleAttr_DualResidualSum,
  GRB_DoubleAttr_DualSResidualSum,
  GRB_DoubleAttr_ComplVio,
  GRB_DoubleAttr_ComplVioSum,
  GRB_DoubleAttr_IntVio,
  GRB_DoubleAttr_IntVioSum,
  GRB_DoubleAttr_Kappa,
  GRB_DoubleAttr_KappaExact,
  GRB_DoubleAttr_SAObjLow,
  GRB_DoubleAttr_SAObjUp,
  GRB_DoubleAttr_SALBLow,
  GRB_DoubleAttr_SALBUp,
  GRB_DoubleAttr_SARHSLow,
  GRB_DoubleAttr_SAUBLow,
  GRB_DoubleAttr_SAUBUp,
  GRB_DoubleAttr_SARHSUp,
  GRB_DoubleAttr_Xn,
  GRB_DoubleAttr_FarkasProof,
  GRB_DoubleAttr_FarkasDual,
  GRB_DoubleAttr_UnbdRay,
  GRB_DoubleAttr_PStart,
  GRB_DoubleAttr_DStart,
  GRB_DoubleAttr_BarX,
  GRB_DoubleAttr_VarHintVal,
  GRB_DoubleAttr_ObjN,
  GRB_DoubleAttr_ObjNCon,
  GRB_DoubleAttr_ObjNWeight,
  GRB_DoubleAttr_ObjNRelTol,
  GRB_DoubleAttr_ObjNAbsTol,
  GRB_DoubleAttr_ObjNVal,
  GRB_DoubleAttr_OpenNodeCount,
  GRB_DoubleAttr_PoolObjBound,
  GRB_DoubleAttr_PoolObjVal,
  GRB_DoubleAttr_ScenNLB,
  GRB_DoubleAttr_ScenNUB,
  GRB_DoubleAttr_ScenNObj,
  GRB_DoubleAttr_ScenNRHS,
  GRB_DoubleAttr_ScenNX,
  GRB_DoubleAttr_ScenNObjBound,
  GRB_DoubleAttr_ScenNObjVal,
  GRB_DoubleAttr_FuncPieceError,
  GRB_DoubleAttr_FuncPieceLength,
  GRB_DoubleAttr_FuncPieceRatio,
  GRB_DoubleAttr_DNumNZs
};

enum GRB_StringAttr {
  GRB_StringAttr_ModelName,
  GRB_StringAttr_VarName,
  GRB_StringAttr_ConstrName,
  GRB_StringAttr_QCName,
  GRB_StringAttr_GenConstrName,
  GRB_StringAttr_ObjNName,
  GRB_StringAttr_ScenNName,
  GRB_StringAttr_BatchID,
  GRB_StringAttr_VTag,
  GRB_StringAttr_CTag,
  GRB_StringAttr_QCTag,
  GRB_StringAttr_BatchErrorMessage
};

class GRBVar;
class GRBExpr;
class GRBLinExpr;
class GRBQuadExpr;
class GRBConstr;
class GRBModel;
class GRBEnv;
class GRBException;
class GRBCallback;
class GRBSOS;
class GRBQConstr;
class GRBGenConstr;
class GRBColumn;
class GRBTempConstr;

std::ostream& operator<<(std::ostream &stream, GRBLinExpr expr);
GRBLinExpr operator+(const GRBLinExpr& x, const GRBLinExpr& y);
GRBLinExpr operator-(const GRBLinExpr& x, const GRBLinExpr& y);
GRBLinExpr operator+(const GRBLinExpr& x);
GRBLinExpr operator+(GRBVar x, GRBVar y);
GRBLinExpr operator+(GRBVar x, double a);
GRBLinExpr operator+(double a, GRBVar x);
GRBLinExpr operator-(const GRBLinExpr& x);
GRBLinExpr operator-(GRBVar x);
GRBLinExpr operator-(GRBVar x, GRBVar y);
GRBLinExpr operator-(GRBVar x, double a);
GRBLinExpr operator-(double a, GRBVar x);
GRBLinExpr operator*(double a, GRBVar x);
GRBLinExpr operator*(GRBVar x, double a);
GRBLinExpr operator*(const GRBLinExpr& x, double a);
GRBLinExpr operator*(double a, const GRBLinExpr& x);
GRBLinExpr operator/(GRBVar x, double a);
GRBLinExpr operator/(const GRBLinExpr& x, double a);

std::ostream& operator<<(std::ostream &stream, GRBQuadExpr expr);
GRBQuadExpr operator+(const GRBQuadExpr& x, const GRBQuadExpr& y);
GRBQuadExpr operator-(const GRBQuadExpr& x, const GRBQuadExpr& y);
GRBQuadExpr operator+(const GRBQuadExpr& x);
GRBQuadExpr operator-(const GRBQuadExpr& x);
GRBQuadExpr operator*(const GRBQuadExpr& x, double a);
GRBQuadExpr operator*(double a, const GRBQuadExpr& x);
GRBQuadExpr operator*(GRBVar x, GRBVar y);
GRBQuadExpr operator*(GRBVar x, const GRBLinExpr& y);
GRBQuadExpr operator*(const GRBLinExpr& y, GRBVar x);
GRBQuadExpr operator*(const GRBLinExpr& x, const GRBLinExpr& y);
GRBQuadExpr operator/(const GRBQuadExpr& x, double a);

GRBTempConstr operator<=(GRBQuadExpr x, GRBQuadExpr y);
GRBTempConstr operator>=(GRBQuadExpr x, GRBQuadExpr y);
GRBTempConstr operator==(GRBQuadExpr x, GRBQuadExpr y);

class GRBEnv
{
  private:

    GRBenv*  env;
    GRBenv** envP;

    const GRBEnv& operator=(const GRBEnv &xenv);

  public:

    friend class GRBModel;
    friend class GRBBatch;

    GRBEnv(const bool empty = false);
    GRBEnv(const char* logfilename);
    GRBEnv(const std::string& logfilename);
    GRBEnv(const std::string&, const std::string&, const std::string&, int, const std::string&);
    GRBEnv(const std::string&, const std::string&, const std::string&);
    GRBEnv(const std::string&, const std::string&, const std::string&, int, const std::string&,
           void* (__stdcall *)(MALLOCCB_ARGS),
           void* (__stdcall *)(CALLOCCB_ARGS),
           void* (__stdcall *)(REALLOCCB_ARGS),
           void  (__stdcall *)(FREECB_ARGS),
           int   (__stdcall *)(THREADCREATECB_ARGS),
           void  (__stdcall *)(THREADJOINCB_ARGS),
           void*);
    GRBEnv(const GRBEnv& xenv) : env(xenv.env),
                                 envP(xenv.envP) {}

    ~GRBEnv();
    void start();
    void message(const std::string& msg);
    void fixTuneParam(GRB_IntParam param);
    void fixTuneParam(GRB_DoubleParam param);
    void fixTuneParam(GRB_StringParam param);
    void fixTuneParam(const std::string& paramname);
    int get(GRB_IntParam param) const;
    double get(GRB_DoubleParam param) const;
    std::string get(GRB_StringParam param) const;
    void set(GRB_IntParam param, int newvalue);
    void set(GRB_DoubleParam param, double newvalue);
    void set(GRB_StringParam param, const std::string& newvalue);
    void set(const std::string& paramname, const std::string& newvalue);
    void getParamInfo(GRB_DoubleParam param, double* valP,
                      double* minP, double* maxP, double* defP);
    void getParamInfo(GRB_IntParam param, int* valP, int* minP,
                      int* maxP, int* defP);
    void getParamInfo(GRB_StringParam param, std::string& value,
                      std::string& defvalue);
    void resetParams();
    void writeParams(const std::string& paramfile);
    void readParams(const std::string& paramfile);
    const std::string getErrorMsg() const;
    int getWLSTokenLifespan() const;
};

class GRBModel
{
  private:

    GRBmodel    *Cmodel;
    GRBenv      *Cenv;
    GRBCallback *cb;

    int rows;
    int cols;
    int numsos;
    int numqconstrs;
    int numgenconstrs;
    int newranges;

    int updatemode;

    std::vector<GRBVar> vars;
    std::vector<GRBConstr> constrs;
    std::vector<GRBSOS> sos;
    std::vector<GRBQConstr> qconstrs;
    std::vector<GRBGenConstr> genconstrs;

    const GRBModel& operator=(const GRBModel &xmodel);


  public:

    GRBModel(const GRBEnv* env);
    GRBModel(const GRBEnv& env);
    GRBModel(const GRBEnv* env, const std::string& filename);
    GRBModel(const GRBEnv& env, const std::string& filename);
    GRBModel(const GRBModel& xmodel);

    ~GRBModel();

    void read(const std::string& filename);
    void write(const std::string& filename);

    void sync();

    GRBModel relax();
    GRBModel fixedModel();
    GRBModel presolve();
    GRBModel dualize();
    GRBModel feasibility();
    GRBModel linearize();
    GRBModel singleScenarioModel();

    double feasRelax(int relaxobjtype, bool minrelax, bool vrelax, bool crelax);
    double feasRelax(int relaxobjtype, bool minrelax, int vlen,
                   const GRBVar* vars, const double* lbpen,
                   const double* ubpen, int clen, const GRBConstr* constrs,
                   const double* rhspen);

    void update();
    void optimize();
    std::string optimizeBatch();
    void optimizeasync();
    void computeIIS();
    void tune();
    void reset(int clearall = 0);
    void check();
    void terminate();

    void getTuneResult(int i);

    GRBQuadExpr getObjective() const;
    GRBLinExpr getObjective(int index) const;
    int  getPWLObj(GRBVar v, double *x, double *y) const;
    void setObjective(GRBLinExpr obje, int sense=0);
    void setObjective(GRBQuadExpr obje, int sense=0);
    void setObjectiveN(GRBLinExpr obj, int index, int priority=0,
                       double weight=1, double abstol=0, double reltol=0,
                       std::string name="");
    void setPWLObj(GRBVar v, int points, double *x, double *y);

    GRBVar getVar(int i) const;
    GRBVar* getVars() const;
    GRBVar getVarByName(const std::string& name);
    GRBConstr getConstr(int i) const;
    GRBConstr* getConstrs() const;
    GRBConstr getConstrByName(const std::string& name);
    GRBSOS* getSOSs() const;
    GRBQConstr* getQConstrs() const;
    GRBGenConstr* getGenConstrs() const;

    GRBVar addVar(double lb, double ub, double obj, char vtype,
                  std::string vname="");
    GRBVar addVar(double lb, double ub, double obj, char vtype,
                  int nonzeros, const GRBConstr* xconstrs,
                  const double* coeffs=NULL, std::string name="");
    GRBVar addVar(double lb, double ub, double obj, char vtype,
                  const GRBColumn& col, std::string name="");
    GRBVar* addVars(int cnt, char type=GRB_CONTINUOUS);
    GRBVar* addVars(const double* lb, const double* ub,
                    const double* obj, const char* type,
                    const std::string* name, int len);
    GRBVar* addVars(const double* lb, const double *ub,
                    const double* obj, const char* type,
                    const std::string* name, const GRBColumn*
                    col, int len);

    GRBConstr addConstr(const GRBLinExpr& expr1, char sense,
                        const GRBLinExpr& expr2,
                        std::string name="");
    GRBConstr addConstr(const GRBLinExpr& expr, char sense, GRBVar v,
                        std::string name="");
    GRBConstr addConstr(GRBVar v1, char sense, GRBVar v2,
                        std::string name="");
    GRBConstr addConstr(GRBVar v, char sense, double rhs,
                        std::string name="");
    GRBConstr addConstr(const GRBLinExpr& expr, char sense, double rhs,
                        std::string name="");
    GRBConstr addConstr(const GRBTempConstr& tc, std::string name="");
    GRBConstr addRange(const GRBLinExpr& expr, double lower, double upper,
                       std::string name="");
    GRBConstr* addConstrs(int cnt);
    GRBConstr* addConstrs(const GRBLinExpr* expr, const char* sense,
                          const double* rhs, const std::string* name,
                          int len);
    GRBConstr* addRanges(const GRBLinExpr* expr, const double* lower,
                         const double* upper, const std::string* name,
                         int len);
    GRBSOS addSOS(const GRBVar* xvars, const double* weight, int len, int type);
    GRBQConstr addQConstr(const GRBQuadExpr& expr1, char sense,
                          const GRBQuadExpr& expr2,
                          std::string name="");
    GRBQConstr addQConstr(const GRBTempConstr& tc, std::string name="");
    GRBQConstr addQConstr(const GRBQuadExpr&  expr, char sense, double rhs,
                          std::string name="");
    GRBGenConstr addGenConstrMax(GRBVar resvar, const GRBVar* xvars,
                                 int len, double constant=-GRB_INFINITY, std::string name="");
    GRBGenConstr addGenConstrMin(GRBVar resvar, const GRBVar* xvars,
                                 int len, double constant=GRB_INFINITY, std::string name="");
    GRBGenConstr addGenConstrAbs(GRBVar resvar, GRBVar argvar,
                                 std::string name="");
    GRBGenConstr addGenConstrAnd(GRBVar resvar, const GRBVar* xvars,
                                 int len, std::string name="");
    GRBGenConstr addGenConstrOr(GRBVar resvar, const GRBVar* xvars,
                                int len, std::string name="");
    GRBGenConstr addGenConstrNorm(GRBVar resvar, const GRBVar* xvars,
                                  int len, double which, std::string name="");
    GRBGenConstr addGenConstrIndicator(GRBVar binvar, int binval,
                                       const GRBLinExpr& expr, char sense, double rhs,
                                       std::string name="");
    GRBGenConstr addGenConstrIndicator(GRBVar binvar, int binval,
                                       const GRBTempConstr& constr,
                                       std::string name="");
    GRBGenConstr addGenConstrPWL(GRBVar xvar, GRBVar yvar, int npts, const double* xpts,
                                 const double* ypts, std::string name="");
    GRBGenConstr addGenConstrPoly(GRBVar xvar, GRBVar yvar, int plen, const double* p,
                                  std::string name="", std::string options="");
    GRBGenConstr addGenConstrExp(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrExpA(GRBVar xvar, GRBVar yvar, double a, std::string name="",
                                  std::string options="");
    GRBGenConstr addGenConstrLog(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrLogA(GRBVar xvar, GRBVar yvar, double a, std::string name="",
                                  std::string options="");
    GRBGenConstr addGenConstrPow(GRBVar xvar, GRBVar yvar, double a, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrSin(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrCos(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrTan(GRBVar xvar, GRBVar yvar, std::string name="",
                                 std::string options="");
    GRBGenConstr addGenConstrLogistic(GRBVar xvar, GRBVar yvar, std::string name="",
                                      std::string options="");

    void remove(GRBVar v);
    void remove(GRBConstr c);
    void remove(GRBSOS xsos);
    void remove(GRBQConstr xqconstr);
    void remove(GRBGenConstr xgenconstr);

    void chgCoeff(GRBConstr c, GRBVar v, double val);
    void chgCoeffs(const GRBConstr* xconstrs, const GRBVar* xvars,
                   const double* val, int len);
    void chgCoeffs(const GRBConstr* xconstrs, const GRBVar* xvars,
                   const double* val, size_t len);
    double getCoeff(GRBConstr c, GRBVar v) const;
    GRBColumn getCol(GRBVar v);
    GRBLinExpr getRow(GRBConstr c);
    int getSOS(GRBSOS xsos, GRBVar* xvars, double* weight, int* typeP);
    void getGenConstrMax(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                         int* lenP, double* constantP);
    void getGenConstrMin(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                         int* lenP, double* constantP);
    void getGenConstrAbs(GRBGenConstr genc, GRBVar* resvarP, GRBVar* argvarP);
    void getGenConstrAnd(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                         int* lenP);
    void getGenConstrOr(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                        int* lenP);
    void getGenConstrNorm(GRBGenConstr genc, GRBVar* resvarP, GRBVar* xvars,
                          int* lenP, double* whichP);
    void getGenConstrIndicator(GRBGenConstr genc, GRBVar* binvarP, int* binvalP,
                               GRBLinExpr* exprP, char* senseP, double* rhsP);
    void getGenConstrPWL(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, int* nptsP,
                         double* xpts, double* ypts);
    void getGenConstrPoly(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, int* plenP, double* p);
    void getGenConstrExp(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrExpA(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, double* aP);
    void getGenConstrLog(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrLogA(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, double* aP);
    void getGenConstrPow(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP, double* aP);
    void getGenConstrSin(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrCos(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrTan(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);
    void getGenConstrLogistic(GRBGenConstr genc, GRBVar* xvarP, GRBVar* yvarP);

    GRBQuadExpr getQCRow(GRBQConstr c);
    GRBEnv getEnv() const;
    GRBEnv getConcurrentEnv(int num);
    void discardConcurrentEnvs();
    GRBEnv getMultiobjEnv(int num);
    void discardMultiobjEnvs();

    // Parameters

    void fixTuneParam(GRB_IntParam param);
    void fixTuneParam(GRB_DoubleParam param);
    void fixTuneParam(GRB_StringParam param);
    void fixTuneParam(const std::string& paramname);

    int    get(GRB_IntParam param) const;
    double get(GRB_DoubleParam param) const;
    std::string get(GRB_StringParam param) const;

    void set(GRB_IntParam param, int val);
    void set(GRB_DoubleParam param, double val);
    void set(GRB_StringParam param, const std::string& val);
    void set(const std::string& param, const std::string& val);

    // Attributes

    int    get(GRB_IntAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int val);
    void set(GRB_DoubleAttr attr, double val);
    void set(GRB_StringAttr attr, const std::string& val);

    int*    get(GRB_IntAttr    attr, const GRBVar* xvars, int len);
    char*   get(GRB_CharAttr   attr, const GRBVar* xvars, int len);
    double* get(GRB_DoubleAttr attr, const GRBVar* xvars, int len);
    std::string* get(GRB_StringAttr attr, const GRBVar* xvars, int len);

    int*    get(GRB_IntAttr    attr, const GRBConstr* xconstrs, int len);
    char*   get(GRB_CharAttr   attr, const GRBConstr* xconstrs, int len);
    double* get(GRB_DoubleAttr attr, const GRBConstr* xconstrs, int len);
    std::string* get(GRB_StringAttr attr, const GRBConstr* xconstrs, int len);

    int*    get(GRB_IntAttr    attr, const GRBQConstr* xqconstrs, int len);
    char*   get(GRB_CharAttr   attr, const GRBQConstr* xqconstrs, int len);
    double* get(GRB_DoubleAttr attr, const GRBQConstr* xqconstrs, int len);
    std::string* get(GRB_StringAttr attr, const GRBQConstr* xqconstrs, int len);

    int*    get(GRB_IntAttr    attr, const GRBGenConstr* xgenconstrs, int len);
    std::string* get(GRB_StringAttr attr, const GRBGenConstr* xgenconstrs, int len);

    void prefetchAttr(GRB_IntAttr    attr);
    void prefetchAttr(GRB_CharAttr   attr);
    void prefetchAttr(GRB_DoubleAttr attr);
    void prefetchAttr(GRB_StringAttr attr);

    std::string getJSONSolution(void);

    void set(GRB_IntAttr    attr, const GRBVar* xvars,
             const int*    val, int len);
    void set(GRB_CharAttr   attr, const GRBVar* xvars,
             const char*   val, int len);
    void set(GRB_DoubleAttr attr, const GRBVar* xvars,
             const double* val, int len);
    void set(GRB_StringAttr attr, const GRBVar* xvars,
             const std::string* val, int len);

    void set(GRB_IntAttr    attr, const GRBConstr* xconstrs,
             const int*    val, int len);
    void set(GRB_CharAttr   attr, const GRBConstr* xconstrs,
             const char*   val, int len);
    void set(GRB_DoubleAttr attr, const GRBConstr* xconstrs,
             const double* val, int len);
    void set(GRB_StringAttr attr, const GRBConstr* xconstrs,
             const std::string* val, int len);

    void set(GRB_CharAttr   attr, const GRBQConstr* xconstrs,
             const char*   val, int len);
    void set(GRB_DoubleAttr attr, const GRBQConstr* xconstrs,
             const double* val, int len);
    void set(GRB_StringAttr attr, const GRBQConstr* xconstrs,
             const std::string* val, int len);

    void set(GRB_StringAttr attr, const GRBGenConstr* xqconstrs,
             const std::string* val, int len);

    void setCallback(GRBCallback* xcb);
};

class GRBVarRep;

class GRBVar
{
  private:

    GRBVarRep* varRep;

  public:

    friend class GRBModel;
    friend class GRBLinExpr;
    friend class GRBQuadExpr;
    friend class GRBCallback;

    GRBVar();
    GRBVar(const GRBVar& xvar) : varRep(xvar.varRep) {}

    GRBVar& operator=(const GRBVar& xvar) { varRep = xvar.varRep; return *this; }

    int index() const;
    int get(GRB_IntAttr attr) const;
    char get(GRB_CharAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int value);
    void set(GRB_CharAttr attr, char value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);

    bool sameAs(GRBVar v2);
};

class GRBConRep;

class GRBConstr
{
  private:

    GRBConRep* conRep;

  public:

    friend class GRBModel;
    friend class GRBColumn;

    GRBConstr();
    GRBConstr(const GRBConstr& constr) : conRep(constr.conRep) {}

    GRBConstr& operator=(const GRBConstr& constr) { conRep = constr.conRep; return *this; }

    int index() const;
    int get(GRB_IntAttr attr) const;
    char get(GRB_CharAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int value);
    void set(GRB_CharAttr attr, char value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);

    bool sameAs(GRBConstr c2);
};

class GRBExpr
{
  private:

  public:
    friend class GRBModel;
};

class GRBLinExpr: public GRBExpr
{
  private:

    double constant;
    std::vector<double> coeffs;
    std::vector<GRBVar> vars;
  public:

    GRBLinExpr(const GRBLinExpr& lin);
    GRBLinExpr(double constant=0.0);
    GRBLinExpr(GRBVar var, double coeff=1.0);

    friend class GRBQuadExpr;

    friend std::ostream& operator<<(std::ostream &stream, GRBLinExpr expr);
    friend GRBLinExpr operator+(const GRBLinExpr& x, const GRBLinExpr& y);
    friend GRBLinExpr operator+(const GRBLinExpr& x);
    friend GRBLinExpr operator+(GRBVar x, GRBVar y);
    friend GRBLinExpr operator+(GRBVar x, double a);
    friend GRBLinExpr operator+(double a, GRBVar x);
    friend GRBLinExpr operator-(const GRBLinExpr& x, const GRBLinExpr& y);
    friend GRBLinExpr operator-(const GRBLinExpr& x);
    friend GRBLinExpr operator-(GRBVar x);
    friend GRBLinExpr operator-(GRBVar x, GRBVar y);
    friend GRBLinExpr operator-(GRBVar x, double a);
    friend GRBLinExpr operator-(double a, GRBVar x);
    friend GRBLinExpr operator*(double a, GRBVar x);
    friend GRBLinExpr operator*(GRBVar x, double a);
    friend GRBLinExpr operator*(const GRBLinExpr& x, double a);
    friend GRBLinExpr operator*(double a, const GRBLinExpr& x);
    friend GRBLinExpr operator/(GRBVar x, double a);
    friend GRBLinExpr operator/(const GRBLinExpr& x, double a);

    unsigned int size(void) const;
    GRBVar getVar(int i) const;
    double getCoeff(int i) const;
    double getConstant() const;
    double getValue() const;

    void addTerms(const double* coeff, const GRBVar* var, int cnt);
    GRBLinExpr& operator=(const GRBLinExpr& expr);
    void operator+=(const GRBLinExpr& expr);
    void operator-=(const GRBLinExpr& expr);
    void operator*=(double mult);
    void operator/=(double a);
    GRBLinExpr operator+(const GRBLinExpr& rhs);
    GRBLinExpr operator-(const GRBLinExpr& rhs);
    void remove(int i);
    bool remove(GRBVar v);

    void clear();
};

class GRBQuadExpr: public GRBExpr
{
  private:

    GRBLinExpr linexpr;
    std::vector<double> coeffs;
    std::vector<GRBVar> vars1;
    std::vector<GRBVar> vars2;
  public:

    GRBQuadExpr(double constant=0.0);
    GRBQuadExpr(const GRBQuadExpr& expr);
    GRBQuadExpr(GRBVar var, double coeff=1.0);
    GRBQuadExpr(GRBLinExpr le);

    friend std::ostream& operator<<(std::ostream &stream, GRBQuadExpr expr);
    friend GRBQuadExpr operator+(const GRBQuadExpr& x, const GRBQuadExpr& y);
    friend GRBQuadExpr operator-(const GRBQuadExpr& x, const GRBQuadExpr& y);
    friend GRBQuadExpr operator+(const GRBQuadExpr& x);
    friend GRBQuadExpr operator-(const GRBQuadExpr& x);
    friend GRBQuadExpr operator*(const GRBQuadExpr& x, double a);
    friend GRBQuadExpr operator*(double a, const GRBQuadExpr& x);
    friend GRBQuadExpr operator*(GRBVar x, GRBVar y);
    friend GRBQuadExpr operator*(GRBVar x, const GRBLinExpr& y);
    friend GRBQuadExpr operator*(const GRBLinExpr& y, GRBVar x);
    friend GRBQuadExpr operator*(const GRBLinExpr& x, const GRBLinExpr& y);
    friend GRBQuadExpr operator/(const GRBQuadExpr& x, double a);

    unsigned int size(void) const;
    GRBVar getVar1(int i) const;
    GRBVar getVar2(int i) const;
    double getCoeff(int i) const;
    GRBLinExpr getLinExpr() const;
    double getValue() const;

    void addConstant(double c);
    void addTerm(double coeff, GRBVar var);
    void addTerm(double coeff, GRBVar var1, GRBVar var2);
    void addTerms(const double* coeff, const GRBVar* var, int cnt);
    void addTerms(const double* coeff, const GRBVar* var1,
                  const GRBVar* var2, int cnt);
    void add(const GRBLinExpr le);
    GRBQuadExpr operator=(const GRBQuadExpr& rhs);
    void operator+=(const GRBQuadExpr& expr);
    void operator-=(const GRBQuadExpr& expr);
    void operator*=(double mult);
    void operator/=(double a);
    GRBQuadExpr operator+(const GRBQuadExpr& rhs);
    GRBQuadExpr operator-(const GRBQuadExpr& rhs);
    void remove(int i);
    bool remove(GRBVar v);

    void clear();
};

class GRBBatch
{
  private:

    GRBenv   *Cenv;
    GRBbatch *Cbatch;

  public:

    // constructor
    GRBBatch(const GRBEnv& env, const std::string& batchID);

    // destructor
    ~GRBBatch();

    // Attributes
    int         get(GRB_IntAttr    attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr    attr, int         val);
    void set(GRB_DoubleAttr attr, double      val);
    void set(GRB_StringAttr attr, std::string val);

    // Control functions
    void abort(void);
    void discard(void);
    void retry(void);
    void update(void);

    // Output functions
    std::string getJSONSolution(void);
    void        writeJSONSolution(std::string filename);
};

class GRBException
{
  private:

    std::string msg;
    int error;

  public:

    GRBException(int errcode = 0);
    GRBException(const GRBException& exc) : msg(exc.msg), error(exc.error) {}
    GRBException(std::string errmsg, int errcode = 0);

    GRBException& operator=(const GRBException& exc) {
      msg = exc.msg;
      error = exc.error;
      return *this;
    }

    const std::string getMessage() const;
    int getErrorCode() const;
};

class GRBCallback
{
  private:

    GRBmodel*   Cmodel;
    int         cols;
    void*       cbdata;
    double*     x;
    double*     newx;
    double*     relx;

  public:

    friend void GRBModel::computeIIS();
    friend void GRBModel::optimize();
    friend void GRBModel::optimizeasync();
    friend void GRBModel::update();

    GRBCallback();
    virtual ~GRBCallback() {};

  protected:

    int where;
    virtual void callback() {};
    double getDoubleInfo(int what);
    int getIntInfo(int what);
    const std::string getStringInfo(int what) const;
    double getSolution(GRBVar v);
    double* getSolution(const GRBVar* xvars, int len);
    double getNodeRel(GRBVar v);
    double* getNodeRel(const GRBVar* xvars, int len);
    void setSolution(GRBVar v, double val);
    void setSolution(const GRBVar* xvars, const double* sol, int len);
    double useSolution();
    void addCut(const GRBTempConstr& tc);
    void addCut(const GRBLinExpr& expr, char sense, double rhs);
    void addLazy(const GRBTempConstr& tc);
    void addLazy(const GRBLinExpr& expr, char sense, double rhs);
    void abort();
    void proceed();
    void stopOneMultiObj(int objnum);
};

class GRBColumn
{
  private:

    std::vector<double> coeffs;
    std::vector<GRBConstr> constrs;

  public:

    GRBColumn() {}
    GRBColumn(const GRBColumn &xcol) : coeffs(xcol.coeffs),
                                       constrs(xcol.constrs) {}

    GRBColumn& operator=(const GRBColumn &xcol) {
      coeffs  = xcol.coeffs;
      constrs = xcol.constrs;
      return *this;
    }

    unsigned int size(void) const;
    GRBConstr getConstr(int i) const;
    double getCoeff(int i) const;

    void addTerm(double coeff, GRBConstr constr);
    void addTerms(const double* coeff, const GRBConstr* constr, int cnt);
    void remove(int i);
    bool remove(GRBConstr c);

    void clear();
};

class GRBSOSRep;

class GRBSOS
{
  private:

    GRBSOSRep* sosRep;

  public:

    friend class GRBModel;

    GRBSOS();
    GRBSOS(const GRBSOS& sos) : sosRep(sos.sosRep) {}

    GRBSOS& operator=(const GRBSOS& sos) { sosRep = sos.sosRep; return *this; }

    int get(GRB_IntAttr attr) const;

    void set(GRB_IntAttr attr, int value);
};

class GRBQConstrRep;

class GRBQConstr
{
  private:

    GRBQConstrRep* qconRep;

  public:

    friend class GRBModel;

    GRBQConstr();
    GRBQConstr(const GRBQConstr& qconstr) : qconRep(qconstr.qconRep) {}

    GRBQConstr& operator=(const GRBQConstr& qconstr) { qconRep = qconstr.qconRep; return *this; }

    char get(GRB_CharAttr attr) const;
    int get(GRB_IntAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_CharAttr attr, char value);
    void set(GRB_IntAttr attr, int value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);
};

class GRBGenConstrRep;

class GRBGenConstr
{
  private:

    GRBGenConstrRep* genconRep;

  public:

    friend class GRBModel;

    GRBGenConstr();
    int get(GRB_IntAttr attr) const;
    double get(GRB_DoubleAttr attr) const;
    std::string get(GRB_StringAttr attr) const;

    void set(GRB_IntAttr attr, int value);
    void set(GRB_DoubleAttr attr, double value);
    void set(GRB_StringAttr attr, const std::string& value);
};

class GRBTempConstr
{
  private:

    GRBQuadExpr expr;
    char        sense;

  public:

    GRBTempConstr() {}
    GRBTempConstr(const GRBTempConstr& tmp) : expr(tmp.expr),
                                              sense(tmp.sense) {}

    GRBTempConstr& operator=(const GRBTempConstr& tmp) {
      expr  = tmp.expr;
      sense = tmp.sense;
      return *this;
    }

    friend class GRBModel;
    friend class GRBCallback;
    friend GRBTempConstr operator<=(GRBQuadExpr x, GRBQuadExpr y);
    friend GRBTempConstr operator>=(GRBQuadExpr x, GRBQuadExpr y);
    friend GRBTempConstr operator==(GRBQuadExpr x, GRBQuadExpr y);
};
#endif
