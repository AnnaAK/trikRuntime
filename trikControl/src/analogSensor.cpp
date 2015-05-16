/* Copyright 2013 - 2015 Nikita Batov and CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "analogSensor.h"

#include <QtCore/QDebug>

#include <trikKernel/configurer.h>
#include <QsLog.h>

#include "i2cCommunicator.h"
#include "configurerHelper.h"
#include <math.h>

using namespace trikControl;

AnalogSensor::AnalogSensor(const QString &port, const trikKernel::Configurer &configurer, I2cCommunicator &communicator)
	: mCommunicator(communicator)
{
	mI2cCommandNumber = ConfigurerHelper::configureInt(configurer, mState, port, "i2cCommandNumber");

	// We use linear subjection to normalize sensor values:
	// normalizedValue = k * rawValue + b
	// To calculate k and b we need two raw values and two corresponding them normalized values.

	const int rawValue1 = ConfigurerHelper::configureInt(configurer, mState, port, "rawValue1");
	const int rawValue2 = ConfigurerHelper::configureInt(configurer, mState, port, "rawValue2");
	const int normalizedValue1 = ConfigurerHelper::configureInt(configurer, mState, port, "normalizedValue1");
	const int normalizedValue2 = ConfigurerHelper::configureInt(configurer, mState, port, "normalizedValue2");
	const int senPow = ConfigurerHelper::configureInt(configurer, mState, port, "senPow");

	if (rawValue1 == rawValue2) {
		QLOG_ERROR() <<  "Sensor calibration error: rawValue1 = rawValue2!";
		mState.fail();
		mK = 0;
		mB = 0;
	} else {
		mK = static_cast<qreal>(normalizedValue2 - normalizedValue1) / (rawValue2 - rawValue1);
		mB = normalizedValue1 - mK * rawValue1;
		sen = senPow;
	}

	mState.ready();
	mIRType = configurer.attributeByPort(port, "type") == "IR" ? Type::IR : Type::IRCommon;
}

AnalogSensor::Status AnalogSensor::status() const
{
	return DeviceInterface::combine(mCommunicator, mState.status());
}

int AnalogSensor::read()
{
	if (!mState.isReady() || mCommunicator.status() != DeviceInterface::Status::ready) {
		return 0;
	}
	QByteArray command(1, '\0');
	command[0] = static_cast<char>(mI2cCommandNumber & 0xFF);
	if (mIRType == Type::IR){
	return mK * pow(mCommunicator.read(command), sen) + mB;
	}

	float x1 = 25;
	float x2 = 35;
	float x3 = 15;
	float y = 322;
	float y2 = 235;
	float y3 = 511;
	float d1 = (x3*(y - y3)*(x2 - x1)) / (y - y2);
	float d2 = x1*x2;
	float d3 = x2*x3;
	float d4 = x3 - x1 - (y - y3)*(x2 - x1) / (y - y2);
	float answer = (d1 + d2 - d3) / d4;
	float help = (y - y2)*(x1 + answer)*(x2 + answer);
	float help2 = (x3 + answer) * (x2 - x1);
	float answer2 = y3 - help / help2;
	float answer3 = (y - y2)*(x2 + answer)*(x1 + answer) / (x2 - x1);
	int fa = (int)(answer3/((mCommunicator.read(command)) + answer) + answer2);
	return fa;

}

int AnalogSensor::readRawData()
{
	if (!mState.isReady() || mCommunicator.status() != DeviceInterface::Status::ready) {
		return 0;
	}

	QByteArray command(1, '\0');
	command[0] = static_cast<char>(mI2cCommandNumber & 0xFF);

	return mCommunicator.read(command);
}
