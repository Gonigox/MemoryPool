
// Código de gpintosuesta@gmail.com
// 
// 2021.01

#include "Memory_Pool.hpp"
#include <stdexcept>

namespace pooling
{
    Memory_Pool::Memory_Pool (size_t desired_pool_size)
    {            
        pool_array    =  new byte * [segment_num = segment_index = 1];
       *pool_array    =  new (std::nothrow) byte [pool_init_size = desired_pool_size];
        pool_trail    = *pool_array;
        pool_size     = *pool_array ? desired_pool_size : 0;
        pool_end      = *pool_array + pool_size; /// Error en allocate mirar ifs
        segment_size  =  false;
        pool_consumed =  false;
    }

    Memory_Pool::~Memory_Pool ()
    {
       for(size_t i = 0; i < segment_num; ++i)
       {
           delete [] pool_array[i];
       }

       delete [] pool_array;
    }

    /** Yo creo que la division entre segment_size se tiene que poder sacar.
      */ 
    byte * Memory_Pool::operator[] (const size_t index) const
    {        
        if (index < pool_init_size)
        {
            return &(*pool_array)[index];
        }
        else
        if (segment_num > 1)
        {            
            if (size_t segmet_objetive = (index - pool_init_size) / segment_size > 0)
            {
                return &(pool_array[segmet_objetive + 1])
                        [
                            (index - pool_init_size) - (segmet_objetive * segment_size)
                        ];
            }
            else
            {                    
                return &(pool_array[segmet_objetive + 1])[index - pool_init_size];
            }
        }
        else
        {
            return nullptr;
        }
    }

    void * Memory_Pool::allocate (size_t amount)
    { 
        if ((pool_trail += amount) <= pool_end)
        {
            pool_consumed +=  amount;
            return pool_trail - amount;
        }
        else 
        if (segment_size > 0)
        {
            pool_trail     = new_segment();
            pool_end       = pool_trail + segment_size;
            pool_trail    += amount;
            pool_consumed += amount;

            return pool_trail - amount;
        }
        else
        {
            pool_trail -= amount;
            return nullptr;
        }
    }

    void * Memory_Pool::allocate (size_t amount, void * position)
    {
        byte * element = reinterpret_cast < byte * > (position);

        if (element == pool_trail)
        {
            pool_consumed += amount;
        }

        if ((pool_trail += amount) < pool_end)
        {            
            return pool_trail - amount;
        }
        else
        if (segment_size > 0)
        {            
            pool_trail     = new_segment();
            pool_end       = pool_trail + segment_size;
            pool_trail    += amount;
            pool_consumed += amount;

            return pool_trail - amount;            
        }
        else
        {
            pool_trail -= amount;
            return nullptr;
        }   
    }

    void Memory_Pool::free(size_t amount)
    { 
        if(segment_index <= 1)
        {
            if (*pool_array <= (pool_trail - amount))
            {
                pool_consumed -= amount;
                pool_trail    -= amount;
            }
        }
        else
        {
            if (pool_array[segment_index - 1] <= (pool_trail -= amount))
            {
                pool_consumed -= amount;
            }
            else
            { 
                /// No borro la memoria de los segmentos ya que si el usuario los necesitó en algun
                /// momento probablemente los vuelva a necesitar.
                //delete [] pool_array[--segment_index];
                --segment_index;

                pool_consumed -= amount;

                if (segment_index > 1)
                {
                    pool_end   = &pool_array[segment_index - 1][segment_size];
                    pool_trail = pool_end - amount;
                }
                else
                {
                    pool_end   = &pool_array[segment_index - 1][pool_init_size];
                    pool_trail = pool_end - amount;
                }
            }
        }
    }

    byte * Memory_Pool::new_segment()
    {
        /// Me aseguro que no existan segmentos creados anteriormente
        if ((pool_size - pool_consumed) > 0)
        {
            return pool_array[segment_num - (pool_size - pool_consumed) / segment_size];
        }
        else
        {
            byte ** aux = new byte * [++segment_num];
            
            segment_index = segment_num;
        
            for (size_t i = 0; i < segment_num - 1; ++i)
            {
                aux[i] = pool_array[i];
            }

            delete [] pool_array;

            pool_array = aux;

            pool_size += segment_size;

            return pool_array[segment_num - 1] = new (std::nothrow) byte [segment_size];
        }
    }
}


