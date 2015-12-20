
  String getMoistureSensorsState()
  {
    String str;

    for (int ii = 0; ii< NB_VALVES; ++ii)
    {
      str+= moistureSensorState[ii];
      str += " "; 
    }    
    return str;
  }

