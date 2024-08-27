#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 512

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int block_size;


public:
    fsInode(int _block_size) {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;
    }

    int getFileSize(){
        return fileSize;
    }

    int getBlockInUse(){
        return block_in_use;
    }

    int getDirectBlock1(){
        return directBlock1;
    }

    int getDirectBlock2(){
        return directBlock2;
    }

    int getDirectBlock3(){
        return directBlock3;
    }

    int getSingleIndirect(){
        return singleInDirect;
    }

    int getDoubleIndirect(){
        return doubleInDirect;
    }

    void addFileSize(int fileSize){
        this->fileSize+=fileSize;
    }

    void addBlockInUse(int block_in_use){
        this->block_in_use+=block_in_use;
    }

    void setDirectBlock1(int directBlock1){
        this->directBlock1=directBlock1;
    }

    void setDirectBlock2(int directBlock2){
        this->directBlock2=directBlock2;
    }

    void setDirectBlock3(int directBlock3){
        this->directBlock3=directBlock3;
    }

    void setSingleIndirect(int singleInDirect){
        this->singleInDirect=singleInDirect;
    }

    void setDoubleIndirect(int doubleInDirect){
        this->doubleInDirect=doubleInDirect;
    }

    void zeroSize(){
        fileSize=0;
    }
};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;

    }



    string getFileName() {
        return file.first;
    }

    fsInode* getInode() {

        return file.second;

    }

    int GetFileSize() {
        return file.second->getFileSize();
    }
    bool isInUse() {
        return (inUse);
    }
    void setInUse(bool _inUse) {
        inUse = _inUse ;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

int fd_opened(vector<FileDescriptor> v,string name){
    if(!v.empty()){
        for(auto it=v.begin();it!=v.end();it++){
            if(it->getFileName()==name){
                if(it->isInUse()==true) {
                    return 1;
                }
            }
        }
    }
    return -1;
}

int fd_created(map<string, fsInode*> m,string name){
    if(!m.empty()){
        for(auto it=m.begin();it!=m.end();it++){
            if(it->first==name){
                return 1;
            }
        }
        return -1;
    }
    return -1;
}

int find_openedFd(vector<FileDescriptor> v,string name){
    if(!v.empty()){
        int i=0;
        for(auto it=v.begin();it!=v.end();it++){
            if(it->getFileName()==name){
                return i;
            }
            i++;
        }
        return i;
    }
    return 0;
}

int findOldest(vector<FileDescriptor> v){
    if(!v.empty()){
        int i=0;
        for(auto it=v.begin();it!=v.end();it++) {
            if (it->isInUse() == false) {
                return i;
            }
            i++;
        }
    }
    return -1;
}

int emptyBlocks(int* arr,int length){
    for(int i=0;i<length;i++){
        if(arr[i]==0)
            return i;
    }
    return -1;
}

int emptyPlaceInBlock(FILE *f,int blockNum,int blockSize,int *position){
    *position=0;
    int x= fseek(f,blockNum*blockSize,SEEK_SET);
    char y;
    for(int i=0;i<blockSize;i++){
        int z= fread(&y , 1 , 1, f);
        if(y=='\0'){
            return 1;
        }
        (*position)++;
    }
    return 0;
}

void wordChanger(char* word, int length, int start){
    char* newWord=new char[length-start+1];
    int j=0;
    for(int i=start;i<length;i++){
        newWord[j]=word[i];
        j++;
    }
    newWord[j]='\0';
    strcpy(word,newWord);
    delete[] newWord;
}

int indirectEmptyBlock(int* arr,int length){
    int counter=0;
    for(int i=0;i<length;i++){
        if(arr[i]==0){
            counter++;
        }
    }
    return counter;
}

int indirectSpace(FILE *f,int blockNum,int blockSize,int *pointer,int *position){
    *pointer=0;
    *position=0;
    char y;
    char m;
    for(int i=0;i<blockSize;i++){
        fseek(f,(blockNum*blockSize)+i,SEEK_SET);
        int z= fread(&y , 1 , 1, f);
        if(y=='\0'){
            return 1;
        }
        for(int j=0;j<blockSize;j++){
            int a=y;
            fseek(f,(a*blockSize)+j,SEEK_SET);
            int l=fread(&m , 1 , 1, f);
            if(m=='\0'){
                return 1;
            }
            (*position)++;
        }
        *position=0;
        (*pointer)++;
    }
    return -1;
}

int doubleIndirectSpace(FILE *f,int blockNum,int blockSize,int *pointer,int *pointer2,int* position){
    *pointer=0;
    char y;
    for(int i=0;i<blockSize;i++){
        fseek(f,(blockNum*blockSize)+i,SEEK_SET);
        int x=fread(&y,1,1,f);
        if(y=='\0'){
            return 1;
        }
        int s=y;
        if(indirectSpace(f,s,blockSize,pointer2,position)!=-1){
            return 1;
        }
        (*pointer2)=0;
        (*pointer)++;
    }
    return -1;
}

int freeBlocks(int* arr,int size){
    int counter=0;
    for(int i=0;i<size;i++) {
        if (arr[i]==0){
            counter++;
        }
    }
    return counter;
}

int findDeleted(vector<FileDescriptor> v,string name){
    if(!v.empty()){
        int i=0;
        for(auto it=v.begin();it!=v.end();it++) {
            if (it->getFileName()==name && it->isInUse() == false) {
                return i;
            }
            i++;
        }
    }
    return -1;
}

// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated;
    int block_size;
    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    vector<fsInode*> deleted;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector< FileDescriptor > OpenFileDescriptors;


public:
    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "r+" );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
        is_formated=false;
    }



    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: "
                 << it->isInUse() << " file Size: " << it->GetFileSize() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
            cout << bufy;
        }
        cout << "'" << endl;


    }

    // ------------------------------------------------------------------------
    void fsFormat( int blockSize =4 ) {
        if(blockSize>1 && blockSize<DISK_SIZE){
            if(is_formated==true){
                for(int i=0;i<deleted.size();i++){
                    delete deleted.at(i);
                }
                delete[] BitVector;
                for(auto it=MainDir.begin();it!=MainDir.end();it++){
                    delete it->second;
                }
            }
            block_size=blockSize;
            is_formated=true;
            BitVectorSize=DISK_SIZE/blockSize;
            BitVector=new int[BitVectorSize];
            MainDir.clear();
            OpenFileDescriptors.clear();
            for (int i=0; i < DISK_SIZE ; i++) {
                int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
                ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
                assert(ret_val == 1);
            }
            for(int i=0;i<BitVectorSize;i++){
                BitVector[i]=0;
            }
            cout << "FORMAT DISK: number of blocks: "<<BitVectorSize << "\n";
        }
        else{
            cout << "ERR\n";
        }
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if(is_formated==true){
            int position=find_openedFd(OpenFileDescriptors,fileName);
            if(fd_created(MainDir,fileName)==-1) {
                fsInode *node = new fsInode(block_size);
                MainDir.insert(make_pair(fileName, node));
                FileDescriptor fd(fileName, node);
                int x = findOldest(OpenFileDescriptors);
                if (x == -1) {
                    OpenFileDescriptors.push_back(fd);
                    return position;
                } else {
                    OpenFileDescriptors[x] = fd;
                    return x;
                }
            }
        }
        cout << "ERR\n";
        return -1;
    }

    // ------------------------------------------------------------------------
    int OpenFile(string FileName ) {
        if(fd_created(MainDir,FileName)==1 && fd_opened(OpenFileDescriptors,FileName)==-1){
            int position=find_openedFd(OpenFileDescriptors,FileName);
            FileDescriptor fd(FileName,MainDir[FileName]);
            int x = findOldest(OpenFileDescriptors);
            if (x == -1) {
                OpenFileDescriptors.push_back(fd);
                return position;
            } else {
                OpenFileDescriptors[x] = fd;
                return x;
            }
        }
        cout << "ERR\n";
        return -1;
    }


    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        if (OpenFileDescriptors.size() > fd && fd >= 0) {
            if (OpenFileDescriptors[fd].isInUse() == true) {
                string name = OpenFileDescriptors[fd].getFileName();
                OpenFileDescriptors[fd].setInUse(false);
                return name;
            }
        }
        cout << "ERR\n";
        return "-1";
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len ) {
        int place=0;
        int max=(3+block_size+block_size*block_size)*block_size;
        if(fd<OpenFileDescriptors.size() && fd>=0) {
            if (OpenFileDescriptors[fd].isInUse() == true && 
                 OpenFileDescriptors[fd].getInode()->getFileSize() < max) {
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock1() == -1) {
                    if (len <= block_size) {
                        if (emptyBlocks(BitVector, BitVectorSize) != -1) {
                            OpenFileDescriptors[fd].getInode()->setDirectBlock1(emptyBlocks(BitVector, BitVectorSize));
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock1();
                            BitVector[position] = 1;
                            for(int i=0;i<block_size;i++){
                                fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            fseek(sim_disk_fd, position * block_size, SEEK_SET);
                            fwrite(buf, sizeof(char), len, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                        }
                        return 1;
                    } else {
                        if (emptyBlocks(BitVector, BitVectorSize) != -1) {
                            OpenFileDescriptors[fd].getInode()->setDirectBlock1(emptyBlocks(BitVector, BitVectorSize));
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock1();
                            BitVector[position] = 1;
                            fseek(sim_disk_fd, position * block_size, SEEK_SET);
                            fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            wordChanger(buf, len, block_size);
                            len -= block_size;
                        }
                    }
                } else {
                    if (emptyPlaceInBlock(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDirectBlock1(),
                                          block_size, &place) == 1) {
                        if (len <= block_size - place) {
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock1();
                            fseek(sim_disk_fd, (position * block_size) + place, SEEK_SET);
                            fwrite(buf, sizeof(char), len, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            return 1;
                        } else {
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock1();
                            fseek(sim_disk_fd, (position * block_size) + place, SEEK_SET);
                            fwrite(buf, sizeof(char), block_size - place, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size - place);
                            wordChanger(buf, len, block_size - place);
                            len -= block_size - place;
                        }
                    }
                }
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock2() == -1) {
                    if (len <= block_size) {
                        if (emptyBlocks(BitVector, BitVectorSize) != -1) {
                            OpenFileDescriptors[fd].getInode()->setDirectBlock2(emptyBlocks(BitVector, BitVectorSize));
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock2();
                            BitVector[position] = 1;
                            for(int i=0;i<block_size;i++){
                                fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            fseek(sim_disk_fd, position * block_size, SEEK_SET);
                            fwrite(buf, sizeof(char), len, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                        }
                        return 1;
                    } else {
                        if (emptyBlocks(BitVector, BitVectorSize) != -1) {
                            OpenFileDescriptors[fd].getInode()->setDirectBlock2(emptyBlocks(BitVector, BitVectorSize));
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock2();
                            BitVector[position] = 1;
                            fseek(sim_disk_fd, position * block_size, SEEK_SET);
                            fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            wordChanger(buf, len, block_size);
                            len -= block_size;
                        }
                    }
                } else {
                    if (emptyPlaceInBlock(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDirectBlock2(),
                                          block_size, &place) == 1) {
                        if (len <= block_size - place) {
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock2();
                            fseek(sim_disk_fd, (position * block_size) + place, SEEK_SET);
                            fwrite(buf, sizeof(char), len, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            return 1;
                        } else {
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock2();
                            fseek(sim_disk_fd, (position * block_size) + place, SEEK_SET);
                            fwrite(buf, sizeof(char), block_size - place, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size - place);
                            wordChanger(buf, len, block_size - place);
                            len -= block_size - place;
                        }
                    }
                }
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock3() == -1) {
                    if (len <= block_size) {
                        if (emptyBlocks(BitVector, BitVectorSize) != -1) {
                            OpenFileDescriptors[fd].getInode()->setDirectBlock3(emptyBlocks(BitVector, BitVectorSize));
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock3();
                            BitVector[position] = 1;
                            for(int i=0;i<block_size;i++){
                                fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            fseek(sim_disk_fd, position * block_size, SEEK_SET);
                            fwrite(buf, sizeof(char), len, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                        }
                        return 1;
                    } else {
                        if (emptyBlocks(BitVector, BitVectorSize) != -1) {
                            OpenFileDescriptors[fd].getInode()->setDirectBlock3(emptyBlocks(BitVector, BitVectorSize));
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock3();
                            BitVector[position] = 1;
                            fseek(sim_disk_fd, position * block_size, SEEK_SET);
                            fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            wordChanger(buf, len, block_size);
                            len -= block_size;
                        }
                    }
                } else {
                    if (emptyPlaceInBlock(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDirectBlock3(),
                                          block_size, &place) == 1) {
                        if (len <= block_size - place) {
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock3();
                            fseek(sim_disk_fd, (position * block_size) + place, SEEK_SET);
                            fwrite(buf, sizeof(char), len, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            return 1;
                        } else {
                            int position = OpenFileDescriptors[fd].getInode()->getDirectBlock3();
                            fseek(sim_disk_fd, (position * block_size) + place, SEEK_SET);
                            fwrite(buf, sizeof(char), block_size - place, sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size - place);
                            wordChanger(buf, len, block_size - place);
                            len -= block_size - place;
                        }
                    }
                }
                if (OpenFileDescriptors[fd].getInode()->getSingleIndirect() == -1){
                    if(indirectEmptyBlock(BitVector,BitVectorSize)>=2) {
                        OpenFileDescriptors[fd].getInode()->setSingleIndirect(emptyBlocks(BitVector, BitVectorSize));
                        int position = OpenFileDescriptors[fd].getInode()->getSingleIndirect();
                        BitVector[position] = 1;
                        for(int i=0;i<block_size;i++){
                            fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                            fwrite("\0", sizeof(char), 1, sim_disk_fd);
                        }
                        OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                        for (int i = 0; i < block_size; i++) {
                            int blockPosition = emptyBlocks(BitVector, BitVectorSize);
                            if (blockPosition == -1) {
                                return 1;
                            }
                            BitVector[blockPosition] = 1;
                            char *x = new char[2];
                            x[0] = decToBinary(blockPosition);
                            fseek(sim_disk_fd, (position * block_size) + i, SEEK_SET);
                            fwrite(x, sizeof(char), 1, sim_disk_fd);
                            for(int j=0;j<block_size;j++){
                                fseek(sim_disk_fd, (blockPosition * block_size)+j, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            fseek(sim_disk_fd, blockPosition * block_size, SEEK_SET);
                            if (len > block_size) {
                                fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                                OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                                OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                wordChanger(buf, len, block_size);
                            } else {
                                fwrite(buf, sizeof(char), len, sim_disk_fd);
                                OpenFileDescriptors[fd].getInode()->addFileSize(len);
                                OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            }
                            len -= block_size;
                            if (len <= 0) {
                                delete[] x;
                                return 1;
                            }
                            delete[] x;
                        }
                    }
                }else{
                    int pointer;
                    int pos;
                    char y;
                    int q;
                    int blockNum=OpenFileDescriptors[fd].getInode()->getSingleIndirect();
                    if(indirectSpace(sim_disk_fd,blockNum,block_size,&pointer,&pos)==1){
                        fseek(sim_disk_fd,(blockNum*block_size)+pointer,SEEK_SET);
                        int z=fread(&y,1,1,sim_disk_fd);
                        if(y=='\0'){
                            q= emptyBlocks(BitVector, BitVectorSize);
                            if(q==-1){
                                return 1;
                            }
                            BitVector[q]=1;
                            for(int i=0;i<block_size;i++){
                                fseek(sim_disk_fd, (q * block_size)+i, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            char *r=new char[2];
                            r[0]= decToBinary(q);
                            fseek(sim_disk_fd,(blockNum*block_size)+pointer,SEEK_SET);
                            fwrite(r,1,1,sim_disk_fd);
                            delete[] r;
                        }
                        else{
                            q=y;
                        }
                        fseek(sim_disk_fd,(q*block_size)+pos,SEEK_SET);
                        if(len>block_size-pos){
                            fwrite(buf,sizeof(char),block_size-pos,sim_disk_fd);
                            wordChanger(buf,len,block_size-pos);
                            len-=block_size-pos;
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size-pos);
                        }
                        else{
                            fwrite(buf,sizeof(char),len,sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            return 1;
                        }
                        for(int i=pointer+1;i<block_size;i++){
                            int blockPosition = emptyBlocks(BitVector, BitVectorSize);
                            if (blockPosition == -1) {
                                return 1;
                            }
                            BitVector[blockPosition] = 1;
                            char *x = new char[2];
                            x[0] = decToBinary(blockPosition);
                            fseek(sim_disk_fd, (blockNum * block_size) + i, SEEK_SET);
                            fwrite(x, sizeof(char), 1, sim_disk_fd);
                            for(int j=0;j<block_size;j++){
                                fseek(sim_disk_fd, (blockPosition * block_size)+j, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            fseek(sim_disk_fd, blockPosition * block_size, SEEK_SET);
                            if (len > block_size) {
                                fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                                OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                                OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                wordChanger(buf, len, block_size);
                            } else {
                                fwrite(buf, sizeof(char), len, sim_disk_fd);
                                OpenFileDescriptors[fd].getInode()->addFileSize(len);
                                OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            }
                            len -= block_size;
                            if (len <= 0) {
                                delete[] x;
                                return 1;
                            }
                            delete[] x;
                        }
                    }
                }
                if (OpenFileDescriptors[fd].getInode()->getDoubleIndirect() == -1) {
                    if (indirectEmptyBlock(BitVector, BitVectorSize) >= 3) {
                        OpenFileDescriptors[fd].getInode()->setDoubleIndirect(emptyBlocks(BitVector, BitVectorSize));
                        int position = OpenFileDescriptors[fd].getInode()->getDoubleIndirect();
                        BitVector[position] = 1;
                        for(int i=0;i<block_size;i++){
                            fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                            fwrite("\0", sizeof(char), 1, sim_disk_fd);
                        }
                        OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                        for(int i=0;i<block_size;i++){
                            int position2=emptyBlocks(BitVector, BitVectorSize);
                            if (position2 == -1) {
                                return 1;
                            }
                            BitVector[position2] = 1;
                            for(int k=0;k<block_size;k++){
                                fseek(sim_disk_fd, (position2 * block_size)+k, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            char *x = new char[2];
                            x[0] = decToBinary(position2);
                            fseek(sim_disk_fd, (position * block_size) + i, SEEK_SET);
                            fwrite(x, sizeof(char), 1, sim_disk_fd);
                            delete[] x;
                            for (int j = 0; j < block_size; j++) {
                                int blockPosition = emptyBlocks(BitVector, BitVectorSize);
                                if (blockPosition == -1) {
                                    return 1;
                                }
                                BitVector[blockPosition] = 1;
                                for(int o=0;o<block_size;o++){
                                    fseek(sim_disk_fd, (blockPosition * block_size)+o, SEEK_SET);
                                    fwrite("\0", sizeof(char), 1, sim_disk_fd);
                                }
                                char *z = new char[2];
                                z[0] = decToBinary(blockPosition);
                                fseek(sim_disk_fd, (position2 * block_size) + j, SEEK_SET);
                                fwrite(z, sizeof(char), 1, sim_disk_fd);
                                fseek(sim_disk_fd, blockPosition * block_size, SEEK_SET);
                                if (len > block_size) {
                                    fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                                    OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                                    OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                    wordChanger(buf, len, block_size);
                                } else {
                                    fwrite(buf, sizeof(char), len, sim_disk_fd);
                                    OpenFileDescriptors[fd].getInode()->addFileSize(len);
                                    OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                }
                                len -= block_size;
                                if (len <= 0) {
                                    delete[] z;
                                    return 1;
                                }
                                delete[] z;
                            }
                        }
                    }
                    return 1;
                }else{
                    int pointer;
                    int pointer2;
                    int pos;
                    char y;
                    char m;
                    int q;
                    int b;
                    int blockNum=OpenFileDescriptors[fd].getInode()->getDoubleIndirect();
                    if(doubleIndirectSpace(sim_disk_fd,blockNum,block_size,&pointer,&pointer2,&pos)==1){
                        fseek(sim_disk_fd,(blockNum*block_size)+pointer,SEEK_SET);
                        int z=fread(&y,1,1,sim_disk_fd);
                        if(y=='\0'){
                            q= emptyBlocks(BitVector, BitVectorSize);
                            if(q==-1){
                                return 1;
                            }
                            BitVector[q]=1;
                            for(int i=0;i<block_size;i++){
                                fseek(sim_disk_fd, (q * block_size)+i, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            char *r=new char[2];
                            r[0]= decToBinary(q);
                            fseek(sim_disk_fd,(blockNum*block_size)+pointer,SEEK_SET);
                            fwrite(r,1,1,sim_disk_fd);
                            delete[] r;
                        }else{
                            q=y;
                        }
                        fseek(sim_disk_fd,(q*block_size)+pointer2,SEEK_SET);
                        int s=fread(&m,1,1,sim_disk_fd);
                        if(m=='\0'){
                            b= emptyBlocks(BitVector, BitVectorSize);
                            if(b==-1){
                                return 1;
                            }
                            BitVector[b]=1;
                            for(int i=0;i<block_size;i++){
                                fseek(sim_disk_fd, (b * block_size)+i, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            char *f=new char[2];
                            f[0]= decToBinary(b);
                            fseek(sim_disk_fd,(q*block_size)+pointer2,SEEK_SET);
                            fwrite(f,1,1,sim_disk_fd);
                            delete[] f;
                        }else{
                            b=m;
                        }
                        fseek(sim_disk_fd,(b*block_size)+pos,SEEK_SET);
                        if(len>block_size-pos){
                            fwrite(buf,sizeof(char),block_size-pos,sim_disk_fd);
                            wordChanger(buf,len,block_size-pos);
                            len-=block_size-pos;
                            OpenFileDescriptors[fd].getInode()->addFileSize(block_size-pos);
                        }
                        else{
                            fwrite(buf,sizeof(char),len,sim_disk_fd);
                            OpenFileDescriptors[fd].getInode()->addFileSize(len);
                            return 1;
                        }
                        for (int i = pointer2+1; i < block_size; i++) {
                            int blockPosition = emptyBlocks(BitVector, BitVectorSize);
                            if (blockPosition == -1) {
                                return 1;
                            }
                            BitVector[blockPosition] = 1;
                            for(int j=0;j<block_size;j++){
                                fseek(sim_disk_fd, (blockPosition * block_size)+j, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            char *x = new char[2];
                            x[0] = decToBinary(blockPosition);
                            fseek(sim_disk_fd, (q * block_size) + i, SEEK_SET);
                            fwrite(x, sizeof(char), 1, sim_disk_fd);
                            fseek(sim_disk_fd, blockPosition * block_size, SEEK_SET);
                            if (len > block_size) {
                                fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                                OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                                OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                wordChanger(buf, len, block_size);
                            } else {
                                fwrite(buf, sizeof(char), len, sim_disk_fd);
                                OpenFileDescriptors[fd].getInode()->addFileSize(len);
                                OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                            }
                            len -= block_size;
                            if (len <= 0) {
                                delete[] x;
                                return 1;
                            }
                            delete[] x;
                        }
                        for(int i=pointer+1;i<block_size;i++) {
                            int pos2 = emptyBlocks(BitVector, BitVectorSize);
                            if (pos2 == -1) {
                                return 1;
                            }
                            BitVector[pos2] = 1;
                            for(int k=0;k<block_size;k++){
                                fseek(sim_disk_fd, (pos2 * block_size)+k, SEEK_SET);
                                fwrite("\0", sizeof(char), 1, sim_disk_fd);
                            }
                            char *x = new char[2];
                            x[0] = decToBinary(pos2);
                            fseek(sim_disk_fd, (blockNum * block_size) + i, SEEK_SET);
                            fwrite(x, sizeof(char), 1, sim_disk_fd);
                            delete[] x;
                            for (int j = 0; j < block_size; j++) {
                                int blockPosition = emptyBlocks(BitVector, BitVectorSize);
                                if (blockPosition == -1) {
                                    return 1;
                                }
                                BitVector[blockPosition] = 1;
                                for(int n=0;n<block_size;n++){
                                    fseek(sim_disk_fd, (blockPosition * block_size)+n, SEEK_SET);
                                    fwrite("\0", sizeof(char), 1, sim_disk_fd);
                                }
                                char *n = new char[2];
                                n[0] = decToBinary(blockPosition);
                                fseek(sim_disk_fd, (pos2 * block_size) + j, SEEK_SET);
                                fwrite(n, sizeof(char), 1, sim_disk_fd);
                                fseek(sim_disk_fd, blockPosition * block_size, SEEK_SET);
                                if (len > block_size) {
                                    fwrite(buf, sizeof(char), block_size, sim_disk_fd);
                                    OpenFileDescriptors[fd].getInode()->addFileSize(block_size);
                                    OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                    wordChanger(buf, len, block_size);
                                } else {
                                    fwrite(buf, sizeof(char), len, sim_disk_fd);
                                    OpenFileDescriptors[fd].getInode()->addFileSize(len);
                                    OpenFileDescriptors[fd].getInode()->addBlockInUse(1);
                                }
                                len -= block_size;
                                if (len <= 0) {
                                    delete[] n;
                                    return 1;
                                }
                                delete[] n;
                            }
                        }
                    }
                    return 1;
                }
            }
        }
        cout << "ERR\n";
        return -1;
    }
    // ------------------------------------------------------------------------
    int DelFile( string FileName ) {
        int x=0;
        char y;
        if(fd_opened(OpenFileDescriptors,FileName)==-1 && fd_created(MainDir,FileName)==1){
            if(MainDir[FileName]->getDirectBlock1()!=-1) {
                BitVector[MainDir[FileName]->getDirectBlock1()] = 0;
            }
            if(MainDir[FileName]->getDirectBlock2()!=-1) {
                BitVector[MainDir[FileName]->getDirectBlock2()] = 0;
            }
            if(MainDir[FileName]->getDirectBlock3()!=-1) {
                BitVector[MainDir[FileName]->getDirectBlock3()] = 0;
            }
            if(MainDir[FileName]->getSingleIndirect()!=-1) {
                BitVector[MainDir[FileName]->getSingleIndirect()] = 0;
                for(int i=0;i<block_size;i++){
                    fseek(sim_disk_fd,(MainDir[FileName]->getSingleIndirect()*block_size)+i,SEEK_SET);
                    fread(&y,1,1,sim_disk_fd);
                    if(y!='\0') {
                        int z = y;
                        BitVector[z] = 0;
                    }
                }
            }
            if(MainDir[FileName]->getDoubleIndirect()!=-1) {
                BitVector[MainDir[FileName]->getDoubleIndirect()] = 0;
                for(int i=0;i<block_size;i++){
                    fseek(sim_disk_fd,(MainDir[FileName]->getDoubleIndirect()*block_size)+i,SEEK_SET);
                    fread(&y,1,1,sim_disk_fd);
                    if(y!='\0') {
                        int z = y;
                        BitVector[z] = 0;
                        for (int j = 0; j < block_size; j++) {
                            fseek(sim_disk_fd, (z * block_size) + j, SEEK_SET);
                            fread(&y, 1, 1, sim_disk_fd);
                            if(y!='\0') {
                                int k = y;
                                BitVector[k] = 0;
                            }
                        }
                    }
                }
            }
            int del=findDeleted(OpenFileDescriptors,FileName);
            while(del!=-1){
                string newName=" ";
                FileDescriptor fd (newName,MainDir[FileName]);
                fd.setInUse(false);
                fd.getInode()->zeroSize();
                OpenFileDescriptors[del] =fd;
                del=findDeleted(OpenFileDescriptors,FileName);
            }
            deleted.push_back(MainDir[FileName]);
            MainDir.erase(FileName);
            return 1;
        }
        cout << "ERR\n";
        return -1;
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len ) {
        int x=0;
        int position=0;
        char y;
        int counter=0;
        int z=0;
        int l=0;
        for(int i=0;i<DISK_SIZE;i++){
            buf[i]='\0';
        }
        if(fd<OpenFileDescriptors.size() && fd>=0 && len>=0) {
            if (OpenFileDescriptors[fd].isInUse() == true) {
                if (len >= OpenFileDescriptors[fd].GetFileSize()) {
                    x = OpenFileDescriptors[fd].GetFileSize();
                } else {
                    x = len;
                }
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock1() != -1) {
                    position=OpenFileDescriptors[fd].getInode()->getDirectBlock1();
                    for(int i=0;i<block_size;i++) {
                        if(x==0){
                            return 1;
                        }
                        fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                        fread(&y, 1, 1, sim_disk_fd);
                        buf[counter]=y;
                        counter++;
                        x--;
                    }
                }else{
                    return 1;
                }
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock2() != -1) {
                    position=OpenFileDescriptors[fd].getInode()->getDirectBlock2();
                    for(int i=0;i<block_size;i++) {
                        if(x==0){
                            return 1;
                        }
                        fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                        fread(&y, 1, 1, sim_disk_fd);
                        buf[counter]=y;
                        counter++;
                        x--;
                    }
                }else{
                    return 1;
                }
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock3() != -1) {
                    position=OpenFileDescriptors[fd].getInode()->getDirectBlock3();
                    for(int i=0;i<block_size;i++) {
                        if(x==0){
                            return 1;
                        }
                        fseek(sim_disk_fd, (position * block_size)+i, SEEK_SET);
                        fread(&y, 1, 1, sim_disk_fd);
                        buf[counter]=y;
                        counter++;
                        x--;
                    }
                }else{
                    return 1;
                }
                if (OpenFileDescriptors[fd].getInode()->getSingleIndirect() != -1) {
                    position=OpenFileDescriptors[fd].getInode()->getSingleIndirect();
                    for(int i=0;i<block_size;i++) {
                        fseek(sim_disk_fd,(position*block_size)+i,SEEK_SET);
                        fread(&y,1,1,sim_disk_fd);
                        z=y;
                        for(int j=0;j<block_size;j++){
                            if(x==0){
                                return 1;
                            }
                            fseek(sim_disk_fd,(z*block_size)+j,SEEK_SET);
                            fread(&y,1,1,sim_disk_fd);
                            buf[counter]=y;
                            counter++;
                            x--;
                        }
                    }
                }else{
                    return 1;
                }
                if (OpenFileDescriptors[fd].getInode()->getDoubleIndirect() != -1) {
                    position=OpenFileDescriptors[fd].getInode()->getDoubleIndirect();
                    for(int i=0;i<block_size;i++) {
                        fseek(sim_disk_fd,(position*block_size)+i,SEEK_SET);
                        fread(&y,1,1,sim_disk_fd);
                        z=y;
                        for(int j=0;j<block_size;j++){
                            fseek(sim_disk_fd,(z*block_size)+j,SEEK_SET);
                            fread(&y,1,1,sim_disk_fd);
                            l=y;
                            for(int k=0;k<block_size;k++){
                                if(x==0){
                                    return 1;
                                }
                                fseek(sim_disk_fd,(l*block_size)+k,SEEK_SET);
                                fread(&y,1,1,sim_disk_fd);
                                buf[counter]=y;
                                counter++;
                                x--;
                            }
                        }
                    }
                    return 1;
                }else{
                    return 1;
                }
            }
        }
        cout << "ERR\n";
        return -1;
    }

    // ------------------------------------------------------------------------
    int GetFileSize(int fd) {
        if(fd>=0 && fd<OpenFileDescriptors.size()){
            if(OpenFileDescriptors[fd].isInUse()==true){
                return OpenFileDescriptors[fd].GetFileSize();
            }
        }
        cout << "ERR\n";
        return -1;
    }

    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {
        int maxLength = (3 + block_size + block_size * block_size) * block_size;
        char reader[DISK_SIZE];
        int x = 0;
        int counter;
        int y;
        if (srcFileName != destFileName && fd_opened(OpenFileDescriptors, srcFileName) == -1 &&
            fd_created(MainDir, srcFileName) == 1 && fd_opened(OpenFileDescriptors, destFileName) == -1) {
            if (fd_created(MainDir, destFileName) == 1) {
                DelFile(destFileName);
            }
            x = OpenFile(srcFileName);
            if (x != -1) {
                counter = freeBlocks(BitVector, BitVectorSize);
                if (OpenFileDescriptors[x].getInode()->getBlockInUse() <= counter) {
                    if (OpenFileDescriptors[x].getInode()->getBlockInUse() != 0) {
                        ReadFromFile(x, reader, maxLength);
                        y = CreateFile(destFileName);
                        WriteToFile(y, reader, strlen(reader));
                    } else {
                        y = CreateFile(destFileName);
                    }
                    CloseFile(y);
                }
                else{
                    CloseFile(x);
                    cout << "ERR\n";
                    return -1;
                }
                CloseFile(x);
                return 1;
            }
        }
        cout << "ERR\n";
        return -1;
    }

    // ------------------------------------------------------------------------
    int MoveFile(string srcFileName, string destFileName) {
        return 0;
    }

    // ------------------------------------------------------------------------
    int RenameFile(string oldFileName, string newFileName) {
        if(fd_opened(OpenFileDescriptors,oldFileName)==-1 && oldFileName!=newFileName && fd_created(MainDir,oldFileName)==1
           && fd_created(MainDir,newFileName)==-1){
            fsInode* node=new fsInode(block_size);
            node->setDirectBlock1(MainDir[oldFileName]->getDirectBlock1());
            node->setDirectBlock2(MainDir[oldFileName]->getDirectBlock2());
            node->setDirectBlock3(MainDir[oldFileName]->getDirectBlock3());
            node->setSingleIndirect(MainDir[oldFileName]->getSingleIndirect());
            node->setDoubleIndirect(MainDir[oldFileName]->getDoubleIndirect());
            node->addFileSize(MainDir[oldFileName]->getFileSize());
            node->addBlockInUse(MainDir[oldFileName]->getBlockInUse());
            deleted.push_back(MainDir[oldFileName]);
            MainDir.erase(oldFileName);
            MainDir.insert(make_pair(newFileName, node));
            return 1;
        }
        cout << "ERR\n";
        return -1;
    }

    ~fsDisk(){
        if(is_formated==true){
            for(int i=0;i<deleted.size();i++){
                delete deleted.at(i);
            }
            delete[] BitVector;
            for(auto it=MainDir.begin();it!=MainDir.end();it++){
                delete it->second;
            }
        }
        fclose(sim_disk_fd);
    }

};

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                // cin >> direct_entries;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;

            default:
                break;
        }
    }
}


