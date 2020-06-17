#include <iostream>
#include <cmath>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

#define MAX_BLOCKS 32768

struct File
{
    int identifier;
    int start_index;
    int size;
};

using namespace std;

// implemented with FAT

int directory[MAX_BLOCKS];
int FAT[MAX_BLOCKS]; // considered to be stored in the directory (each entry needs 4 bytes of directory space).
map<int, File> directory_table;
int available = MAX_BLOCKS;
int block_size = 1024;
int num_files = 0;
int num_extensions = 0;
int creation_failures = 0;
int extension_failures = 0;
string fname;
time_t start_time;
time_t end_time;

void init_directory();
void print_directory(int directory[]);
void parse_input(string fname);
void create_file(int file_id, int file_length);
int access(int file_id, int byte_offset);
void extend(int file_id, int amount);
void shrink(int file_id, int amount);

int main()
{

    init_directory();
    // inputting the name of the file to be executed.
    cout << "File name: ";
    cin >> fname;
    stringstream linestream(fname);
    string item;
    int k = 0;
    // obtaining the block size for the simulation.
    while (getline(linestream, item, '_'))
    {
        if (k == 1)
        {
            block_size = stoi(item);
            break;
        }
        k++;
    }
    // cout << block_size << endl;
    // updating available blocks, assuming that each FAT entry takes 4 bytes.
    available = MAX_BLOCKS * (block_size - 4) / block_size;
    auto start_time = chrono::steady_clock::now();
    parse_input(fname);
    auto end_time = chrono::steady_clock::now();
    // timing.
    auto duration = end_time - start_time;
    cout << "The simulation took " << chrono::duration_cast<chrono::milliseconds>(duration).count() << " milliseconds." << endl;
    cout << "Number of successful file creations: " << num_files << " - Number of failed file creation attempts: " << creation_failures << endl;
    cout << "Number of successful file extensions: " << num_extensions << " - Number of failed file extension attempts: " << extension_failures << endl;
    return 0;
}

void init_directory()
{

    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        directory[i] = 0;
    }
}

void print_directory(int directory[])
{
    for (int k = 0; k < MAX_BLOCKS; k++)
    {
        cout << directory[k] << " ";
    }
}

void parse_input(string fname)
{
    // 3 fields will be obtained at most.
    ifstream file(fname);
    string line;
    int l;
    string opt, val1, val2;
    while (getline(file, line))
    {
        stringstream linestream(line);
        string item;
        l = 0;
        // storing the content of each line, separated by :, in fields.
        while (getline(linestream, item, ':'))
        {
            if (l == 0)
            {
                opt = item;
                // cout << "opt " << opt << endl;
            }
            else if (l == 1)
            {
                val1 = item;
                // cout << "v1 " << val1 << endl;
            }
            else
            {
                val2 = item;
                // cout << "v2 " << val2 << endl;
            }
            l++;
        }

        if (opt == "a")
        {
            // access operation
            // cout << "Accessing file " << val1 << "'s " << val2 << "th byte." << endl;
            access(stoi(val1), stoi(val2));
        }
        else if (opt == "c")
        {
            // creation operation
            // cout << "Creating file  " << val1 << endl;
            create_file(num_files, stoi(val1));
        }
        else if (opt == "e")
        {
            // extension operation
            // cout << "Extending  " << val1 << " by " << val2 << " blocks." << endl;
            extend(stoi(val1), stoi(val2));
        }
        else if (opt == "sh")
        {
            // shrink operation
            // cout << "Shrinking file " << val1 << " by "  << val2 << " blocks." << endl;
            shrink(stoi(val1), stoi(val2));
        }
    }
}

void create_file(int file_id, int file_length)
{
    // calculating the number of blocks needed to store the file and checking whether it exceeds the available space.
    int num_blocks = ceil((double)file_length / block_size); // since there are as many FAT entries as directory blocks
    if (num_blocks > available || directory_table.count(file_id) > 0)
    {
        cout << "Cannot create file, the size is too big or it already exists." << endl;
        creation_failures++;
        return;
    }

    File to_insert;
    to_insert.identifier = file_id;
    to_insert.size = num_blocks;
    int cur_ind, next_ind;
    int placing_block = 0;
    // iterating to find empty spaces within the directory.
    for (int i = 0; i < MAX_BLOCKS; i++)
    {
        if (directory[i] == 0)
        {
            // empty index to store the content.
            cur_ind = i;
            // if it is the first block to be inserted, the start index is given.
            if (placing_block == 0)
                to_insert.start_index = cur_ind;
            if (placing_block < num_blocks)
            {
                // looking for the next index to store the content, still have blocks to place
                for (int j = i + 1; j < MAX_BLOCKS; j++)
                {
                    if (directory[j] == 0)
                    {
                        // pointing to the next index.
                        next_ind = j;
                        // arrived at the last block.
                        if (placing_block == num_blocks - 1)
                        {
                            // next index is -1 to indicate the end of file.
                            FAT[cur_ind] = -1;
                            // content is a random digit.
                            directory[cur_ind] = rand() % 9 + 1;
                            placing_block++;
                        }
                        else
                        {
                            // still have blocks to place, assigning next index.
                            FAT[cur_ind] = next_ind;
                            // content is a random digit.
                            directory[cur_ind] = rand() % 9 + 1;
                            placing_block++;
                        }
                        //                        cout << cur_ind << " -> " << directory[cur_ind].next_ind << endl;
                        //                        cout << cur_ind << " -> " << directory[cur_ind].value << endl;
                        break;
                    }
                }
            }
            else
            {
                // all blocks have been placed, no need to iterate more.
                break;
            }
        }
    }
    // inserting the placed file into the directory table.
    directory_table.insert(pair<int, File>(file_id, to_insert));
    // decreasing the number of available blocks by the file size in blocks.
    available -= num_blocks;
    num_files++;
    return;
}

