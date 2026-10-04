/*
 JUMA-PA100 Amplifier Controller
 Juha Niinikoski, OH2NLT 06.07.2008

 Microchip MPLAB + C30 compiler
 MCU dsPIC30F6014A

 Auto F-sense code 08.07.2008
 Boot loader & Temp scaling 09.07.2008
 EEPROM structures & Encoder simulator 17.07.2008
 Power meter fast attack, slow release 04.08.2008
 Improved F-sense code 05.08.2008
 Yaesu 817 band select tests, check_io_board_type() removed 07.08.2008
 TRX2 serial band set protocol 09.08.2008
 7,3728MHz XTAL, repeat for UP/DN buttons 13.08.2008
 SWR Alarm, Alarm etc, setup displays 13.08.2008
 PA100 - TRX2 serial protocol 14.08.2008
 Yaesu FT817 band voltage table matched with 4M7 input resistance 16.08.2008
 SWR alarm fine tuning 22.08.2008
 Display clean up, Auto/Man logic changed, Alarm beep added 12.11.2008 v0.04
 RS232 band set & query  active only if TRX2 protocol is selected 15.11.2008
 RS232 printouts only in the test mode, do not send garbage to the TRX2 15.11.2008
 Some text changed, default calibration values changed 15.11.2008
 TX capability added to the service mode 15.11.2008 v1.00 
 PA100 temp meter degree sign added to the soft fonts #4 16.11.2008 v1.00
 TRX2 protocol poll rate adjust added 17.11.2008 v1.00
 Alarms force normal display, some text changes 18.11.2008
 UART module data type cast corrections 18.11.2008
 Band +/- end of range tones changed, OPER ==> STBY while TX on corrected 19.11.2008 v1.01
 Band info, ! = out of band, ? = not known, TX/tx display added 22.11.2008	v1.02
 Poll timer changed from calval to defval, TRX2 poll response time-out, some display text changes 30.11.2008 v1.03
 Service mode factory setup function corrected 16.12.2008 v1.04
 ---------------------------------- 5B4AIY Modifications ----------------------------------
 Version 1.05h
 Various typographical errors fixed, minor display formatting improvements - 5B4AIY 23/AUG/2011
 Changed current measurement to a 50 sample running average - 5B4AIY 24/SEP/2012
 Added selectable graphical display of measurements in relation to their trip limits. - 5B4AIY 04/OCT/2012
 Cleaned up some code, removed redundant variables. - 5B4AIY 04/OCT/2012
 Build 6.
 Added feature to allow PA-100D to work with the Elecraft KX3 transceiver.
 Build 7.
 Removed redundant code related to fsense logic - Frequency Sense board is always present.
 Minor changes to prompt messages for Calibration and User Configuration menus.
 Build 8.
 Changed polling so that it now increments in 1 second intervals from 0 - 10 seconds.
 As before, 0 = Disabled. - 5B4AIY 15/OCT/2012
 Build 10.
 Revised EEPROM layout and changed id_mult, batt_mult, rev_pwr_mult and fwd_pwr_mult to integers. A.Ryan 5B4AIY 22/OCT/2012
 Build 14.
 Revised peak hold time from 350mS to 1S, and fast decay time from 50mS to 20mS.
 Optimised service and configuration routines. A.Ryan - 5B4AIY
 Build 15.
 Commented out all the loop counter code used to test the loop execution time. A.Ryan 23/NOV/2012
 Build 16.
 Replaced several one-line functions with equivalent macro definitions in juma-pa100.h A.Ryan - 5B4AIY - 27/NOV/2012
 Build 17.
 Replaced inelegant attenuator relay switch and fan speed switch code with more elegant version.
 34 lines of switch/case code replaced with 4 lines of conditional expression code.
 Replaced loop invariant address calculations with pointer set during boot initialisation. (rs232_mode)
 Re-organised the order of the frequency sense modes.
 Modified the way the serial test mode is selected. If either the TRX-2 or KX3 frequency sense modes are selected,
 then the serial test mode selection is skipped. If either the F-Sense or FT-817 frequency sense modes are selected,
 then the serial test page is displayed, and the test mode can be turned on or off. A.Ryan - 5B4AIY - 7/JAN/2013

 Version 1.05i
 Build 1
 Moved the Polling Interval time setting from the calibration menu to the configuration menu. With the
 option of using either the TRX-2 or the KX3 serial port auto-band detect modes, it made more sense to
 combine this with their polling interval settings rather than having to switch to the calibration setting.
 The logic now skips the polling interval setting page if either the Frequency Sense or the FT-817 auto band detect
 mode is selected, or skips the Serial Test mode setting page if either the TRX-2 or KX3 mode is selected.
 Modified the Service Mode Default so that it uses a prompted exit as with the normal save settings.
 Modified the power up boot sequence so that the long blank screen when selecting the Service Mode is now
 replaced with the normal sign-on splash screen. In addition, the save_settings() function has been modified
 to check for the BAND+ push button pressed as well as the PWR button, which greatly simplified the default
 restore option when powering up. A.Ryan - 5B4AIY - 9/JAN/2013
 Build 2
 Moved all embedded literal prompts and messages to constant character strings.
 Modified the way the main display page is selected. If the KEY status is used directly rather than assigning
 its value to an internal variable, then this takes 54 bytes more code! A.Ryan - 5B4AIY - 18/JAN/2013
 Build 3
 Added a test in four places to inhibit printouts to the serial port if the Auto-Band Detect mode is set to
 either TRX-2 or KX3, or if the Serial Test Mode is set to OFF. This prevents spurious characters being
 transmitted to the TRX-2 or the KX3, which might confuse them. Modified the operation of the command time-out
 timer so that it is disabled if the polling timer is enabled. This permits the user to perform a test to
 verify that the KX3 command parser is working by sending the polling response string from a terminal.
 For example, set the Auto-Band Detect to KX3. Select the AUTO mode of band detection.
 Enable the Polling Timer and set it to 10 seconds. Connect the serial port to a PC running a terminal program.
 Verify that every 10 seconds you receive the polling request: IF; then enter the text: IF14000000; and the
 amplifier should immediately switch to 20m. Wait for the polling time-out to occur, and the band should change
 to: ? By triggering the TX data line by grounding the tip of a standard 3.5mm stereo patch cable connected to
 the T/R socket on the rear of the amplifier, you can also verify that the TX Enable/Disable logic is working.
 Select MANUAL, and OPER. Ground the tip. The amplifier should show TX. Now select AUTO, ground the tip, and
 the amplifier should show NOP. Wait for the poll request, and send: IF7000000; and the amplifier should switch
 to 40m and TX. When the polling request next times-out, the state should switch to NOP if the tip is still
 grounded.
 Removed set_cgram() from lcd-trx2.c - never referenced. Optimised set_ch_bits() A.Ryan - 5B4AIY - 22/JAN/2013
 Build 4
 Restructured both the clr_rx_buffer() and clear_buffer() functions to use a simple DO-WHILE loop rather than
 the more usual FOR-NEXT loop. This used 15 bytes less memory in each case. A.Ryan - 5B4AIY - 25/JAN/2013
 Build 5
 Removed send_band_data() from serial_pa100.c Only referenced once, replaced with single line of code.
 Removed ms_counter from timers_pwm.c, never used. Changed maximum backlight level from 1100 to 1000,
 there was very little change above 1000.
 Set initial backlight and contrast to their default values. A.Ryan - 5B4AIY - 04/MAR/2013
 Build 6
 Added function display_screen() to bring together 5 sections of identical code, as well as improve
 the usage of the data section by removing a duplicate prompt. Re-wrote the section of code that
 displays the STDBY/TX/NOP/OPER message. Replaced SWITCH/CASE construct with simpler IF/ELSE. A.Ryan - 04/JUN/2013

 Version 1.05j
 Build 1
 Re-wrote set_relay() function to eliminate large amounts of duplicated code. Added a feature to the serial test
 suite to display the currently selected output filter. A.Ryan - 5B4AIY - 07/JUN/2013
 Build 2
 Further simplification of the set_relay() function, reducing it to only 8 lines of code, as compared
 to 57 in the original. A.Ryan - 5B4AIY - 08/JUN/2013
 Build 3
 Changed ammeter to a peak reading long hold meter. The graphic meter is still an instantaneous reading
 meter. The peak reading ammeter is disabled in the Service Mode. A.Ryan - 5B4AIY - 11/JUN/2013

 Version 1.05k
 Build 1
 With the availability of an actual KX3, it was possible to optimise and improve the compatibility. With
 the previous firmware when using the KX3's AUTO-INFORM mode set to ANT-CTRL, the band change response time
 of the PA-100D was often as long as 5 seconds. This was caused by the KX3 sending the IF data packet
 last, and this could take 5 seconds. An examination of the KX3's response in this mode showed that the
 first data packet was the FA VFO-A frequency data, followed by the FB VFO-B data. It thus made sense to
 recognise these packets and set the PA-100D's band accordingly. This version of the firmware modifies the
 operation. If polling is enabled, then the only acceptable response string is the IF data packet, and this
 must be received within the prescribed time-out period to be valid. The time-out period is the polling interval.
 If polling is disabled, then the KX3 must be set to AUTO-INFORM with the ANT-CTRL mode selected. In this mode
 the acceptable responses for a band change is the FA/FB VFO frequency data, as well as the IF information packet.
 In the AUTO-INFORM mode, a message timer of 500mS is started when the first valid character is received.
 This is more than sufficient for the longest message at the slowest speed. It simply ensures that
 if serial communications is interrupted then the PA-100D does not simply wait forever for a response but will
 clear the buffer and restart the serial command handler.
 An example of a KX3 Auto-Inform data packet is:
 Select 20m:
 FA00014060000;
 FB00014060400;
 RT0;
 RO-0000;
 MD2;
 MD$2;
 IF00014060000     -000000 0002000001 ;
 A. Ryan - 5B4AIY - 14/JUL/2013
 Build 2
 After examining the KX3 data packets, it was obvious that only the FA packet needs to be examined. Whenever
 a band change occurs and AUTO-INFORM is on, then the FA packet is always sent first. There is no need to
 examine any other data packet. This permitted a considerable simplification of the KX3 parser, eliminating
 the check_buffer() function, and allowing the parser to automatically select the FA data and reject everything
 else. I also added a 'hidden' debugging function. If the BAND- button is pressed, and then the power turned on,
 the KX3 message time-out value is increased from 500mS to 5000mS so that the PA-100D can be tested with a
 terminal program when in the KX3 mode. To ensure that when polling is disabled the PA-100D still selects a
 valid band, the first time the serial port function is exercised a forced FA query command is transmitted,
 and the KX3 will respond with the current setting. This will cause the band to be set correctly. To ensure
 that this happens, power up the KX3 first and let it initialise, then power up the PA-100D. A.Ryan 14/JUL/2013
 Build 3
 Changed the Yes_No[] prompt message. Since the PWR button is used for the 'No' response, and it is located on the
 left-hand side of the amplifier's control panel, and the BAND+ button for the 'Yes' response, located on the
 right-hand side of the control panel, it made more sense for the message to be so arranged. In addition,
 the BAND- button response has been eliminated. A.Ryan - 5B4AIY - 31/AUG/2013
 Build 4
 Modified the set_value() function to avoid duplicative address calculations. The second argument is now a pointer
 to an integer rather than the value itself, and the function now modifies the value directly via this pointer, thus
 avoiding needless address duplication. This saved 45 bytes. Removed the null() function. This simply discarded any characters
 in the USART1 buffer if the test mode was off, but the same effect can be obtained by simply resetting the buffer pointers,
 thus eliminating a redundant function, which saved an additional 21 bytes. A.Ryan - 5B4AIY - 21/NOV/2013

 1.05m
 NOTE: There is no version l.05l - this is because the l and the figure 1 are not easily distinguishable.

 Build 1
 Rationalised the way the User Configuration and System Calibration Settings are saved. Previously the User Configuration
 settings were saved by pressing and holding the DISPLAY/CONFIG button, and the System Calibration Settings by pressing
 the OPER button. This version rationalises this by using the OPER button for both. A.Ryan - 5B4AIY - 03/DEC/2013
 Build 2
 Added Auto-Increment to the User Configuration and System Calibration Menus. To use this feature, press and hold the
 DISPLAY/CONFIG button. After a short delay, the pages will automatically increment. A.Ryan - 5B4AIY - 04/DEC/2013
 Build 3
 Modified serial_kx3() to eliminate a switch-case, replaced with two IF-THEN constructs. A.Ryan - 5B4AIY - 04/DEC/2013
 Build 4
 Optimised increment_cal_page() and increment_cfg_page() by combining their logic tests into a single expression.
 This saved 18 bytes. A.Ryan - 5B4AIY - 06/DEC/2013
 Build 5
 Removed header file: lcd-trx2.h and incorporated its contents into: juma-pa100.h. Fixed low-level re-definition bug.
 In file: juma-pa100.c the symbol S_BAR was re-defined, although it was never used in this module. A.Ryan - 5B4AIY - 27/DEC/2013
 Build 6
 Removed set_cur_lcd() and replaced with lcd_cmd(), changing all the cursor and line positioning commands to their
 intrinsic values. Optimised initlcd(). A.Ryan - 5B4AIY - 02/JAN/2014
 Build 7
 Relocated local static variable old_data to exchg_data_spi1(). Removed local variables spi_rx_data, never used.
 Optimised SPI1 Interrupt, added explanation of how the SPI bus operates to spi1.c Removed SPI code - not used. A.Ryan - 5B4AIY - 3/JAN/2014
 Build 8
 Simplified the code to show band information. A.Ryan - 5B4AIY - 26/JAN/2014
 Build 9
 Re-wrote disp_meter() considerably simplifying it. Rationalised the SWR display. A.Ryan - 5B4AIY - 27/JAN/2014
 Build 10
 Changed the PAGE_CHANGE constant to 1000mS. A.Ryan - 5B4AIY - 27/JAN/2014
 Build 11
 Fixed calibration display bug. Voltage was displayed in mV rather than Volts. A.Ryan - 5B4AIY - 31/JAN/2014
 Build 12
 Replaced EEPROM variable: eeprom.defval.pa_state with global flag: pa_state. Added new feature, Start-Up Page Select. This utilises
 the variable: eeprom.defval.pa_state. It is defined as: Start_Page as a macro in juma-pa100.h A.Ryan - 5B4AIY - 07/FEB/2014
 Build 13
 Changed the LONG_PUSH to 650mS, and the VERY_LONG_PUSH to 1,500mS. A.Ryan - 5B4AIY - 08/MAR/2014
 Build 14
 Used svc_flag to both remain in and exit from service(). A.Ryan - 5B4AIY - 22/MAR/2014
 Build 15
 Modified putch() in uart.c to eliminate redundant mask operation. Optimised sound test in serial_test.c. Fixed minor bug. If a
 loop-back plug is inserted into the RS-232 port and the RS-232 loop-back test is selected, it would continue to run forever. This
 has now been fixed. A.Ryan - 5B4AIY - 07/APR/2014
 Build 16
 Eliminated redundant Get_Character() function in Serial Test Suite, and added a buffer clear to the illegal character
 code. Eliminated redundant variable ultemp. A.Ryan - 5B4AIY - 17/APR/2014

 1.05p
 Build 1
 Complete re-write of button recognition and function dispatch logic. Removed redundant logic tests, superfluous variables
 and function calls. Although this did not have as dramatic an effect as with the TRX-2, it was still a worthwhile exercise
 and resulted in more compact code, and more elegant construction. Modified the auto-increment feature to give a longer beep
 when switching back to the first page, as well as adding an extra delay. The same sounds and logic is used for the
 Calibration & Service and the User Configuration functions. Added code to ignore DISPLAY/OPER/AUTO/BAND-/BAND+ buttons in
 alarm state. A.Ryan - 5B4AIY - 28/APR/2014
 Build 2
 Added code to ignore UP/DOWN buttons in alarm state. Only the PWR button is now recognised. Modified the 1mS interrupt
 handler to use a DO-WHILE loop in the band select module with a pre-increment loop counter, as this uses fewer machine cycles.
 Similarly, the functions eval_band() and reset_fsense() were also modified. A.Ryan - 5B4AIY - 29/APR/2014
 Build 3
 One of the persistent annoyances is the relay chatter when using the frequency sense mode and SSB. An examination of the
 fsense test output reveals why. During speech you can sometimes get significant counts in the lower frequency bins because
 of the syllabic rate of speech. This is particularly evident when using only a low power to drive the amplifier, and it
 causes a low-frequency band filter to be temporarily selected, because the count in the correct band is zero, whereas
 there is a count in a lower band because of this miscounting occurring during speech syllables. To effect a cure, the
 eval_band() function was greatly modified. A new static variable, last_band, has been introduced, and during the
 transmit mode this is set to the measured frequency band. If on subsequent iterations a higher frequency band is detected,
 then this is updated, as is eeprom.defval.band. In this way the band is always set to the highest frequency and never
 reset to a lower band. In the receive mode, last_band is reset for the next transmit cycle. The frequency sense auto band
 detect works in STANDBY as well as OPERATE, whether there is an external key signal, or whether using the JUMA transmit
 key signal. A.Ryan - 5B4AIY - 29/APR/2014
 Build 4
 Inhibited manual band changes in transmit. A.Ryan - 5B4AIY - 01/MAY/2014
 Build 5
 Re-wrote display_pwr_meter() and re-named it scaled_value(). It was only called once and the SWITCH-CASE construct could
 be replaced with a simple function call. Optimised getch() and kbhit(). Minor improvements to the LCD Write ASCII and
 Write HEX tests. A.Ryan - 5B4AIY - 23/MAY/2014
 Build 6
 Re-wrote reset_fsense() to eliminate redundant flag. The variable freq_sample_ctr already contains the required information
 to determine when a valid number of counts has been accumulated. Modified the routine in the 1mS timer interrupt to suit.
 Modified the interrupt handler so that if a frequency is detected that is higher than 30MHz, the amplifier is immediately
 forced to the STANDBY state, and the band set to NOT_KNOWN. A.Ryan - 5B4AIY - 11/JUN/2014
 Build 7
 Optimised clear_buffer() in serial_pa100.c and simplified serial_pa100(). A.Ryan - 5B4AIY - 13/JUN/2014
 Build 8
 Combined the KX3 and PA-100 serial buffers. Removed the redundant KX3 clear buffer function. A.Ryan - 5B4AIY - 06/AUG/2014
 Build 9
 Modified wait_lcd_ready() in lcd-trx2.c to remove redundant assignment, and use simpler DO-WHILE loop. Shortened the Read Data
 setup delay from 2uS to 1uS in wait_lcd_ready(). The maximum delay to data valid is 320nS, the minimum available delay time
 is 1uS. Removed redundant Restore Defaults page in System Calibration & Setup. This facility is already available by using
 the MODE button from the initial power-on. A.Ryan - 5B4AIY - 17/AUG/2014

 1.05q
 Build 1
 Changed the way the User Configuration menu is initially displayed. In the previous versions, when the User Configuration menu
 was selected, the current configuration page was displayed. In this version, the message: 'User Configuration' is displayed as
 long as the DISPLAY/CONFIG button is depressed. When it is released, the current configuration page is displayed. A subsequent
 long push will auto-increment the configuration pages, as before. A.Ryan - 5B4AIY - 14/SEP/2014
 Build 2
 Fixed bug in serial test suite. If using the Yaesu band information, there would be an Address Trap error when the band value
 was greater than 7 - insufficient entries in the table - fixed. A.Ryan - 5B4AIY - 08/OCT/2014

 1.05r
 Build 1
 Eliminated an unused prompt in service.c, and simplified the TX switch logic. Forced the use of the Frequency Sense mode,
 and the minimum gain for the ammeter and power meter calibration. Changed logic so that the amplifier will only switch to
 the transmit mode when either the ammeter or RF power calibration pages are selected. Forced Calibration & Setup abort if
 an alarm is detected. Saved the current band gain settings on entry, and restored them on exit to the Calibration & Setup
 mode. A.Ryan - 5B4AIY - 27/OCT/2014
 Build 2
 Modified disp_id() so that the current is always displayed, even in the receive mode. Since the current measured is the
 total amplifier current, there is no reason not to display it at all times if this display mode is selected. The display
 has been updated so that if there is a change of amplifier state then the display is immediately updated without waiting
 for the hold time to expire, and the hold counter is reset. A.Ryan - 5B4AIY - 28/OCT/2014
 Build 3
 In the previous firmware, when the Service Mode is selected, there is a low-level bug that sometimes causes a mode and/or
 a VFO change to the TRX-2. The conditions have to be very specific: the Juma TRX-2 has to have the TRX2 serial port
 protocol selected; the speed has to be 9600 Baud; the serial port cable has to be connected to the amplifier's RS-232
 port; the amplifier's User Configuration frequency select mode has to be either F-SENSE or FT-817; the Serial Test flag
 has to be ON; and the Juma TRX-2 mode has to be TUNE and VFO-A. If these conditions are met, then when exiting the Service
 Mode after a short delay of about 1 or 2 seconds, the TRX-2 mode will change to USB, and sometimes VFO-B will also be
 selected. However, if the amplifier's Serial Test flag is set to OFF, this does not occur. The reason for this is that
 when the transceiver's band is changed, or if it is keyed this will send a status update command to the amplifier, which
 will be saved in the serial buffer. When the Service Mode is exited, the amplifier's Serial Test Suite will retrieve
 these saved commands from the buffer and attempt to implement them. This will provoke a series of error messages, and
 some of these could be interpreted by the TRX-2 as mode or VFO change commands. Although the same effect could be achieved
 by simply clearing the USART1 queue, when exiting the Service Mode if the Serial Test Suite is enabled and the TRX-2
 is in the TRX2 protocol mode, then the continuing status updates would cause more spurious operation, so it is much
 better to simply set the flag OFF. The user can then reset it for when it is required. A.Ryan - 5B4AIY - 10/NOV/2014

 1.06a	This version uses additional EEPROM and will cause a checksum error on initial load.
 Build 1
 Added feature to allow display of RF O/P power in either Watts or dBm. A.Ryan - 5B4AIY - 13/NOV/2014
 Build 2
 Added feature to allow the use of the F-SENSE mode to display the input frequency. (See notes in timers_pwm.c for details
 of the changes and an explanation of the algorithm used.) A.Ryan - 5B4AIY - 15/NOV/2014
 Build 3
 Added calibration feature for the low power range of the RF power meter. This uses an additional word of EEPROM
 and so will cause a checksum error on initial load. Fixed minor bug in Serial Test suite. When using the F-SENSE
 automatic mode of band detection if the ADC test is invoked, the band resets to UNKNOWN at the conclusion of the
 test. This was because the FT-817 band evaluation function was invoked, and this would reset the current band
 based upon any voltage detected on this port. Modified eval_y817() and added function get_817() to
 fix this problem. Changed power meter display so that you no longer need to key the amplifier in order to display
 output power. This allows a transceiver's output power to be displayed in the STANDBY mode. Also modified the
 lower limit of the display to be +26dBm/400mW. Measurements indicate that the frequency counter will read reliably
 with input powers as low as 100mW, and the low power reading is accurate down to 500mW. Equally, the counter will
 display frequencies up to 54MHz with only a slightly reduced input sensitivity. A.Ryan - 5B4AIY - 21/NOV/2014
 Build 4
 Changed test in disp_fwd_pwr() from: if(out_pwr > peak_pwr) to: if(out_pwr >= peak_pwr) to eliminate an annoying
 display blink when the power is constant. Improved the accuracy of the SWR indication by changing calculation
 from long integer to floating-point. A.Ryan - 5B4AIY - 23/NOV/2014
 Build 5
 Introduced a new global variable vswr. This is a double, and takes the value from calc_swr(). The value is converted
 to an int and assigned to swr. vswr is used in the call to disp_meter() to save having to convert from a double to an
 int and then back to a double, and so should marginally improve the accuracy of the meter. A.Ryan - 5B4AIY - 25/NOV/2014
 Build 6
 Moved the LCD setup for the EEPROM values of backlighting and contrast to occur after the check for the default
 restore. Minor optimisations in initlcd(), draw_s_meter() (lcd-trx2.c), set_factory_defaults(), get_band(),
 reset_fsense(), get_817_band(). Minor optimisations in serial_test.c A.Ryan - 5B4AIY - 24/JAN/2015

 1.06b
 Build 1
 Considerably simplified the frequency counter calculation, which greatly improved its accuracy. See comments
 in _T3Interrupt() (timers_pwm.c) and get_freq(). The calibration factor adjustment limits give a +/- 10kHz
 correction at 10m. A.Ryan - 5B4AIY - 07/FEB/2015
 Build 2
 Minor optimisation in get_freq() Minor formatting changes in serial_test.c A.Ryan - 5B4AIY - 08/FEB/2015
 Build 3
 Added feature that uses a short push of the PWR button to decrement the main display pages. Modified the main loop
 so that the BAND+/BAND- buttons now auto-repeat. A.Ryan - 5B4AIY - 10/FEB/2015
 Build 4
 Minor optimisation in service.c Fixed minor annoyance. When exiting the service module, the power meter would briefly
 display 0.6W, which would then quickly decay to zero. Moved peak_pwr to a global rather than an embedded static in
 function: disp_fwd_pwr(). Removed calibration and configuration page skip logic. A.Ryan - 5B4AIY - 11/FEB/2015
 Build 5
 Restored the automatic page skip of Serial Test On/Off or Polling Timer settings pages if inconsistent serial port
 mode selection in User Configuration menu. Restored Under/Over Voltage settings page skip in Calibration menu if
 their respective alarms are disabled. A.Ryan - 5B4AIY - 12/FEB/2015
 Build 6
 Another attempt at eliminating the spurious 0.6W display on exiting the service module. A.Ryan - 5B4AIY - 13/FEB/2015

 1.06c
 Build 1
 Added optimisation to the auto-increment and auto-decrement function. The speed at which the UP/DOWN buttons
 now auto-repeat is dependent upon which User Configuration page is selected. In the Calibration & Setup mode
 only page 5 has a slow repeat. A.Ryan - 5B4AIY - 15/FEB/2015
 Build 2
 Removed cfg_rpt[] array and substituted an integer constant. See comment for set_repeat_speed(), and comment
 for defined constant SPEED_ARRAY in juma-pa100.h A.Ryan - 5B4AIY - 16/FEB/2015
 Build 3
 Replaced multiple occurrences of button hold/release checks with wait_PWR_BAND_UP_rls() and wait_PWR_rls() which
 saved 39 bytes. Simplified the BAND+/BAND- button logic, saving a further 33 bytes. Simplified the page change
 and auto-repeat logic in service.c. A.Ryan - 5B4AIY - 23/FEB/2015
 Build 4
 Added function second_line(). This eliminates numerous lines of duplicate code. A.Ryan - 5B4AIY - 15/MAR/2015
 Build 5
 Replaced second_line() with more general display_line(), and re-located this function to lcd-trx2.c, replaced
 clear_lcd() with a macro. A.Ryan - 5B4AIY - 19/MAR/2015

 1.06d
 Build 1
 Fixed spurious parameter change that occurs if either the UP or DOWN button is held whilst the fast page select
 function is in operation in either the Calibration or User Configuration modes. A.Ryan - 5B4AIY - 20/MAR/2015
 Build 2
 Added splash screen enable/disable to service menu. Change VERY_LONG_PUSH from 1,500mS to 1,200mS to accord
 with Juma TRX-2. A.Ryan - 5B4AIY - 18/APR/2015
 Build 3
 Added firmware version and build number to System Configuration dump in serial_test.c A.Ryan - 5B4AIY - 19/APR/2015
 Build 4
 Minor optimisation. Removed the direction global and instead passed the parameters to cfg_page_change() and
 change_cfg_page() functions, as well as in the service module. A.Ryan - 5B4AIY - 28/MAY/2015
 Build 5
 Minor re-organisation of boot code. Optimised scaled_temperature(). Fixed a subtle display bug. When switching between
 the Celsius and Fahrenheit scales the graphic limit display was not the same despite the temperature limits being
 equivalent. This is now fixed. Changed the scale factors for the C/F scales. (See notes in juma-pa100.h) Minor
 optimisation of calc_swr() A.Ryan - 5B4AIY - 20/JUN/2015
 Build 6
 Fixed minor bug in service.c which could have allowed a spurious parameter change when switching pages in the fast
 page switch mode. Fixed minor bug in LCD Test in serial_test.c Added a default choice to set_chgen() in lcd-trx.c
 A.Ryan - 5B4AIY - 28/JUN/2015
 Build 7
 Optimised set_chgen() in lcd-trx2.c A.Ryan - 5B4AIY - 01/JUN/2015

 2.00a
 Build 1
 Completely changed the way the checksum is calculated and verified. Added crc_16(), crc_8(), and crc_1() to perform
 a 16-bit cyclic redundancy check. The previous simple addition and complement checksum was not particularly robust,
 and, whilst I have not had any reports of this method failing, it certainly could not detect many types of error.
 The 16-bit CRC on the other hand can detect quite subtle errors. Even so, since it can only generate 65,536 different
 values, it is clearly possible for even this method to fail, but the probability is very low. This version will cause
 a checksum error when first loaded, and the existing calibration and configuration data should be saved prior to loading
 this version. A.Ryan - 5B4AIY - 31/JUL/2015
 Build 2
 Optimised crc_1(). A.Ryan - 5B4AIY - 01/AUG/2015
 Build 3
 Added function get_one_zero() to simplify service and user configuration adjustment code. A.Ryan - 5B4AIY - 05/AUG/2015
 Build 4
 At the suggestion of Olaf, LA3RK, the SWR alarm is now only asserted if the PA State is OPERATE. This avoids nuisance
 SWR alarms in the STANDBY state when tuning an antenna using just the driver transceiver. A.Ryan - 5B4AIY - 09/SEP/2015
 Build 5
 Removed redundant 'old_swr' variable. In the previous firmware it took two main loop cycles for a SWR alarm to be
 registered, but repeated testing has not shown any real need for this, so in this firmware this double sampling has
 been eliminated. This required the maximum sample count to be increased from 8 to at least 16, giving a maximum alarm
 delay of about 80mS. A.Ryan - 5B4AIY - 23/SEP/2015
 Build 6
 Re-designed the Alarm Test and A-D Test modules in serial_test.c to use indexed function arrays rather than a SWITCH-CASE
 construct. Changed the alarm logic so that a SWR/O-C/TEMP/Hi-V/Lo-V alarm forces the amplifier to assume the STANDBY state,
 and turns the RF off. The Battery alarm (Low Voltage pre-limit) is a warning, it will not force the STANDBY state. Removed
 the alarm check logic from the Serial Test, as it is handled in the main loop. A.Ryan - 5B4AIY - 23/OCT/2015
 
 2.00b
 Build 1
 Changed cal.calval.alarm_flags so that it is now used as an alarm mask.
 NOTE: This requires the user to enter the Calibration & Setup menu immediately after loading this version and stepping to
 either the Over-Voltage or the Under-Voltage page and cycling the value from Off to On and back to the desired setting in
 order to set the correct mask values. A.Ryan - 5B4AIY - 24/OCT/2015
 Build 2
 Re-designed display_alarms() function eliminating redundant code and considerably simplifying it. A.Ryan - 5B4AIY - 25/OCT/2015
 Build 3
 Added a line of code to automatically update the alarm mask, cal.calval.alarm_flags, if the user fails to reset the mask
 as instructed on the Juma website. A.Ryan - 5B4AIY - 27/OCT/2015
 Build 4
 Changed the return from reading the calibration and configuration values to a flag, the actual CRC value is of little use
 in determining what has gone wrong in the event of an EEPROM checksum error. Modified the calc_swr() function to provide
 protection in the case of a really excessive SWR which might provoke a math trap error. A.Ryan - 5B4AIY - 06/JAN/2016

 3.00a
 Build 1
 Added remote control capability for Uwe Gartmann, HB9FZG. This build does not time-out if no message is received from the
 remote terminal. See remote() for details. A.Ryan - 5B4AIY - 05/JUN/2016
 Build 2
 Implemented the remote time-out. If the amplifier mode is REMOTE and a message has not been received for approximately 5 seconds,
 the timer will expire, and the state will be set to STANDBY. A.Ryan - 05/JUN/2016
 Build 3
 Changed flags so that no information printouts occur if the serial port mode is REMOTE. Removed redundant variable in
 remote(). A.Ryan - 5B4AIY - 05/JUN/2016
 Build 4
 Re-designed the remote() function for a more efficient operation. Moved drain_current from being a local static double
 in disp_id() to a global float, this avoids duplication which occurred in remote(). A.Ryan - 06/JUN/2016
 Build 5
 Considerable simplification of the Tx/Rx logic. A.Ryan - 5B4AIY - 07/JUN/2016

 3.00b
 Build 1
 Re-designed measurement system so that now all the A-D channels are converted sequentially, and every time through the
 main loop. This slowed the loop cycle time down to 6.3mSec. An investigation of the sampling delay time in convert_adc12()
 showed that it was much too long. Even 1uS was long enough. The final value was set at 100uS giving a loop cycle time
 of about 3.2mSec. This re-design resulted in a number of changes in the Service module, as well as other areas of the code.
 As a consequence, the remote status message will now show all the parameters even when the display page for that value is
 not selected, which was a bug with the previous version. A.Ryan - 5B4AIY - 08/JUN/2016
 Build 2
 Minor optimisation, relocated drain_current from being a global to a local static in dis_id(). A.Ryan - 5B4AIY - 09/JUN/2016
 Build 3
 Initialised pa_temp by getting the current temperature value from the A-D converter rather than simply starting at a fixed
 value. Optimised remote() by removing redundant variables. Changed the way the remote mode command time-out occurs. Previously
 it counted the main loop cycle time, but as this varies depending upon which page is displayed, I added a timer to
 timers_pwm.c to precisely time the event. This timer still permits local operation without a time-out in the remote mode.
 Changed the temperature display from a 50-sample to a 100-sample running average display. A.Ryan - 5B4AIY - 10/JUN/2016
 Build 4
 Removed redundant variable and redundant DO-WHILE loop in remote(). Default serial port mode is now off. A.Ryan - 5B4AIY - 12/JUN/2016
 Build 5
 Changed the maximum SWR from 10.0:1 to 9.9:1 so that there is not an error in the remote display. This involved also changing
 the maximum SWR to 9:1 in the alarms section. A.Ryan - 5B4AIY - 14/JUN/2016
 Build 6
 Changed delay in adc12.c from 1uS to 50uS. This increases the margins, although I have not had any reports of A-D converter
 errors as a result of the initial setting. A.Ryan - 5B4AIY - 07/JUL/2016
 Build 7
 Re-organised serial test suite. All tests are now in alphabetic order, and both lower and upper case characters are acceptable.
 A.Ryan - 5B4AIY - 14/JUL/2016
 
 3.00c
 Build 1
 Added the feature that polling can be enabled in the remote mode. If enabled, then when the remote mode is selected, status
 messages will be automatically transmitted at each expiry of the polling interval timer. A.Ryan - 5B4AIY - 18/AUG/2016
 Build 2
 Minor optimisation of crc_8() function. A.Ryan - 5B4AIY - 27/NOV/2016
 Build 3
 Minor optimisation in change_cal_page() in service.c module. Added T_Char[] character array for F/C temperature indication,
 re-ordered the User Configuration menu so that the temperature selection is immediately followed by the limits settings,
 moving the fan mode to accommodate. Added pa[] and ba[] arrays, used in send_status(). Fixed minor bug in remote(). If the 'B'
 command was not followed by a digit, an illegal band prompt would occur. Introduced more robust error checking in remote() to
 ensure that invalid bands and gains cannot be set. A.Ryan - 5B4AIY - 26/MAR/2017
 Build 4
 Minor enhancement to Serial Test Suite. Test 'C' now displays the 'Test Terminated' message. Corrected some minor comment errors.
 A.Ryan - 5B4AIY - 24/APR/2017
 Build 5
 Minor change to how max_page is set. Minor optimisation of serial_test(). Minor change to the polling interval prompt.
 A.Ryan - 5B4AIY - 15/MAY/2017
 Build 6
 Replaced long variable names with shorter macros. (See juma-pa100.h for details) A.Ryan - 25/AUG/2017
 Build 7
 Removed some redundant variables, restructured the fan speed code, fixed bug in service mode that stopped the printing of the
 checksum values. A.Ryan - 5B4AIY - 27/APR/2020
 Build 8
 Changed save_setting() to a procedure since the return flag is now never used. Removed all the code that printed the number of words
 and the CRC when saving either the calibration or configuration settings, of little use. Left the state of the serial port unchanged
 in the Service Mode. Fixed spurious beep when returning from Service Mode. A.Ryan - 5B4AIY - 29/APR/2020
 
 3.00d
 Build 1
 This version adds a new feature - Yaesu 5-Byte Binary automatic band setting. This allows the Yaesu FT-817/818 transceivers to
 automatically update the amplifier's band filter setting. It also closes a 'window of opportunity' that existed that could have allowed
 the wrong output filter to be selected. Now, even if polling is disabled, if one of the CAT modes is selected then whenever a manual
 band is selected it also forces a frequency query command to be sent to the connected transceiver and the response will over-ride an
 incorrect manual band setting. Equally, if the mode is changed from MANUAL to AUTO or vice-versa, this will also force a query command
 to be sent. Only if you deliberately set the mode to an unused selection, or have no data link connection to the transceiver can you now
 force an incorrect band selection - in which case, that is your fault! A.Ryan - 5B4AIY - 01/MAY/2020
 Build 2
 Optimised both the KX3 and Yaesu serial port functions. Modified the names of the timeouts. Now, the old cmd_timout is renamed to
 rmt_timeout, and the old kx3_timeout becomes cmd_timeout. The first is used with the REMOTE function, the second is a general timer
 for the serial port data that helps to recognise a loss of communications event. Added the cmd_timer to the Yaesu serial port function.
 The Yaesu CAT protocol warns that there can be up to 200mS between bytes. The flaw in the binary protocol is that there is no way of
 reliably determining if the response to a command is correct, you simply have to collect 5 bytes and assume that everything is OK.
 I have attempted to protect against the possibility of an incorrect response or a communications error by implementing a 200mS timer
 which is reset every time a character is received. If a character is received and there is more than 200mS before the next byte is
 received, then I assume that a communications problem occurred, the serial buffer is cleared, and the process re-started. This cannot
 take account of all errors, it is still possible for a faulty message to indicate the wrong frequency, but as there is no control
 byte to indicate what sort of message this is, then I have no way of absolutely preventing a momentary frequency error. Altered the
 order of the Auto Frequency Sense modes to correspond with the order in the Juma TRX-2. A.Ryan - 5B4AIY - 02/MAY/2020
 Build 3
 Most transceivers do not automatically inform the amplifier if the band has been changed. In order to take account of band changes
 the polling interval timer has to be enabled. In this version, if either the Yaesu or the KX3 mode is selected, and the polling timer
 is disabled, then it is set to the default interval of 1 second. If the Juma TRX-2 mode is selected, then the timer is automatically
 disabled, as this transceiver automatically updates the amplifier on any band change. However, if the user wishes to enable polling
 then this can still be over-ridden. If when selecting either the Yaseu of KX3 modes the polling timer is already set, then its value
 is left undisturbed. Minor changes to the low-pass filter selection frequencies to coincide with those from the Juma TRX-2.
 Modified *select_auto_band[]() so that it no longer required passing it the band select index. This is already contained in
 Band_Select_Mode and thus is globally visible anyway. A.Ryan - 5B4AIY - 03/MAY/2020
 Build 4
 Minor update to the serial test routine. The Low Voltage and Pre-Limit messages are clarified. Tabs replaced with spaces as some terminal
 programs do not recognise the TAB character. A.Ryan - 5B4AIY - 05/MAY/2020

 4.00a	THIS WILL CAUSE A CHECKSUM ERROR ON INITIAL LOAD!
 Build 1
 Introduced an additional word in the EEPROM to contain the now adjustable Pre-Limit trip point.
 Build 2
 Automatic updating of the Pre-Limit trip point setting if there is a change to the Under-Voltage trip point. Minor updating of the standard
 defaults for the EEPROM. The new default for the Pre-Limit trip is the Under-Voltage trip + 200mV. The new adjustable Pre-Limit is adjustable
 from +100mV to +800mV in relation to the Under-Voltage trip setting. Fixed a serial test bug whereby when the Over Voltage or Under Voltage
 alarms were disabled, their alarm settings were still displayed. A.Ryan - 5B4AIY - 09/MAY/2020
 Build 3
 Fixed some inconsistencies in the Manual/Auto frequency selection over-ride logic. In the previous firmware, if the mode was MANUAL, then if
 an out-of-band frequency was detected, the amplifier reverted to the last known manual band selection. If the KX3 was being used and the 6m
 band accidentally selected, then this could cause a serious problem. In this version, any out-of-band or unknown frequency will cause the ?
 symbol to be displayed and the 10m LPF selected, and this will also inhibit entering the transmit state. A.Ryan - 5B4AIY - 10/MAY/2020
 Build 4
 This version adds a Manual Band selection mode. With all the protection now built in to avoid inadvertent incorrect band selection, it was
 difficult to effect a truly manual band selection. To be honest, with all the auto options I could not see why you would even need a true
 manual band selection, but just in case there is a QRP transceiver out there that cannot utilise any of the automatic modes, then there is
 a last resort, Manual Band selection. In this mode, there are no protections, if you select the wrong band and damage the filters, then that
 is your fault. A.Ryan - 5B4AIY - 11/MAY/2020
 
 4.01a
 Build 1
 In the interests of defensive programming, fixed a possible 'window of opportunity' in the poll_cmd[] message array and xmit_cmd() function.
 In order to transmit the Yaesu 5-byte binary commands it was necessary to uses a counted message string rather than the usual ASCIIZ string.
 As a result, the first byte of each message contained a value indicating the number of characters to transmit, and the xmit_cmd() function
 used a DO - WHILE loop. It was possible therefore if the index into the poll_cmd[] was beyond the end of the array into the spare entries that
 instead of the xmit_cmd() function safely aborting it would continue to transmit characters until the character count had reached zero.
 In order to avoid this, even though in more than 10 years this has never occurred to my knowledge, I decided the change the xmit_cmd() function
 to use a simple WHILE loop, and decrement the character counter after the test. By inserting a NULL byte as the character count in the polling
 message the xmit_cmd() function will now safely abort if there is ever an index into this spare message section of the array. Of course, if the
 index is wildly in error then the xmit_cmd() function will send up to 256 bytes of garbage, but this would imply a really gross error had
 occurred, and in all probability this would be the least of your troubles! A.Ryan - 5B4AIY - 20/DEC/2021
 Build 2
 Minor update of some definitions. No change to the HEX file checksum. A.Ryan - 5B4AIY - 09/SEP/2022
 Build 3
 This fixes a subtle 'bug' in the display screen logic. If the F-Sense automatic band select mode was selected, then there is an additional display
 screen available which shows the injected frequency to a resolution of 1kHz as well as the current O/P power. If, whilst this screen is being
 displayed the automatic band select mode is changed to something other than F-Sense, when this new configuration is saved the additional frequency
 display screen is still displayed. It vanishes once the display page is changed. This update fixes this anomaly and now if the configuration is
 changed and saved the starting page is shown. A.Ryan - 5B4AIY - 26/AUG/2023
 ---------------------------------- DL4JC Modifications ----------------------------------
 v4.03 - DL4JC - 04/OCT/2026. Based on v4.01a Build 3. From this version on, every release has its own version number,
 which is shown on the start-up screen; there is no separate build number any more. (The test builds before this release
 were called v4.02a Build 1 - 5-DL4JC.) The original configuration and calibration blocks are unchanged, so there is no
 checksum error on loading, and the original firmware can be loaded again without losing the calibration.
 TX protection:
 - TX_ON is forced off in the User Configuration mode. Previously it stayed in the state it was in when the mode was entered.
 - The trap handlers now force TX_ON off and run the fan at high speed before anything else. (See safe_state() in traps.c)
 - New function tx_guard() in the 1mS interrupt (timers_pwm.c). It forces TX_ON off if KEY is inactive for 2mS, if there is an
   active alarm, if the SWR exceeds the trip limit, or if the main loop has not run for 2 seconds. Previously all protection was
   in the main loop, and was suspended whenever the main loop was blocked, e.g. waiting for a button release or a save prompt.
 - All A-D conversions are now made in the 1mS interrupt (adc12.c), which allows the SWR test to run there. convert_adc12() now
   returns the latest value. The tone generator (TMR2) interrupt priority has been raised above that of TMR3.
 Filter protection:
 - A band change now turns RF off first, the filter relays are switched once the PA relays have released, and TX is
   held off until the new relays have settled (RELAY_SETTLE). Previously the relays were switched under full power.
 - The 1mS interrupt compares the measured input frequency with the selected low-pass filter. If the frequency is above
   the filter for 2mS, RF is turned off until the band has been corrected. (See tx_guard() in timers_pwm.c) This fixes
   the occasional O/C alarm in the F-Sense mode on the first transmission after a band change, e.g. 40m to 20m, when
   the 20m signal was amplified through the 40m filter until the new band had been measured. There is no additional
   delay when the band is unchanged, so full QSK still works.
 - F-Sense mode only: if, within the first 200mS of a transmission, the input frequency is below the selected filter for
   3mS, RF is also turned off until the band has been measured. This avoids poorly suppressed harmonics on the first
   transmission after a band change from a higher to a lower band, e.g. 80m through the 20m filter. Later in the
   transmission this test is not made, as SSB speech can produce low miscounts (see the notes for v1.05p Build 3).
 - New User Configuration page "F-Sense QSK", only shown in the F-Sense mode. Off (default): TX is only enabled once the
   input frequency has been measured in the current transmission, approx. 20-40mS without the PA at the start of each
   transmission. On: TX immediately, for full QSK.
 Band select:
 - KX2/KX3 (ASCII) mode: frequencies above 30MHz are limited before the conversion to unsigned int. Previously e.g. 144MHz
   wrapped to 12.9MHz and selected the 20m filter with TX enabled.
 - Juma TRX-2 mode: an invalid band now sets NOT_KNOWN (TX inhibited) instead of 10m.
 - New band select mode 6, Xiegu, using the Xiegu ACC port band voltages (230mV steps). (See get_xiegu_band()) It is
   stored as F-Sense in the original configuration block, with Xiegu in the extension block. If the original firmware is
   loaded again, it uses F-Sense instead of an unknown mode value.
 - New band select mode 7, HR50: the PA-100D answers the serial commands of the HobbyPCB Hardrock-50 amplifier (FA, IF,
   HRBN, HRMD, HRRX, HRTP, HRVT, HRAT, HRBR, HRKX, HRTM), so that programs and transceivers with HR50 support can select
   the band and the Operate/Standby state, and read the status. (See serial_hr50()) The mode is stored as KX2/KX3 in the
   original configuration block, with HR50 in the extension block. The original firmware then uses the FA frequency data.
 Serial interface:
 - The receive interrupt reads the whole UART FIFO, and an overrun is cleared. Previously reception could stop for good at
   higher baud rates (e.g. 115200) after a short burst. (See uart.c)
 - The remote status reply is formatted completely and sent in one piece. Previously gaps between the fields could split
   the line. The format is unchanged.
 - Remote mode: the known limitation that the command time-out never expires when polling is enabled is now documented in remote().
 EEPROM:
 - New EEPROM extension block at 0xF100 with its own checksum for settings added by DL4JC (see pa100_eeprom.h). It is
   saved with the user configuration and with the service settings.
 Other:
 - Service mode: an alarm now really exits the service mode. Previously the service loop continued without handling any buttons.
 - New service menu page "Beep Tone" after "Beep Len": JUMA (default, unchanged) or RS-928. The RS-928 clone has a buzzer
   that only sounds clean between about 2300 and 2800Hz, so the JUMA tones of 600 - 2000Hz sound harsh on it. RS-928
   moves the tones into this range, in the same order (see beep() in timers_pwm.c). The alarm beep uses the new constant
   HZ2000 instead of 1843.
 - Start-up screen: "JUMA PA100 v4.03" / "OH2NLT/7SV DL4JC".
 - Builds with MPLAB XC16 (build-xc16.sh). The program memory ends below the Ingenia boot loader (juma-trx2.gld).
 v4.03 - DL4JC - 04/OCT/2026 (see above)
 v4.04 - DL4JC - 04/OCT/2026
 - F-Sense: a modulated signal (two-tone, noise, SSB) is counted too low by the input shaper, e.g. 14MHz two-tone as
   approx. 11MHz in every sample. At the start of a transmission this selected a lower band, i.e. a filter below the
   transmitted frequency (also in v4.01a). A lower band is now only selected from a clean carrier, whose samples lie
   within approx. 3% of each other (measured: TUNE < 0.1%, two-tone and noise > 15%). A higher band is still selected
   from any signal. (See eval_band())
 - F-Sense: the low frequency test at the start of a transmission ends with the first evaluated sample set. With a
   modulated signal it turned RF off and on with every sample set for 200mS, and the relays chattered.
 - Serial test 'G' (F-Sense test) also shows the lowest and highest sample of each set, and clean/mod.
 v4.05 - DL4JC - 04/OCT/2026 (code review)
 - The timing test pin was toggled with 'MAIN_TEST = !MAIN_TEST', a read-modify-write of the whole LATB. If tx_guard()
   turned TX_ON (LATB4) off in the interrupt between the read and the write, TX_ON was switched on again for up to 1mS.
   Now a single btg instruction.
 - set_relays() only waited for the PA relays to release if TX_ON was still on at the band change. If tx_guard() had
   just turned it off (filter mismatch, KEY released), the filter relays were switched while the PA relays were still
   releasing. The interrupt now counts the time since TX_ON was on (tx_off_ms), and the filter relays are switched
   RELAY_SETTLE mS after that at the earliest, with TX held off meanwhile.
 - F-Sense: a modulated signal may also select a lower band if its highest sample is below 2/3 of the lowest frequency
   of the current filter, e.g. 20m to 40m with SSB. The debug output shows 'far' in that case.
 - save_defval() no longer changes Band_Select_Mode during the EEPROM write; the substitute value for the original
   firmware is only used in the stored copy.
 - The extension block stores the checksum of the configuration block. If the configuration has been saved since,
   e.g. by the original firmware, the Xiegu/HR50 mode in the extension block is out of date and is not used.
 - Service menu: if saving is cancelled, the extension block (Beep Tone) is read back as well.
 - KX2/KX3 mode: the time-out for an incomplete FA message never applied, as the timer was set again before the test
   (also in v4.01a). A stale partial message is now dumped before a new character is added.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "juma-pa100.h"						// JUMA-TRX2 hardware specific definitions
#include "pa100_eeprom.h"					// EEPROM storage structure
#include "DataEEPROM.h"						// Original MicroChip Read/Erase/Write EEPROM definitions

#define REMOTE_TEST		FALSE				// Used to verify the operation of the remote() function.
#define LOOP_TIME		FALSE				// Used to measure loop cycle time.

#ifdef __XC16__
/*
 Configuration bits for the MPLAB XC16 compiler, identical to the C30 settings below. XC16 names the 2V brown out
 setting (BORV = 11) NONE. The settings not listed (code protection, boot and secure segments) remain unprogrammed,
 i.e. off, as with CODE_PROT_OFF.
*/
#pragma config FOSFPR = XT_PLL4							// 7,3728MHz XT, 29,4912MHz clock = 7,3728MHz cycle. FCKSMEN is left
														// unprogrammed (11), clock switching and monitor disabled, as with C30.
