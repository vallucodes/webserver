#pragma once

#include <iostream>
#include "Request.hpp"

class Parser{
    private:
        Request createRequest(const std::string& httpString);
    public:
       const std::string& createResponse( const std::string& request );
};