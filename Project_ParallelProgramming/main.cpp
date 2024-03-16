/*
HW3 by Durga Naidu.
SUID 422434702
values to be updated for testing are kept at the top of global variables declartion.
*/
#pragma warning(disable: 4996) //ctime only for visual studio
#include <iostream>
#include <thread>
#include <vector>
#include<mutex>
//#include<string>
using namespace std;
mutex m1, m2;
//values to be updated for testing
const int MAX_TIME_PART{ 30000 }, MAX_TIME_PRODUCT{ 28000 };
const int no_part_workers = 4, no_product_workers = 4; //m: number of Part Workers
struct Parts {
	int id;
	int make_part_time;
	int part_buffer_time;
	int buffer_capacity;
	int buffer_product_time;
	int assemble_time;
}Part[5];

//global variables
vector<chrono::time_point<chrono::system_clock>> acc_part_time(no_part_workers);
vector<chrono::time_point<chrono::system_clock>> acc_prod_time(no_product_workers);
vector<vector<int>> load_order(no_part_workers, vector<int>(5, 0));
vector<vector<int>> pickup_order(no_product_workers, vector<int>(5, 0));
vector<vector<int>> cart_state(no_product_workers, vector<int>(5, 0));
vector<int> buffer_state(5);
vector<int> itr_Part(no_part_workers);
vector<int> itr_Product(no_product_workers);
chrono::system_clock::time_point Begin;
chrono::system_clock::time_point End;
chrono::system_clock::duration RunTime;
condition_variable cv1, cv2;

//function Declarations and definitions
//bool check_cond_part(int id);
//bool check_cond_prod(int id);
void partWorker(int id, string s);
void productWorker(int id, string s);
void partsdeclare();

