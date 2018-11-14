#ifdef USES_P001
//#######################################################################################################
//#################################### Plugin 001: Input Switch #########################################
//#######################################################################################################

/**************************************************\
CONFIG
TaskDevicePluginConfig settings:
0: button type (switch or dimmer)
1: dim value
2: button option (normal, push high, push low)
3: send boot state (true,false)
4: use doubleclick (0,1,2,3)
5: use longpress (0,1,2,3)
6: LP fired (true,false)
7: doubleclick counter (=0,1,2,3)

TaskDevicePluginConfigFloat settings:
0: debounce interval ms
1: doubleclick interval ms
2: longpress interval ms
3:

TaskDevicePluginConfigLong settings:
0: clickTime debounce ms
1: clickTime doubleclick ms
2: clickTime longpress ms
3:
\**************************************************/

#define PLUGIN_001
#define PLUGIN_ID_001         1
#define PLUGIN_NAME_001       "Switch input - Switch"
#define PLUGIN_VALUENAME1_001 "Switch"
#if defined(ESP8266)
  Servo servo1;
  Servo servo2;
#endif
#define GPIO_MAX 17
// Make sure the initial default is a switch (value 0)
#define PLUGIN_001_TYPE_SWITCH 0
#define PLUGIN_001_TYPE_DIMMER 3 // Due to some changes in previous versions, do not use 2.
#define PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH 0
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW 1
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH 2
#define PLUGIN_001_DOUBLECLICK_MIN_INTERVAL 1000
#define PLUGIN_001_DOUBLECLICK_MAX_INTERVAL 3000
#define PLUGIN_001_LONGPRESS_MIN_INTERVAL 1000
#define PLUGIN_001_LONGPRESS_MAX_INTERVAL 5000
#define PLUGIN_001_DC_DISABLED 0
#define PLUGIN_001_DC_LOW 1
#define PLUGIN_001_DC_HIGH 2
#define PLUGIN_001_DC_BOTH 3
#define PLUGIN_001_LONGPRESS_DISABLED 0
#define PLUGIN_001_LONGPRESS_LOW 1
#define PLUGIN_001_LONGPRESS_HIGH 2
#define PLUGIN_001_LONGPRESS_BOTH 3

boolean Plugin_001_read_switch_state(struct EventStruct *event) {
  return digitalRead(Settings.TaskDevicePin1[event->TaskIndex]) == HIGH;
}

boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];
  static byte outputstate[TASKS_MAX];
  static int8_t PinMonitor[GPIO_MAX];
  static int8_t PinMonitorState[GPIO_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_001;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_001);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_001));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        // FIXME TD-er: This plugin is handling too much.
        // - switch/dimmer input
        // - PWM output
        // - switch output (relays)
        // - servo output
        // - sending pulses
        // - playing tunes
        event->String1 = formatGpioName_bidirectional("");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        String options[2];
        options[0] = F("Switch");
        options[1] = F("Dimmer");
        int optionValues[2] = { PLUGIN_001_TYPE_SWITCH, PLUGIN_001_TYPE_DIMMER };
        const byte switchtype = P001_getSwitchType(event);
        addFormSelector(F("Switch Type"), F("p001_type"), 2, options, optionValues, switchtype);

        if (switchtype == PLUGIN_001_TYPE_DIMMER)
        {
          char tmpString[128];
          sprintf_P(tmpString, PSTR("<TR><TD>Dim value:<TD><input type='text' name='plugin_001_dimvalue' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
          addHtml(tmpString);
        }

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String buttonOptions[3];
        buttonOptions[0] = F("Normal Switch");
        buttonOptions[1] = F("Push Button Active Low");
        buttonOptions[2] = F("Push Button Active High");
        int buttonOptionValues[3] = {PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH};
        addFormSelector(F("Switch Button Type"), F("p001_button"), 3, buttonOptions, buttonOptionValues, choice);

        addFormCheckBox(F("Send Boot state"),F("p001_boot"),
        		Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        addFormSubHeader(F("Advanced event management"));

        addFormNumericBox(F("De-bounce (ms)"), F("p001_debounce"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]), 0, 250);

        //set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_001_DOUBLECLICK_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_001_DOUBLECLICK_MIN_INTERVAL;

        byte choiceDC = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        String buttonDC[4];
        buttonDC[0] = F("Disabled");
        buttonDC[1] = F("Active only on LOW (EVENT=3)");
        buttonDC[2] = F("Active only on HIGH (EVENT=3)");
        buttonDC[3] = F("Active on LOW & HIGH (EVENT=3)");
        int buttonDCValues[4] = {PLUGIN_001_DC_DISABLED, PLUGIN_001_DC_LOW, PLUGIN_001_DC_HIGH,PLUGIN_001_DC_BOTH};

        addFormSelector(F("Doubleclick event"), F("p001_dc"), 4, buttonDC, buttonDCValues, choiceDC);

        //addFormCheckBox(F("Doubleclick event (3)"), F("p001_dc"), Settings.TaskDevicePluginConfig[event->TaskIndex][4]);
        addFormNumericBox(F("Doubleclick max. interval (ms)"), F("p001_dcmaxinterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]), PLUGIN_001_DOUBLECLICK_MIN_INTERVAL, PLUGIN_001_DOUBLECLICK_MAX_INTERVAL);

        //set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_001_LONGPRESS_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_001_LONGPRESS_MIN_INTERVAL;

        byte choiceLP = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        String buttonLP[4];
        buttonLP[0] = F("Disabled");
        buttonLP[1] = F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])");
        buttonLP[2] = F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])");
        buttonLP[3] = F("Active on LOW & HIGH (EVENT= 10 or 11)");
        int buttonLPValues[4] = {PLUGIN_001_LONGPRESS_DISABLED, PLUGIN_001_LONGPRESS_LOW, PLUGIN_001_LONGPRESS_HIGH,PLUGIN_001_LONGPRESS_BOTH};
        addFormSelector(F("Longpress event"), F("p001_lp"), 4, buttonLP, buttonLPValues, choiceLP);

        //addFormCheckBox(F("Longpress event (10 & 11)"), F("p001_lp"), Settings.TaskDevicePluginConfig[event->TaskIndex][5]);
        addFormNumericBox(F("Longpress min. interval (ms)"), F("p001_lpmininterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]), PLUGIN_001_LONGPRESS_MIN_INTERVAL, PLUGIN_001_LONGPRESS_MAX_INTERVAL);

        //TO-DO: add Extra-Long Press event
        //addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("p001_elp"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
        //addFormNumericBox(F("Extra-Longpress min. interval (ms)"), F("p001_elpmininterval"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][2], 500, 2000);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p001_type"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == PLUGIN_001_TYPE_DIMMER)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p001_dimvalue"));
        }

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p001_button"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = isFormItemChecked(F("p001_boot"));

        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemInt(F("p001_debounce"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("p001_dc"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemInt(F("p001_dcmaxinterval"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("p001_lp"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemInt(F("p001_lpmininterval"));

        //TO-DO: add Extra-Long Press event
        //Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = isFormItemChecked(F("p001_elp"));
        //Settings.TaskDevicePluginConfigLong[event->TaskIndex][2] = getFormItemInt(F("p001_elpmininterval"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        for (byte x=0; x < GPIO_MAX; x++){
           PinMonitor[x] = 0;
           PinMonitorState[x] = 0;
          }

        if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        else
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);

        // read and store current state to prevent switching at boot time
        switchstate[event->TaskIndex] = Plugin_001_read_switch_state(event);
        outputstate[event->TaskIndex] = switchstate[event->TaskIndex];

        // @giig1967g-20181022: if it is in the device list we assume it's an input pin
        setPinState(PLUGIN_ID_001, Settings.TaskDevicePin1[event->TaskIndex], PIN_MODE_INPUT, switchstate[event->TaskIndex]);

        // if boot state must be send, inverse default state
        // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])
        {
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];
          outputstate[event->TaskIndex] = !outputstate[event->TaskIndex];
        }

        // set initial UserVar of the switch
        if (Settings.TaskDevicePin1Inversed[event->TaskIndex]){
          UserVar[event->BaseVarIndex] = !switchstate[event->TaskIndex];
        } else {
          UserVar[event->BaseVarIndex] = switchstate[event->TaskIndex];
        }

        // counter = 0
        Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;

        //used to track if LP has fired
        Settings.TaskDevicePluginConfig[event->TaskIndex][6]=false;

        //store millis for debounce, doubleclick and long press
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]=millis(); //debounce timer
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis(); //doubleclick timer
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis(); //longpress timer

        //set minimum value for doubleclick MIN interval speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_001_DOUBLECLICK_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_001_DOUBLECLICK_MIN_INTERVAL;

        //set minimum value for longpress MIN interval speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_001_LONGPRESS_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_001_LONGPRESS_MIN_INTERVAL;

        success = true;
        break;
      }

    case PLUGIN_REQUEST:
      {
        //String device = parseString(string, 1);
        //String command = parseString(string, 2);
        //String strPar1 = parseString(string, 3);

        // returns pin value using syntax: [plugin#gpio#pinstate#xx]
        if (string.length()>=13 && string.substring(0,13).equalsIgnoreCase(F("gpio,pinstate")))
        {
          int par1;
            if (validIntFromString(parseString(string, 3), par1)) {
            string = digitalRead(par1);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_UNCONDITIONAL_POLL:
      {
        // port monitoring, on request by rule command
        for (byte x=0; x < GPIO_MAX; x++)
           if (PinMonitor[x] != 0){
             byte state = digitalRead(x);
             if (PinMonitorState[x] != state){
               String eventString = F("GPIO#");
               eventString += x;
               eventString += '=';
               eventString += state;
               rulesProcessing(eventString);
               PinMonitorState[x] = state;
             }
           }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        const boolean state = Plugin_001_read_switch_state(event);

        /**************************************************************************\
        20181009 - @giig1967g: new doubleclick logic is:
        if there is a 'state' change, check debounce period.
        Then if doubleclick interval exceeded, reset Settings.TaskDevicePluginConfig[event->TaskIndex][7] to 0
        Settings.TaskDevicePluginConfig[event->TaskIndex][7] contains the current status for doubleclick:
        0: start counting
        1: 1st click
        2: 2nd click
        3: 3rd click = doubleclick event if inside interval (calculated as: '3rd click time' minus '1st click time')

        Returned EVENT value is = 3 always for doubleclick
        In rules this can be checked:
        on Button#Switch=3 do //will fire if doubleclick
        \**************************************************************************/

        if (state != switchstate[event->TaskIndex])
        {
          //reset timer for long press
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis();
          Settings.TaskDevicePluginConfig[event->TaskIndex][6] = false;

          const unsigned long debounceTime = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]);
          if (debounceTime >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0])) //de-bounce check
          {
            const unsigned long deltaDC = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
            if ((deltaDC >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1])) ||
                 Settings.TaskDevicePluginConfig[event->TaskIndex][7]==3)
            {
              //reset timer for doubleclick
              Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;
              Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis();
            }

//just to simplify the reading of the code
#define COUNTER Settings.TaskDevicePluginConfig[event->TaskIndex][7]
#define DC Settings.TaskDevicePluginConfig[event->TaskIndex][4]

              //check settings for doubleclick according to the settings
              if ( COUNTER!=0 || ( COUNTER==0 && (DC==3 || (DC==1 && state==0) || (DC==2 && state==1))) )
                Settings.TaskDevicePluginConfig[event->TaskIndex][7]++;
#undef DC
#undef COUNTER

            switchstate[event->TaskIndex] = state;
            const boolean currentOutputState = outputstate[event->TaskIndex];
            boolean new_outputState = currentOutputState;
            switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
            {
              case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                  new_outputState = state;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                if (!state)
                  new_outputState = !currentOutputState;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                if (state)
                  new_outputState = !currentOutputState;
                break;
            }

            // send if output needs to be changed
            if (currentOutputState != new_outputState)
            {
              byte output_value;
              outputstate[event->TaskIndex] = new_outputState;
              boolean sendState = new_outputState;

              if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                sendState = !sendState;

              if (Settings.TaskDevicePluginConfig[event->TaskIndex][7]==3 && Settings.TaskDevicePluginConfig[event->TaskIndex][4]>0)
              {
                output_value = 3; //double click
              } else {
                output_value = sendState ? 1 : 0; //single click
              }
              event->sensorType = SENSOR_TYPE_SWITCH;
              if (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) {
                if (sendState) {
                  output_value = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
                  // Only set type to being dimmer when setting a value else it is "switched off".
                  event->sensorType = SENSOR_TYPE_DIMMER;
                }
              }
              UserVar[event->BaseVarIndex] = output_value;
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("SW  : GPIO=");
                log += Settings.TaskDevicePin1[event->TaskIndex];
                log += F(" State=");
                log += state ? '1' : '0';
                log += output_value==3 ? F(" Doubleclick=") : F(" Output value=");
                log += output_value;
                addLog(LOG_LEVEL_INFO, log);
              }
              sendData(event);

              //reset Userdata so it displays the correct state value in the web page
              UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
            }
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = millis();
          }
        }