#pragma config WDT = WDT_OFF								// Turn off the Watch-Dog Timer.
#pragma config FPWRT = PWRT_64, BODENV = NONE, BOREN = PBOR_ON, MCLRE = MCLR_EN	// Enable MCLR, power-on timer, brown out 2V
#else
_FOSC(CSW_FSCM_OFF & XT_PLL4);				// 7,3728MHz XT, 29,4912MHz clock = 7,3728MHz cycle
//_FOSC(CSW_FSCM_OFF & XT_PLL8);			// 3,68640MHz XT, 29,4912MHz clock = 7,3728MHz cycle
_FWDT(WDT_OFF);                				// Turn off the Watch-Dog Timer.
_FBORPOR(MCLR_EN & PWRT_64 & BORV_20);   	// Enable MCLR, power-on timer, brown out 2V
//_FGS(GEN_PROT);            				// Disable Code Protection, old compiler
 _FBS(CODE_PROT_OFF);
 _FSS(CODE_PROT_OFF);
 _FGS(CODE_PROT_OFF);
#endif

// External Functions
extern void us_delay(unsigned int);			// Delay routines
extern void ms_delay(unsigned int);

extern void init_timers_pwm(void);			// Timers & PWM DAC
extern void beep(int, int);					// Generate beep, period, length
extern int encoder_get(void);				// Get simulated encoder value
extern void init_adc12(void);
extern int convert_adc12(unsigned int);		// ADC

