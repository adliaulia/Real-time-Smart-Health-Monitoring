/*!
 * @file from DFRobot_MAX30102.h
 * @brief Define the basic structure of class DFRobot_MAX30102
 * @n This is a library used to drive heart rate and oximeter sensor
 * @n Function: freely control sensor, collect readings of red light and IR, algorithms for heart-rate and SPO2
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [YeHangYu](hangyu.ye@dfrobot.com)
 * @version  V1.0
 * @date  2020-05-30
 * @https://github.com/DFRobot/DFRobot_MAX30102
 */
#include "MAX30102_lib_intg.h"

MAX30102_LIB_INTG::MAX30102_LIB_INTG(void)
{
  
}

bool MAX30102_LIB_INTG::begin(TwoWire *pWire, uint8_t i2cAddr)
{
  _i2cAddr = i2cAddr;
  _pWire = pWire;
//  _pWire->begin(0,2);
  if (getPartID() != MAX30102_EXPECTED_PARTID) {
    return false;
  }
  softReset();
  return true;
}

uint8_t MAX30102_LIB_INTG::getPartID()
{
  uint8_t byteTemp;
  readReg(MAX30102_PARTID, &byteTemp, 1);
  return byteTemp;
}

void MAX30102_LIB_INTG::softReset(void)
{
  sMode_t modeReg;
  readReg(MAX30102_MODECONFIG, &modeReg, 1);
  modeReg.reset = 1;
  writeReg(MAX30102_MODECONFIG, &modeReg, 1);
  uint32_t startTime = millis();
  while (millis() - startTime < 100) {
    readReg(MAX30102_MODECONFIG, &modeReg, 1);
    if (modeReg.reset == 0) break; 
    delay(1);
  }
}

void MAX30102_LIB_INTG::sensorConfiguration(uint8_t ledBrightness, uint8_t sampleAverage, uint8_t ledMode, uint8_t sampleRate, uint8_t pulseWidth, uint8_t adcRange)
{

  setFIFOAverage(sampleAverage);


  setADCRange(adcRange);

  setSampleRate(sampleRate);

  setPulseWidth(pulseWidth);

  setPulseAmplitudeRed(ledBrightness);
  setPulseAmplitudeIR(ledBrightness);

  enableSlot(1, SLOT_RED_LED);
  if (ledMode > MODE_REDONLY) enableSlot(2, SLOT_IR_LED);


  setLEDMode(ledMode);

  if (ledMode == MODE_REDONLY) {
    _activeLEDs = 1;
  } else {
    _activeLEDs = 2;
  }

  enableFIFORollover(); 
  resetFIFO(); 
}

void MAX30102_LIB_INTG::setFIFOAverage(uint8_t numberOfSamples)
{
  sFIFO_t FIFOReg;
  readReg(MAX30102_FIFOCONFIG, &FIFOReg, 1);
  FIFOReg.sampleAverag = numberOfSamples;
  writeReg(MAX30102_FIFOCONFIG, &FIFOReg, 1);
}

void MAX30102_LIB_INTG::setADCRange(uint8_t adcRange)
{
  sParticle_t particleReg;
  readReg(MAX30102_PARTICLECONFIG, &particleReg, 1);
  particleReg.adcRange = adcRange;
  writeReg(MAX30102_PARTICLECONFIG, &particleReg, 1);
}

void MAX30102_LIB_INTG::setSampleRate(uint8_t sampleRate)
{
  sParticle_t particleReg;
  readReg(MAX30102_PARTICLECONFIG, &particleReg, 1);
  particleReg.sampleRate = sampleRate;
  writeReg(MAX30102_PARTICLECONFIG, &particleReg, 1);
}

void MAX30102_LIB_INTG::setPulseWidth(uint8_t pulseWidth)
{
  sParticle_t particleReg;
  readReg(MAX30102_PARTICLECONFIG, &particleReg, 1);
  particleReg.pulseWidth = pulseWidth;
  writeReg(MAX30102_PARTICLECONFIG, &particleReg, 1);
}

