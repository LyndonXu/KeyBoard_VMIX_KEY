/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptors for Joystick Mouse Demo
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


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* USB Standard Device Descriptor */
const uint8_t Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC] =
{
	0x12,                       /*bLength */
	DEVICE_DESCRIPTOR, 			/*bDescriptorType*/
	0x00,                       /*bcdUSB */
	0x02,
	0x00,                       /*bDeviceClass*/
	0x00,                       /*bDeviceSubClass*/
	0x00,                       /*bDeviceProtocol*/
	0x40,                       /*bMaxPacketSize 64*/
	0x83,                       /*idVendor (0x0483)*/
	0x04,
	0x22,                       /*idProduct = 0x5722*/
	0x57,
	0x00,                       /*bcdDevice rel. 1.00*/
	0x01,
	1,                          /*Index of string descriptor describing
                                                  manufacturer */
	2,                          /*Index of string descriptor describing
                                                 product*/
	3,                          /*Index of string descriptor describing the
                                                 device serial number */
	0x01                        /*bNumConfigurations*/
}
; /* Joystick_DeviceDescriptor */


const uint8_t Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC] =
{
	0x09, /* bLength: Configuation Descriptor size */
	CONFIG_DESCRIPTOR, /* bDescriptorType: Configuration */
	MIDI_SIZ_CONFIG_DESC,
	/* wTotalLength: Bytes returned */
	0x00,
	0x03,         /*bNumInterfaces: 2 interface*/
	0x01,         /*bConfigurationValue: Configuration value*/
	0x00,         /*iConfiguration: Index of string descriptor describing
                                 the configuration*/
	0x80,         /*bmAttributes: bus powered */
	0xC8,         /*MaxPower 400 mA: this current is used for detecting Vbus*/

	/************** Descriptor of standard audio control interface ****************/
	/* 09 */
	0x09,         /*bLength: Interface Descriptor size*/
	INTERFACE_DESCRIPTOR,/*bDescriptorType: Interface descriptor type*/
	0x00,         /*bInterfaceNumber: Number of Interface*/
	0x00,         /*bAlternateSetting: Alternate setting*/
	0x00,         /*bNumEndpoints: no endpoints*/
	0x01,         /*bInterfaceClass: Audio device class*/
	0x01,         /*bInterfaceSubClass : Audio control*/
	0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
	0x00,            /*iInterface: Index of string descriptor*/
//class_specific_interface_descritor        //类特殊接口描述符：类特殊音频控制接口
	0x09,                          //类特殊接口描述符的字节数大小
	CS_INTERFACE_DESCRIPTOR,               //类特殊接口描述符类型编号
	HEADER,                                //描述符子类
	0x00,
	0x01,                        //子类版本，1.0
	0x09,
	0x00,                        //类特殊描述大小
	0x01,                        //流接口数量
	0x01,                       //MIDI 流接口1属于此音频控制接口

//Standard MS Interface Descriptor
	0x09,                             //接口描述符的字节数大小
	INTERFACE_DESCRIPTOR,             //接口描述符类型编号
	0x01,                             //接口编号为1
	0x00,                             //备用编号为0
	0x02,                             //非0端点数量为2
	AUDIO,                            //音频设备（AUDIO）类
	MIDISTREAMING,                    //使用的子类：MIDI流(MIDI STREAMING)类
	0x00,                             //未用
	0x00,                              //未用


//Class-specific MS Interface Descriptor
	0x07,                      //类特殊MS接口描述符的字节数大小
	CS_INTERFACE_DESCRIPTOR,   //类特殊接口描述符类型编号
	MS_HEADER,                 //描述符子类
	0x00,
	0x01,                     //子类版本，1.0
	0x41,
	0x00,                    //类特殊描述总长度

//MIDI IN Jack Descriptor(Embedded)
	0x06,                          //MIDI输入插孔描述符
	CS_INTERFACE_DESCRIPTOR,       //类特殊接口
	MIDI_IN_JACK,                  //MIDI_IN_JACK子类
	EMBEDDED,                      //EMBEDDED插孔类
	0x01,                          //该插口的ID
	0x00,                          //未用

//MIDI IN Jack Descriptor (External)
	0x06,                          //MIDI输入插孔描述符
	CS_INTERFACE_DESCRIPTOR,       //类特殊接口
	MIDI_IN_JACK,                  //MIDI_IN_JACK子类
	EXTERNAL,                      //EXTERNAL插孔类
	0x02,                          //该插口的ID
	0x00,                          //未用

//MIDI OUT Jack Descriptor
	0x09,                          //MIDI输入插孔描述符
	CS_INTERFACE_DESCRIPTOR,       //类特殊接口
	MIDI_OUT_JACK,                 //MIDI_OUT_JACK子类
	EMBEDDED,                      //EMBEDDED插孔类
	0x03,                          //该插口的ID
	0x01,                          //该插口的输入引脚数
	0x02,                          //该引脚所在实体的ID
	0x01,                          //连接到该实体输出脚的输入脚的编号
	0x00,                          //未用

//MIDI OUT Jack Descriptor (External)
	0x09,                          //MIDI输入插孔描述符
	CS_INTERFACE_DESCRIPTOR,       //类特殊接口
	MIDI_OUT_JACK,                 //MIDI_OUT_JACK子类
	EXTERNAL,                      //EXTERNAL插孔类
	0x04,                          //该插口的ID
	0x01,                          //该插口的输入引脚数
	0x01,                          //该引脚所在实体的ID
	0x01,                          //连接到该实体输出脚的输入脚的编号
	0x00,

//Standard Bulk OUT Endpoint Descriptor
	0x09,                          //端点描述符的字节数大小
	ENDPOINT_DESCRIPTOR,           //端点描述符类型编号
	MAIN_POINT_OUT,                //端点号，主输出端点
	ENDPOINT_TYPE_BULK,            //使用的传输类型：批量传输
	0x40,
	0x00,                          //该端点支持的最大包尺寸，64字节
	0x0A,                          //中断扫描时间，无效
	0x00,                          //未用
	0x00,                          //未用

//Class-specific MS Bulk OUT Endpoint Descriptor
	0x05,                          //端点描述符的字节数大小
	CS_ENDPOINT_DESCRIPTOR,        //端点描述符类型编号
	MS_GENERAL,                    //MS_GENERAL子类
	0x01,                          //MIDI输入插孔数目
	0x01,                          //Embedded MIDI 输入插孔的ID

//Standard Bulk IN Endpoint Descriptor                                                                //主端点输出描述
	0x09,                          //端点描述符的字节数大小
	ENDPOINT_DESCRIPTOR,           //端点描述符类型编号
	MAIN_POINT_IN,                 //端点号，主输入端点
	ENDPOINT_TYPE_BULK,            //使用的传输类型：批量传输
	0x40,
	0x00,                          //该端点支持的最大包尺寸，64字节
	0x0A,                          //中断扫描时间，无效
	0x00,                          //未用
	0x00,                          //未用

//Class-specific MS Bulk IN Endpoint Descriptor
	0x05,                          //端点描述符的字节数大小
	CS_ENDPOINT_DESCRIPTOR,        //端点描述符类型编号
	MS_GENERAL,                    //MS_GENERAL子类
	0x01,                          //MIDI输入插孔数目
	0x03,                          //Embedded MIDI 输入插孔的ID
	/* total 101 bytes above */

    /************** Descriptor of Joystick Mouse interface ****************/
    0x09,         /*bLength: Interface Descriptor size*/
    INTERFACE_DESCRIPTOR,/*bDescriptorType: Interface descriptor type*/
    0x02,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x01,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,            /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick HID ********************/
    0x09,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x00,         /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,         /*bDescriptorType*/
    JOYSTICK_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Joystick endpoint ********************/
    0x07,          /*bLength: Endpoint Descriptor size*/
    ENDPOINT_DESCRIPTOR, /*bDescriptorType:*/

    0x82,          /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,          /*bmAttributes: Interrupt endpoint*/
    0x03,          /*wMaxPacketSize: 3 Byte max */
    0x00,
    0x20,          /*bInterval: Polling Interval (32 ms)*/
    /* total 126 bytes above */
};

