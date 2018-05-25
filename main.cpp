#include <iostream>
#include <map>
#include <Windows.h>
#include <cstring>
#include <vector>


using namespace std;

#define DEVIDER 4
#define MAGIC 32
#define CHEAK_FILE_HEAD false
#define PRINTF_ONLI_HEAD true

enum status { BUSY , FREE , BUSY_HEAD};

struct fileInf{
    unsigned int id;
    unsigned int nextId;
    unsigned int pos;
    unsigned int size;
    status stat;
};

fileInf * end_struct_fo_registr;

#define block sizeof(fileInf)

vector<fileInf*> header;

unsigned int idCouner = 1;

HANDLE File;

unsigned int MAX_SIZE;

unsigned int HEADER_SIZE;

void create_file(unsigned int max_size){
    MAX_SIZE = max_size;
    HEADER_SIZE = (max_size/DEVIDER) + 1;
    int j;
    File = CreateFile("disk_SOK.txt",GENERIC_READ|GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
    if (File != INVALID_HANDLE_VALUE) {
        CreateFileMapping(File, 0, PAGE_READWRITE, 0, MAX_SIZE * sizeof(fileInf), 0);
        SetFilePointer(File,0,0,FILE_BEGIN);
        WriteFile(File, end_struct_fo_registr, sizeof(fileInf), reinterpret_cast<LPDWORD>(&j), NULL);
    }
    else {
        exit(-1);
    }
}

fileInf* init_struct (status stat,unsigned int id, unsigned int pos, unsigned int size, fileInf* file = nullptr, unsigned int nextId = 0){
    if(file == nullptr){
        file = new fileInf();
    }
    file->size= size;
    file->id =id;
    file->pos=pos;
    file->nextId = nextId;
    file->stat = stat;
    if(pos == 0){
        delete file;
        file = nullptr;
    }
    return file;
}

int printf_menu();

bool make_magic(int i);

void create_new_file(string str = "");

string enter_file_inf();

bool enought_space(unsigned int size);

fileInf * find_free_pos(int *size);

void printf_file_description_hader(bool f = false);

void delete_file(int res = 0);

fileInf *find_file(unsigned int id, bool fl = true);

void see_file_inf(int res = 0);

vector<fileInf *> get_full_file(int res);

void refresh_file_inf();

void defragmintaiton();

int get_ful_file_size(vector<fileInf *> * vec);

string get_file_inf(char * buf, vector<fileInf*> * vec);

int move_all_files(int offset, int size, int i);

void cheack_and_clear_header();

void delete_ziro_element();

void push(vector<fileInf *> vec);

vector<fileInf *> find_secret_and_do_magic(int i);

void load_disk();

bool try_open_file(unsigned int i);

int main() {
    end_struct_fo_registr = init_struct(BUSY,0,-1,0);
    if(!try_open_file(MAGIC)){
        fileInf* file = init_struct(FREE,idCouner++,HEADER_SIZE* block , (MAX_SIZE - HEADER_SIZE)*block);
        header.push_back(file);
    }
    while(make_magic(printf_menu()));
    return 0;
}

bool try_open_file(unsigned int max_size) {
    MAX_SIZE = max_size;
    bool res = true;
    HEADER_SIZE = (max_size/DEVIDER) + 1;
    int j;
    File = CreateFile("disk_SOK.txt",
                      GENERIC_READ|GENERIC_WRITE,
                      FILE_SHARE_READ,       // share for reading
                      NULL,                  // default security
                      OPEN_EXISTING,         // existing file only
                      FILE_ATTRIBUTE_NORMAL, // normal file
                      NULL);
    if (File != INVALID_HANDLE_VALUE) {
        load_disk();
    }
    else {
        //cout<<GetLastError()<<endl;
        res =false;
        create_file(max_size);
    }
    return res;
}

void load_disk() {
    int j;
    fileInf * str;
    SetFilePointer(File,0,0,FILE_BEGIN);
    do{
        str = new fileInf();
        ReadFile(File, str, sizeof(fileInf), reinterpret_cast<LPDWORD>(&j), NULL);
        if(str->pos == end_struct_fo_registr->pos && str->size == end_struct_fo_registr->size){
            break;
        }else{
            header.push_back(str);
        }
    }while(true);
    idCouner = header[header.size()-1]->id + 1;
}


/* ********************************** FUN DEFINITION **************************************************** */

int printf_menu(){
    //system("cls");
    puts("1. add new file");
    puts("2. get file list");
    puts("3. delete element");
    puts("4. see file inf");
    puts("5. refresh(realloc)");
    puts("6. defragmintation");
    puts("0. exit");

    int i = 0;
    rewind(stdin);
    fflush(stdin);
    cin.clear();
    cin>> i;
    return i;
}

bool make_magic(int i){
    switch (i){
        case 0 :  return false;
        case 1 :  create_new_file(); break;
        case 2 :  printf_file_description_hader(); break;
        case 3 :  delete_file(); break;
        case 4 :  see_file_inf(); break;
        case 5 :  refresh_file_inf(); break;
        case 6 :  defragmintaiton(); break;
        default:  return make_magic(printf_menu());
    }
    return true;
}

void defragmintaiton() {
    vector<vector<fileInf*>> vect;
    for(fileInf * file : header){
        if(file->stat == BUSY_HEAD){
            vect.push_back(get_full_file(file->id));
        }
    }
    char buf[(MAX_SIZE - HEADER_SIZE) * block + 1];
    int offset = 0;
    int next_id_psevdo = 1;
    for(vector<fileInf*> files : vect){
        int i =0;
        int size = get_ful_file_size(&files);
        string file_inf = get_file_inf(buf,&files); //CHEAK THIS FUNCTION!!!
        delete_file(files[i]->id);
        switch (move_all_files(offset,size,next_id_psevdo++)){
            case 0 :  create_new_file(file_inf); break;
            default : exit(10);
        }
        offset += size;
    }
}

fileInf * free_zone;

int move_all_files(int offset, int size, int i) {
    do {
        if (header[i - 1]->size >= size) {
            return 0;
        } else {
            vector<fileInf *> befor;
            befor = find_secret_and_do_magic(i);
            if(free_zone->size == 0){
                delete_ziro_element();
                befor = find_secret_and_do_magic(i);
            }
            if (free_zone->size != 0) {
                header[i-1]->size += 1;
                push(befor);
            }
        }
    }while(true);
}

vector<fileInf *> find_secret_and_do_magic(int i) {
    vector<fileInf*> vec;
    for(fileInf * f : header){
        if(vec.empty()) {
            if (f->stat != FREE) {
                i--;
                if(i==0) {
                    vec.push_back(f);
                }else{
                    continue;
                }
            }
        }else{
            if(f->stat == FREE){
                free_zone = f;
                return vec;
            }else{
                vec.push_back(f);
            }
        }
    }
    return vec;
}

void push(vector<fileInf *> vec){
    char buffer = '\0';
    char buffer_after;
    char buf[free_zone->pos - vec[0]->pos +2];
    buf[free_zone->pos - vec[0]->pos + 1] = buffer;
    int j;
    SetFilePointer(File,vec[0]->pos,0,FILE_BEGIN);
    ReadFile(File, buf,free_zone->pos - vec[0]->pos +1, reinterpret_cast<LPDWORD>(&j), NULL);
    buffer = buf[0];
    for(int i = 0; i < free_zone->pos - vec[0]->pos ; i++){
        buffer_after = buf[i+1];
        buf[i + 1] = buffer;
        buffer = buffer_after;
    }
    SetFilePointer(File,vec[0]->pos,0,FILE_BEGIN);
    WriteFile(File, buf,free_zone->pos - vec[0]->pos +1, reinterpret_cast<LPDWORD>(&j), NULL);
    for(fileInf * e : vec){
        e->pos+= 1;
    }
    free_zone->size-=1;
    free_zone->pos+=1;
}

//CHEAK THIS FUNCTION!!! is return value is correct?!
string get_file_inf(char * buf, vector<fileInf*> *vec) {
    int s = 0;
    int res;
    for(fileInf * file : *vec){
        SetFilePointer(File,file->pos,0,FILE_BEGIN);
        ReadFile(File, buf + s, file->size, reinterpret_cast<LPDWORD>(&res), NULL);
        s += file->size;
    }
    buf[s] = '\0';
    return string(buf);
}

int get_ful_file_size( vector<fileInf *> * vec){
    int size = 0;

    for(fileInf* file : *vec){
        size += file->size;
    }
    return size;
}

void refresh_file_inf() {
    int rewrite_id;
    printf_file_description_hader(PRINTF_ONLI_HEAD);
    cout << "enter id of file that you want to rewrite : ";
    cin >> rewrite_id;
    see_file_inf(rewrite_id);
    delete_file(rewrite_id);
    create_new_file();
}

void see_file_inf(int ress) {
    int res;
    if( ress == 0) {
        printf_file_description_hader(PRINTF_ONLI_HEAD);
        cout << "enter file id : ";
        cin >> res;
    }else{
        res = ress;
    }
    vector<fileInf*> vec = get_full_file(res);
    if(vec.empty()){
        cout<<"sorry no such file"<<endl;
        return;
    }else{
        cout<<"file "<<res<<" inf :";
        char buf[(MAX_SIZE - HEADER_SIZE) * block + 1];
        int s = 0;
        for(fileInf * file : vec){

            SetFilePointer(File,file->pos,0,FILE_BEGIN);
            ReadFile(File, buf + s, file->size, reinterpret_cast<LPDWORD>(&res), NULL);
            s += file->size;
        }
        buf[s] = '\0';
        cout<<buf<<endl;
    }
}

vector<fileInf *> get_full_file(int res) {
    vector<fileInf*> vec;
    fileInf * file;
    file = find_file(res, CHEAK_FILE_HEAD);
    if(file == nullptr){
        return vec;
    }else{
        do{
            vec.push_back(file);
            if(file->nextId == 0){
                break;
            }else{
                file = find_file(file->nextId);
            }
        }while(true);
    }
    return vec;
}

void change_stat(fileInf * file,status stat){
    file->stat=stat;
}

void delete_file(int res) {
    int delete_id;
    if(res == 0) {
        printf_file_description_hader();
        cout << "enter id of file that you want to delete : ";
        cin >> delete_id;
    }else{
        delete_id = res;
    }
    fileInf * file = nullptr;
    for(fileInf* f : header){
        if(f->stat == BUSY_HEAD && f->id ==  delete_id){
            file = f;
            break;
        }else{
            continue;
        }
    }
    if(file == nullptr){
        cout<<"no such file"<<endl;
        return;
    }else{
        do{
            change_stat(file,FREE);
            if(file->nextId== 0){
                break;
            }else{
                fileInf * temp = file;
                file = find_file(file->nextId);
                temp->nextId = 0;
            }
        }while(true);

    }
   cheack_and_clear_header();
}

void cheack_and_clear_header() {
    int i = 0;
    do{
        if(header[i]->stat == FREE && header[i+1]->stat== FREE){
            header[i]->size += header[i+1]->size;
            header[i+1]->size = 0;
            delete_ziro_element();
        }else{
            i++;
            if(i == header.size() - 1 ){
                break;
            }else{
                continue;
            }
        }
        if(i == header.size() -1){
            break;
        }
    }while(true);
}

void delete_ziro_element() {
    vector<fileInf*> temp;
    int i = 0;
    do{
        if(header[i]->size != 0){
            temp.push_back(header[i]);
        }else{
            i++;
            continue;
        }
        i++;
    }while(i != header.size());
    header = temp;
}

fileInf *find_file(unsigned int id, bool fl ) {
    fileInf* f = nullptr;
    for( fileInf* file : header){
        if(file->id == id && (fl || file->stat==BUSY_HEAD)){
            f= file;
            break;
        }
    }
    return f;
}

void printf_file_description_hader(bool f) {
    cout<<"file list:"<<endl;
    for( fileInf* file : header){
        if(file->stat != FREE) {
            if(file->stat == BUSY && f){ continue; }
            if (file->nextId == 0) {
                cout << file->id << endl;
            } else {
                cout << file->id << " -> " << file->nextId << endl;
            }
        }
    }
}

vector<fileInf*> create_segmentation_file(unsigned int size){
    int ssize = size;
    vector<fileInf*> vec;
    if(enought_space(size)) {
        do {
            fileInf * file = find_free_pos(&ssize);
            if(vec.empty()){
                file->stat=BUSY_HEAD;
                vec.push_back(file);
            }else{
                vec[vec.size()-1]->nextId = file->id;
                vec.push_back(file);
            }
        }while(ssize > 0);
        return vec;
    }else{
        return vec;
    }
}

fileInf * find_free_pos(int *size) {
    for(auto element : header){
        if(element->stat == FREE){
            if(element->size > *size){
                fileInf* pice = init_struct(FREE,
                                            idCouner++,
                                            element->pos + *size,
                                            element->size - *size);
                header.push_back(pice);
                element->size = *size;
                element->stat =BUSY;
                *size -= element->size;
                return element;
            }else{
                *size -= element->size ;
                element->stat = BUSY;
                return element;
            }
        }
    }
    return 0;
}

bool enought_space(unsigned int size) {
    int ssize = size;
    for(auto element : header){
        if(element->stat == FREE){
            ssize -= element->size;
            if(ssize<= 0){
                return true;
            }
        }else{
            continue;
        }
    }
    return false;
}

void create_new_file(string str){
    if(header.size() == MAGIC/DEVIDER){
        cout<<"sorry no space in header"<<endl;
        return;
    }
    string file_inf;
    if(str.empty()) {
        file_inf = enter_file_inf();
    }else{
        file_inf = str;
    }
    if(file_inf.empty()){
        return;
    }else{
        vector<fileInf*> file = create_segmentation_file(file_inf.size());
        if(file.empty()){
            cout<<"SORRY we can't create that file =("<<endl;
            return;
        }else{
            int i =0;
            int res;
            for(auto element : file){
                auto v = SetFilePointer(File,element->pos,0,FILE_BEGIN);
                if(v == INVALID_SET_FILE_POINTER){
                    cout<<"error"<<endl;
                    return ;
                }
                string sub = file_inf.substr(i, i + element->size);
                WriteFile(File, sub.c_str(), sub.size(), reinterpret_cast<LPDWORD>(&res), NULL);
                i+=element->size;
            }
        }
    }

    header.push_back(end_struct_fo_registr);
    SetFilePointer(File,0,0,FILE_BEGIN);
    int j;
    for(fileInf * f: header){
        WriteFile(File, f, sizeof(fileInf), reinterpret_cast<LPDWORD>(&j), NULL);
    }
    header.pop_back();
}

string enter_file_inf() {
    char file_inf [(MAX_SIZE - HEADER_SIZE) * block + 1];
    cout<<"enter information (empty string for exit): "<<endl;
    rewind(stdin);
    fflush(stdin);
    cin.clear();
    int s =(MAX_SIZE - HEADER_SIZE) * block;
    cin.getline(file_inf, s );
    string str;
    char c;
    for(int i =0 ; i<strlen(file_inf);i++){
        c=file_inf[i];
        str +=c;
    }
    return str;
}