void MAX30102_LIB_INTG::setPulseAmplitudeRed(uint8_t amplitude)
{
  uint8_t byteTemp = amplitude;
  writeReg(MAX30102_LED1_PULSEAMP, &byteTemp, 1);
}

void MAX30102_LIB_INTG::setPulseAmplitudeIR(uint8_t amplitude)
{
  uint8_t byteTemp = amplitude;
  writeReg(MAX30102_LED2_PULSEAMP, &byteTemp, 1);
}

void MAX30102_LIB_INTG::enableSlot(uint8_t slotNumber, uint8_t device)
{
  sMultiLED_t multiLEDReg;
  switch (slotNumber) {
  case (1):
    readReg(MAX30102_MULTILEDCONFIG1, &multiLEDReg, 1);
    multiLEDReg.SLOT1 = device;
    writeReg(MAX30102_MULTILEDCONFIG1, &multiLEDReg, 1);
    break;
  case (2):
    readReg(MAX30102_MULTILEDCONFIG1, &multiLEDReg, 1);
    multiLEDReg.SLOT2 = device;
    writeReg(MAX30102_MULTILEDCONFIG1, &multiLEDReg, 1);
    break;
  default:
    break;
  }
}

void MAX30102_LIB_INTG::setLEDMode(uint8_t ledMode)
{
  sMode_t modeReg;
  readReg(MAX30102_MODECONFIG, &modeReg, 1);
  modeReg.ledMode = ledMode;
  writeReg(MAX30102_MODECONFIG, &modeReg, 1);
}

void MAX30102_LIB_INTG::enableFIFORollover(void)
{
  sFIFO_t FIFOReg;
  readReg(MAX30102_FIFOCONFIG, &FIFOReg, 1);
  FIFOReg.RollOver = 1;
  writeReg(MAX30102_FIFOCONFIG, &FIFOReg, 1);
}

void MAX30102_LIB_INTG::resetFIFO(void)
{
  uint8_t byteTemp = 0;
  writeReg(MAX30102_FIFOWRITEPTR, &byteTemp, 1);
  writeReg(MAX30102_FIFOOVERFLOW, &byteTemp, 1);
  writeReg(MAX30102_FIFOREADPTR, &byteTemp, 1);
}

uint32_t MAX30102_LIB_INTG::getRed(void)
{
  getNewData();
  return (senseBuf.red[senseBuf.head]);
}

uint32_t MAX30102_LIB_INTG::getIR(void)
{
  getNewData();
  return (senseBuf.IR[senseBuf.head]);
}

void MAX30102_LIB_INTG::getNewData(void)
{
  int32_t numberOfSamples = 0;
  uint8_t readPointer = 0;
  uint8_t writePointer = 0;
  while (1) {
    readPointer = getReadPointer();
    writePointer = getWritePointer();

    if (readPointer == writePointer) {
//      DBG("no data");
    } else {
      numberOfSamples = writePointer - readPointer;
      if (numberOfSamples < 0) numberOfSamples += 32;
      int32_t bytesNeedToRead = numberOfSamples * _activeLEDs * 3;
   
        while (bytesNeedToRead > 0) {
          senseBuf.head++;
          senseBuf.head %= MAX30102_SENSE_BUF_SIZE;
          uint32_t tempBuf = 0;
          if (_activeLEDs > 1) { 
            uint8_t temp[6];
            uint8_t tempex;

            readReg(MAX30102_FIFODATA, temp, 6);

            for(uint8_t i = 0; i < 3; i++){
              tempex = temp[i];
              temp[i] = temp[5-i];
              temp[5-i] = tempex;
            }

            memcpy(&tempBuf, temp, 3*sizeof(temp[0]));
            tempBuf &= 0x3FFFF;
            senseBuf.IR[senseBuf.head] = tempBuf;
            memcpy(&tempBuf, temp+3, 3*sizeof(temp[0]));
            tempBuf &= 0x3FFFF;
            senseBuf.red[senseBuf.head] = tempBuf;
          } else { 
            uint8_t temp[3];
            uint8_t tempex;


            readReg(MAX30102_FIFODATA, temp, 3);
            tempex = temp[0];
            temp[0] = temp[2];
            temp[2] = tempex;

            memcpy(&tempBuf, temp, 3*sizeof(temp[0]));
            tempBuf &= 0x3FFFF;
            senseBuf.red[senseBuf.head] = tempBuf;
          }
          bytesNeedToRead -= _activeLEDs * 3;
        }
      return;
    }
    delay(1);
  }
}

