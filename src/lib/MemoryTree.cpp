void MemoryTree::insert(void* data, size_t size) {
    if (!data) {
        // TODO: handle errors
        return;
    }

    // The case when we're empty
    if (!m_data) {
        m_data = data;
        m_size = size;
    }

    TODO("not implemented");
}
