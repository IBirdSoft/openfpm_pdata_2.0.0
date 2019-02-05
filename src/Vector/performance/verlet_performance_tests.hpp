/*
 * vector_dist_verlet_performance_tests.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: Yaroslav Zaluzhnyi
 */

#ifndef SRC_VECTOR_VECTOR_DIST_VERLET_PERFORMANCE_TESTS_HPP_
#define SRC_VECTOR_VECTOR_DIST_VERLET_PERFORMANCE_TESTS_HPP_

/*! \brief Print a string about the test
 *
 * \param test string to print
 * \param sz size
 *
 */
void print_test_v(std::string test, size_t sz)
{
	if (create_vcluster().getProcessUnitID() == 0)
		std::cout << test << " " << sz << "\n";
}

BOOST_AUTO_TEST_SUITE( verletlist_part_reorder_performance_test )

///////////////////// INPUT DATA //////////////////////

// Cut-off radiuses. Can be put different number of values
openfpm::vector<float> r_cutoff {0.004, 0.007, 0.01};
// Orders of a curve. Can be put different number of values
// The starting amount of particles (remember that this number is multiplied by number of processors you use for testing)
size_t k_start = 100000;
// The minimal amount of particles
size_t k_min = 15000;
// Ghost part of distributed vector

///////////////////////////////////////////////////////

// Numbers of particles vector
openfpm::vector<size_t> n_particles;
// Vectors to store the data for 2D
openfpm::vector<openfpm::vector<double>> time_force_mean;
openfpm::vector<openfpm::vector<double>> time_force_dev;
openfpm::vector<openfpm::vector<double>> time_create_mean;
openfpm::vector<openfpm::vector<double>> time_create_dev;

// Vectors to store the data for 3D
openfpm::vector<openfpm::vector<double>> time_force_mean_2;
openfpm::vector<openfpm::vector<double>> time_force_dev_2;
openfpm::vector<openfpm::vector<double>> time_create_mean_2;
openfpm::vector<openfpm::vector<double>> time_create_dev_2;

/*! \brief Function for verlet test without an Hilbert curve reordering (unordered positioning)
 *
 */
template<unsigned int dim> void vd_verlet_random_benchmark(size_t k_start,
		                                                   size_t k_min,
														   openfpm::vector<float> & r_cutoff,
														   openfpm::vector<size_t> & n_particles,
														   openfpm::vector<openfpm::vector<double>> & time_force_mean,
														   openfpm::vector<openfpm::vector<double>> & time_create_mean,
														   openfpm::vector<openfpm::vector<double>> & time_force_dev,
														   openfpm::vector<openfpm::vector<double>> & time_create_dev)
{
	time_force_mean.resize(r_cutoff.size());
	time_create_mean.resize(r_cutoff.size());
	time_force_dev.resize(r_cutoff.size());
	time_create_dev.resize(r_cutoff.size());

	std::string str("Testing " + std::to_string(dim) + "D vector no-order, Verlet-list");
	print_test_v(str,0);

	{
		//For different r_cut
		for (size_t r = 0; r < r_cutoff.size(); r++ )
		{
			Vcluster & v_cl = create_vcluster();

			//Cut-off radius
			float r_cut = r_cutoff.get(r);

			//Number of particles
			size_t k = k_start * v_cl.getProcessingUnits();

			//Counter number for amounts of particles
			size_t k_count = 1 + log2(k/k_min);

			for (size_t k_int = k ; k_int >= k_min ; k_int/=2 )
			{
				BOOST_TEST_CHECKPOINT( "Testing " << dim << "D vector without an Hilbert curve reordering k=" << k_int );

				if (n_particles.size() < k_count)
					n_particles.add(k_int);

				Box<dim,float> box;

				for (size_t i = 0; i < dim; i++)
				{
					box.setLow(i,0.0);
					box.setHigh(i,1.0);
				}

				// Boundary conditions
				size_t bc[dim];
				for (size_t i = 0; i < dim; i++)
					bc[i] = PERIODIC;

				vector_dist<dim,float, aggregate<float[dim]> > vd(k_int,box,bc,Ghost<dim,float>(r_cut));

				// Initialize a dist vector
				vd_initialize<dim>(vd, v_cl, k_int);

				vd.template ghost_get<0>();

				//Get verlet list

				openfpm::vector<double> measures;
				double sum_verlet_mean = 0;
				double sum_verlet_dev = 0;
				for (size_t n = 0 ; n < N_STAT_TEST; n++)
					measures.add(benchmark_get_verlet(vd,r_cut));
				standard_deviation(measures,sum_verlet_mean,sum_verlet_dev);

				//Average total time
				time_create_mean.get(r).add(sum_verlet_mean);
				time_create_dev.get(r).add(sum_verlet_dev);

				//Calculate forces

				auto NN = vd.getCellList(r_cut);
				double sum_fr_mean = 0;
				double sum_fr_dev = 0;

				measures.clear();
				for (size_t l = 0 ; l < N_STAT_TEST ; l++)
					measures.add(benchmark_calc_forces<dim>(NN,vd,r_cut));
				standard_deviation(measures,sum_fr_mean,sum_fr_dev);
				time_force_mean.get(r).add(sum_fr_mean);
				time_force_dev.get(r).add(sum_fr_dev);

				if (v_cl.getProcessUnitID() == 0)
					std::cout << "Particles: " << k_int << "," << "cut-off: " << r_cut << " time to construct a Verlet list = " << sum_verlet_mean << " dev: " << sum_verlet_dev << "    calculate force = " << sum_fr_mean << " dev: " << sum_fr_dev << std::endl;
			}
		}
	}
}

