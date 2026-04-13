#include <bits/stdc++.h>
using namespace std;
//comparator used to sort in top-k query
bool cmp(const pair<string,int>& a, const pair<string,int>& b)
        { return a.second != b.second ? a.second > b.second : a.first < b.first; }
//=====================================================
// Template Definition and Frequency Map data structure
//=====================================================
template <typename K, typename V>
class FrequencyMap {
    unordered_map<K, V> data;
public:
    void increment(const K& key) { data[key]++; }

    V get(const K& key) const {
        auto it = data.find(key);
        return (it != data.end()) ? it->second : V{};
    }

    unordered_map<K, V>& getAll() { return data; }
};

//================================================================
// CLASS 1: Versioned index, to store one FrequencyMap per version
//================================================================
class Version {
    unordered_map<string, FrequencyMap<string, int>> store;
public:
    void addVersion(const string& name, FrequencyMap<string, int>&& fmap) {
        store[name] = std::move(fmap);
    }

    FrequencyMap<string, int>& getMap(const string& name) {
        auto it = store.find(name);
        if (it == store.end()) throw runtime_error("Version not found: " + name);
        return it->second;
    }
};

//=========================================================
// CLASS 2: Buffered file reader, to read fixed-size chunks
//=========================================================
class BufferedFileReader {
    ifstream file;
    vector<char> buffer;
    streamsize bytesRead;
public:
    BufferedFileReader(const string& path, int bufferBytes) : bytesRead(0) {
        buffer.resize(bufferBytes);
        file.open(path);

        if (!file.is_open())
            throw runtime_error("Cannot open file: " + path);
    }

    bool readNext() {
        file.read(buffer.data(), buffer.size());
        bytesRead = file.gcount();
        return bytesRead > 0;
    }

    vector<char>& getBuffer() { return buffer; }
    streamsize getBytesRead() { return bytesRead; }

    ~BufferedFileReader() { if (file.is_open()) file.close(); }
};

//===========================================================
// CLASS 3: Tokenizer, to handle cross-buffer word boundaries
//===========================================================
class Tokenizer {
    string partial;
    FrequencyMap<string, int>& index;

public:
    Tokenizer(FrequencyMap<string, int>& idx) : index(idx) {}

    void tokenize(const vector<char>& buf, int len) {
        for (int i = 0; i < len; i++) {
            char c = buf[i];
            if (isalnum((unsigned char)c))
                partial += tolower((unsigned char)c);
            else if (!partial.empty()) {
                index.increment(partial);
                partial.clear();
            }
        }
    }

    // Overload, to tokenize a plain string
    void tokenize(const string& text) {
        for (char c : text) {
            if (isalnum((unsigned char)c))
                partial += tolower((unsigned char)c);
            else if (!partial.empty()) {
                index.increment(partial);
                partial.clear();
            }
        }
        if (!partial.empty()) {
            index.increment(partial);
            partial.clear();
        }
    }

    void flush() {
        if (!partial.empty())
            tokenize(partial);
    }
};

// Building index from file
FrequencyMap<string, int> buildIndex(const string& path, int bufSize) {
    FrequencyMap<string, int> index;
    Tokenizer tokenizer(index);
    BufferedFileReader reader(path, bufSize);
    while (reader.readNext())
        tokenizer.tokenize(reader.getBuffer(), reader.getBytesRead());
    tokenizer.flush();
    return index;
}

//==========================
// CLASS 4: Query Processing
//==========================
class Query {
public:
    virtual ~Query() {}
    virtual void execute() = 0;
};

//==================
// Word count query
//==================
class WordCountQuery : public Query {
    string word, version;
    Version& vIndex;
public:
    WordCountQuery(string& w, Version& vi, string v) : word(w), version(v), vIndex(vi) {}

    void execute() override {
        cout << "Version: " << version << "\n";
        cout << "Count: " << vIndex.getMap(version).get(word) << "\n";
    }
};

//========================================
// Top-K query, use sort to find top words
//========================================
class Top_KQuery : public Query {
    Version& vIndex;
    int k;
    string version;
public:
    Top_KQuery(Version& vi, int kval, string v) : vIndex(vi), k(kval), version(v) {}

    void execute() override {
        auto& wordMap = vIndex.getMap(version).getAll();

        vector<pair<string,int>> sorted(wordMap.begin(), wordMap.end());
        sort(sorted.begin(), sorted.end(),cmp);

        int limit = min(k, (int)sorted.size());
        cout << "Top-" << k << " Words in version "<< version << "\n";
        for (int i = 0; i < limit; i++)
            cout << sorted[i].first <<" "<<sorted[i].second << "\n";
    }
};

//=================
// Difference query
//=================
class DifferenceQuery : public Query {
    Version& vIndex;
    string word, version1, version2;
public:
    DifferenceQuery(Version& vi, string& w, string v1, string v2)
        : vIndex(vi), word(w), version1(v1), version2(v2) {}

