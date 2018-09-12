#include "config.h"
#include "island_EA.h"
#include "master.h"
#include "slave.h"

#include <sstream>
#include <string>

char* getParam(char * needle, char* haystack[], int count) {
    int i = 0;
    for (i = 0; i < count; i++) {
        if (strcmp(needle, haystack[i]) == 0) {
            if (i < count - 1) {
                return haystack[i + 1];
            }
        }
    }
    return 0;
}

vector<string> &split(const string &str, char delim, vector<string> &elems, bool skip_empty = true)
{
    istringstream iss(str);
    for (string item; getline(iss, item, delim);)
        if (skip_empty && item.empty()) continue;
        else elems.push_back(item);
        return elems;
}

int SetParameters(vector<int> &total_functions, vector<int> &total_runs, IslandInfo &island_info, ProblemInfo &problem_info, NodeInfo &node_info, int argc, char** argv)
{
//---------------------------problem info----------------------------------------------------//
    problem_info.max_base_FEs = 1000000;
    problem_info.seed = 1;
    problem_info.max_bound = 100;
    problem_info.min_bound = -100;
    problem_info.dim = 100;
    problem_info.computing_time = 10;
    for (int i = 23; i <= 30; i++)
      total_functions.push_back(i);
    for (int i = 1; i <= 15; i++)
      total_runs.push_back(i);
    if (getParam("-dim", argv, argc))
      problem_info.dim = atoi(getParam("-dim", argv, argc));
    if (getParam("-total_functions", argv, argc))
    {
      total_functions.clear();
      string str = getParam("-total_functions", argv, argc);
      vector<string> functions;
      split(str, '-', functions);
      const char *tmp_function1 = functions[0].c_str();
      const char *tmp_function2 = functions[1].c_str();

      for (int i = atoi(tmp_function1); i <= atoi(tmp_function2); i++)
          total_functions.push_back(i);
    }
    if (getParam("-total_runs", argv, argc))
    {
      total_runs.clear();
      string str = getParam("-total_runs", argv, argc);
      vector<string> runs;
      split(str, '-', runs);
      const char *tmp_run1 = runs[0].c_str();
      const char *tmp_run2 = runs[1].c_str();

      for (int i = atoi(tmp_run1); i <= atoi(tmp_run2); i++)
          total_runs.push_back(i);

    }
    if (getParam("-computing_time", argv, argc))
      problem_info.computing_time = atoi(getParam("-computing_time", argv, argc));
    if (getParam("-max_base_FEs", argv, argc))
      problem_info.max_base_FEs = atoi(getParam("-max_base_FEs", argv, argc));
    if (getParam("-computing_time", argv, argc))
//---------------------------island info----------------------------------------------------//
    island_info.island_size = 64;
    island_info.island_num = 4;
    if (getParam("-island_num", argv, argc))
        island_info.island_num = atoi(getParam("-island_num", argv, argc));
    if (getParam("-pop_size", argv, argc))
        island_info.island_size = atoi(getParam("-pop_size", argv, argc)) / island_info.island_num;
    if (getParam("-island_size", argv, argc))
        island_info.island_size = atoi(getParam("-island_size", argv, argc));

//---------------------------node info----------------------------------------------------//
    node_info.task_ID = 1;
    if (getParam("-task_ID", argv, argc))
        node_info.task_ID = atoi(getParam("-task_ID", argv, argc));
    if(node_info.node_ID == 0)
        node_info.task_ID = 0;
    if(node_info.node_ID > island_info.island_num)
        node_info.task_ID = -1;

    return 0;

}

int ConstructAndExecuteIslandModule(vector<int> &total_functions, vector<int> &total_runs, IslandInfo island_info, ProblemInfo problem_info, NodeInfo node_info)
{
    IslandEA island_EA(node_info);

    for (int i = 0; i < total_functions.size(); i++)
    {
        problem_info.function_ID = total_functions[i];
        for (int j = 0; j < total_runs.size(); j++)
        {
            problem_info.run_ID = total_runs[j];
            problem_info.seed = (node_info.node_ID + problem_info.function_ID) * (problem_info.run_ID + problem_info.function_ID);
            srand(problem_info.seed);
            island_EA.Initilize(island_info, problem_info);
            island_EA.Execute();
            island_EA.Unitilize();
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    return 0;
}

int ConstructAndExecuteMasterModule(vector<int> &total_functions, vector<int> &total_runs, IslandInfo island_info, ProblemInfo problem_info, NodeInfo node_info)
{
    Master master(node_info);

    for (int i = 0; i < total_functions.size(); i++)
    {
        problem_info.function_ID = total_functions[i];
        if (node_info.node_ID == 1)
            printf("--------function_ID = %d---------------------\n", problem_info.function_ID);
        for (int j = 0; j < total_runs.size(); j++)
        {
            problem_info.run_ID = total_runs[j];
            printf("------run_ID = %d--------\n", problem_info.run_ID);
            master.Initilize(island_info, problem_info);
            master.Execute();
            master.Unitilize();
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    return 0;
}

int ConstructAndExecuteSlaveModule(vector<int> &total_functions, vector<int> &total_runs, IslandInfo island_info, ProblemInfo problem_info, NodeInfo node_info)
{
    Slave slave(node_info);

    for (int i = 0; i < total_functions.size(); i++)
    {
        problem_info.function_ID = total_functions[i];

        for (int j = 0; j < total_runs.size(); j++)
        {
            problem_info.run_ID = total_runs[j];
            slave.Initilize(island_info, problem_info);
            slave.Execute();
            slave.Unitilize();
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int current_node_id = 0;
    int total_node_num = 0;
    double start_time, finish_time;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &current_node_id);
    MPI_Comm_size(MPI_COMM_WORLD, &total_node_num);

    IslandInfo island_info;
    ProblemInfo problem_info;
    NodeInfo node_info;


    node_info.node_num = total_node_num;
    node_info.node_ID = current_node_id;

    start_time = MPI_Wtime();
    srand(time(NULL) * current_node_id);

    vector<int> total_functions, total_runs;
    SetParameters(total_functions, total_runs, island_info, problem_info, node_info, argc, argv);

    if(node_info.task_ID == 0)
        ConstructAndExecuteMasterModule(total_functions, total_runs, island_info, problem_info, node_info);
    if(node_info.task_ID == 1)
        ConstructAndExecuteIslandModule(total_functions, total_runs, island_info, problem_info, node_info);
    if(node_info.task_ID == -1)
        ConstructAndExecuteSlaveModule(total_functions, total_runs, island_info, problem_info, node_info);
    finish_time = MPI_Wtime();
    MPI_Finalize();

    printf("Total Elapsed time: %f seconds\n", finish_time - start_time);
    return 0;
}