// LCD functions
extern void initlcd(void);
extern void lcd_cmd(unsigned char);
extern void lcd_putst(register const char *);
extern void display_line(int, const char *);
extern void display_screen(const char *, const char *);
extern void set_ch_bits(char, int);
extern void set_chgen(int);
extern void draw_s_meter(int);
extern void disp_meter(double, int);		// New version - 5B4AIY - 27/JAN/2014

extern void InitUSART1(void);				// Terminal functions - See file uart.c
extern void SetUSART1baud(int);				// Baud rate select
extern void ClearUSART1queue(void);			// Clear buffer
extern void putch(char);
extern unsigned char kbhit(void);
extern unsigned char getch(void);
extern unsigned char getche(void);

//extern void init_spi1(void);				// SPI, main board 74HC595 I/O latch (Not used)

extern void service(int);					// Service functions
extern void serial_test(void);				// Serial I/O
extern void dump_eeprom(void);				// Dump EEPROM contents

extern void serial_pa100(void);				// PA100 protocol engine
extern void clear_buffer(void);				// Clear Juma PA-100D receive buffer
extern void get_yaesu(void);				// Get Yaesu 5-Byte binary frequency data

// Frequency Counter Test
extern volatile int band_bins[];						// Found samples
extern volatile int freq_sample_ctr;					// Sample counter for F-SENSE mode
extern volatile unsigned int fs_min, fs_max;	// Spread of the F-Sense sample set, see eval_band()
extern volatile int fsense_evaluated;			// F-Sense sample set evaluated in this transmission, see tx_guard()
extern volatile unsigned int tx_off_ms;			// mS since TX_ON was last on, see tx_guard()
extern unsigned int filter_lower(int band);		// Lowest input frequency (kHz) of the filter for a band, see timers_pwm.c
extern volatile int freq_ctr;						// Frequency counter averaging counter

// External Data
extern volatile unsigned int rmt_timeout;			// Remote Mode command timer
extern volatile unsigned int button_timer;			// Long push timer, count from set value to zero, global visibility
extern volatile unsigned int polling_timer;			// Band query and remote control and monitoring polling timer
extern volatile unsigned int cmd_timeout;			// Elecraft KX-3 Time-out (Defined in timers_pwm.c)
extern unsigned char cmd_buf[];				// Defined in serial_pa100.c
extern int cmd_buf_idx;						// Defined in serial_pa100.c
extern int blink;							// Alarm blink flag
extern volatile int decay_counter;					// Power meter slow decay
extern int poll_resp_rec;					// Poll response (freq set) received from TRX-2/KX3, 1 = received
extern volatile int enc;
extern volatile unsigned int adc_raw[];		// Latest A-D values, updated every 1mS in adc12.c
extern volatile unsigned int main_heartbeat;	// Main loop watchdog, see tx_guard() in timers_pwm.c
extern volatile int isr_swr_trip;			// SWR trip detected in the 1mS interrupt, see tx_guard() in timers_pwm.c
extern volatile int filter_mismatch;		// Input frequency above the selected filter, see tx_guard() in timers_pwm.c
extern volatile unsigned int relay_settle;	// Relay settling timer, mS, decremented in _T3Interrupt()

// Local Data
void (*rs232_mode)(void);					// Pointer to function taking void and returning void

const char *prompt_msg;

// Baud Rate divisors for 29,4912 MHz oscillator =  7.3728 MHz FCY 
/*
	Actual baud rate divisors
	Divisor = ((FCY / (Baud rate * 16)) - 1
[0] 1200	383.0
[1] 2400	191.0
[2] 4800	95.0
[3] 9600	47.0
[4] 19200	23.0
[5] 38400	11.0
[6] 57600	7.0
[7] 115200	3.0
*/
const int br_txt[] = {					// Baud Rate messages, indexed by: eeprom.defval.br
							12,			// 1200 Baud
							24,			// 2400 Baud
							48,			// 4800 Baud
							96,			// 9600 Baud
							192,		// 19200 Baud
							384,		// 38400 Baud
							576,		// 57600 Baud
							1152		// 115200 Baud
							};

const char *fan_spd[] = {				// Indexed by: Fan_Speed
							"Normal",	// 0
							"Low",		// 1
							"Medium",	// 2
							"High"		// 3
							};

const char *c_or_f[] = {				// Indexed by: Temp_Scale
							"Fahrenheit",	// 0
							"Celsius"		// 1
							};

const char *f_sense[] = {				// Frequency Sense Mode Indexed by: Band_Select_Mode
							"Yaesu CAT",// 0 - Yaesu Band Select Voltage levels
							"KX2/KX3",	// 1 - ASCII, Query = FA; Usable with any ASCII protocol transceiver
							"Juma-TRX2",// 2 - Juma TRX-2 special command set
							"F-Sense",	// 3 - Automatic frequency measurement
							"FT817/818",// 4 - Yaesu 5-Byte Binary
							"Manual",	// 5 - Manual band selection - NO PROTECTION!
							"Xiegu",	// 6 - Xiegu ACC port band voltage
							"HR50"		// 7 - Hardrock-50 serial protocol
							};

const char *auto_man[] = {				// Indexed by: Auto_Manual
							" M ",
							" A "
							};

const char *gain[] = {					// Indexed by: RF_Gain[Current_Band], and eeprom.defval.band
							"G1",		// 0
							"G2",		// 1
							"G3",		// 2
							"G4"		// 3
							};

const char *band_units[] = {			// Indexed by: Band_Units
							"MHz",
							"Metres"
							};

const char *on_off[] = {
							"Off",
							"On"
							};

const char *amp_state[] = {				// Indexed by: pa_state in serial_test.c module.
							"Standby",
							"Operate"
							};

const char *band_select[] = {			// Indexed by: Auto_Manual in serial_test.c module.
							"Manual",
							"Auto",
							};

// Band selector display texts
const char *mb_txt[] = {				// Indexed by: Current_Band
							"  ?!  ",	// Band = 0 	OUT_OF_BAND
							"  160m",	// Band = 1
							"   80m",	// Band = 2
							"   40m",	// Band = 3
							"   30m",	// Band = 4
							"   20m",	// Band = 5
							"   17m",	// Band = 6
							"   15m",	// Band = 7
							"   12m",	// Band = 8
							"   10m",	// Band = 9		MAX_BAND
							"   ?  "	// Band = 10	NOT_KNOWN
							};

const char *bs_txt[] = {				// Indexed by: Current_Band
							"  ?!  ",	// Band = 0 	OUT_OF_BAND
							"1.8MHz",	// Band = 1
							"3.5MHz",	// Band = 2
							"  7MHz",	// Band = 3
							" 10MHz",	// Band = 4
							" 14MHz",	// Band = 5
							" 18MHz",	// Band = 6
							" 21MHz",	// Band = 7
							" 24MHz",	// Band = 8
							" 28MHz",	// Band = 9		MAX_BAND
							"   ?  "	// Band = 10 	NOT_KNOWN
							};

const char *save_settings_prompt[] = {	// Indexed by: prompt variable passed to save_settings() function.
							" Save Settings? ",
							" Reset Defaults?"
							};

const char *start_page_select[] =	{	// Indexed by: Start_Page in cfg_14()
							"O/P Power",
							"SWR",
							"Voltage",
							"Current",
							"Temperature"
							};

const char *page_prompt[] = {					// Indexed by: sub_page1 (Line 1)
							"Auto Band Detect",	// 0
							"Serial Speed",		// 1
							"Serial Port",		// 2
							"Polling Interval",	// 3
							"LCD Display",		// 4
							"LCD Display",		// 5
							"SWR Trip",			// 6
							"Fan Control",		// 7
							"Temperature",		// 8
							"Overtemp",			// 9
							"Fan Cut-In Temp",	// 10
							"Band Display",		// 11
							"Graphical Limits",	// 12
							"Graphic Display",	// 13
							"RF Power Meter",	// 14
							"Start-Up Display",	// 15
							"F-Sense QSK"		// 16
							};

const char *poll_cmd[] = 	{	// Polling Messages. Indexed by: Band_Select_Mode. First character is byte count.
							"\005\000\000\000\000\003",	// 0 - Yaesu 5-byte binary
							"\003FA;",					// 1 - ASCII (KX2/KX3/Kenwood/Yaesu)
							"\005=?B\n\r",				// 2 - Juma TRX-2
							"\000",						// 3 - Dummy Value for possible Icom protocol
							"\000"						// 4 - Dummy Value (Spare)
							};

const char *graph_type[] = {			// Indexed by: Scale_Type
							"Original",
							"Large",
							"Small"
							};
const char *CS_Msg[] = {				// Indexed by: flag in save_settings()
							"   Cancelled!   ",
							"     Saved!     "
						};

const char *pwr_mtr[] = {				// Indexed by: Power_Units
							"Watts",
							"dBm"
							};

const char *pwr_mtr_msg[] =	{			// Indexed by: Power_Units
							" ---.-W",
							"--.-dBm"
							};

const char *checksum_msg[] = {
							"OK",
							"Error!"
							};

const char *rs232_mode_msg[] = {		// Indexed by: Serial_Test_Mode
							"Off",
							"Remote",
							"Test"
							};

const char temperature_fmt[] = {"%5s:%8i\337%c"};	// Octal 337 = 0xDF = Degree Symbol
const char limit[] = {"Limit"};
const char start[] = {"Start"};
const char units[] = {"Units:%10s"};
const char Yes_No[] = {"PWR:No BAND+:Yes"};
const char firmware[] = {"\n\r%sD Firmware: %s Date: %s\n\r"};
const char copyright[] = {"Copyright: Juha Niinikoski - OH2NLT & Matti Hohtola - OH7SV\n\r"};
const char additional_features[] = {"(Additional features and modifications - Adrian Ryan - 5B4AIY)\n\r"};
const char dl4jc_features[] = {"(Modified build - DL4JC)\n\r"};
const char Rmt_Pwr_Off[] = {"Remote Power Off"};
const char Data_Saved[] = {"   Data Saved"};

// Standard Display Prompts
const char Hello[] = {"%10s%6s"};
const char SWR_Alarm[] = {" SWR "};
const char CURR_Alarm[] = {" O/C "};
const char TEMP_Alarm[] = {" TEMP"};
const char HI_V_Alarm[] = {" Hi-V"};
const char LO_Batt_Alarm[] = {" Batt"};
const char LO_V_Alarm[] = {" Lo-V"};
const char RS232_Test_Msg[] = {"RS-232 Echo Test"};
const char Awaiting_Data[] = {"Awaiting data..."};
const char Press_PWR_To_Exit[] = {"\n\rRS-232 Test\n\rPress PWR button to exit...\n\r"};
const char RX_Data[] = {"RX Data: %c  0x%2.2X"};
const char RS232_Test_Terminated[] = {"\n\rRS-232 test terminated\n\r"};
const char _2I[] = {"%2i  "};
const char New_Line[] = {"\n\r"};
const char cfg_0_Msg[] = {"Select:%9s"};
const char cfg_1_Msg[] = {"Baud Rate:%4i00"};
const char cfg_2_Msg[] = {"Mode:%11s"};
const char cfg_3_Msg1[] = {"Timer:%6i Sec"};
const char cfg_3_Msg2[] = {"Timer:  Disabled"};
const char cfg_4_Msg[] = {"Backlight:%6i"};
const char cfg_5_Msg[] = {"Contrast:%7i"};
const char cfg_6_Msg[] = {"Limit:%8.1f:1"};
const char cfg_12_Msg[] = {"Display:%8s"};
const char cfg_13_Msg[] = {"Type:%11s"};
const char cfg_15_Msg[] = {"Page:%11s"};
const char SWR_NA[] = {"SWR--.-"};
const char EEPROM_Chksum[] = {"\n\rEEPROM Checksums\n\rCalibration  : %s\n\rConfiguration: %s\n\rReset Counter: %d\n\r"};
const char Chksum_Err[] = {"\n\rEEPROM Checksum Error! Re-loading Factory Defaults"};
const char Chksum_Err_Msg[] = {" Checksum Error "};
const char Loading_Defaults[] = {"Loading Defaults"};
const char Juma_PA100[] = {"JUMA PA100"};
const char OH2NLT_OH7SV[] = {"OH2NLT/7SV DL4JC"};	// OH2NLT, OH7SV and DL4JC (modified build). 16 characters, the display width.
const char Calibration_msg[] = {"  Calibration"};
const char Mode_msg[] = {"  Mode%7s"};
const char Display_Next[] = {" DISPLAY = Page "};
const char Oper_Save[] =    {"OPER = Save/Exit"};
const char System_Clk_Msg[] = {"System Clock: %li kHz\n\r"};
const char STBY_Msg[] = {" STBY"};
const char TX_Msg[] = {"  TX "};
const char Err_Msg[] = {" Err "};
const char OPER_Msg[] = {" OPER"};
const char _16s[] = {"%-16s"};
const char KX3_msg1[] = {"KX3 Msg Time-Out"};
const char KX3_msg2[] = {"Set to:   5 Secs"};
const char User_Msg[] = {"      User"};
const char Config_Msg[] = {" Configuration"};
const char freq_fmt[] = {" %6.3f  "};
const char five_spc[] = {"     "};
const char pa[] = {'S', 'O'};		// Used in send_status(). Indexed by: pa_state
const char ba[] = {'M', 'A'};		// Used in send_status(). Indexed by: Auto_Manual
const char T_Char[] = {'F', 'C'};	// Used in send_status(). Indexed by: Temp_Scale

