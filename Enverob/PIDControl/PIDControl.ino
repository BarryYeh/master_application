#include "Simple_Function.h"
#include "Constant_Definition.h"


// #include <Encoder.h>

/*== Timer Variables ==*/
Constant_Timer Timer_Encoder ;
Constant_Timer Timer_USB ;
Constant_Timer Timer_Control ;
Constant_Timer Timer_Speed ;

/*== Motor Variables ==*/
volatile signed long slEncoder_Counter = 0 ;
volatile signed long slEncoder_Counter_last = 0 ;
volatile signed long slSpeed = 0;
volatile signed long slLoadCell_Volt_Read_x = 0 ;
volatile signed long slLoadCell_Volt_Read_y = 0 ;
volatile signed long slLoadCell_Volt_Read_z = 0 ;
volatile signed long slLoadCell_Volt_Read_last = 0 ;
volatile signed long slLoadcell_Force = 0; //用於最後做圖
volatile signed long slVolt = 0 ;
volatile signed long slSet_Point = 0 ;
volatile signed long slSet_Volt = 0 ;
volatile signed long slSet_Force = 0 ;
volatile signed long slVolt_Max = 0 ;

static signed long slSet_Wave_Center = 0 ;
static float fWave_Amp = AMPLITUDE_DEFAULT ;
static float fWave_Period = PERIOD_DEFAULT_MS ;
static float fWave_Interval = 6.28*TIME_CONTROL_MS/fWave_Period ;
static float fWaveCounter = 0 ;

static int iSpeed = 0 ;
static int iForce = 0;
static char cControlMode = 0 ;

// Encoder myEnc(PIN_ENCODER_A, PIN_ENCODER_B ) ;

void setup(){
	Hardware_Setup() ;
	Software_Setup() ;
}

void loop(){
	USB_CMD_Task() ;
	Control_Task() ;
	// USB_Plot_SpeedTask() ;
  USB_Plot_Task() ;
}

/*========= Function =========*/
/*==== Setup Function ====*/
void Hardware_Setup(void ){
	/*== LED Setup Function ==*/
		// config IO direction of LED
	pinMode(PIN_LED_ONBOARD, OUTPUT ) ;

	/*== USB Setup Function ==*/
		// config USB UART baud rate
	Serial.begin(BAUDRATE_USB ) ;

	/*== Motor Setup Function ==*/
	pinMode(PIN_INA, OUTPUT ) ;
	pinMode(PIN_INB, OUTPUT ) ;
	pinMode(PIN_ENABLE, OUTPUT ) ;

	pinMode(PIN_ENCODER_A, INPUT ) ;
	pinMode(PIN_ENCODER_B, INPUT ) ;

	// attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A ), Encoder_Task_Simple, CHANGE ) ;
	// attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B ), Encoder_Task_Simple, CHANGE ) ;

	attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A ), Encoder_Task, CHANGE ) ;
	attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B ), Encoder_Task, CHANGE ) ;

	// Encoder_Task_Simple() ;
}

void Software_Setup(void ){
	/*== Timer Setup Function ==*/
	Timer_USB.Timer_Task(0 ) ;
	Timer_Encoder.Timer_Task(0 ) ;
	Timer_Control.Timer_Task(0 ) ;
  Timer_Speed.Timer_Task(0 ) ;

	/*== Motor Control Setup Function ==*/
	slEncoder_Counter = 0 ;
	slSet_Point = 0 ;
	iSpeed = 0 ;
	cControlMode = MODE_STOP ;
	MOTOR_STOP() ;

	/*== Setup Finished!! ==*/
	Serial.print("SetPoint" ) ;
	Serial.print(" ") ;
	Serial.println("Encoder" ) ;
}

/*==== Basic Function ====*/

