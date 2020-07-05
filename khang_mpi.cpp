#include <unordered_map>
#include <vector>
#include <map>
#include <iostream>
#include <chrono>
#include <fstream>
#include "mpi.h"
using namespace std;
using namespace std::chrono;
std::vector<int> encoding(std::string s1) 
{ 
    std::cout << "Encoding\n"; 
    std::unordered_map<std::string, int> table; 
    for (int i = 0; i <= 255; i++) { 
        std::string ch = ""; 
        ch += char(i); 
        table[ch] = i; 
    } 
  
    std::string p = "", c = ""; 
    p += s1[0]; 
    int code = 256; 
    std::vector<int> output_code; 
    std::cout << "String\tOutput_Code\tAddition\n"; 
    for (int i = 0; i < s1.length(); i++) { 
        if (i != s1.length() - 1) 
            c += s1[i + 1]; 
        if (table.find(p + c) != table.end()) { 
            p = p + c; 
        } 
        else { 
            std::cout << p << "\t" << table[p] << "\t\t" 
                 << p + c << "\t" << code << std::endl; 
            output_code.push_back(table[p]); 
            table[p + c] = code; 
            code++; 
            p = c; 
        } 
        c = ""; 
    } 
    std::cout << p << "\t" << table[p] << std::endl; 
    output_code.push_back(table[p]); 
    return output_code; 
} 
string get_data()
{
    ifstream ifs("random_text_10M.txt");
    string content((istreambuf_iterator<char>(ifs)),
                   (istreambuf_iterator<char>()));
    return content;
}
std::vector<string> decoding(std::vector<int> op) 
{ 
    std::cout << "Decoding\n"; 
    std::unordered_map<int, std::string> table; 
    std::vector<string> output_ori ;
    for (int i = 0; i <= 255; i++) { 
        std::string ch = ""; 
        ch += char(i); 
        table[i] = ch; 
    } 
    int old = op[0], n; 
    std::string s = table[old]; 
    std::string c = ""; 
    c += s[0]; 
    //std::cout << s; 
    output_ori.push_back(s) ;
    int count = 256; 
    for (int i = 0; i < op.size() - 1; i++) { 
        n = op[i + 1]; 
        if (table.find(n) == table.end()) { 
            s = table[old]; 
            s = s + c; 
        } 
        else { 
            s = table[n]; 
        } 
        output_ori.push_back(s);
        c = ""; 
        c += s[0]; 
        table[count] = table[old] + c; 
        count++; 
        old = n; 
    }
    return output_ori ;
} 
int main(int argc, char** argv) 
{   // Thoi gian bat dau chay
    auto startTime = high_resolution_clock::now();
    int size, rank;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    int signal=1;
    int SERVER_RANK = 0;
    int max_message_length = 256 * 1000;
    // std::string s = get_data(); 
    std::string s = "123456789"; 
    int len = s.size();
    int l = len/size;
    
    // diem xuat phat string cut
    int start = l * rank;
    // diem ket thuc
    int end = l * (rank + 1);
    // Neu la process cuoi cung thi xu ly phan du
    if(rank == size - 1)
        end = len;
    if(rank == SERVER_RANK){
        std::cout <<"Hello FROM: " << rank << "START:" << start << "END: " << end <<"\n";
        std::string s0= s.substr(start,end - start);
        std::vector<int> output_code0 = encoding(s0); 
        std::vector<string> outp =decoding(output_code0);
        for(int i=0; i<outp.size();i++){
            cout<<outp[i];
        }
        MPI_Send( &signal, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - startTime);
        cout << "time: " << duration.count() << endl;
        
    } else {
        std::cout <<"Hello FROM: " << rank << "START:" << start << "END: " << end <<"\n";
        std::string s1= s.substr(start, end - start);
        std::vector<int> output_code1 = encoding(s1); 
        std::vector<string> outp = decoding(output_code1);
        // nhan thong bao process rank - 1  xong

        MPI_Recv( &signal, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);      
        // in ra
        for(int i=0; i<outp.size();i++){
            cout<<outp[i];
        }
        // Thong bao rank + 1 tao da xong
        if(rank < size - 1)
        MPI_Send( &signal, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD); 
        
    }
    MPI_Finalize();
} 