// Timing
volatile int loop_ctr = 0;				// Alarm blink counter
int display_message = 0;		// Alarm message display flag
int beep_flag = FALSE;
int msg_time = 200;				// ASCII message time-out value, mS.
volatile int rep_dly = _FAST;			// UP/DOWN button repeat delay time

// Display Control
int	lcd_mode = NORMAL_DISPLAY_MODE;
int sub_page0 = 0;				// LCD display sub page for lcd_mode0 - Normal Display Start Page
int sub_page1 = 0;				// LCD display sub page for lcd_mode1 - User Configuration Start Page
int max_page = MAX_SUB_PAGE0;	// Set max_page to its normal value, it is increased by 1 if Auto Band Detect mode is set to F-SENSE
int adjust_flag = TRUE;			// Calibration & Configuration adjustment flag

char lcdpbuff[20];				// LCD print buffer. Maximum display length is 16 characters, this is a safety margin.

// ADC & Meter Variables
unsigned int AD_Values[6];		// A-D converter output values. (See definitions in juma-pa100.h for actual use of this array.)
double batt_avg;				// Average battery voltage, in mV
double batt_scale_factor;		// Used in the graphical battery voltage display
double id_cur = 0.0;			// PA Drain current raw value
double vswr = 0.0;				// SWR * 100 (SWR of 2.5:1 = 250)
double pa_temp;					// Heat-sink temperature

int scaled_pa_temp;				// Scaled to deg C or deg F depending upon setting.
int batt_pre_limit;				// Pre-limit warning threshold value

unsigned long pwr_scale_factor;	// Used to calculate maximum power of graphic power meter display.

volatile long input_freq = 0L;			// Used to display the input frequency in the F-SENSE mode.

long fwd_pwr = 0L;				// Relative forward power
long rev_pwr = 0L;				// Relative reverse power
long out_pwr = 0L;				// Scaled output power = (fwd_pwr - rev_pwr) * scale

int sample_counter = 1;			// Power measurement sample counter
int swr = 100;					// Calculated SWR * 100

// PA Status
volatile int pa_state = STANDBY;			// PA Standby/Operate Status, always start in the STANDBY state
volatile unsigned int alarms = 0;		// Alarm bits
int not_used = TRUE;

int key;						// TX request status - 1 = Tx, 0 = Rx
int last_man_band;				// Last selected manual band.
int fan_stop;					// Fan cut-off temperature.
int changed = FALSE;			// Change flag. Set if any user setting is changed.
int Current_Scale;				// Current temperature units, 0 = Fahrenheit, 1 = Celsius.
int svc_flag = FALSE;			// Flag for Service Mode (Used by service(), and disp_id())
int	fsense_tst = FALSE;			// Flag for F-sense test printouts - Can be Set/Cleared from Serial Test Suite.
int fan_speed = OFF;			// Fan speed, 0 = Off, 1 = Slow, 2 = Medium, 3 = Fast, used by remote() and fan_control().
char tx = 'R';					// TX/RX indicator, used in remote(), and send_status().

int fd_counter;					// Counts how many times factory defaults have been reloaded.
int fsense_confirmed = FALSE;	// F-Sense band measured in the current transmission (F-Sense QSK Off)
int last_key = FALSE;			// Previous KEY state, used to detect the start of a transmission

// Band Select Limits
const unsigned int band_limits[] = {
									LM1,	// 1,500kHz		Band = 0	Invalid if below this frequency
									LM2,	// 2,001kHz		Band = 1	If below this frequency, then 160m
									LM4,	// 4,001kHz		Band = 2	If below this frequency, then 80m
									LM7,	// 8,000kHz		Band = 3	If below this frequency, then 40m
									LM10,	// 12,000kHz	Band = 4	If below this frequency, then 30m
									LM14,	// 15,000kHz	Band = 5	If below this frequency, then 20m
									LM18,	// 19,000kHz	Band = 6	If below this frequency, then 17m
									LM21,	// 23,000kHz	Band = 7	If below this frequency, then 15m
									LM24,	// 26,000kHz	Band = 8	If below this frequency, then 12m
									LM28	// 30,001kHz	Band = 9	If below this frequency, then 10m
									};

// Yaesu 817 band select ADC #
// This table is with 100k || input, 817 internal impedance is about 6,5k
// Band when over this value                   1.8  3.5  7    10   14    18    21    24    28    50    144   430 MHz
//const unsigned int y817_band_limits[13] = {10, 122, 388, 655, 905, 1155, 1423, 1690, 1931, 2172, 2441, 2709, 2962}; 
/*
 This table is with no load (4M7 || input)
 The band is calculated by comparing the ADC value with this table. Starting from 10, if the voltage is greater
 than the entry, the index is the band number. If 10 is returned, then the band is unknown, and the 28MHz filters selected.
 if 0 is returned, then the band is out of range.
 The first column is the actual threshold voltage level.
 The second column is the Yaesu FT-817 band select voltage.
 The voltages in the fourth column represent the measured threshold at which this band was just selected.
*/
const unsigned int y817_band_limits[] = {		// Voltages calculated for a 5.000V reference.
										10,		// 0.012mV	0	Out-Of-Range
										135,	// 0.165mV	1	0.33V 1.8MHz	160m	171mV
										409,	// 0.499mV	2	0.67V 3.5MHz	75/80m	507mV
										683,	// 0.834mV	3	1.00V 7MHz		40m		850mV
										954,	// 1,164V	4	1.33V 10MHz		30m		1.176V
										1228,	// 1.499V	5	1.67V 14MHz		20m		1.513V
										1503,	// 1.835V	6	2.00V 18MHz		17m		1.851V
										1773,	// 2.164V	7	2.33V 21MHz		15m		2.181V
										2047,	// 2.499V	8	2.67V 24MHz		12m		2.522V
										2322,	// 2.835V	9	3.00V 28MHz		10m		2.856V
										2592	// 3.164V	10	Unknown Band			3.186V
										};

const double temp_cal_factor[] = {				// Indexed by: Temp_Scale
								T_MULTF,		// 0
								T_MULTC			// 1
								};

// User Configuration Data
union
	{
	struct defval defval;
	unsigned int storage[sizeof(struct defval) / 2];
	} eeprom;

// System Calibration Data
union
	{
	struct calval calval;
	unsigned int ee[sizeof(struct calval) / 2];
	} cal;

// DL4JC Extension Data, see pa100_eeprom.h
union
	{
	struct extval extval;
	unsigned int ee[sizeof(struct extval) / 2];
	} ext;

void set_ext_defaults(void);			// Extension block functions, defined after save_defval()
void save_extval(void);
unsigned int read_extval(void);
void load_ext_modes(void);

/*
 CRC16 is a simple 16-bit cyclic redundancy checksum that provides a more robust method of checking data than many other methods.
 The routine returns a 16-bit CRC. Obviously, the largest block of data that can be checked is limited to 65536 bytes. For
 successive calls for larger blocks, the contents of the checksum can be left, and this partial CRC can continue to be used.

 The algorithm used is similar to that in many EPROM programmers and also disc drives. In the following notes, Bit 0 is the
 Least Significant Bit, Bit 7 is the Most Significant Bit.

 The routine calculates the CRC starting with Bit 0, then Bit 7, Bit 6, .... Bit 1.

 It relies on the following steps:

 1.	XOR the current data bit with Bit 1 of the current checksum,
 2.	XOR the result of Step 1 with Bit 3 of the checksum,
 3.	XOR the result of Step 2 with Bit 5 of the checksum,
 4.	XOR the result of Step 3 with Bit 14 of the checksum,
 5.	Multiply the checksum by 2, discarding any carry from the MSB,
 6.	Add the result of Step 4 to the result of Step 5.

 Steps 1 through 6 are repeated for all subsequent bits in the data byte. The entire process is repeated for all subsequent bytes.
 Thus, if the bit values in the CRC register are:

  15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p

 and x is the current data bit value, y is the partial result, and ^ represents XOR, then the calculation for steps 1 through 4 is:

	y = ( ( ( ( x ^ o ) ^ m ) ^ k ) ^ b )

 After the left shift (multiply by 2) in Step 5, and the addition in step 6, the bits in the CRC register are:

  15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   y

 To give an example, suppose we wish to calculate the CRC for three bytes of data: C3, 55, AA. If we start with the CRC = 0, then
 the following steps will result (The HEX value is the result after the operation):

	DATA     BIT  VALUE	RESULT         CHECKSUM         HEX
	========================================================
	C3        0     1     1      0000 0000 0000 0001    0001
              7     1     1      0000 0000 0000 0011    0003
              6     1     0      0000 0000 0000 0110    0006
              5     0     1      0000 0000 0000 1101    000D
              4     0     1      0000 0000 0001 1011    001B
              3     0     0      0000 0000 0011 0110    0036
              2     0     0      0000 0000 0110 1100    006C
              1     1     1      0000 0000 1101 1001    00D9

	55        0     1     0      0000 0001 1011 0010    01B2
              7     0     0      0000 0011 0110 0100    0364
              6     1     0      0000 0110 1100 1000    06C8
              5     0     1      0000 1101 1001 0001    0D91
              4     1     1      0001 1011 0010 0011    1B23
              3     0     0      0011 0110 0100 0110    3646
              2     1     0      0110 1100 1000 1100    6C8C
              1     0     0      1101 1001 0001 1000    D918

	AA        0     0     0      1011 0010 0011 0000    B230
              7     1     0      0110 0100 0110 0000    6460
              6     0     0      1100 1000 1100 0000    C8C0
              5     1     0      1001 0001 1000 0000    9180
              4     0     0      0010 0011 0000 0000    2300
              3     1     1      0100 0110 0000 0001    4601
              2     0     1      1000 1100 0000 0011    8C03
              1     1     0      0001 1000 0000 0110    1806

 Final Checksum = 0x1806
*/

unsigned int crc_1(unsigned char data, unsigned int csum)
	{
	data &= 0x01;							// Extract data bit
	data ^= (csum >> 1);					// Step 1
	data ^= (csum >> 3);					// Step 2
	data ^= (csum >> 5);					// Step 3
	data ^= (csum >> 14);					// Step 4
	csum <<= 1;								// Step 5
	return csum + (data & 0x0001);			// Step 6
	}

unsigned int crc_8(unsigned char data, unsigned int csum)
	{
	int i = 7;

	csum = crc_1(data, csum);						// Bit 0

	do {
		csum = crc_1((data >> i), csum);			// Bits 7 through 1
		} while (--i);

	return csum;
	}

unsigned int crc_16(unsigned int data, unsigned int csum)
	{
	csum = crc_8((unsigned char)data, csum);		// Low byte
	return crc_8((unsigned char)(data >> 8), csum);	// Low byte plus High byte
	}

// Set factory defaults for EEPROM storage
void set_factory_defaults(void)
	{
	int i = 0;
// User settings
	do	{
		RF_Gain[i++] = 0;				// Minimum gain
		} while (i < 11);

	eeprom.defval.band = MAX_BAND;					// 28MHz default band
	eeprom.defval.contrast = DEFAULT_CONTRAST;		// 2000
	eeprom.defval.back_light = DEFAULT_BL;			// 300
	eeprom.defval.serial_test = OFF;				// Remote and Test Mode Off
	eeprom.defval.br = DEFAULT_BAUD_RATE;			// Initial Baud rate = 9600
	eeprom.defval.bsel_mode = FREQ_SENSE;			// 0 = Juma TRX-2, 1 = Elecraft KX3, 2 = Frequency Sense, 3 = FT-817
	eeprom.defval.poll_timer = POLL_TIMER;			// Yaesu CAT and Elecraft KX3 Polling Interval Time, 0 = Off, Default = 2 seconds
	eeprom.defval.swr_limit = SWR_LIMIT;			// SWR alarm trip point
	eeprom.defval.temp_units = CELSIUS;				// Temperature units, 1 = C, 0 = F
	eeprom.defval.fan_control = NORMAL_SPEED;		// Fan Speed Control 0 = Normal, 1 = Low, 2 = Medium, 3 = High
	eeprom.defval.temp_limit = TEMP_LIMITC;			// Temperature alarm
	eeprom.defval.fan_start = FAN_STARTC;			// Fan start point
	eeprom.defval.pa_state = DEFAULT_PWR;			// Not Used, replaced by global flag pa_state. Now used for start-up display page.
	eeprom.defval.auto_band = MANUAL_BAND;			// Band select type, 0 = Manual, 1 = Auto
	eeprom.defval.band_units = MHZ;					// 0 = MHz, 1 = Metres
	eeprom.defval.graph_limits = OFF;				// Graphical Limits Display 0 = Off, 1 = On
	eeprom.defval.display_type = ORIGINAL;			// Graphic Display 0 = Original, 1 = Large Scale, 2 = Small Scale
	eeprom.defval.rf_power = _WATT;					// RF Power Meter scaling, 0 = Watts, 1 = dBm

// Calibration values
	cal.calval.id_mult = ID_MULT;					// I = (ADC * ID_MULT) / 200000 
	cal.calval.batt_mult = BATT_MULT;				// V = (ADC * BATT_MULT) / 1000000
	cal.calval.fwd_pwr_mult = FWD_PWR_MULT;			// P = (((ADC * ADC) / 10) * FWD_PWR_MULT) / 10000000
	cal.calval.beep_len = DEFAULT_BEEP_LEN;			// 50mS
	cal.calval.samples = SAMPLE_MIN;				// Default sample count for O/P power and SWR measurements
	cal.calval.splash = ON;							// Display Splash Screen
	cal.calval.alarm_flags = ALL_ALARMS_ON;			// Under & Over-voltage alarms on
	cal.calval.overvoltage_trip = OVERVOLTAGE_TRIP;	// Nominally 14.5V
	cal.calval.undervoltage_trip = UNDERVOLTAGE_TRIP;// Nominally 11.0V
	cal.calval.pre_limit_trip = PRELIMIT_TRIP;		// Nominally 11.2V
	cal.calval.max_power = 100;						// Nominal 100W full-scale setting
	cal.calval.freq_cal = FREQ_CAL;					// Nominal frequency meter calibration factor (999985)
	cal.calval.lo_pwr_offset = LO_PWR_OFFSET;		// Low power range correction

// Extension values
	set_ext_defaults();

// Increment factory default reset counter
	ReadEE(EEPAGE, EE_FD_LOC, &fd_counter, WORD);
	fd_counter++;

// Save factory defaults reset counter
	EraseEE(EEPAGE, EE_FD_LOC, WORD);
	WriteEE(&fd_counter, EEPAGE, EE_FD_LOC, WORD);
	}

// Save defaults
/*
 Band select modes that the original firmware does not know are stored as a mode it knows, with the actual mode in the
 extension block. Xiegu is stored as F-Sense, which works with any transceiver and protects the filters. HR50 is stored
 as KX2/KX3, which uses the same FA frequency data. See load_ext_modes(). DL4JC
*/
void save_defval(void)
	{
	int i;
	int mode = Band_Select_Mode;
	unsigned int word;
	int mode_idx = ((char *)&eeprom.defval.bsel_mode - (char *)&eeprom.defval) / 2;	// Word index of bsel_mode

	ext.extval.bsel_ext = (mode == XIEGU) ? BSEL_EXT_XIEGU : (mode == HR50) ? BSEL_EXT_HR50 : BSEL_EXT_NONE;

	Cfg_Checksum = 0;

	for(i = 0; i < (sizeof(struct defval) / 2); i++)
		{
		word = eeprom.storage[i];
// The mode is substituted only in the stored copy. Previously Band_Select_Mode itself was changed during the whole write,
// and tx_guard() in the interrupt saw the wrong mode for approx. 150mS. DL4JC
		if(i == mode_idx) word = (mode == XIEGU) ? FREQ_SENSE : (mode == HR50) ? ELECRAFT_KX3 : mode;

		if(i == ((sizeof(struct defval) / 2) - 1)) word = Cfg_Checksum;	// The last word is the checksum
		else Cfg_Checksum = crc_16(word, Cfg_Checksum);

 		EraseEE(EEPAGE, ((2 * i) + EEDEF), WORD);
		WriteEE((int *)&word, EEPAGE, ((2 * i) + EEDEF), WORD);	// EEPROM Address 8 high bits, address + physical EEPROM start 16 low bits
		}

	ext.extval.cfg_csum = Cfg_Checksum;	// Ties bsel_ext to this configuration block, see load_ext_modes(),
	save_extval();						// and save the extension block, which holds further user settings. DL4JC
	}

// Extension block defaults
void set_ext_defaults(void)
	{
	int i = 0;

	do	{
		ext.ee[i] = 0;					// All settings off, spare words 0
		} while (++i < (sizeof(struct extval) / 2));

	ext.extval.magic = EXT_MAGIC;
	ext.extval.version = EXT_VERSION;
	}

// Save extension block
void save_extval(void)
	{
	int i;

	Ext_Checksum = 0;

	for(i = 0; i < (sizeof(struct extval) / 2); i++)
		{
		if(i < ((sizeof(struct extval) / 2) - 1)) Ext_Checksum = crc_16(ext.ee[i], Ext_Checksum);

		EraseEE(EEPAGE, ((2 * i) + EEEXT), WORD);
		WriteEE((int *)&(ext.ee[i]), EEPAGE, ((2 * i) + EEEXT), WORD);
		}
	}

// Read extension block. If it is missing or invalid, set the extension defaults. Returns TRUE if it was invalid.
unsigned int read_extval(void)
	{
	int i;
	unsigned int checksum = 0;

	for(i = 0; i < (sizeof(struct extval) / 2); i++)
		{
		ReadEE(EEPAGE, ((2 * i) + EEEXT), (int *)&(ext.ee[i]), WORD);

		if(i < ((sizeof(struct extval) / 2) - 1)) checksum = crc_16(ext.ee[i], checksum);
		}

	if((checksum != Ext_Checksum) || (ext.extval.magic != EXT_MAGIC) || (ext.extval.version != EXT_VERSION)
		|| (FSense_QSK < 0) || (FSense_QSK > 1) || (ext.extval.bsel_ext < BSEL_EXT_NONE) || (ext.extval.bsel_ext > BSEL_EXT_HR50)
		|| (Beep_Tone < 0) || (Beep_Tone > 1))
		{
		set_ext_defaults();
		return TRUE;
		}

	return FALSE;
	}

// Restore the actual band select mode after reading the configuration and extension blocks, see save_defval().
void load_ext_modes(void)
	{
// If the configuration block has been saved since, e.g. by the original firmware with F-Sense or KX2/KX3 chosen on
// purpose, bsel_ext is out of date and is not used. 0 = blocks saved by v4.03/v4.04, which did not store the checksum.
	if(ext.extval.cfg_csum && (ext.extval.cfg_csum != Cfg_Checksum))
		ext.extval.bsel_ext = BSEL_EXT_NONE;

	if((Band_Select_Mode == FREQ_SENSE) && (ext.extval.bsel_ext == BSEL_EXT_XIEGU))
		Band_Select_Mode = XIEGU;

	if((Band_Select_Mode == ELECRAFT_KX3) && (ext.extval.bsel_ext == BSEL_EXT_HR50))
		Band_Select_Mode = HR50;
	}

// Read defaults
unsigned int read_defval(void)
	{
	int i;
	unsigned int checksum = 0;

	for(i = 0; i < (sizeof(struct defval) / 2); i++)
		{
		ReadEE(EEPAGE, ((2 * i) + EEDEF), (int *)&(eeprom.storage[i]), WORD);	// EEPROM Address 8 high bits, address + physical EEPROM start 16 low bits

		if(i < ((sizeof(struct defval) / 2) - 1)) checksum = crc_16(eeprom.storage[i], checksum);
		}

// Read factory defaults reset counter
	ReadEE(EEPAGE, EE_FD_LOC, &fd_counter, WORD);
	return (checksum != Cfg_Checksum);	// Compare the calculated checksum with that stored in the EEPROM
	}

// Save calibration values
void save_calval(void)
	{
	int i;

	Cal_Checksum = 0;

	for(i = 0; i < (sizeof(struct calval) / 2); i++)
		{
		if(i < ((sizeof(struct calval) / 2) - 1)) Cal_Checksum = crc_16(cal.ee[i], Cal_Checksum);

		EraseEE(EEPAGE, ((2 * i) + EECAL), WORD);
		WriteEE((int *)&(cal.ee[i]), EEPAGE, ((2 * i) + EECAL), WORD);	// EEPROM Address 8 high bits, address + physical EEPROM start 16 low bits
		}

	save_extval();						// The extension block holds the Beep Tone service setting. DL4JC
	}

// Read calibration values
unsigned int read_calval(void)
	{
	int i;
	unsigned int checksum = 0;

	for(i = 0; i < (sizeof(struct calval) / 2); i++)
		{
		ReadEE(EEPAGE, ((2 * i) + EECAL), (int *)&(cal.ee[i]), WORD);	// EEPROM Address 8 high bits, address + physical EEPROM start 16 low bits

		if(i < ((sizeof(struct calval) / 2) - 1)) checksum = crc_16(cal.ee[i], checksum);
		}

	return (checksum != Cal_Checksum);	// Compare the calculated checksum with that retrieved from the EEPROM
	}

void wait_PWR_BAND_UP_rls(void)
	{
	while (PWR_SW || !BAND_UP);
	}

void wait_PWR_rls(void)
	{
	while (PWR_SW);
	ms_delay(BUTTON_DEBOUNCE);
	}

/*
 This function transmits the frequency request command.
*/
void xmit_cmd(const char *p)
	{
	int chr_count;
	
	chr_count = *p++;

	while(chr_count--) putch(*p++);
	}
	