/*! \brief Function for verlet test with an Hilbert curve reordering
 *
 */
template<unsigned int dim> void vd_verlet_hilbert_benchmark(size_t k_start, size_t k_min, double ghost_part,openfpm::vector<float> & r_cutoff, openfpm::vector<size_t> & n_particles, openfpm::vector<size_t> &orders, openfpm::vector<openfpm::vector<openfpm::vector<double>>> &time_hilb, openfpm::vector<openfpm::vector<openfpm::vector<double>>> &time_total_hilb)
{
	time_hilb.resize(r_cutoff.size());
	for (size_t r = 0; r < time_hilb.size(); r++)
	{
		time_hilb.get(r).resize(n_particles.size());
		for (size_t k = 0; k < time_hilb.get(r).size(); k++)
		{
			time_hilb.get(r).get(k).resize(orders.size());
		}
	}

	time_total_hilb.resize(r_cutoff.size());
	for (size_t r = 0; r < time_total_hilb.size(); r++)
	{
		time_total_hilb.get(r).resize(n_particles.size());
		for (size_t k = 0; k < time_total_hilb.get(r).size(); k++)
		{
			time_total_hilb.get(r).get(k).resize(orders.size());
		}
	}

	std::string str("Testing " + std::to_string(dim) + "D vector, Hilbert curve reordering, Verlet-list");
	print_test_v(str,0);

	// For different r_cut
	for (size_t r = 0; r < r_cutoff.size(); r++ )
	{
		Vcluster & v_cl = create_vcluster();

		//Cut-off radius
		float r_cut = r_cutoff.get(r);

		// Number of particles
		size_t k = k_start * v_cl.getProcessingUnits();

		//For different curve orders
		for ( size_t i = 0; i < orders.size(); i++)
		{
			size_t m = orders.get(i);
			size_t part = 0;

			for (size_t k_int = k ; k_int >= k_min ; k_int/=2, part++ )
			{
				BOOST_TEST_CHECKPOINT( "Testing " << dim << "D vector with an Hilbert curve reordering k=" << k_int );

				Box<dim,float> box;

				for (size_t i = 0; i < dim; i++)
				{
					box.setLow(i,0.0);
					box.setHigh(i,1.0);
				}

				// Boundary conditions
				size_t bc[dim];

				for (size_t i = 0; i < dim; i++)
					bc[i] = PERIODIC;

				vector_dist<dim,float, aggregate<float[dim]>, CartDecomposition<dim,float> > vd(k_int,box,bc,Ghost<dim,float>(ghost_part));

				// Initialize a dist vector
				vd_initialize<dim>(vd, v_cl, k_int);

				vd.template ghost_get<0>();

				//Reorder a vector

				double sum_reorder = 0;
				for (size_t h = 0 ; h < N_VERLET_TEST; h++)
					sum_reorder += benchmark_reorder(vd,m);
				sum_reorder /= N_VERLET_TEST;

				//Get verlet list

				double sum_verlet = 0;

				for (size_t n = 0 ; n < N_VERLET_TEST; n++)
					sum_verlet += benchmark_get_verlet(vd,r_cut);
				sum_verlet /= N_VERLET_TEST;
				//Average total time
				time_total_hilb.get(r).get(part).get(i) = sum_verlet;

				//Calculate forces

				auto NN = vd.getCellList(r_cut);
				double sum_forces = 0;

				for (size_t l = 0 ; l < N_VERLET_TEST; l++)
					sum_forces += benchmark_calc_forces<dim>(NN,vd,r_cut);
				sum_forces /= N_VERLET_TEST;
				time_hilb.get(r).get(part).get(i) = sum_forces;

				if (v_cl.getProcessUnitID() == 0)
					std::cout << "Order = " << m << ", Cut-off = " << r_cut << ", Particles = " << k_int << ". Time to reorder: " << sum_reorder << " time to get the verlet-list: " << sum_verlet << " time to calculate forces: " << sum_forces << std::endl;
			}
		}
	}
}


