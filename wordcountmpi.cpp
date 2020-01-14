/*
----------------------------------------------------------
COL-730 Project
Implemented by:

Suraj S,
2018MCS2024

Karra Akhilesh
2015CS10233
----------------------------------------------------------
 */

#include "mpi.h"
#include <sys/stat.h>
#include<bits/stdc++.h>
#include<math.h>
using namespace std;
#define MEMORY              256000000 // 256MB of data

size_t lastPosition = 0;

long long int buff_size;
/*
-----------------------------------------------------------
Utility Structure for storing the word and it's count
-----------------------------------------------------------
 */
struct pairs{
        int count;
        char word[30];
    };
/*
-----------------------------------------------------------
Module for reading the input file
-----------------------------------------------------------
 */

char* readFile(FILE* file, size_t fileSize) {
    long long readsize = min((size_t)buff_size, fileSize - lastPosition);
    if (0>=readsize )
        return NULL;

    char* str = new char[readsize + 1];
    long start = 0;
    fseek(file, lastPosition, SEEK_SET); 
    fread(str, 1, readsize, file);
    
    
    if (readsize > 50)
        start = readsize - 50;
    
    char* currptr = strchr(&str[start], ' '), *pre = NULL;
    
    if (readsize < buff_size||currptr == NULL) { 
        lastPosition = fileSize;
        str[readsize] = '\0';
        return str;
    }

    while (!(currptr == NULL)) {
        pre = currptr;
        currptr = strchr(currptr + 1, ' ');
    }
    int rSize = pre - str + 1;
    if (!(pre == NULL)) {
        *pre = '\0';
    }
    lastPosition += rSize;
    return str;
}