//just to simplify the reading of the code
#define LP Settings.TaskDevicePluginConfig[event->TaskIndex][5]
#define FIRED Settings.TaskDevicePluginConfig[event->TaskIndex][6]

        //check if LP is enabled and if LP has not fired yet
        else if (!FIRED && (LP==3 ||(LP==1 && state==0)||(LP==2 && state==1) ) ) {

#undef LP
#undef FIRED

          /**************************************************************************\
          20181009 - @giig1967g: new longpress logic is:
          if there is no 'state' change, check if longpress interval reached
          When reached send longpress event.
          Returned Event value = state + 10
          So if state = 0 => EVENT longpress = 10
          if state = 1 => EVENT longpress = 11
          So we can trigger longpress for high or low contact

          In rules this can be checked:
          on Button#Switch=10 do //will fire if longpress when state = 0
          on Button#Switch=11 do //will fire if longpress when state = 1
          \**************************************************************************/
          const unsigned long deltaLP = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]);
          if (deltaLP >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]))
          {
            byte output_value;
            byte needToSendEvent = false;

            Settings.TaskDevicePluginConfig[event->TaskIndex][6] = true;

            switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
            {
              case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                  needToSendEvent = true;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                if (!state)
                  needToSendEvent = true;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                if (state)
                  needToSendEvent = true;
                break;
            }

            if (needToSendEvent) {
              boolean sendState = state;
              if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                sendState = !sendState;
              output_value = sendState ? 1 : 0;
              output_value = output_value + 10;

              UserVar[event->BaseVarIndex] = output_value;
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("SW  : LongPress: GPIO= ");
                log += Settings.TaskDevicePin1[event->TaskIndex];
                log += F(" State=");
                log += state ? '1' : '0';
                log += F(" Output value=");
                log += output_value;
                addLog(LOG_LEVEL_INFO, log);
              }
              sendData(event);

              //reset Userdata so it displays the correct state value in the web page
              UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
            }
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("SW   : State ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("gpio"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            if (event->Par2 == 2) {
              pinMode(event->Par1, INPUT);
              setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_INPUT, 0);
            } else {
              pinMode(event->Par1, OUTPUT);
              digitalWrite(event->Par1, event->Par2);
              setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            }
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("gpiotoggle"))
        {
          success = true;
          byte mode;
          uint16_t currentState;

          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            if (hasPinState(PLUGIN_ID_001,event->Par1)) {
              getPinState(PLUGIN_ID_001, event->Par1, &mode, &currentState);
            } else {
              currentState = (digitalRead(event->Par1) == HIGH);
              mode = PIN_MODE_OUTPUT;
            }

            if (mode != PIN_MODE_INPUT) { //toggle only output pins
              pinMode(event->Par1, OUTPUT);
              digitalWrite(event->Par1, !currentState);
              setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, !currentState);
              log = String(F("SW   : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(!currentState);
              addLog(LOG_LEVEL_INFO, log);
              SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
            }
          }
        }

        if (command == F("pwm"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            #if defined(ESP8266)
              pinMode(event->Par1, OUTPUT);
            #endif
            if(event->Par3 != 0)
            {
              byte prev_mode;
              uint16_t prev_value;
              getPinState(PLUGIN_ID_001, event->Par1, &prev_mode, &prev_value);
              if(prev_mode != PIN_MODE_PWM)
                prev_value = 0;

              int32_t step_value = ((event->Par2 - prev_value) << 12) / event->Par3;
              int32_t curr_value = prev_value << 12;

              int i = event->Par3;
              while(i--){
                curr_value += step_value;
                int16_t new_value;
                new_value = (uint16_t)(curr_value >> 12);
                #if defined(ESP8266)
                  analogWrite(event->Par1, new_value);
                #endif
                #if defined(ESP32)
                  analogWriteESP32(event->Par1, new_value);
                #endif
                delay(1);
              }
            }

            #if defined(ESP8266)
              analogWrite(event->Par1, event->Par2);
            #endif
            #if defined(ESP32)
              analogWriteESP32(event->Par1, event->Par2);
            #endif
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_PWM, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set PWM to ")) + String(event->Par2);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("pulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, event->Par2);
            delay(event->Par3);
            digitalWrite(event->Par1, !event->Par2);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if ((command == F("longpulse")) || (command == F("longpulse_ms")))
        {
          boolean time_in_msec = command == F("longpulse_ms");
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            const bool pinStateHigh = event->Par2 != 0;
            const uint16_t pinStateValue = pinStateHigh ? 1 : 0;
            const uint16_t inversePinStateValue = pinStateHigh ? 0 : 1;
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, pinStateValue);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, pinStateValue);
            unsigned long timer = time_in_msec ? event->Par3 : event->Par3 * 1000;
            // Create a future system timer call to set the GPIO pin back to its normal value.
            setPluginTaskTimer(timer, PLUGIN_ID_001, event->TaskIndex, event->Par1, inversePinStateValue);
            log = String(F("SW   : GPIO ")) + String(event->Par1) +
                  String(F(" Pulse set for ")) + String(event->Par3) + String(time_in_msec ? F(" msec") : F(" sec"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("servo"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 2)
            switch (event->Par1)
            {
              case 1:

                //IRAM: doing servo stuff uses 740 bytes IRAM. (doesnt matter how many instances)
                #if defined(ESP8266)
                  //SPECIAL CASE TO ALLOW SERVO TO BE DETATTCHED AND SAVE POWER.
                  if (event->Par3 >= 9000) {
                    servo1.detach();

                  }else{
                    servo1.attach(event->Par2);
                    servo1.write(event->Par3);
                  }
                #endif
                break;
              case 2:
                #if defined(ESP8266)
                if (event->Par3 >= 9000) {
                  servo2.detach();
                }else{
                  servo2.attach(event->Par2);
                  servo2.write(event->Par3);
                }
                #endif
                break;
            }
          setPinState(PLUGIN_ID_001, event->Par2, PIN_MODE_SERVO, event->Par3);
          log = String(F("SW   : GPIO ")) + String(event->Par2) + String(F(" Servo set to ")) + String(event->Par3);
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, log, 0));
        }

        if (command == F("status"))
        {
          if (parseString(string, 2) == F("gpio"))
          {
            success = true;
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, dummyString, 0));
          }
        }

        if (command == F("monitor"))
        {
          if (parseString(string, 2) == F("gpio"))
          {
            PinMonitor[event->Par2] = 1;
            success = true;
          }
        }

        if (command == F("inputswitchstate"))
        {
          success = true;
          UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
          outputstate[event->Par1] = event->Par2;
        }

        // FIXME: Absolutely no error checking in play_rtttl, until then keep it only in testing
        //play a tune via a RTTTL string, look at https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=343&hilit=speaker&start=10 for more info.
        if (command == F("rtttl"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 16)
          {
            pinMode(event->Par1, OUTPUT);
            // char sng[1024] ="";
            String tmpString=string;
            tmpString.replace('-', '#');
            // tmpString.toCharArray(sng, 1024);
            play_rtttl(event->Par1, tmpString.c_str());
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : ")) + string;
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        //play a tone on pin par1, with frequency par2 and duration par3.
        if (command == F("tone"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            pinMode(event->Par1, OUTPUT);
            tone_espEasy(event->Par1, event->Par2, event->Par3);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : ")) + string;
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        digitalWrite(event->Par1, event->Par2);
        setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        break;
      }
  }
  return success;
}


#if defined(ESP32)
void analogWriteESP32(int pin, int value)
{
  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;
  for(byte x = 0; x < 16; x++)
    if (ledChannelPin[x] == pin)
      ledChannel = x;

  if(ledChannel == -1) // no channel set for this pin
    {
      for(byte x = 0; x < 16; x++) // find free channel
        if (ledChannelPin[x] == -1)
          {
            int freq = 5000;
            ledChannelPin[x] = pin;  // store pin nr
            ledcSetup(x, freq, 10);  // setup channel
            ledcAttachPin(pin, x);   // attach to this pin
            ledChannel = x;
            break;
          }
    }
  ledcWrite(ledChannel, value);
}
#endif

// TD-er: Needed to fix a mistake in earlier fixes.
byte P001_getSwitchType(struct EventStruct *event) {
  byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
  switch (choice) {
    case 2: // Old implementation for Dimmer
    case PLUGIN_001_TYPE_DIMMER:
      choice = PLUGIN_001_TYPE_DIMMER;
      break;
    case 1: // Old implementation for switch
    case PLUGIN_001_TYPE_SWITCH:
    default:
      choice = PLUGIN_001_TYPE_SWITCH;
      break;
  }
  return choice;
}
#endif // USES_P001
