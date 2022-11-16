make_nn_dag: make_nn_dag.cpp
	g++ -o make_nn_dag make_nn_dag.cpp -std=c++17

make_simon_dag: make_simon_dag.cpp
	g++ -o make_simon_dag make_simon_dag.cpp -std=c++17

txt_to_vcg: txt_to_vcg.cpp ../CPP_code/custom_ddg_format_parser.cpp ../CPP_code/custom_ddg_format_parser.h ../CPP_code/shared_utils.cpp ../CPP_code/shared_utils.h
	g++ -o txt_to_vcg txt_to_vcg.cpp ../CPP_code/custom_ddg_format_parser.cpp ../CPP_code/shared_utils.cpp -std=c++17

clean:
	rm make_nn_dag txt_to_vcg