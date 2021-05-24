#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <queue>

#define RAII_LOCK(mtx) std::lock_guard<std::mutex> lock(mtx);

#define QTOP_PRINT(q, outm) \
if(!q.empty()) \
{ \
std::cout << q.top() << std::endl; \
} \
else \
{ \
std::cout << "err" << std::endl; \
} \

#define TEST_OPS 1000
#define TEST_PROD_THREADS 6
#define TEST_CONS_THREADS 1
#define TEST_SLEEP_MS 1

#define CHRONO_DBG_START(OP) \
std::chrono::high_resolution_clock::time_point OP##s = std::chrono::high_resolution_clock::now(); \

#define CHRONO_DBG_END(OP) \
std::chrono::duration<float> OP##d = std::chrono::high_resolution_clock::now() - OP##s; \
std::chrono::nanoseconds OP##ns = std::chrono::duration_cast<std::chrono::nanoseconds>(OP##d); \
//std::cout << #OP " op took " << OP##ns.count() << " nanos" << std::endl; \

#define LOG_SYNC(out_op) \
{ \
std::lock_guard<std::mutex> lock(log_mtx); \
out_op \
} \

std::priority_queue<int, std::vector<int>, std::greater<int> > q; //use greater to match

std::mutex log_mtx;

std::mutex stl_q_mtx;

void producer_loop()
{
    std::random_device rd;
    std::mt19937 r_eng(rd());
    std::uniform_int_distribution<> distr(INT_MIN, INT_MAX);
    
    size_t dall = 0;
    size_t dops = 0;
    size_t dextr = 0;
    
    while(dops < ((TEST_OPS * TEST_PROD_THREADS) / TEST_PROD_THREADS))
    {
        CHRONO_DBG_START(push);
        {
            RAII_LOCK(stl_q_mtx);
            q.push(distr(r_eng));
        }
        CHRONO_DBG_END(push);
        
        size_t dcnt = pushns.count();
        if(dcnt > dextr)
        {
            dextr = dcnt;
        }
        
        dall += dcnt;
        ++dops;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_MS));
    }
    
    LOG_SYNC(std::cout << "T" << std::this_thread::get_id() << " | avg push: " << (dall / dops) << " nanos, " << "extreme push: " << dextr << " nanos" << std::endl;);
}

void consumer_loop()
{
    int ctr = 0;
    
    size_t dall = 0;
    size_t dops = 0;
    size_t dextr = 0;
    while(1)
    {
        CHRONO_DBG_START(pop);
        uint8_t popr = 0;
        {
            RAII_LOCK(stl_q_mtx);
            if(q.empty())
            {
                popr = 1;
            }
            else
            {
                q.pop();
            }
        }
        CHRONO_DBG_END(pop);
        if(popr == 0)
        {
            ++ctr;
            
            size_t dcnt = popns.count();
            if(dcnt > dextr)
            {
                dextr = dcnt;
            }
            
            dall += dcnt;
            ++dops;
        }
        else if(ctr > 0)
        {
            break;
        }
        else
        {
            LOG_SYNC(std::cout << "zerospin" << std::endl;);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_MS));
    }
    
    LOG_SYNC(std::cout << "T" << std::this_thread::get_id() << " | avg pop: " << (dall / dops) << " nanos, " << "extreme pop: " << dextr << " nanos" << std::endl;);
    assert(ctr == (TEST_OPS * TEST_PROD_THREADS) /*DERR: consumed != produced*/);
}

int main()
{
    q.push(9);
    q.push(3);
    q.push(10);
    q.push(12);
    q.push(21);
    q.push(45);
    q.push(15);
    
    int out = 0;
    for(int i = 0; i < 7; ++i)
    {
        {
            RAII_LOCK(stl_q_mtx);
            QTOP_PRINT(q, out);
        }
        
        CHRONO_DBG_START(pop);
        {
            RAII_LOCK(stl_q_mtx);
            if(!q.empty())
            {
                q.pop();
            }
        }
        CHRONO_DBG_END(pop);
    }
    
    for(int i = 0; i < TEST_CONS_THREADS; ++i)
    {
        std::thread th(consumer_loop);
        th.detach();
    }
    
    for(int i = 0; i < TEST_PROD_THREADS; ++i)
    {
        std::thread th(producer_loop);
        th.detach();
    }
    
    std::cin.get();
    return 0;
}
