#include <iostream>
#include <stdio.h>
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

int directory[MAX_BLOCKS];
map<int, File> directory_table;
int available = MAX_BLOCKS;
int block_size = 1024;
int num_files = 0;
int num_extensions = 0;
int creation_failures = 0;
int extension_failures = 0;
string fname;

void init_directory();
void print_directory(int directory[]);
void parse_input(string fname);
void create_file(int file_id, int file_length);
int access(int file_id, int byte_offset);
void extend(int file_id, int amount);
void shrink(int file_id, int amount);
void compact(int directory[]);
int find_start(int ind, map<int, File> directory_table);

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
        // each line has at most 3 content to store, according to the given format.
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
    int num_blocks = ceil((double)file_length / block_size); // calculate how many blocks
    if (num_blocks > available || directory_table.count(file_id) > 0)
    {
        cout << "Cannot create file, the size is too big or it already exists." << endl;
        creation_failures++;
        return;
    }

    File to_insert;
    to_insert.identifier = file_id;
    to_insert.size = num_blocks;

    int i = 0;
    int available_blocks = 0;
    // counting the number of available blocks from the start (considering the holes that may have been formed).
    while (i < MAX_BLOCKS && available_blocks < num_blocks)
    {
        if (directory[i++] == 0)
        {
            available_blocks++;
        }
        else
        {
            // in order to detect any holes, and therefore the need of compaction.
            available_blocks = 0;
        }
    }

    int start, j;
    if (available_blocks < num_blocks)
    {
        // holes are present, going to perform compaction.
        compact(directory);
        i = 0;
        // finding the first available block after compaction, will be placing from here on.
        while (i < MAX_BLOCKS)
        {
            if (directory[i++] == 0)
                break;
        }
        start = i - 1;
        to_insert.start_index = start;
        // inserting blocks' contents, as random digits.
        for (j = start; j < (start + num_blocks); j++)
        {
            directory[j] = rand() % 9 + 1;
        }
    }
    else
    {
        // enough contiguous space has been found, will be inserting the blocks immediately.
        // obtaining the start index.
        start = i - num_blocks;
        to_insert.start_index = start;
        // inserting blocks' contents, as random digits.
        for (j = start; j < (start + num_blocks); j++)
        {
            directory[j] = rand() % 9 + 1;
        }
    }
    // inserting the placed file into the directory table.
    directory_table.insert(pair<int, File>(file_id, to_insert));
    // decreasing the number of available blocks by the file size in blocks.
    available -= num_blocks;
    num_files++;
    return;
}
// method used for compaction.
void compact(int directory[])
{
    int i = 0;
    int cnt = 0;
    int buf;
    int key1, key2;
    while (i < MAX_BLOCKS)
    {
        // incrementing the counter until an empty block has been encountered.
        if (directory[i] == 0)
        {
            cnt++;
        }
        else
        {
            if (cnt != 0)
            { // in order to start after collecting some empty blocks
                // shifting the directory contents block by block.
                buf = directory[i - cnt];
                directory[i - cnt] = directory[i - cnt - 1];
                directory[i - cnt - 1] = buf;
                // will need to fix starting indexes of files.
                key1 = find_start(i - cnt - 1, directory_table);
                key2 = find_start(i - cnt, directory_table);
                // updating the start index of a file that's been shifted.
                if (key1 != key2 && key2 != -1)
                    directory_table.at(key2).start_index = (i - cnt);
            }
        }
        i++;
    }
}

void shrink(int file_id, int amount)
{

    if (directory_table.count(file_id) == 0)
    {
        cout << "Cannot shrink file, it is not in the directory." << endl;
        return;
    }

    int start = directory_table.at(file_id).start_index;
    int blocks = directory_table.at(file_id).size;
    int end = start + blocks - 1;
    int i = 0;
    // starting from the back, deleting file contents at the same time.
    // incrementing the variable until the amount of blocks deleted from the equals the given amount (forward).
    // keeping at least 1 block in the file at the same time (in case the given amount is greater than the file size).
    while ((end - i) > start && i < amount)
    {
        directory[end - i] = 0;
        i++;
    }
    // updating the size and the amount of available blocks.
    directory_table.at(file_id).size -= i;
    available += i;
    return;
}

int access(int file_id, int amount)
{

    if (directory_table.count(file_id))
    {
        int block_offset = ceil((double)amount / block_size);
        // if the given offset is accessible.
        if (block_offset <= directory_table.at(file_id).size)
        {
            return (directory_table.at(file_id).start_index + block_offset - 1);
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

    int start = directory_table.at(file_id).start_index;
    int blocks = directory_table.at(file_id).size;
    // calculating the index for the end of file.
    int end = start + blocks - 1;
    int i = 0, cnt = 0;
    // checking if there's available blocks for extension right after the file ends.
    for (i = (end + 1); i <= (end + amount); i++)
    {
        if (directory[i] == 0)
            cnt++;
    }
    // there are enough contiguous blocks to store the extended amount.
    if (cnt == amount)
    {
        for (i = (end + 1); i <= (end + amount); i++)
        {
            directory[i] = file_id; // rand() % 9 + 1;;
        }
    }
    else
    {
        // there aren't enough contiguous blocks to store the extended amount.
        // will need to shift the files if there is not enough space in the directory after the end of the file.
        if (end + amount >= MAX_BLOCKS)
        {
            // initially shifting every file so that every empty block is after the file at hand.
            compact(directory);
            end = directory_table.at(file_id).start_index + blocks - 1;
        }
        int key;
        int comp_cnt = 0;
        int buf;
        cnt = 0;
        for (i = (end + 1); i < MAX_BLOCKS; i++)
        {
            // incrementing the counter until an empty block.
            if (directory[i] != 0)
            {
                comp_cnt++;
            }
            else
            {
                // incrementing the amount that needs to be shifted if no empty blocks have been seen before.
                if (comp_cnt == 0)
                {
                    cnt++;
                }
                else
                {
                    // if there are non-empty blocks present behind, shifting the empty block towards the file at hand.
                    for (int j = i; j > (i - comp_cnt); j--)
                    {
                        buf = directory[j];
                        directory[j] = directory[j - 1];
                        directory[j - 1] = buf;
                    }
                    cnt++;
                }
            }
            if (cnt == amount)
                break;
        }

        int z_cnt = 0;
        // updating the starting indexes of later-shifted files (shifted down, after the end of the file at hand).
        for (i = (end + 1); i <= (end + amount); i++)
        {
            if (directory[i] == 0)
                z_cnt++;
            // fixing for the start indexes.
            key = find_start(i, directory_table);
            if (key != -1)
                directory_table.at(key).start_index = i + (cnt - z_cnt + 1);
            // extended file content, a random digit.
            directory[i] = rand() % 9 + 1;
        }
    }
    // updating the file size and the available amount of blocks.
    directory_table.at(file_id).size += amount;
    available -= amount;
    num_extensions++;
    return;
}

// method used to find file that corresponds to the given start index within the directory.
int find_start(int ind, map<int, File> directory_table)
{

    for (int i = 0; i < num_files; i++)
    {
        // there's a corresponding file to the given start index.
        if (directory_table.at(i).start_index == ind)
            return i;
    }

    return -1;
}