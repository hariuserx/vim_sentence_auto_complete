// Execute with  g++ automater.cpp -std=c++17 -lstdc++fs 
// g++ (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0
// Sample run command ./a.out none sanitizePatterns.txt 0 1 100, use existing train data and dont clone any repo.
// author: hari kishore
// Top K sentences are collected using min heap and trie data structure. Inspired from https://www.geeksforgeeks.org/find-the-k-most-frequent-words-from-a-file/
// top k frequent words

#include <iostream>
#include <stdlib.h>
#include <experimental/filesystem>
#include <string>
#include <vector>
#include <regex>
#include <fstream>


# define CHAR_START 32
# define CHAR_END 127
# define MAX_CHARS 96 

using namespace std;
namespace fs = experimental::filesystem;

namespace git {
	const char* userName = "harikishore-xxxxx";	// github user name
	const char* token = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";	// github auth token
	
	char* get_clone_cmd(char* repoName, const char* repoDir){
		char* buffer = (char*)malloc(200 * sizeof(char));
		sprintf(buffer, "git clone https://%s:%s@github.com/%s %s", userName, token, repoName, repoDir);	
		return buffer;
	}
}

namespace stringUtils{
	
	const string delimiters = " \f\n\r\t\v";

	inline string trim_right(const string& s){
		if(s.size() == 0)
			return s;
		return s.substr(0, s.find_last_not_of(delimiters) + 1);
	}

	inline string trim_left(const string& s){
		if(s.size() == 0)
			return s;
		return s.substr(s.find_first_not_of(delimiters));
	}

	inline string trim(const string& s){
		return trim_left(trim_right(s));
	}
}

struct TrieNode{
	bool isEnd;
	unsigned frequency;
	int minHeapIndex;
	TrieNode* child[MAX_CHARS]; // sequence of words from space to del.
};

struct MinHeapNode{
	TrieNode* node;
	unsigned frequency;
	char* sentence;
};

struct MinHeap{
	int capacity;
	int count;
	MinHeapNode* array;
};

// declarations
void exec(char*);
vector<string> getAllFilesMatchingRegex(const char*, const char*);
void collectAllDescriptions(vector<string>&);
void collectMostCommonLines(const char*, const char*, int);
void sanitize(const char*, const char*, const char*);
void getFreqSentences(const char*, int, const char*);
MinHeap* constructMinHeapOfSize(int);
void insertTrieAndHeap(TrieNode**, const char*, MinHeap*);
void insertUtil(TrieNode**, MinHeap*, const char*, const char*);
TrieNode* newTrieNode();
void insertInMinHeap(MinHeap*, TrieNode**, const char*);
void buildMinHeap(MinHeap*);
void minHeapify(MinHeap*, int);
void swapMinHeapNodes(MinHeapNode*, MinHeapNode*);

void insertUtil(TrieNode** node, MinHeap* minHeap, const char* sentence, const char* dupSentence){

	if(*node == nullptr)
		*node = newTrieNode();

	if(*sentence != '\0')
		insertUtil(&((*node)->child[tolower(*sentence) - 32]), minHeap, sentence + 1, dupSentence);
	else{
		if((*node)->isEnd)
			++((*node)->frequency);
		else{
			(*node)->isEnd = 1;
			(*node)->frequency = 1;
		}
		insertInMinHeap(minHeap, node, dupSentence);
	}

}

void insertTrieAndHeap(TrieNode** root, const char* sentence, MinHeap* minHeap){
	if(sentence[0] == '\0')
		return;
	insertUtil(root, minHeap, sentence, sentence);
}

void swapMinHeapNodes(MinHeapNode* n1, MinHeapNode* n2){
	MinHeapNode temp = *n1;
	*n1 = *n2;
	*n2 = temp;
}

void minHeapify(MinHeap* minHeap, int i){
	int left = 2*i + 1;
	int right = 2*i + 2;
	int smallest = i;

	if(left < minHeap->count && minHeap->array[left].frequency <
			minHeap->array[smallest].frequency)
		smallest = left;
	if(right < minHeap->count && minHeap->array[right].frequency <
			minHeap->array[smallest].frequency)
		smallest = right;
	if(smallest != i){
		minHeap->array[smallest].node->minHeapIndex = i;
		minHeap->array[i].node->minHeapIndex = smallest;

		swapMinHeapNodes(&minHeap->array[smallest], &minHeap->array[i]);

		minHeapify(minHeap, smallest);
	}
}

void buildMinHeap(MinHeap* minHeap){
	
	for(int i = (minHeap->count -1 -1)/2; i >=0; --i)
		minHeapify(minHeap, i);
}

