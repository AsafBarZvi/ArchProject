#include <iostream>
#include "vq.h"

std::ostream& operator <<(std::ostream& osObject, const VQ& obj)
{
    return osObject << "tag:" <<  obj.tag() << ",val:" << obj.val(); 
}
