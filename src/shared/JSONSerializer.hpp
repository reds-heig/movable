/*******************************************************************************
 ** MOVABLE project - REDS Institute, HEIG-VD, Yverdon-les-Bains (CH) - 2016  **
 **                                                                           **
 ** This file is part of MOVABLE.                                             **
 **                                                                           **
 **  MOVABLE is free software: you can redistribute it and/or modify          **
 **  it under the terms of the GNU General Public License as published by     **
 **  the Free Software Foundation, either version 3 of the License, or        **
 **  (at your option) any later version.                                      **
 **                                                                           **
 **  MOVABLE is distributed in the hope that it will be useful,               **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of           **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            **
 **  GNU General Public License for more details.                             **
 **                                                                           **
 **  You should have received a copy of the GNU General Public License        **
 **  along with MOVABLE.  If not, see <http://www.gnu.org/licenses/>.         **
 ******************************************************************************/

#ifndef JSON_SERIALIZER_HPP_
#define JSON_SERIALIZER_HPP_

#include "json/json.h"

#include "JSONSerializable.hpp"

/**
 * class JSONSerializer - Deserializer object
 *
 * Code adapted from http://www.danielsoltyka.com/programming/2011/04/15/simple-class-serialization-with-jsoncpp/
 */
class JSONSerializer
{
private:
    /* Private to prevent class instantiation */
    JSONSerializer(void) { };
public:
    /**
     * Serialize() - Serialize a given object as a JSON string
     *
     * @obj   : object to serialize
     *
     * @output: resulting JSON representation
     *
     * Return: true if the object has been serialized, false if the pointer
     *     to the object in invalid (NULL)
     */
    static bool Serialize(JSONSerializable *obj, std::string &output);

    /**
     * Deserialize() - Deserialize an object from a given JSON string
     *
     * @obj  : deserialized object
     *
     * @input: input JSON representation
     *
     * Return: true if the object has been deserialized, false if an error
     *     occurred
     */
    static bool Deserialize(JSONSerializable *obj, std::string &input);
};

#endif /* JSON_SERIALIZER_HPP_ */