void insertInMinHeap(MinHeap* minHeap, TrieNode** root, const char* sentence){
	if((*root)->minHeapIndex != -1){
		++(minHeap->array[(*root)->minHeapIndex].frequency);	
		minHeapify(minHeap, (*root)->minHeapIndex);
	}
	else if(minHeap->count < minHeap->capacity){
		int count = minHeap->count;
		minHeap->array[count].frequency = (*root)->frequency;
		minHeap->array[count].sentence = new char[strlen(sentence) + 1]; // give space for null char
		strcpy(minHeap->array[count].sentence, sentence);

		minHeap->array[count].node = *root;
		(*root)->minHeapIndex = minHeap->count;

		++(minHeap->count);
		buildMinHeap(minHeap);
	}
	else if((*root)->frequency > minHeap->array[0].frequency){
		minHeap->array[0].node->minHeapIndex = -1;
		minHeap->array[0].node = *root;
		minHeap->array[0].node->minHeapIndex = 0;		
		minHeap->array[0].frequency = (*root)->frequency;

		delete [] minHeap->array[0].sentence;
		minHeap->array[0].sentence = new char[strlen(sentence) + 1];
		strcpy(minHeap->array[0].sentence, sentence);

		minHeapify(minHeap, 0);	
	}
}


MinHeap* constructMinHeapOfSize(int size){
	MinHeap* minHeap = new MinHeap;
	minHeap->capacity = size;
	minHeap->count = 0;
	minHeap->array = new MinHeapNode[minHeap->capacity];

	return minHeap;
}

TrieNode* newTrieNode(){

	TrieNode* node = new TrieNode;
	node->isEnd = 0;
	node->frequency = 0;
	node->minHeapIndex = -1;
	for (int i = 0; i < MAX_CHARS; i++)
		node->child[i] = nullptr;
	return node;
}

void displayAndWriteMinHeap(MinHeap* minHeap, const char* outputFilePath){
	ofstream out(outputFilePath);
	if(out.is_open()){
		for (int i = 0; i < minHeap->count; i++){
			printf("%s\t%d\n", minHeap->array[i].sentence, minHeap->array[i].frequency);
			out<<minHeap->array[i].sentence<<"\t"<<minHeap->array[i].frequency<<"\n";
		}	
	}
	else
		cerr<<"Unable to open freq file for writing\n";
	out.close();
}

void getFreqSentences(const char* inputFilePath, int k, const char* outputFilePath){
	
	MinHeap* minHeap = constructMinHeapOfSize(k);
	TrieNode* root = nullptr;

	ifstream myFile(inputFilePath);
	if(!myFile.is_open())
		cerr<<"Unable to open file"<<endl;
	else{
		string line;
		while(getline(myFile, line))
			insertTrieAndHeap(&root, line.c_str(), minHeap);	
		
	}

	displayAndWriteMinHeap(minHeap, outputFilePath);

	// free up resources	
	myFile.close();
	free(minHeap->array);
	free(minHeap);
}

void sanitize(const char* inputFilePath, const char* outputFilePath, const char* sanitizePatternFile){
	cout<<endl<<endl;

	vector<regex> patterns;

	if(sanitizePatternFile == nullptr)
		cout<<"No sanitize pattern file is provided. Skipping sanitization"<<endl;
	else{
		const char* header = "Pattern\tDescription";
		const char* split = "\t";
		printf("Sanitizing with %s\n", sanitizePatternFile);

		ifstream myFile(sanitizePatternFile);

		if(myFile.is_open()){
			string line;
			bool headerParsed = false;
			while(getline(myFile, line)){
				if(!headerParsed && line == header)
					headerParsed = true;
				else{
					char* charLine = strdup(line.c_str());
					char* token = strtok(charLine, split);
					if(token != nullptr)
						patterns.push_back(regex(token));
					// free heap space
				  	free(charLine);	
				}
			}		
		}
		myFile.close();
	}

	ifstream myFile(inputFilePath);
	ofstream out(outputFilePath);

	if(out.is_open()){
		if(myFile.is_open()){
			string line;
			while(getline(myFile, line)){
				string result = line;
				for(vector<regex>::iterator it = patterns.begin(); it != patterns.end(); it++){
					result = regex_replace(result, *it, "");
				}
				out<<stringUtils::trim(result)<<endl;
			}	
		}
		myFile.close();	
	}
	out.close();

	cout<<endl<<endl;
}

void collectMostCommonLines(const char* inputFilePath, const char* outputFilePath, int k){
	getFreqSentences(inputFilePath, k, outputFilePath);
}

vector<string> getAllFilesMatchingRegex(const char* dirPath, const regex& pattern){

	vector<string> listOfFiles;

	try {
		if (fs::exists(dirPath) && fs::is_directory(dirPath))
		{
			fs::recursive_directory_iterator iter(dirPath);
 
			// Create a Recursive Directory Iterator object pointing to end.
			fs::recursive_directory_iterator end;
 
			while (iter != end)
			{
				if(fs::is_regular_file(iter->path()))
				{
					if(regex_match(iter->path().string(), pattern))
						listOfFiles.push_back(iter->path().string());
				}
 
				error_code ec;
				// Increment the iterator to point to next entry in recursive iteration
				iter.increment(ec);
				if (ec) {
					cerr << "Error While Accessing : " << iter->path().string() << " :: " << ec.message() << '\n';
				}
			}
		}
	}
	catch (system_error &e)
	{
		cerr << "Exception :: " << e.what();
	}
	catch (exception& e)
	{
		cerr << "Exception:: " << e.what();
	}

	return listOfFiles;
}