int main(int argc, char **argv){
/*
-----------------------------------------------------------
Initialization for MPI 
-----------------------------------------------------------
 */

    int nTotalLines = 0,blocks[2] = {1, 30};
    MPI_Aint charex, intex, displacements[2];
    MPI_Datatype obj_type, types[2] = {MPI_INT, MPI_CHAR};
    
    buff_size = MEMORY;
    double startTime = 0;
    int nTasks, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&nTasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Type_extent(MPI_INT, &intex);
    MPI_Type_extent(MPI_CHAR, &charex);
    displacements[0] = (MPI_Aint)(0);
    displacements[1] = intex;
    MPI_Type_struct(2, blocks, displacements, types, &obj_type);
    MPI_Type_commit(&obj_type);
    
    char *plengFileBuffer; 
    long long int *StartId; 
    startTime = MPI_Wtime();
/*
-----------------------------------------------------------
Reading the input from the file 
<Optional Field> in CLI takes input.txt by default
-----------------------------------------------------------
 */

    size_t total_size;
    FILE* file;
    if (rank == 0) {
        if(argc>=2)
        file = fopen(argv[1], "r+");
        else
        file = fopen("input.txt", "r+");
        struct stat filestatus;
        if(argc>=2)
        stat(argv[1], &filestatus);
        else
        stat("input.txt", &filestatus);
        total_size = filestatus.st_size;
    }
/*
-----------------------------------------------------------
Map for saving the String and it's relative frequency
-----------------------------------------------------------
 */

    double totalTime_noRead = 0,totalTime_NoDist = 0;
    map<string, int> totalHashMap;
    while (true) {
        int status = 1;
        if (rank == 0) {
            StartId = new long long int[buff_size / 10]; 
            plengFileBuffer = readFile(file, total_size);
            StartId[0] = 0;
            if (plengFileBuffer == NULL)
                status = 0;
        }
        nTotalLines = 0;
        MPI_Bcast(&status, 1, MPI_INT, 0,MPI_COMM_WORLD);
        if (status == 0) 
            break;
/*
-----------------------------------------------------------
1.Process with RANK 0 is considered as parent process 

2.All other process gets the data through process 0,
  and here the mapping part is done.
-----------------------------------------------------------
 */

        if (rank == 0) {
            char* currptr = strchr(plengFileBuffer, 10); 
            while (!(currptr == NULL)) {
                StartId[++nTotalLines] = 1+currptr - plengFileBuffer;
                currptr = strchr(currptr + 1, 10);
            }
            StartId[nTotalLines + 1] = strlen(plengFileBuffer);
        }
        double startTime_noRead = MPI_Wtime();
        
        char *buffer = NULL;
        int totalChars = 0, portion = 0,startNum = 0, endNum = 0;

        if (rank == 0) 
        {   startNum = 0;
            portion = nTotalLines / nTasks;
            endNum = portion;
            buffer = new char[StartId[endNum] + 1];
            strncpy(buffer, plengFileBuffer, StartId[endNum]);
            buffer[StartId[endNum]] = '\0';
            for (int i = 1; i <=nTasks-1; i++) {
                    int curStartNum =  portion*i;
                    int curEndNum =   portion*(i + 1) - 1;
                    if (i+1 == nTasks) {
                        curEndNum = nTotalLines;
                    }
                
                if (curStartNum < 0) {
                    curStartNum = 0;
                }
                int curLength = StartId[curEndNum + 1] - StartId[curStartNum];
                MPI_Send(&curLength, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                if (curLength > 0)
                MPI_Send(plengFileBuffer + StartId[curStartNum], curLength, MPI_CHAR, i, 2, MPI_COMM_WORLD);
                                            }
            free(plengFileBuffer);
            free(StartId);
        }
        else  
        {MPI_Status status;
            
            MPI_Recv(&totalChars, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            if (totalChars > 0) {
                buffer = new char[totalChars + 1];
                MPI_Recv(buffer, totalChars, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
                buffer[totalChars] = '\0';
            }
        }
        
        pairs* words = NULL;
        int size = 0;
        double startTime_noDist = MPI_Wtime();
        map<string, int> HashMap;
/*
-----------------------------------------------------------
Counting the occurences of each and every words from the 
part of the text file that is alloted to a process
-----------------------------------------------------------
 */

        if ((buffer != NULL)){
            char* word = strtok(buffer, " ,.\r\n");
            while (!(word == NULL)) {
                //It is needed to be cleansed
                if (HashMap.find(word) != HashMap.end())
                    HashMap[word]++;
                else
                    HashMap[word]=1;
                word = strtok(NULL, " ,.\r\n");
               

            }
            free(buffer);

            size = HashMap.size();
            
            
            if (size > 0) {
                words = (pairs*)malloc(size*sizeof(pairs));
                int i = 0;
                for (map<string, int>::iterator it = HashMap.begin(); it != HashMap.end(); it++) {

                    strcpy(words[i].word, (it->first).c_str());
                    words[i].count = it->second;
                    i++;}}}
        
        
/*
-----------------------------------------------------------
Reduction Operation
-----------------------------------------------------------
 */       
        if (rank == 0) {
            

            for (int i = 1; i < nTasks; i++) {
                int leng;
                MPI_Status status;
                MPI_Recv(&leng, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
                if (leng > 0) {
                    pairs* local_words = (pairs*)malloc(leng*sizeof(pairs));
                    MPI_Recv(local_words, leng, obj_type, i, 4, MPI_COMM_WORLD, &status);

                    for (int j = 0; j < leng; j++) {
                        if(totalHashMap.find(local_words[j].word)!=totalHashMap.end())
                        totalHashMap[local_words[j].word] += local_words[j].count;
                        else
                        totalHashMap[local_words[j].word] = local_words[j].count;
                        
                        
                    }
                    free(local_words);
                }
            }

            for (map<string, int>::iterator it = HashMap.begin(); it != HashMap.end(); it++) {
                totalHashMap[it->first] += it->second;
            }
        } else {
            MPI_Send(&size, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
            if (size > 0) {
            MPI_Send(words, size, obj_type, 0, 4, MPI_COMM_WORLD);
            }
        }
        HashMap.clear();
        if (words != NULL && size > 0)
            delete []words;
        totalTime_noRead += (MPI_Wtime() - startTime_noRead);
        totalTime_NoDist += (MPI_Wtime() - startTime_noDist);
    }
    if (rank == 0) {
        fclose(file);   
        double t = MPI_Wtime();
        FILE * fptr=fopen("outputmpi.txt","w+");
        for (map<string, int>::iterator it = totalHashMap.begin(); it != totalHashMap.end(); it++) {
            fprintf( fptr,"%s:%d\n",(it->first).c_str() ,it->second );
        }
        
        double endTime = MPI::Wtime();
        cout << "Time taken is: " << totalTime_NoDist + endTime - t << " seconds" << endl;
        totalHashMap.clear();
        FILE * ptr2=fopen("resultmpi.txt","a");
        fprintf(ptr2,"%d\t%f\n",nTasks,totalTime_NoDist + endTime - t);
        fclose(ptr2);
        
    }
    MPI_Finalize();
    return 0;  
}