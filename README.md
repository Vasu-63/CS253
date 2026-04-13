# CS253 Assignment 1 — Memory-Efficient Versioned File Indexer

A C++ program that builds a word-frequency index over large text files using a fixed-size buffer, and supports three query types over the indexed data.

---

## Compilation

```bash
g++ -std=c++17 -O2 -o analyzer solution.cpp
```

---

## Usage

### Word Query
Returns the frequency of a word in a file.
```bash
./analyzer --file text_logs.txt --version v1 --buffer 512 --query word --word error
```

### Top-K Query
Lists the K most frequent words in a file.
```bash
./analyzer --file text_logs.txt --version v1 --buffer 512 --query top --top 10
```

### Difference Query
Shows the frequency difference of a word between two files.
```bash
./analyzer --file1 text_logs.txt --version1 v1 --file2 verbose_logs.txt --version2 v2 --buffer 512 --query diff --word error
```

---

## Flags

| Flag | Description |
|------|-------------|
| `--file` | Input file (word / top queries) |
| `--file1` | First input file (diff query) |
| `--file2` | Second input file (diff query) |
| `--version` | Version name (word / top queries) |
| `--version1` | First version name (diff query) |
| `--version2` | Second version name (diff query) |
| `--buffer` | Buffer size in KB (256 to 1024) |
| `--query` | Query type: `word`, `top`, or `diff` |
| `--word` | Word to search (word / diff queries) |
| `--top` | Number of top results (top query) |

---

## Design

The program is built around four classes:

- **FrequencyMap\<K,V\>** — template class that maps words to their counts
- **Version** — stores one FrequencyMap per named version
- **BufferedFileReader** — reads the file in fixed-size chunks, never loading it fully into memory
- **Tokenizer** — extracts lowercase alphanumeric tokens from buffer chunks, handles words split across buffer boundaries
- **Query** — abstract base class; derived by `WordCountQuery`, `Top_KQuery`, and `DifferenceQuery`

---

## Assumptions

- Words are contiguous sequences of alphanumeric characters
- Word matching is case-insensitive
- Buffer size must be between 256 and 1024 KB
