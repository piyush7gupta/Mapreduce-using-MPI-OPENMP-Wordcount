#include<bits/stdc++.h>
#include<omp.h>
#include<stdio.h>
#include<string.h>
using namespace std;
#define NUMPROCS 4

struct word_reader : std::ctype<char> {
    word_reader(std::string const &delims) : std::ctype<char>(get_table(delims)) {}
    static std::ctype_base::mask const* get_table(std::string const &delims) {
        static std::vector<std::ctype_base::mask> rc(table_size, std::ctype_base::mask());

        for (char ch : delims)
            rc[ch] = std::ctype_base::space;
        return &rc[0];
    }
};
int main(int argc,char **argv){
    if(argc==1)
    omp_set_num_threads(NUMPROCS);
    else
    omp_set_num_threads(atoi(argv[1]));
    int num=NUMPROCS;
    if(argc>1)
    num=atoi(argv[1]);
    string str;
    vector<string> vals;
    ifstream inpstream;
   inpstream.open("input.txt");
   while(getline(inpstream,str)){
       istringstream in(str);
       in.imbue(std::locale(std::locale(), new word_reader(" ,.\r\n")));
    std::string word;

    while (in >> word){
        //std::cout << word << "\n";
        vals.push_back(word);
    }
   }
    int len=vals.size();
   string *arr=new string[len];
   double starttime=omp_get_wtime();
   for(int i=0;i<len;i++){
       arr[i]=vals[i];
   }
   map<string,int> mapper[num]; 
   #pragma omp parallel for
       for(int i=0;i<len;i++)
       {   if(mapper[omp_get_thread_num()].find(arr[i])!=mapper[omp_get_thread_num()].end()){
               mapper[omp_get_thread_num()][arr[i]]=1;
            
           }
           else
            {
                mapper[omp_get_thread_num()][arr[i]]+=1;
            }
            
       }
    map <string,int> res;
    #pragma omp critical
    {   for(int i=0;i<num;i++)
        for (map<string, int>::iterator it = mapper[i].begin(); it != mapper[i].end(); it++) {
                if(res.find(it->first)!=res.end())
                res[it->first] += it->second;
                else
                res[it->first] = it->second;
            }
    }
    cout<<omp_get_wtime()-starttime<<endl;
    FILE * ptr2=fopen("resultopenmp.dat","a");
        fprintf(ptr2,"%d\t%f\n",num,omp_get_wtime()-starttime);
        fclose(ptr2);
    FILE *fptr=fopen("outputopenmp.txt","w+");
        for (map<string, int>::iterator it = res.begin(); it != res.end(); it++) 
        fprintf(fptr,"%s : %d\n",(it->first).c_str(),it->second);
    
        
   }