/*== USB Function ==*/
void USB_CMD_Task(void ){
	static signed long _slWave = 0 ;
	static signed long _slData0 = 0 ;
	static int _iData0 = 0 ;
	static char _cCommand, _cCommand2, _cTemp ;

	static String _sStringIn = "" ;
	static char _cDataIn[BUFFER_SIZE_USB] ;
	static bool _bData_Complete ;

	static int _iTemp =  0 ;

	// Read buffer
	while((_cTemp = Serial.read() ) != -1 )
	{
		if(_cTemp == '\n'){
			_bData_Complete = true ;
			break ;
		}
		else _sStringIn += _cTemp ;
	}

	if(_bData_Complete ){
		_sStringIn.toCharArray(_cDataIn, BUFFER_SIZE_USB ) ;
		// check case
		switch(_cDataIn[0] )
		{
			case CASE_SPEED:
				cControlMode = MODE_SPEED ;
				_iTemp = sscanf(_cDataIn, "s%d", &_iData0 ) ;
				if(_iTemp > 0 ){
					if(abs(_iData0 ) > MAX_DUTY ) ;
          else if(_iData0 == 0 ) cControlMode = MODE_STOP ;
					else iSpeed = _iData0 ;
				}
			break ;
			case CASE_POSITION_STEP:
				_iTemp = sscanf(_cDataIn, "p%ld", &_slData0 ) ;
				if(_iTemp > 0 ){
					if(abs(_slData0 ) > 350000 ) ;//Encoder讀值大於35000會直接變成-35000
					else{
						cControlMode = MODE_POSITION_STEP ;
						slSet_Point = _slData0 ;
					}					
				}
			break ;
			case CASE_WAVE_SIN:
				// check sub-case
				switch(_cDataIn[1] )
				{
					case SUB_CASE_WAVE_AMP:
						_iTemp = sscanf(_cDataIn, "wa%ld", &_slData0 ) ;
						if(_iTemp > 0 ){
							fWave_Amp = abs(_slData0 ) ;
						}
					break ;
					case SUB_CASE_WAVE_PERIOD:
						_iTemp = sscanf(_cDataIn, "wf%ld", &_slData0 ) ;
						if(_iTemp > 0 ){
							_slData0 = abs(_slData0 ) ;
							fWaveCounter = fWaveCounter*(float)_slData0/fWave_Period ;
							fWave_Period = (float)_slData0 ;
							fWave_Interval = 6.28*TIME_CONTROL_MS/fWave_Period ;
						}
					break ;
					default:
					break ;
				}
				if(cControlMode != MODE_POSITION_WAVE ){
					slSet_Wave_Center = slSet_Point ;
				}
				cControlMode = MODE_POSITION_WAVE ;
			break ;
      case CASE_FORCE:
				cControlMode = MODE_FORCE ;
				_iTemp = sscanf(_cDataIn, "f%d", &_iData0 ) ;
				if(_iTemp > 0 ){
					if(abs(_iData0 ) > MAX_FORCE ) iForce = MAX_FORCE;
					else iForce = _iData0 ;
				}
			break ;
      case CASE_VOLT:
				cControlMode = MODE_VOLT ;
				_iTemp = sscanf(_cDataIn, "f%d", &_iData0 ) ;
				if(_iTemp > 0 ){
					if(abs(_iData0 ) > MAX_VOLT ) slVolt = MAX_VOLT;
					else slVolt = _iData0 ;
				}
			break ;

			default:
			break ;
		}
		_sStringIn = "" ;
		_bData_Complete = false ;
	}
}

void USB_Plot_Task(void ){
	if(Timer_USB.Timer_Task(TIME_USB_MS_PLOT ) ){
		Serial.print(slSet_Point ) ;
		Serial.print(" ") ;
		Serial.println(slEncoder_Counter ) ;
	}
}

void USB_Plot_SpeedTask(void ){
	if(Timer_USB.Timer_Task(TIME_USB_MS_PLOT ) ){
		Serial.print(iSpeed ) ;
		Serial.print(" ") ;
		Serial.println(slSpeed ) ;
	}
}

/*== Motor Function ==*/
void Encoder_Task(void ){
	/*== Encoder Conunter ==*/
	static uint8_t _pin_data = 0x00 ;
	static uint8_t _pin_data_last = 0x00 ;
	static uint8_t _pin_temp_1 = 0x00 ;
	static uint8_t _pin_temp_2 = 0x00 ;

	_pin_data = DATA_ENCODER ;
	_pin_temp_1 = _pin_data^_pin_data_last ;
	_pin_temp_2 = (_pin_data >> 1 )^_pin_data_last ;

	if(_pin_temp_1 & MASK_ENCODER ){
		if(_pin_temp_2 & MASK_ENCODER_PLUS ) slEncoder_Counter++ ;
		else slEncoder_Counter-- ;
	}

	_pin_data_last = _pin_data ;
}