/*
 This function sets the repeat speed for the UP/DOWN buttons when in the User Configuration mode.
 The integer constant SPEED_ARRAY contains a bit pattern representing the repeat speed. The bit position represents
 the configuration page, and a '1' in that position represents the fast repeat, a '0' represents the slow speed.
 By shifting a 1 left to the bit position represented by the page number contained in sub_page1 and then masking
 the constant SPEED_ARRAY to determine if it contains a '1' or a '0' rep_dly can be assigned either the fast or the
 slow delay constant. A.Ryan - 5B4AIY - 16/FEB/2015
*/
void set_repeat_speed(void)
	{
	rep_dly = (SPEED_ARRAY & (1L << sub_page1)) ? _FAST : _SLOW;	// 1L: there are now 17 pages
	}

void display_beeps(int page)
	{
	if(page) beep(HZ587_31, Beep_Time);	// D tone beep for other displays
	else
		{
		beep(HZ466_85, Beep_Time * 10);	// B-flat tone for basic display
		ms_delay(500);
		}
	}

int int_round(int dividend, int divisor)
	{
	int q, r;

	q = dividend / divisor;
	r = dividend % divisor;

	if(r >= (divisor >> 1)) q++;

	return q;
	}

// Clear alarms & try to reset over-current latch
void clear_alarms(void)
	{
	loop_ctr = 0;								// Reset the blink counter

	if(!alarms) return;

	if(alarms & CURR_AL)
		{
		alarms &= CURR_AL_OFF;					// Clear alarm flag
		OC_CLR = 0;								// Generate reset pulse for over-current latch
		ms_delay(20);
		OC_CLR = 1;
		return;
		}

	if(alarms & SWR_AL)
		{
		alarms &= SWR_AL_OFF;
		return;
		}

	if(alarms & TEMP_AL)
		{
		alarms &= TEMP_AL_OFF;
		return;
		}

	if(alarms & HI_V)
		{
		alarms &= HI_V_AL_OFF;
		return;
		}

	if(alarms & LO_BATT)
		{
		batt_pre_limit = OFF;
		alarms &= LO_BATT_AL_OFF;
		return;
		}

	if(alarms & LO_V)
		{
		alarms &= LO_V_AL_OFF;
		return;
		}
	}

void poll_chk(int poll_type)				// Check if we need to query the transceiver for its frequency
	{
	if(not_used == TRUE && Poll_Time == FALSE)
		{
		xmit_cmd(poll_cmd[poll_type]);		// If this is the first time, or a manual band change, or a switch to auto, force a frequency query.
		not_used = FALSE;
		return;
		}
	}

// ASCII Command Protocol Auto-Band Select Module
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int get_band(unsigned frequency)
	{
	int i = 0;

	do	{
		if(frequency < band_limits[i])
			break;					// We have found the band,
		} while (++i < NOT_KNOWN);	// otherwise return NOT_KNOWN.

	return i;
	}

// Band for an ASCII frequency in Hz, as in the FA and IF data packets
int ascii_freq_band(char *digits)
	{
	long freq;

	freq = (atol(digits) + 500L) / 1000L;	// Convert to nearest kHz

	if(freq > 30000L)						// Check limit before the cast to unsigned int, otherwise e.g. 144MHz
		freq = 30002L;						// would wrap to 12.9MHz and select the 20m filter.

	return get_band((unsigned int)freq);
	}

void serial_kx3(void)				// Serial Data Handler
	{
	char c;
/*
 First check to see if the polling timer has timed out, or if the not_used flag is set. The not_used
 flag is set if this is the first time we have checked, or if the manual band switch has been pressed,
 or if we have switched from manual to auto band select. Then check to see if there are any characters
 in the UART receive buffer with kbhit().

 If there is a character waiting, then we collect it. Under normal circumstances this would
 be the first character, so we check that the buffer index is zero, and the character is the
 letter F. If it is not, then we simply do nothing and return.

 If it is the letter F, then this is possibly the start of a valid FA data packet, so now we
 insert this character into the buffer, and increment the pointer, and start the message timer.

 When the next character is received the buffer index is now 1, so this letter should be an A.

 If it is not then we clear the buffer and return and await another character.

 If it is, then this is the start of a valid frequency data packet, so we insert this character
 into the buffer, increment the pointer, and continue. We then check to see whether we have
 reached the end of the buffer or if a time-out has occurred, in which case we simply dump the
 buffer and restart.

 Assuming we are not at the end of the buffer nor a time-out, eventually we will receive the
 end-of-message character ; and at this point we convert the first buffer entry into a frequency
 and select a band. If the polling timer is non-zero then we set the polled message received flag,
 clear the buffer and start again.
  
 The KX-3 default baud rate is 4800, and as there are 14 bytes to send, then the minimum
 time is: 10 bits/character (8 + start + stop) * 14 * 1/4800 = 29mSec. So setting the
 time-out value to a generous 200mS should be more than enough to ensure reasonable
 operation but still provide a guard against something going wrong.

 At the slowest speed of 1200 baud, the message would take 117mS, so again, there is
 enough margin to ensure that the message can be received, but still protect against
 some communications problem that would cause an incomplete message to be received.

 The reason for not setting the band to NOT_KNOWN if the message ident is wrong is
 that the amplifier may be 'piggy-backed' onto a serial link that is also being used
 by another piece of software, and that there may well be other messages present in
 the data stream. By simply remembering the last valid band setting we avoid constantly
 unsetting the band and forcing the amplifier into standby simply because the message
 is not intended for us. We still have to set the NOT_KNOWN setting in the event of a
 gross error.
 
 If the PA-100D has polling disabled, then to ensure that the amplifier is set to the current
 band of the KX3, a forced FA query command is sent the first time. To ensure that this
 occurs, it is necessary to power the KX3 up first and let it initialise, then power up
 the PA-100D. The forced FA query command will cause the KX3 to return its current
 frequency, and this will set the current band. Equally, if polling is disabled, as soon
 as we attempt a manual band switch, or switch from AUTO to MANUAL then a frequency query
 command is sent to the KX3 to set the band correctly. Obviously, if we are piggy-backing
 on the received data, and the transmit data is not connected, then this will fail, but
 that is a risk the user has to accept if operating in this condition, but at the next
 FA; data packet the band should be correctly set. A.Ryan - 5B4AIY - 30/APR/2020
*/
	poll_chk(ELECRAFT_KX3);							// Check if we need to get the frequency

	if(kbhit())										// We've received a character
		{
		c = getch();								// Get character from UART

		if(cmd_buf_idx && !cmd_timeout)				// The rest of the previous message did not arrive in time, so dump it.
			clear_buffer();							// (Previously the timer was set again before the test, which then
													// never applied. DL4JC)
		if(cmd_buf_idx == 0 && c != 'F')			// Not a valid start of message
			return;

		cmd_timeout = msg_time;						// Set the message timer

		if(cmd_buf_idx == 1 && c != 'A')			// These are not the droids you're looking for,
			{
			cmd_timeout = 0;						// so clear the message timer,
			clear_buffer();							// and the buffer and index.
			return;
			}

		cmd_buf[cmd_buf_idx++] = c;					// Character is valid, so add it to the buffer.

		if((cmd_buf_idx > MSG_LEN) || !cmd_timeout)	// Are we about to overrun the buffer or have we timed-out?
			clear_buffer();							// Yes, so, dump the buffer and restart.
		else										// No, so continue.
			{
			if(c == ';')							// We've received the End-Of-Message character of a data packet.
				{
				if(Poll_Time)
					poll_resp_rec = TRUE;			// If polling is enabled, set the poll response received flag.

				Current_Band = ascii_freq_band((char *)cmd_buf + 2);
				cmd_timeout = 0;					// Reset the message timer,
				clear_buffer();						// reset the buffer and index, and restart.
				}	// End IF
			}	// End ELSE
		}	// End IF
	}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Hardrock-50 Protocol Emulation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
 In the HR50 band select mode the PA-100D answers the serial commands of the HobbyPCB Hardrock-50 amplifier, so that
 programs and transceivers that support the HR50 can be used. The commands and the reply formats follow the HR50 firmware
 V3.0 (github.com/hobbypcb/hardrock-50, uart.c and config.c), which differs from the manual in places, e.g. HRBN 3 = 15m
 and 4 = 17m. Commands end with ';' and may be upper or lower case. Replies are upper case and end with ";\r\n". As with
 the HR50, a SET command is not answered, and a GET command is the command letters without data.

	FAxxxxxxxxxxx;	Frequency in Hz, selects the band. IFxxxxxxxxxxx...; (Kenwood IF data) is used in the same way.
	HRBN; HRBNn;	Band, HR50 numbering: 0 = 6m, 1 = 10m, 2 = 12m, 3 = 15m, 4 = 17m, 5 = 20m, 6 = 30m, 7 = 40m, 8 = 60m,
					9 = 80m, 10 = 160m, 99 = unknown.
	HRMD; HRMDn;	Keying mode, 0 = OFF (Standby), 1 = PTT (Operate). 2 (COR) and 3 (QRP) select Standby: the PA-100D has
					no COR keying, and the HR50 without ATU does not accept QRP either.
	HRRX;			Status, RX,<mode>,<band>,<temp>,<voltage>; e.g. RX,PTT,20M,27C,13.8V;
	HRTP;			Heat-sink temperature, e.g. HRTP27C;
	HRVT;			Supply voltage, e.g. HRVT13.8V;
	HRAT;			ATU mode, always 0 = not present.
	HRBR;			Serial speed, 0 = 4800, 1 = 9600, 2 = 19200, 3 = 38400. Other speeds report the nearest value.
	HRKX;			KX3 inverted data, always 0.
	HRTM...;		ATU pass-through, answered with HRTM; as by an HR50 without ATU.

 The serial speed, the temperature scale and the KX3 mode cannot be changed by a command (HRBR, HRTP and HRKX SET are
 ignored). They are set in the User Configuration menu, where the temperature scale also sets the alarm and fan limits.

 Bands the PA-100D cannot amplify (6m, unknown) set NOT_KNOWN, which inhibits TX. 60m uses the 40m filter, and FA/IF
 frequencies are assigned with band_limits[], as in the other modes. As with the HR50, nothing is transmitted except
 the replies, the transceiver is not polled, and the band is held until the host sends a new one. DL4JC
*/
#define HR_CMD(a, b)	(((a) << 8) | (b))		// Two command letters after HR as one value for switch()

const int hr50_to_band[] = {NOT_KNOWN, 9, 8, 7, 6, 5, 4, 3, 3, 2, 1};	// Indexed by: HR50 band number 0 - 10
const int band_to_hr50[] = {99, 10, 9, 7, 6, 5, 4, 3, 2, 1, 99};		// Indexed by: Current_Band
const char *hr50_band_txt[] = {"UNK", "160", "80M", "40M", "30M", "20M", "17M", "15M", "12M", "10M", "UNK"};	// Indexed by: Current_Band
const char *hr50_mode_txt[] = {"OFF", "PTT"};							// Indexed by: pa_state

// Execute a complete command in cmd_buf, without the terminating ';'
void hr50_command(void)
	{
	char reply[40];
	char *p = reply;
	char *arg = (char *)cmd_buf + 4;				// Data after HRxx
	int n;
	double volts;

	reply[0] = 0;
	volts = (double)batt_raw * (double)Voltmeter_Cal / 1000000.0;

	if(((cmd_buf[0] == 'F') && (cmd_buf[1] == 'A')) || ((cmd_buf[0] == 'I') && (cmd_buf[1] == 'F')))
		{
		if(cmd_buf_idx < 13) return;				// FA; or a short packet, no frequency.

		cmd_buf[13] = 0;							// 11 digits, Hz. The IF packet continues with other data.
		Current_Band = ascii_freq_band((char *)cmd_buf + 2);
		return;
		}

	if((cmd_buf[0] != 'H') || (cmd_buf[1] != 'R')) return;	// Not for us, e.g. other CAT traffic on the same line.

	switch(HR_CMD(cmd_buf[2], cmd_buf[3]))
		{
		case HR_CMD('B', 'N'):						// Band
			if(!*arg)
				sprintf(reply, "HRBN%d", band_to_hr50[Current_Band]);
			else if(isdigit(*arg))
				{
				n = atoi(arg);
				Current_Band = ((n >= 0) && (n <= 10)) ? hr50_to_band[n] : NOT_KNOWN;	// n < 0: atoi() overflow
				}
		break;

		case HR_CMD('M', 'D'):						// Keying mode
			if(!*arg)
				sprintf(reply, "HRMD%d", pa_state);
			else if(isdigit(*arg))
				pa_state = (*arg == '1') ? OPERATE : STANDBY;
		break;

		case HR_CMD('R', 'X'):						// Status
			if(!*arg)
				sprintf(reply, "RX,%s,%s,%d%c,%.1fV", hr50_mode_txt[pa_state], hr50_band_txt[Current_Band],
					scaled_pa_temp, T_Char[Temp_Scale], volts);
		break;

		case HR_CMD('T', 'P'):						// Temperature
			if(!*arg) sprintf(reply, "HRTP%d%c", scaled_pa_temp, T_Char[Temp_Scale]);
		break;

		case HR_CMD('V', 'T'):						// Supply voltage
			if(!*arg) sprintf(reply, "HRVT%.1fV", volts);
		break;

		case HR_CMD('A', 'T'):						// ATU, not present
			if(!*arg) sprintf(reply, "HRAT0");
		break;

		case HR_CMD('B', 'R'):						// Serial speed, index 2 = 4800 ... 5 = 38400
			if(!*arg)
				{
				n = eeprom.defval.br - 2;

				if(n < 0) n = 0;
				if(n > 3) n = 3;

				sprintf(reply, "HRBR%d", n);
				}
		break;

		case HR_CMD('K', 'X'):						// KX3 inverted data, not used
			if(!*arg) sprintf(reply, "HRKX0");
		break;

		case HR_CMD('T', 'M'):						// ATU pass-through, no ATU
			sprintf(reply, "HRTM");
		break;
		}

	if(!*p) return;									// SET commands are not answered.

	while(*p) putch(*p++);

	putch(';');
	putch('\r');
	putch('\n');
	}

void serial_hr50(void)
	{
	char c;

	while(kbhit())
		{
		c = toupper(getch());

		if(cmd_buf_idx && !cmd_timeout) clear_buffer();		// Incomplete command timed out, start again.

		if(c == ';')										// End of command,
			{
			hr50_command();									// so execute it,
			clear_buffer();									// and start again.
			}
		else if(cmd_buf_idx || isalpha(c))					// CR, LF and spaces between commands are ignored.
			{
			cmd_timeout = msg_time;							// Time-out for the next character

			if(cmd_buf_idx < (MAX_BUFFER - 1))				// Longer commands (IF) are truncated, only their start is used.
				cmd_buf[cmd_buf_idx++] = c;					// The last byte remains 0 and terminates the data.
			}
		}
	}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Yaesu 5-byte Binary Command Protocol Auto-Band Select Module
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int bcd2bin(int num)
    {
    int n;

    n = num >> 4;
    n *= 10;
    return n + (num & 0x0F);
    }

// Convert CAT frequency data to JUMA-TRX2 format
int convert_cat_freq(void)
	{
	long freq = 0L;
	int i = 0;

	do	{
		freq *= 100L;
		freq += (unsigned long)bcd2bin(cmd_buf[i]);
		} while(++i < 4);

	freq += 50;					// Round up
	freq /= 100L;				// and divide to the nearest kilo-hertz

	if(freq > 30000L)			// Check limit
		freq = 30002L;

	return get_band((unsigned int)freq);
	}

void get_yaesu(void)
	{
	poll_chk(YAESU);						// Check if we need to get the frequency

	if(kbhit())
		{
		cmd_buf[cmd_buf_idx++] = getch();	// Get character, add to buffer and increment index
		cmd_timeout = 200;					// Set the timeout to 200mS

		if(cmd_buf_idx >= MAX_YAESU_CMD)	// MAX_YAESU_CMD = 5
			{
			if(Poll_Time) poll_resp_rec = TRUE;	// If polling is enabled, set the poll response received flag.

			Current_Band = convert_cat_freq();
			clear_buffer();					// Clear the buffer and reset the pointer for the next message
			}
		}
	else									// If we have received a character, but the timeout has expired,
		{									// then clear the buffer and re-start.
		if(cmd_buf_idx && !cmd_timeout) clear_buffer();
		}		
	}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
 As has been explained in the comments in timers_pwm.c, the constant chosen for the TMR3 interrupt is 7378.
 This gives an actual interrupt timing of 1000.7053uS, but from this must be subtracted the machine cycles
 necessary to stop the counter, transfer the accumulated count, and re-start the counter. Measurements
 indicated that this is about 6 cycles.

 The following table shows the actual frequencies that just result in the tabulated frequency display,
 the frequency correction factor is: 999984. The average positive difference is: 523.7Hz, the average
 negative difference is: -518.2Hz.

	DISPLAY			ACTUAL FREQUENCY	DIFFERENCE
	==============================================
	1.901				1.900521			+521
	1.899				1.899474			-526

	3.601				3.600524			+524
	3.599				3.599485			-515

	7.101				7.100522			+522
	7.099				7.099484			-516

	10.126				10.125522			+522
	10.124				10.124481			-519

	14.201				14.200524			+524
	14.199				14.199487			-513

	18.119				18.118525			+525
	18.117				18.117469			-531

	21.301				21.300523			+523
	21.299				21.299484			-516

	24.941				24.940527			+527
	24.939				24.939487			-513

	28.851				28.850525			+525
	28.849				28.849485			-515
	==============================================
*/
double get_freq(void)
	{
	static double freq;

	if(!freq_ctr)
		{
		freq = (input_freq > 100000L)	// This eliminates the spurious 0.001 display with no input
			? (double)input_freq / ((double)cal.calval.freq_cal / 10.0)
			: 0.0;
		input_freq = 0L;				// Reset frequency averaging counter
		freq_ctr = 100;					// Reset sample counter
		}

	return freq;
	}

// Measure & display Drain current meter
void disp_id(void)
	{
	static double drain_current;
	static int hold_counter, state;

	if(state != key)							// If we have changed from RX to TX or vice versa,
		{										// then reset the counter and amplifier current data.
		state = key;
		drain_current = 0.0;
		hold_counter = 0;
		}
	else
		{
		id_cur = (double)((double)amp_current * (double)Ammeter_Cal);

		if(id_cur > drain_current)
			{
			drain_current = id_cur;
			hold_counter = 200;
			}
		else
			{
			if(hold_counter) hold_counter--;
			else drain_current = 0.0;
			}

		disp_meter((svc_flag) ? id_cur : drain_current, AMPS);	// I = id_cur / 200000
		}
	}

void shut_down(void)
	{
	set_pwm3_dac(OFF);							// Turn backlighting off
	FAN1 = OFF;
	FAN2 = OFF;									// Turn fan off
	wait_PWR_rls();								// Wait for power switch to be released,
	PWR_ON = OFF;								// Unlock the power latch,

	while (TRUE);								// and walk off into that long goodnight...
	}

void disp_fwd_pwr(void)
	{
	static long peak_pwr;
	double power;

	if(out_pwr >= peak_pwr)						// Fast attack
		{
		peak_pwr = out_pwr;						// Fast attack
		decay_counter = PEAK_SHOW_TIME;			// Reload decay timer (1,000mS)
		}
	else if(!decay_counter)
		{
		peak_pwr = 0L;
		power = 0.0;
		}

	power = ((double)peak_pwr * (double)cal.calval.fwd_pwr_mult);	// Scaling, P = ((ADC * ADC) * Calibration Factor) / 100,000,000

	if(Power_Units)					// Power in dBm = 10 * log10(power in mW)
		{
		if(power > LOW_PWR_LIMIT)				// 26dBm
			disp_meter((log10(power / 100000.0)), DBM);				// The 10 multiplier is embedded in scale_factor[] in lcd-trx2.c
		else
			lcd_putst(pwr_mtr_msg[1]);
		}
	else										// Power in Watts
		{
		if(power > LOW_PWR_LIMIT)				// 0.4W
			disp_meter(power, WATTS);
		else
			lcd_putst(pwr_mtr_msg[0]);
		}
	}

void dp_1(void)							// Display SWR
	{
	(swr == 0) ? lcd_putst(SWR_NA) : disp_meter(vswr, SWR);
	}
/*
 The battery voltage is sensed via a potential divider of 33K in series with 10K. The division ratio
 is therefore 10 / 43 = 0.2325581395. The battery calibration factor is scaled to give a battery
 voltage in uV, and for the nominal setting is 5250. Thus, for a nominal amplifier a 13.8V input
 voltage is divided to 13.8 * 0.2325581395 = 3.2093023256, which gives an A-D output of:
 4096 * 3.2093023256 / 5.0 = 2629. The full-scale reading of the voltmeter is 21.5V
 The initial averaging of the battery voltage was 16 samples, but this gave rise to a fair amount
 of jitter at times. After a lot of experimenting the present value of 50 samples was decided. If
 1000 samples were used it would take too long for a true reading to be obtained, 100 samples was
 better, but there was still a significant lag in the readings. 25 samples was not much better than
 16, but 50 samples seems to be just about optimum between reasonable freedom from jitter, and reasonable
 response time. Note that the voltage alarms are virtually instantaneous as they only require a single
 sample. I had given some thought to using the average battery voltage to trip the alarm, but as I'd
 not experienced any problems with transients or other noise triggering alarms, this has not been done
 for this version of the software. A. Ryan 5B4AIY - 30/MAY/2012

 I decided to experiment some more, and I found that the present version using a 50-sample running average
 seems to works the best. It gives a reasonably fast response for large changes, but still smooths out
 the jitter. A. Ryan - 31/MAY/2012
 Note that the expression for the 50-sample running average: batt_avg = (0.98 * batt_avg) + ((0.02 * voltage) / 1000)
 simplifies to: batt_avg = (0.98 * batt_avg) + (voltage / 50) - A. Ryan - 4/OCT/2012
*/
void dp_2(void)							// Display Supply Voltage
	{
	batt_avg = (0.98 * batt_avg) + (((double)batt_raw * (double)Voltmeter_Cal) / 50.0);
	disp_meter(batt_avg, VOLTS);		// batt_avg is in uV
	}

