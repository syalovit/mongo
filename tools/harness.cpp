// test.cpp

#include <iostream>
#include <list>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

using namespace std;

size_t fileSize;
string dir;
vector<char*> indexFiles;
vector<char*> dataFiles;


char * getFile( const string& file ){
        time_t start = time( 0 );

    long z = 8192;
    char buf[z];
    memset( buf , 0 , z );
    int nSets = fileSize / z;

    int fd = open( file.c_str() , O_CREAT | O_RDWR | O_NOATIME , S_IRUSR | S_IWUSR );
    assert( fd > 0 );    
    size_t old = lseek( fd , 0 , SEEK_END );
    assert( old <= fileSize );
    if ( old != fileSize ){
        
        assert( fileSize - 1 == lseek( fd , fileSize - 1 , SEEK_SET ) );
        assert( lseek( fd , 0 , SEEK_SET ) == 0 );
        for ( int i=0; i<nSets; i++ )
            assert( z == write( fd , buf , z ) );
        close( fd );
        
        fd = open( file.c_str() , O_CREAT | O_RDWR | O_NOATIME , S_IRUSR | S_IWUSR );        
        assert( fd );
    }

    
    char * m = (char*)mmap( 0 , fileSize , PROT_READ|PROT_WRITE , MAP_SHARED   , fd , 0 );
    assert( m );
    memcpy( m , "eliot" , 6 );    
        time_t end = time( 0 );
    return m;
}



string makeFile( const string& type , int num ){
    stringstream ss;
    ss << dir << "/" << type << "." << num;
    return ss.str();
}

void writeIndex(){
    for ( int i=0; i<5; i++ ){
        int f = rand() % indexFiles.size();
        indexFiles[f][rand()%fileSize] = 5;
    }
}

void readIndex(){
   
    for ( int i=0; i<5; i++ ){
        int f = rand() % indexFiles.size();
        int idx = indexFiles[f][rand()%fileSize];
    }
}

void readData(){
    if (dataFiles.size() > 0)
    {
        for ( int i=0; i<5; i++ ){
            int f = rand() % dataFiles.size();
            int idx = dataFiles[f][rand()%fileSize];
        }
    }
}

void writeData(){
    if (dataFiles.size() > 0)
    for ( int i=0; i<5; i++ ){
        int f = rand() % dataFiles.size();
        dataFiles[f][rand()%fileSize] = 5;
    }
}





int main( int argc , char ** argv ){
    if ( argc != 9 ){
        cout << "usage: " << argv[0] << " <dir> <file size (mb)> <index size (mb)> <data size(mb)> <read ratio> <rand write ratio> <sleep time> <operations per cycle>" << endl;
        return -1;
    }
    std::cout << "parameters: ";
    for (int j = 1; j < argc; j++)
    std::cout << argv[j] << " ";
    std::cout << std::endl;   
    dir = argv[1];
    fileSize = (size_t)atoi( argv[2] ) * 1024 * 1024;
    int readRatio = atoi(argv[5]);
    int readCounter = 0;
    int randWriteCounter = 0;
    int randWriteRatio = atoi(argv[6]);
    int sleepTime = atoi(argv[7]);
    int OPERATION_BLOCK = atoi(argv[8]);
    size_t indexSize = (size_t)atoi( argv[3] ) * 1024 * 1024;
    size_t dataSize = (size_t)atoi( argv[4] ) * 1024 * 1024;
    
    list<int> fds;
    
    int numIndexFiles = indexSize / fileSize;
    int numDataFiles = dataSize / fileSize;
    cout << "numIndexFiles:\t" << numIndexFiles << endl;
    cout << "numDataFiles:\t" << numDataFiles << endl;

    for ( int i=0; i<numIndexFiles; i++ ){
        string fn = makeFile( "index" , i );
        cout << fn << endl;
        indexFiles.push_back( getFile( fn ) );
    }

    int recordSize = 8192;
    char record[8192];
    for ( int i=0; i<recordSize-1; i++ ){
        record[i] = (char)rand();
    }

    record[recordSize-1] = 0;
    size_t numRecordsPerFile = fileSize / recordSize;
    int a = 0;
    string f = makeFile( "data" , a );
    char * d = getFile( f );
    dataFiles.push_back( d );
    int b = 0;
    int allRecordsWritten = 0; 
    for ( int iterations = 0; iterations < 100; iterations++)
    {
        time_t start = time( 0 );
        time_t t = start;
        std::vector<int> growthTime;
        for (int opCounter = 0; (opCounter < OPERATION_BLOCK); opCounter++ )
        {
            if (readCounter < readRatio*OPERATION_BLOCK)
            {
                readIndex();
                readData();
                readCounter++;
            }
            else
            {
                writeIndex();
                char *LOC = d;
                if (randWriteCounter < randWriteRatio*OPERATION_BLOCK)
                {
                    if (dataFiles.size() > 0)
                    {
                        int f = rand() % dataFiles.size();
                        LOC = dataFiles[f];
                    }
                    memcpy( LOC , record , recordSize );
                    randWriteCounter++;
                }
                else
                { 
                    memcpy( LOC , record , recordSize );
                    b++;
                    d += recordSize;
                }
                if (b>=numRecordsPerFile)
                {
                    time_t middle = time(0);
                    std::string f = makeFile( "data" , a);
                    d = getFile ( f );
                    dataFiles.push_back( d );
                    allRecordsWritten = allRecordsWritten + b;
                    b = 0;
                    a++;
                    time_t restart = time(0);
                    std::cout << "total records written/time to grow: " << allRecordsWritten<<"," << (restart - middle) << std::endl;
                    growthTime.push_back(restart-middle);
                }
            }
            usleep(1000*sleepTime);
        }
        time_t end = time(0);
        for (int i = 0; i < growthTime.size(); i++)
            end = end - growthTime[i];
        std::cout << "time: " << (end - start - sleepTime*OPERATION_BLOCK/1000) << std::endl;
        readCounter = 0;
        randWriteCounter = 0;
    }
}
