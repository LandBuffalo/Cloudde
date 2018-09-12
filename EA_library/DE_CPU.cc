#include "EA_CPU.h"

DE_CPU::DE_CPU()
{

}

DE_CPU::~DE_CPU()
{

}

string DE_CPU::GetParameters()
{
	string str;
	ostringstream temp1, temp2;
	string parameters = "CR/F=";

	double CR = DE_info_.CR;
	temp1<<CR;
	str=temp1.str();
	parameters.append(str);

	parameters.append("/");
	double F = DE_info_.F;
	temp2<<F;
	str=temp2.str();
	parameters.append(str);

	if(DE_info_.strategy_ID == 0)
		parameters.append("_current/1/bin");
	else if(DE_info_.strategy_ID == 1)
		parameters.append("_current/2/bin");
	else if(DE_info_.strategy_ID == 2)
		parameters.append("_current-to-best/1/bin");
	else if(DE_info_.strategy_ID == 3)
		parameters.append("_current-to-best/2/bin");
	else if(DE_info_.strategy_ID == 4)
		parameters.append("_rand/1/bin");
	else if(DE_info_.strategy_ID == 5)
		parameters.append("_rand/2/bin");
	else if(DE_info_.strategy_ID == 6)
		parameters.append("_best/1/bin");
	else if(DE_info_.strategy_ID == 7)
		parameters.append("_best/2/bin");
	else if(DE_info_.strategy_ID == 8)
		parameters.append("_current_to_rand/1/bin");
	return parameters;
}

int DE_CPU::Initilize(ProblemInfo problem_info, int configurations)
{
    EA_CPU::Initilize(problem_info, configurations);
    if(configurations == 0)
    {
        DE_info_.CR = 0.9;
        DE_info_.F = 0.5;
        DE_info_.strategy_ID = 4;  
    }
    if(configurations == 1)
    {
        DE_info_.CR = 0.9;
        DE_info_.F = 0.5;
        DE_info_.strategy_ID = 4;  
    }
    if(configurations == 2)
    {
        DE_info_.CR = 0.1;
        DE_info_.F = 0.5;
        DE_info_.strategy_ID = 6;  
    }
    if(configurations == 3)
    {
        DE_info_.CR = 0.1;
        DE_info_.F = 0.5;
        DE_info_.strategy_ID = 6;  
    }

	return 0;
}


int DE_CPU::Unitilize()
{
    EA_CPU::Unitilize();
    return 0;
}

int DE_CPU::Reproduce(Population & candidate, Population & population)
{

    Individual best_individual = FindBestIndividual(population);
    vector<int> r = random_.Permutate(population.size(), 5);
    double F = DE_info_.F;
    double CR = DE_info_.CR;
    for (int i = 0; i < population.size(); i++)
    {
    	Individual tmp_candidate;
        double tmp_value = 0;
        for (int j = 0; j < problem_info_.dim; j++)
        {
            switch (DE_info_.strategy_ID)
            {
                case 0:
                    tmp_value = population[i].elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]);
                    break;
                case 1:
                    tmp_value = population[i].elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]) + \
                    + F * (population[r[2]].elements[j] - population[r[3]].elements[j]);
                    break;
                case 2:
                    tmp_value = population[i].elements[j] + F * (best_individual.elements[j] - population[i].elements[j]) + \
                    + F * (population[r[0]].elements[j] - population[r[1]].elements[j]);
                    break;
                case 3:
                    tmp_value = population[i].elements[j] + F * (best_individual.elements[j] - population[i].elements[j]) + \
                    + F * (population[r[0]].elements[j] - population[r[1]].elements[j]) + F * (population[r[2]].elements[j] - population[r[3]].elements[j]);
                    break;
                case 4:
                    tmp_value = population[r[0]].elements[j] + F * (population[r[1]].elements[j] - population[r[2]].elements[j]);
                    break;
                case 5:
                    tmp_value = population[r[0]].elements[j] + F * (population[r[1]].elements[j] - population[r[2]].elements[j]) + \
                    + F * (population[r[3]].elements[j] - population[r[4]].elements[j]);
                    break;
                case 6:
                    tmp_value = best_individual.elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]);
                    break;
                case 7:
                    tmp_value = best_individual.elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]) + \
                    + F * (population[r[2]].elements[j] - population[r[3]].elements[j]);
                    break;
                case 8:
                    tmp_value = population[i].elements[j] + F * (population[r[0]].elements[j] - population[i].elements[j]) + \
                    + F * (population[r[1]].elements[j] - population[r[2]].elements[j]) + F * (population[r[3]].elements[j] - population[r[4]].elements[j]);
                    break;
                default:
                    break;
            }
            if (random_.RandDoubleUnif(0, 1) > CR && j != random_.RandIntUnif(0, problem_info_.dim - 1))
                tmp_value = population[i].elements[j];
            tmp_value = CheckBound(tmp_value, problem_info_.min_bound, problem_info_.max_bound);
            tmp_candidate.elements.push_back(tmp_value);
        }
        tmp_candidate.fitness_value = 0;
        candidate.push_back(tmp_candidate);
    }

    return 0;
}

