/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptor Header for Joystick Mouse Demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
//#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
//#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
//#define USB_STRING_DESCRIPTOR_TYPE              0x03
//#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
//#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define MIDI_SIZ_CONFIG_DESC_INNER				101
#define JOYSTICK_SIZ_CONFIG_DESC_INNER			25
#define JOYSTICK_CONFIG_DESC_OFFSET				MIDI_SIZ_CONFIG_DESC_INNER


#define JOYSTICK_SIZ_DEVICE_DESC                18
#define JOYSTICK_SIZ_CONFIG_DESC                126
#define JOYSTICK_SIZ_STRING_LANGID              4
#define JOYSTICK_SIZ_STRING_VENDOR              26
#define JOYSTICK_SIZ_STRING_PRODUCT             20
#define JOYSTICK_SIZ_STRING_SERIAL              24


#define MIDI_SIZ_DEVICE_DESC                JOYSTICK_SIZ_DEVICE_DESC    
#define MIDI_SIZ_CONFIG_DESC                JOYSTICK_SIZ_CONFIG_DESC    
#define MIDI_SIZ_STRING_LANGID              JOYSTICK_SIZ_STRING_LANGID  
#define MIDI_SIZ_STRING_VENDOR              JOYSTICK_SIZ_STRING_VENDOR  
#define MIDI_SIZ_STRING_PRODUCT             JOYSTICK_SIZ_STRING_PRODUCT 
#define MIDI_SIZ_STRING_SERIAL              JOYSTICK_SIZ_STRING_SERIAL  


#define STANDARD_ENDPOINT_DESC_SIZE             0x09



//定义的端点类型
#define ENDPOINT_TYPE_CONTROL           0x00  //控制传输
#define ENDPOINT_TYPE_ISOCHRONOUS       0x01  //同步传输
#define ENDPOINT_TYPE_BULK              0x02  //批量传输
#define ENDPOINT_TYPE_INTERRUPT         0x03  //中断传输

//类特殊接口描述符类型
#define CS_INTERFACE_DESCRIPTOR 0x24

//类特殊端点描述符类型
#define CS_ENDPOINT_DESCRIPTOR 0x25

//音频类
#define AUDIO                   0x01
//音频控制子类
#define AUDIO_CONTROL           0x01
//头描述符子类
#define HEADER                  0x01

//MIDI流子类
#define MIDISTREAMING           0x03

//MS头描述符号子类
#define MS_HEADER               0x01

//MIDI_IN_JACK子类
#define MIDI_IN_JACK         0x02

//EMBEDDED类
#define EMBEDDED             0x01

//EXTERNAL类
#define EXTERNAL             0x02

//MIDI_OUT_JACK子类
#define MIDI_OUT_JACK        0x03

//MS_GENERAL子类
#define MS_GENERAL           0x01

//实体数
#define NUMBER_OF_ENTITY        1

//插口数
#define NUMBER_OF_JACK          1

//MIDI接口数
#define NUMBER_OF_MIDI_INTERFACE 1


#define MAIN_POINT_OUT 				0x01
#define MAIN_POINT_IN 				0x81

#define MIDI_JACK_SIZE				4

#define JOYSTICK_POINT_IN			0x82
#define HID_DESCRIPTOR_TYPE			0x21
#define REPORT_DESCRIPTOR_TYPE		0x22

#define JOYSTICK_REPORT_DESC_SIZE	30

/* Exported functions ------------------------------------------------------- */
extern const uint8_t Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC];
extern const uint8_t Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC];
extern const uint8_t Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID];
extern const uint8_t Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR];
extern const uint8_t Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT];
extern uint8_t Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL];
extern const uint8_t Joystick_ReportDescriptor[JOYSTICK_REPORT_DESC_SIZE];


#endif /* __USB_DESC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
