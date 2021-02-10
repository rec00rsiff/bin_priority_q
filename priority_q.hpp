#include <iostream>
#include <mutex>

#define DEF_INIT_LEN 1
#define RAII_LOCK(mtx) std::lock_guard<std::mutex> lock(mtx); 

#define RESIZE_FACTOR 2

#define PARENT(i) (i - 1) / 2
#define LEFT(i) (i * 2) + 1
#define RIGHT(i) (i * 2) + 2

template <typename T>
class priority_q
{
private:
    std::mutex mtx;
    T* buf;
    size_t buflen;
    size_t bufpos;
public:
    priority_q()
    {
        buf = new T[DEF_INIT_LEN];
        buflen = DEF_INIT_LEN;
        bufpos = 0;
    }
    
    priority_q(size_t init_len)
    {
        buf = new T[init_len];
        buflen = init_len;
        bufpos = 0;
    }
    
    ~priority_q()
    {
        delete [] buf;
    }
    
    void resize(size_t len)
    {
        RAII_LOCK(mtx);
        if(len <= buflen) { buflen = len; return; }
        T* nbuf = new T[len];
        memcpy(nbuf, buf, buflen * sizeof(T));
        delete [] buf;
        buf = nbuf;
        buflen = len;
    }
    
    void resize_unsafe(size_t len)
    {
        if(len <= buflen) { buflen = len; return; }
        T* nbuf = new T[len];
        memcpy(nbuf, buf, buflen * sizeof(T));
        delete [] buf;
        buf = nbuf;
        buflen = len;
    }
    
    //0 - ok, 1 - err
    uint8_t top(T* dat)
    {
        RAII_LOCK(mtx);
        if(bufpos == 0) { return 1; }
        *dat = buf[0];
        return 0;
    }
    
    uint8_t push(T el)
    {
        RAII_LOCK(mtx);
        if(bufpos == buflen)
        {
            resize_unsafe(buflen * RESIZE_FACTOR);
        }
        
        size_t i = bufpos;
        buf[i] = el;
        ++bufpos;
        
        while(i > 0)
        {
            size_t pi = PARENT(i);
            if(buf[i] < buf[pi])
            {
                T pv = buf[pi];
                buf[pi] = buf[i];
                buf[i] = pv;
            }
            else
            {
                return 0;
            }
            i = pi;
        }
        return 0;
    }
    
    uint8_t pop()
    {
        RAII_LOCK(mtx);
        if(bufpos == 0) { return 1; }
        
        buf[0] = buf[bufpos - 1];
        --bufpos;
        
        size_t leafl = 0;
        size_t leafr = 0;
        size_t low_i = 0;
        for(size_t i = 0; i < bufpos; )
        {
            leafl = LEFT(i);
            leafr = RIGHT(i);
            low_i = i;
            
            if(leafl < bufpos && buf[leafl] < buf[low_i])
            {
                low_i = leafl;
            }
            if(leafr < bufpos && buf[leafr] < buf[low_i])
            {
                low_i = leafr;
            }
            
            if(low_i != i)
            {
                T low_v = buf[low_i];
                buf[low_i] = buf[i];
                buf[i] = low_v;
                i = low_i;
            }
            else
            {
                break;
            }
        }
        return 0;
    }
};
