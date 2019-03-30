# vim_sentence_auto_complete
A sentence auto completer plugin for VIM with a file containing most frequent sentences. A C++ utility is used to get the most frequent sentences from a given data set.

Plugin was developed to ease up writing repeated JAMA automated test descriptions.

Most frequent sentences are collected using trie and heap data structure for faster performance and low memory foot print for large data sets.

##### Generating most frequent sentences from a dataset/github repo

1. Compile automater.cpp
```bash
g++ automater.cpp -std=c++17 -lstdc++fs 
```
2. Get the most frequent sentences from an existing folder `train_data` or freshly clone a repo from github you wish to collect most frequent lines from(set your appropriate github username and authtoken in the source file). Training data file pattern, description start/stop are hardcoded in the file which can be modified.<br>
> Sample command:<br>
use existing train data without cloning any repo and get the top 100 frequent sentences.
```bash
./a.out none sanitizePatterns.txt 0 1 100
```

3. The output will be a file `mostFeqLines.txt` containing tab separated sentences with frequency.

##### Using the plugin

1. Filter the output from previous steps as per your requirement and create a file `most_freq_lines.txt` with the final list of sentences that will be used for VIM autocomplete.

2. Make sure the plugin file `.vim` and `most_freq_lines.txt` are in the same directory. You can source it in `.vimrc` or source it using `:source hari_autocomplete.vim` from your current buffer.

3. Once sourced use `:SSC` to active the new omni function and `:USSC` to reset it. Completion can be triggered using `Ctrl-x Ctrl-o` in insert mode. Both the lines from current file and `most_freq_lines.txt` are used in auto completion.
