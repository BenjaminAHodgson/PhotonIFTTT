


// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_TSL2561_U.h>
  
  
// ASSIGNING SENSOR TO HEADER FILE CONSTRUCTOR //
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

// DEFINING RELEVANT VARIABLES //
   int totalSunlight = 0;
   int dailySunlight = 0;
   int startDay = 0;
   bool dailyStart = false;
   bool dailyFinish = false;
   bool dayComplete = false;




void configureSensor()
{
  // changes sensitivity of the sensor based on available sunlight //
  tsl.enableAutoRange(true);           
  
  // The resolution of the sensor - correlates with accuracy //
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS); 

}

// Iterates our sunlight measure for the day //
int sunlightTimer(double currentLux, int totalSunlight)
{
    
    if(currentLux > 500)
    {
        totalSunlight += 5;
    }
    return totalSunlight;
    
}

// Calculates analysis in minutes for total sunlight //
int dailyInformation(int totalSunlight)
{
    int averageSunlight = totalSunlight / 60;
    return averageSunlight;
    
}


void setup() 
{
  /* Declaring Variables for IFTTT */
    
  Particle.publish("Light Sensor Test");
  tsl.begin();
  
  
  /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* Declaring particle.variables */
  Particle.variable("DailySunlight", &dailySunlight, INT);

}



void loop() 
{  
  // Initialize the Time //
  int currentTime = Time.hour();
  
  
  // This checks if we need to start the Lux check again
  // currently set to 6am as the time to start recording lux.
  if(dayComplete == true)
  {
      if(currentTime == 6)
      {
        // changing this variable starts the day again //
        dayComplete = false;
      }
      
      
      // resetting our sunlight time measurements
      dailySunlight = 0;
      totalSunlight = 0;
      dailyFinish = false;
      dailyStart = false;
      
  }

  
  
  if(dayComplete == false)
  {
    // Sets the time variable for each measurement we may take. //
    int runningTime = Time.hour();
    
    // Get a new sensor event // 
    sensors_event_t event;
    tsl.getEvent(&event);
  
    /* Display the results (light is measured in lux) */
    Serial.print(event.light); Serial.println(" lux");
    Particle.publish("Lux: ", String(event.light));
    
    // Here we increment our total time of sunlight measurement
    // passing in the light measurement to check if light is 
    // sufficient //
    totalSunlight = sunlightTimer(event.light, totalSunlight);
    Particle.publish("Sunlight Addition", String(totalSunlight));
    
    // This is our loop delay, we measure lux every 5 seconds //
    delay(5000);
    
    // THis checks if the day has started yet, if not, we check
    // if we have enough lux to start the day //
    if(dailyStart == false && dailyFinish == false)
    {
        // If we have enough light //
        if(event.light > 500)
        {
            // Indicating the outcome in Particle console //
            Particle.publish("Sufficient Light", "case1");
            // Initializing the start of the day time //
            startDay = Time.hour();
            // Here we assign our measured lux to a string variable //
            String measuredLux = String(event.light);
            // And publish to the console //
            Particle.publish("measuredLux", measuredLux);
            // This event is for the IFTT, to indicate the start of the day //
            Particle.publish("Day_status", "started");
            dailyStart = true;
        }
    }
    
    // Once our day has started, we want to check when we should end it //
    if(dailyStart == true && dailyFinish == false)
    {
        // This accounts for variation when the sun is rising //
        int sunriseError = Time.hour() - startDay;
        // If we no longer have enough light //
        if(event.light < 500 && sunriseError > 1)
        {
          // TO ensure it's not a small interference, we wait 
          // and then check again //
          delay(2000);
          if(event.light < 500)
          {
            Particle.publish("Insufficient Light", "case2");
            String measuredLux = String(event.light);
            Particle.publish("measuredLux", measuredLux);
            Particle.publish("Day_status", "ended");
            dailyFinish = true;
          }

        }
    }
    
    
    // Here we send through the details of the total sunlight
    // to the IFTTT, only if the day has ended //
    if(dailyFinish == true)
    {
         dailySunlight = dailyInformation(totalSunlight);
         Particle.publish("DailySunlight", String(dailySunlight));
         dayComplete = true;
    }
    
     
     
   } 
  
  
}
