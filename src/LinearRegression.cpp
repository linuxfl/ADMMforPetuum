#include <string>
#include <glog/logging.h>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <petuum_ps_common/include/petuum_ps.hpp>
#include <io/general_fstream.hpp>
#include <stdio.h>

#include "util/Eigen/Dense"
#include "util/context.hpp"
#include "LinearRegression.hpp"

namespace LR {

    // Constructor
    LinearRegression::LinearRegression(): thread_counter_(0) {
        // timer
        initT_ = boost::posix_time::microsec_clock::local_time();

        /* context */
        lda::Context & context = lda::Context::get_instance();
        // input and output
        data_file_ = context.get_string("data_file");
        input_data_format_ = context.get_string("input_data_format");
        
        output_path_ = context.get_string("output_path");
        output_data_format_ = context.get_string("output_data_format");
        maximum_running_time_ = context.get_double("maximum_running_time");

        // objective function parameters
        feature = context.get_int32("feature");
		row = context.get_int32("row");
		
        // petuum parameters
        client_id_ = context.get_int32("client_id");
        num_clients_ = context.get_int32("num_clients");
        num_worker_threads_ = context.get_int32("num_worker_threads");

        // optimization parameters
        num_epochs_ = context.get_int32("num_epochs");
        LOG(INFO) <<"row: "<<row<< "feature: " << feature;
        
        //ADMM parameters
        rho = context.get_double("rho");
    }

    // linearRegression
    void LinearRegression::Start() {
        // thread id on a client
        int thread_id = thread_counter_++;
        petuum::PSTableGroup::RegisterThread();
        LOG(INFO) << "client " << client_id_ << ", thread " 
            << thread_id << " registers!";

        // Get z table
        petuum::Table<float> z_table = 
            petuum::PSTableGroup::GetTableOrDie<float>(0);
        //register row
        if(thread_id == 0)
			z_table.GetAsyncForced(0);
        
        //init table
        petuum::PSTableGroup::GlobalBarrier();
        float temp,error;
        char Adir[100],bdir[100];
        std::vector<float> z_cache;
        sprintf(Adir,"/home/fangling/petuum/bosen/app/ADMM/data/A%d.dat",thread_id);
        sprintf(bdir,"/home/fangling/petuum/bosen/app/ADMM/data/b%d.dat",thread_id);

        FILE *fpA = fopen(Adir,"r");
		FILE *fpb = fopen(bdir,"r");
		FILE *fps = fopen("/home/fangling/petuum/bosen/app/ADMM/data/solution.dat","r");

        //source data
        Eigen::MatrixXf A(row,feature);
        Eigen::VectorXf b(row);
        Eigen::VectorXf s(feature);
        Eigen::MatrixXf lemon(feature,feature);
        Eigen::MatrixXf lemonI(feature,feature);
        Eigen::MatrixXf identity(feature,feature);
		Eigen::VectorXf result(feature);
		
        //parament
        Eigen::VectorXf x(feature);
        Eigen::VectorXf x_diff(feature);
        Eigen::VectorXf z(feature);
        Eigen::VectorXf y(feature);
        Eigen::VectorXf z_old(feature);
        Eigen::VectorXf z_diff(feature);

        x.setZero();
        z.setZero();
        y.setZero();
        z_old.setZero();
        z_diff.setZero();
        
        //init the z_table
        if(thread_id == 0 && client_id_ == 0){
			petuum::UpdateBatch<float> z_update;
			for(int i = 0;i < feature;i++){
				z_update.Update(i,z(i));
			}
			z_table.BatchInc(0, z_update);
		}

        //load data from file
        for(int i = 0;i < row;i++){
			for(int j=0;j < feature;j++){
				fscanf(fpA,"%f",&temp);
				A(i,j) = temp;
			}
		}
        for(int i = 0;i < row;i++){
			fscanf(fpb,"%f",&temp);
			b(i) = temp;
		}
		
		for(int i = 0;i < feature;i++){
			fscanf(fps,"%f",&temp);
			s(i) = temp;
		}

		lemon = A.transpose()  * A + rho * identity.setIdentity();
		lemonI = lemon.inverse();
		petuum::PSTableGroup::GlobalBarrier();
		
        for(int iter = 0 ;iter < num_epochs_;iter++){
			//get z from server
			petuum::RowAccessor row_acc;
			const auto & row = z_table.Get<petuum::DenseRow<float> >(0, &row_acc);
            row.CopyToVector(&z_cache);
            for (int col_id = 0; col_id < feature; ++col_id) {
				z(col_id) = z_cache[col_id];
            }
            
			//std::cout <<"thread_id "<<thread_id<< " :z "<<z<< std::endl;
			
            //update x
			x = lemonI * (A.transpose() * b + rho * z - y);
			/*if(thread_id == 0){
				std::cout << "after :"<< (A.transpose() * b + rho * z - y) << std::endl;
				std::cout << "lemonI :"<< lemonI << std::endl;
				std::cout << "x :"<< x << std::endl;
			}*/
			//update y
			y = y + rho * (x - z);
			//update z
			z = 1.0/num_worker_threads_ * (x + 1.0/rho * y);
			
			//z diff
			z_diff = z - z_old;
				
			//update new z_diff to server
			petuum::UpdateBatch<float> z_update;
			for(int i = 0;i < feature;i++){
				z_update.Update(i,z_diff(i));
			}
			z_table.BatchInc(0, z_update);
			
			petuum::PSTableGroup::Clock();
			 
			z_old = z;
			x_diff = x - s;
			error = x_diff.squaredNorm()/s.squaredNorm();
			if(thread_id == 0)
				LOG(INFO) << "iter: " << iter << ", client " 
                        << client_id_ << ", thread " << thread_id <<
                        " primal error: " << error << "object value";
            if(error < 10e-11)
            {
				return;
			}
		}
	}

    LinearRegression::~LinearRegression() {
    }
} // namespace LR
