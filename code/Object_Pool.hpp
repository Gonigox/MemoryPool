// Este código es de dominio publico
// gpintoscuesta@gmail.com
// 11.2020

#pragma once

#include "Memory_Pool.hpp"
#include <vector>

namespace pooling
{
    template < typename OBJECT >
    class Object_Pool
    {
    public:

        typedef OBJECT Object;

    private:

        /// Claramente la alineación del los nodos o el puntero next esta estropeando el rendimiento
        struct Node
        {
            Object item;
            Node * next;
        };

        Memory_Pool memory_pool;

        size_t      currently_allocated;
        Node *      free_nodes;

    public:

        Object_Pool() = delete;
        Object_Pool(Object_Pool &&) = delete;
        Object_Pool(const Object_Pool &&) = delete;

        Object_Pool(size_t pool_size) : memory_pool(sizeof(Node) * pool_size)
        {
            currently_allocated = false;
            free_nodes          = reinterpret_cast < Node * > (memory_pool[0]);

            /// Este bucle hace que la construccion del object_pool sea O(n), hay que trabajarlo
            /// Este es mi problema de rendimiento me tengo que evitar este bucle
            if(memory_pool.is_ok())
            {
                Node * last = reinterpret_cast < Node * > (memory_pool[memory_pool.size() - sizeof(Node)]);

                for 
                (
                    Node * node = free_nodes;
                    node < last;
                    ++node
                )
                {
                    node->next = node + 1;
                }

                last->next = nullptr;
            }
        }

        ~Object_Pool()
        {
            assert(currently_allocated == 0);
        }

        Object & operator[] (size_t index)
        {
            return *(reinterpret_cast < Object * > (memory_pool[sizeof(Node) * index]));
        }
        
       

        template < typename ...ARGUMETS >
        Object * allocate (ARGUMETS && ...arguments)
        {            
            auto element = memory_pool.allocate(sizeof(Node), free_nodes);

            if (element)
            {
                ++currently_allocated;

                free_nodes = reinterpret_cast < Node * > (element)->next;

                reinterpret_cast < Node * > (element)->next = free_nodes;

                return new (reinterpret_cast< Object * > (element)) Object(std::forward< ARGUMETS >(arguments)...);
            }

            return nullptr;
        }

        void free (Object * object)
        {
            if (object)
            {
                /// ¿Porque es necesario borrarlo en lugar de simplemente sobreescribirlo?
                /// Porque el objecto al crearse puede que reserva memoria en su constructor y hay que liberarla
                object->~Object();

                reinterpret_cast < Node * > (object)->next = free_nodes;

                free_nodes = reinterpret_cast < Node * > (object);

                --currently_allocated;
            }
        }

    };
}