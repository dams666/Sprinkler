 //------------------------------------------------------------------------------------
  // PROGRAM STATE
  //------------------------------------------------------------------------------------

  enum  programStates {
    INITIALIZING,
    ACTIVATING_MOISTURE_SENSORS,
    READING_MOISTURE_SENSORS,
    INSPECTING_FOR_CHANGES,
    CLOSING_MAIN_VALVE,
    ALERT
  } 
  programState;

  /*
  On définit un état spécifique des lors qu'il dure un certain temps
   
   INITIALIZING : 
   
   après allumage ou reboot de l'arduino après plantage. Les vannes secondaires sont ouvertes puis refermées pour dissiper l'eau 
   qui se serait éventuellement accumulée dans les tuyaux entre la vanne princuppale et la vanne secondaire
   
   ETAT(S) PRECEDENT(S) :
   AUCUN
   ETAT(S) SUIVANT(S)   : 
   ACTIVATING_MOISTURE_SENSORS 
   
   ACTIVATING_MOISTURE_SENSORS :
   
   activation de l'ensemble des détecteurs d'humidité. On laisse un court instant passer avant de lire les résultats (cf état READING_MOISTURE_SENSORS)
   
   ETAT(S) PRECEDENT(S) :
   INITIALIZING
   INSPECTING FOR CHANGES : si toutes les vannes secondaires ont été fermées, on passe en veille
   
   ETAT(S) SUIVANT(S)   : 
   READING_MOISTURE_SENSORS
   
   READING_MOISTURE_SENSORS :
   
   lecture des détecteurs d'humidité
   
   ETAT(S) PRECEDENT(S) : 
   ACTIVATING_MOISTURE_SENSORS
   INSPECTING FOR CHANGES : si une vanne secondaire est ouverte, donc une plante est arrosée
   
   ETAT(S) SUIVANT(S)   : 
   INSPECTING_FOR_CHANGES
   
   INSPECTING_FOR_CHANGES :
   
   inspection des changements de valeur des détecteurs d'humidité, et ouverture / fermeture des vannes
   
   ETAT(S) PRECEDENT(S) : 
   INSPECTING FOR CHANGES : si incrémentation de l'identifiant de vanne secondaire à inspecter
   
   ETAT(S) SUIVANT(S)   : 
   INSPECTING FOR CHANGES : si incrémentation de l'identifiant de vanne secondaire à inspecter
   CLOSING_MAIN_VALVE : si une vanne secondaire doit être fermée, on ferme d'abord la vanne principale et on attend que l'eau ne circule plus
   
   CLOSING_MAIN_VALVE :
   
   avant de fermer une vanne secondaire, on ferme la vanne principale et on attend un instant que l'eau ne circule plus (l'effet de fermeture n'est pas toujours immédiat).
   Comme la fermeture n'est pas immédiate, cela permet de maintenir une pression nulle dans les tuyaux entre la vanne principale et les vannes intermédiaires.
   
   ETAT(S) PRECEDENT(S) : 
   INSPECTING FOR CHANGES
   
   ETAT(S) SUIVANT(S)   : 
   
   */

  void action()
  {  
      switch (programState)
      {
      case INITIALIZING:
  
        initializeAction();
        break;
  
      case ACTIVATING_MOISTURE_SENSORS:
  
  #ifdef WITH_SERIAL
        Serial.println("ACTIVATING MOISTURE SENSORS");
  #endif
  
        activateMoistSensorsAction();
        break;
  
      case READING_MOISTURE_SENSORS:
  
        readMoistSensorsAction();
  
        break;
  
      case INSPECTING_FOR_CHANGES:
      
        inspectForChangesAction();
        break;
  
      case ALERT:
      
        alertAction();
        break;
      }

  }


