#ifndef _CLOCK_H
#define _CLOCK_H



#include <string>
#include <iostream>

#include <sys/time.h>
#include <sys/resource.h>

#define MAX_NAME_CLOCK 10

using namespace std;

//classe para contermos toda a informação dos vários clocks existentes
class GlobalClocks {
                        private:
                                bool         Clocks_enabled;
                                unsigned     Clock_starts;
                                unsigned     Wall_start;
                                unsigned     Clock_gets, Clock_frees;
                                double Time_passed;
                                
                        public:
                                GlobalClocks();
                                ~GlobalClocks();
                            
                            friend class myClock;
                            friend class LadrVglobais;
};



#define CPU_TIME(msec) { struct rusage r; getrusage(RUSAGE_SELF, &r); msec=r.ru_utime.tv_sec * 1000 + r.ru_utime.tv_usec /1000;}



//estrutura para guardar a informação de um clock
struct clockStructure {
                                        string *name;
                                        unsigned accu_msec;
                                        unsigned curr_msec;
                                        int level;         
                                        
};


typedef clockStructure * ClockStructure;


//objecto que contém um pointer para uma estrutura clock e tem as funções para manipular um clock
class myClock {

                            private :  
                                        ClockStructure data; //estrutura que contém os dados do clock
                                        ClockStructure get_clock(void);
                                        void free_clock(void); //liberta a estrutura deste clock
                                        void clock_init(string); //cria uma estrutura e afecta o nome
                                        void clock_init(); //cria uma estrutura
                                     
                                    
                           
                            public: 
                                    
                                    ~myClock(); //não faz nada
                                    myClock(); //não faz nada
                                    void init_clock(string); //chama o clock_init(string) e passa a string de nome
                                    void init_clock(); //chama o clock_init
                                    void zap_clock(); //chama o free_clock
                                    static string get_date(); //returns system date
                                    void clock_start(void);
                                    void clock_stop(void);
                                    unsigned clock_milliseconds(void); 
                                    double clock_seconds(void);
                                    bool clock_running(void);
                                    void clock_reset(void);
                                    static unsigned user_time(void);
                                    static double user_seconds(void);
                                    static unsigned system_time(void);
                                    static double system_seconds(void);
                                    static unsigned absolute_wallclock(void);
                                    static void init_wallclock(void);
                                    static unsigned wallclock(void);
                                    static void disable_clocks(void);
                                    static void enable_clocks(void);
                                    static bool clocks_enabled(void);
                                    string get_name();
                                    void clock_start(ClockStructure);
                                    void clock_stop(ClockStructure);
                                    bool clock_running(ClockStructure);
                                    
                                    
                                    ClockStructure get_ClockStructure(); //devolve a estrutura de dados deste clock
                                    void set_ClockStructure(ClockStructure); //afecta a estrutura de dados deste clock
                                    void fprint_clock(ostream &);
                                    void operator=(ClockStructure);
                                    
                                    static void time_passed(string, string,bool);
                                    static void reset_time_passed(); 
                                    
};






#endif
