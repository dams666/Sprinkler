// Calcul des statistiques de consommation d'eau une fois une vanne ouverte
// ATTENTION : on part du principe qu'une seule vanne est ouverte à la fois.
// 0 : l'eau coule
// 
  int calcFlowStats() 
  {
    unsigned long newTime = millis();

      // Disable the interrupt while calculating flow rate and sending the value to
      // the host
      detachInterrupt(flowSensorInterrupt);

      // Because this loop may not complete in exactly 1 second intervals we calculate
      // the number of milliseconds that have passed since the last execution and use
      // that to scale the output. We also apply the flowSensorCalibrationFactor to scale the output
      // based on the number of pulses per second per units of measure (litres/minute in
      // this case) coming from the sensor.

      //DEBUG_PRINTLN(flowPulseCount);
      
      float flowRate = ((1000.0f / (newTime - flowStatsOldTime)) * flowPulseCount) / flowSensorCalibrationFactor;
     
      // Divide the flow rate in litres/minute by 60 to determine how many litres have
      // passed through the sensor in this 1 second interval, then multiply by 1000 to
      // convert to millilitres.
      unsigned int flowMilliLitres = (flowRate / 60) * 1000;

      // Add the millilitres passed in this second to the cumulative total
      totalMilliLitres[numValveToInspect] += flowMilliLitres;

      unsigned int frac;

#ifdef WITH_SERIAL  
      // Print the flow rate for this second in litres / minute
      Serial.print("Flow rate: ");
      Serial.print(int(flowRate));  // Print the integer part of the variable
      Serial.print(".");             // Print the decimal point
      // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
      frac = (flowRate - int(flowRate)) * 10;
      Serial.print(frac, DEC) ;      // Print the fractional part of the variable
      Serial.print("L/min");
      // Print the number of litres flowed in this second
      Serial.print("  Current Liquid Flowing: ");             // Output separator
      Serial.print(flowMilliLitres);
      Serial.print("mL/Sec");

      // Print the cumulative total of litres flowed since starting
      Serial.print("  Output Liquid Quantity: ");             // Output separator
      Serial.print(totalMilliLitres[numValveToInspect]);
      Serial.println("mL"); 
      delay(40);
#endif
      
      // Enable the interrupt again now that we've finished sending output
    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    attachInterrupt(flowSensorInterrupt, flowIncPulseCounter, FALLING);

      //DEBUG_PRINT("diff:");
      //DEBUG_PRINTLN(diffMillilitres);

      // détection d'incohérences
      
      int diffMillilitres = totalMilliLitres[numValveToInspect] - lastTotalMilliLitres[numValveToInspect];

      // Reset the pulse counter so we can start incrementing again
      flowPulseCount = 0;
      lastTotalMilliLitres[numValveToInspect] = totalMilliLitres[numValveToInspect];
      
      if (diffMillilitres > 0)
      {
        if ( diffMillilitres > 1000 )          
          return WATER_OVERFLOW;
        
        // Note the time this processing pass was executed. Note that because we've
        // disabled interrupts the millis() function won't actually be incrementing right
        // at this point, but it will still return the value it was set to just before
        // interrupts went away.
        flowStatsOldTime = newTime;        
    
        return WATER_FLOWING;
        
      } else {
        return WATER_STOPPED;
      }
  }


  /*
  Flow sensor Insterrupt Service Routine
   */
  void flowIncPulseCounter()
  {
    if (getNbValvesOpened() == 0 && programState != CLOSING_MAIN_VALVE)
    {
      if (lastIncoherentPulseCountTime == 0 || ((millis() - lastIncoherentPulseCountTime) > 60000))
      {
         lastIncoherentPulseCountTime = millis();
         incoherentPulseCount = 0;
      }
      
      incoherentPulseCount++;
      
      if (incoherentPulseCount > 40)
      {
        msg = "water is flowing but valves are closed!"; 
        programState = ALERT;
         alertAction();
      }
       
    } 

    // Increment the pulse counter
    flowPulseCount++;

  }