int main() {
	cout << "STARTTT!!!!" << endl;

	//parts declarations
	partsdeclare();
	Begin = chrono::system_clock::now();

	//printing hardcoded values related to parts
	/*for (int i = 0; i < 5; i++) {
		cout << "Main---->";
		cout << "id " << Part[i].id << " make_part_time " << Part[i].make_part_time << " part_buffer_time " <<
			Part[i].part_buffer_time << " buffer_capacity " << Part[i].buffer_capacity <<
			" buffer_product_time " << Part[i].buffer_product_time <<" assemble_time "<< Part[i].assemble_time << endl;
	}*/

	//calling threads for functions and joining


	vector<thread> PartW, ProductW;

	for (int i = 0; i < no_part_workers; ++i) {
		PartW.emplace_back(partWorker, i, "New Load Order");
	}
	for (int i = 0; i < no_product_workers; ++i) {
		ProductW.emplace_back(productWorker, i, "New Load Order");
	}
	for (auto& i : PartW) i.join();
	for (auto& i : ProductW) i.join();

	/*//for testing
	for (int i = 0; i < 5; i++) {
		buffer_state[i] = Part[i].buffer_capacity;
	}*/



	//printing 2-D vector Parts P
	/*for (int i = 0; i < no_part_workers; ++i) {
		cout << "for id " << i <<"   ";
		for (int j = 0; j < 5; j++)
			cout << P[i][j] << "  " ;
		cout << endl;
	}*/


	return 0;
}
void partWorker(int id, string s) {
	unique_lock<mutex> ul1(m1);
	if (itr_Part[id] > 6)
		return;
	if (itr_Part[id] == 0) {
		acc_part_time[id] = chrono::system_clock::now();
	}

	RunTime = chrono::system_clock::now() - acc_part_time[id];
	itr_Part[id]++;
	//for different values for different threads, in multiple executions
	srand(time(nullptr) + id);
	auto current_time = chrono::system_clock::now();
	time_t current_time_t = chrono::system_clock::to_time_t(current_time);
	cout << endl << endl;
	cout << "partWorker " << id << "--->Current Time " << ctime(&current_time_t) << endl;
	cout << "partWorker--->Iteration " << itr_Part[id] << endl;
	cout << "partWorker " << id << "--->Part Worker ID: " << id << endl;
	cout << "partWorker " << id << "--->Status: " << s << endl;
	//needs work
	cout << "productWorker " << id << "---->Accumulated Wait Time: " << chrono::duration_cast<chrono::milliseconds>(RunTime).count() <<
		"us" << endl;
	int sum = load_order[id][0] + load_order[id][1] + load_order[id][2] + load_order[id][3] + load_order[id][4];
	for (int j = 0; j < 6 - sum; ++j)
	{
		int a = rand() % 5;
		if (load_order[id][a] < Part[a].buffer_capacity) {
			load_order[id][a]++;
			std::this_thread::sleep_for(std::chrono::microseconds(Part[a].make_part_time));
		}
		else {
			--j;
		}
		//can all be more than buffer capacity? no way
	}
	cout << "partWorker " << id << "--->Buffer State: " << "(";
	for (int j = 0; j < 5; ++j)
		cout << buffer_state[j] << ",";
	cout << ")" << endl;

	cout << "partWorker " << id << "--->Load Order : " << "(";
	for (int j = 0; j < 5; ++j)
		cout << load_order[id][j] << ",";
	cout << ")" << endl;
	int time = 0;
	for (int i = 0; i < 5; i++) {
		time = Part[i].part_buffer_time * load_order[id][i];
		std::this_thread::sleep_for(std::chrono::microseconds(time));
	}
	for (int i = 0; i < 5; i++) {
		if (Part[i].buffer_capacity > buffer_state[i]) {
			if (Part[i].buffer_capacity - buffer_state[i] > load_order[id][i]) {
				//	cout << "partWorker--->inside gretaer buffer state" << endl;
				buffer_state[i] += load_order[id][i];
				load_order[id][i] = 0;
			}
			else {
				//	cout << "partWorker--->on the lower side" << endl;
				load_order[id][i] -= Part[i].buffer_capacity - buffer_state[i];
				buffer_state[i] = Part[i].buffer_capacity;
			}
		}
	}
	cv2.notify_all();
	cv1.notify_one();
	cout << "partWorker " << id << "--->Updated Buffer State: " << "(";
	for (int j = 0; j < 5; ++j)
		cout << buffer_state[j] << ",";
	cout << ")" << endl;

	cout << "partWorker " << id << "--->UPdated Load Order : " << "(";
	for (int j = 0; j < 5; ++j)
		cout << load_order[id][j] << ",";
	cout << ")" << endl;

	int stat = 0;
	if (load_order[id][0] == 0 && load_order[id][1] == 0 && load_order[id][2] == 0 && load_order[id][3] == 0 && load_order[id][4] == 0) {
		//	cout << "partWorker " << id << "going to be calling New Load Order " << endl;
		ul1.unlock();
		partWorker(id, " New Load Order ");
	}
	else {
		if (cv1.wait_for(ul1, std::chrono::microseconds(MAX_TIME_PART)), [] {return false; }) {
			stat = 1;
			cout << "partWorker " << id << "time out" << endl;
		}
		cout << "partWorker " << id << "--->resuming the operation" << endl;
		for (int i = 0; i < 5; i++) {
			time = Part[i].part_buffer_time * load_order[id][i];
			std::this_thread::sleep_for(std::chrono::microseconds(time));
		}
		if (stat == 1)
		{
			//	cout << "partWorker " << id << "going to be calling Wakeup-TimeOUT " << endl;
			ul1.unlock();
			partWorker(id, " Wakeup-TimeOUT ");
		}
		else if (stat == 0) {
			//	cout << "partWorker " << id << "going to be calling Wakeup-Notified " << endl;
			ul1.unlock();
			partWorker(id, " Wakeup-Notified");
		}
		/*unique_lock<mutex> ul2(m2);
		cout << "partworker " << id << "---->out of the godamm loop" << endl;
		ul2.unlock();*/
	}
	/*unique_lock<mutex> ul2(m2);
	cout << "partworker " << id << " returning from the iterations" << endl;
	ul2.unlock();*/
	return;
}
void productWorker(int id, string s) {
	unique_lock<mutex> ul1(m1);
	if (itr_Product[id] > 6)
		return;
	if (itr_Product[id] == 0) {
		acc_prod_time[id] = chrono::system_clock::now();
	}

	RunTime = chrono::system_clock::now() - acc_prod_time[id];
	itr_Product[id]++;
	//for creating different values for everythread, in every loop
	srand(time(nullptr) + id);

	auto current_time = chrono::system_clock::now();
	time_t current_time_t = chrono::system_clock::to_time_t(current_time);
	cout << "productWorker " << id << "--->Current Time " << ctime(&current_time_t) << endl;
	cout << "productWorker---->Iteration " << itr_Product[id] << endl;
	cout << "productWorker " << id << "---->Product Worker ID: " << id << endl;
	cout << "productWorker " << id << "---->Status: " << s << endl;
	//needs work
	cout << "productWorker " << id << "---->Accumulated Wait Time: " << chrono::duration_cast<chrono::milliseconds>(RunTime).count() <<
		"us" << endl;
	if (s == "New Load Order") {
		int j = 2 + rand() % 2;
		//cout << "productWorker---->j is " << j << endl;
		int random = 0;
		for (int i = 0; i < j; i++) {
			random = rand() % 5;
			/*	cout << "productWorker---->random is " << random << endl;
				cout << "productWorker---->pickup order before is " << pickup_order[id][random] << endl;*/
			if (pickup_order[id][random] == 0)
				pickup_order[id][random]++;
			else
				i--;
			//	cout << "productWorker---->pickup order now is " << pickup_order[id][random] << endl;
		}
		int total = 5 - j - 1;
		while (total >= 0) {
			random = rand() % 5;
			//cout << "productWorker---->inside second loop " << random << endl;
			if (pickup_order[id][random] != 0) {
				pickup_order[id][random]++;
				total--;
				//	cout << "productWorker---->now total is " << total << endl;
			}
			/*else
				cout<<"productWorker---->it is zero"<<endl;*/
		}
	}
	cout << "productWorker " << id << "---->Buffer State: " << "(";
	for (int j = 0; j < 5; ++j)
		cout << buffer_state[j] << ",";
	cout << ")" << endl;

	cout << "productWorker " << id << "---->pick up Order : " << "(";
	for (int j = 0; j < 5; ++j)
		cout << pickup_order[id][j] << ",";
	cout << ")" << endl;
	int time = 0;
	for (int i = 0; i < 5; i++) {
		time = pickup_order[id][i] * Part[i].buffer_product_time;
		cart_state[id][i] = pickup_order[id][i];
		this_thread::sleep_for(std::chrono::microseconds(time));
	}
	for (int i = 0; i < 5; i++) {
		if (buffer_state[i] > 0)
			if (pickup_order[id][i] < buffer_state[i]) {
				//cout << "productWorker---->inside lesser buffer state" << endl;
				buffer_state[i] -= pickup_order[id][i];
				pickup_order[id][i] = 0;
			}
			else {
				//	cout << "productWorker---->on the higher side" << endl;
				pickup_order[id][i] -= buffer_state[i];
				buffer_state[i] = 0;
			}
		/*else
			cout << "productWorker " << id << "---->buffer state is zero "<<buffer_state[i] << endl;*/

	}
	cv1.notify_all();
	cv2.notify_one();
	cout << "productWorker " << id << "---->Updated Buffer State: " << "(";
	for (int j = 0; j < 5; ++j)
		cout << buffer_state[j] << ",";
	cout << ")" << endl;

	cout << "productWorker " << id << "---->UPdated pickup Order : " << "(";
	for (int j = 0; j < 5; ++j)
		cout << pickup_order[id][j] << ",";
	cout << ")" << endl;

	int stat = 0;
	if (pickup_order[id][0] == 0 && pickup_order[id][1] == 0 && pickup_order[id][2] == 0 && pickup_order[id][3] == 0 && pickup_order[id][4] == 0) {
		//cout << "partWorker " << id << "going to be calling New Load Order " << endl;
		for (int i = 0; i < 5; i++) {
			time = cart_state[id][i] * Part[i].assemble_time;
			this_thread::sleep_for(std::chrono::microseconds(time));
		}
		ul1.unlock();
		productWorker(id, "New Load Order");
	}
	else {
		if (cv2.wait_for(ul1, std::chrono::microseconds(MAX_TIME_PRODUCT), [] {return false; })) {
			stat = 1;
			cout << "productWorker " << id << "--->time out " << endl;
		}
		//cout << "productWorker " << id << "--->resuming the operation" << endl;
		if (stat == 1)
		{
			//cout << "productWorker " << id << "going to be calling Wakeup-TimeOUT " << endl;
			ul1.unlock();
			productWorker(id, " Wakeup-TimeOUT ");
		}
		else if (stat == 0) {
			//cout << "productWorker " << id << "going to be calling Wakeup-Notified " << endl;
			ul1.unlock();
			productWorker(id, " Wakeup-Notified");
		}
		/*unique_lock<mutex> ul2(m2);
		cout << "productWorker " << id << "---->out of the godamm loop" << endl;*/

	}
	//unique_lock<mutex> ul2(m2);
	//cout << "productWorker " << id << "back from iterations " << endl;
	return;
}

