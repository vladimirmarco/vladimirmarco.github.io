#include "unity.h"

#define TEST_CASE(...)
#define TEST_RANGE(...)

#include "fsm.h"
#include "mock_test_fsm.h"

#include <stdlib.h>

/**
 * @file test_fsm_legacy.c
 * @author 
 * @author 
 * @brief Tests for existing fsm module
 * @version 0.1
 * @date 2024-04-09
 * 
 */
/**
 * @brief Stub or Callback for fsm_malloc that calls real malloc. 
 * 
 * @param[in] s Amount of bytes to allocate
 * @param[in] n Amount of calls to this function
 * 
 * @return pointer to allocated memory if success; NULL if fails
 * 
 */
static void* cb_malloc(size_t s, int n) {
    return malloc(s);
}

/**
 * @brief Stub or Callback for fsm_free that calls real free. 
 * 
 * @param[in] p Pointer to allocated memory to free
 * @param[in] n Amount of calls to this function
 * 
 */
static void cb_free(void* p, int n) {
    return free(p);
}

void setUp(void)
{
}

void tearDown(void)
{
}

/**
 * @brief Comprueba que la funcion de fsm_new devuelve NULL 
 *        y no llama a fsm_malloc si la tabla de transiciones es NULL 
 */
void test_fsm_new_nullWhenNullTransition(void)
{
    fsm_t *f = (fsm_t*)1;

    f = fsm_new(NULL);  // tabla de transiciones es NULL

    TEST_ASSERT_EQUAL (NULL, f);
}

/**
 * @brief Comprueba que la función de inicialización devuelve cero si el puntero a la maquina de estado es NULL 
 *
 */
void test_fsm_init_ZeroWhenNullFsm(void)
{
    fsm_trans_t tt[] = {{ 0, is_true, 1, do_nothing},
                        {-1, NULL, -1, NULL}};
    int res=1;

    res=fsm_init(NULL,tt);     // Maquina de estados NULL

    TEST_ASSERT_EQUAL (0,res);

}

/**
 * @brief Función de inicializacion devuelve cero si la tabla de transiciones es nula
 * 
 */
void test_fsm_init_ZeroWhenNullTransitions(void)
{
    fsm_t *f = (fsm_t*)1;

    int res=0;

    res=fsm_init(f,NULL);     //Tabla de transiciones es NULL

    TEST_ASSERT_EQUAL (0,res);

}


/**
 * @brief Comprueba el numero de transiciones validas de la tabla de transiciones 
 * 
 */
void test_fsm_init_returnNumTransitionsValids(void) //New function
{
    fsm_trans_t tt[] = {            //hay 4 transciones 
        {0, is_true, 1, NULL},
        {1, is_true, 2, NULL},   
        {2, is_true, 3, NULL},   
        {3, is_true, 0, NULL},   
        {-1, NULL, -1, NULL}
    };

    fsm_t f ;

    int res = fsm_init(&f,tt);     

    TEST_ASSERT_EQUAL (4,res);

}

/**
 * @brief Comprueba que se deveulve 0 con mas de 128 transciones validas
 * 
 */
void test_fsm_init_returnZeroWith128tt(void)    //New function
{
    fsm_trans_t tt[130];

    for(int i=0; i<130;i++){
        tt[i].orig_state = 0; 
        tt[i].in = NULL; 
        tt[i].dest_state = 1; 
        tt[i].out = NULL; 
        if(i>=129){
            tt[i].orig_state = -1; 
            tt[i].in = NULL; 
            tt[i].dest_state = -1; 
            tt[i].out = NULL;  
        }
    }

    fsm_t f ;

    int res=0;

    res=fsm_init(&f,tt);     

    TEST_ASSERT_EQUAL (0,res);

}

/**
* @brief La máquina de estados devuelve NULL 
*        y no llama a fsm_malloc si el estado de origen 
*        de la primera transición es -1 (fin de la tabla)
*/
void test_fsm_nullWhenFirstOrigStateIsMinusOne (void) {
    fsm_trans_t tt[] = {{-1, is_true, 1, do_nothing}};
    fsm_t *f = (fsm_t*)1;
    f = fsm_new(tt);

    TEST_ASSERT_EQUAL (NULL,f);

}

/**
 * @brief La máquina de estados devuelve NULL y no llama a fsm_malloc si el estado de destino de la primera transición es -1 (fin de la tabla)
 * 
 */