void Encoder_Task_Simple(void ){
	/*== Encoder Conunter ==*/
	static uint8_t _pin_data_A = 0x00 ;
	static uint8_t _pin_data_A_last = 0x00 ;
  
	_pin_data_A = _pin_data_A_last & 0b00000011 ;
	if (digitalRead(PIN_ENCODER_A ) ) _pin_data_A |= 0b00000100 ;
	if (digitalRead(PIN_ENCODER_B ) ) _pin_data_A |= 0b00001000 ;
	switch (_pin_data_A ) {
		case 0: case 5: case 10: case 15:
			break;
		case 1: case 7: case 8: case 14:
			slEncoder_Counter++; break;
		case 2: case 4: case 11: case 13:
			slEncoder_Counter--; break;
		case 3: case 12:
			slEncoder_Counter += 2; break;
		default:
			slEncoder_Counter -= 2; break;
	}
	_pin_data_A_last = (_pin_data_A >> 2 ) ;
}

void Control_Task(void ){
	static int _iAbs_duty ;
  static int _iCurrentTime = 0; //ms
  static int _iPrevTime = 0; //ms
  //static signed long _slTime_diff;
	static signed long _slPos_err ;
  //static signed long _slSpeed_err ;//RPM
	static signed long _slDelta_err ;
  //static signed long _slDelta_err_s ;
	static signed long _slIntegral_err ;
  //static signed long _slIntegral_err_s ;
	static signed long _slPrev_err ;
  //static signed long _slPrev_err_s ;
	static signed long _slOutput_temp ;
  static signed long _slOutput_temp_s ;
  static signed long _slVolt_err ;
	static signed long _slDelta_err_v ;
	static signed long _slIntegral_err_v ;
	static signed long _slPrev_err_v ;
	static signed long _slOutput_temp_v ;
	static bool _bSign_output = true ;
	static float _fTemp1, _fTemp2 = 0.0 ;
  
  //slLoadCell_Volt_Read = 5.0 * analogRead(A0) / 1023; //analogRead: 0~5V -> 0~1023
  //slVolt_Max = MAX_FORCE; //* 力對電壓關係;
  slLoadCell_Volt_Read_x = anologRead(X_AXIS); 
  slLoadCell_Volt_Read_y = anologRead(Y_AXIS); 
  slLoadCell_Volt_Read_z = anologRead(Z_AXIS); 

  if(cControlMode == MODE_STOP){
    _slOutput_temp_s = 0;
    if(_slOutput_temp_s > 0 ) MOTOR_CW(abs(_slOutput_temp_s )) ;
		  else MOTOR_CCW(abs(_slOutput_temp_s )) ;
      slSet_Point = slEncoder_Counter ;
  }
	else if(cControlMode == MODE_SPEED ){
    if(Timer_Speed.Timer_Task(TIME_SPEED_CONTROL_MS ) ){
      slSet_Point += (signed long)iSpeed ;
    }
    if(Timer_Control.Timer_Task(TIME_CONTROL_MS ) ){
      _slPos_err = slSet_Point - slEncoder_Counter ;
			_slDelta_err = _slPos_err - _slPrev_err ;
			_slIntegral_err += _slPos_err ;

			_slOutput_temp = ((_slPos_err*POSITION_CONTROL_P + 
								_slIntegral_err*POSITION_CONTROL_I + 
								_slDelta_err*POSITION_CONTROL_D ) >> 8 ) ;
			_slPrev_err = _slPos_err ;

			if(_slOutput_temp >= 0 ) _bSign_output = true ;
			else _bSign_output = false ;

			_slOutput_temp = abs(_slOutput_temp ) ;

			if(_slOutput_temp > MAX_DUTY ){
				LED_ONBOARD_ON() ;
				_slOutput_temp = MAX_DUTY ;
			}
			else LED_ONBOARD_OFF() ;
      
			if(_bSign_output ){
				MOTOR_CW(_slOutput_temp ) ;
			}
			else{
				MOTOR_CCW(_slOutput_temp ) ;
			}
    }
    
	}
  else if(cControlMode == MODE_FORCE){
    if(Timer_Control.Timer_Task(TIME_FORCE_CONTROL_MS ) ){
      if(slLoadCell_Volt_Read_z >= MappedVolt(SET_FORCE) && slLoadCell_Volt_Read_z < MappedVolt(MAX_FORCE)) cControlMode = MODE_STOP;
      if(slLoadCell_Volt_Read_z >= MappedVolt(MAX_FORCE)) cControlMode = MODE_REVERSE;
      _slVolt_err = slVolt - slLoadCell_Volt_Read_z ;
		  _slDelta_err_v = _slVolt_err - _slPrev_err ;
		  _slIntegral_err_v += _slVolt_err ;

		  _slOutput_temp_v = ((_slVolt_err * FORCE_CONTROL_P + 
								_slIntegral_err_v * FORCE_CONTROL_I + 
								_slDelta_err_v * FORCE_CONTROL_D ) >> 8 ) ;

		  _slPrev_err_v = _slVolt_err ;
      slLoadcell_Force = slLoadCell_Volt_Read_z; //* 電壓對力關係;

			if(_slOutput_temp_v >= 0 ) _bSign_output = true ;
			else _bSign_output = false ;

			_slOutput_temp_v = abs(_slOutput_temp_v ) ;

			if(_slOutput_temp_v > MAX_DUTY ){
				_slOutput_temp_v = MAX_DUTY ;
			}

			if(_bSign_output ){
				MOTOR_CW(_slOutput_temp_v ) ;
				//iSpeed = _slOutput_temp_v ;
			}
			else{
				MOTOR_CCW(_slOutput_tem_v ) ;
				//iSpeed = -_slOutput_temp_v ;
			}
    }
  }
  else if(cControlMode == MODE_POSITION_STEP | cControlMode == MODE_POSITION_WAVE ){
		if(Timer_Control.Timer_Task(TIME_CONTROL_MS ) ){
			if(cControlMode == MODE_POSITION_WAVE ){
				// sin wave generator
				_fTemp1 = fWave_Amp*sin(fWaveCounter*fWave_Interval ) ;
				slSet_Point = slSet_Wave_Center + (signed long )_fTemp1 ;
				fWaveCounter += 1 ;
			}
			else fWaveCounter = 0 ;

			// PID controller
			_slPos_err = slSet_Point - slEncoder_Counter ;
			_slDelta_err = _slPos_err - _slPrev_err ;
			_slIntegral_err += _slPos_err ;

			_slOutput_temp = ((_slPos_err*POSITION_CONTROL_P + 
								_slIntegral_err*POSITION_CONTROL_I + 
								_slDelta_err*POSITION_CONTROL_D ) >> 8 ) ;
			_slPrev_err = _slPos_err ;

			if(_slOutput_temp >= 0 ) _bSign_output = true ;
			else _bSign_output = false ;

			_slOutput_temp = abs(_slOutput_temp ) ;

			if(_slOutput_temp > MAX_DUTY ){
				LED_ONBOARD_ON() ;
				_slOutput_temp = MAX_DUTY ;
			}
			else LED_ONBOARD_OFF() ;

			if(_bSign_output ){
				MOTOR_CW(_slOutput_temp ) ;
				iSpeed = _slOutput_temp ;
			}
			else{
				MOTOR_CCW(_slOutput_temp ) ;
				iSpeed = -_slOutput_temp ;
			}
		}
	}
  else if(cControlMode == MODE_VOLT){
    if(Timer_Control.Timer_Task(TIME_FORCE_CONTROL_MS ) ){
      if(slLoadCell_Volt_Read_z >= MAX_FORCE) cControlMode = MODE_STOP;
      
      _slVolt_err = slVolt - slLoadCell_Volt_Read_z ; //slLoadCell_Volt_Read用電供模擬
	  	_slDelta_err_v = _slVolt_err - _slPrev_err ;
	  	_slIntegral_err_v += _slVolt_err ;

	  	_slOutput_temp_v = ((_slVolt_err * FORCE_CONTROL_P + 
								_slIntegral_err_v * FORCE_CONTROL_I + 
								_slDelta_err_v * FORCE_CONTROL_D ) >> 8 ) ;

		  _slPrev_err_v = _slVolt_err ;
      //slLoadcell_Force = slLoadCell_Volt_Read_z; //* 電壓對力關係;

			if(_slOutput_temp_v >= 0 ) _bSign_output = true ;
			else _bSign_output = false ;

			_slOutput_temp_v = abs(_slOutput_temp_v) ;

			if(_slOutput_temp_v > MAX_DUTY ){
				_slOutput_temp_v = MAX_DUTY ;
			}

			if(_bSign_output ){
				MOTOR_CW(_slOutput_temp_v ) ;
			}
			else{
				MOTOR_CCW(_slOutput_tem_v ) ;
			}
    }
  }
  else if(cControlMode == MODE_REVERSE){
    n++;
    MOTOR_CCW(200) ;
    if(n = 50) cControlMode = MODE_STOP;
  }
}
long Mapped_Volt(long _lForce){
  return (_lForce/2 + 7)/4;
}
