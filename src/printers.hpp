#include <iostream>
#include "vq.h"
#include "defines.h"
#include "trace.h"
#include "register.h"

std::ostream& operator <<(std::ostream& osObject, const VQ& obj);

std::ostream& operator <<(std::ostream& osObject, const Instruction & obj);

std::ostream& operator <<(std::ostream& osObject, const Register & obj);

std::ostream& operator <<(std::ostream& osObject , const std::map<int , TraceEntry> & obj);
