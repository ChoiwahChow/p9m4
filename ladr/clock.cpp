#include "clock.h"
#include "memory.h"
#include "mystring.h"
#include "ladrvglobais.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <iomanip>



GlobalClocks::GlobalClocks() {
    Clocks_enabled=false;
    Clock_starts=0;
    Wall_start=0;
    Clock_gets=0;
    Clock_frees=0;
    Time_passed=0;
}


GlobalClocks::~GlobalClocks() {
}


myClock::myClock() {

}



myClock::~myClock() {

    
}



void myClock::operator = (ClockStructure cs) {
    this->set_ClockStructure(cs);
}



void myClock::init_clock(string n) {
    clock_init(n);
}

void myClock::init_clock() {
    clock_init();
}


void myClock::zap_clock() {
    free_clock();    
}


void myClock::clock_init() {
 data=get_clock();
 data->level=0;
 data->accu_msec=0;
 *(data->name)=myString::null_string();
}
                                    
void myClock::clock_init(string n) {
    data=get_clock();
    *(data->name)=n;
    data->level=0;
    data->accu_msec=0;
}


void myClock::clock_start(ClockStructure p) {
  if (LADR_GLOBAL_CLOCK.Clocks_enabled) {
    p->level++;
    if (p->level == 1) {
      CPU_TIME(p->curr_msec);
      LADR_GLOBAL_CLOCK.Clock_starts++;
    }
  }
}



void myClock::clock_stop(ClockStructure p) {
  if (LADR_GLOBAL_CLOCK.Clocks_enabled) {
    if (p->level <= 0)
      cerr<<"WARNING, clock_stop: clock "<<*(p->name)<<" not running"<<endl;
    else {
      p->level--;
      if (p->level == 0) {
        unsigned msec;
        CPU_TIME(msec);
        p->accu_msec += msec - p->curr_msec;
      }
    }
  }
} 

bool myClock::clock_running(ClockStructure p) {
  return p->level > 0;    
}


void myClock::clock_start() {
    if(LADR_GLOBAL_CLOCK.Clocks_enabled) {
        data->level++;
        if(data->level==1) {
            CPU_TIME(data->curr_msec);
            LADR_GLOBAL_CLOCK.Clock_starts++;
        }
    }
}


void myClock::clock_stop() {
    if(LADR_GLOBAL_CLOCK.Clocks_enabled) {
        if(data->level<=0)  cerr << "Warning, clock_stop: clock "<<data->name<<" not running"<<endl;
        else {
                data->level--;
                if(data->level==0) {
                    unsigned msec;
                    CPU_TIME(msec);
                    data->accu_msec+=msec - data->curr_msec;
                }
        }
    }
}




unsigned myClock::clock_milliseconds(void) {
    if(!data) return 0;
    int i=data->accu_msec;
    if(data->level==0) return i;
    int msec;
    CPU_TIME(msec);
    return i + (msec - data->curr_msec);
}



double myClock::clock_seconds(void) {
    if(!data) return 0.0;
    int i=data->accu_msec;
    if(data->level==0) return (i/1000.0);
    int msec;
    CPU_TIME(msec);
    return (i+(msec-data->curr_msec))/1000.0;
}






bool myClock::clock_running(void) {
        return data->level>0;
}



void myClock::clock_reset(void) {
    if(data) {
                data->level=0;
                data->accu_msec=0;
    }
}




ClockStructure myClock::get_clock(void) {
        data= (ClockStructure) Memory::memCNew(sizeof(clockStructure));
        data->name=new string();
        LADR_GLOBAL_CLOCK.Clock_gets++;
        return data;
}

void myClock::free_clock(void) {
        if (data) {
            if (data->name) delete data->name;
            Memory::memFree (data, sizeof(clockStructure));
            LADR_GLOBAL_CLOCK.Clock_frees++;
        }
}

string myClock::get_date() {
    time_t i=time(NULL);
    return asctime(localtime(&i));
}


unsigned myClock::user_time(void) {
struct rusage r;
unsigned sec, usec;
    getrusage(RUSAGE_SELF, &r);
    sec=r.ru_utime.tv_sec;
    usec=r.ru_utime.tv_usec;
    return (sec*1000) + (usec/1000);
}

double myClock::user_seconds() {
    struct rusage r;
    unsigned sec, usec;
    getrusage(RUSAGE_SELF, &r);
    sec=    r.ru_utime.tv_sec;
    usec=   r.ru_utime.tv_usec;
    return (sec + (usec / 1000000.0));
}



unsigned myClock::system_time(void) {
    struct rusage r;
    unsigned sec, usec;
    getrusage(RUSAGE_SELF, &r);
    sec=r.ru_stime.tv_sec;
    usec=r.ru_stime.tv_usec;
    return (sec*1000) + (usec/1000);
}

double myClock::system_seconds(void) {
    struct rusage r;
    unsigned sec, usec;
    getrusage(RUSAGE_SELF, &r);
    sec=r.ru_stime.tv_sec;
    usec=r.ru_stime.tv_usec;
    return (sec + (usec/1000000.0));
    
    
}

unsigned myClock::absolute_wallclock(void) {
    time_t t=time( (time_t *) NULL );
    return (unsigned) t;
}


void myClock::init_wallclock(void) {
    LADR_GLOBAL_CLOCK.Wall_start= absolute_wallclock();
}


unsigned myClock::wallclock(void) {
        return absolute_wallclock()-LADR_GLOBAL_CLOCK.Wall_start;
}


void myClock::disable_clocks(void) {
    LADR_GLOBAL_CLOCK.Clocks_enabled=false;
}

void myClock::enable_clocks(void){
    LADR_GLOBAL_CLOCK.Clocks_enabled=true;
}

bool myClock::clocks_enabled(void){
    return LADR_GLOBAL_CLOCK.Clocks_enabled;
}

string myClock::get_name() {
return *data->name;
}

ClockStructure myClock::get_ClockStructure() {
    return data;
}

void myClock::set_ClockStructure(ClockStructure cs) {
    data=cs;
}


void myClock::fprint_clock(ostream &fp) {
 if (data != NULL)
     fp<<"clock "<<data->name<<" : "<< clock_seconds() <<"seconds."<<endl; 
}  


void myClock::time_passed(string from, string to,bool e) {
  char c;
  double t=user_seconds();
  cout << "Time passed from "<< from<< " to "<< to << " is "<<setprecision(5)<<t-LADR_GLOBAL_CLOCK.Time_passed<<" seconds"<< endl;
  LADR_GLOBAL_CLOCK.Time_passed=t;
  if (e) exit(0);
  else {
        c=getchar();
  }
}

void myClock::reset_time_passed(void) {
    LADR_GLOBAL_CLOCK.Time_passed=0;
}
