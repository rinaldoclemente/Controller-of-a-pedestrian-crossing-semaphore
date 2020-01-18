# ADVANCED VERSION of the Controller-of-a-pedestrian-crossing-semaphore

The scope of this project is to implement a controller of a pedestrian crossing semaphore, using the behaviour of the LPC1768 and the LANDTIGER Board.
For doing that I started from “sample_BUTTON_LED_NVIC_PCON_TIMER_RIT_bouncing_buttons_joystick_ potentiometer_loudspeaker” project. First of all, I implemented all the functionality I used in the project. The first one is the LED_init(), to start with a green-pedestrian red-car configuration (State 0), then I declared global variables and the other states in which the semaphore can fall in:
- State 0: red-car green-pedestrian
- State 1: flashing green
- State 2: green-car red-pedestrian
- State 3: yellow-car red-pedestrian
- State 4: return to State 0
- State 5: maintenance

After that I initialized RIT, which control button pressing (instead of IRQ_Button) with continuous 50ms polling and Timer0, which control different times according to different state (0.5, 1, 5, 15) and joystick functions, which turns into Manteinance (in which is possible to use Potentiometer to regulate the loudness of the speaker) or StopManteinance mode.
The Timer1 is implemented for the sound of the speaker, using the DAC, which have in input a sinusoid (with value divided by 10 to avoid high frequency) multiplied by an amplitude shifted by 6 to entry into VALUE bits.
Amplitude is initialized 10 to reproduce sinusoid, and the sound can be modified only into State 5.
Timer1 is always reproduce with freq=2249, find with k=25MHz/(f'*45), where 25MHz is the bus clock and 45 the sinusoid values; this value reproduce the frequency of 247Hz.

FIRST STEP
As soon as I load the project, the Timer0 starts count (15 sec) into State 0; after that, without any pedestrian request, it passes into State 1 configuration for 5 seconds (with LED_Off(6) and LED_On(6) alternate every 0,5 seconds) and finally into State 2 waiting a pedestrian request.

SECOND STEP
The semaphore is into State 2 and there are two kind of pedestrian request:
- Normal pedestrian request (KEY1/KEY2):
which enables the Timer0 for 5 seconds; after that the semaphore fall into State 3, for other 5 seconds and then into State 4, which implements 15 seconds and return to State 0.
- Blind pedestrian request (INT0):
which enables Timer1, raise the variable blind to 1 and with DAC emits a sound to confirm request, and do the same thing of normal request until State 4, in which there is a control on the blind variable, enabling the DAC, witch emits sound 1 sec on and 1 sec off during green-pedestrian configuration and 0,5 sec on 0,5 sec off during flashing configuration.

THIRD STEP
The semaphore is into State 0 and there is a request:
- Normal request (KEY1/KEY2):
Timer0 reset the counter with initial configuration
- Blind request(INT0):
sound check to confirm, Timer0 reset the counter with initial configuration and Timer1 starts counting with freq=2249

FOURTH STEP
The semaphore is switching between State 2 to State 4 and there is a request:
- Normal request (KEY1/KEY2): no operation
- Blind request (INT0): sound check to confirm, no operation in Timer0, then into State 0 Timer1 starts counting, and speaker stars sound.

FIFTH STEP
The semaphore is into State 0:
- Push joystick to right: join into Mantenaince mode (flashing red-pedestrian, flashing yellow-car), which raises potentiometer value to 1, and so it is possible to use the potentiometer to regulate the loudness of the loudspeaker. From this state if joystick is pressed to left, the program join into StopMaintenance, potentiometer became 0 and current volume is saved and used by Timer1 when a blind request is pushed.