/*! \brief Function for verlet performance report
 *
 */
template<unsigned int dim> void vd_verlet_performance_write_report(GoogleChart & cg,
																   openfpm::vector<float> & r_cutoff,
		                                                           openfpm::vector<size_t> & n_particles,
																   openfpm::vector<openfpm::vector<double>> time_force_mean,
																   openfpm::vector<openfpm::vector<double>> time_force_dev,
																   openfpm::vector<openfpm::vector<double>> time_create_mean,
																   openfpm::vector<openfpm::vector<double>> time_create_dev)
{
	{
	std::string file_mean(test_dir);
	std::string file_var(test_dir);
	file_mean += std::string("/openfpm_pdata/verlet_comp_force_mean_" + std::to_string(dim) + std::string("_ref"));
	file_var += std::string("/openfpm_pdata/verlet_comp_force_dev_" + std::to_string(dim) + std::string("_ref"));

	std::string file_mean_save = std::string("verlet_comp_force_mean_" + std::to_string(dim) + std::to_string("_ref"));
	std::string file_var_save = std::string("verlet_comp_force_dev_" + std::to_string(dim) + std::to_string("_ref"));

	openfpm::vector<size_t> xp = n_particles;

	openfpm::vector<openfpm::vector<openfpm::vector<double>>> yp_mean;
	openfpm::vector<openfpm::vector<openfpm::vector<double>>> yp_dev;

	openfpm::vector<std::string> names;
	openfpm::vector<std::string> gnames;

	yp_mean.resize(time_force_mean.size());
	yp_dev.resize(time_force_dev.size());
	for (size_t i = 0 ; i < yp_mean.size() ; i++)
	{
		yp_mean.get(i).resize(time_force_mean.get(i).size());
		yp_dev.get(i).resize(time_force_dev.get(i).size());

		for (size_t j = 0 ; j < yp_mean.get(i).size() ; j++)
		{
			yp_mean.get(i).get(j).resize(1);
			yp_dev.get(i).get(j).resize(1);

			yp_mean.get(i).get(j).get(0) = time_force_mean.get(i).get(j);
			yp_dev.get(i).get(j).get(0) = time_force_dev.get(i).get(j);
		}
	}

	names.add("Force verlet");

	for (size_t i = 0 ; i < r_cutoff.size() ; i++)
		gnames.add("Verlet-list performance, cut-off radius: " + std::to_string(r_cutoff.get(i)));

	std::string y_string = std::string("Time to calculate forces (s)");
	std::string x_string = std::string("Number of particles");

	std::string str("<h1>Verlet-list " + std::to_string(dim) + "-D performance test force calculation: </h1>");
	cg.addHTML(str);

	StandardPerformanceGraph(file_mean,
			                 file_var,
							 file_mean_save,
							 file_var_save,
							 cg,
							 xp,
							 yp_mean,
							 yp_dev,
							 names,
							 gnames,
							 x_string,
							 y_string,
							 true);
	}
	//////////////////// TIME TO CREATE //////////////////////////

	{
	std::string file_mean(test_dir);
	std::string file_var(test_dir);
	file_mean += std::string("/openfpm_pdata/verlet_comp_create_mean_" + std::to_string(dim) + std::string("_ref"));
	file_var += std::string("/openfpm_pdata/verlet_comp_create_dev_" + std::to_string(dim) + std::string("_ref"));

	std::string file_mean_save = std::string("verlet_comp_create_mean_" + std::to_string(dim) + std::to_string("_ref"));
	std::string file_var_save = std::string("verlet_comp_create_dev_" + std::to_string(dim) + std::to_string("_ref"));

	openfpm::vector<size_t> xp = n_particles;

	openfpm::vector<openfpm::vector<openfpm::vector<double>>> yp_mean;
	openfpm::vector<openfpm::vector<openfpm::vector<double>>> yp_dev;

	openfpm::vector<std::string> names;
	openfpm::vector<std::string> gnames;

	yp_mean.resize(time_create_mean.size());
	yp_dev.resize(time_create_dev.size());
	for (size_t i = 0 ; i < yp_mean.size() ; i++)
	{
		yp_mean.get(i).resize(time_create_mean.get(i).size());
		yp_dev.get(i).resize(time_create_dev.get(i).size());

		for (size_t j = 0 ; j < yp_mean.get(i).size() ; j++)
		{
			yp_mean.get(i).get(j).resize(1);
			yp_dev.get(i).get(j).resize(1);

			yp_mean.get(i).get(j).get(0) = time_create_mean.get(i).get(j);
			yp_dev.get(i).get(j).get(0) = time_create_dev.get(i).get(j);
		}
	}

	names.add("Create verlet");

	for (size_t i = 0 ; i < r_cutoff.size() ; i++)
		gnames.add("Verlet-list performance, cut-off radius: " + std::to_string(r_cutoff.get(i)));

	std::string y_string = std::string("Time to construct a verlet-list (s)");
	std::string x_string = std::string("Number of particles");

	std::string str("<h1>Verlet-list " + std::to_string(dim) + "-D performance test force calculation: </h1>");
	cg.addHTML(str);

	StandardPerformanceGraph(file_mean,
			                 file_var,
							 file_mean_save,
							 file_var_save,
							 cg,
							 xp,
							 yp_mean,
							 yp_dev,
							 names,
							 gnames,
							 x_string,
							 y_string,
							 true);
	}
}

BOOST_AUTO_TEST_CASE( vector_dist_verlet_test )
{
	//Benchmark test for 2D and 3D
	vd_verlet_random_benchmark<3>(k_start,k_min,r_cutoff,n_particles,time_force_mean,time_create_mean,time_force_dev,time_create_dev);
	vd_verlet_random_benchmark<2>(k_start,k_min,r_cutoff,n_particles,time_force_mean_2,time_create_mean_2,time_force_dev_2,time_create_dev_2);
}

BOOST_AUTO_TEST_CASE(vector_dist_verlet_performance_write_report)
{
	GoogleChart cg;

	//Write report for 2D and 3D
	vd_verlet_performance_write_report<3>(cg,r_cutoff,n_particles,time_force_mean,time_force_dev,time_create_mean,time_create_dev);
	vd_verlet_performance_write_report<2>(cg,r_cutoff,n_particles,time_force_mean_2,time_force_dev_2,time_create_mean_2,time_create_dev_2);

	if (create_vcluster().getProcessUnitID() == 0)
	{
		addUpdtateTime(cg);

		cg.write("Verletlist_comp.html");
	}
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* SRC_VECTOR_VECTOR_DIST_VERLET_PERFORMANCE_TESTS_HPP_ */
