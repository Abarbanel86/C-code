/****************************************************************************/
/*                    Exercise: Caculator Test                              */
/*                    Date: 12\11\2018                                      */
/*                    Team Name: RD56                                       */
/*                    Programmer: Tom                                       */
/*                    Reviewer: Yoni                                        */
/****************************************************************************/

/*double (char *expression, int *status)*/

#include <stdio.h>   /* printf */
#include <stdlib.h> /* strtod */
#include <math.h>   /* Power */
#include "stack.h"  /* stack func decleration */
#include "caculator.h"

#define STACKSIZE 100
#define STACKELEMENTSIZE sizeof(double)

int flag = 0;
int next_step = 0;

enum next_step{NUM, OPER};
enum status {OK, STRINGERROR, MATHERROR, PRAMTESSESERROR};

typedef void (*act_func_t) (stack_t *opr_stk, stack_t *num_stk, int *status);

typedef struct oper
{
    int rank;
    act_func_t func;
}oper_t;


static char num[256] = {0};
static oper_t oper[256] = {0};
stack_t *opr_stk = NULL, *num_stk = NULL;

void CalcError (stack_t *opr_stk, stack_t *num_stk, int *status);

void CalcInsertParan (stack_t *opr_stk, stack_t *num_stk, int *status);
void CalcResolvePar (stack_t *opr_stk, stack_t *num_stk, int *status);

void CalcAdd (stack_t *opr_stk, stack_t *num_stk, int *status);
void CalcSub (stack_t *opr_stk, stack_t *num_stk, int *status);
void CalcMultiply (stack_t *opr_stk, stack_t *num_stk, int *status);
void CalcDevision (stack_t *opr_stk, stack_t *num_stk, int *status);
void CalcPower (stack_t *opr_stk, stack_t *num_stk, int *status);

void CalcInit ()
{
    int i = 0;

    for (i = '0'; i <= '9'; ++i)
    {
        num[i] = i;
    }
    num['.'] = 1;
    num['-'] = 1;
    num['+'] = 1;
    num['('] = 13;
    num['['] = 13;
    num['{'] = 13;

    for (i = 0; i < 256; ++i)
    {
        oper[i].func = CalcError;
        oper[i].rank = 0;
    }

    oper['('].rank = 11;
    oper['['].rank = 11;
    oper['{'].rank = 11;
    oper[')'].func = CalcResolvePar;
    oper[')'].rank = 11;
    oper['}'].func = CalcResolvePar;
    oper['}'].rank = 11;
    oper[']'].func = CalcResolvePar;
    oper[']'].rank = 11;
    oper['+'].func = CalcAdd;
    oper['+'].rank = 9;
    oper['-'].func = CalcSub;
    oper['-'].rank = 9;
    oper['*'].func = CalcMultiply;
    oper['*'].rank = 7;
    oper['/'].func = CalcDevision;
    oper['/'].rank = 7;
    oper['^'].func = CalcPower;
    oper['^'].rank = 10;
    oper['#'].rank = 99;

    flag = 1;
}

double Calculate (char *expression, int *status)
{
    double current_num = 0, ret = 0;
    char cur = '\0';
    char first_oper = '#';
    char last_opr = '\0';
    int last_opr_rank = oper['#'].rank;

    next_step = NUM;

    opr_stk = StackCreate (STACKSIZE, sizeof(char));
    num_stk = StackCreate (STACKSIZE, STACKELEMENTSIZE);

    StackPush(opr_stk, &first_oper);

    if (0 == flag)
    {
        CalcInit();
    }

    while (*expression && *status == OK)
    {
        if (*expression == ' ')
        {
            ++expression;
        }
        cur = *expression;

        if (next_step == NUM)
        {
            if (0 == num[(int)cur])
            {
              *status = STRINGERROR;
            }
            else if (13 == num[(int)cur])
            {
                StackPush(opr_stk, &cur);
                ++expression;

            }
            else
            {
                current_num = strtod(expression, &expression);
                StackPush(num_stk, &current_num);
                next_step = OPER;
            }
        }
        else/*(next_step == OPER)*/
        {
            last_opr = *(char *)StackPeek(opr_stk);
            last_opr_rank = oper[(int)last_opr].rank;

            if (0 == oper[(int)cur].rank)
            {
                *status = STRINGERROR;
            }
            else if (oper[(int)cur].rank < last_opr_rank)
            {
                StackPush(opr_stk, &cur);
                next_step = NUM;
            }
            else
            {
                if (oper[(int)cur].rank == 11)
                {
                    while (oper[(int)cur].rank > last_opr_rank)
                    {
                        oper[(int)last_opr].func (opr_stk, num_stk, status);
                        last_opr = *(char *)StackPeek(opr_stk);
                        last_opr_rank = oper[(int)last_opr].rank;
                    }
                    StackPush(opr_stk, &cur);
                    CalcResolvePar(opr_stk, num_stk, status);
                    next_step = OPER;
                }
                else
                {
                    while (oper[(int)cur].rank >= last_opr_rank)
                    {
                        oper[(int)last_opr].func (opr_stk, num_stk, status);
                        last_opr = *(char *)StackPeek(opr_stk);
                        last_opr_rank = oper[(int)last_opr].rank;
                    }

                    StackPush(opr_stk, &cur);
                    next_step = NUM;
                }
            }

            ++expression;
        }
    }

    last_opr = *(char *)StackPeek(opr_stk);
    if ('#' != last_opr)
    {
        while ('#' != last_opr && *status == OK)
        {
            oper[(int)last_opr].func (opr_stk, num_stk, status);
            last_opr = *(char *)StackPeek(opr_stk);
            last_opr_rank = oper[(int)last_opr].rank;
        }
    }

    ret = *(double *) StackPeek(num_stk);

    StackDestroy(num_stk);
    StackDestroy(opr_stk);

    return ret;
}