void test_fsm_nullWhenFirstDstStateIsMinusOne (void) {
    fsm_trans_t tt[] = {{0, NULL, -1, NULL}};

    fsm_t *f = (fsm_t*)1;
    f = fsm_new(tt);

    TEST_ASSERT_EQUAL (NULL,f );

}



/**
 * @brief La máquina de estados devuelve NULL y no llama a fsm_malloc si la función de comprobación de la primera transición es NULL (fin de la tabla)
 *        En esta segunda version la guarda acepta NULL como true
 */
void test_fsm_nullWhenFirstCheckFunctionIsNull (void) {
    fsm_trans_t tt[] = {{0, NULL, 1, NULL},
                        {-1, NULL , -1, NULL}};

    fsm_malloc_ExpectAnyArgsAndReturn(0);
                     
    fsm_t *f = (fsm_t*)1;
    f = fsm_new(tt);

    TEST_ASSERT_EQUAL (NULL,f );

}


/**
 * @brief Devuelve puntero no NULL y llama a fsm_malloc (CALLBACK) al crear la maquina de estados con una transición válida con función de actualización (salida) NULL o no NULL.
 *        Hay que liberar la memoria al final llamando a free
 * 
 */
TEST_CASE(NULL)
TEST_CASE(do_nothing)
void test_fsm_new_nonNullWhenOneValidTransitionCondition(fsm_output_func_t out)
{
    fsm_trans_t tt[] = {
        {0, is_true,1, out},
        {-1, NULL, -1, NULL}
    };

    fsm_malloc_AddCallback(cb_malloc);
    fsm_malloc_ExpectAnyArgsAndReturn(0);

    fsm_t * F1= fsm_new(tt);

    TEST_ASSERT_NOT_EQUAL(NULL,F1);
    free(F1);

}


/**
 * @brief Estado inicial corresponde al estado de entrada de la primera transición de la lista al crear una maquiina de estados y es valido. 
 * 
 */
void test_fsm_new_fsmGetStateReturnsOrigStateOfFirstTransitionAfterInit(void)
{
    fsm_trans_t tt[] = {
        {0, is_true, 1, do_nothing},
        {-1, NULL, -1, NULL}
    };

    fsm_t*  f ;
   is_true_IgnoreAndReturn(false);

    fsm_malloc_Stub(cb_malloc); //equivalente al ignore con callback 

    f= fsm_new(tt);       //con el new
    
    //ignore me da igual si la llamas o no 
    TEST_ASSERT_EQUAL_INT (0,fsm_get_state(f));
    free(f); 

}

/**
 * @brief La maquina de estado no transiciona si la funcion devuelve 0
 * 
 */
void test_fsm_fire_isTrueReturnsFalseMeansDoNothingIsNotCalledAndStateKeepsTheSame(void)
{
    fsm_trans_t tt[] = {
        {0, is_true, 1, do_nothing},
        {-1, NULL, -1, NULL}
    };

    is_true_ExpectAnyArgsAndReturn(false);
    //do_nothing_Ignore();
    fsm_t f;
    fsm_init(&f, tt);
    fsm_fire(&f);
    TEST_ASSERT_EQUAL_INT (0,fsm_get_state(&f) );

}

/**
 * @brief Comprueba que el puntero pasado a fsm_fire es pasado a la función de guarda cuando se comprueba una transición
 * 
 */
void test_fsm_fire_checkFunctionCalledWithFsmPointerFromFsmFire(void)
{

    fsm_trans_t tt[] = {
        {0, is_true , 1, NULL},
        {1, is_true, 0, NULL},
        {-1, NULL, -1, NULL}
    };

    fsm_t f;
    int res;

    is_true_ExpectAndReturn(&f,true);

    fsm_init(&f, tt);

    fsm_fire(&f);

    res = fsm_get_state(&f);        // comprobar la transiscion 
    TEST_ASSERT_EQUAL_INT (1,res );
  
}

/** 
 * @brief Comprueba que el fsm_fire funciona y tiene el estado correcto cuando la transición devuelve true (cambia) y cuando devuelve false (mantiene)
 * 
 */
TEST_CASE(false, 0)
TEST_CASE(true, 1)
void test_fsm_fire_checkFunctionIsCalledAndResultIsImportantForTransition(bool returnValue, int expectedState)
{
 
    fsm_trans_t tt[] = {
        {0, is_true, 1, NULL},
        {-1, NULL, -1, NULL}
    };
    fsm_t f;

    fsm_init(&f, tt);

    is_true_ExpectAnyArgsAndReturn(returnValue);

    fsm_fire(&f);
    TEST_ASSERT_EQUAL(expectedState, f.current_state);

}


