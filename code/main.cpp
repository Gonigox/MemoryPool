
// Este código es de dominio público.
// angel.rodriguez@esne.edu
// 2020.10
//
// Este ejemplo realiza un benchmark sintético entre usar una memory pool incremental o usar el
// operador new. Para que la comparación sea válida se debe compilar y ejecutar el programa en
// configuración Release. También es recomendable usar tipos POD (se puede configurar cambiando
// TYPE más abajo), ya que es el uso típico de una memory pool incremental.
// Es habitual que la memory pool sea entre 20 y 40 veces más rápida que el operador new, aunque

// el resultado depende de la versión del sistema operativo, del modelo de procesador y del tipo
// de dato usado, entre otros factores.

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include "Object_Pool.hpp"
#include "SimpleMemoryPool/Object_Pool.hpp"

using namespace std;
using namespace std::chrono;

struct  Point4 { float x, y, z, w; };           // Tipos POD para hacer pruebas
struct  Odd    { char  c[3];       };

class MyString
{
    char z, w;
public:
    MyString() = default;

    MyString (int a)
    {
        z = char(a);
        w = char(a);
    }

    virtual int get_length(){return z;}
};

class Test
{
    int         i;
    std::string s; ///< La clase string debe usar en su contructor un new o algo asi porque simplemente con existir hace que el rendimiento baje muchisimo
    size_t      x;
    Point4      p4;
    Odd         od;

public:

    Test() : p4(), od()
    { 
        //cout << "Test constructor\n"; 
        i = 0;
        s = "hola";
        x = false;
    }

    Test(int a, std::string string) : p4(), od()
    {
        i = a;
        //s = string;
        x = LLONG_MAX;
    }

    ~Test() = default;//cout << "Test destructor \n"; }

};

typedef Point4 TYPE;                            // Tipo usado para la instanciación

int main ()
{
    constexpr size_t iterations         = 1000000;
    constexpr size_t half_iterations    = iterations >> 1;
    constexpr size_t quarter_iterations = iterations >> 2;

    high_resolution_clock::time_point start, finish;

    //
    // HEAP ALLOCATION
    //

    cout << "Running heap allocation...";
    vector< TYPE * > heap_objects(iterations);

    start = high_resolution_clock::now ();
    {
        for (size_t count = 0; count < iterations; ++count)
        {
            heap_objects[count] = new TYPE;
        }

        for (size_t count = 0; count < iterations; ++count)
        {
            delete heap_objects[count];
        }
    }
    finish = high_resolution_clock::now ();

    double heap_elapsed = duration_cast< duration< double > >(finish - start).count ();

    cout << "\nHeap allocation required " << heap_elapsed << " seconds.";

    //
    //  MemoryPool POOLING
    //
    
    cout << "\nRunning pool allocation using MemoryPool...";
    pooling::Memory_Pool memory_pool(sizeof(TYPE) * iterations);   

    start  = high_resolution_clock::now ();
    { 
        //memory_pool.set_segment_size(sizeof(TYPE) * quarter_iterations);

        for (size_t count = 0; count < iterations; ++count)
        {
            new (memory_pool.allocate(sizeof(TYPE))) TYPE;
        }

        for (int count = iterations - 1; count >= 0; --count)
        {
            reinterpret_cast< TYPE * > (memory_pool[sizeof(TYPE) * count])->~TYPE();
            memory_pool.free(sizeof(TYPE));
        }
    }
    finish = high_resolution_clock::now ();

    double pool_operator_elapsed = duration_cast< duration< double > >(finish - start).count ();

    cout << "\nMemoryPool required      " << pool_operator_elapsed << " seconds.";

    // 
    //  OBJECT POOLING
    //

    cout << "\nRunning Object allocation...";
    pooling::Object_Pool < TYPE > object_pool(iterations);

    start  = high_resolution_clock::now ();
    {
        for (size_t count = 0; count < iterations; ++count)
        {    
            object_pool.allocate();
        }

        for (size_t count = 0; count < iterations; ++count)
        {
            object_pool.free(&object_pool[count]);
        }
    }
    finish = high_resolution_clock::now ();

    double object_pool_elapsed = duration_cast< duration< double > >(finish - start).count ();

    cout << "\nObjectPool time required " << object_pool_elapsed << " seconds.";



    cout << "\n\nMemoryPool     performance vs Heap allocator : ";
    cout << setprecision (3) << (heap_elapsed / pool_operator_elapsed) << " times.";
    cout << "\nObject Pooling performance vs Heap allocator : ";
    cout << setprecision (3) << (heap_elapsed / object_pool_elapsed) << " times.";
    cout << "\n\nMemoryPool     performance vs Object Pooling : ";
    cout << setprecision (3) << (object_pool_elapsed /pool_operator_elapsed) << " times.";    
    cout << endl;
    cout << endl;

    return 0;
}
