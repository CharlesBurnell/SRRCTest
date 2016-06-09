// Auto-generated. Do not edit!

// (in-package basics.msg)


"use strict";

let _serializer = require('../base_serialize.js');
let _deserializer = require('../base_deserialize.js');
let _finder = require('../find.js');

//-----------------------------------------------------------

class Complex {
  constructor() {
    this.real = 0.0;
    this.imaginary = 0.0;
  }

  static serialize(obj, bufferInfo) {
    // Serializes a message object of type Complex
    // Serialize message field [real]
    bufferInfo = _serializer.float32(obj.real, bufferInfo);
    // Serialize message field [imaginary]
    bufferInfo = _serializer.float32(obj.imaginary, bufferInfo);
    return bufferInfo;
  }

  static deserialize(buffer) {
    //deserializes a message object of type Complex
    let tmp;
    let len;
    let data = new Complex();
    // Deserialize message field [real]
    tmp = _deserializer.float32(buffer);
    data.real = tmp.data;
    buffer = tmp.buffer;
    // Deserialize message field [imaginary]
    tmp = _deserializer.float32(buffer);
    data.imaginary = tmp.data;
    buffer = tmp.buffer;
    return {
      data: data,
      buffer: buffer
    }
  }

  static datatype() {
    // Returns string type for a message object
    return 'basics/Complex';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '54da470dccf15d60bd273ab751e1c0a1';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float32 real
    float32 imaginary
    
    `;
  }

};

module.exports = Complex;
