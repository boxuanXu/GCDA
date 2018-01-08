/* *************************
 * Author: xbx1992
 * Created Time:  Tue 11  July 2015
 * LastModified:  Tue 11July 2015 04:00:27 PM CST
 * C File Name:  init.h
 * ************************/
#ifndef INIT_H
#  define INIT_H

#include <cstring>
#include <map>
#include "common.h"
#include "xg_datetime.h"
#include "state.h"
#include "configclass.h"

class DAConfig{
	private:
	map<string, string> DA_config;

	public:
	DAConfig(){}
	void Initialize(const char * conf, daconfig& dc, fileconfig& fc)
	{
    		/************************************
     		 * read configuration file 
     		 ************************************/
    		ReadConfig(conf, DA_config);
	 		
		/*initialization ds's file*/
	
		fc.DA_dir = DA_config["DA_dir"].c_str();
		
		fc.restart_f = DA_config["restart_file"].c_str();
	
		fc.obs_f = DA_config["obs"].c_str();
		
		fc.geos_f = DA_config["geos"].c_str(); 		
		
		fc.input = DA_config["input.geos"].c_str(); 		
			
		/*initialization ds's base configuration*/
		dc.K = atoi(DA_config["ensemble"].c_str());

		dc.grid_xres = atof(DA_config["res_lon"].c_str());	
		dc.grid_yres = atof(DA_config["res_lat"].c_str());	
    		dc.XSIZE     = int(360 / dc.grid_xres + 1e-5);
    		dc.YSIZE     = int(180 / dc.grid_yres + 1e-5) + 1;

		datetime start_date(DA_config["start_time"], DA_config["date_format"]);
		dc.start_date = start_date;
		datetime end_date(DA_config["end_time"], DA_config["date_format"]);
		dc.end_date = end_date;

		dc.DA_step = atoi(DA_config["DA_step"].c_str()); // assimilation time step     [minute]
		dc.DA_length = atoi(DA_config["DA_length"].c_str()); // assimilation step length   [minute]
		dc.F = dc.DA_length / dc.DA_step;

		dc.nlag = atoi(DA_config["nlag"].c_str());

		dc.rho = atof(DA_config["inflation"].c_str());

		dc.Class = atoi(DA_config["Class"].c_str());
		dc.M = dc.Class * dc.nlag;

		dc.grid_M = dc.XSIZE * dc.YSIZE * dc.nlag;	
	}



};	

	
statevector<double> generate_x(int mpi_rank, daconfig dc,map<int,vector<pair<int,int> > >& regions_index)
{
    		double *x_b_b = (double*)malloc(dc.nlag * dc.Class * dc.K * sizeof(double));
    		statevector<double> x_b_lagMembers(dc.nlag, dc.Class, dc.K);
		if(mpi_rank == 0) {
			x_b_lagMembers.Initialization(dc,regions_index);
			//x_b_lagMembers = readvector("/data1/xubx/GEOS-Chem/tools/main_experience_P_a_test/statevector.20050917000000.nc",dc); 
			x_b_b = x_b_lagMembers.expand();
			MPI_Bcast(x_b_b, dc.nlag * dc.Class * dc.K, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   		}
   		else{
    			MPI_Bcast(x_b_b, dc.nlag * dc.Class * dc.K, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   			reverse_expand(x_b_lagMembers,x_b_b, dc.nlag, dc.Class, dc.K);
		}
		//free(x_b_b);
		//x_b_p = NULL;
		return x_b_lagMembers;	
}	

mpi_config SAVE_MPI_Config(int mpi_size, int mpi_rank, int mpi_name_len, char * mpi_host_name)
{
	mpi_config mpic;
	mpic.size = mpi_size;
	mpic.rank = mpi_rank;
	mpic.name_len = mpi_name_len;
	//strcpy(mpic.host_name, mpi_host_name);

	return mpic;
}
#endif
