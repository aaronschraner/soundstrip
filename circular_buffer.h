#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

template <typename T, int size>
class CircularBuffer {
    private:
        T data[size];
        int starti, endi, len; //start index, end index, length (of valid data)

        void advance(int& ptr) {
            ptr = (ptr + 1) % size;
        }

    public:
        // default constructor
        CircularBuffer(): starti(0), endi(0), len(0)
        {}
        bool push(T value) {
            if(len < size)
            {
                len++;
                data[endi] = value;
                advance(endi);
                return true;
            } else {
                //return false; // (do not) pop something out of the way if buffer is full
                len = size;
                data[endi] = value;
                advance(endi);
                advance(starti);
                return false;
            }

        }
        T pop() { 
            if(len > 0) {
                len--;
                T& temp = data[starti];
                advance(starti);
                return temp;
            }
            // return default constructed object if buffer empty
            T d;
            return d;
        }
        int length() const { return len; }
        bool empty() const { return len == 0; }
        bool full() const { return len == size; }
        void flush() { starti = endi = len = 0; }
        T& operator[] (int index) {
            return data[(index + starti) % size];
        }
        const T& operator[] (int index) const {
            return data[(index + starti) % size];
        }
        // iterator class
        class Iterator {
            private:
                CircularBuffer& owner;
                int index;

            public:
                Iterator(CircularBuffer& owner, int index):owner(owner), index(index) {}
                Iterator(const Iterator& i):owner(i.owner), index(i.index) {}
                T& operator*() { return owner[index]; }
                const T& operator*() const { return owner[index]; }
                Iterator& operator++() { index++; return *this; }
                bool operator==(const Iterator& other) const { return index == other.index && &owner == &other.owner; }
                bool operator!=(const Iterator& other) const { return !(*this == other); }
        };
        // begin and end for for(obj: list) syntax
        Iterator begin() {
            return Iterator(*this, 0);
        }
        Iterator end() {
            return Iterator(*this, size);
        }




};

#endif
