#pragma once
#include <string>
#include <atomic>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <petuum_ps_common/include/petuum_ps.hpp>
#include "matrix_loader.hpp"

namespace LR {
class LinearRegression {
    public:
        LinearRegression();
        ~LinearRegression();
        void Start();
    private:
        std::atomic<int> thread_counter_;

        // petuum parameters
        int client_id_, num_clients_, num_worker_threads_;
        float maximum_running_time_;
        
        // objective function parameters
        int rank_;

        // evaluate parameters
        int num_epochs_;

        // input and output
        std::string data_file_, input_data_format_, output_path_, 
            output_data_format_, cache_path_;

        // matrix loader for data X and S
        //MatrixLoader<float> X_matrix_loader_, L_matrix_loader_;

        // timer
        boost::posix_time::ptime initT_;
		
		//data
		int row,feature;
		
		//ADMM parament
		float rho;
};
}; // namespace LR
