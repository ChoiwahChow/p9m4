#ifndef TP_DIOPH_H
#define TP_DIOPH_H


#define MAX_COEF   250  /* total # of coef. from both sides */
#define MAX_BASIS  100  /* must be <= MAX_VARS, because rows are indexed */
#define MAX_COMBOS 200  /* for superset-restricted AC unif. */

#define DOWN    1
#define OVER    2
#define BACKUP  3
#define SUCCESS 4
#define FAILURE 5


#define ADD_TO_SUM(sum,basis,i,len) \
{int j,*p; for (j=0,p=basis[i];j<len;j++) sum[j] += p[j];}

#define SUBTRACT_FROM_SUM(sum,basis,i,len) \
{int j,*p; for (j=0,p=basis[i];j<len;j++) sum[j] -= p[j];}

class DiophantineEquation {

private:
            static int gcd(int, int);
            static int lcm(int, int);
            static int less_vec(int *, int *, int);
            static int var_check_1(int *, int *, int , int);
            static int var_check_2(int *, int *,int , int);
            static int add_solution(int *xy, int length, int *num_basis, int(*basis)[MAX_COEF]);
            static int a_in_bounds( int *ab, int *xy, int *max_y, int (*d)[MAX_COEF],int (*e)[MAX_COEF], int m, int n, int xypos, int max_a,int max_b, int suma, int *constraints);
            static int b_in_bounds(int *xy, int *max_y, int *constraints, int xypos, int suma, int sumb, int m, int n);
            static int superset_degree(int *a, int *b, int n);


public:
            static int dio(int ab[MAX_COEF],int m,int n,int constraints[MAX_COEF],    int basis[MAX_BASIS][MAX_COEF],int *num_basis);
            static int next_combo_a(int length,int basis[MAX_BASIS][MAX_COEF],int num_basis,int constraints[MAX_COEF],int combo[MAX_BASIS],int sum[MAX_COEF],int start_flag);
            static int next_combo_b(int length,int basis[MAX_BASIS][MAX_COEF],int num_basis,int constraints[MAX_COEF],int combo[MAX_BASIS],int sum[MAX_COEF],int start_flag);
            static int next_combo_c(int length,int basis[MAX_BASIS][MAX_COEF],int num_basis,int constraints[MAX_COEF],int combo[MAX_BASIS],int sum[MAX_COEF],int start_flag);
            static int next_combo_ss(int length,int basis[MAX_BASIS][MAX_COEF],int num_basis,int constraints[MAX_COEF],int combo[MAX_BASIS], int sum[MAX_COEF],int start_flag,int combos[MAX_COMBOS][MAX_BASIS],int *np,int ss_parm);
            static void p_ac_basis(int basis[MAX_BASIS][MAX_COEF],int num_basis,int m,int n);
            #ifdef SOLO
            static int all_combos( int m, int n, int basis[MAX_BASIS][MAX_COEF],int num_basis,int constraints[MAX_COEF], int (*proc) (/* ??? */));
            #endif
};
            
#endif            
            