void dp_4(void)							// Display Heat-Sink Temperature
	{
	disp_meter((double)scaled_pa_temp, Temp_Scale);
	}

void dp_5(void)							// Display frequency and power
	{
	sprintf(lcdpbuff, freq_fmt, get_freq());
	lcd_putst(lcdpbuff);
	disp_fwd_pwr();
	}

void (*display_page[])(void) = {			// Indexed by: sub_page0
							disp_fwd_pwr,	// 0 - Forward Power
							dp_1,			// 1 - SWR
							dp_2,			// 2 - Voltage
							disp_id,		// 3 - Drain Current
							dp_4,			// 4 - Temperature
							dp_5			// 5 - Frequency Display (Extended page, requires F-SENSE mode selected.)
							};

/*
 The status line is first formatted into a buffer and then transmitted in one piece. Previously each field was sent
 with its own printf(), and with the XC16 library each printf() waits until the last bit has been transmitted before
 the next field is formatted. The resulting gaps split the line, e.g. in USB serial adapters, and remote programs such
 as JUMA_CTRL could then mis-read a fragment as a status line, showing ??? instead of OPER/STBY. The output format is
 unchanged.  DL4JC - 04/OCT/2026
*/
void send_status(void)
	{
	double power;
	char status[80];					// Formatted status line, approx. 46 characters
	char *p = status;

	power = ((double)out_pwr * (double)cal.calval.fwd_pwr_mult);	// Scaling, P = ((ADC * ADC) * Calibration Factor) / 100,000,000
	sprintf(status, "%c:%c:%c:%c:%2d:%1d:%3.1f:%5.2f:%4.1f:%5.1f:%3d:%1d:%2X\n\r",
		pa[pa_state],
		ba[Auto_Manual],
		tx,
		T_Char[Temp_Scale],
		Current_Band,
		RF_Gain[Current_Band] + 1,
		swr / 100.0,
		(double)batt_raw * (double)Voltmeter_Cal / 1000000.0,
		((double)amp_current * (double)Ammeter_Cal) / 200000.0,
		(power > LOW_PWR_LIMIT) ? power / 100000000.0 : 0.0,
		scaled_pa_temp,
		fan_speed,
		alarms);

	while(*p) putch(*p++);				// Transmit the complete line without gaps.
	}
/*
 Remote Control & Status Reporting
 Select the F-Sense mode, and then select the REMOTE serial port mode in the User Configuration menu.
 Commands take the form:

 =Cn\n\r

 Where C is a command letter which may be followed by a parameter, which is a single decimal digit in the range 0 - 9.

 Valid commands are:
 A	Select Auto band select mode.
 Bn	Select Manual band n, where n is 1 - 9. 1 = 160m, 9 = 10m.
 C	Attempt to cancel any alarms.
 Gn	Select Gain n, where n is 1 - 4.
 O	Select the Operate mode.
 Pn	Power Off. If n = 0, power off without saving the current state. If n = 1, save the current state.
 R	Request current status.
 S	Select the Standby mode.

 Status Report Format
 The amplifier will respond to a status request by transmitting a string, similar to this example, terminated by \n\r:

 O:A:T:C: 5:1:1.0:14.09: 8.1: 27.2: 26:0: 0

 O		O operate, or S standby state.
 A		A auto band, or M manual band select.
 T		T transmit, or R for receive.
 C		C Centigrade or F Fahrenheit temperature scale.
 5		Current band, in this case, 20m, formatted as 2 digits, right justified.
 1		Current gain setting, G1.
 1.0	SWR, 1.0:1 formatted as 2 digits, 1 decimal place.
 14.09	Supply voltage, V, formatted as 4 digits, 2 decimal places, right justified.
 8.1	Amplifier total current, A, formatted as 3 digits, 1 decimal place, right justified.
 27.2	Output power, W, formatted as 4 digits, 1 decimal place, right justified.
 26		Heat-sink temperature, Centigrade in this case, formatted as 3 digits, no decimal places, right justified.
 0		Fan speed, Off in this case, formatted as a single digit. 1 = Slow, 2 = Medium, 3 = Fast.
 0		Alarm, none, formatted as a 2-digit hexadecimal number. Bit-mapped alarms.
*/
void remote(void)
	{
	unsigned int digit;
	char c;

	if(lcd_mode == USER_CONFIG_MODE) return;// Do not respond if in the User Configuration Mode

	if((!polling_timer) && Poll_Time)
		{									// If polling is enabled, and timer has expired,
		send_status();						// send the current status, and reset the polling timer,
		polling_timer = Poll_Time * 1000;
		rmt_timeout = polling_timer + 100;	// and reset the command time-out.
/*
 KNOWN LIMITATION: Resetting the command time-out here means that with polling enabled the time-out never expires,
 so the loss of the remote host is not detected and the amplifier does not fall back to STANDBY. This is kept
 deliberately for compatibility with existing remote hosts that rely on the automatic status messages and only send
 occasional commands. Without polling, the 5 second time-out after the last received command still applies.
*/
		}

	if(!kbhit()) return;					// No characters pending, so exit.

	c = getch();							// Get the next character from serial buffer,

#if	REMOTE_TEST
	printf("\n\rCharacter: %c $%2.2X, Index: %i", (isprint(c)) ? c : '#', c, cmd_buf_idx); // Test
#endif

	if(c == 0x0A) return;					// Ignore

	switch(cmd_buf_idx)
		{
		case 0:
			if(c != MSG_START) return;		// Not a valid start of message character
		break;

		case 1:
			if(!isupper(c))					// If second character is not an upper-case letter,
				{
				clear_buffer();				// then clear the buffer, and exit.
				return;
				}
		break;

		case 2:
			if(c != MSG_END && !isdigit(c))	// If the third character is neither the end-of-message nor a digit,
				{
				clear_buffer();				// then clear the buffer and exit.
				return;
				}
		break;

		case 3:
			if(c != MSG_END)				// If this is not the end-of-message, then clear the buffer and exit.
				{
				clear_buffer();
				return;
				}
		break;

		default:							// Illegal buffer index, so clear the buffer and exit.
			clear_buffer();
			return;
		break;
		}

	cmd_buf[cmd_buf_idx++] = c;				// Otherwise, add the character to the buffer, and increment the index,

	if(c != MSG_END) return;				// and exit if not end of message.
	
#if	REMOTE_TEST
	printf("\n\rCommand: %s\n", cmd_buf);	// Test
#endif

	rmt_timeout = (Poll_Time)
		? (Poll_Time * 1000) + 100
		: 5000;								// Message received, so reset the command time-out, decremented in _T3Interrupt() in timers_pwm.c
// 2.443mS/1uS, 2.672mS/25uS, 2.855mS/50uS, 3.237mS/100uS Loop cycle time versus delay in adc12.c (Display Page 0, RF Power)
	c = cmd_buf[1];							// Get the command,
	digit = cmd_buf[2] - '0';				// and any numeric parameter.

	switch(c)
		{
		case 'A':							// Request to set band select to AUTO
			Auto_Manual = AUTO_BAND;
		break;

		case 'B':							// Request to set band select to MANUAL and band 1 - 9 (160m through to 10m)
			if(digit > 0 && digit < 10)
				{
				Auto_Manual = MANUAL_BAND;
				Current_Band = digit;
				}
		break;

		case 'C':							// Request to clear alarms
			clear_alarms();
		break;

		case 'G':							// Request to set the gain.
			if(digit > 0 && digit < 5) RF_Gain[Current_Band] = digit - 1;
		break;

		case 'O':							// Request to change state to OPERATE
			pa_state = OPERATE;
		break;

		case 'P':							// Request to power down
			if(digit) save_defval();		// Save current state

			display_screen(Rmt_Pwr_Off, (digit) ? Data_Saved : "");
			ms_delay(1500);
			shut_down();
		break;

		case 'R':							// Request to send status
			send_status();
		break;

		case 'S':							// Request to change state to STANDBY
			pa_state = FALSE;
		break;
		}
	clear_buffer();
	}
/*
 Setting the address of rs232_mode here avoids having to do numerous loop invariant address calculations.
 This somewhat complex logic sets the address of the function to be executed as follows:
 If the Yaesu 5-Byte Binary mode is selected, then set the address to get_yaesu. If the ASCII protocol is selected, then set the address
 to serial_kx3. If the Juma TRX-2 mode is selected set the address to serial_pa100, and in the HR50 mode to serial_hr50. If the
 REMOTE mode is selected set the address to
 remote. If the TEST mode is selected set the address to serial_test, and if none of these modes is selected, then just keep clearing the
 buffer, effectively discarding any received characters.
*/
void _rs232_mode(void)
	{
	rs232_mode = (Band_Select_Mode == YAESU)
				? get_yaesu
				: (Band_Select_Mode == ELECRAFT_KX3)
				? serial_kx3
				: (Band_Select_Mode == JUMA_TRX2)
				? serial_pa100
				: (Band_Select_Mode == HR50)
				? serial_hr50
				: (Serial_Test_Mode == REMOTE)
				? remote
				: (Serial_Test_Mode == SERIAL_TEST)
				? serial_test
				: ClearUSART1queue;
	}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Calculate SWR
 The Voltage Standing Wave Ratio or VSWR can be calculated from the forward and reverse voltages from the directional
 coupler used to sense power. If Vf is the forward voltage, and Vr the reverse voltage, then:
 
	Ro = Vr / Vf
	VSWR = 1 + Ro / 1 - Ro

 In this case, we want the VSWR to be 100 for a VSWR of 1:1 as we are dealing with integer arithmetic, and the final
 value will be scaled by a factor of 100 when displayed.

 Added a check to the reflection coefficient to prevent a possible math trap error if the SWR is very high. MAX_RO
 is defined as 0.816514, giving a maximum SWR of 9.9:1
*/
double calc_swr(long fwd, long rev)
	{
	double Ro;

	if(fwd < PWR_MTR_DEAD_BAND) return 0.0;	// Only calculate if we have some power

	Ro = (double)rev / (double)fwd;

	if(Ro > MAX_RO) Ro = MAX_RO;			// Prevent a possible math trap error.

	return ((1.0 + Ro) / (1.0 - Ro)) * 100.0;
	}

// Display Alarms
const char * display_alarms(void)
	{
	if(!loop_ctr)						// If alarm blink time has timed-out,
		{
		loop_ctr = LOOP_COUNT;			// reset loop counter for next blink cycle,
		display_message ^= 1;			// toggle message display flag,
		beep(HZ2000, ALARM_BEEP);		// and always sound beep for alarms.
		}

	if(display_message)					// If set, display alarm message, in priority order,
		{
		if(alarms & CURR_AL) return CURR_Alarm;
		if(alarms & SWR_AL) return SWR_Alarm;
		if(alarms & TEMP_AL) return TEMP_Alarm;
		if(alarms & HI_V) return HI_V_Alarm;
		if(alarms & LO_BATT) return LO_Batt_Alarm;
		if(alarms & LO_V) return LO_V_Alarm;
		}

	return five_spc;					// Otherwise, display blanks
	}

// Calculate & display bar graph meter
int scaled_value(void)
	{
	if(eeprom.defval.graph_limits)		// If Graph Limits is set to ON...
		{
		if(sub_page0 == 1)
			{
/*
 SWR Bridge forward voltage is 3.6V @ 100W. This gives an A-D output value of 4096 * (3.6 / 5.0) = 2950
 Forward Power = (A-D * A-D) which at 100W = 2950 * 2950 = 8,702,500. The graphical meter has a full-scale
 of 48, thus the scale factor = 2950 * 2950 * 100 * (Full-Scale Power/100) / 48

  Scale Factor  Full-Scale Power	Voltage		A-D
  ==================================================
	181302			100W				3.6		2950
	199432			110W				3.8		3094
	217432			120W				3.9		3232
	235693			130W				4.1		3364
	253823			140W				4.3		3490
	271953			150W				4.4		3613
*/
			if(swr < 100) return 0;		// This avoids garbage character in graphic display at first position when there is no reasonable output power.
			return int_round((48 * (swr - 99)), (SWR_Trip - 99));
			}
/*
 The display is effectively that of a suppressed-zero voltmeter. The minimum reading is the under-voltage trip limit,
 and the maximum is the over-voltage trip limit. The scale length is therefore: over-voltage - under-voltage
 The fractional length to display is thus: (actual voltage - under-voltage) / scale length
 and the number of bars is: 48 * fractional length
 The battery voltage is measured from a potential divider consisting of a 33K resistor in series with a 10K resistor.
 The fractional factor is therefore: 10 / (33 + 10) = 0.232558
 If we assume an under-voltage trip of 11V and an over-voltage trip of 15V, this gives a scale length of 4V, and as there
 are 8 divisions, this resolves to 0.5V/division. Using the default battery calibration multiplier of 5249, the
 under-voltage trip A-D value is therefore 2095, and the over-voltage A-D value is 2858.
 The scale length is therefore 2858 - 2095 = 763
 If the input voltage for example is 13V, this is an A-D value of 2477, batt_raw would therefore be 2477 * 5250 = 13004250
 Thus the first value is: v = (batt_raw - cal.calval.undervoltage_trip) * cal.calval.batt_mult / BATT_MULT
 The variable value would then be 48 * 2447 / 763 = 117456 / 763 = 153
 Taking account of the scale length, v = v /  (cal.calval.overvoltage_trip - cal.calval.undervoltage_trip)
 Since there are 6 divisions to each scale bar, this gives a fraction display of 153 / 6 = 25, or half scale.
 Note that there is a constant loop-invariant expression here: 48 * cal.calval.batt_mult / BATT_MULT,
 as well as: 48 / (cal.calval.overvoltage_trip - cal.calval.undervoltage_trip)
 It is pointless re-calculating this constant each time through the loop, so it is moved to the initialisation
 section where: batt_scale_factor = cal.calval.overvoltage_trip - cal.calval.undervoltage_trip
*/
		if(sub_page0 == 2) return int_round(48 * (batt_raw - cal.calval.undervoltage_trip), batt_scale_factor);
/*
 The nominal sense voltage for the drain current is 100mV/A.
 id_cur = (double)(convert_adc12(ID_CUR) * (double)cal.calval.id_mult)
 Using the design value of the calibration factor 2442, and for a 24A limit current, the A-D output voltage would be:
 4096 * 2.4 / 5.0 = 1966. The variable: id_cur would therefore be: 1966 * 2442 = 4,800,972.
 The maximum number of bars is 48, thus the scale factor is: 4,800,972 / 48 = 100020.25 i.e, 100020.
 The number of bars to display is therefore id_cur / 100020
*/
		if(sub_page0 == 3) return (int)(id_cur / 100020.0);

		if(sub_page0 == 4)
			{
			if(Temp_Scale) return int_round((48 * scaled_pa_temp), Alarm_Temp);
			else return int_round((48 * (scaled_pa_temp - 32)), (Alarm_Temp - 32));
			}
		}
	return (int)(fwd_pwr / pwr_scale_factor);	// Default display, RF O/P Power
	}

/*
 Check for any alarms. The alarm word is bitmapped:

 D0	High SWR
 D1	Over-Current
 D2	High Temperature
 D3	Over-Voltage
 D4	Low-Voltage (Pre-Limit)
 D5	Low-Voltage (Final Limit)
 D6 - D15 Not Used
*/
void check_alarms(void)
	{
/*
 In the following tests, the battery voltage calibration factor has already been taken into account in the limits,
 and we are comparing corrected A-D converter values. (See function svc_7() in service.c for an example.)
*/
	if(batt_raw > cal.calval.overvoltage_trip) alarms |= HI_V;			// Set alarm bit
// If the low-voltage alarm is disabled, then batt_pre_limit is set to 0 as part of the start-up sequence.
	if(batt_raw < batt_pre_limit) alarms |= LO_BATT;					// Set alarm bit

	if(batt_raw < cal.calval.undervoltage_trip) alarms |= LO_V;			// Set alarm bit

// Check over current alarm bit & set alarm
	if(OC) alarms |= CURR_AL;											// Set alarm bit

// Check PA temperature
	if(scaled_pa_temp > Alarm_Temp) alarms |= TEMP_AL;	// Set alarm bit

// Check SWR, SWR Alarm only active in the OPERATE state.
	if((swr > SWR_Trip) & pa_state) alarms |= SWR_AL;	// Set alarm bit

	if(isr_swr_trip)									// SWR trip detected in the 1mS interrupt. The alarm bit is set
		{												// before the flag is cleared, so RF stays off in between.
		alarms |= SWR_AL;
		isr_swr_trip = FALSE;
		}

	alarms &= Enabled_Alarms;							// Select only the enabled alarms

	if(alarms & ALARM_MASK)								// If SWR, O/C, High-Temp, or, if enabled, High-Voltage, or Low Voltage
		{												// Mask off the Low-Voltage Pre-Limit alarm, as it is only a warning.
		TX_ON = OFF;									// RF off immediately,
		tx = 'R';
		pa_state = STANDBY;								// Force Standby
		lcd_mode = NORMAL_DISPLAY_MODE;					// Force normal display
		}
	}

// Save Current Settings with prompt
/*
 The prompt flag selects either the Save Settings or the Save Defaults message.
 Mode
 1 - Save Calibration Values,
 2 - Save Configuration Values,
 3 - Save both Calibration & Configuration values,
 7 - Restore Factory Defaults and save all
*/
void save_settings(int prompt, int mode)
	{
	int flag = TRUE;

	display_screen(save_settings_prompt[prompt], Yes_No);
	wait_PWR_BAND_UP_rls();					// Wait for button to be released...
	ms_delay(BUTTON_DEBOUNCE);				// Added to prevent spurious cancel command

	while (BAND_UP && !PWR_SW);				// Wait for a BAND+ or PWR button push...

	lcd_cmd(LINE2);

	if(!BAND_UP)							// If BAND+ button was pressed
		{
		if(mode & 4)						// If complete factory default restore
			set_factory_defaults();

		if(mode & 1)
			save_calval();					// Save new System Calibration Settings

		if(mode & 2)						// Save new User Configuration settings
			{
			save_defval();
			changed = FALSE;				// Reset User Settings Changed flag
			}

		beep(HZ698_45, Beep_Time * 5);
		}
	else
		{
		if(mode & 1)						// Restore previous System Calibration Settings,
			{
			read_calval();
			read_extval();					// and the Beep Tone service setting in the extension block. DL4JC
			}

		if(mode & 2)						// Restore previous User Configuration Settings
			{
			read_defval();
			read_extval();
			load_ext_modes();
			}

		flag = FALSE;
		beep(HZ587_31, Beep_Time);
		}

	lcd_putst(CS_Msg[flag]);
	ms_delay(1000);
	wait_PWR_BAND_UP_rls();					// If still pressed, wait for button to be released...
	set_chgen(Scale_Type);					// Load bar graph fonts
	}

// Handle PWR button.
void power_off(void)
	{
	button_timer = MEDIUM_PUSH;

	if(alarms)
		{
		clear_alarms();
		display_message = FALSE;			// Reset alarm message display flag.
		wait_PWR_rls();						// This prevents spurious display page change
		}
	else
		{
		do	{
			if(!button_timer)
				{
				beep(HZ392_01, Beep_Time * 2);		// Play low sound when ready for power down

				if(changed) save_settings(0, 2);	// Prompt to save the current User Settings

				clear_lcd();
				shut_down();
				}
			} while (PWR_SW);

		ms_delay(BUTTON_DEBOUNCE);
		sub_page0--;

		if(sub_page0 < 0) sub_page0 = max_page;

		display_beeps(sub_page0);
		}
	}
 
// RS-232 loop back test (Hold DISPLAY button during power-up)
void rs232_test(void)
	{
	unsigned char c;

	display_screen(RS232_Test_Msg, Awaiting_Data);
	printf(Press_PWR_To_Exit);
	ms_delay(400);							// Ensure message has been sent
	ClearUSART1queue();

	do	{
		if(kbhit())
			{
			c = getch();
			beep(HZ4000, 50);				// Give beep tone
			putch(c);						// Echo character
			sprintf(lcdpbuff, RX_Data, c, c);
			display_line(LINE2, lcdpbuff);
			}
		} while (!PWR_SW);

	beep(HZ466_85, Beep_Time);				// Notify PWR button pressed
	printf(RS232_Test_Terminated);
	ClearUSART1queue();
	wait_PWR_rls();							// Wait for switch to be released, this avoids spurious power down
	}

// F-Sense auto band
void reset_fsense(void)						// Clear F-sense 
	{
	int i = 9;

	do	{									// Clear the bins for the next sample set.
		band_bins[i] = 0;
		} while (i--);

	fs_min = 0xFFFF;						// Spread of the next sample set
	fs_max = 0;
	freq_sample_ctr = F_SAMPLES;			// Reload sample counter
	}

/*
 A modulated signal (two-tone, noise, SSB speech) is counted too low, as the input shaper loses the cycles where the
 amplitude is small: e.g. 14MHz two-tone is measured as approx. 11MHz in every sample. At the start of a transmission
 this selected a lower band, i.e. a filter below the transmitted frequency. A lower band is therefore only selected from
 a clean carrier (TUNE, CW), whose samples lie within FS_SPREAD of each other; modulated signals spread more. A higher
 band, the safe direction for the amplifier, and any band while the band is unknown, are still selected at once.
 A modulated signal may also select a lower band if even its highest sample is far below the current filter, i.e. below
 2/3 of its lowest frequency: the highest sample was measured at approx. 85% (two-tone) to 100% (noise) of the real
 frequency, while a real band change down is usually a much larger step, e.g. 20m to 40m. (v4.05) DL4JC
*/
#define FS_SPREAD	5				// Clean carrier: highest - lowest sample <= highest / 2^FS_SPREAD (approx. 3%)