void shrink(int file_id, int amount)
{

    if (directory_table.count(file_id) == 0)
    {
        cout << "Cannot shrink file, it is not in the directory." << endl;
        return;
    }

    int cur_ind = directory_table.at(file_id).start_index;
    int blocks = directory_table.at(file_id).size;
    int i = 0;
    int next_ind;
    // incrementing the variable until the amount of blocks left for the file equals the given amount (backward).
    // keeping at least 1 block in the file at the same time (in case the given amount is greater than the file size).
    while ((blocks - i - 2) >= amount)
    {
        cur_ind = FAT[cur_ind];
        i++;
    }
    // trimming the file here, the following blocks will be emptied.
    next_ind = FAT[cur_ind];
    FAT[cur_ind] = -1;
    cur_ind = next_ind;
    // to keeep track of the number of blocks deleted.
    int del_cnt = 0;
    // loop won't continue if the (older) end of file has already been reached.
    while (FAT[cur_ind] != -1)
    {
        next_ind = FAT[cur_ind];
        FAT[cur_ind] = -1;
        directory[cur_ind] = 0;
        cur_ind = next_ind;
        del_cnt++;
    }
    // also emptying the end of file, incrementing again.
    FAT[cur_ind] = -1;
    directory[cur_ind] = 0;
    del_cnt++;
    // updating the size and the amount of available blocks.
    directory_table.at(file_id).size -= del_cnt;
    available += del_cnt;
    return;
}

int access(int file_id, int amount)
{

    if (directory_table.count(file_id))
    {
        int cur_ind = directory_table.at(file_id).start_index;
        int next_ind;
        int block_offset = ceil((double)amount / block_size);
        int i = 0;
        // if the given offset is accessible.
        if (block_offset <= directory_table.at(file_id).size)
        {
            // iterating until the end point is reached.
            while (FAT[cur_ind] != -1)
            {
                next_ind = FAT[cur_ind];
                cur_ind = next_ind;
                // reached the desired block.
                if (i == block_offset)
                    break;
                i++;
            }
            return next_ind;
        }
        else
        {
            // offset is not accessible.
            cout << "Offset exceeds file size!" << endl;
            return -1;
        }
    }
    else
    {
        // given file ID is not present.
        return -1;
    }
}

void extend(int file_id, int amount)
{

    if (amount > available || directory_table.count(file_id) == 0)
    {
        cout << "Cannot extend file, the size is too big or it is not in the directory." << endl;
        extension_failures++;
        return;
    }

    int cur_ind = directory_table.at(file_id).start_index;
    // finding the end of file.
    while (FAT[cur_ind] != -1)
    {
        cur_ind = FAT[cur_ind];
    }
    // extending from here on.
    int ext_cur_ind, ext_next_ind;
    int placing_block = 0;
    // iterating to find empty blocks.
    for (int i = 0; i < MAX_BLOCKS; i++)
    {

        if (directory[i] == 0)
        {
            // initial block to store the content.
            ext_cur_ind = i;
            if (placing_block == 0)
            {
                // pointing from the initial end of file index to this block if this is the first block to be inserted.
                FAT[cur_ind] = ext_cur_ind;
                // content is a random digit.
                directory[cur_ind] = rand() % 9 + 1;
                //                cout << cur_ind << " -> " << directory[cur_ind].next_ind << endl;
                //                cout << cur_ind << " -> " << directory[cur_ind].value << endl;
                placing_block++;
            }

            // if there are still blocks to be placed and this is not the first block to be inserted.
            if (placing_block <= amount && placing_block != 0)
            {
                // if there are still blocks to be placed and this is not the first block to be inserted.
                for (int j = i + 1; j < MAX_BLOCKS; j++)
                {
                    // next empty block, will be pointing there if this is not the last block to be inserted.
                    if (directory[j] == 0)
                    {
                        ext_next_ind = j;
                        // if this is the last block to be inserted (after extending the initial end of file).
                        if (placing_block == amount)
                        {
                            FAT[ext_cur_ind] = -1;
                            directory[ext_cur_ind] = rand() % 9 + 1;
                            //cout << ext_cur_ind << " -> " << directory[ext_cur_ind].next_ind << endl;
                            //cout << ext_cur_ind << " -> " << directory[ext_cur_ind].value << endl;
                        }
                        else
                        {
                            // if there are still blocks to be inserted.
                            FAT[ext_cur_ind] = ext_next_ind;
                            directory[ext_cur_ind] = rand() % 9 + 1;
                            //cout << ext_cur_ind << " -> " << directory[ext_cur_ind].next_ind << endl;
                            //cout << ext_cur_ind << " -> " << directory[ext_cur_ind].value << endl;
                        }
                        placing_block++;
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    // updating the file size and the amount of blocks available.
    directory_table.at(file_id).size += amount;
    available -= amount;
    num_extensions++;
    return;
}