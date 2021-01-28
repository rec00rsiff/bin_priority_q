#include "priority_q.hpp"

#define QTOP_PRINT(q, outm) \
if(q.top(&outm) == 0) \
{ \
std::cout << outm << std::endl; \
} \
else \
{ \
std::cout << "err" << std::endl; \
} \

int main()
{
    priority_q<int> q(2);
    q.push(9);
    q.push(3);
    q.push(10);
    q.push(12);
    q.push(21);
    q.push(45);
    q.push(15);
    
    int out = 0;
    for(int i = 0; i < 8; ++i)
    {
        QTOP_PRINT(q, out);
        q.pop();
    }
    
    return 0;
}
