
// Este código es de dominio público.
// angel.rodriguez@esne.edu
// 2020.10

#pragma once

#include <new>
#include <cassert>
#include <cstddef>

namespace utils
{

    using std::byte;

    // Esta implementación es simple para intentar dejar claras algunas ideas básicas.
    // Si se desea hacer una implementación mejor se podrían realizar los siguientes ajustes:
    //
    //  + Hacer que la clase no sea una plantilla, ya que cada ITEM_SIZE provoca una instanciación.
    //    Ello implica deshacer Node y recibir ITEM_SIZE como parámetro del constructor.
    //  + Tener en cuenta la alineación de los items.
    //  + Tener en cuenta la alineación del puntero next.
    //  + Hacer que la memory pool pueda crecer y decrecer mediante el uso de segmentación.
    //
    template< size_t ITEM_SIZE >
    class Memory_Pool
    {

        struct Node
        {
            byte   item[ITEM_SIZE];
            Node * next;
        };

        size_t pool_size;
        Node * node_array;
        Node * free_nodes;

    public:

        Memory_Pool() = delete;
        Memory_Pool(Memory_Pool && ) = delete;
        Memory_Pool(const Memory_Pool & ) = delete;

        Memory_Pool(size_t desired_pool_size)
        {
            free_nodes = node_array = new (std::nothrow) Node[pool_size = desired_pool_size];

            if (node_array)
            {
                Node * last = node_array + pool_size - 1;

                for (Node * node = node_array; node < last; ++node)
                {
                    node->next = node + 1;
                }

                last->next = nullptr;
            }
            else
                pool_size = 0;
        }

       ~Memory_Pool()
        {
            delete [] node_array;
        }

        size_t size () const
        {
            return pool_size;
        }

        void * allocate ()
        {
            Node * item = free_nodes;

            if (item)
            {
                free_nodes = item->next;
            }

            return item;
        }

        void free (void * chunk)
        {
            Node * item = reinterpret_cast< Node * >(chunk);

            if (item < node_array || item >= node_array + pool_size)
            {
                assert(false);
            }
            else
            {
                item->next = free_nodes;

                free_nodes = item;
            }
        }

    };

}
