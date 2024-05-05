/**
 * @file fsm.c
 * @brief Library to create Finite State Machines using composition.
 *
 * This library is expected to be used using composition
 * @author Teachers from the Departamento de Ingeniería Electrónica. Original authors: José M. Moya and Pedro J. Malagón. Latest contributor: Román Cárdenas.
 * @date 2023-09-20
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#define FSM_MAX_TRANSITIONS 128

// GCOVR_EXCL_START
void* __attribute__((weak)) fsm_malloc(size_t s)
{
    return malloc(s);
}

void __attribute__((weak)) fsm_free(void* p)
{
    free(p);
}
// GCOVR_EXCL_STOP

/* Other includes */
#include "fsm.h"

static bool fsm_check_transitions(fsm_trans_t *p_tt)
{
    if (p_tt == NULL)
    {
        return false;
    }
    if ((p_tt->orig_state == -1) ||/*(p_tt->in == NULL) ||*/ (p_tt->dest_state == -1))
    {
        return false;
    }
    return true;
}

static void fsm_init_no_check(fsm_t *p_fsm, fsm_trans_t *p_tt)
{
    p_fsm->p_tt = p_tt;
    p_fsm->current_state = p_tt->orig_state;
}

fsm_t *fsm_new(fsm_trans_t *p_tt)
{
    if (!fsm_check_transitions(p_tt)) {
        return NULL;
    }
    fsm_t *p_fsm = (fsm_t *)fsm_malloc(sizeof(fsm_t));
    if (p_fsm != NULL)
    {
        fsm_init_no_check(p_fsm, p_tt);
    }
    return p_fsm;
}
// llamar a fsm_free solo si el puntero pasado no es NULL
void fsm_destroy(fsm_t *p_fsm)
{
    if (p_fsm != NULL)     fsm_free(p_fsm);
}

// ahora debe devolver un int con el numero de transciones validas como maximo 128 incluido el 0
// 
int fsm_init(fsm_t *p_fsm, fsm_trans_t *p_tt)
{
    int num_tran_val=0;

    if (p_fsm == NULL) {
        return 0;
    }

    if (!fsm_check_transitions(p_tt)) {
        return 0;
    }
    fsm_init_no_check(p_fsm, p_tt);

    //Calcula el numero de transiciones validas 
    while(fsm_check_transitions(p_tt+num_tran_val)){
        num_tran_val=num_tran_val+1;
    }

    if(num_tran_val> FSM_MAX_TRANSITIONS) num_tran_val=0;

    return num_tran_val;
}

int fsm_get_state(fsm_t *p_fsm)
{
    return p_fsm->current_state;
}

// GCOVR_EXCL_START
void fsm_set_state(fsm_t *p_fsm, int state)
{
    p_fsm->current_state = state;
}
// GCOVR_EXCL_STOP


//Añadir valor de entorno int a fsm_fire:
//-1: si no hay ninguna transicion para el estado actual
//0: si hay transiciones para el estado actual pero la funcion de guarda devuelve false
//1: si hay al menos una transicion para el estado actual con la funcion de gurada true

int fsm_fire(fsm_t *p_fsm)
{
    fsm_trans_t *p_t;
    int8_t val_return = -1;  // Start without valid transition

    for (p_t = p_fsm->p_tt; p_t->orig_state >= 0; ++p_t)
    {
        if ((p_fsm->current_state == p_t->orig_state)){
            val_return = 0;
            if ((p_t->in == NULL) || ((p_t->in != NULL) && (p_t->in(p_fsm)))) {
                p_fsm->current_state = p_t->dest_state;
                if (p_t->out)
                {
                    p_t->out(p_fsm);
                }
                return 1;
            }
        }
    }
    return val_return;
}