const uint8_t Joystick_ReportDescriptor[JOYSTICK_REPORT_DESC_SIZE] =
{
	/**for joystick 30*	*/
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x32,                    //     USAGE (Z)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)  //
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)   //	
    0x75, 0x08,                    //     REPORT_SIZE (8)		  //  
    0x95, 0x03,                    //     REPORT_COUNT (3)		 
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,						   //   END_COLLECTION
    0xc0                          // END_COLLECTION

};

/* USB String Descriptors (optional) */
const uint8_t Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID] =
{
	JOYSTICK_SIZ_STRING_LANGID,
	STRING_DESCRIPTOR,
	0x04,
	0x08
}
; /* LangID = 0x0409: U.S. English */

const uint8_t Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR] =
{
	JOYSTICK_SIZ_STRING_VENDOR, /* Size of Vendor string */
	STRING_DESCRIPTOR,  /* bDescriptorType*/
	/* Manufacturer: "STMicroelectronics" */
	0x29, 0x59, //天
	0x25, 0x6d, //津
	0x02, 0x5e, //市
	0xD5, 0x71, //燕诺安\u71d5\u8bfa\u5b89
	0xFA, 0x8B, //
	0x89, 0x5B, //
	0xd1, 0x79, //科
	0x80, 0x62, //技
	0x09, 0x67, //有
	0x50, 0x96, //限
	0x6c, 0x51, //公
	0xf8, 0x53  //司
};

const uint8_t Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT] =
{
	JOYSTICK_SIZ_STRING_PRODUCT,          /* bLength */
	STRING_DESCRIPTOR,        /* bDescriptorType */
	0xD5, 0x71, //燕诺安\u71d5\u8bfa\u5b89
	0xFA, 0x8B, //
	0x89, 0x5B, //
	0x5A, 0x86, //虚
	0xDF, 0x62, //拟
	0x35, 0x75, //电
	0x11, 0x81, //脑
	0x2e, 0x95, //键
	0xd8, 0x76  //盘
};
uint8_t Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL] =
{
	JOYSTICK_SIZ_STRING_SERIAL,           /* bLength */
	STRING_DESCRIPTOR,        /* bDescriptorType */
	0xD5, 0x71, //燕诺安\u71d5\u8bfa\u5b89
	0xFA, 0x8B, //
	0x89, 0x5B, //
	0x2e, 0x95, //键
	0xd8, 0x76, //盘
	0xfb, 0x7c, //系
	0x17, 0x52, //列
	0x56, 0x00, //V
	0x31, 0x00, //1
	0x2e, 0x00, //.
	0x33, 0x00  //0
};

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
