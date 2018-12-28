#ifndef __MODULE_H__
#define __MODULE_H__

class Module
{
  public:

  Module(){}  
  
  virtual ~Module(){}
    
  virtual bool stop() = 0;
  virtual bool start() = 0;
  virtual bool execute() = 0;
};


#endif