uint8_t MAX30102_LIB_INTG::getReadPointer(void)
{
  uint8_t byteTemp;
  readReg(MAX30102_FIFOREADPTR, &byteTemp, 1);
  return byteTemp;
}

uint8_t MAX30102_LIB_INTG::getWritePointer(void)
{
  uint8_t byteTemp;
  readReg(MAX30102_FIFOWRITEPTR, &byteTemp, 1);
  return byteTemp;
}

void MAX30102_LIB_INTG::heartrateAndOxygenSaturation(double* SPO2,int8_t* SPO2Valid,int32_t* heartRate,int8_t* heartRateValid, double* ratio, double* correl)
{
  uint32_t irBuffer[100];
  uint32_t redBuffer[100];
  int32_t bufferLength = 100;

  for (uint8_t i = 0 ; i < bufferLength ; ) {
    getNewData(); 
 
    int8_t numberOfSamples = senseBuf.head - senseBuf.tail;
    if (numberOfSamples < 0) {
      numberOfSamples += MAX30102_SENSE_BUF_SIZE;
    }

    while(numberOfSamples--) {
      //beberapa modul max30102 terbalik antara red dan ir lednya 
      //https://github.com/aromring/MAX30102_by_RF/issues/13
      irBuffer[i] = senseBuf.red[senseBuf.tail];
      redBuffer[i] = senseBuf.IR[senseBuf.tail];

      senseBuf.tail++;
      senseBuf.tail %= MAX30102_SENSE_BUF_SIZE;
      i++;
      if(i == bufferLength) break;
    }
  }

  rf_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, SPO2, SPO2Valid, heartRate, heartRateValid, ratio, correl);
}

void MAX30102_LIB_INTG::writeReg(uint8_t reg, const void* pBuf, uint8_t size)
{
  if(pBuf == NULL) {
//    DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_i2cAddr);
  _pWire->write(&reg, 1);

  for(uint16_t i = 0; i < size; i++) {
    _pWire->write(_pBuf[i]);
  }
  _pWire->endTransmission();
}

uint8_t MAX30102_LIB_INTG::readReg(uint8_t reg, const void* pBuf, uint8_t size)
{
  if(pBuf == NULL) {
//    DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_i2cAddr);
  _pWire->write(&reg, 1);

  if( _pWire->endTransmission() != 0) {
    return 0;
  }

  _pWire->requestFrom(_i2cAddr,  size);
  for(uint16_t i = 0; i < size; i++) {
    _pBuf[i] = _pWire->read();
  }
  return size;
}

float MAX30102_LIB_INTG::readTemperatureC()
{

  uint8_t byteTemp = 0x01;
  writeReg(MAX30102_DIETEMPCONFIG, &byteTemp, 1);

  uint32_t startTime = millis();
  while (millis() - startTime < 100) { 
    readReg(MAX30102_DIETEMPCONFIG, &byteTemp, 1);
    if ((byteTemp & 0x01) == 0) break; 
    delay(1);
  }


  uint8_t tempInt;
  readReg(MAX30102_DIETEMPINT, &tempInt, 1);

  uint8_t tempFrac;
  readReg(MAX30102_DIETEMPFRAC, &tempFrac, 1);

  return (float)tempInt + ((float)tempFrac * 0.0625);
}
