#ifndef VECTOR_H
#define VECTOR_H

template <typename T>
class vector {
    private:
        T* data;
        int len;
        int len_reserved;
    public:

        // vector constructors
        vector(int size=16) { 
            data = new T[size]; 
            len = 0; len_reserved = size;
        }
        vector(int size, const T& value) { 
            data = new T[size]; 
            len = size; 
            len_reserved = size;
            for(int i=0; i<size; i++)
                data[i] = value;
        }

        void push_back(const T& value) {
            // if there is not space for the new object, allocate it
            if(len_reserved < len + 1)
                resize(len + 5);
            data[len] = value;
            len++;
        }

        void resize(int new_size) {
            if(new_size <= len_reserved) return;
            T* new_data = new T[new_size];
            for(int i=0; i<len; i++)
                new_data[i] = data[i];
            len_reserved = new_size;
            delete data;
            data = new_data;
        }

        const T& operator[](int index) const { 
            return data[index];
        }
        T& operator[](int index) {
            return data[index];
        }
        int length() { return len; }

        class iterator {
            public:
                iterator(vector<T>& owner, int index=0): owner(owner), index(index) {}
                const T& operator*() const {
                    return owner[index];
                }
                T& operator* (){
                    return owner[index];
                }
                iterator& operator++() {
                    index++;
                    return *this;
                }
                bool operator==(const iterator& other) const {
                    return &other.owner == &owner && other.index == index;
                }
                bool operator!=(const iterator& other) const {
                    return !(other == *this);
                }

            private:
                int index;
                vector<T>& owner;
        };
        iterator begin() {
            return iterator(*this, 0);
        }
        iterator end() {
            return iterator(*this, len+1);
        }
        class const_iterator {
            public:
                const_iterator(const vector<T>& owner, int index=0): owner(owner), index(index) {}
                const T& operator*() const {
                    return owner[index];
                }
                const_iterator& operator++() {
                    index++;
                    return *this;
                }
                bool operator==(const const_iterator& other) const {
                    return &other.owner == &owner && other.index == index;
                }
                bool operator!=(const const_iterator& other) const {
                    return !(other == *this);
                }

            private:
                int index;
                const vector<T>& owner;
        };
        const_iterator begin() const {
            return const_iterator(*this, 0);
        }
        const_iterator end() const {
            return const_iterator(*this, len+1);
        }

};

#endif

