/*******************************************************************************
 ** MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016  **
 **									      **
 ** This file is part of MOVABLE.					      **
 **									      **
 **  MOVABLE is free software: you can redistribute it and/or modify	      **
 **  it under the terms of the GNU General Public License as published by     **
 **  the Free Software Foundation, either version 3 of the License, or	      **
 **  (at your option) any later version.				      **
 **									      **
 **  MOVABLE is distributed in the hope that it will be useful,		      **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of	      **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	      **
 **  GNU General Public License for more details.			      **
 **									      **
 **  You should have received a copy of the GNU General Public License	      **
 **  along with MOVABLE.  If not, see <http://www.gnu.org/licenses/>.	      **
 ******************************************************************************/

#include "JSONSerializer.hpp"

bool
JSONSerializer::Serialize(JSONSerializable *obj, std::string &output)
{
	if (obj == NULL) {
		return false;
	}

	Json::Value root;
	obj->Serialize(root);

	Json::StyledWriter writer;
	output = writer.write(root);

	return true;
}

bool
JSONSerializer::Deserialize(JSONSerializable *obj, std::string &input)
{
	if (obj == NULL) {
		return false;
	}

	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(input, root)) {
		return false;
	}

	obj->Deserialize(root);

	return true;
}