/**
 * @brief La maquina de estado devuelve -1 si no hay ninguna transicion para el estado actual
 * 
 */
void test_fsm_fire_isTrueReturnsNoneTrasition(void)
{
    fsm_trans_t tt[] = {
        {-1, NULL, -1, NULL}
    };

   
    fsm_t f;
    int res;
    fsm_init(&f, tt);
    res = fsm_fire(&f);
    TEST_ASSERT_EQUAL_INT (-1,res );

}


/**
 * @brief La maquina de estado devuelve 0 si la funcion de guarada devuelve false de lo contrario true y devuelve 1
 * 
 */
TEST_CASE(false,0)
TEST_CASE(true,1)
void test_fsm_fire_isTrueReturnsZeroOrOne(bool returnValue, int expectedState  )
{
    fsm_trans_t tt[] = {
        {0, is_true, 1, NULL},
        {-1, NULL, -1, NULL}
    };

    is_true_ExpectAnyArgsAndReturn(returnValue);

    fsm_t f;
    int res;
    fsm_init(&f, tt);
    res = fsm_fire(&f);
    TEST_ASSERT_EQUAL_INT (expectedState,res );

}


/**
 * @brief  Este test prueba que la funcion de guarda es true con un NULL
 *       
 */
void test_fsm_fireNULLIsTrue (void) {
    fsm_trans_t tt[] = {{0, NULL, 1, NULL},
                        {-1, NULL , -1, NULL}};
                     
    fsm_t f;
    fsm_init(&f,tt);
    fsm_fire(&f);

    TEST_ASSERT_EQUAL (1,f.current_state);

}

/**
 * @brief La creación de una máquina de estados devuelve NULL si la reserva de memoria falla (Mock, no Stub)
 * 
 */
void test_fsm_new_nullWhenFsmMallocReturnsNull(void)
{
    fsm_trans_t tt[] = {
        {0, is_true, 1, NULL},
        {1, is_true, 0, NULL},
        {-1, NULL, -1, NULL}
    };

    fsm_malloc_ExpectAnyArgsAndReturn(NULL);
    
    fsm_t* FSM = fsm_new(tt);
    TEST_ASSERT_NULL(FSM);
}

/**
 * @brief Llamar a fsm_destroy provoca una llamada a fsm_free (Mock, no Stub)
 * 
 */
void test_fsm_destroy_callsFsmFree(void)
{

    fsm_t *f = (fsm_t *)0x1;
    fsm_free_Expect(f);
    fsm_destroy(f);

}

/**
 * @brief Llamar a fsm_destroy NO provoca una llamada a fsm_free si f es NULL (Mock, no Stub)
 * 
 */
void test_fsm_destroy_doesntCallsFsmFreeIfNull(void)
{
    fsm_t *f = NULL;
    
    fsm_destroy(f);
}

/**
 * @brief Comprueba que solo se llame a la función de guarda que toca según el estado actual
 * 
 */

void test_fsm_fire_callsFirstIsTrueFromState0AndThenIsTrue2FromState1(void)
{
    fsm_trans_t tt[] = {
        {0, is_true, 1, do_nothing},
        {1, is_true2, 0, NULL},   //Descomentar cuando se haya declarado una nueva función para mock is_true2

        {-1, NULL, -1, NULL}
    };

    is_true_ExpectAnyArgsAndReturn(true);
    do_nothing_Ignore();
    is_true2_ExpectAnyArgsAndReturn(true);

    fsm_t f;
    int res;
    fsm_init(&f, tt);
    fsm_fire(&f);
    fsm_fire(&f);

    res = fsm_get_state(&f);

    TEST_ASSERT_EQUAL_INT (0,res );

}

/**
 * @brief Comprueba que se pueden crear dos instancias de máquinas de estados simultánteas y son punteros distintos
 * 
 */
void test_fsm_new_calledTwiceWithSameValidDataCreatesDifferentInstancePointer(void)
{
    fsm_trans_t tt[] = {
        {0, is_true, 1, NULL},
        {-1, NULL, -1, NULL}
    };

    fsm_malloc_AddCallback(cb_malloc);
    fsm_malloc_ExpectAnyArgsAndReturn(0);
    fsm_malloc_AddCallback(cb_malloc);
    fsm_malloc_ExpectAnyArgsAndReturn(0);

    fsm_t* F1 = fsm_new(tt);
    fsm_t* F2 = fsm_new(tt);

    TEST_ASSERT_NOT_EQUAL(F1,F2);
    free(F1);
    free(F2);
}