#ifndef __CPU_EA_LIBRARY_H__
#define __CPU_EA_LIBRARY_H__
#include <stdio.h>
#include "config.h"
#include "CEC2014.h"
#include "random.h"
#include <sstream>
typedef double real;
struct GAInfo
{
    double CR;
	double MR;
	int selection_strategy;
	int pool_size;
};

struct PSOInfo
{
	double C1;
	double C2;
	double W;
	double Vmax;
};

struct DEInfo
{
	double CR;
	double F;
	int strategy_ID;
};

struct ESInfo
{
	double overall_learning_rate;
	double coordinate_wise_learning_rate;
	int lamda_rate;
	int combination_strategy;
	int parents_num;
	int mu;
};

class EA_CPU
{
protected:
    ProblemInfo             problem_info_;
    CEC2014   				cec2014_;
    Random          		random_;
public:
                    		EA_CPU();
                    		~EA_CPU();
	virtual int     		Initilize(ProblemInfo problem_info, int configurations);
    virtual int             Unitilize();
    virtual string          GetParameters() = 0;
	virtual int     		Run(Population & population) = 0;
	virtual int     		Reproduce(Population & candidate, Population & population) = 0;
	virtual int     		SelectSurvival(Population & population, Population & candidate) = 0;
    
    int             		InitilizePopulation(Population & population, int pop_size);
	int 					EvaluateFitness(Population & population);
    real                    CheckBound(real to_check_elements, real min_bound, real max_bound);
	Individual     	        FindBestIndividual(Population & population);
#ifdef DEBUG
    void            		FprintPopulation(Population &population);
#endif
};


class GA_CPU : public EA_CPU
{
private:
	GAInfo          		GA_info_;
    vector<int>             SelectParents(Population & population, vector<int> parents_index_pool);

public:
							GA_CPU();
							~GA_CPU();
	virtual int    		 	Initilize(ProblemInfo problem_info, int configurations);
	virtual int            	Unitilize();
	virtual int    		 	Run(Population & population);
	virtual string          GetParameters();
	virtual int     		Reproduce(Population & candidate, Population & population);
	virtual int     		SelectSurvival(Population & population, Population & candidate);
};


class PSO_CPU : public EA_CPU
{
private:
	PSOInfo         		PSO_info_;
    Individual              gbest_;
    Population              pbest_;
    Population              velocity_;
    int                     Optimizer(Population & population);
    int                     UpdatePbest(Population & population);
    int                     UpdateGbest();
public:
							PSO_CPU();
							~PSO_CPU();
	virtual int     		Initilize(ProblemInfo problem_info, int configurations);

	virtual int             Unitilize();
	virtual int             Run(Population & population);
	virtual string          GetParameters();
	virtual int     		Reproduce(Population & candidate, Population & population);
	virtual int     		SelectSurvival(Population & population, Population & candidate);
};


class DE_CPU : public EA_CPU
{
private:
	DEInfo          		DE_info_;
public:
							DE_CPU();
							~DE_CPU();
	virtual int            	Initilize(ProblemInfo problem_info, int configurations);
	virtual int             Unitilize();
	virtual int             Run(Population & population);
	virtual string          GetParameters();
	virtual int     		Reproduce(Population & candidate, Population & population);
	virtual int     		SelectSurvival(Population & population, Population & candidate);
};


#endif
