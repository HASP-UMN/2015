typedef enum SM_State {IDLE,RD_PEAK,RD_IMU,RD_GPS,DOWNLINK,LOG_DATA,...} state;

volatile bool eventA_flag = false;
volatile bool eventB_flag = false;
volatile bool coincEvent_flag = false;


void checkFlags(void);
void checkState(void);

void eventA_ISR(void)
{
    if(!eventA_flag && !eventB_flag && !coincEvent_flag)
        eventA_flag = true;
}

void eventB_ISR(void)
{
    if(!eventA_flag && !eventB_flag && !coincEvent_flag)
        eventB_flag = true;
}

void coincEvent_ISR(void)
{
    if(!eventA_flag && !eventB_flag && !coincEvent_flag)
        coincEvent_flag = true;
}

void main()
{
	//setup and initialization code goes here
	state = IDLE;
	
	while(1) {
		checkFlags();
		checkState();
	}
}

void checkFlags()
{
	if(eventA_flag || eventB_flag || coincEvent_flag) {
		state = RD_PEAK;
	}
}

void checkState()
{
	switch(state) {
		
		case IDLE:
			if(// 50 ms has passed since last IMU read)
				state = RD_IMU;
			else if(// 50 ms has passed since last logged packet)
				state = LOG_DATA;
			else if(// 1 sec has passed since last GPS read)
				state = RD_GPS;
			else if(// 5 sec has passed since last downlink packet)
				state = DOWNLINK;
			else
				state = IDLE;
			break;
		
		case RD_PEAK:
			if(eventA_flag) {
				// get timestamp
				// get detector A peak value
				// pulse channel A reset
				eventA_flag = false;
			}
			else if(eventB_flag) {
				// get timestamp
				// get detector B peak value
				// pulse channel B reset
				eventB_flag = false;
			}
			else if(coincEvent_flag) {
				// get timestamp
				// get detector A peak value
				// pulse channel A reset
				// get detector B peak value
				// pulse channel B reset
				coincEvent_flag = false;
			}
			state = IDLE;
			break;
		
		case RD_IMU:
			// read from IMU and into data structure
			state = IDLE;
			break;
			
		case RD_GPS:
			// read from GPS and into data structure
			state = IDLE;
			break;
			
		case LOG_DATA:
			// log entire data structure
			state = IDLE;
			break;
			
		case DOWNLINK:
			// downlink a data packet
			state = IDLE;
			break;
			
		default:
			state = IDLE;
			break;
			
	}
}