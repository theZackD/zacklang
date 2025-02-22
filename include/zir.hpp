// include/zir.hpp
#ifndef ZIR_HPP
#define ZIR_HPP

#include <memory>
#include <string>

namespace zir
{

    class ZIRBuilder
    {
    public:
        ZIRBuilder() = default;
        ~ZIRBuilder() = default;

        // Simple test method
        std::string getVersion() const { return "0.1.0"; }
    };

} // namespace zir

#endif // ZIR_HPP