void partsdeclare() {
	//cout << "parsdeclare--->START!!<" << endl;
	Part[0].id = 1;//id will be i-1
	Part[0].make_part_time = 500;
	Part[0].part_buffer_time = 200;
	Part[0].buffer_capacity = 7;
	Part[0].buffer_product_time = 200;
	Part[0].assemble_time = 600;

	Part[1].id = 2;
	Part[1].make_part_time = 500;
	Part[1].part_buffer_time = 200;
	Part[1].buffer_capacity = 6;
	Part[1].buffer_product_time = 200;
	Part[1].assemble_time = 600;

	Part[2].id = 3;
	Part[2].make_part_time = 600;
	Part[2].part_buffer_time = 300;
	Part[2].buffer_capacity = 5;
	Part[2].buffer_product_time = 300;
	Part[2].assemble_time = 700;

	Part[3].id = 4;
	Part[3].make_part_time = 600;
	Part[3].part_buffer_time = 300;
	Part[3].buffer_capacity = 5;
	Part[3].buffer_product_time = 300;
	Part[3].assemble_time = 700;

	Part[4].id = 5;
	Part[4].make_part_time = 700;
	Part[4].part_buffer_time = 400;
	Part[4].buffer_capacity = 4;
	Part[4].buffer_product_time = 400;
	Part[4].assemble_time = 800;
	//cout << "parsdeclare--->ENd!!" << endl;
	return;
}