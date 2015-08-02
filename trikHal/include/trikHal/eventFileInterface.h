/* Copyright 2015 Yurii Litvinov and CyberTech Labs Ltd.
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

#pragma once

#include <QtCore/QString>
#include <QtCore/QObject>

namespace trikHal {

class EventFileInterface : public QObject
{
	Q_OBJECT

public:
	enum class EventType {
		unknown
		, evAbsDistance
		, evAbsMisc
		, evAbsX
		, evAbsY
		, evAbsZ
		, evSyn
		, evKey
	};

	virtual bool open(const QString &fileName) = 0;
	virtual bool close() = 0;

signals:
	void newEvent(EventType eventType, int code, int value);
};

}