void eval_band()	// Improved F-sense - A.Ryan - 5B4AIY - 30/APR/2014
	{
	static int last_band;					// Initialised to zero by default
	int i;
	int clean, far;

	if(!key) last_band = 0;					// Reset last_band in RX mode

	if(!freq_sample_ctr)					// Decide when we have complete set of measurements available
		{
		clean = (fs_max >= fs_min) && ((fs_max - fs_min) <= (fs_max >> FS_SPREAD));
		far = (fs_max >= fs_min) && (fs_max < (unsigned int)(((unsigned long)filter_lower(Current_Band) * 2UL) / 3UL));

		if(fsense_tst)						// Debug Mode - Can be invoked from Serial Test Suite
			{
			i = 0;

			do	{
				printf(_2I, band_bins[i]);
				} while (++i < 10);

			if(fs_max) printf("%5u %5u %s", fs_min, fs_max, clean ? "clean" : far ? "far" : "mod");	// Sample spread, kHz. DL4JC

			printf(New_Line);
			}

		i = 9;

		if(band_bins[0] < V_SAMPLES)		// We have meaningful sample set, enough measurements != 0
			{
			do	{
				if(band_bins[i]) break;
				} while (--i);

			if((last_band < i)
				&& ((i > Current_Band) || clean || far || (Current_Band == NOT_KNOWN) || (Current_Band == OUT_OF_BAND)))
				{
				last_band = i;
				Current_Band = i;			// Set band
				}

			if(key) fsense_confirmed = TRUE;	// The band has been measured in this transmission. (F-Sense QSK Off)

			if(key) fsense_evaluated = TRUE;	// The low frequency test in tx_guard() ends here. DL4JC

			if(key) filter_mismatch = FALSE;	// A valid measurement during TX: the band is now correct, tx_guard() checks
			}								// the filter again. (A band change is handled by set_relays().) DL4JC
		reset_fsense();						// Start next round
		}
	}

int get_817_band(int v)
	{
	int i = 10;

	do	{
		if(v > y817_band_limits[i]) break;	// We have found the band,
		} while (--i);						// otherwise return UNKNOWN.

	return i;
	}
// Check Yaesu 817 band data (voltage)
void eval_y817_band(void)
	{
	Current_Band = get_817_band(YAESU_FT_817);
	}
/*
 Xiegu ACC port band voltages. These use 230mV steps, and unlike Yaesu there is a separate level for 60m:

	160m 230mV, 80m 460mV, 60m 690mV, 40m 920mV, 30m 1150mV, 20m 1380mV, 17m 1610mV, 15m 1840mV, 12m 2070mV, 10m 2300mV

 Each threshold is midway between two adjacent levels, giving a tolerance of +/-115mV. The A-D count is mV * 4096 / 5000.
 The 60m band (5MHz) uses the 40m low-pass filter, as it does in the other band select modes. (See band_limits[], LM7)
 Below the 160m threshold the band is OUT_OF_BAND, above the 10m threshold it is NOT_KNOWN, both inhibit TX.
*/
const unsigned int xiegu_band_limits[] = {		// Thresholds, A-D counts for a 5.000V reference.
										94,		// 115mV	Below this: Out-Of-Range
										283,	// 345mV	160m / 80m
										471,	// 575mV	80m / 60m
										659,	// 805mV	60m / 40m
										848,	// 1035mV	40m / 30m
										1036,	// 1265mV	30m / 20m
										1225,	// 1495mV	20m / 17m
										1413,	// 1725mV	17m / 15m
										1602,	// 1955mV	15m / 12m
										1790,	// 2185mV	12m / 10m
										1978	// 2415mV	Above this: Unknown Band
										};

const int xiegu_bands[] = {						// Indexed by: number of thresholds exceeded
										OUT_OF_BAND,
										1,		// 160m
										2,		// 80m
										3,		// 60m, uses the 40m filter
										3,		// 40m
										4,		// 30m
										5,		// 20m
										6,		// 17m
										7,		// 15m
										8,		// 12m
										9,		// 10m
										NOT_KNOWN
										};

int get_xiegu_band(int v)
	{
	int i = 0;

	while((i < 11) && (v > (int)xiegu_band_limits[i])) i++;

	return xiegu_bands[i];
	}
// Check Xiegu band data (voltage), same input as the Yaesu FT-817 band data.
void eval_xiegu_band(void)
	{
	Current_Band = get_xiegu_band(YAESU_FT_817);
	}

// Set Gain & Filter Relays
void set_relays(void)
	{
	static int relay_band = -1;			// Band the filter relays are set for, -1 = not yet set
	static int release_wait = FALSE;	// Waiting for the PA relays to release before switching the filters
/*
 Never switch the filter relays under power. If the band changes while transmitting, RF is turned off first, and the
 relays are switched once the PA relays have released. After any band change TX is held off until the new relays have
 settled, see the TX evaluation in main(). Both use relay_settle, which is decremented in _T3Interrupt(). DL4JC
*/
	if(Current_Band != relay_band)
		{
		if(TX_ON)						// Band change while transmitting,
			{
			TX_ON = OFF;				// so RF off first,
			tx = 'R';
			relay_settle = RELAY_SETTLE;
			release_wait = TRUE;
			return;						// and switch the relays later.
			}

		if(release_wait && relay_settle) return;	// Still waiting for the PA relays to release.

		if(tx_off_ms < RELAY_SETTLE)	// RF was turned off only recently, e.g. by tx_guard() in the interrupt, and the
			{							// PA relays may still be releasing: wait, and hold TX off meanwhile.
			relay_settle = RELAY_SETTLE;
			return;
			}

		release_wait = FALSE;
		relay_band = Current_Band;
		relay_settle = RELAY_SETTLE;	// TX is held off until the new relays have settled,
		filter_mismatch = FALSE;		// and the new filter is checked again in tx_guard().
		}
// Set RF Gain relays
/*
 If we write it this way:
	DB_2 = (RF_Gain[Current_Band] & 0x01) ? 0 : 1;
	DB_4 = (RF_Gain[Current_Band] & 0x02) ? 0 : 1;
 it takes 18 bytes more code! A.Ryan - 5B4AIY - 07/JUN/2013
*/
	if(RF_Gain[Current_Band] & 0x01) DB_2 = 0; else DB_2 = 1;
	if(RF_Gain[Current_Band] & 0x02) DB_4 = 0; else DB_4 = 1;

// Set RF Filter relays, elegant method
/*
 Now here's an interesting point. If we use this expression to set/clear the relays:
 M1_8 = (Current_Band == 1);
 or if we use:
 M1_8 = (Current_Band == 1) ? 1 : 0;
 we get the same code. Interestingly, if we use:
 if(Current_Band == 1) M1_8 = 1; else M1_8 = 0;
 then the resultant code is 168 bytes shorter! A.Ryan - 5B4AIY - 07/JUN/2013

 The valid band numbers are:

	1	-	160m
	2	-	80m
	3	-	40m
	4	-	30m
	5	-	20m
	6	-	17m
	7	-	15m
	8	-	12m
	9	-	10m

 If the band number is 0, this is out-of-band, and the 10m filter is selected.
 Similarly, if the band number is 10, then the 10m filter is also selected.
*/
	if(Current_Band == 1)	M1_8 = 1;	else M1_8 = 0;		// Set/Clear 160m relay
	if(Current_Band == 2)	M3_5 = 1;	else M3_5 = 0;		// Set/Clear 80m relay
	if(Current_Band == 3)	M7 = 1;		else M7 = 0;		// Set/Clear 40m relay
	if(Current_Band == 4)	M10 = 1;	else M10 = 0;		// Set/Clear 30m relay
	if((Current_Band == 5) || (Current_Band == 6)) M14_18 = 1;	else M14_18 = 0;	// Set/Clear 20m-17m relay
	if((Current_Band == 0) || (Current_Band > 6)) M21_28 = 1;	else M21_28 = 0;	// Set/Clear 15m-12m-10m relay
	}

void check_polling(void)
	{
	if(Poll_Time && !polling_timer)						// Check if polling is enabled and timer has timed-out.
		{
		polling_timer = Poll_Time * 1000;				// Yes, so reset the timer.

		if(poll_resp_rec == FALSE)						// We have not received a valid band set response since
			Current_Band = NOT_KNOWN;					// the last query, so the band information has been lost.

		xmit_cmd(poll_cmd[Band_Select_Mode]);			// Transmit next polling request
		poll_resp_rec = FALSE;							// and reset the poll response flag.
		}
	}

void manual_band(void)
	{
	Auto_Manual = MANUAL_BAND;
	}

void hold_band(void)		// HR50 mode: the band is held until the host sends a new one, see serial_hr50().
	{
	}

void (*select_auto_band[])(void) = {					// Indexed by: Band_Select_Mode
									check_polling,		// 0 Yaesu 5-Byte Binary (FT-817/FT-818)
									check_polling,		// 1 KX-3
									check_polling,		// 2 TRX-2
									eval_band,			// 3 Frequency Sense Mode
									eval_y817_band,		// 4 FT-817 Mode
									manual_band,		// 5 Manual Band Selection
									eval_xiegu_band,	// 6 Xiegu Mode
									hold_band			// 7 HR50 Mode
									};

int scaled_temperature(double t)
	{
	t /= temp_cal_factor[Temp_Scale];

	if(Temp_Scale == FAHRENHEIT) t += 32.0;

	return (int)(floor(t + 0.5));
	}

// Measure RF Power and PA Temperature
void analog_measurements(void)
	{
	int i = 0;
	static long fwd_avg, rev_avg;

	_T3IE = 0;												// Take a consistent snapshot of all channels, the
															// conversions are made in the 1mS interrupt. (See adc12.c)
	do	{
		AD_Values[i] = adc_raw[i];
		} while (++i < 6);

	_T3IE = 1;

// Measure temperature
	pa_temp = (pa_temp * 0.99) + ((double)hs_temp * 0.01);	// 100-sample running average
	scaled_pa_temp = scaled_temperature(pa_temp);

// Convert fwd & rev A/D channels
	if(sample_counter)
		{
		sample_counter--;
		fwd_avg += (long)(forward_pwr + cal.calval.lo_pwr_offset);	// forward_pwr = AD_Values[4]
		rev_avg += (long)(reverse_pwr);								// reverse_pwr = AD_Values[3] 
		}
	else
		{
		fwd_avg /= (long)cal.calval.samples;
		rev_avg /= (long)cal.calval.samples;
		fwd_pwr = fwd_avg * fwd_avg;
		rev_pwr = rev_avg * rev_avg;
		out_pwr = fwd_pwr - rev_pwr;
		vswr = calc_swr(fwd_avg, rev_avg);
		swr = (int)vswr;
		fwd_avg = rev_avg = 0L;
		sample_counter = (int)cal.calval.samples;
		}
	}

void max_min(int *value, int max, int min)
	{
	if(*value > max) *value = max;
	if(*value < min) *value = min;
	}
/*
 PA cooling fan control
 This slightly more complex method avoids unnecessary switching of the fan as we progress
 through the various choices. If we simply turned the various control bits on as we climbed
 through the temperature range, on each pass through the main loop we would sequentially switch
 the various fan control bits which might generate a transient. This method ensures that we select
 the desired fan speed immediately, and only switch when necessary. The Fan Control logic now has
 an additional parameter. The user can select whether the fan is on all the time, and the speed
 at which it will run. In the User Configuration menu, the Fan Control option can be set to Normal,
 in which case the fan speed will be controlled entirely by the heat-sink temperature, or Low,
 Medium, or High, in which case the fan will run at the selected speed all the time, and only switch
 to a higher speed as the heat sink temperature rises. Obviously, if you select 'High', then the fan
 will run at this speed all the time. Adrian Ryan - 5B4AIY

 In order to avoid "fan-twitch", the fan speed control is a static variable that retains its value.
 Fan-twitch occurred because as the temperature decreased, with no hysteresis, the fan would be commanded
 on and off repeatedly. The fan_stop parameter is calculated once at start-up, or whenever either the
 temperature scale or the start temperature is changed. Adrian Ryan, 5B4AIY
 
 Revised the structure so that it is more efficient and elegant, resulting in a code saving of 42 bytes
 compared with the previous version. OK, so I'm OCD, but I had nothing else better to do during the
 COVID-19 lock-down. A.Ryan - 5B4AIY - 27/APR/2010
*/

void fan_control(void)
	{
	int medium_speed, high_speed;
	
	if(Temp_Scale)	// Centigrade Temperature Scale
		{
		medium_speed = Fan_Start + PLUS_5C;
		high_speed  = Fan_Start + PLUS_10C;
		}
	else			// Fahrenheit Temperature Scale
		{
		medium_speed = Fan_Start + PLUS_10F;
		high_speed = Fan_Start + PLUS_20F;
		}

	fan_speed = OFF;

	if(scaled_pa_temp >= Fan_Start) fan_speed = LOW_SPEED;

	if(scaled_pa_temp >= medium_speed) fan_speed = MEDIUM_SPEED;

	if(scaled_pa_temp >= high_speed) fan_speed = HIGH_SPEED;

	fan_speed += Fan_Speed;		// Select whether the fan runs continuously
	max_min(&fan_speed, HIGH_SPEED, OFF);
	FAN1 = (fan_speed & 0x02) ? ON : OFF;
	FAN2 = (fan_speed & 0x01) ? ON : OFF;
	}

void display_hdr(void)
	{
	printf(firmware, Juma_PA100, VERSION, BUILD_DATE);
	printf(copyright);
	printf(additional_features);
	printf(dl4jc_features);
	printf(System_Clk_Msg, CLK_FRQ);
	}

void set_value(int multiplier, int *value, int max, int min)
	{
	if(adjust_flag) *value += (encoder_get() * multiplier);

	max_min(value, max, min);
	}

void get_one_zero(int *value)
	{
	set_value(1, value, 1, 0);
	}
/*
 The following functions are the user configuration code from the previous version. In all previous versions the actual
 code selected was part of a long switch/case construct that was difficult to read. It was also easy to get lost
 among the various indent levels. One of the attributes of elegant well-written code is the overall feature of burying
 complexity. The higher levels of the code should be simple to read and easy to modify. To this end I decided to remove
 this complex switch/case construct and replace it with a single line of code. This is achieved by placing each page's
 code into a small function, and then collecting these functions into an array, and finally executing the appropriate
 function using the page as an index into the array. Adrian Ryan - 5B4AIY May 2012.
*/
/*
 Menu order of the band select modes. The stored values are unchanged (MANUAL = 5, XIEGU = 6), so that the saved settings
 remain compatible with the original firmware, but the menu shows Xiegu before Manual. DL4JC
*/
const int bsel_menu[] = {YAESU, ELECRAFT_KX3, HR50, JUMA_TRX2, FREQ_SENSE, FT_817, XIEGU, MANUAL};

void cfg_0(void)	// Auto band select mode (0 = Yaesu CAT, 1 = Elecraft KX-3, 2 = Juma TRX-2, 3 = F-Sense, 4 = FT-817, 5 = Manual, 6 = Xiegu, 7 = HR50)
	{
	static int current_mode;
	int pos = 0;

	current_mode = Band_Select_Mode;

	while((pos < MAX_BSEL_MODE) && (bsel_menu[pos] != Band_Select_Mode)) pos++;	// Find the menu position of the current mode,

	set_value(1, &pos, MAX_BSEL_MODE, 0);		// adjust it,
	Band_Select_Mode = bsel_menu[pos];			// and select the mode at the new position.

	if(current_mode != Band_Select_Mode)		// The mode has changed,
		{
		if(Band_Select_Mode < 2 && !Poll_Time)	// so if either the Yaesu or the KX3 mode is selected, and the polling timer is disabled,
			Poll_Time = POLL_TIMER;				// set the polling time to a default value (2 seconds).

		if(Band_Select_Mode == JUMA_TRX2)		// If the TRX-2 mode is selected,
			Poll_Time = DISABLED;				// then disable the polling timer. This can still be over-ridden.

		if(Band_Select_Mode == HR50)			// The HR50 mode does not poll. Polling is also disabled for the original
			Poll_Time = DISABLED;				// firmware, which reads this mode as KX2/KX3, see save_defval().

		if(Band_Select_Mode == MANUAL)
			Current_Band = MAX_BAND;			// Ensure that we have a valid band selected.
		}
	
	_rs232_mode();								// Set appropriate serial mode
	max_page = (Band_Select_Mode == FREQ_SENSE) ? MAX_SUB_PAGE0 + 1 : MAX_SUB_PAGE0;	// Enable the frequency counter page in the F-Sense mode.
	sprintf(lcdpbuff, cfg_0_Msg, f_sense[Band_Select_Mode]);
	}

void cfg_1(void)						// Set Serial Port Speed (1200 - 115200)
	{
	set_value(1, &eeprom.defval.br, 7, 0);
	SetUSART1baud(eeprom.defval.br);	// Set new Baud rate
	sprintf(lcdpbuff, cfg_1_Msg, br_txt[eeprom.defval.br]);	// Display current selection - Added size of string specifier - 5B4AIY
	}

void cfg_2(void)						// Serial Port Mode Selection 0 = Off, 1 = Remote, 2 = Serial Test
	{
	set_value(1, &Serial_Test_Mode, SERIAL_TEST, OFF);
	_rs232_mode();						// Set appropriate serial mode
	sprintf(lcdpbuff, cfg_2_Msg, rs232_mode_msg[Serial_Test_Mode]);
	}

void cfg_3(void)						// Set Polling Interval Time
	{
	set_value(1, &Poll_Time, MAX_POLL_TIMER, OFF);

	if(Poll_Time)						// Improved display - 5B4AIY
		sprintf(lcdpbuff, cfg_3_Msg1, Poll_Time);
	else
		sprintf(lcdpbuff, _16s, cfg_3_Msg2);
	}

void cfg_4(void)						// Set Display Backlighting Level
	{
	set_value(10, &eeprom.defval.back_light, MAX_BL, MIN_BL);
	set_pwm3_dac(eeprom.defval.back_light);
	sprintf(lcdpbuff, cfg_4_Msg,eeprom.defval.back_light);	// Alternative display - 5B4AIY
	}

void cfg_5(void)						// Set Display Contrast Setting
	{
	set_value(50, &eeprom.defval.contrast, MAX_CONTRAST, MIN_CONTRAST);
	set_pwm4_dac(eeprom.defval.contrast);
	sprintf(lcdpbuff, cfg_5_Msg, eeprom.defval.contrast);	// Alternative display - 5B4AIY
	}

void cfg_6(void)						// Set SWR Trip Limit (1.0 - 9.0)
	{
	set_value(10, &SWR_Trip, MAX_SWR, MIN_SWR);
	sprintf(lcdpbuff, cfg_6_Msg, ((double)SWR_Trip) / 100);	// Improved formatting - 5B4AIY
	}

void cfg_7(void)						// Select Fan Control Mode (Normal/Low/Medium/High)
	{
	set_value(1, &Fan_Speed, HIGH_SPEED, NORMAL_SPEED);
	sprintf(lcdpbuff, cfg_2_Msg, fan_spd[Fan_Speed]);
	}

void cfg_8(void)						// Select Display Temperature Units (C/F)
	{
	get_one_zero(&Temp_Scale);

	if(Temp_Scale != Current_Scale)
		{
		Current_Scale = Temp_Scale;

		if(Temp_Scale)
			{
			Alarm_Temp = TEMP_LIMITC;
			Fan_Start = FAN_STARTC;
			}
		else
			{
			Alarm_Temp = TEMP_LIMITF;
			Fan_Start = FAN_STARTF;
			}
		}
	sprintf(lcdpbuff, units, c_or_f[Temp_Scale]);
	}

void cfg_9(void)						// Set Over Temperature Limit
	{
	if(Temp_Scale) set_value(1, &Alarm_Temp, MAX_TEMPC, MIN_TEMPC);
	else set_value(1, &Alarm_Temp, MAX_TEMPF, MIN_TEMPF);

	sprintf(lcdpbuff, temperature_fmt, limit, Alarm_Temp, T_Char[Temp_Scale]);	// Better formatting - 5B4AIY
	}

void cfg_10(void)						// Set Fan Cut-In Temperature
	{
	if(Temp_Scale)		// Celsius
		{
		set_value(1, &Fan_Start, (Alarm_Temp - 20), 0);
		fan_stop = Fan_Start - 2;
		}
	else								// Fahrenheit
		{
		set_value(1, &Fan_Start, (Alarm_Temp - 40), 32);
		fan_stop = Fan_Start - 4;
		}

	sprintf(lcdpbuff, temperature_fmt, start, Fan_Start, T_Char[Temp_Scale]);	// Better formatting - 5B4AIY
	}

void cfg_11(void)						// Select Band Units (MHz/Metres)
	{
	get_one_zero(&Band_Units);
	sprintf(lcdpbuff, units, band_units[Band_Units]);
	}

void cfg_12(void)						// Select graphical display of parameter limits. 
	{
	get_one_zero(&eeprom.defval.graph_limits);
	sprintf(lcdpbuff, cfg_12_Msg, on_off[eeprom.defval.graph_limits]);
	}

void cfg_13(void)						// Select type of graphic display scale
	{
	set_value(1, &Scale_Type, SMALL, ORIGINAL);
	sprintf(lcdpbuff, cfg_13_Msg, graph_type[Scale_Type]);
	}

void cfg_14(void)						// RF Power Meter Display, Watts/dBm
	{
	get_one_zero(&Power_Units);
	sprintf(lcdpbuff, cfg_13_Msg, pwr_mtr[Power_Units]);
	}

void cfg_15(void)						// Start-Up Page Select
	{
	set_value(1, &Start_Page, TEMPERATURE, DEFAULT_PWR);
	sprintf(lcdpbuff, cfg_15_Msg, start_page_select[Start_Page]);
	}
/*
 F-Sense QSK On/Off. Off: in the F-Sense mode TX is only enabled once the input frequency has been measured in the current
 transmission, so the PA never amplifies through the wrong filter, at the cost of approx. 20-40mS without the PA at the
 start of each transmission. On: TX is enabled immediately, suitable for full QSK; the filter is then protected by
 tx_guard() in timers_pwm.c, which leaves a window of a few mS after a band change. DL4JC
*/
void cfg_16(void)
	{
	get_one_zero(&FSense_QSK);
	sprintf(lcdpbuff, cfg_2_Msg, on_off[FSense_QSK]);
	}

