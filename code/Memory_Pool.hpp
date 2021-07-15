
// El codigo original era de angel.rodriguez@esne.edu
// Modificado por gpintoscuesta@gmail.com
// 2021.01

#pragma once

#include <new>
#include <cassert>
#include <cstddef>

using std::byte;                    // Requiere C++17

namespace pooling
{

    class Memory_Pool
    {

        byte ** pool_array;        ///< Puntero al primer segmento continuo de memoria de la memory pool.
        byte *  pool_end;          ///< Puntero al byte destrás del último byte de la memory pool.
        byte *  pool_trail;        ///< Puntero al byte destrás del último byte reservado.
        size_t  pool_size;         ///< Tamaño total en bytes de la memory pool.
        size_t  pool_consumed;     ///< Total de bytes consumidos de la memory pool.
        size_t  pool_init_size;    ///< Tamaño del segmento incial de la memory pool.
        size_t  segment_size;      ///< Tamaño de los segmentos en los que se amplia la memory pool.
        size_t  segment_num;       ///< Numero de segmentos(grupos de memoria contiguos) totales de la memory pool.
        size_t  segment_index;     ///< Segmentos de la memory pool en uso.
        

    public:

        /** @param desired_pool_size Tamaño en bytes de la memory pool.
          */
        Memory_Pool (size_t desired_pool_size); 

       ~Memory_Pool ();

        /** @return Retorna true si la memory pool se construyó sin problemas.
          */ 
        bool is_ok () const
        {
            return pool_size > 0;                 
        }

        /** @return Retorna el tamaño en bytes que se reservó para la memory pool. 0 si hubo algún error.
          */ 
        size_t size () const
        {          
            return pool_size;            
        }

        /** @return Retorna el tamaño en bytes de los segmentos en los que se amplia la memory pool.
          */
        size_t size_segment () const
        {
            return segment_size;
        }

        /** @return Numero total de segmentos creados contando el inicial
          */
        size_t segment_count () const
        {
            return segment_num;
        }
        
        /** Sobrecarga del operador [] 
          * 
          * @param  iterator Posicion de la memory pool a la que quieres acceder
          * 
          * @return Byte en la posicion deseada
          */ 
        byte * operator[] (const size_t index) const;
        

        /** @return Retorna la cantidad de memoria que se ha reservado hasta el momento.
          */         
        size_t consumed () const
        {
            return pool_consumed;       
        }

        /** @return Retorna la cantidad de memoria en bytes disponible.
          */         
        size_t available () const
        {
            return pool_size - pool_consumed;
        }

        /** Cambia el tamaño por defecto de los segmentos en los que se amplia la memory pool.
          * Si ya se creo algun segmento no te deja modificarlo.
          * 
          * @param  size Tamaño en bytes deseado para los nuevos segmentos
          * 
          * @return True si se cambio el tamaño, false en caso de que ya se haya generado algun segmento
          *     en cuyo caso el tamaño no se cambia.
          */
        bool set_segment_size (size_t size)
        {
            return segment_num <= 1 ? (segment_size = size, true) : false;
        }

        /** Este método muestra cómo se puede reservar memoria de la memory pool de un modo trivial.
          * No está teniendo en cuenta la alineación de cada bloque reservado ni otros detalles.
          * Esta versión sin alineación solo debería usarse si el tamaño reservado permite una 
          * alineación automática (8, 16, etc.) en cuyo caso se consigue una pequeña ventaja.
          * Si se utilizo set_segment_size () para indicar el tamaño de los nuevos segmentos, 
          * la memory pool aumentara si es necesario.
          * 
          * @param  amount Cantidad de bytes que se desea reservar. Con el valor cero se retorna una
          *     dirección de memoria válida.
          * @return Retorna un puntero al inicio del bloque reservado, un puntero al primer elemento del nuevo
          *     bloque de memoria reservado o nullptr en caso de no reservar nuevos bloques de memoria.
          */
        void * allocate (size_t amount);
        
        /// No entiendo el ALIGMENT de donde sale, (¿que numero meto ahí?)
        /** Este método muestra cómo se puede reservar memoria de la memory pool manteniendo la
          * alineación requerida.
          * La alineación es un parámetro de plantilla porque se requiere un valor conocido en tiempo
          * de compilación. Queda cierto margen para la optimización a costa de la claridad.
          * @param  amount Cantidad de bytes que se desea reservar. Con el valor cero se retorna una
          *     dirección de memoria válida.
          * @return Retorna un puntero al inicio del bloque reservado o nullptr si no hay suficiente
          *     memoria disponible.
          */
        template< size_t  ALIGNMENT >
        void * allocate (size_t amount)
        {
            static_assert(ALIGNMENT > 0);

            ptrdiff_t remainder = (pool_trail - *pool_array) % ALIGNMENT;
            size_t    padding   =  remainder  ? ALIGNMENT - remainder : 0;

            return (pool_trail +=  padding + amount) > pool_end
                ?  (pool_trail -=  padding + amount, nullptr)
                :   pool_trail - amount;
        }

        void * allocate (size_t amount, void * position);

        void free(size_t amount);

    private:            

        /** Funcion encargada de generar un nuevo segmento del memory pool
          */
        byte * new_segment();        

    };

}