    void execute() override {
        int c1 = vIndex.getMap(version1).get(word);
        int c2 = vIndex.getMap(version2).get(word);
        cout << "Difference (v2-v1): " << (c2 - c1) << "\n";
    }
};

//=====
// Main
//=====
int main(int argc, char* argv[]) {
    string queryType, word, file1, file2, version1, version2;
    int bufferSize = 256*1024, k = 5;
    if (argc < 2) {
        cerr << "Error: No arguments provided.\n"; return 1;
    }
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (i + 1 >= argc) { cerr << "Error: Flag " << arg << " is missing a value.\n"; return 1; }

        if      (arg == "--query")                    queryType  = argv[++i];
        else if (arg == "--word")                     word = argv[++i];
        else if (arg == "--file" || arg == "--file1") file1 = argv[++i];
        else if (arg == "--file2")                    file2 = argv[++i];
        else if (arg == "--version" || arg == "--version1") version1 = argv[++i];
        else if (arg == "--version2")                 version2 = argv[++i];

        else if (arg == "--buffer") {
            // Buffer value must be a valid integer
            try { bufferSize = stoi(argv[++i]) * 1024; }
            catch (...) { cerr << "Error: --buffer requires a valid integer.\n"; return 1; }
        }

        else if (arg == "--top") {
            // Top-K value must be a valid integer
            try { k = stoi(argv[++i]); }
            catch (...) { cerr << "Error: --top requires a valid integer.\n"; return 1; }
        }

        else  {cerr << "Error: Unknown flag '" << arg << "'.\n"; return 1;}
    }
//=========================================================================
// Error Handling
    
    // Required flags missing
    if (queryType.empty()) { cerr << "Error: --query is required.\n"; return 1; }
    if (file1.empty())     { cerr << "Error: --file is required.\n";  return 1;  }

    // Invalid query type
    if (queryType != "word" && queryType != "top" && queryType != "diff") {
        cerr << "Error: Unknown query type '" << queryType << "'. Must be word | top | diff.\n";
        return 1;
    }

    // Buffer size out of range
    if (bufferSize < 256*1024 || bufferSize > 1024*1024) {
        cerr << "Error: --buffer must be between 256 and 1024 KB. Got: " << bufferSize/1024 << " KB.\n";
        return 1;
    }

    // Top-K must be positive
    if (queryType == "top" && k <= 0) {
        cerr << "Error: --top value must be a positive integer. Got: " << k << ".\n";
        return 1;
    }

    // Word required for word and diff queries
    if ((queryType == "word" || queryType == "diff") && word.empty()) {
        cerr << "Error: --word is required for '" << queryType << "' query.\n";
        return 1;
    }

    // Diff query needs two files and two versions
    if (queryType == "diff") {
        if (file2.empty())    { cerr << "Error: --file2 is required for diff query.\n";    return 1; }
        if (version1.empty()) { cerr << "Error: --version1 is required for diff query.\n"; return 1; }
        if (version2.empty()) { cerr << "Error: --version2 is required for diff query.\n"; return 1; }
        if (version1 == version2) { cerr << "Error: --version1 and --version2 must be different.\n"; return 1; }
        if (file1 == file2)   { cerr << "Error: --file1 and --file2 must be different files.\n"; return 1; }
    }

    // Single-version queries need a version name
    if ((queryType == "word" || queryType == "top") && version1.empty()) {
        cerr << "Error: --version is required for '" << queryType << "' query.\n";
        return 1;
    }
//=================================================================================
    
    for (char& c : word) c = tolower((unsigned char)c);

    auto startTime = chrono::high_resolution_clock::now();
    Query* query = nullptr;
    Version vIndex;

    try {
        if (queryType == "word") {
            vIndex.addVersion(version1, buildIndex(file1, bufferSize));
            query = new WordCountQuery(word, vIndex, version1);
        }
        else if (queryType == "top") {
            vIndex.addVersion(version1, buildIndex(file1, bufferSize));
            query = new Top_KQuery(vIndex, k, version1);
        }
        else if (queryType == "diff") {
            vIndex.addVersion(version1, buildIndex(file1, bufferSize));
            vIndex.addVersion(version2, buildIndex(file2, bufferSize));
            query = new DifferenceQuery(vIndex, word, version1, version2);
        }

        if (query) { query->execute(); delete query; }
    }
    catch (const invalid_argument& e) { cerr << "Invalid Argument: " << e.what() << "\n"; return 1; }
    catch (const runtime_error& e)    { cerr << "Runtime Error: "    << e.what() << "\n"; return 1; }
    catch (const exception& e)        { cerr << "Error: "            << e.what() << "\n"; return 1; }

    auto endTime = chrono::high_resolution_clock::now();
    cout << "Buffer Size: " << bufferSize / 1024 << " KB\n";
    cout << "Execution Time: " << chrono::duration<double>(endTime - startTime).count() << " seconds\n";

    return 0;
}