int DE_CPU::SelectSurvival(Population & population, Population & candidate)
{
    for(int i = 0; i < population.size(); i++)
        if(candidate[i].fitness_value < population[i].fitness_value)
            population[i] = candidate[i];

    return 0;
}

int DE_CPU::Run(Population & population)
{
    Individual best_individual = FindBestIndividual(population);
    vector<int> r = random_.Permutate(population.size(), 5);

    double F = DE_info_.F;
    double CR = DE_info_.CR;

    for (int i = 0; i < population.size(); i++)
    {
    	Individual tmp_candidate;
        double tmp_value = 0;
        for (int j = 0; j < problem_info_.dim; j++)
        {
            switch (DE_info_.strategy_ID)
            {
                case 0:
                    tmp_value = population[i].elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]);
                    break;
                case 1:
                    tmp_value = population[i].elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]) + \
                    + F * (population[r[2]].elements[j] - population[r[3]].elements[j]);
                    break;
                case 2:
                    tmp_value = population[i].elements[j] + F * (best_individual.elements[j] - population[i].elements[j]) + \
                    + F * (population[r[0]].elements[j] - population[r[1]].elements[j]);
                    break;
                case 3:
                    tmp_value = population[i].elements[j] + F * (best_individual.elements[j] - population[i].elements[j]) + \
                    + F * (population[r[0]].elements[j] - population[r[1]].elements[j]) + F * (population[r[2]].elements[j] - population[r[3]].elements[j]);
                    break;
                case 4:
                    tmp_value = population[r[0]].elements[j] + F * (population[r[1]].elements[j] - population[r[2]].elements[j]);
                    break;
                case 5:
                    tmp_value = population[r[0]].elements[j] + F * (population[r[1]].elements[j] - population[r[2]].elements[j]) + \
                    + F * (population[r[3]].elements[j] - population[r[4]].elements[j]);
                    break;
                case 6:
                    tmp_value = best_individual.elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]);
                    break;
                case 7:
                    tmp_value = best_individual.elements[j] + F * (population[r[0]].elements[j] - population[r[1]].elements[j]) + \
                    + F * (population[r[2]].elements[j] - population[r[3]].elements[j]);
                    break;
                case 8:
                    tmp_value = population[i].elements[j] + F * (population[r[0]].elements[j] - population[i].elements[j]) + \
                    + F * (population[r[1]].elements[j] - population[r[2]].elements[j]) + F * (population[r[3]].elements[j] - population[r[4]].elements[j]);
                    break;
                default:
                    break;
            }
            if (random_.RandDoubleUnif(0, 1) > CR && j != random_.RandIntUnif(0, problem_info_.dim - 1))
               tmp_value = population[i].elements[j];
            //tmp_value = CheckBound(tmp_value, problem_info_.min_bound, problem_info_.max_bound);
            tmp_candidate.elements.push_back(tmp_value); 
        }
        tmp_candidate.fitness_value = cec2014_.EvaluateFitness(tmp_candidate.elements);
        if(tmp_candidate.fitness_value < population[i].fitness_value)
        	population[i] = tmp_candidate;
    }

    return 0;
}