void (*set_cfg[])(void) = {				// Indexed by: sub_page1
						cfg_0,			// 0	Auto Band Select Mode (0 = Yaesu, 1 = Elecraft KX-3, 3 = Juma TRX-2, 4 = F-Sense, 5 = FT-817)
						cfg_1,			// 1	Set Serial Port Speed (1200 - 115200)
						cfg_2,			// 2	Set Serial Port Mode, 0 = Off, 1 = Remote, 2 = Test
						cfg_3,			// 3	Set Polling Interval Time (Off - 10S)
						cfg_4,			// 4	Set Display Backlighting Level (50 - 1000)
						cfg_5,			// 5	Set Display Contrast Setting (0 - 3000)
						cfg_6,			// 6	Set SWR Trip Limit (1.0 - 10.0)
						cfg_7,			// 7	Select Fan Control Mode (Normal/Low/Medium/High)
						cfg_8,			// 8	Select Display Temperature Units (C/F)
						cfg_9,			// 9	Set Over-Temperature Limit
						cfg_10,			// 10	Set Fan Cut-In Temperature
						cfg_11,			// 11	Select Band Units (MHz/Metres)
						cfg_12,			// 12	Select Graphical Limits Display (On/Off)
						cfg_13,			// 13	Select Graphical Display Scale (Original/Large/Small)
						cfg_14,			// 14	Select RF Power Meter Display, (Watts/dBm)
						cfg_15,			// 15	Start-Up Page select (Power/SWR/Voltage/Current/Temperature)
						cfg_16			// 16	F-Sense QSK (On/Off), only in the F-Sense mode
						};

void display_cfg_page(void)
	{
	sprintf(lcdpbuff, _16s, page_prompt[sub_page1]);	// Select page prompt,
	display_line(LINE1, lcdpbuff);			// and display it on line 1.
	set_cfg[sub_page1]();					// Execute selected page code,
	display_line(LINE2, lcdpbuff);			// and display returned value on line 2.
	}

void change_cfg_page(int direction)
	{
	sub_page1 += direction;

	if(sub_page1 > MAX_SUB_PAGE1) sub_page1 = 0;
	if(sub_page1 < 0) sub_page1 = MAX_SUB_PAGE1;

	if((Band_Select_Mode != FREQ_SENSE) && (sub_page1 == FSENSE_QSK_PAGE))	// The F-Sense QSK page is only shown in the
		sub_page1 += direction;												// F-Sense mode.

	if(sub_page1 > MAX_SUB_PAGE1) sub_page1 = 0;
	if(sub_page1 < 0) sub_page1 = MAX_SUB_PAGE1;

	if((Band_Select_Mode > 2) && (Serial_Test_Mode != REMOTE) && (sub_page1 == 3))		// if the F-SENSE/FT-817 Modes are selected, and Serial Port is not in REMOTE,
		sub_page1 += direction;							// then skip the Polling Timer page

	if(BAND_FROM_SERIAL && (sub_page1 == 2))			// JUMA-TRX2/KX3/YAESU/HR50 Mode and
		sub_page1 += direction;							// then skip the Serial Test On/Off page

	while((Band_Select_Mode == HR50) && ((sub_page1 == 2) || (sub_page1 == 3)))	// The HR50 mode also skips the Polling Timer page,
		sub_page1 += direction;													// in both directions.

	if(sub_page1 > MAX_SUB_PAGE1) sub_page1 = 0;
	if(sub_page1 < 0) sub_page1 = MAX_SUB_PAGE1;

	set_repeat_speed();
	display_beeps(sub_page1);
	}

void cfg_page_change(int direction)
	{
	change_cfg_page(direction);
	adjust_flag = FALSE;		// Prevent spurious adjustments if UP/DOWN button is held
	display_cfg_page();			// whilst fast page repeat is in operation.
	adjust_flag = TRUE;			// Restore normal adjustment.
	ms_delay(PAGE_CHANGE);
	enc = 0;					// Reset encoder.
	}

void range_beep(int value, int upper_limit, int lower_limit)
	{
	if(value == upper_limit) beep(HZ698_45, LONG_BEEP);			// End-of-range F tone beep
	else if(value == lower_limit) beep(HZ466_85, LONG_BEEP);	// End-of-range B-flat tone beep
	else beep(HZ587_31, Beep_Time);								// Normal D tone beep
	}

// main __________________________________________________________________________________
int main(void)
	{
	int atten;						// Added - 5B4AIY Used to set the input attenuator.
#if	LOOP_TIME
	int loop_counter = 3125;
#endif
	unsigned int y, w;

// Set general I/O
	TRISA = INIT_TRISA;				// SWITCHES, PTT
	PORTA = INIT_PORTA;

	TRISB = INIT_TRISB;				// ADC inputs, TXEN, RXEN, General I/O
	PORTB = INIT_PORTB;

	TRISC = INIT_TRISC;				// Sideband select & switches
	PORTC = INIT_PORTC;

	TRISD = INIT_TRISD;				// LEDs & LCD pins = output
	PORTD = INIT_PORTD;

	TRISF = INIT_TRISF;				// DDS controls
	PORTF = INIT_PORTF;

	TRISG = INIT_TRISG;				// DDS controls, EEPROM, DScard signals
	PORTG = INIT_PORTG;
// Power switch
	PWR_ON = ON;					// Latch power ON
// Test fan
	FAN2 = ON;						// Test FAN2, low speed on
// Start PWM system
	IPC0 = 0x5444;					// Set tone generator (TMR1) priority higher than others
	IPC1bits.T2IP = 5;				// The tone generator is TMR2. Its priority must be above the 1mS TMR3 interrupt (4),
									// which now includes the A-D conversions, otherwise the tones would be distorted.
	IPC2bits.U1RXIP = 6;			// The UART receive interrupt is very short, so give it the highest priority to avoid
									// receive FIFO overruns while the other interrupts are running.
	init_timers_pwm();				// Setup PWM & tone generators

// Init Main Board SPI traffic
//	init_spi1();					// Not used in this application

// Start LCD
	initlcd();						// Initialise LCD with default contrast and backlighting
// Start UART
	InitUSART1();					// Wake up serial interface
// Start ADC
	init_adc12();					// Setup A/D converter, must be done before Keyer is started, RB I/O bits
// Read calibration & default values from EEPROM
	y = read_calval();
	w = read_defval();

	if(read_extval()) save_extval();	// Extension block missing (first start of this firmware) or invalid: save the defaults.

	load_ext_modes();					// Restore a band select mode stored in the extension block (Xiegu)

 	if((Serial_Test_Mode == SERIAL_TEST) && !BAND_FROM_SERIAL)
		printf(EEPROM_Chksum, checksum_msg[y], checksum_msg[w], fd_counter);

	if(y || w)						// Test return flags. If either is non-zero, then there is an EEPROM read fault
		{
		dump_eeprom();				// Test, show EEPROM content before reset
		printf(Chksum_Err);
		display_screen(Chksum_Err_Msg, Loading_Defaults);
		set_factory_defaults();		// EEPROM empty or corrupted, set factory defaults
		save_calval();
		save_defval();
		ms_delay(1500);
		}
// Ensure O/C, SWR and Temperature alarms are enabled. (Only required if user has not updated the alarm mask)
	Enabled_Alarms |= 0x07;
// Check if factory defaults asked
	if(!BAND_UP) save_settings(1, 7);	// Check for BAND+ button press
// Set LCD contrast & back light with EEPROM data
	set_pwm4_dac(eeprom.defval.contrast);
	set_pwm3_dac(eeprom.defval.back_light);
	set_chgen(Scale_Type);	// Load bar graph fonts
// Check for extension of KX3 message timer for debug purposes
	if(!BAND_DN)
		{
		msg_time = 5000;
		display_screen(KX3_msg1, KX3_msg2);

		while (!BAND_DN);			// Wait for button release
		}
// Put hello messages to LCD if Splash Screen flag is ON
	if((int)cal.calval.splash)
		{
		sprintf(lcdpbuff, Hello, Juma_PA100, VERSION);
		display_screen(lcdpbuff, OH2NLT_OH7SV);
		y = 2000;					// and set sign-on prompt delay time
		}
// Check if service mode start
	button_timer = VERY_LONG_PUSH;	// Set button timer

	while (PWR_SW)					// While PWR button is pressed...
		{
		if(!button_timer)
			{
			beep(HZ392_01, LONG_BEEP);	// Give service beep
			sprintf((char *)lcdpbuff, Mode_msg, VERSION);
			display_screen(Calibration_msg, lcdpbuff);
			wait_PWR_rls();			// Wait for PWR button to be released...
			display_screen(Display_Next, Oper_Save);
			ms_delay(2000);			// and wait for 2 seconds
			svc_flag = TRUE;
			Current_Band = MAX_BAND;
			service(svc_flag);
			y = 0;					// Set sign-on prompt delay. (No delay required)
			}
		}
// General settings
	if(Current_Band > MAX_BAND)						// If last shut down was with undefined frequency
		Current_Band = MAX_BAND;
// Start-up beep
	if(!svc_flag) beep(HZ698_45, SHORT_BEEP * 2);	// Give start-up beep
	svc_flag = FALSE;
	FAN1 = ON;										// High speed on, fan test phase 2
	FAN2 = OFF;										// Test FAN2, low speed off
	OC_CLR = 0;										// Clear over-current latch, start pulse
	ms_delay(y);									// Also used as sign-on prompt delay time
	OC_CLR = 1;										// End over-current latch pulse
	FAN1 = OFF;										// Fan test end
	FAN2 = OFF;
// Set user baud rate
	SetUSART1baud(eeprom.defval.br);
// Conditional start up text printout, print only if Serial Test Mode is On
  	if((Serial_Test_Mode == SERIAL_TEST) && !BAND_FROM_SERIAL)
		display_hdr();
// RS-232 I/O Test
	if(!DISP) rs232_test();							// If DISPLAY pressed during startup, Goto RS-232 test loop.
// Other
	Current_Scale = Temp_Scale;						// 0 = Fahrenheit, 1 = Celsius

	batt_pre_limit = (Enabled_Alarms & LO_V)
		? cal.calval.pre_limit_trip
		: OFF;

	last_man_band = (Auto_Manual == MANUAL_BAND)
		? Current_Band
		: TEN_METRES;

	fan_stop = (Fan_Start - 4) + (2 * Temp_Scale);	// Hysteresis to stop fan 'twitch', 4F or 2C

	if(Band_Select_Mode == FREQ_SENSE) max_page = MAX_SUB_PAGE0 + 1;					// Allow extended Frequency Display page.

	clear_buffer();								// Initialise the PA-100D receive buffer and index.
	batt_avg = ((double)convert_adc12(BATT_CH) * (double)Voltmeter_Cal);				// Initial sample for battery voltage, mV
	pa_temp = (double)convert_adc12(TEMP);		// Initial heat-sink temperature value
	pwr_scale_factor = (SCALE_CONST * cal.calval.max_power / cal.calval.fwd_pwr_mult);	// See explanation in juma-pa100.h
	batt_scale_factor = cal.calval.overvoltage_trip - cal.calval.undervoltage_trip;
	_rs232_mode();								// Select serial port protocol
	rep_dly = _SLOW;
	decay_counter = 0;							// This seems to help eliminate the spurious 0.6W power
	fwd_pwr = rev_pwr = 0L;						// display when exiting the service mode. A.Ryan 13/FEB/2015
	sub_page0 = Start_Page;

// Main forever loop _____________________________________________________________________________________________________________
	do	{
/*
 Test counter to determine loop cycle time.
 With the power sampling set to 64, each loop takes 13.1 seconds, thus, 4.19mSec / iteration.
 With the power sampling set to 32, each loop takes 13.0 seconds, thus, 4.16mSec / iteration.
 With the power sampling set to 1, each loop takes 11.6 seconds, thus, 3.56mSec / iteration.
*/
#if	LOOP_TIME
		loop_counter--;

		if(!loop_counter)
			{
			beep(HZ2000, SHORT_BEEP / 5);
			loop_counter = 3125;		// This should give a 10-second loop
			}
#endif
/*
 Loop Cycle Time - Measured using an oscilloscope on J19-5, Version 2.0a, Build 5, 23/SEP/2015
 Mode: TRX-2/KX3/F-Sense
 Display
 PWR		4.5mS
 SWR		4.4mS
 Volts		5.3mS
 Amps		5.6mS
 Temp		5.2mS
 Freq/Pwr	4.5mS (Only with F-Sense)
 Mode: FT-817
 PWR		5.0mS
 SWR		4.9mS
 Volts		5.9mS
 Amps		6.2
 Temp		5.8
*/ 
		__builtin_btg((unsigned int *)&LATB, 1);	// MAIN_TEST, timing test J19-5. Check with oscilloscope for reversals.
										// A single btg instruction: 'MAIN_TEST = !MAIN_TEST' compiled to a read-modify-write
										// of the whole LATB, which could switch TX_ON (LATB4) on again just after tx_guard()
										// had turned it off in the 1mS interrupt. DL4JC
		main_heartbeat = 0;				// Main loop is running, see tx_guard() in timers_pwm.c
		key = KEY;						// Copy I/O bit to status flag, this uses less code than using the I/O bit directly.

		if(!key) filter_mismatch = FALSE;	// The filter mismatch flag is reset at the end of each transmission.

		if(Band_Select_Mode == FREQ_SENSE && !FSense_QSK)	// F-Sense with QSK Off: TX only after the band has been measured
			{												// in this transmission.
			if(!key) fsense_confirmed = FALSE;
			else if(!last_key) reset_fsense();				// Start a new measurement when KEY becomes active.
			}
		last_key = key;
// Auto band select
		select_auto_band[Band_Select_Mode]();				// Get the current auto band selection,
// Measurements & basic calculations
		analog_measurements();								// Measure RF Power, PA Temperature, and SWR
// Check alarm conditions
		check_alarms();										// Measure Voltage, Current, and check if any alarms
		button_timer = LONG_PUSH;							// Set button timer
// Do front panel switches
// DISPLAY / CONFIG
		if(!DISP && !alarms)								// SW1
			{
			do	{
				if(!button_timer)
					{
					if(lcd_mode == NORMAL_DISPLAY_MODE)
						{
						beep(HZ466_85, LONG_BEEP);			// Long beep for major mode change
						lcd_mode = USER_CONFIG_MODE;
						display_screen(User_Msg, Config_Msg);
						set_repeat_speed();
						while (!DISP);						// Wait for button release...
						ms_delay(BUTTON_DEBOUNCE);			// This prevents spurious page change on button release.
						}
					else cfg_page_change(INCREMENT);		// Repeat User Configuration page change whilst button is held...
					}
				} while (!DISP);							// Wait for button release...

			ms_delay(BUTTON_DEBOUNCE);

			if(button_timer)
				{
				if(lcd_mode == NORMAL_DISPLAY_MODE)			// Select sub page to adjust
					{										// Normal display pages
					sub_page0++;
					sub_page0 %= (max_page + 1);
					display_beeps(sub_page0);
					}
				else change_cfg_page(INCREMENT);			// Single User Configuration page change
				}
			encoder_get();									// Clear encoder after display change
			}

		if(lcd_mode == NORMAL_DISPLAY_MODE)					// Normal Operating Mode
			{
			if(!alarms)										// No alarms
				{
// OPER
				if(!OPER)									// SW3 - Do not allow change of state in User Config Mode
					{
					pa_state ^= 1;							// Toggle PA State. (Standby = 0, Operate = 1)
					beep((pa_state) ? HZ587_31 : HZ466_85, Beep_Time);
					while (!OPER);							// Wait for button release...
					ms_delay(BUTTON_DEBOUNCE);
					}
// AUTO
// 11.11.2008 toggle logic
				if(!AUTO && (Band_Select_Mode != MANUAL))	// SW2 - AUTO mode cannot be selected if Manual Band selection is in effect.
					{
					Auto_Manual ^= 1;						// Toggle AUTO/MANUAL Band select. (Manual = 0, Auto = 1)
					not_used = TRUE;						// There has been a change of state, so force a frequency query
					changed = TRUE;							// Set configuration changed flag

					if(Auto_Manual)							// AUTO
						{
						reset_fsense();						// Reset F-sense logic,
						last_man_band = Current_Band;		// Save current manual band selection,
						Current_Band = NOT_KNOWN;			// and set band select to UNKNOWN.
						}
					else Current_Band = last_man_band;		// MANUAL, so restore last known manual band

					beep((Auto_Manual) ? HZ466_85 : HZ587_31, Beep_Time);
					while (!AUTO);							// Wait for button release...
					ms_delay(BUTTON_DEBOUNCE);
					}
// BAND+ / BAND-
				if((!BAND_UP || !BAND_DN) && !key)			// SW6 - BAND+ SW4 - BAND- Do not allow manual band changes in TX mode!
					{
					changed = TRUE;

					if(Auto_Manual)							// AUTO
						{
						Auto_Manual = MANUAL_BAND;			// So, change to MANUAL band select mode,
						Current_Band = last_man_band;		// and restore last known manual band.
						beep(HZ587_31, Beep_Time);			// B-flat tone beep
						}
					else									// MANUAL
						{
						do	{
							Current_Band += (!BAND_UP) ? INCREMENT : DECREMENT;
							max_min(&Current_Band, MAX_BAND, MIN_BAND);
							range_beep(Current_Band, MAX_BAND, MIN_BAND);
							display_line(LINE2PLUS10, (Band_Units) ? mb_txt[Current_Band] : bs_txt[Current_Band]);
							ms_delay(PAGE_CHANGE);			// Also serves as the button debounce
							} while (!BAND_UP || !BAND_DN);	// Wait for button release...

						last_man_band = Current_Band;		// Set the new band.
						}
					not_used = TRUE;						// Set the flag to force a frequency query command in the Yaesu/KX3/TRX-2 mode
					}
				}
// Do LCD display
			lcd_cmd(LINE1);							// Start from line 1
// Normal display
// First draw graphical meter if this is not the Extended Frequency Display page
			if(sub_page0 < 5) draw_s_meter(scaled_value());

			display_page[sub_page0]();				// Display selected sub-page
			display_line(LINE2, gain[RF_Gain[Current_Band]]);	// Display gain setting
// Show PA state if no alarms, otherwise display alarms
			if(alarms) prompt_msg = display_alarms();// Get alarm message prompt, if active.
			else
				{
				if(pa_state)						// If there are no alarms, and the PA State is OPERATE,
					{
					if(key)							// and there is a transmit request,
						{
						if((Current_Band != NOT_KNOWN) && (Current_Band != OUT_OF_BAND))	// and a valid band is selected,
							prompt_msg = TX_Msg;	// then select the TX message,
						else
							prompt_msg = Err_Msg;	// otherwise select the Error message.
						}
					else prompt_msg = OPER_Msg;		// In the idle state, select the OPERATE message.
					}
				else prompt_msg = STBY_Msg;			// The PA State is STANDBY, so select the STBY message,
				}

			lcd_putst(prompt_msg);					// and display the message.
// Show frequency select mode
			lcd_putst(auto_man[Auto_Manual]);
// Show selected band
			lcd_putst((Band_Units) ? mb_txt[Current_Band] : bs_txt[Current_Band]);
// Adjust gain setting, buttons active only in normal display
			atten = encoder_get();

			if(atten)
				{
				changed = TRUE;
				RF_Gain[Current_Band] += (atten > 0) ? INCREMENT : (atten < 0) ? DECREMENT : 0;
				max_min(&RF_Gain[Current_Band], MAX_GAIN, MIN_GAIN);
				range_beep(RF_Gain[Current_Band], MAX_GAIN, MIN_GAIN);
				}
//____________________________________________________________
// Do I/O
// Main board relays
			set_relays();							// Set gain & filter relays
// Evaluate TX possibility
			if((key) && (pa_state) && (!alarms) && (Current_Band != NOT_KNOWN) && (Current_Band != OUT_OF_BAND)
				&& (!relay_settle) && (!filter_mismatch)
				&& ((Band_Select_Mode != FREQ_SENSE) || FSense_QSK || fsense_confirmed))	// and with F-Sense QSK Off the band has been measured,
				{
				TX_ON = TRANSMIT;					// RF on
				tx = 'T';							// State is Transmit
				}
			else	// No TX request, or state is STDBY, or there are alarms, or an invalid band is selected, so,
				{
				TX_ON = OFF;						// RF Off
				tx = 'R';							// State is RECEIVE
				}
			}
		else										// User Configuration Mode
			{
			pa_state = STANDBY;						// Force state to Standby in User Configuration mode
			TX_ON = OFF;							// and ensure RF is off, TX_ON is not otherwise updated in this mode.
			tx = 'R';
			display_cfg_page();

			if(!OPER)								// In User Configuration Mode, OPER is the Save & Exit button
				{
				beep(HZ587_31, Beep_Time);
				save_settings(0, 2);				// Prompt to save User Configuration Settings,
				lcd_mode = NORMAL_DISPLAY_MODE;		// and return to normal display mode.
				rep_dly = _SLOW;					// Slow repeat for Gain changes
				encoder_get();						// Clear encoder after display change
				clear_buffer();						// Flush the serial buffer
/*
 This fixes a subtle bug in the page display logic. If the Band Select mode was F-Sense, and the frequency display page
 was displayed, and then the mode was changed and saved, the frequency display page was still displayed until the page
 was changed. Now the displayed page is the start page. A.Ryan - 5B4AIY - 26/AUG/2023
*/
				if((Band_Select_Mode != FREQ_SENSE) && (sub_page0 == 5)) sub_page0 = 0;
				}
			}
// Fan control
		fan_control();
// Run serial interface functions
		rs232_mode();
// Power switch
		if(PWR_SW)									// SW7
			{
			if(lcd_mode == USER_CONFIG_MODE)		// Increment/Decrement the current User Configuration page
				{
				do	{
					cfg_page_change(DECREMENT);
					} while (PWR_SW);				// Wait for button release...
				}
			else power_off();						//  SW7 - Go to Cancel Alarm/Shut-down/Decrement Display Page
			}
		} while (TRUE);
	}	// End main

