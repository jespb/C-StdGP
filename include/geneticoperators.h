#ifndef _GENETICOPERATORS_H
#define _GENETICOPERATORS_H

struct individual_t** getElite(int p_size, struct individual_t** population, int e_size);

struct individual_t** getOffspring(int p_size, struct individual_t** population, int t_size);

struct individual_t* tournament(int p_size, struct individual_t** population, int t_size);

struct individual_t** STXO(int p_size, struct individual_t** population, int t_size);
struct individual_t** STMUT(int p_size, struct individual_t** population, int t_size);

struct individual_t** discardDeep(struct individual_t** population, int max_depth);


#endif