void exec(char* cmd){
	FILE *fp;
	int status;
	char buffer[2000];
	string result = "";
	
	fp = popen(cmd, "r");
	
	if(fp == nullptr)
		throw runtime_error("popen() failed!");
	try{
		while(fgets(buffer, sizeof(buffer), fp) != nullptr){
			result += buffer;
		}
	}catch(...){
		pclose(fp);
		throw;
	}
	
	pclose(fp);
	cout<<result<<endl;
}

void collectAllDescriptions(vector<string>& listOfFiles, const char* allDescrpFile){
	const char* descriptionStart = "{{JAMA_START}}"; // your description beginning. To read the entire file set it as ""
	const char* descriptionEnd = "{{JAMA_END}}"; 	// your description end. To read the entire file set it as
							// some value that you know won't be there in the file. Like "xzxzxzxzxzx"
	ofstream allDescriptions(allDescrpFile);
	if(allDescriptions.is_open()){

		for(vector<string>::iterator it = listOfFiles.begin(); it != listOfFiles.end(); it++){
			ifstream myfile(*it);
			if(myfile.is_open()){
				string line;
				bool description = false;
				while(getline(myfile, line)){

					if(stringUtils::trim(line).find(descriptionStart) != string::npos){
						description = true;
					}

					if(description)
						allDescriptions << stringUtils::trim(line)<< "\n";

					if(stringUtils::trim(line).find(descriptionEnd) != string::npos){
						description = false;
					}
				}		
			}
			myfile.close();
		}

		allDescriptions.close();
	}
	else
		cout<<"Unable to open file";
}

typedef char* (*Operation)(int argc, char** argv, int index);

struct ArgsHandler{
	int argc;
	char** argv;
	Operation opt;
};

char* getArgValue(int argc, char** argv, int index){
	if(index > argc)
		return nullptr;
	return argv[index - 1];
}

int main(int argc, char **argv){

	const char *train_data_dir = "train_data";

	cout<<"Hello"<<endl;

	error_code errorCode;

	ArgsHandler argsHandler;
	argsHandler.argc = argc;
	argsHandler.argv = argv;
	argsHandler.opt = getArgValue;
	char* sanitizePatternFile = argsHandler.opt(argsHandler.argc, argsHandler.argv, 3);
	char* clone = argsHandler.opt(argsHandler.argc, argsHandler.argv, 4);
	char* collectDescr = argsHandler.opt(argsHandler.argc, argsHandler.argv, 5);
	char* topFreq = argsHandler.opt(argsHandler.argc, argsHandler.argv, 6);
	
	char *cmd = nullptr;
	bool cloneV = true;
	bool collectDescrV = true;
	int k = 50;
	if(clone != nullptr)
		istringstream(clone) >> cloneV;
	if(collectDescr != nullptr)
		istringstream(collectDescr) >> collectDescrV;
	if(topFreq != nullptr)
		istringstream(topFreq)	>> k;
	if(cloneV){
		// remove directory first	
		if(fs::exists(train_data_dir, errorCode)){
			if (!fs::remove_all(train_data_dir, errorCode)) {
			    cout << errorCode.message() << endl;
			}
		}
		
		// mkdir Repos
		if (!fs::create_directory(train_data_dir)){
		    cout << errorCode.message() << endl;
		    exit(1);
		}
		cout<<"Cloning repository: "<<argv[1]<<endl;
		
		cmd = git::get_clone_cmd(argv[1], train_data_dir);
		cout<<cmd<<endl;
		exec(cmd);
	}


	regex pattern("(.*)\\.py");	// set your pattern of files
	vector<string> pyFiles = getAllFilesMatchingRegex(train_data_dir, pattern);
	
	cout<<pyFiles.size()<<endl;

	for(vector<string>::iterator it = pyFiles.begin(); it != pyFiles.end(); it++)
		cout<<*it<<endl;
	
	const char* allDescFile = "AllDescriptions.txt";
	const char* allDescFileSanitized = "AllDescriptionsSanitized.txt";
	const char* mostFreqLinesFile = "mostFeqLines.txt";

	if(collectDescrV){
		collectAllDescriptions(pyFiles, allDescFile);
		sanitize(allDescFile, allDescFileSanitized, sanitizePatternFile);
	}
	collectMostCommonLines(allDescFileSanitized, mostFreqLinesFile, k);

	// free up the heap
	free(cmd);
	
	return 0;
}
