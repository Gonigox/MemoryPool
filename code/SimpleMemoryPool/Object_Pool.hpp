
// Este código es de dominio público.
// angel.rodriguez@esne.edu
// 2020.10

#pragma once

#include "Memory_Pool.hpp"

namespace utils
{

    template< typename ELEMENT >
    class Object_Pool
    {
    public:

        typedef ELEMENT Element;

    private:

        Memory_Pool< sizeof(Element) > memory_pool;

        size_t currently_allocated;

    public:

        Object_Pool() = delete;
        Object_Pool(Object_Pool && ) = delete;
        Object_Pool(const Object_Pool & ) = delete;

        Object_Pool(size_t desired_pool_size) : memory_pool(desired_pool_size)
        {
            currently_allocated = false;
        }

       ~Object_Pool()
        {
            assert(currently_allocated == 0);
        }

        template <typename ...ARGUMENTS >
        Element * allocate (ARGUMENTS && ...arguments)
        {
            auto chunk = memory_pool.allocate ();

            if (chunk)
            {
              ++currently_allocated;

                return new (chunk) Element(std::forward< ARGUMENTS >(arguments)...);
            }

            return nullptr;
        }

        void free (Element * element)
        {
            if (element)
            {
                element->~Element ();

                memory_pool.free (element);

              --currently_allocated;
            }
        }

    };

}
