export module Memory:MemoryTree;

import std;

export class MemoryTree {
public:
    void insert(void* data, std::size_t size);
    MemoryTree* descendLeft();
    MemoryTree* descendRight();

private:
    void* m_data = nullptr;
    std::size_t m_size = 0;
    MemoryTree* m_left = nullptr;
    MemoryTree* m_right = nullptr;
};