void CalcAdd (stack_t *opr_stk, stack_t *num_stk, int *status)
{
    double x = 0, y = 0, res = 0;

    if (StackSize(num_stk) < 2)
    {
        *status = STRINGERROR;
    }

    x = *(double *) StackPeek(num_stk);
    StackPop(num_stk);
    y = *(double *) StackPeek(num_stk);
    StackPop(num_stk);

    res = x + y;
    StackPush(num_stk, &res);
    StackPop(opr_stk);
}

void CalcSub (stack_t *opr_stk, stack_t *num_stk, int *status)
{
    double x = 0, y = 0, res = 0;

    if (StackSize(num_stk) < 2)
    {
        *status = STRINGERROR;
    }

    x = *(double *)StackPeek(num_stk);
    StackPop(num_stk);
    y = *(double *)StackPeek(num_stk);
    StackPop(num_stk);

    res =  y - x;

    StackPush(num_stk, &res);
    StackPop(opr_stk);
}

void CalcMultiply (stack_t *opr_stk, stack_t *num_stk, int *status)
{
    double x = 0, y = 0, res = 0;

    if (StackSize(num_stk) < 2)
    {
        *status = STRINGERROR;
    }

    x = *(double *)StackPeek(num_stk);
    StackPop(num_stk);
    y = *(double *)StackPeek(num_stk);
    StackPop(num_stk);

    res =  x * y;

    StackPush(num_stk, &res);
    StackPop(opr_stk);
}

void CalcDevision (stack_t *opr_stk, stack_t *num_stk, int *status)
{
    double x = 0, y = 0, res = 0;

    if (StackSize(num_stk) < 2)
    {
        *status = STRINGERROR;
    }

    x = *(double *)StackPeek(num_stk);
    StackPop(num_stk);
    y = *(double *)StackPeek(num_stk);
    StackPop(num_stk);

    if (0 != x)
    {
        res =  y / x;

        StackPush(num_stk, &res);
        StackPop(opr_stk);
    }

    *status = MATHERROR;
}

void CalcResolvePar(stack_t *opr_stk, stack_t *num_stk, int *status)
{
    char paren = '\0';

    paren = *(char *)StackPeek(opr_stk);
    StackPop(opr_stk);

    if (*(char *)StackPeek(opr_stk) == '(' && paren == ')')
    {
        StackPop(opr_stk);
    }
    else if (*(char *)StackPeek(opr_stk) == '[' && paren == ']')
    {
        StackPop(opr_stk);
    }
    else if (*(char *)StackPeek(opr_stk) == '{' && paren == '}')
    {
        StackPop(opr_stk);
    }
    else
    {
        *status = PRAMTESSESERROR;
    }

}

void CalcPower (stack_t *opr_stk, stack_t *num_stk, int *status)
{
    double x = 0, y = 0, res = 0;

    if (StackSize(num_stk) < 2)
    {
        *status = STRINGERROR;
    }

    x = *(double *)StackPeek(num_stk);
    StackPop(num_stk);
    y = *(double *)StackPeek(num_stk);
    StackPop(num_stk);

    res = pow(y, x);

    StackPush(num_stk, &res);
    StackPop(opr_stk);
}

void CalcError(stack_t *opr_stk, stack_t *num_stk, int *status)
{
    *status = STRINGERROR;

}
