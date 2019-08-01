var Buff;
(function(f){if(typeof exports==="object"&&typeof module!=="undefined"){module.exports=f()}else if(typeof define==="function"&&define.amd){define([],f)}else{var g;if(typeof window!=="undefined"){g=window}else if(typeof global!=="undefined"){g=global}else if(typeof self!=="undefined"){g=self}else{g=this}g.ipfs = f()}})(function(){var define,module,exports;return (function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
(function (Buffer){
var ipfsApi;

try {
  ipfsApi  = require('ipfs-api/dist/ipfsapi.min.js');
} catch(e) {}

var base58   = require('bitcore/lib/encoding/base58.js');
var concat   = require('concat-stream');

var ipfs = {};

ipfs.currentProvider = {host: null, port: null};
ipfs.defaultProvider = {host: 'localhost', port: '5001'};
ipfs.api = null;

ipfs.setProvider = function(opts) {
  if (!opts) opts = this.defaultProvider;
  if (typeof opts === 'object' && !opts.hasOwnProperty('host')) {
    ipfs.api = opts;
    return;
  }

  ipfs.currentProvider = opts;
  ipfs.api = ipfsApi(opts.host, opts.port, opts);
};

ipfs.utils = {};

ipfs.utils.base58ToHex = function(b58) {
  var hexBuf = base58.decode(b58);
  return hexBuf.toString('hex');
};

ipfs.utils.hexToBase58 = function(hexStr) {
  var buf = new Buffer(hexStr, 'hex');
  return base58.encode(buf);
};

ipfs.add = function(input, callback) {

  var buf;
  if (typeof input === 'string') {
    buf = new Buffer(input);
  }
  else {
    buf = input;
  }

  ipfs.api.add(buf, function (err, ret) {
    if (err) callback(err, null);
    else callback(null, ret[0] ? ret[0].Hash : ret.Hash);
  });
};

ipfs.catText = function(ipfsHash, callback) {
  ipfs.cat(ipfsHash, function(err, data) {
    if (ipfs.api.Buffer.isBuffer(data))
      data = data.toString();
    callback(err, data);
  });
}

ipfs.cat = function(ipfsHash, callback) {
  ipfs.api.cat(ipfsHash, function(err, res) {
    if (err || !res) return callback(err, null);

    var gotIpfsData = function (ipfsData) {
      callback(err, ipfsData);
    };

    var concatStream = concat(gotIpfsData);

    if(res.readable) {
      // Returned as a stream
      res.pipe(concatStream);
    } else {

      if (!ipfs.api.Buffer.isBuffer(res)) {

        if (typeof res === 'object')
          res = JSON.stringify(res);

        if (typeof res !== 'string')
          throw new Error("ipfs.cat response type not recognized; expecting string, buffer, or object");

        res = new ipfs.api.Buffer(res, 'binary');
      }

      // Returned as a string
      callback(err, res);
    }
  });
};

ipfs.addJson = function(jsonObject, callback) {
  var jsonString = JSON.stringify(jsonObject);
  ipfs.add(jsonString, callback);
};

ipfs.catJson = function(ipfsHash, callback) {
  ipfs.catText(ipfsHash, function (err, jsonString) {
    if (err) callback(err, {});

    var jsonObject = {};
    try {
      jsonObject = typeof jsonString === 'string' ?  JSON.parse(jsonString) : jsonString;
    } catch (e) {
      err = e;
    }
    callback(err, jsonObject);
  });
};

module.exports = ipfs;

}).call(this,require("buffer").Buffer)
},{"bitcore/lib/encoding/base58.js":3,"buffer":7,"concat-stream":8,"ipfs-api/dist/ipfsapi.min.js":13}],2:[function(require,module,exports){
var lookup = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

;(function (exports) {
	'use strict';

  var Arr = (typeof Uint8Array !== 'undefined')
    ? Uint8Array
    : Array

	var PLUS   = '+'.charCodeAt(0)
	var SLASH  = '/'.charCodeAt(0)
	var NUMBER = '0'.charCodeAt(0)
	var LOWER  = 'a'.charCodeAt(0)
	var UPPER  = 'A'.charCodeAt(0)
	var PLUS_URL_SAFE = '-'.charCodeAt(0)
	var SLASH_URL_SAFE = '_'.charCodeAt(0)

	function decode (elt) {
		var code = elt.charCodeAt(0)
		if (code === PLUS ||
		    code === PLUS_URL_SAFE)
			return 62 // '+'
		if (code === SLASH ||
		    code === SLASH_URL_SAFE)
			return 63 // '/'
		if (code < NUMBER)
			return -1 //no match
		if (code < NUMBER + 10)
			return code - NUMBER + 26 + 26
		if (code < UPPER + 26)
			return code - UPPER
		if (code < LOWER + 26)
			return code - LOWER + 26
	}

	function b64ToByteArray (b64) {
		var i, j, l, tmp, placeHolders, arr

		if (b64.length % 4 > 0) {
			throw new Error('Invalid string. Length must be a multiple of 4')
		}

		// the number of equal signs (place holders)
		// if there are two placeholders, than the two characters before it
		// represent one byte
		// if there is only one, then the three characters before it represent 2 bytes
		// this is just a cheap hack to not do indexOf twice
		var len = b64.length
		placeHolders = '=' === b64.charAt(len - 2) ? 2 : '=' === b64.charAt(len - 1) ? 1 : 0

		// base64 is 4/3 + up to two characters of the original data
		arr = new Arr(b64.length * 3 / 4 - placeHolders)

		// if there are placeholders, only get up to the last complete 4 chars
		l = placeHolders > 0 ? b64.length - 4 : b64.length

		var L = 0

		function push (v) {
			arr[L++] = v
		}

		for (i = 0, j = 0; i < l; i += 4, j += 3) {
			tmp = (decode(b64.charAt(i)) << 18) | (decode(b64.charAt(i + 1)) << 12) | (decode(b64.charAt(i + 2)) << 6) | decode(b64.charAt(i + 3))
			push((tmp & 0xFF0000) >> 16)
			push((tmp & 0xFF00) >> 8)
			push(tmp & 0xFF)
		}

		if (placeHolders === 2) {
			tmp = (decode(b64.charAt(i)) << 2) | (decode(b64.charAt(i + 1)) >> 4)
			push(tmp & 0xFF)
		} else if (placeHolders === 1) {
			tmp = (decode(b64.charAt(i)) << 10) | (decode(b64.charAt(i + 1)) << 4) | (decode(b64.charAt(i + 2)) >> 2)
			push((tmp >> 8) & 0xFF)
			push(tmp & 0xFF)
		}

		return arr
	}

	function uint8ToBase64 (uint8) {
		var i,
			extraBytes = uint8.length % 3, // if we have 1 byte left, pad 2 bytes
			output = "",
			temp, length

		function encode (num) {
			return lookup.charAt(num)
		}

		function tripletToBase64 (num) {
			return encode(num >> 18 & 0x3F) + encode(num >> 12 & 0x3F) + encode(num >> 6 & 0x3F) + encode(num & 0x3F)
		}

		// go through the array every three bytes, we'll deal with trailing stuff later
		for (i = 0, length = uint8.length - extraBytes; i < length; i += 3) {
			temp = (uint8[i] << 16) + (uint8[i + 1] << 8) + (uint8[i + 2])
			output += tripletToBase64(temp)
		}

		// pad the end with zeros, but make sure to not forget the extra bytes
		switch (extraBytes) {
			case 1:
				temp = uint8[uint8.length - 1]
				output += encode(temp >> 2)
				output += encode((temp << 4) & 0x3F)
				output += '=='
				break
			case 2:
				temp = (uint8[uint8.length - 2] << 8) + (uint8[uint8.length - 1])
				output += encode(temp >> 10)
				output += encode((temp >> 4) & 0x3F)
				output += encode((temp << 2) & 0x3F)
				output += '='
				break
		}

		return output
	}

	exports.toByteArray = b64ToByteArray
	exports.fromByteArray = uint8ToBase64
}(typeof exports === 'undefined' ? (this.base64js = {}) : exports))

},{}],3:[function(require,module,exports){
(function (Buffer){
'use strict';

var _ = require('lodash');
var bs58 = require('bs58');
var buffer = require('buffer');

var ALPHABET = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'.split('');

var Base58 = function Base58(obj) {
  /* jshint maxcomplexity: 8 */
  if (!(this instanceof Base58)) {
    return new Base58(obj);
  }
  if (Buffer.isBuffer(obj)) {
    var buf = obj;
    this.fromBuffer(buf);
  } else if (typeof obj === 'string') {
    var str = obj;
    this.fromString(str);
  } else if (obj) {
    this.set(obj);
  }
};

Base58.validCharacters = function validCharacters(chars) {
  if (buffer.Buffer.isBuffer(chars)) {
    chars = chars.toString();
  }
  return _.all(_.map(chars, function(char) { return _.contains(ALPHABET, char); }));
};

Base58.prototype.set = function(obj) {
  this.buf = obj.buf || this.buf || undefined;
  return this;
};

Base58.encode = function(buf) {
  if (!buffer.Buffer.isBuffer(buf)) {
    throw new Error('Input should be a buffer');
  }
  return bs58.encode(buf);
};

Base58.decode = function(str) {
  if (typeof str !== 'string') {
    throw new Error('Input should be a string');
  }
  return new Buffer(bs58.decode(str));
};

Base58.prototype.fromBuffer = function(buf) {
  this.buf = buf;
  return this;
};

Base58.prototype.fromString = function(str) {
  var buf = Base58.decode(str);
  this.buf = buf;
  return this;
};

Base58.prototype.toBuffer = function() {
  return this.buf;
};

Base58.prototype.toString = function() {
  return Base58.encode(this.buf);
};

module.exports = Base58;

}).call(this,require("buffer").Buffer)
},{"bs58":4,"buffer":7,"lodash":5}],4:[function(require,module,exports){
// Base58 encoding/decoding
// Originally written by Mike Hearn for BitcoinJ
// Copyright (c) 2011 Google Inc
// Ported to JavaScript by Stefan Thomas
// Merged Buffer refactorings from base58-native by Stephen Pair
// Copyright (c) 2013 BitPay Inc

var ALPHABET = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'
var ALPHABET_MAP = {}
for(var i = 0; i < ALPHABET.length; i++) {
  ALPHABET_MAP[ALPHABET.charAt(i)] = i
}
var BASE = 58

function encode(buffer) {
  if (buffer.length === 0) return ''

  var i, j, digits = [0]
  for (i = 0; i < buffer.length; i++) {
    for (j = 0; j < digits.length; j++) digits[j] <<= 8

    digits[0] += buffer[i]

    var carry = 0
    for (j = 0; j < digits.length; ++j) {
      digits[j] += carry

      carry = (digits[j] / BASE) | 0
      digits[j] %= BASE
    }

    while (carry) {
      digits.push(carry % BASE)

      carry = (carry / BASE) | 0
    }
  }

  // deal with leading zeros
  for (i = 0; buffer[i] === 0 && i < buffer.length - 1; i++) digits.push(0)

  return digits.reverse().map(function(digit) { return ALPHABET[digit] }).join('')
}

function decode(string) {
  if (string.length === 0) return []

  var i, j, bytes = [0]
  for (i = 0; i < string.length; i++) {
    var c = string[i]
    if (!(c in ALPHABET_MAP)) throw new Error('Non-base58 character')

    for (j = 0; j < bytes.length; j++) bytes[j] *= BASE
    bytes[0] += ALPHABET_MAP[c]

    var carry = 0
    for (j = 0; j < bytes.length; ++j) {
      bytes[j] += carry

      carry = bytes[j] >> 8
      bytes[j] &= 0xff
    }

    while (carry) {
      bytes.push(carry & 0xff)

      carry >>= 8
    }
  }

  // deal with leading zeros
  for (i = 0; string[i] === '1' && i < string.length - 1; i++) bytes.push(0)

  return bytes.reverse()
}

module.exports = {
  encode: encode,
  decode: decode
}

},{}],5:[function(require,module,exports){
(function (global){
/**
 * @license
 * lodash 3.10.1 (Custom Build) <https://lodash.com/>
 * Build: `lodash modern -d -o ./index.js`
 * Copyright 2012-2015 The Dojo Foundation <http://dojofoundation.org/>
 * Based on Underscore.js 1.8.3 <http://underscorejs.org/LICENSE>
 * Copyright 2009-2015 Jeremy Ashkenas, DocumentCloud and Investigative Reporters & Editors
 * Available under MIT license <https://lodash.com/license>
 */
;(function() {

  /** Used as a safe reference for `undefined` in pre-ES5 environments. */
  var undefined;

  /** Used as the semantic version number. */
  var VERSION = '3.10.1';

  /** Used to compose bitmasks for wrapper metadata. */
  var BIND_FLAG = 1,
      BIND_KEY_FLAG = 2,
      CURRY_BOUND_FLAG = 4,
      CURRY_FLAG = 8,
      CURRY_RIGHT_FLAG = 16,
      PARTIAL_FLAG = 32,
      PARTIAL_RIGHT_FLAG = 64,
      ARY_FLAG = 128,
      REARG_FLAG = 256;

  /** Used as default options for `_.trunc`. */
  var DEFAULT_TRUNC_LENGTH = 30,
      DEFAULT_TRUNC_OMISSION = '...';

  /** Used to detect when a function becomes hot. */
  var HOT_COUNT = 150,
      HOT_SPAN = 16;

  /** Used as the size to enable large array optimizations. */
  var LARGE_ARRAY_SIZE = 200;

  /** Used to indicate the type of lazy iteratees. */
  var LAZY_FILTER_FLAG = 1,
      LAZY_MAP_FLAG = 2;

  /** Used as the `TypeError` message for "Functions" methods. */
  var FUNC_ERROR_TEXT = 'Expected a function';

  /** Used as the internal argument placeholder. */
  var PLACEHOLDER = '__lodash_placeholder__';

  /** `Object#toString` result references. */
  var argsTag = '[object Arguments]',
      arrayTag = '[object Array]',
      boolTag = '[object Boolean]',
      dateTag = '[object Date]',
      errorTag = '[object Error]',
      funcTag = '[object Function]',
      mapTag = '[object Map]',
      numberTag = '[object Number]',
      objectTag = '[object Object]',
      regexpTag = '[object RegExp]',
      setTag = '[object Set]',
      stringTag = '[object String]',
      weakMapTag = '[object WeakMap]';

  var arrayBufferTag = '[object ArrayBuffer]',
      float32Tag = '[object Float32Array]',
      float64Tag = '[object Float64Array]',
      int8Tag = '[object Int8Array]',
      int16Tag = '[object Int16Array]',
      int32Tag = '[object Int32Array]',
      uint8Tag = '[object Uint8Array]',
      uint8ClampedTag = '[object Uint8ClampedArray]',
      uint16Tag = '[object Uint16Array]',
      uint32Tag = '[object Uint32Array]';

  /** Used to match empty string literals in compiled template source. */
  var reEmptyStringLeading = /\b__p \+= '';/g,
      reEmptyStringMiddle = /\b(__p \+=) '' \+/g,
      reEmptyStringTrailing = /(__e\(.*?\)|\b__t\)) \+\n'';/g;

  /** Used to match HTML entities and HTML characters. */
  var reEscapedHtml = /&(?:amp|lt|gt|quot|#39|#96);/g,
      reUnescapedHtml = /[&<>"'`]/g,
      reHasEscapedHtml = RegExp(reEscapedHtml.source),
      reHasUnescapedHtml = RegExp(reUnescapedHtml.source);

  /** Used to match template delimiters. */
  var reEscape = /<%-([\s\S]+?)%>/g,
      reEvaluate = /<%([\s\S]+?)%>/g,
      reInterpolate = /<%=([\s\S]+?)%>/g;

  /** Used to match property names within property paths. */
  var reIsDeepProp = /\.|\[(?:[^[\]]*|(["'])(?:(?!\1)[^\n\\]|\\.)*?\1)\]/,
      reIsPlainProp = /^\w*$/,
      rePropName = /[^.[\]]+|\[(?:(-?\d+(?:\.\d+)?)|(["'])((?:(?!\2)[^\n\\]|\\.)*?)\2)\]/g;

  /**
   * Used to match `RegExp` [syntax characters](http://ecma-international.org/ecma-262/6.0/#sec-patterns)
   * and those outlined by [`EscapeRegExpPattern`](http://ecma-international.org/ecma-262/6.0/#sec-escaperegexppattern).
   */
  var reRegExpChars = /^[:!,]|[\\^$.*+?()[\]{}|\/]|(^[0-9a-fA-Fnrtuvx])|([\n\r\u2028\u2029])/g,
      reHasRegExpChars = RegExp(reRegExpChars.source);

  /** Used to match [combining diacritical marks](https://en.wikipedia.org/wiki/Combining_Diacritical_Marks). */
  var reComboMark = /[\u0300-\u036f\ufe20-\ufe23]/g;

  /** Used to match backslashes in property paths. */
  var reEscapeChar = /\\(\\)?/g;

  /** Used to match [ES template delimiters](http://ecma-international.org/ecma-262/6.0/#sec-template-literal-lexical-components). */
  var reEsTemplate = /\$\{([^\\}]*(?:\\.[^\\}]*)*)\}/g;

  /** Used to match `RegExp` flags from their coerced string values. */
  var reFlags = /\w*$/;

  /** Used to detect hexadecimal string values. */
  var reHasHexPrefix = /^0[xX]/;

  /** Used to detect host constructors (Safari > 5). */
  var reIsHostCtor = /^\[object .+?Constructor\]$/;

  /** Used to detect unsigned integer values. */
  var reIsUint = /^\d+$/;

  /** Used to match latin-1 supplementary letters (excluding mathematical operators). */
  var reLatin1 = /[\xc0-\xd6\xd8-\xde\xdf-\xf6\xf8-\xff]/g;

  /** Used to ensure capturing order of template delimiters. */
  var reNoMatch = /($^)/;

  /** Used to match unescaped characters in compiled string literals. */
  var reUnescapedString = /['\n\r\u2028\u2029\\]/g;

  /** Used to match words to create compound words. */
  var reWords = (function() {
    var upper = '[A-Z\\xc0-\\xd6\\xd8-\\xde]',
        lower = '[a-z\\xdf-\\xf6\\xf8-\\xff]+';

    return RegExp(upper + '+(?=' + upper + lower + ')|' + upper + '?' + lower + '|' + upper + '+|[0-9]+', 'g');
  }());

  /** Used to assign default `context` object properties. */
  var contextProps = [
    'Array', 'ArrayBuffer', 'Date', 'Error', 'Float32Array', 'Float64Array',
    'Function', 'Int8Array', 'Int16Array', 'Int32Array', 'Math', 'Number',
    'Object', 'RegExp', 'Set', 'String', '_', 'clearTimeout', 'isFinite',
    'parseFloat', 'parseInt', 'setTimeout', 'TypeError', 'Uint8Array',
    'Uint8ClampedArray', 'Uint16Array', 'Uint32Array', 'WeakMap'
  ];

  /** Used to make template sourceURLs easier to identify. */
  var templateCounter = -1;

  /** Used to identify `toStringTag` values of typed arrays. */
  var typedArrayTags = {};
  typedArrayTags[float32Tag] = typedArrayTags[float64Tag] =
  typedArrayTags[int8Tag] = typedArrayTags[int16Tag] =
  typedArrayTags[int32Tag] = typedArrayTags[uint8Tag] =
  typedArrayTags[uint8ClampedTag] = typedArrayTags[uint16Tag] =
  typedArrayTags[uint32Tag] = true;
  typedArrayTags[argsTag] = typedArrayTags[arrayTag] =
  typedArrayTags[arrayBufferTag] = typedArrayTags[boolTag] =
  typedArrayTags[dateTag] = typedArrayTags[errorTag] =
  typedArrayTags[funcTag] = typedArrayTags[mapTag] =
  typedArrayTags[numberTag] = typedArrayTags[objectTag] =
  typedArrayTags[regexpTag] = typedArrayTags[setTag] =
  typedArrayTags[stringTag] = typedArrayTags[weakMapTag] = false;

  /** Used to identify `toStringTag` values supported by `_.clone`. */
  var cloneableTags = {};
  cloneableTags[argsTag] = cloneableTags[arrayTag] =
  cloneableTags[arrayBufferTag] = cloneableTags[boolTag] =
  cloneableTags[dateTag] = cloneableTags[float32Tag] =
  cloneableTags[float64Tag] = cloneableTags[int8Tag] =
  cloneableTags[int16Tag] = cloneableTags[int32Tag] =
  cloneableTags[numberTag] = cloneableTags[objectTag] =
  cloneableTags[regexpTag] = cloneableTags[stringTag] =
  cloneableTags[uint8Tag] = cloneableTags[uint8ClampedTag] =
  cloneableTags[uint16Tag] = cloneableTags[uint32Tag] = true;
  cloneableTags[errorTag] = cloneableTags[funcTag] =
  cloneableTags[mapTag] = cloneableTags[setTag] =
  cloneableTags[weakMapTag] = false;

  /** Used to map latin-1 supplementary letters to basic latin letters. */
  var deburredLetters = {
    '\xc0': 'A',  '\xc1': 'A', '\xc2': 'A', '\xc3': 'A', '\xc4': 'A', '\xc5': 'A',
    '\xe0': 'a',  '\xe1': 'a', '\xe2': 'a', '\xe3': 'a', '\xe4': 'a', '\xe5': 'a',
    '\xc7': 'C',  '\xe7': 'c',
    '\xd0': 'D',  '\xf0': 'd',
    '\xc8': 'E',  '\xc9': 'E', '\xca': 'E', '\xcb': 'E',
    '\xe8': 'e',  '\xe9': 'e', '\xea': 'e', '\xeb': 'e',
    '\xcC': 'I',  '\xcd': 'I', '\xce': 'I', '\xcf': 'I',
    '\xeC': 'i',  '\xed': 'i', '\xee': 'i', '\xef': 'i',
    '\xd1': 'N',  '\xf1': 'n',
    '\xd2': 'O',  '\xd3': 'O', '\xd4': 'O', '\xd5': 'O', '\xd6': 'O', '\xd8': 'O',
    '\xf2': 'o',  '\xf3': 'o', '\xf4': 'o', '\xf5': 'o', '\xf6': 'o', '\xf8': 'o',
    '\xd9': 'U',  '\xda': 'U', '\xdb': 'U', '\xdc': 'U',
    '\xf9': 'u',  '\xfa': 'u', '\xfb': 'u', '\xfc': 'u',
    '\xdd': 'Y',  '\xfd': 'y', '\xff': 'y',
    '\xc6': 'Ae', '\xe6': 'ae',
    '\xde': 'Th', '\xfe': 'th',
    '\xdf': 'ss'
  };

  /** Used to map characters to HTML entities. */
  var htmlEscapes = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;',
    '`': '&#96;'
  };

  /** Used to map HTML entities to characters. */
  var htmlUnescapes = {
    '&amp;': '&',
    '&lt;': '<',
    '&gt;': '>',
    '&quot;': '"',
    '&#39;': "'",
    '&#96;': '`'
  };

  /** Used to determine if values are of the language type `Object`. */
  var objectTypes = {
    'function': true,
    'object': true
  };

  /** Used to escape characters for inclusion in compiled regexes. */
  var regexpEscapes = {
    '0': 'x30', '1': 'x31', '2': 'x32', '3': 'x33', '4': 'x34',
    '5': 'x35', '6': 'x36', '7': 'x37', '8': 'x38', '9': 'x39',
    'A': 'x41', 'B': 'x42', 'C': 'x43', 'D': 'x44', 'E': 'x45', 'F': 'x46',
    'a': 'x61', 'b': 'x62', 'c': 'x63', 'd': 'x64', 'e': 'x65', 'f': 'x66',
    'n': 'x6e', 'r': 'x72', 't': 'x74', 'u': 'x75', 'v': 'x76', 'x': 'x78'
  };

  /** Used to escape characters for inclusion in compiled string literals. */
  var stringEscapes = {
    '\\': '\\',
    "'": "'",
    '\n': 'n',
    '\r': 'r',
    '\u2028': 'u2028',
    '\u2029': 'u2029'
  };

  /** Detect free variable `exports`. */
  var freeExports = objectTypes[typeof exports] && exports && !exports.nodeType && exports;

  /** Detect free variable `module`. */
  var freeModule = objectTypes[typeof module] && module && !module.nodeType && module;

  /** Detect free variable `global` from Node.js. */
  var freeGlobal = freeExports && freeModule && typeof global == 'object' && global && global.Object && global;

  /** Detect free variable `self`. */
  var freeSelf = objectTypes[typeof self] && self && self.Object && self;

  /** Detect free variable `window`. */
  var freeWindow = objectTypes[typeof window] && window && window.Object && window;

  /** Detect the popular CommonJS extension `module.exports`. */
  var moduleExports = freeModule && freeModule.exports === freeExports && freeExports;

  /**
   * Used as a reference to the global object.
   *
   * The `this` value is used if it's the global object to avoid Greasemonkey's
   * restricted `window` object, otherwise the `window` object is used.
   */
  var root = freeGlobal || ((freeWindow !== (this && this.window)) && freeWindow) || freeSelf || this;

  /*--------------------------------------------------------------------------*/

  /**
   * The base implementation of `compareAscending` which compares values and
   * sorts them in ascending order without guaranteeing a stable sort.
   *
   * @private
   * @param {*} value The value to compare.
   * @param {*} other The other value to compare.
   * @returns {number} Returns the sort order indicator for `value`.
   */
  function baseCompareAscending(value, other) {
    if (value !== other) {
      var valIsNull = value === null,
          valIsUndef = value === undefined,
          valIsReflexive = value === value;

      var othIsNull = other === null,
          othIsUndef = other === undefined,
          othIsReflexive = other === other;

      if ((value > other && !othIsNull) || !valIsReflexive ||
          (valIsNull && !othIsUndef && othIsReflexive) ||
          (valIsUndef && othIsReflexive)) {
        return 1;
      }
      if ((value < other && !valIsNull) || !othIsReflexive ||
          (othIsNull && !valIsUndef && valIsReflexive) ||
          (othIsUndef && valIsReflexive)) {
        return -1;
      }
    }
    return 0;
  }

  /**
   * The base implementation of `_.findIndex` and `_.findLastIndex` without
   * support for callback shorthands and `this` binding.
   *
   * @private
   * @param {Array} array The array to search.
   * @param {Function} predicate The function invoked per iteration.
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {number} Returns the index of the matched value, else `-1`.
   */
  function baseFindIndex(array, predicate, fromRight) {
    var length = array.length,
        index = fromRight ? length : -1;

    while ((fromRight ? index-- : ++index < length)) {
      if (predicate(array[index], index, array)) {
        return index;
      }
    }
    return -1;
  }

  /**
   * The base implementation of `_.indexOf` without support for binary searches.
   *
   * @private
   * @param {Array} array The array to search.
   * @param {*} value The value to search for.
   * @param {number} fromIndex The index to search from.
   * @returns {number} Returns the index of the matched value, else `-1`.
   */
  function baseIndexOf(array, value, fromIndex) {
    if (value !== value) {
      return indexOfNaN(array, fromIndex);
    }
    var index = fromIndex - 1,
        length = array.length;

    while (++index < length) {
      if (array[index] === value) {
        return index;
      }
    }
    return -1;
  }

  /**
   * The base implementation of `_.isFunction` without support for environments
   * with incorrect `typeof` results.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
   */
  function baseIsFunction(value) {
    // Avoid a Chakra JIT bug in compatibility modes of IE 11.
    // See https://github.com/jashkenas/underscore/issues/1621 for more details.
    return typeof value == 'function' || false;
  }

  /**
   * Converts `value` to a string if it's not one. An empty string is returned
   * for `null` or `undefined` values.
   *
   * @private
   * @param {*} value The value to process.
   * @returns {string} Returns the string.
   */
  function baseToString(value) {
    return value == null ? '' : (value + '');
  }

  /**
   * Used by `_.trim` and `_.trimLeft` to get the index of the first character
   * of `string` that is not found in `chars`.
   *
   * @private
   * @param {string} string The string to inspect.
   * @param {string} chars The characters to find.
   * @returns {number} Returns the index of the first character not found in `chars`.
   */
  function charsLeftIndex(string, chars) {
    var index = -1,
        length = string.length;

    while (++index < length && chars.indexOf(string.charAt(index)) > -1) {}
    return index;
  }

  /**
   * Used by `_.trim` and `_.trimRight` to get the index of the last character
   * of `string` that is not found in `chars`.
   *
   * @private
   * @param {string} string The string to inspect.
   * @param {string} chars The characters to find.
   * @returns {number} Returns the index of the last character not found in `chars`.
   */
  function charsRightIndex(string, chars) {
    var index = string.length;

    while (index-- && chars.indexOf(string.charAt(index)) > -1) {}
    return index;
  }

  /**
   * Used by `_.sortBy` to compare transformed elements of a collection and stable
   * sort them in ascending order.
   *
   * @private
   * @param {Object} object The object to compare.
   * @param {Object} other The other object to compare.
   * @returns {number} Returns the sort order indicator for `object`.
   */
  function compareAscending(object, other) {
    return baseCompareAscending(object.criteria, other.criteria) || (object.index - other.index);
  }

  /**
   * Used by `_.sortByOrder` to compare multiple properties of a value to another
   * and stable sort them.
   *
   * If `orders` is unspecified, all valuess are sorted in ascending order. Otherwise,
   * a value is sorted in ascending order if its corresponding order is "asc", and
   * descending if "desc".
   *
   * @private
   * @param {Object} object The object to compare.
   * @param {Object} other The other object to compare.
   * @param {boolean[]} orders The order to sort by for each property.
   * @returns {number} Returns the sort order indicator for `object`.
   */
  function compareMultiple(object, other, orders) {
    var index = -1,
        objCriteria = object.criteria,
        othCriteria = other.criteria,
        length = objCriteria.length,
        ordersLength = orders.length;

    while (++index < length) {
      var result = baseCompareAscending(objCriteria[index], othCriteria[index]);
      if (result) {
        if (index >= ordersLength) {
          return result;
        }
        var order = orders[index];
        return result * ((order === 'asc' || order === true) ? 1 : -1);
      }
    }
    // Fixes an `Array#sort` bug in the JS engine embedded in Adobe applications
    // that causes it, under certain circumstances, to provide the same value for
    // `object` and `other`. See https://github.com/jashkenas/underscore/pull/1247
    // for more details.
    //
    // This also ensures a stable sort in V8 and other engines.
    // See https://code.google.com/p/v8/issues/detail?id=90 for more details.
    return object.index - other.index;
  }

  /**
   * Used by `_.deburr` to convert latin-1 supplementary letters to basic latin letters.
   *
   * @private
   * @param {string} letter The matched letter to deburr.
   * @returns {string} Returns the deburred letter.
   */
  function deburrLetter(letter) {
    return deburredLetters[letter];
  }

  /**
   * Used by `_.escape` to convert characters to HTML entities.
   *
   * @private
   * @param {string} chr The matched character to escape.
   * @returns {string} Returns the escaped character.
   */
  function escapeHtmlChar(chr) {
    return htmlEscapes[chr];
  }

  /**
   * Used by `_.escapeRegExp` to escape characters for inclusion in compiled regexes.
   *
   * @private
   * @param {string} chr The matched character to escape.
   * @param {string} leadingChar The capture group for a leading character.
   * @param {string} whitespaceChar The capture group for a whitespace character.
   * @returns {string} Returns the escaped character.
   */
  function escapeRegExpChar(chr, leadingChar, whitespaceChar) {
    if (leadingChar) {
      chr = regexpEscapes[chr];
    } else if (whitespaceChar) {
      chr = stringEscapes[chr];
    }
    return '\\' + chr;
  }

  /**
   * Used by `_.template` to escape characters for inclusion in compiled string literals.
   *
   * @private
   * @param {string} chr The matched character to escape.
   * @returns {string} Returns the escaped character.
   */
  function escapeStringChar(chr) {
    return '\\' + stringEscapes[chr];
  }

  /**
   * Gets the index at which the first occurrence of `NaN` is found in `array`.
   *
   * @private
   * @param {Array} array The array to search.
   * @param {number} fromIndex The index to search from.
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {number} Returns the index of the matched `NaN`, else `-1`.
   */
  function indexOfNaN(array, fromIndex, fromRight) {
    var length = array.length,
        index = fromIndex + (fromRight ? 0 : -1);

    while ((fromRight ? index-- : ++index < length)) {
      var other = array[index];
      if (other !== other) {
        return index;
      }
    }
    return -1;
  }

  /**
   * Checks if `value` is object-like.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is object-like, else `false`.
   */
  function isObjectLike(value) {
    return !!value && typeof value == 'object';
  }

  /**
   * Used by `trimmedLeftIndex` and `trimmedRightIndex` to determine if a
   * character code is whitespace.
   *
   * @private
   * @param {number} charCode The character code to inspect.
   * @returns {boolean} Returns `true` if `charCode` is whitespace, else `false`.
   */
  function isSpace(charCode) {
    return ((charCode <= 160 && (charCode >= 9 && charCode <= 13) || charCode == 32 || charCode == 160) || charCode == 5760 || charCode == 6158 ||
      (charCode >= 8192 && (charCode <= 8202 || charCode == 8232 || charCode == 8233 || charCode == 8239 || charCode == 8287 || charCode == 12288 || charCode == 65279)));
  }

  /**
   * Replaces all `placeholder` elements in `array` with an internal placeholder
   * and returns an array of their indexes.
   *
   * @private
   * @param {Array} array The array to modify.
   * @param {*} placeholder The placeholder to replace.
   * @returns {Array} Returns the new array of placeholder indexes.
   */
  function replaceHolders(array, placeholder) {
    var index = -1,
        length = array.length,
        resIndex = -1,
        result = [];

    while (++index < length) {
      if (array[index] === placeholder) {
        array[index] = PLACEHOLDER;
        result[++resIndex] = index;
      }
    }
    return result;
  }

  /**
   * An implementation of `_.uniq` optimized for sorted arrays without support
   * for callback shorthands and `this` binding.
   *
   * @private
   * @param {Array} array The array to inspect.
   * @param {Function} [iteratee] The function invoked per iteration.
   * @returns {Array} Returns the new duplicate-value-free array.
   */
  function sortedUniq(array, iteratee) {
    var seen,
        index = -1,
        length = array.length,
        resIndex = -1,
        result = [];

    while (++index < length) {
      var value = array[index],
          computed = iteratee ? iteratee(value, index, array) : value;

      if (!index || seen !== computed) {
        seen = computed;
        result[++resIndex] = value;
      }
    }
    return result;
  }

  /**
   * Used by `_.trim` and `_.trimLeft` to get the index of the first non-whitespace
   * character of `string`.
   *
   * @private
   * @param {string} string The string to inspect.
   * @returns {number} Returns the index of the first non-whitespace character.
   */
  function trimmedLeftIndex(string) {
    var index = -1,
        length = string.length;

    while (++index < length && isSpace(string.charCodeAt(index))) {}
    return index;
  }

  /**
   * Used by `_.trim` and `_.trimRight` to get the index of the last non-whitespace
   * character of `string`.
   *
   * @private
   * @param {string} string The string to inspect.
   * @returns {number} Returns the index of the last non-whitespace character.
   */
  function trimmedRightIndex(string) {
    var index = string.length;

    while (index-- && isSpace(string.charCodeAt(index))) {}
    return index;
  }

  /**
   * Used by `_.unescape` to convert HTML entities to characters.
   *
   * @private
   * @param {string} chr The matched character to unescape.
   * @returns {string} Returns the unescaped character.
   */
  function unescapeHtmlChar(chr) {
    return htmlUnescapes[chr];
  }

  /*--------------------------------------------------------------------------*/

  /**
   * Create a new pristine `lodash` function using the given `context` object.
   *
   * @static
   * @memberOf _
   * @category Utility
   * @param {Object} [context=root] The context object.
   * @returns {Function} Returns a new `lodash` function.
   * @example
   *
   * _.mixin({ 'foo': _.constant('foo') });
   *
   * var lodash = _.runInContext();
   * lodash.mixin({ 'bar': lodash.constant('bar') });
   *
   * _.isFunction(_.foo);
   * // => true
   * _.isFunction(_.bar);
   * // => false
   *
   * lodash.isFunction(lodash.foo);
   * // => false
   * lodash.isFunction(lodash.bar);
   * // => true
   *
   * // using `context` to mock `Date#getTime` use in `_.now`
   * var mock = _.runInContext({
   *   'Date': function() {
   *     return { 'getTime': getTimeMock };
   *   }
   * });
   *
   * // or creating a suped-up `defer` in Node.js
   * var defer = _.runInContext({ 'setTimeout': setImmediate }).defer;
   */
  function runInContext(context) {
    // Avoid issues with some ES3 environments that attempt to use values, named
    // after built-in constructors like `Object`, for the creation of literals.
    // ES5 clears this up by stating that literals must use built-in constructors.
    // See https://es5.github.io/#x11.1.5 for more details.
    context = context ? _.defaults(root.Object(), context, _.pick(root, contextProps)) : root;

    /** Native constructor references. */
    var Array = context.Array,
        Date = context.Date,
        Error = context.Error,
        Function = context.Function,
        Math = context.Math,
        Number = context.Number,
        Object = context.Object,
        RegExp = context.RegExp,
        String = context.String,
        TypeError = context.TypeError;

    /** Used for native method references. */
    var arrayProto = Array.prototype,
        objectProto = Object.prototype,
        stringProto = String.prototype;

    /** Used to resolve the decompiled source of functions. */
    var fnToString = Function.prototype.toString;

    /** Used to check objects for own properties. */
    var hasOwnProperty = objectProto.hasOwnProperty;

    /** Used to generate unique IDs. */
    var idCounter = 0;

    /**
     * Used to resolve the [`toStringTag`](http://ecma-international.org/ecma-262/6.0/#sec-object.prototype.tostring)
     * of values.
     */
    var objToString = objectProto.toString;

    /** Used to restore the original `_` reference in `_.noConflict`. */
    var oldDash = root._;

    /** Used to detect if a method is native. */
    var reIsNative = RegExp('^' +
      fnToString.call(hasOwnProperty).replace(/[\\^$.*+?()[\]{}|]/g, '\\$&')
      .replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g, '$1.*?') + '$'
    );

    /** Native method references. */
    var ArrayBuffer = context.ArrayBuffer,
        clearTimeout = context.clearTimeout,
        parseFloat = context.parseFloat,
        pow = Math.pow,
        propertyIsEnumerable = objectProto.propertyIsEnumerable,
        Set = getNative(context, 'Set'),
        setTimeout = context.setTimeout,
        splice = arrayProto.splice,
        Uint8Array = context.Uint8Array,
        WeakMap = getNative(context, 'WeakMap');

    /* Native method references for those with the same name as other `lodash` methods. */
    var nativeCeil = Math.ceil,
        nativeCreate = getNative(Object, 'create'),
        nativeFloor = Math.floor,
        nativeIsArray = getNative(Array, 'isArray'),
        nativeIsFinite = context.isFinite,
        nativeKeys = getNative(Object, 'keys'),
        nativeMax = Math.max,
        nativeMin = Math.min,
        nativeNow = getNative(Date, 'now'),
        nativeParseInt = context.parseInt,
        nativeRandom = Math.random;

    /** Used as references for `-Infinity` and `Infinity`. */
    var NEGATIVE_INFINITY = Number.NEGATIVE_INFINITY,
        POSITIVE_INFINITY = Number.POSITIVE_INFINITY;

    /** Used as references for the maximum length and index of an array. */
    var MAX_ARRAY_LENGTH = 4294967295,
        MAX_ARRAY_INDEX = MAX_ARRAY_LENGTH - 1,
        HALF_MAX_ARRAY_LENGTH = MAX_ARRAY_LENGTH >>> 1;

    /**
     * Used as the [maximum length](http://ecma-international.org/ecma-262/6.0/#sec-number.max_safe_integer)
     * of an array-like value.
     */
    var MAX_SAFE_INTEGER = 9007199254740991;

    /** Used to store function metadata. */
    var metaMap = WeakMap && new WeakMap;

    /** Used to lookup unminified function names. */
    var realNames = {};

    /*------------------------------------------------------------------------*/

    /**
     * Creates a `lodash` object which wraps `value` to enable implicit chaining.
     * Methods that operate on and return arrays, collections, and functions can
     * be chained together. Methods that retrieve a single value or may return a
     * primitive value will automatically end the chain returning the unwrapped
     * value. Explicit chaining may be enabled using `_.chain`. The execution of
     * chained methods is lazy, that is, execution is deferred until `_#value`
     * is implicitly or explicitly called.
     *
     * Lazy evaluation allows several methods to support shortcut fusion. Shortcut
     * fusion is an optimization strategy which merge iteratee calls; this can help
     * to avoid the creation of intermediate data structures and greatly reduce the
     * number of iteratee executions.
     *
     * Chaining is supported in custom builds as long as the `_#value` method is
     * directly or indirectly included in the build.
     *
     * In addition to lodash methods, wrappers have `Array` and `String` methods.
     *
     * The wrapper `Array` methods are:
     * `concat`, `join`, `pop`, `push`, `reverse`, `shift`, `slice`, `sort`,
     * `splice`, and `unshift`
     *
     * The wrapper `String` methods are:
     * `replace` and `split`
     *
     * The wrapper methods that support shortcut fusion are:
     * `compact`, `drop`, `dropRight`, `dropRightWhile`, `dropWhile`, `filter`,
     * `first`, `initial`, `last`, `map`, `pluck`, `reject`, `rest`, `reverse`,
     * `slice`, `take`, `takeRight`, `takeRightWhile`, `takeWhile`, `toArray`,
     * and `where`
     *
     * The chainable wrapper methods are:
     * `after`, `ary`, `assign`, `at`, `before`, `bind`, `bindAll`, `bindKey`,
     * `callback`, `chain`, `chunk`, `commit`, `compact`, `concat`, `constant`,
     * `countBy`, `create`, `curry`, `debounce`, `defaults`, `defaultsDeep`,
     * `defer`, `delay`, `difference`, `drop`, `dropRight`, `dropRightWhile`,
     * `dropWhile`, `fill`, `filter`, `flatten`, `flattenDeep`, `flow`, `flowRight`,
     * `forEach`, `forEachRight`, `forIn`, `forInRight`, `forOwn`, `forOwnRight`,
     * `functions`, `groupBy`, `indexBy`, `initial`, `intersection`, `invert`,
     * `invoke`, `keys`, `keysIn`, `map`, `mapKeys`, `mapValues`, `matches`,
     * `matchesProperty`, `memoize`, `merge`, `method`, `methodOf`, `mixin`,
     * `modArgs`, `negate`, `omit`, `once`, `pairs`, `partial`, `partialRight`,
     * `partition`, `pick`, `plant`, `pluck`, `property`, `propertyOf`, `pull`,
     * `pullAt`, `push`, `range`, `rearg`, `reject`, `remove`, `rest`, `restParam`,
     * `reverse`, `set`, `shuffle`, `slice`, `sort`, `sortBy`, `sortByAll`,
     * `sortByOrder`, `splice`, `spread`, `take`, `takeRight`, `takeRightWhile`,
     * `takeWhile`, `tap`, `throttle`, `thru`, `times`, `toArray`, `toPlainObject`,
     * `transform`, `union`, `uniq`, `unshift`, `unzip`, `unzipWith`, `values`,
     * `valuesIn`, `where`, `without`, `wrap`, `xor`, `zip`, `zipObject`, `zipWith`
     *
     * The wrapper methods that are **not** chainable by default are:
     * `add`, `attempt`, `camelCase`, `capitalize`, `ceil`, `clone`, `cloneDeep`,
     * `deburr`, `endsWith`, `escape`, `escapeRegExp`, `every`, `find`, `findIndex`,
     * `findKey`, `findLast`, `findLastIndex`, `findLastKey`, `findWhere`, `first`,
     * `floor`, `get`, `gt`, `gte`, `has`, `identity`, `includes`, `indexOf`,
     * `inRange`, `isArguments`, `isArray`, `isBoolean`, `isDate`, `isElement`,
     * `isEmpty`, `isEqual`, `isError`, `isFinite` `isFunction`, `isMatch`,
     * `isNative`, `isNaN`, `isNull`, `isNumber`, `isObject`, `isPlainObject`,
     * `isRegExp`, `isString`, `isUndefined`, `isTypedArray`, `join`, `kebabCase`,
     * `last`, `lastIndexOf`, `lt`, `lte`, `max`, `min`, `noConflict`, `noop`,
     * `now`, `pad`, `padLeft`, `padRight`, `parseInt`, `pop`, `random`, `reduce`,
     * `reduceRight`, `repeat`, `result`, `round`, `runInContext`, `shift`, `size`,
     * `snakeCase`, `some`, `sortedIndex`, `sortedLastIndex`, `startCase`,
     * `startsWith`, `sum`, `template`, `trim`, `trimLeft`, `trimRight`, `trunc`,
     * `unescape`, `uniqueId`, `value`, and `words`
     *
     * The wrapper method `sample` will return a wrapped value when `n` is provided,
     * otherwise an unwrapped value is returned.
     *
     * @name _
     * @constructor
     * @category Chain
     * @param {*} value The value to wrap in a `lodash` instance.
     * @returns {Object} Returns the new `lodash` wrapper instance.
     * @example
     *
     * var wrapped = _([1, 2, 3]);
     *
     * // returns an unwrapped value
     * wrapped.reduce(function(total, n) {
     *   return total + n;
     * });
     * // => 6
     *
     * // returns a wrapped value
     * var squares = wrapped.map(function(n) {
     *   return n * n;
     * });
     *
     * _.isArray(squares);
     * // => false
     *
     * _.isArray(squares.value());
     * // => true
     */
    function lodash(value) {
      if (isObjectLike(value) && !isArray(value) && !(value instanceof LazyWrapper)) {
        if (value instanceof LodashWrapper) {
          return value;
        }
        if (hasOwnProperty.call(value, '__chain__') && hasOwnProperty.call(value, '__wrapped__')) {
          return wrapperClone(value);
        }
      }
      return new LodashWrapper(value);
    }

    /**
     * The function whose prototype all chaining wrappers inherit from.
     *
     * @private
     */
    function baseLodash() {
      // No operation performed.
    }

    /**
     * The base constructor for creating `lodash` wrapper objects.
     *
     * @private
     * @param {*} value The value to wrap.
     * @param {boolean} [chainAll] Enable chaining for all wrapper methods.
     * @param {Array} [actions=[]] Actions to peform to resolve the unwrapped value.
     */
    function LodashWrapper(value, chainAll, actions) {
      this.__wrapped__ = value;
      this.__actions__ = actions || [];
      this.__chain__ = !!chainAll;
    }

    /**
     * An object environment feature flags.
     *
     * @static
     * @memberOf _
     * @type Object
     */
    var support = lodash.support = {};

    /**
     * By default, the template delimiters used by lodash are like those in
     * embedded Ruby (ERB). Change the following template settings to use
     * alternative delimiters.
     *
     * @static
     * @memberOf _
     * @type Object
     */
    lodash.templateSettings = {

      /**
       * Used to detect `data` property values to be HTML-escaped.
       *
       * @memberOf _.templateSettings
       * @type RegExp
       */
      'escape': reEscape,

      /**
       * Used to detect code to be evaluated.
       *
       * @memberOf _.templateSettings
       * @type RegExp
       */
      'evaluate': reEvaluate,

      /**
       * Used to detect `data` property values to inject.
       *
       * @memberOf _.templateSettings
       * @type RegExp
       */
      'interpolate': reInterpolate,

      /**
       * Used to reference the data object in the template text.
       *
       * @memberOf _.templateSettings
       * @type string
       */
      'variable': '',

      /**
       * Used to import variables into the compiled template.
       *
       * @memberOf _.templateSettings
       * @type Object
       */
      'imports': {

        /**
         * A reference to the `lodash` function.
         *
         * @memberOf _.templateSettings.imports
         * @type Function
         */
        '_': lodash
      }
    };

    /*------------------------------------------------------------------------*/

    /**
     * Creates a lazy wrapper object which wraps `value` to enable lazy evaluation.
     *
     * @private
     * @param {*} value The value to wrap.
     */
    function LazyWrapper(value) {
      this.__wrapped__ = value;
      this.__actions__ = [];
      this.__dir__ = 1;
      this.__filtered__ = false;
      this.__iteratees__ = [];
      this.__takeCount__ = POSITIVE_INFINITY;
      this.__views__ = [];
    }

    /**
     * Creates a clone of the lazy wrapper object.
     *
     * @private
     * @name clone
     * @memberOf LazyWrapper
     * @returns {Object} Returns the cloned `LazyWrapper` object.
     */
    function lazyClone() {
      var result = new LazyWrapper(this.__wrapped__);
      result.__actions__ = arrayCopy(this.__actions__);
      result.__dir__ = this.__dir__;
      result.__filtered__ = this.__filtered__;
      result.__iteratees__ = arrayCopy(this.__iteratees__);
      result.__takeCount__ = this.__takeCount__;
      result.__views__ = arrayCopy(this.__views__);
      return result;
    }

    /**
     * Reverses the direction of lazy iteration.
     *
     * @private
     * @name reverse
     * @memberOf LazyWrapper
     * @returns {Object} Returns the new reversed `LazyWrapper` object.
     */
    function lazyReverse() {
      if (this.__filtered__) {
        var result = new LazyWrapper(this);
        result.__dir__ = -1;
        result.__filtered__ = true;
      } else {
        result = this.clone();
        result.__dir__ *= -1;
      }
      return result;
    }

    /**
     * Extracts the unwrapped value from its lazy wrapper.
     *
     * @private
     * @name value
     * @memberOf LazyWrapper
     * @returns {*} Returns the unwrapped value.
     */
    function lazyValue() {
      var array = this.__wrapped__.value(),
          dir = this.__dir__,
          isArr = isArray(array),
          isRight = dir < 0,
          arrLength = isArr ? array.length : 0,
          view = getView(0, arrLength, this.__views__),
          start = view.start,
          end = view.end,
          length = end - start,
          index = isRight ? end : (start - 1),
          iteratees = this.__iteratees__,
          iterLength = iteratees.length,
          resIndex = 0,
          takeCount = nativeMin(length, this.__takeCount__);

      if (!isArr || arrLength < LARGE_ARRAY_SIZE || (arrLength == length && takeCount == length)) {
        return baseWrapperValue((isRight && isArr) ? array.reverse() : array, this.__actions__);
      }
      var result = [];

      outer:
      while (length-- && resIndex < takeCount) {
        index += dir;

        var iterIndex = -1,
            value = array[index];

        while (++iterIndex < iterLength) {
          var data = iteratees[iterIndex],
              iteratee = data.iteratee,
              type = data.type,
              computed = iteratee(value);

          if (type == LAZY_MAP_FLAG) {
            value = computed;
          } else if (!computed) {
            if (type == LAZY_FILTER_FLAG) {
              continue outer;
            } else {
              break outer;
            }
          }
        }
        result[resIndex++] = value;
      }
      return result;
    }

    /*------------------------------------------------------------------------*/

    /**
     * Creates a cache object to store key/value pairs.
     *
     * @private
     * @static
     * @name Cache
     * @memberOf _.memoize
     */
    function MapCache() {
      this.__data__ = {};
    }

    /**
     * Removes `key` and its value from the cache.
     *
     * @private
     * @name delete
     * @memberOf _.memoize.Cache
     * @param {string} key The key of the value to remove.
     * @returns {boolean} Returns `true` if the entry was removed successfully, else `false`.
     */
    function mapDelete(key) {
      return this.has(key) && delete this.__data__[key];
    }

    /**
     * Gets the cached value for `key`.
     *
     * @private
     * @name get
     * @memberOf _.memoize.Cache
     * @param {string} key The key of the value to get.
     * @returns {*} Returns the cached value.
     */
    function mapGet(key) {
      return key == '__proto__' ? undefined : this.__data__[key];
    }

    /**
     * Checks if a cached value for `key` exists.
     *
     * @private
     * @name has
     * @memberOf _.memoize.Cache
     * @param {string} key The key of the entry to check.
     * @returns {boolean} Returns `true` if an entry for `key` exists, else `false`.
     */
    function mapHas(key) {
      return key != '__proto__' && hasOwnProperty.call(this.__data__, key);
    }

    /**
     * Sets `value` to `key` of the cache.
     *
     * @private
     * @name set
     * @memberOf _.memoize.Cache
     * @param {string} key The key of the value to cache.
     * @param {*} value The value to cache.
     * @returns {Object} Returns the cache object.
     */
    function mapSet(key, value) {
      if (key != '__proto__') {
        this.__data__[key] = value;
      }
      return this;
    }

    /*------------------------------------------------------------------------*/

    /**
     *
     * Creates a cache object to store unique values.
     *
     * @private
     * @param {Array} [values] The values to cache.
     */
    function SetCache(values) {
      var length = values ? values.length : 0;

      this.data = { 'hash': nativeCreate(null), 'set': new Set };
      while (length--) {
        this.push(values[length]);
      }
    }

    /**
     * Checks if `value` is in `cache` mimicking the return signature of
     * `_.indexOf` by returning `0` if the value is found, else `-1`.
     *
     * @private
     * @param {Object} cache The cache to search.
     * @param {*} value The value to search for.
     * @returns {number} Returns `0` if `value` is found, else `-1`.
     */
    function cacheIndexOf(cache, value) {
      var data = cache.data,
          result = (typeof value == 'string' || isObject(value)) ? data.set.has(value) : data.hash[value];

      return result ? 0 : -1;
    }

    /**
     * Adds `value` to the cache.
     *
     * @private
     * @name push
     * @memberOf SetCache
     * @param {*} value The value to cache.
     */
    function cachePush(value) {
      var data = this.data;
      if (typeof value == 'string' || isObject(value)) {
        data.set.add(value);
      } else {
        data.hash[value] = true;
      }
    }

    /*------------------------------------------------------------------------*/

    /**
     * Creates a new array joining `array` with `other`.
     *
     * @private
     * @param {Array} array The array to join.
     * @param {Array} other The other array to join.
     * @returns {Array} Returns the new concatenated array.
     */
    function arrayConcat(array, other) {
      var index = -1,
          length = array.length,
          othIndex = -1,
          othLength = other.length,
          result = Array(length + othLength);

      while (++index < length) {
        result[index] = array[index];
      }
      while (++othIndex < othLength) {
        result[index++] = other[othIndex];
      }
      return result;
    }

    /**
     * Copies the values of `source` to `array`.
     *
     * @private
     * @param {Array} source The array to copy values from.
     * @param {Array} [array=[]] The array to copy values to.
     * @returns {Array} Returns `array`.
     */
    function arrayCopy(source, array) {
      var index = -1,
          length = source.length;

      array || (array = Array(length));
      while (++index < length) {
        array[index] = source[index];
      }
      return array;
    }

    /**
     * A specialized version of `_.forEach` for arrays without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Array} Returns `array`.
     */
    function arrayEach(array, iteratee) {
      var index = -1,
          length = array.length;

      while (++index < length) {
        if (iteratee(array[index], index, array) === false) {
          break;
        }
      }
      return array;
    }

    /**
     * A specialized version of `_.forEachRight` for arrays without support for
     * callback shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Array} Returns `array`.
     */
    function arrayEachRight(array, iteratee) {
      var length = array.length;

      while (length--) {
        if (iteratee(array[length], length, array) === false) {
          break;
        }
      }
      return array;
    }

    /**
     * A specialized version of `_.every` for arrays without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {boolean} Returns `true` if all elements pass the predicate check,
     *  else `false`.
     */
    function arrayEvery(array, predicate) {
      var index = -1,
          length = array.length;

      while (++index < length) {
        if (!predicate(array[index], index, array)) {
          return false;
        }
      }
      return true;
    }

    /**
     * A specialized version of `baseExtremum` for arrays which invokes `iteratee`
     * with one argument: (value).
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {Function} comparator The function used to compare values.
     * @param {*} exValue The initial extremum value.
     * @returns {*} Returns the extremum value.
     */
    function arrayExtremum(array, iteratee, comparator, exValue) {
      var index = -1,
          length = array.length,
          computed = exValue,
          result = computed;

      while (++index < length) {
        var value = array[index],
            current = +iteratee(value);

        if (comparator(current, computed)) {
          computed = current;
          result = value;
        }
      }
      return result;
    }

    /**
     * A specialized version of `_.filter` for arrays without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {Array} Returns the new filtered array.
     */
    function arrayFilter(array, predicate) {
      var index = -1,
          length = array.length,
          resIndex = -1,
          result = [];

      while (++index < length) {
        var value = array[index];
        if (predicate(value, index, array)) {
          result[++resIndex] = value;
        }
      }
      return result;
    }

    /**
     * A specialized version of `_.map` for arrays without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Array} Returns the new mapped array.
     */
    function arrayMap(array, iteratee) {
      var index = -1,
          length = array.length,
          result = Array(length);

      while (++index < length) {
        result[index] = iteratee(array[index], index, array);
      }
      return result;
    }

    /**
     * Appends the elements of `values` to `array`.
     *
     * @private
     * @param {Array} array The array to modify.
     * @param {Array} values The values to append.
     * @returns {Array} Returns `array`.
     */
    function arrayPush(array, values) {
      var index = -1,
          length = values.length,
          offset = array.length;

      while (++index < length) {
        array[offset + index] = values[index];
      }
      return array;
    }

    /**
     * A specialized version of `_.reduce` for arrays without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {*} [accumulator] The initial value.
     * @param {boolean} [initFromArray] Specify using the first element of `array`
     *  as the initial value.
     * @returns {*} Returns the accumulated value.
     */
    function arrayReduce(array, iteratee, accumulator, initFromArray) {
      var index = -1,
          length = array.length;

      if (initFromArray && length) {
        accumulator = array[++index];
      }
      while (++index < length) {
        accumulator = iteratee(accumulator, array[index], index, array);
      }
      return accumulator;
    }

    /**
     * A specialized version of `_.reduceRight` for arrays without support for
     * callback shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {*} [accumulator] The initial value.
     * @param {boolean} [initFromArray] Specify using the last element of `array`
     *  as the initial value.
     * @returns {*} Returns the accumulated value.
     */
    function arrayReduceRight(array, iteratee, accumulator, initFromArray) {
      var length = array.length;
      if (initFromArray && length) {
        accumulator = array[--length];
      }
      while (length--) {
        accumulator = iteratee(accumulator, array[length], length, array);
      }
      return accumulator;
    }

    /**
     * A specialized version of `_.some` for arrays without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {boolean} Returns `true` if any element passes the predicate check,
     *  else `false`.
     */
    function arraySome(array, predicate) {
      var index = -1,
          length = array.length;

      while (++index < length) {
        if (predicate(array[index], index, array)) {
          return true;
        }
      }
      return false;
    }

    /**
     * A specialized version of `_.sum` for arrays without support for callback
     * shorthands and `this` binding..
     *
     * @private
     * @param {Array} array The array to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {number} Returns the sum.
     */
    function arraySum(array, iteratee) {
      var length = array.length,
          result = 0;

      while (length--) {
        result += +iteratee(array[length]) || 0;
      }
      return result;
    }

    /**
     * Used by `_.defaults` to customize its `_.assign` use.
     *
     * @private
     * @param {*} objectValue The destination object property value.
     * @param {*} sourceValue The source object property value.
     * @returns {*} Returns the value to assign to the destination object.
     */
    function assignDefaults(objectValue, sourceValue) {
      return objectValue === undefined ? sourceValue : objectValue;
    }

    /**
     * Used by `_.template` to customize its `_.assign` use.
     *
     * **Note:** This function is like `assignDefaults` except that it ignores
     * inherited property values when checking if a property is `undefined`.
     *
     * @private
     * @param {*} objectValue The destination object property value.
     * @param {*} sourceValue The source object property value.
     * @param {string} key The key associated with the object and source values.
     * @param {Object} object The destination object.
     * @returns {*} Returns the value to assign to the destination object.
     */
    function assignOwnDefaults(objectValue, sourceValue, key, object) {
      return (objectValue === undefined || !hasOwnProperty.call(object, key))
        ? sourceValue
        : objectValue;
    }

    /**
     * A specialized version of `_.assign` for customizing assigned values without
     * support for argument juggling, multiple sources, and `this` binding `customizer`
     * functions.
     *
     * @private
     * @param {Object} object The destination object.
     * @param {Object} source The source object.
     * @param {Function} customizer The function to customize assigned values.
     * @returns {Object} Returns `object`.
     */
    function assignWith(object, source, customizer) {
      var index = -1,
          props = keys(source),
          length = props.length;

      while (++index < length) {
        var key = props[index],
            value = object[key],
            result = customizer(value, source[key], key, object, source);

        if ((result === result ? (result !== value) : (value === value)) ||
            (value === undefined && !(key in object))) {
          object[key] = result;
        }
      }
      return object;
    }

    /**
     * The base implementation of `_.assign` without support for argument juggling,
     * multiple sources, and `customizer` functions.
     *
     * @private
     * @param {Object} object The destination object.
     * @param {Object} source The source object.
     * @returns {Object} Returns `object`.
     */
    function baseAssign(object, source) {
      return source == null
        ? object
        : baseCopy(source, keys(source), object);
    }

    /**
     * The base implementation of `_.at` without support for string collections
     * and individual key arguments.
     *
     * @private
     * @param {Array|Object} collection The collection to iterate over.
     * @param {number[]|string[]} props The property names or indexes of elements to pick.
     * @returns {Array} Returns the new array of picked elements.
     */
    function baseAt(collection, props) {
      var index = -1,
          isNil = collection == null,
          isArr = !isNil && isArrayLike(collection),
          length = isArr ? collection.length : 0,
          propsLength = props.length,
          result = Array(propsLength);

      while(++index < propsLength) {
        var key = props[index];
        if (isArr) {
          result[index] = isIndex(key, length) ? collection[key] : undefined;
        } else {
          result[index] = isNil ? undefined : collection[key];
        }
      }
      return result;
    }

    /**
     * Copies properties of `source` to `object`.
     *
     * @private
     * @param {Object} source The object to copy properties from.
     * @param {Array} props The property names to copy.
     * @param {Object} [object={}] The object to copy properties to.
     * @returns {Object} Returns `object`.
     */
    function baseCopy(source, props, object) {
      object || (object = {});

      var index = -1,
          length = props.length;

      while (++index < length) {
        var key = props[index];
        object[key] = source[key];
      }
      return object;
    }

    /**
     * The base implementation of `_.callback` which supports specifying the
     * number of arguments to provide to `func`.
     *
     * @private
     * @param {*} [func=_.identity] The value to convert to a callback.
     * @param {*} [thisArg] The `this` binding of `func`.
     * @param {number} [argCount] The number of arguments to provide to `func`.
     * @returns {Function} Returns the callback.
     */
    function baseCallback(func, thisArg, argCount) {
      var type = typeof func;
      if (type == 'function') {
        return thisArg === undefined
          ? func
          : bindCallback(func, thisArg, argCount);
      }
      if (func == null) {
        return identity;
      }
      if (type == 'object') {
        return baseMatches(func);
      }
      return thisArg === undefined
        ? property(func)
        : baseMatchesProperty(func, thisArg);
    }

    /**
     * The base implementation of `_.clone` without support for argument juggling
     * and `this` binding `customizer` functions.
     *
     * @private
     * @param {*} value The value to clone.
     * @param {boolean} [isDeep] Specify a deep clone.
     * @param {Function} [customizer] The function to customize cloning values.
     * @param {string} [key] The key of `value`.
     * @param {Object} [object] The object `value` belongs to.
     * @param {Array} [stackA=[]] Tracks traversed source objects.
     * @param {Array} [stackB=[]] Associates clones with source counterparts.
     * @returns {*} Returns the cloned value.
     */
    function baseClone(value, isDeep, customizer, key, object, stackA, stackB) {
      var result;
      if (customizer) {
        result = object ? customizer(value, key, object) : customizer(value);
      }
      if (result !== undefined) {
        return result;
      }
      if (!isObject(value)) {
        return value;
      }
      var isArr = isArray(value);
      if (isArr) {
        result = initCloneArray(value);
        if (!isDeep) {
          return arrayCopy(value, result);
        }
      } else {
        var tag = objToString.call(value),
            isFunc = tag == funcTag;

        if (tag == objectTag || tag == argsTag || (isFunc && !object)) {
          result = initCloneObject(isFunc ? {} : value);
          if (!isDeep) {
            return baseAssign(result, value);
          }
        } else {
          return cloneableTags[tag]
            ? initCloneByTag(value, tag, isDeep)
            : (object ? value : {});
        }
      }
      // Check for circular references and return its corresponding clone.
      stackA || (stackA = []);
      stackB || (stackB = []);

      var length = stackA.length;
      while (length--) {
        if (stackA[length] == value) {
          return stackB[length];
        }
      }
      // Add the source value to the stack of traversed objects and associate it with its clone.
      stackA.push(value);
      stackB.push(result);

      // Recursively populate clone (susceptible to call stack limits).
      (isArr ? arrayEach : baseForOwn)(value, function(subValue, key) {
        result[key] = baseClone(subValue, isDeep, customizer, key, value, stackA, stackB);
      });
      return result;
    }

    /**
     * The base implementation of `_.create` without support for assigning
     * properties to the created object.
     *
     * @private
     * @param {Object} prototype The object to inherit from.
     * @returns {Object} Returns the new object.
     */
    var baseCreate = (function() {
      function object() {}
      return function(prototype) {
        if (isObject(prototype)) {
          object.prototype = prototype;
          var result = new object;
          object.prototype = undefined;
        }
        return result || {};
      };
    }());

    /**
     * The base implementation of `_.delay` and `_.defer` which accepts an index
     * of where to slice the arguments to provide to `func`.
     *
     * @private
     * @param {Function} func The function to delay.
     * @param {number} wait The number of milliseconds to delay invocation.
     * @param {Object} args The arguments provide to `func`.
     * @returns {number} Returns the timer id.
     */
    function baseDelay(func, wait, args) {
      if (typeof func != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      return setTimeout(function() { func.apply(undefined, args); }, wait);
    }

    /**
     * The base implementation of `_.difference` which accepts a single array
     * of values to exclude.
     *
     * @private
     * @param {Array} array The array to inspect.
     * @param {Array} values The values to exclude.
     * @returns {Array} Returns the new array of filtered values.
     */
    function baseDifference(array, values) {
      var length = array ? array.length : 0,
          result = [];

      if (!length) {
        return result;
      }
      var index = -1,
          indexOf = getIndexOf(),
          isCommon = indexOf == baseIndexOf,
          cache = (isCommon && values.length >= LARGE_ARRAY_SIZE) ? createCache(values) : null,
          valuesLength = values.length;

      if (cache) {
        indexOf = cacheIndexOf;
        isCommon = false;
        values = cache;
      }
      outer:
      while (++index < length) {
        var value = array[index];

        if (isCommon && value === value) {
          var valuesIndex = valuesLength;
          while (valuesIndex--) {
            if (values[valuesIndex] === value) {
              continue outer;
            }
          }
          result.push(value);
        }
        else if (indexOf(values, value, 0) < 0) {
          result.push(value);
        }
      }
      return result;
    }

    /**
     * The base implementation of `_.forEach` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Array|Object|string} Returns `collection`.
     */
    var baseEach = createBaseEach(baseForOwn);

    /**
     * The base implementation of `_.forEachRight` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Array|Object|string} Returns `collection`.
     */
    var baseEachRight = createBaseEach(baseForOwnRight, true);

    /**
     * The base implementation of `_.every` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {boolean} Returns `true` if all elements pass the predicate check,
     *  else `false`
     */
    function baseEvery(collection, predicate) {
      var result = true;
      baseEach(collection, function(value, index, collection) {
        result = !!predicate(value, index, collection);
        return result;
      });
      return result;
    }

    /**
     * Gets the extremum value of `collection` invoking `iteratee` for each value
     * in `collection` to generate the criterion by which the value is ranked.
     * The `iteratee` is invoked with three arguments: (value, index|key, collection).
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {Function} comparator The function used to compare values.
     * @param {*} exValue The initial extremum value.
     * @returns {*} Returns the extremum value.
     */
    function baseExtremum(collection, iteratee, comparator, exValue) {
      var computed = exValue,
          result = computed;

      baseEach(collection, function(value, index, collection) {
        var current = +iteratee(value, index, collection);
        if (comparator(current, computed) || (current === exValue && current === result)) {
          computed = current;
          result = value;
        }
      });
      return result;
    }

    /**
     * The base implementation of `_.fill` without an iteratee call guard.
     *
     * @private
     * @param {Array} array The array to fill.
     * @param {*} value The value to fill `array` with.
     * @param {number} [start=0] The start position.
     * @param {number} [end=array.length] The end position.
     * @returns {Array} Returns `array`.
     */
    function baseFill(array, value, start, end) {
      var length = array.length;

      start = start == null ? 0 : (+start || 0);
      if (start < 0) {
        start = -start > length ? 0 : (length + start);
      }
      end = (end === undefined || end > length) ? length : (+end || 0);
      if (end < 0) {
        end += length;
      }
      length = start > end ? 0 : (end >>> 0);
      start >>>= 0;

      while (start < length) {
        array[start++] = value;
      }
      return array;
    }

    /**
     * The base implementation of `_.filter` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {Array} Returns the new filtered array.
     */
    function baseFilter(collection, predicate) {
      var result = [];
      baseEach(collection, function(value, index, collection) {
        if (predicate(value, index, collection)) {
          result.push(value);
        }
      });
      return result;
    }

    /**
     * The base implementation of `_.find`, `_.findLast`, `_.findKey`, and `_.findLastKey`,
     * without support for callback shorthands and `this` binding, which iterates
     * over `collection` using the provided `eachFunc`.
     *
     * @private
     * @param {Array|Object|string} collection The collection to search.
     * @param {Function} predicate The function invoked per iteration.
     * @param {Function} eachFunc The function to iterate over `collection`.
     * @param {boolean} [retKey] Specify returning the key of the found element
     *  instead of the element itself.
     * @returns {*} Returns the found element or its key, else `undefined`.
     */
    function baseFind(collection, predicate, eachFunc, retKey) {
      var result;
      eachFunc(collection, function(value, key, collection) {
        if (predicate(value, key, collection)) {
          result = retKey ? key : value;
          return false;
        }
      });
      return result;
    }

    /**
     * The base implementation of `_.flatten` with added support for restricting
     * flattening and specifying the start index.
     *
     * @private
     * @param {Array} array The array to flatten.
     * @param {boolean} [isDeep] Specify a deep flatten.
     * @param {boolean} [isStrict] Restrict flattening to arrays-like objects.
     * @param {Array} [result=[]] The initial result value.
     * @returns {Array} Returns the new flattened array.
     */
    function baseFlatten(array, isDeep, isStrict, result) {
      result || (result = []);

      var index = -1,
          length = array.length;

      while (++index < length) {
        var value = array[index];
        if (isObjectLike(value) && isArrayLike(value) &&
            (isStrict || isArray(value) || isArguments(value))) {
          if (isDeep) {
            // Recursively flatten arrays (susceptible to call stack limits).
            baseFlatten(value, isDeep, isStrict, result);
          } else {
            arrayPush(result, value);
          }
        } else if (!isStrict) {
          result[result.length] = value;
        }
      }
      return result;
    }

    /**
     * The base implementation of `baseForIn` and `baseForOwn` which iterates
     * over `object` properties returned by `keysFunc` invoking `iteratee` for
     * each property. Iteratee functions may exit iteration early by explicitly
     * returning `false`.
     *
     * @private
     * @param {Object} object The object to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {Function} keysFunc The function to get the keys of `object`.
     * @returns {Object} Returns `object`.
     */
    var baseFor = createBaseFor();

    /**
     * This function is like `baseFor` except that it iterates over properties
     * in the opposite order.
     *
     * @private
     * @param {Object} object The object to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {Function} keysFunc The function to get the keys of `object`.
     * @returns {Object} Returns `object`.
     */
    var baseForRight = createBaseFor(true);

    /**
     * The base implementation of `_.forIn` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Object} object The object to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Object} Returns `object`.
     */
    function baseForIn(object, iteratee) {
      return baseFor(object, iteratee, keysIn);
    }

    /**
     * The base implementation of `_.forOwn` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Object} object The object to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Object} Returns `object`.
     */
    function baseForOwn(object, iteratee) {
      return baseFor(object, iteratee, keys);
    }

    /**
     * The base implementation of `_.forOwnRight` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Object} object The object to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Object} Returns `object`.
     */
    function baseForOwnRight(object, iteratee) {
      return baseForRight(object, iteratee, keys);
    }

    /**
     * The base implementation of `_.functions` which creates an array of
     * `object` function property names filtered from those provided.
     *
     * @private
     * @param {Object} object The object to inspect.
     * @param {Array} props The property names to filter.
     * @returns {Array} Returns the new array of filtered property names.
     */
    function baseFunctions(object, props) {
      var index = -1,
          length = props.length,
          resIndex = -1,
          result = [];

      while (++index < length) {
        var key = props[index];
        if (isFunction(object[key])) {
          result[++resIndex] = key;
        }
      }
      return result;
    }

    /**
     * The base implementation of `get` without support for string paths
     * and default values.
     *
     * @private
     * @param {Object} object The object to query.
     * @param {Array} path The path of the property to get.
     * @param {string} [pathKey] The key representation of path.
     * @returns {*} Returns the resolved value.
     */
    function baseGet(object, path, pathKey) {
      if (object == null) {
        return;
      }
      if (pathKey !== undefined && pathKey in toObject(object)) {
        path = [pathKey];
      }
      var index = 0,
          length = path.length;

      while (object != null && index < length) {
        object = object[path[index++]];
      }
      return (index && index == length) ? object : undefined;
    }

    /**
     * The base implementation of `_.isEqual` without support for `this` binding
     * `customizer` functions.
     *
     * @private
     * @param {*} value The value to compare.
     * @param {*} other The other value to compare.
     * @param {Function} [customizer] The function to customize comparing values.
     * @param {boolean} [isLoose] Specify performing partial comparisons.
     * @param {Array} [stackA] Tracks traversed `value` objects.
     * @param {Array} [stackB] Tracks traversed `other` objects.
     * @returns {boolean} Returns `true` if the values are equivalent, else `false`.
     */
    function baseIsEqual(value, other, customizer, isLoose, stackA, stackB) {
      if (value === other) {
        return true;
      }
      if (value == null || other == null || (!isObject(value) && !isObjectLike(other))) {
        return value !== value && other !== other;
      }
      return baseIsEqualDeep(value, other, baseIsEqual, customizer, isLoose, stackA, stackB);
    }

    /**
     * A specialized version of `baseIsEqual` for arrays and objects which performs
     * deep comparisons and tracks traversed objects enabling objects with circular
     * references to be compared.
     *
     * @private
     * @param {Object} object The object to compare.
     * @param {Object} other The other object to compare.
     * @param {Function} equalFunc The function to determine equivalents of values.
     * @param {Function} [customizer] The function to customize comparing objects.
     * @param {boolean} [isLoose] Specify performing partial comparisons.
     * @param {Array} [stackA=[]] Tracks traversed `value` objects.
     * @param {Array} [stackB=[]] Tracks traversed `other` objects.
     * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
     */
    function baseIsEqualDeep(object, other, equalFunc, customizer, isLoose, stackA, stackB) {
      var objIsArr = isArray(object),
          othIsArr = isArray(other),
          objTag = arrayTag,
          othTag = arrayTag;

      if (!objIsArr) {
        objTag = objToString.call(object);
        if (objTag == argsTag) {
          objTag = objectTag;
        } else if (objTag != objectTag) {
          objIsArr = isTypedArray(object);
        }
      }
      if (!othIsArr) {
        othTag = objToString.call(other);
        if (othTag == argsTag) {
          othTag = objectTag;
        } else if (othTag != objectTag) {
          othIsArr = isTypedArray(other);
        }
      }
      var objIsObj = objTag == objectTag,
          othIsObj = othTag == objectTag,
          isSameTag = objTag == othTag;

      if (isSameTag && !(objIsArr || objIsObj)) {
        return equalByTag(object, other, objTag);
      }
      if (!isLoose) {
        var objIsWrapped = objIsObj && hasOwnProperty.call(object, '__wrapped__'),
            othIsWrapped = othIsObj && hasOwnProperty.call(other, '__wrapped__');

        if (objIsWrapped || othIsWrapped) {
          return equalFunc(objIsWrapped ? object.value() : object, othIsWrapped ? other.value() : other, customizer, isLoose, stackA, stackB);
        }
      }
      if (!isSameTag) {
        return false;
      }
      // Assume cyclic values are equal.
      // For more information on detecting circular references see https://es5.github.io/#JO.
      stackA || (stackA = []);
      stackB || (stackB = []);

      var length = stackA.length;
      while (length--) {
        if (stackA[length] == object) {
          return stackB[length] == other;
        }
      }
      // Add `object` and `other` to the stack of traversed objects.
      stackA.push(object);
      stackB.push(other);

      var result = (objIsArr ? equalArrays : equalObjects)(object, other, equalFunc, customizer, isLoose, stackA, stackB);

      stackA.pop();
      stackB.pop();

      return result;
    }

    /**
     * The base implementation of `_.isMatch` without support for callback
     * shorthands and `this` binding.
     *
     * @private
     * @param {Object} object The object to inspect.
     * @param {Array} matchData The propery names, values, and compare flags to match.
     * @param {Function} [customizer] The function to customize comparing objects.
     * @returns {boolean} Returns `true` if `object` is a match, else `false`.
     */
    function baseIsMatch(object, matchData, customizer) {
      var index = matchData.length,
          length = index,
          noCustomizer = !customizer;

      if (object == null) {
        return !length;
      }
      object = toObject(object);
      while (index--) {
        var data = matchData[index];
        if ((noCustomizer && data[2])
              ? data[1] !== object[data[0]]
              : !(data[0] in object)
            ) {
          return false;
        }
      }
      while (++index < length) {
        data = matchData[index];
        var key = data[0],
            objValue = object[key],
            srcValue = data[1];

        if (noCustomizer && data[2]) {
          if (objValue === undefined && !(key in object)) {
            return false;
          }
        } else {
          var result = customizer ? customizer(objValue, srcValue, key) : undefined;
          if (!(result === undefined ? baseIsEqual(srcValue, objValue, customizer, true) : result)) {
            return false;
          }
        }
      }
      return true;
    }

    /**
     * The base implementation of `_.map` without support for callback shorthands
     * and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {Array} Returns the new mapped array.
     */
    function baseMap(collection, iteratee) {
      var index = -1,
          result = isArrayLike(collection) ? Array(collection.length) : [];

      baseEach(collection, function(value, key, collection) {
        result[++index] = iteratee(value, key, collection);
      });
      return result;
    }

    /**
     * The base implementation of `_.matches` which does not clone `source`.
     *
     * @private
     * @param {Object} source The object of property values to match.
     * @returns {Function} Returns the new function.
     */
    function baseMatches(source) {
      var matchData = getMatchData(source);
      if (matchData.length == 1 && matchData[0][2]) {
        var key = matchData[0][0],
            value = matchData[0][1];

        return function(object) {
          if (object == null) {
            return false;
          }
          return object[key] === value && (value !== undefined || (key in toObject(object)));
        };
      }
      return function(object) {
        return baseIsMatch(object, matchData);
      };
    }

    /**
     * The base implementation of `_.matchesProperty` which does not clone `srcValue`.
     *
     * @private
     * @param {string} path The path of the property to get.
     * @param {*} srcValue The value to compare.
     * @returns {Function} Returns the new function.
     */
    function baseMatchesProperty(path, srcValue) {
      var isArr = isArray(path),
          isCommon = isKey(path) && isStrictComparable(srcValue),
          pathKey = (path + '');

      path = toPath(path);
      return function(object) {
        if (object == null) {
          return false;
        }
        var key = pathKey;
        object = toObject(object);
        if ((isArr || !isCommon) && !(key in object)) {
          object = path.length == 1 ? object : baseGet(object, baseSlice(path, 0, -1));
          if (object == null) {
            return false;
          }
          key = last(path);
          object = toObject(object);
        }
        return object[key] === srcValue
          ? (srcValue !== undefined || (key in object))
          : baseIsEqual(srcValue, object[key], undefined, true);
      };
    }

    /**
     * The base implementation of `_.merge` without support for argument juggling,
     * multiple sources, and `this` binding `customizer` functions.
     *
     * @private
     * @param {Object} object The destination object.
     * @param {Object} source The source object.
     * @param {Function} [customizer] The function to customize merged values.
     * @param {Array} [stackA=[]] Tracks traversed source objects.
     * @param {Array} [stackB=[]] Associates values with source counterparts.
     * @returns {Object} Returns `object`.
     */
    function baseMerge(object, source, customizer, stackA, stackB) {
      if (!isObject(object)) {
        return object;
      }
      var isSrcArr = isArrayLike(source) && (isArray(source) || isTypedArray(source)),
          props = isSrcArr ? undefined : keys(source);

      arrayEach(props || source, function(srcValue, key) {
        if (props) {
          key = srcValue;
          srcValue = source[key];
        }
        if (isObjectLike(srcValue)) {
          stackA || (stackA = []);
          stackB || (stackB = []);
          baseMergeDeep(object, source, key, baseMerge, customizer, stackA, stackB);
        }
        else {
          var value = object[key],
              result = customizer ? customizer(value, srcValue, key, object, source) : undefined,
              isCommon = result === undefined;

          if (isCommon) {
            result = srcValue;
          }
          if ((result !== undefined || (isSrcArr && !(key in object))) &&
              (isCommon || (result === result ? (result !== value) : (value === value)))) {
            object[key] = result;
          }
        }
      });
      return object;
    }

    /**
     * A specialized version of `baseMerge` for arrays and objects which performs
     * deep merges and tracks traversed objects enabling objects with circular
     * references to be merged.
     *
     * @private
     * @param {Object} object The destination object.
     * @param {Object} source The source object.
     * @param {string} key The key of the value to merge.
     * @param {Function} mergeFunc The function to merge values.
     * @param {Function} [customizer] The function to customize merged values.
     * @param {Array} [stackA=[]] Tracks traversed source objects.
     * @param {Array} [stackB=[]] Associates values with source counterparts.
     * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
     */
    function baseMergeDeep(object, source, key, mergeFunc, customizer, stackA, stackB) {
      var length = stackA.length,
          srcValue = source[key];

      while (length--) {
        if (stackA[length] == srcValue) {
          object[key] = stackB[length];
          return;
        }
      }
      var value = object[key],
          result = customizer ? customizer(value, srcValue, key, object, source) : undefined,
          isCommon = result === undefined;

      if (isCommon) {
        result = srcValue;
        if (isArrayLike(srcValue) && (isArray(srcValue) || isTypedArray(srcValue))) {
          result = isArray(value)
            ? value
            : (isArrayLike(value) ? arrayCopy(value) : []);
        }
        else if (isPlainObject(srcValue) || isArguments(srcValue)) {
          result = isArguments(value)
            ? toPlainObject(value)
            : (isPlainObject(value) ? value : {});
        }
        else {
          isCommon = false;
        }
      }
      // Add the source value to the stack of traversed objects and associate
      // it with its merged value.
      stackA.push(srcValue);
      stackB.push(result);

      if (isCommon) {
        // Recursively merge objects and arrays (susceptible to call stack limits).
        object[key] = mergeFunc(result, srcValue, customizer, stackA, stackB);
      } else if (result === result ? (result !== value) : (value === value)) {
        object[key] = result;
      }
    }

    /**
     * The base implementation of `_.property` without support for deep paths.
     *
     * @private
     * @param {string} key The key of the property to get.
     * @returns {Function} Returns the new function.
     */
    function baseProperty(key) {
      return function(object) {
        return object == null ? undefined : object[key];
      };
    }

    /**
     * A specialized version of `baseProperty` which supports deep paths.
     *
     * @private
     * @param {Array|string} path The path of the property to get.
     * @returns {Function} Returns the new function.
     */
    function basePropertyDeep(path) {
      var pathKey = (path + '');
      path = toPath(path);
      return function(object) {
        return baseGet(object, path, pathKey);
      };
    }

    /**
     * The base implementation of `_.pullAt` without support for individual
     * index arguments and capturing the removed elements.
     *
     * @private
     * @param {Array} array The array to modify.
     * @param {number[]} indexes The indexes of elements to remove.
     * @returns {Array} Returns `array`.
     */
    function basePullAt(array, indexes) {
      var length = array ? indexes.length : 0;
      while (length--) {
        var index = indexes[length];
        if (index != previous && isIndex(index)) {
          var previous = index;
          splice.call(array, index, 1);
        }
      }
      return array;
    }

    /**
     * The base implementation of `_.random` without support for argument juggling
     * and returning floating-point numbers.
     *
     * @private
     * @param {number} min The minimum possible value.
     * @param {number} max The maximum possible value.
     * @returns {number} Returns the random number.
     */
    function baseRandom(min, max) {
      return min + nativeFloor(nativeRandom() * (max - min + 1));
    }

    /**
     * The base implementation of `_.reduce` and `_.reduceRight` without support
     * for callback shorthands and `this` binding, which iterates over `collection`
     * using the provided `eachFunc`.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {*} accumulator The initial value.
     * @param {boolean} initFromCollection Specify using the first or last element
     *  of `collection` as the initial value.
     * @param {Function} eachFunc The function to iterate over `collection`.
     * @returns {*} Returns the accumulated value.
     */
    function baseReduce(collection, iteratee, accumulator, initFromCollection, eachFunc) {
      eachFunc(collection, function(value, index, collection) {
        accumulator = initFromCollection
          ? (initFromCollection = false, value)
          : iteratee(accumulator, value, index, collection);
      });
      return accumulator;
    }

    /**
     * The base implementation of `setData` without support for hot loop detection.
     *
     * @private
     * @param {Function} func The function to associate metadata with.
     * @param {*} data The metadata.
     * @returns {Function} Returns `func`.
     */
    var baseSetData = !metaMap ? identity : function(func, data) {
      metaMap.set(func, data);
      return func;
    };

    /**
     * The base implementation of `_.slice` without an iteratee call guard.
     *
     * @private
     * @param {Array} array The array to slice.
     * @param {number} [start=0] The start position.
     * @param {number} [end=array.length] The end position.
     * @returns {Array} Returns the slice of `array`.
     */
    function baseSlice(array, start, end) {
      var index = -1,
          length = array.length;

      start = start == null ? 0 : (+start || 0);
      if (start < 0) {
        start = -start > length ? 0 : (length + start);
      }
      end = (end === undefined || end > length) ? length : (+end || 0);
      if (end < 0) {
        end += length;
      }
      length = start > end ? 0 : ((end - start) >>> 0);
      start >>>= 0;

      var result = Array(length);
      while (++index < length) {
        result[index] = array[index + start];
      }
      return result;
    }

    /**
     * The base implementation of `_.some` without support for callback shorthands
     * and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {boolean} Returns `true` if any element passes the predicate check,
     *  else `false`.
     */
    function baseSome(collection, predicate) {
      var result;

      baseEach(collection, function(value, index, collection) {
        result = predicate(value, index, collection);
        return !result;
      });
      return !!result;
    }

    /**
     * The base implementation of `_.sortBy` which uses `comparer` to define
     * the sort order of `array` and replaces criteria objects with their
     * corresponding values.
     *
     * @private
     * @param {Array} array The array to sort.
     * @param {Function} comparer The function to define sort order.
     * @returns {Array} Returns `array`.
     */
    function baseSortBy(array, comparer) {
      var length = array.length;

      array.sort(comparer);
      while (length--) {
        array[length] = array[length].value;
      }
      return array;
    }

    /**
     * The base implementation of `_.sortByOrder` without param guards.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function[]|Object[]|string[]} iteratees The iteratees to sort by.
     * @param {boolean[]} orders The sort orders of `iteratees`.
     * @returns {Array} Returns the new sorted array.
     */
    function baseSortByOrder(collection, iteratees, orders) {
      var callback = getCallback(),
          index = -1;

      iteratees = arrayMap(iteratees, function(iteratee) { return callback(iteratee); });

      var result = baseMap(collection, function(value) {
        var criteria = arrayMap(iteratees, function(iteratee) { return iteratee(value); });
        return { 'criteria': criteria, 'index': ++index, 'value': value };
      });

      return baseSortBy(result, function(object, other) {
        return compareMultiple(object, other, orders);
      });
    }

    /**
     * The base implementation of `_.sum` without support for callback shorthands
     * and `this` binding.
     *
     * @private
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} iteratee The function invoked per iteration.
     * @returns {number} Returns the sum.
     */
    function baseSum(collection, iteratee) {
      var result = 0;
      baseEach(collection, function(value, index, collection) {
        result += +iteratee(value, index, collection) || 0;
      });
      return result;
    }

    /**
     * The base implementation of `_.uniq` without support for callback shorthands
     * and `this` binding.
     *
     * @private
     * @param {Array} array The array to inspect.
     * @param {Function} [iteratee] The function invoked per iteration.
     * @returns {Array} Returns the new duplicate-value-free array.
     */
    function baseUniq(array, iteratee) {
      var index = -1,
          indexOf = getIndexOf(),
          length = array.length,
          isCommon = indexOf == baseIndexOf,
          isLarge = isCommon && length >= LARGE_ARRAY_SIZE,
          seen = isLarge ? createCache() : null,
          result = [];

      if (seen) {
        indexOf = cacheIndexOf;
        isCommon = false;
      } else {
        isLarge = false;
        seen = iteratee ? [] : result;
      }
      outer:
      while (++index < length) {
        var value = array[index],
            computed = iteratee ? iteratee(value, index, array) : value;

        if (isCommon && value === value) {
          var seenIndex = seen.length;
          while (seenIndex--) {
            if (seen[seenIndex] === computed) {
              continue outer;
            }
          }
          if (iteratee) {
            seen.push(computed);
          }
          result.push(value);
        }
        else if (indexOf(seen, computed, 0) < 0) {
          if (iteratee || isLarge) {
            seen.push(computed);
          }
          result.push(value);
        }
      }
      return result;
    }

    /**
     * The base implementation of `_.values` and `_.valuesIn` which creates an
     * array of `object` property values corresponding to the property names
     * of `props`.
     *
     * @private
     * @param {Object} object The object to query.
     * @param {Array} props The property names to get values for.
     * @returns {Object} Returns the array of property values.
     */
    function baseValues(object, props) {
      var index = -1,
          length = props.length,
          result = Array(length);

      while (++index < length) {
        result[index] = object[props[index]];
      }
      return result;
    }

    /**
     * The base implementation of `_.dropRightWhile`, `_.dropWhile`, `_.takeRightWhile`,
     * and `_.takeWhile` without support for callback shorthands and `this` binding.
     *
     * @private
     * @param {Array} array The array to query.
     * @param {Function} predicate The function invoked per iteration.
     * @param {boolean} [isDrop] Specify dropping elements instead of taking them.
     * @param {boolean} [fromRight] Specify iterating from right to left.
     * @returns {Array} Returns the slice of `array`.
     */
    function baseWhile(array, predicate, isDrop, fromRight) {
      var length = array.length,
          index = fromRight ? length : -1;

      while ((fromRight ? index-- : ++index < length) && predicate(array[index], index, array)) {}
      return isDrop
        ? baseSlice(array, (fromRight ? 0 : index), (fromRight ? index + 1 : length))
        : baseSlice(array, (fromRight ? index + 1 : 0), (fromRight ? length : index));
    }

    /**
     * The base implementation of `wrapperValue` which returns the result of
     * performing a sequence of actions on the unwrapped `value`, where each
     * successive action is supplied the return value of the previous.
     *
     * @private
     * @param {*} value The unwrapped value.
     * @param {Array} actions Actions to peform to resolve the unwrapped value.
     * @returns {*} Returns the resolved value.
     */
    function baseWrapperValue(value, actions) {
      var result = value;
      if (result instanceof LazyWrapper) {
        result = result.value();
      }
      var index = -1,
          length = actions.length;

      while (++index < length) {
        var action = actions[index];
        result = action.func.apply(action.thisArg, arrayPush([result], action.args));
      }
      return result;
    }

    /**
     * Performs a binary search of `array` to determine the index at which `value`
     * should be inserted into `array` in order to maintain its sort order.
     *
     * @private
     * @param {Array} array The sorted array to inspect.
     * @param {*} value The value to evaluate.
     * @param {boolean} [retHighest] Specify returning the highest qualified index.
     * @returns {number} Returns the index at which `value` should be inserted
     *  into `array`.
     */
    function binaryIndex(array, value, retHighest) {
      var low = 0,
          high = array ? array.length : low;

      if (typeof value == 'number' && value === value && high <= HALF_MAX_ARRAY_LENGTH) {
        while (low < high) {
          var mid = (low + high) >>> 1,
              computed = array[mid];

          if ((retHighest ? (computed <= value) : (computed < value)) && computed !== null) {
            low = mid + 1;
          } else {
            high = mid;
          }
        }
        return high;
      }
      return binaryIndexBy(array, value, identity, retHighest);
    }

    /**
     * This function is like `binaryIndex` except that it invokes `iteratee` for
     * `value` and each element of `array` to compute their sort ranking. The
     * iteratee is invoked with one argument; (value).
     *
     * @private
     * @param {Array} array The sorted array to inspect.
     * @param {*} value The value to evaluate.
     * @param {Function} iteratee The function invoked per iteration.
     * @param {boolean} [retHighest] Specify returning the highest qualified index.
     * @returns {number} Returns the index at which `value` should be inserted
     *  into `array`.
     */
    function binaryIndexBy(array, value, iteratee, retHighest) {
      value = iteratee(value);

      var low = 0,
          high = array ? array.length : 0,
          valIsNaN = value !== value,
          valIsNull = value === null,
          valIsUndef = value === undefined;

      while (low < high) {
        var mid = nativeFloor((low + high) / 2),
            computed = iteratee(array[mid]),
            isDef = computed !== undefined,
            isReflexive = computed === computed;

        if (valIsNaN) {
          var setLow = isReflexive || retHighest;
        } else if (valIsNull) {
          setLow = isReflexive && isDef && (retHighest || computed != null);
        } else if (valIsUndef) {
          setLow = isReflexive && (retHighest || isDef);
        } else if (computed == null) {
          setLow = false;
        } else {
          setLow = retHighest ? (computed <= value) : (computed < value);
        }
        if (setLow) {
          low = mid + 1;
        } else {
          high = mid;
        }
      }
      return nativeMin(high, MAX_ARRAY_INDEX);
    }

    /**
     * A specialized version of `baseCallback` which only supports `this` binding
     * and specifying the number of arguments to provide to `func`.
     *
     * @private
     * @param {Function} func The function to bind.
     * @param {*} thisArg The `this` binding of `func`.
     * @param {number} [argCount] The number of arguments to provide to `func`.
     * @returns {Function} Returns the callback.
     */
    function bindCallback(func, thisArg, argCount) {
      if (typeof func != 'function') {
        return identity;
      }
      if (thisArg === undefined) {
        return func;
      }
      switch (argCount) {
        case 1: return function(value) {
          return func.call(thisArg, value);
        };
        case 3: return function(value, index, collection) {
          return func.call(thisArg, value, index, collection);
        };
        case 4: return function(accumulator, value, index, collection) {
          return func.call(thisArg, accumulator, value, index, collection);
        };
        case 5: return function(value, other, key, object, source) {
          return func.call(thisArg, value, other, key, object, source);
        };
      }
      return function() {
        return func.apply(thisArg, arguments);
      };
    }

    /**
     * Creates a clone of the given array buffer.
     *
     * @private
     * @param {ArrayBuffer} buffer The array buffer to clone.
     * @returns {ArrayBuffer} Returns the cloned array buffer.
     */
    function bufferClone(buffer) {
      var result = new ArrayBuffer(buffer.byteLength),
          view = new Uint8Array(result);

      view.set(new Uint8Array(buffer));
      return result;
    }

    /**
     * Creates an array that is the composition of partially applied arguments,
     * placeholders, and provided arguments into a single array of arguments.
     *
     * @private
     * @param {Array|Object} args The provided arguments.
     * @param {Array} partials The arguments to prepend to those provided.
     * @param {Array} holders The `partials` placeholder indexes.
     * @returns {Array} Returns the new array of composed arguments.
     */
    function composeArgs(args, partials, holders) {
      var holdersLength = holders.length,
          argsIndex = -1,
          argsLength = nativeMax(args.length - holdersLength, 0),
          leftIndex = -1,
          leftLength = partials.length,
          result = Array(leftLength + argsLength);

      while (++leftIndex < leftLength) {
        result[leftIndex] = partials[leftIndex];
      }
      while (++argsIndex < holdersLength) {
        result[holders[argsIndex]] = args[argsIndex];
      }
      while (argsLength--) {
        result[leftIndex++] = args[argsIndex++];
      }
      return result;
    }

    /**
     * This function is like `composeArgs` except that the arguments composition
     * is tailored for `_.partialRight`.
     *
     * @private
     * @param {Array|Object} args The provided arguments.
     * @param {Array} partials The arguments to append to those provided.
     * @param {Array} holders The `partials` placeholder indexes.
     * @returns {Array} Returns the new array of composed arguments.
     */
    function composeArgsRight(args, partials, holders) {
      var holdersIndex = -1,
          holdersLength = holders.length,
          argsIndex = -1,
          argsLength = nativeMax(args.length - holdersLength, 0),
          rightIndex = -1,
          rightLength = partials.length,
          result = Array(argsLength + rightLength);

      while (++argsIndex < argsLength) {
        result[argsIndex] = args[argsIndex];
      }
      var offset = argsIndex;
      while (++rightIndex < rightLength) {
        result[offset + rightIndex] = partials[rightIndex];
      }
      while (++holdersIndex < holdersLength) {
        result[offset + holders[holdersIndex]] = args[argsIndex++];
      }
      return result;
    }

    /**
     * Creates a `_.countBy`, `_.groupBy`, `_.indexBy`, or `_.partition` function.
     *
     * @private
     * @param {Function} setter The function to set keys and values of the accumulator object.
     * @param {Function} [initializer] The function to initialize the accumulator object.
     * @returns {Function} Returns the new aggregator function.
     */
    function createAggregator(setter, initializer) {
      return function(collection, iteratee, thisArg) {
        var result = initializer ? initializer() : {};
        iteratee = getCallback(iteratee, thisArg, 3);

        if (isArray(collection)) {
          var index = -1,
              length = collection.length;

          while (++index < length) {
            var value = collection[index];
            setter(result, value, iteratee(value, index, collection), collection);
          }
        } else {
          baseEach(collection, function(value, key, collection) {
            setter(result, value, iteratee(value, key, collection), collection);
          });
        }
        return result;
      };
    }

    /**
     * Creates a `_.assign`, `_.defaults`, or `_.merge` function.
     *
     * @private
     * @param {Function} assigner The function to assign values.
     * @returns {Function} Returns the new assigner function.
     */
    function createAssigner(assigner) {
      return restParam(function(object, sources) {
        var index = -1,
            length = object == null ? 0 : sources.length,
            customizer = length > 2 ? sources[length - 2] : undefined,
            guard = length > 2 ? sources[2] : undefined,
            thisArg = length > 1 ? sources[length - 1] : undefined;

        if (typeof customizer == 'function') {
          customizer = bindCallback(customizer, thisArg, 5);
          length -= 2;
        } else {
          customizer = typeof thisArg == 'function' ? thisArg : undefined;
          length -= (customizer ? 1 : 0);
        }
        if (guard && isIterateeCall(sources[0], sources[1], guard)) {
          customizer = length < 3 ? undefined : customizer;
          length = 1;
        }
        while (++index < length) {
          var source = sources[index];
          if (source) {
            assigner(object, source, customizer);
          }
        }
        return object;
      });
    }

    /**
     * Creates a `baseEach` or `baseEachRight` function.
     *
     * @private
     * @param {Function} eachFunc The function to iterate over a collection.
     * @param {boolean} [fromRight] Specify iterating from right to left.
     * @returns {Function} Returns the new base function.
     */
    function createBaseEach(eachFunc, fromRight) {
      return function(collection, iteratee) {
        var length = collection ? getLength(collection) : 0;
        if (!isLength(length)) {
          return eachFunc(collection, iteratee);
        }
        var index = fromRight ? length : -1,
            iterable = toObject(collection);

        while ((fromRight ? index-- : ++index < length)) {
          if (iteratee(iterable[index], index, iterable) === false) {
            break;
          }
        }
        return collection;
      };
    }

    /**
     * Creates a base function for `_.forIn` or `_.forInRight`.
     *
     * @private
     * @param {boolean} [fromRight] Specify iterating from right to left.
     * @returns {Function} Returns the new base function.
     */
    function createBaseFor(fromRight) {
      return function(object, iteratee, keysFunc) {
        var iterable = toObject(object),
            props = keysFunc(object),
            length = props.length,
            index = fromRight ? length : -1;

        while ((fromRight ? index-- : ++index < length)) {
          var key = props[index];
          if (iteratee(iterable[key], key, iterable) === false) {
            break;
          }
        }
        return object;
      };
    }

    /**
     * Creates a function that wraps `func` and invokes it with the `this`
     * binding of `thisArg`.
     *
     * @private
     * @param {Function} func The function to bind.
     * @param {*} [thisArg] The `this` binding of `func`.
     * @returns {Function} Returns the new bound function.
     */
    function createBindWrapper(func, thisArg) {
      var Ctor = createCtorWrapper(func);

      function wrapper() {
        var fn = (this && this !== root && this instanceof wrapper) ? Ctor : func;
        return fn.apply(thisArg, arguments);
      }
      return wrapper;
    }

    /**
     * Creates a `Set` cache object to optimize linear searches of large arrays.
     *
     * @private
     * @param {Array} [values] The values to cache.
     * @returns {null|Object} Returns the new cache object if `Set` is supported, else `null`.
     */
    function createCache(values) {
      return (nativeCreate && Set) ? new SetCache(values) : null;
    }

    /**
     * Creates a function that produces compound words out of the words in a
     * given string.
     *
     * @private
     * @param {Function} callback The function to combine each word.
     * @returns {Function} Returns the new compounder function.
     */
    function createCompounder(callback) {
      return function(string) {
        var index = -1,
            array = words(deburr(string)),
            length = array.length,
            result = '';

        while (++index < length) {
          result = callback(result, array[index], index);
        }
        return result;
      };
    }

    /**
     * Creates a function that produces an instance of `Ctor` regardless of
     * whether it was invoked as part of a `new` expression or by `call` or `apply`.
     *
     * @private
     * @param {Function} Ctor The constructor to wrap.
     * @returns {Function} Returns the new wrapped function.
     */
    function createCtorWrapper(Ctor) {
      return function() {
        // Use a `switch` statement to work with class constructors.
        // See http://ecma-international.org/ecma-262/6.0/#sec-ecmascript-function-objects-call-thisargument-argumentslist
        // for more details.
        var args = arguments;
        switch (args.length) {
          case 0: return new Ctor;
          case 1: return new Ctor(args[0]);
          case 2: return new Ctor(args[0], args[1]);
          case 3: return new Ctor(args[0], args[1], args[2]);
          case 4: return new Ctor(args[0], args[1], args[2], args[3]);
          case 5: return new Ctor(args[0], args[1], args[2], args[3], args[4]);
          case 6: return new Ctor(args[0], args[1], args[2], args[3], args[4], args[5]);
          case 7: return new Ctor(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
        }
        var thisBinding = baseCreate(Ctor.prototype),
            result = Ctor.apply(thisBinding, args);

        // Mimic the constructor's `return` behavior.
        // See https://es5.github.io/#x13.2.2 for more details.
        return isObject(result) ? result : thisBinding;
      };
    }

    /**
     * Creates a `_.curry` or `_.curryRight` function.
     *
     * @private
     * @param {boolean} flag The curry bit flag.
     * @returns {Function} Returns the new curry function.
     */
    function createCurry(flag) {
      function curryFunc(func, arity, guard) {
        if (guard && isIterateeCall(func, arity, guard)) {
          arity = undefined;
        }
        var result = createWrapper(func, flag, undefined, undefined, undefined, undefined, undefined, arity);
        result.placeholder = curryFunc.placeholder;
        return result;
      }
      return curryFunc;
    }

    /**
     * Creates a `_.defaults` or `_.defaultsDeep` function.
     *
     * @private
     * @param {Function} assigner The function to assign values.
     * @param {Function} customizer The function to customize assigned values.
     * @returns {Function} Returns the new defaults function.
     */
    function createDefaults(assigner, customizer) {
      return restParam(function(args) {
        var object = args[0];
        if (object == null) {
          return object;
        }
        args.push(customizer);
        return assigner.apply(undefined, args);
      });
    }

    /**
     * Creates a `_.max` or `_.min` function.
     *
     * @private
     * @param {Function} comparator The function used to compare values.
     * @param {*} exValue The initial extremum value.
     * @returns {Function} Returns the new extremum function.
     */
    function createExtremum(comparator, exValue) {
      return function(collection, iteratee, thisArg) {
        if (thisArg && isIterateeCall(collection, iteratee, thisArg)) {
          iteratee = undefined;
        }
        iteratee = getCallback(iteratee, thisArg, 3);
        if (iteratee.length == 1) {
          collection = isArray(collection) ? collection : toIterable(collection);
          var result = arrayExtremum(collection, iteratee, comparator, exValue);
          if (!(collection.length && result === exValue)) {
            return result;
          }
        }
        return baseExtremum(collection, iteratee, comparator, exValue);
      };
    }

    /**
     * Creates a `_.find` or `_.findLast` function.
     *
     * @private
     * @param {Function} eachFunc The function to iterate over a collection.
     * @param {boolean} [fromRight] Specify iterating from right to left.
     * @returns {Function} Returns the new find function.
     */
    function createFind(eachFunc, fromRight) {
      return function(collection, predicate, thisArg) {
        predicate = getCallback(predicate, thisArg, 3);
        if (isArray(collection)) {
          var index = baseFindIndex(collection, predicate, fromRight);
          return index > -1 ? collection[index] : undefined;
        }
        return baseFind(collection, predicate, eachFunc);
      };
    }

    /**
     * Creates a `_.findIndex` or `_.findLastIndex` function.
     *
     * @private
     * @param {boolean} [fromRight] Specify iterating from right to left.
     * @returns {Function} Returns the new find function.
     */
    function createFindIndex(fromRight) {
      return function(array, predicate, thisArg) {
        if (!(array && array.length)) {
          return -1;
        }
        predicate = getCallback(predicate, thisArg, 3);
        return baseFindIndex(array, predicate, fromRight);
      };
    }

    /**
     * Creates a `_.findKey` or `_.findLastKey` function.
     *
     * @private
     * @param {Function} objectFunc The function to iterate over an object.
     * @returns {Function} Returns the new find function.
     */
    function createFindKey(objectFunc) {
      return function(object, predicate, thisArg) {
        predicate = getCallback(predicate, thisArg, 3);
        return baseFind(object, predicate, objectFunc, true);
      };
    }

    /**
     * Creates a `_.flow` or `_.flowRight` function.
     *
     * @private
     * @param {boolean} [fromRight] Specify iterating from right to left.
     * @returns {Function} Returns the new flow function.
     */
    function createFlow(fromRight) {
      return function() {
        var wrapper,
            length = arguments.length,
            index = fromRight ? length : -1,
            leftIndex = 0,
            funcs = Array(length);

        while ((fromRight ? index-- : ++index < length)) {
          var func = funcs[leftIndex++] = arguments[index];
          if (typeof func != 'function') {
            throw new TypeError(FUNC_ERROR_TEXT);
          }
          if (!wrapper && LodashWrapper.prototype.thru && getFuncName(func) == 'wrapper') {
            wrapper = new LodashWrapper([], true);
          }
        }
        index = wrapper ? -1 : length;
        while (++index < length) {
          func = funcs[index];

          var funcName = getFuncName(func),
              data = funcName == 'wrapper' ? getData(func) : undefined;

          if (data && isLaziable(data[0]) && data[1] == (ARY_FLAG | CURRY_FLAG | PARTIAL_FLAG | REARG_FLAG) && !data[4].length && data[9] == 1) {
            wrapper = wrapper[getFuncName(data[0])].apply(wrapper, data[3]);
          } else {
            wrapper = (func.length == 1 && isLaziable(func)) ? wrapper[funcName]() : wrapper.thru(func);
          }
        }
        return function() {
          var args = arguments,
              value = args[0];

          if (wrapper && args.length == 1 && isArray(value) && value.length >= LARGE_ARRAY_SIZE) {
            return wrapper.plant(value).value();
          }
          var index = 0,
              result = length ? funcs[index].apply(this, args) : value;

          while (++index < length) {
            result = funcs[index].call(this, result);
          }
          return result;
        };
      };
    }

    /**
     * Creates a function for `_.forEach` or `_.forEachRight`.
     *
     * @private
     * @param {Function} arrayFunc The function to iterate over an array.
     * @param {Function} eachFunc The function to iterate over a collection.
     * @returns {Function} Returns the new each function.
     */
    function createForEach(arrayFunc, eachFunc) {
      return function(collection, iteratee, thisArg) {
        return (typeof iteratee == 'function' && thisArg === undefined && isArray(collection))
          ? arrayFunc(collection, iteratee)
          : eachFunc(collection, bindCallback(iteratee, thisArg, 3));
      };
    }

    /**
     * Creates a function for `_.forIn` or `_.forInRight`.
     *
     * @private
     * @param {Function} objectFunc The function to iterate over an object.
     * @returns {Function} Returns the new each function.
     */
    function createForIn(objectFunc) {
      return function(object, iteratee, thisArg) {
        if (typeof iteratee != 'function' || thisArg !== undefined) {
          iteratee = bindCallback(iteratee, thisArg, 3);
        }
        return objectFunc(object, iteratee, keysIn);
      };
    }

    /**
     * Creates a function for `_.forOwn` or `_.forOwnRight`.
     *
     * @private
     * @param {Function} objectFunc The function to iterate over an object.
     * @returns {Function} Returns the new each function.
     */
    function createForOwn(objectFunc) {
      return function(object, iteratee, thisArg) {
        if (typeof iteratee != 'function' || thisArg !== undefined) {
          iteratee = bindCallback(iteratee, thisArg, 3);
        }
        return objectFunc(object, iteratee);
      };
    }

    /**
     * Creates a function for `_.mapKeys` or `_.mapValues`.
     *
     * @private
     * @param {boolean} [isMapKeys] Specify mapping keys instead of values.
     * @returns {Function} Returns the new map function.
     */
    function createObjectMapper(isMapKeys) {
      return function(object, iteratee, thisArg) {
        var result = {};
        iteratee = getCallback(iteratee, thisArg, 3);

        baseForOwn(object, function(value, key, object) {
          var mapped = iteratee(value, key, object);
          key = isMapKeys ? mapped : key;
          value = isMapKeys ? value : mapped;
          result[key] = value;
        });
        return result;
      };
    }

    /**
     * Creates a function for `_.padLeft` or `_.padRight`.
     *
     * @private
     * @param {boolean} [fromRight] Specify padding from the right.
     * @returns {Function} Returns the new pad function.
     */
    function createPadDir(fromRight) {
      return function(string, length, chars) {
        string = baseToString(string);
        return (fromRight ? string : '') + createPadding(string, length, chars) + (fromRight ? '' : string);
      };
    }

    /**
     * Creates a `_.partial` or `_.partialRight` function.
     *
     * @private
     * @param {boolean} flag The partial bit flag.
     * @returns {Function} Returns the new partial function.
     */
    function createPartial(flag) {
      var partialFunc = restParam(function(func, partials) {
        var holders = replaceHolders(partials, partialFunc.placeholder);
        return createWrapper(func, flag, undefined, partials, holders);
      });
      return partialFunc;
    }

    /**
     * Creates a function for `_.reduce` or `_.reduceRight`.
     *
     * @private
     * @param {Function} arrayFunc The function to iterate over an array.
     * @param {Function} eachFunc The function to iterate over a collection.
     * @returns {Function} Returns the new each function.
     */
    function createReduce(arrayFunc, eachFunc) {
      return function(collection, iteratee, accumulator, thisArg) {
        var initFromArray = arguments.length < 3;
        return (typeof iteratee == 'function' && thisArg === undefined && isArray(collection))
          ? arrayFunc(collection, iteratee, accumulator, initFromArray)
          : baseReduce(collection, getCallback(iteratee, thisArg, 4), accumulator, initFromArray, eachFunc);
      };
    }

    /**
     * Creates a function that wraps `func` and invokes it with optional `this`
     * binding of, partial application, and currying.
     *
     * @private
     * @param {Function|string} func The function or method name to reference.
     * @param {number} bitmask The bitmask of flags. See `createWrapper` for more details.
     * @param {*} [thisArg] The `this` binding of `func`.
     * @param {Array} [partials] The arguments to prepend to those provided to the new function.
     * @param {Array} [holders] The `partials` placeholder indexes.
     * @param {Array} [partialsRight] The arguments to append to those provided to the new function.
     * @param {Array} [holdersRight] The `partialsRight` placeholder indexes.
     * @param {Array} [argPos] The argument positions of the new function.
     * @param {number} [ary] The arity cap of `func`.
     * @param {number} [arity] The arity of `func`.
     * @returns {Function} Returns the new wrapped function.
     */
    function createHybridWrapper(func, bitmask, thisArg, partials, holders, partialsRight, holdersRight, argPos, ary, arity) {
      var isAry = bitmask & ARY_FLAG,
          isBind = bitmask & BIND_FLAG,
          isBindKey = bitmask & BIND_KEY_FLAG,
          isCurry = bitmask & CURRY_FLAG,
          isCurryBound = bitmask & CURRY_BOUND_FLAG,
          isCurryRight = bitmask & CURRY_RIGHT_FLAG,
          Ctor = isBindKey ? undefined : createCtorWrapper(func);

      function wrapper() {
        // Avoid `arguments` object use disqualifying optimizations by
        // converting it to an array before providing it to other functions.
        var length = arguments.length,
            index = length,
            args = Array(length);

        while (index--) {
          args[index] = arguments[index];
        }
        if (partials) {
          args = composeArgs(args, partials, holders);
        }
        if (partialsRight) {
          args = composeArgsRight(args, partialsRight, holdersRight);
        }
        if (isCurry || isCurryRight) {
          var placeholder = wrapper.placeholder,
              argsHolders = replaceHolders(args, placeholder);

          length -= argsHolders.length;
          if (length < arity) {
            var newArgPos = argPos ? arrayCopy(argPos) : undefined,
                newArity = nativeMax(arity - length, 0),
                newsHolders = isCurry ? argsHolders : undefined,
                newHoldersRight = isCurry ? undefined : argsHolders,
                newPartials = isCurry ? args : undefined,
                newPartialsRight = isCurry ? undefined : args;

            bitmask |= (isCurry ? PARTIAL_FLAG : PARTIAL_RIGHT_FLAG);
            bitmask &= ~(isCurry ? PARTIAL_RIGHT_FLAG : PARTIAL_FLAG);

            if (!isCurryBound) {
              bitmask &= ~(BIND_FLAG | BIND_KEY_FLAG);
            }
            var newData = [func, bitmask, thisArg, newPartials, newsHolders, newPartialsRight, newHoldersRight, newArgPos, ary, newArity],
                result = createHybridWrapper.apply(undefined, newData);

            if (isLaziable(func)) {
              setData(result, newData);
            }
            result.placeholder = placeholder;
            return result;
          }
        }
        var thisBinding = isBind ? thisArg : this,
            fn = isBindKey ? thisBinding[func] : func;

        if (argPos) {
          args = reorder(args, argPos);
        }
        if (isAry && ary < args.length) {
          args.length = ary;
        }
        if (this && this !== root && this instanceof wrapper) {
          fn = Ctor || createCtorWrapper(func);
        }
        return fn.apply(thisBinding, args);
      }
      return wrapper;
    }

    /**
     * Creates the padding required for `string` based on the given `length`.
     * The `chars` string is truncated if the number of characters exceeds `length`.
     *
     * @private
     * @param {string} string The string to create padding for.
     * @param {number} [length=0] The padding length.
     * @param {string} [chars=' '] The string used as padding.
     * @returns {string} Returns the pad for `string`.
     */
    function createPadding(string, length, chars) {
      var strLength = string.length;
      length = +length;

      if (strLength >= length || !nativeIsFinite(length)) {
        return '';
      }
      var padLength = length - strLength;
      chars = chars == null ? ' ' : (chars + '');
      return repeat(chars, nativeCeil(padLength / chars.length)).slice(0, padLength);
    }

    /**
     * Creates a function that wraps `func` and invokes it with the optional `this`
     * binding of `thisArg` and the `partials` prepended to those provided to
     * the wrapper.
     *
     * @private
     * @param {Function} func The function to partially apply arguments to.
     * @param {number} bitmask The bitmask of flags. See `createWrapper` for more details.
     * @param {*} thisArg The `this` binding of `func`.
     * @param {Array} partials The arguments to prepend to those provided to the new function.
     * @returns {Function} Returns the new bound function.
     */
    function createPartialWrapper(func, bitmask, thisArg, partials) {
      var isBind = bitmask & BIND_FLAG,
          Ctor = createCtorWrapper(func);

      function wrapper() {
        // Avoid `arguments` object use disqualifying optimizations by
        // converting it to an array before providing it `func`.
        var argsIndex = -1,
            argsLength = arguments.length,
            leftIndex = -1,
            leftLength = partials.length,
            args = Array(leftLength + argsLength);

        while (++leftIndex < leftLength) {
          args[leftIndex] = partials[leftIndex];
        }
        while (argsLength--) {
          args[leftIndex++] = arguments[++argsIndex];
        }
        var fn = (this && this !== root && this instanceof wrapper) ? Ctor : func;
        return fn.apply(isBind ? thisArg : this, args);
      }
      return wrapper;
    }

    /**
     * Creates a `_.ceil`, `_.floor`, or `_.round` function.
     *
     * @private
     * @param {string} methodName The name of the `Math` method to use when rounding.
     * @returns {Function} Returns the new round function.
     */
    function createRound(methodName) {
      var func = Math[methodName];
      return function(number, precision) {
        precision = precision === undefined ? 0 : (+precision || 0);
        if (precision) {
          precision = pow(10, precision);
          return func(number * precision) / precision;
        }
        return func(number);
      };
    }

    /**
     * Creates a `_.sortedIndex` or `_.sortedLastIndex` function.
     *
     * @private
     * @param {boolean} [retHighest] Specify returning the highest qualified index.
     * @returns {Function} Returns the new index function.
     */
    function createSortedIndex(retHighest) {
      return function(array, value, iteratee, thisArg) {
        var callback = getCallback(iteratee);
        return (iteratee == null && callback === baseCallback)
          ? binaryIndex(array, value, retHighest)
          : binaryIndexBy(array, value, callback(iteratee, thisArg, 1), retHighest);
      };
    }

    /**
     * Creates a function that either curries or invokes `func` with optional
     * `this` binding and partially applied arguments.
     *
     * @private
     * @param {Function|string} func The function or method name to reference.
     * @param {number} bitmask The bitmask of flags.
     *  The bitmask may be composed of the following flags:
     *     1 - `_.bind`
     *     2 - `_.bindKey`
     *     4 - `_.curry` or `_.curryRight` of a bound function
     *     8 - `_.curry`
     *    16 - `_.curryRight`
     *    32 - `_.partial`
     *    64 - `_.partialRight`
     *   128 - `_.rearg`
     *   256 - `_.ary`
     * @param {*} [thisArg] The `this` binding of `func`.
     * @param {Array} [partials] The arguments to be partially applied.
     * @param {Array} [holders] The `partials` placeholder indexes.
     * @param {Array} [argPos] The argument positions of the new function.
     * @param {number} [ary] The arity cap of `func`.
     * @param {number} [arity] The arity of `func`.
     * @returns {Function} Returns the new wrapped function.
     */
    function createWrapper(func, bitmask, thisArg, partials, holders, argPos, ary, arity) {
      var isBindKey = bitmask & BIND_KEY_FLAG;
      if (!isBindKey && typeof func != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      var length = partials ? partials.length : 0;
      if (!length) {
        bitmask &= ~(PARTIAL_FLAG | PARTIAL_RIGHT_FLAG);
        partials = holders = undefined;
      }
      length -= (holders ? holders.length : 0);
      if (bitmask & PARTIAL_RIGHT_FLAG) {
        var partialsRight = partials,
            holdersRight = holders;

        partials = holders = undefined;
      }
      var data = isBindKey ? undefined : getData(func),
          newData = [func, bitmask, thisArg, partials, holders, partialsRight, holdersRight, argPos, ary, arity];

      if (data) {
        mergeData(newData, data);
        bitmask = newData[1];
        arity = newData[9];
      }
      newData[9] = arity == null
        ? (isBindKey ? 0 : func.length)
        : (nativeMax(arity - length, 0) || 0);

      if (bitmask == BIND_FLAG) {
        var result = createBindWrapper(newData[0], newData[2]);
      } else if ((bitmask == PARTIAL_FLAG || bitmask == (BIND_FLAG | PARTIAL_FLAG)) && !newData[4].length) {
        result = createPartialWrapper.apply(undefined, newData);
      } else {
        result = createHybridWrapper.apply(undefined, newData);
      }
      var setter = data ? baseSetData : setData;
      return setter(result, newData);
    }

    /**
     * A specialized version of `baseIsEqualDeep` for arrays with support for
     * partial deep comparisons.
     *
     * @private
     * @param {Array} array The array to compare.
     * @param {Array} other The other array to compare.
     * @param {Function} equalFunc The function to determine equivalents of values.
     * @param {Function} [customizer] The function to customize comparing arrays.
     * @param {boolean} [isLoose] Specify performing partial comparisons.
     * @param {Array} [stackA] Tracks traversed `value` objects.
     * @param {Array} [stackB] Tracks traversed `other` objects.
     * @returns {boolean} Returns `true` if the arrays are equivalent, else `false`.
     */
    function equalArrays(array, other, equalFunc, customizer, isLoose, stackA, stackB) {
      var index = -1,
          arrLength = array.length,
          othLength = other.length;

      if (arrLength != othLength && !(isLoose && othLength > arrLength)) {
        return false;
      }
      // Ignore non-index properties.
      while (++index < arrLength) {
        var arrValue = array[index],
            othValue = other[index],
            result = customizer ? customizer(isLoose ? othValue : arrValue, isLoose ? arrValue : othValue, index) : undefined;

        if (result !== undefined) {
          if (result) {
            continue;
          }
          return false;
        }
        // Recursively compare arrays (susceptible to call stack limits).
        if (isLoose) {
          if (!arraySome(other, function(othValue) {
                return arrValue === othValue || equalFunc(arrValue, othValue, customizer, isLoose, stackA, stackB);
              })) {
            return false;
          }
        } else if (!(arrValue === othValue || equalFunc(arrValue, othValue, customizer, isLoose, stackA, stackB))) {
          return false;
        }
      }
      return true;
    }

    /**
     * A specialized version of `baseIsEqualDeep` for comparing objects of
     * the same `toStringTag`.
     *
     * **Note:** This function only supports comparing values with tags of
     * `Boolean`, `Date`, `Error`, `Number`, `RegExp`, or `String`.
     *
     * @private
     * @param {Object} object The object to compare.
     * @param {Object} other The other object to compare.
     * @param {string} tag The `toStringTag` of the objects to compare.
     * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
     */
    function equalByTag(object, other, tag) {
      switch (tag) {
        case boolTag:
        case dateTag:
          // Coerce dates and booleans to numbers, dates to milliseconds and booleans
          // to `1` or `0` treating invalid dates coerced to `NaN` as not equal.
          return +object == +other;

        case errorTag:
          return object.name == other.name && object.message == other.message;

        case numberTag:
          // Treat `NaN` vs. `NaN` as equal.
          return (object != +object)
            ? other != +other
            : object == +other;

        case regexpTag:
        case stringTag:
          // Coerce regexes to strings and treat strings primitives and string
          // objects as equal. See https://es5.github.io/#x15.10.6.4 for more details.
          return object == (other + '');
      }
      return false;
    }

    /**
     * A specialized version of `baseIsEqualDeep` for objects with support for
     * partial deep comparisons.
     *
     * @private
     * @param {Object} object The object to compare.
     * @param {Object} other The other object to compare.
     * @param {Function} equalFunc The function to determine equivalents of values.
     * @param {Function} [customizer] The function to customize comparing values.
     * @param {boolean} [isLoose] Specify performing partial comparisons.
     * @param {Array} [stackA] Tracks traversed `value` objects.
     * @param {Array} [stackB] Tracks traversed `other` objects.
     * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
     */
    function equalObjects(object, other, equalFunc, customizer, isLoose, stackA, stackB) {
      var objProps = keys(object),
          objLength = objProps.length,
          othProps = keys(other),
          othLength = othProps.length;

      if (objLength != othLength && !isLoose) {
        return false;
      }
      var index = objLength;
      while (index--) {
        var key = objProps[index];
        if (!(isLoose ? key in other : hasOwnProperty.call(other, key))) {
          return false;
        }
      }
      var skipCtor = isLoose;
      while (++index < objLength) {
        key = objProps[index];
        var objValue = object[key],
            othValue = other[key],
            result = customizer ? customizer(isLoose ? othValue : objValue, isLoose? objValue : othValue, key) : undefined;

        // Recursively compare objects (susceptible to call stack limits).
        if (!(result === undefined ? equalFunc(objValue, othValue, customizer, isLoose, stackA, stackB) : result)) {
          return false;
        }
        skipCtor || (skipCtor = key == 'constructor');
      }
      if (!skipCtor) {
        var objCtor = object.constructor,
            othCtor = other.constructor;

        // Non `Object` object instances with different constructors are not equal.
        if (objCtor != othCtor &&
            ('constructor' in object && 'constructor' in other) &&
            !(typeof objCtor == 'function' && objCtor instanceof objCtor &&
              typeof othCtor == 'function' && othCtor instanceof othCtor)) {
          return false;
        }
      }
      return true;
    }

    /**
     * Gets the appropriate "callback" function. If the `_.callback` method is
     * customized this function returns the custom method, otherwise it returns
     * the `baseCallback` function. If arguments are provided the chosen function
     * is invoked with them and its result is returned.
     *
     * @private
     * @returns {Function} Returns the chosen function or its result.
     */
    function getCallback(func, thisArg, argCount) {
      var result = lodash.callback || callback;
      result = result === callback ? baseCallback : result;
      return argCount ? result(func, thisArg, argCount) : result;
    }

    /**
     * Gets metadata for `func`.
     *
     * @private
     * @param {Function} func The function to query.
     * @returns {*} Returns the metadata for `func`.
     */
    var getData = !metaMap ? noop : function(func) {
      return metaMap.get(func);
    };

    /**
     * Gets the name of `func`.
     *
     * @private
     * @param {Function} func The function to query.
     * @returns {string} Returns the function name.
     */
    function getFuncName(func) {
      var result = func.name,
          array = realNames[result],
          length = array ? array.length : 0;

      while (length--) {
        var data = array[length],
            otherFunc = data.func;
        if (otherFunc == null || otherFunc == func) {
          return data.name;
        }
      }
      return result;
    }

    /**
     * Gets the appropriate "indexOf" function. If the `_.indexOf` method is
     * customized this function returns the custom method, otherwise it returns
     * the `baseIndexOf` function. If arguments are provided the chosen function
     * is invoked with them and its result is returned.
     *
     * @private
     * @returns {Function|number} Returns the chosen function or its result.
     */
    function getIndexOf(collection, target, fromIndex) {
      var result = lodash.indexOf || indexOf;
      result = result === indexOf ? baseIndexOf : result;
      return collection ? result(collection, target, fromIndex) : result;
    }

    /**
     * Gets the "length" property value of `object`.
     *
     * **Note:** This function is used to avoid a [JIT bug](https://bugs.webkit.org/show_bug.cgi?id=142792)
     * that affects Safari on at least iOS 8.1-8.3 ARM64.
     *
     * @private
     * @param {Object} object The object to query.
     * @returns {*} Returns the "length" value.
     */
    var getLength = baseProperty('length');

    /**
     * Gets the propery names, values, and compare flags of `object`.
     *
     * @private
     * @param {Object} object The object to query.
     * @returns {Array} Returns the match data of `object`.
     */
    function getMatchData(object) {
      var result = pairs(object),
          length = result.length;

      while (length--) {
        result[length][2] = isStrictComparable(result[length][1]);
      }
      return result;
    }

    /**
     * Gets the native function at `key` of `object`.
     *
     * @private
     * @param {Object} object The object to query.
     * @param {string} key The key of the method to get.
     * @returns {*} Returns the function if it's native, else `undefined`.
     */
    function getNative(object, key) {
      var value = object == null ? undefined : object[key];
      return isNative(value) ? value : undefined;
    }

    /**
     * Gets the view, applying any `transforms` to the `start` and `end` positions.
     *
     * @private
     * @param {number} start The start of the view.
     * @param {number} end The end of the view.
     * @param {Array} transforms The transformations to apply to the view.
     * @returns {Object} Returns an object containing the `start` and `end`
     *  positions of the view.
     */
    function getView(start, end, transforms) {
      var index = -1,
          length = transforms.length;

      while (++index < length) {
        var data = transforms[index],
            size = data.size;

        switch (data.type) {
          case 'drop':      start += size; break;
          case 'dropRight': end -= size; break;
          case 'take':      end = nativeMin(end, start + size); break;
          case 'takeRight': start = nativeMax(start, end - size); break;
        }
      }
      return { 'start': start, 'end': end };
    }

    /**
     * Initializes an array clone.
     *
     * @private
     * @param {Array} array The array to clone.
     * @returns {Array} Returns the initialized clone.
     */
    function initCloneArray(array) {
      var length = array.length,
          result = new array.constructor(length);

      // Add array properties assigned by `RegExp#exec`.
      if (length && typeof array[0] == 'string' && hasOwnProperty.call(array, 'index')) {
        result.index = array.index;
        result.input = array.input;
      }
      return result;
    }

    /**
     * Initializes an object clone.
     *
     * @private
     * @param {Object} object The object to clone.
     * @returns {Object} Returns the initialized clone.
     */
    function initCloneObject(object) {
      var Ctor = object.constructor;
      if (!(typeof Ctor == 'function' && Ctor instanceof Ctor)) {
        Ctor = Object;
      }
      return new Ctor;
    }

    /**
     * Initializes an object clone based on its `toStringTag`.
     *
     * **Note:** This function only supports cloning values with tags of
     * `Boolean`, `Date`, `Error`, `Number`, `RegExp`, or `String`.
     *
     * @private
     * @param {Object} object The object to clone.
     * @param {string} tag The `toStringTag` of the object to clone.
     * @param {boolean} [isDeep] Specify a deep clone.
     * @returns {Object} Returns the initialized clone.
     */
    function initCloneByTag(object, tag, isDeep) {
      var Ctor = object.constructor;
      switch (tag) {
        case arrayBufferTag:
          return bufferClone(object);

        case boolTag:
        case dateTag:
          return new Ctor(+object);

        case float32Tag: case float64Tag:
        case int8Tag: case int16Tag: case int32Tag:
        case uint8Tag: case uint8ClampedTag: case uint16Tag: case uint32Tag:
          var buffer = object.buffer;
          return new Ctor(isDeep ? bufferClone(buffer) : buffer, object.byteOffset, object.length);

        case numberTag:
        case stringTag:
          return new Ctor(object);

        case regexpTag:
          var result = new Ctor(object.source, reFlags.exec(object));
          result.lastIndex = object.lastIndex;
      }
      return result;
    }

    /**
     * Invokes the method at `path` on `object`.
     *
     * @private
     * @param {Object} object The object to query.
     * @param {Array|string} path The path of the method to invoke.
     * @param {Array} args The arguments to invoke the method with.
     * @returns {*} Returns the result of the invoked method.
     */
    function invokePath(object, path, args) {
      if (object != null && !isKey(path, object)) {
        path = toPath(path);
        object = path.length == 1 ? object : baseGet(object, baseSlice(path, 0, -1));
        path = last(path);
      }
      var func = object == null ? object : object[path];
      return func == null ? undefined : func.apply(object, args);
    }

    /**
     * Checks if `value` is array-like.
     *
     * @private
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is array-like, else `false`.
     */
    function isArrayLike(value) {
      return value != null && isLength(getLength(value));
    }

    /**
     * Checks if `value` is a valid array-like index.
     *
     * @private
     * @param {*} value The value to check.
     * @param {number} [length=MAX_SAFE_INTEGER] The upper bounds of a valid index.
     * @returns {boolean} Returns `true` if `value` is a valid index, else `false`.
     */
    function isIndex(value, length) {
      value = (typeof value == 'number' || reIsUint.test(value)) ? +value : -1;
      length = length == null ? MAX_SAFE_INTEGER : length;
      return value > -1 && value % 1 == 0 && value < length;
    }

    /**
     * Checks if the provided arguments are from an iteratee call.
     *
     * @private
     * @param {*} value The potential iteratee value argument.
     * @param {*} index The potential iteratee index or key argument.
     * @param {*} object The potential iteratee object argument.
     * @returns {boolean} Returns `true` if the arguments are from an iteratee call, else `false`.
     */
    function isIterateeCall(value, index, object) {
      if (!isObject(object)) {
        return false;
      }
      var type = typeof index;
      if (type == 'number'
          ? (isArrayLike(object) && isIndex(index, object.length))
          : (type == 'string' && index in object)) {
        var other = object[index];
        return value === value ? (value === other) : (other !== other);
      }
      return false;
    }

    /**
     * Checks if `value` is a property name and not a property path.
     *
     * @private
     * @param {*} value The value to check.
     * @param {Object} [object] The object to query keys on.
     * @returns {boolean} Returns `true` if `value` is a property name, else `false`.
     */
    function isKey(value, object) {
      var type = typeof value;
      if ((type == 'string' && reIsPlainProp.test(value)) || type == 'number') {
        return true;
      }
      if (isArray(value)) {
        return false;
      }
      var result = !reIsDeepProp.test(value);
      return result || (object != null && value in toObject(object));
    }

    /**
     * Checks if `func` has a lazy counterpart.
     *
     * @private
     * @param {Function} func The function to check.
     * @returns {boolean} Returns `true` if `func` has a lazy counterpart, else `false`.
     */
    function isLaziable(func) {
      var funcName = getFuncName(func);
      if (!(funcName in LazyWrapper.prototype)) {
        return false;
      }
      var other = lodash[funcName];
      if (func === other) {
        return true;
      }
      var data = getData(other);
      return !!data && func === data[0];
    }

    /**
     * Checks if `value` is a valid array-like length.
     *
     * **Note:** This function is based on [`ToLength`](http://ecma-international.org/ecma-262/6.0/#sec-tolength).
     *
     * @private
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is a valid length, else `false`.
     */
    function isLength(value) {
      return typeof value == 'number' && value > -1 && value % 1 == 0 && value <= MAX_SAFE_INTEGER;
    }

    /**
     * Checks if `value` is suitable for strict equality comparisons, i.e. `===`.
     *
     * @private
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` if suitable for strict
     *  equality comparisons, else `false`.
     */
    function isStrictComparable(value) {
      return value === value && !isObject(value);
    }

    /**
     * Merges the function metadata of `source` into `data`.
     *
     * Merging metadata reduces the number of wrappers required to invoke a function.
     * This is possible because methods like `_.bind`, `_.curry`, and `_.partial`
     * may be applied regardless of execution order. Methods like `_.ary` and `_.rearg`
     * augment function arguments, making the order in which they are executed important,
     * preventing the merging of metadata. However, we make an exception for a safe
     * common case where curried functions have `_.ary` and or `_.rearg` applied.
     *
     * @private
     * @param {Array} data The destination metadata.
     * @param {Array} source The source metadata.
     * @returns {Array} Returns `data`.
     */
    function mergeData(data, source) {
      var bitmask = data[1],
          srcBitmask = source[1],
          newBitmask = bitmask | srcBitmask,
          isCommon = newBitmask < ARY_FLAG;

      var isCombo =
        (srcBitmask == ARY_FLAG && bitmask == CURRY_FLAG) ||
        (srcBitmask == ARY_FLAG && bitmask == REARG_FLAG && data[7].length <= source[8]) ||
        (srcBitmask == (ARY_FLAG | REARG_FLAG) && bitmask == CURRY_FLAG);

      // Exit early if metadata can't be merged.
      if (!(isCommon || isCombo)) {
        return data;
      }
      // Use source `thisArg` if available.
      if (srcBitmask & BIND_FLAG) {
        data[2] = source[2];
        // Set when currying a bound function.
        newBitmask |= (bitmask & BIND_FLAG) ? 0 : CURRY_BOUND_FLAG;
      }
      // Compose partial arguments.
      var value = source[3];
      if (value) {
        var partials = data[3];
        data[3] = partials ? composeArgs(partials, value, source[4]) : arrayCopy(value);
        data[4] = partials ? replaceHolders(data[3], PLACEHOLDER) : arrayCopy(source[4]);
      }
      // Compose partial right arguments.
      value = source[5];
      if (value) {
        partials = data[5];
        data[5] = partials ? composeArgsRight(partials, value, source[6]) : arrayCopy(value);
        data[6] = partials ? replaceHolders(data[5], PLACEHOLDER) : arrayCopy(source[6]);
      }
      // Use source `argPos` if available.
      value = source[7];
      if (value) {
        data[7] = arrayCopy(value);
      }
      // Use source `ary` if it's smaller.
      if (srcBitmask & ARY_FLAG) {
        data[8] = data[8] == null ? source[8] : nativeMin(data[8], source[8]);
      }
      // Use source `arity` if one is not provided.
      if (data[9] == null) {
        data[9] = source[9];
      }
      // Use source `func` and merge bitmasks.
      data[0] = source[0];
      data[1] = newBitmask;

      return data;
    }

    /**
     * Used by `_.defaultsDeep` to customize its `_.merge` use.
     *
     * @private
     * @param {*} objectValue The destination object property value.
     * @param {*} sourceValue The source object property value.
     * @returns {*} Returns the value to assign to the destination object.
     */
    function mergeDefaults(objectValue, sourceValue) {
      return objectValue === undefined ? sourceValue : merge(objectValue, sourceValue, mergeDefaults);
    }

    /**
     * A specialized version of `_.pick` which picks `object` properties specified
     * by `props`.
     *
     * @private
     * @param {Object} object The source object.
     * @param {string[]} props The property names to pick.
     * @returns {Object} Returns the new object.
     */
    function pickByArray(object, props) {
      object = toObject(object);

      var index = -1,
          length = props.length,
          result = {};

      while (++index < length) {
        var key = props[index];
        if (key in object) {
          result[key] = object[key];
        }
      }
      return result;
    }

    /**
     * A specialized version of `_.pick` which picks `object` properties `predicate`
     * returns truthy for.
     *
     * @private
     * @param {Object} object The source object.
     * @param {Function} predicate The function invoked per iteration.
     * @returns {Object} Returns the new object.
     */
    function pickByCallback(object, predicate) {
      var result = {};
      baseForIn(object, function(value, key, object) {
        if (predicate(value, key, object)) {
          result[key] = value;
        }
      });
      return result;
    }

    /**
     * Reorder `array` according to the specified indexes where the element at
     * the first index is assigned as the first element, the element at
     * the second index is assigned as the second element, and so on.
     *
     * @private
     * @param {Array} array The array to reorder.
     * @param {Array} indexes The arranged array indexes.
     * @returns {Array} Returns `array`.
     */
    function reorder(array, indexes) {
      var arrLength = array.length,
          length = nativeMin(indexes.length, arrLength),
          oldArray = arrayCopy(array);

      while (length--) {
        var index = indexes[length];
        array[length] = isIndex(index, arrLength) ? oldArray[index] : undefined;
      }
      return array;
    }

    /**
     * Sets metadata for `func`.
     *
     * **Note:** If this function becomes hot, i.e. is invoked a lot in a short
     * period of time, it will trip its breaker and transition to an identity function
     * to avoid garbage collection pauses in V8. See [V8 issue 2070](https://code.google.com/p/v8/issues/detail?id=2070)
     * for more details.
     *
     * @private
     * @param {Function} func The function to associate metadata with.
     * @param {*} data The metadata.
     * @returns {Function} Returns `func`.
     */
    var setData = (function() {
      var count = 0,
          lastCalled = 0;

      return function(key, value) {
        var stamp = now(),
            remaining = HOT_SPAN - (stamp - lastCalled);

        lastCalled = stamp;
        if (remaining > 0) {
          if (++count >= HOT_COUNT) {
            return key;
          }
        } else {
          count = 0;
        }
        return baseSetData(key, value);
      };
    }());

    /**
     * A fallback implementation of `Object.keys` which creates an array of the
     * own enumerable property names of `object`.
     *
     * @private
     * @param {Object} object The object to query.
     * @returns {Array} Returns the array of property names.
     */
    function shimKeys(object) {
      var props = keysIn(object),
          propsLength = props.length,
          length = propsLength && object.length;

      var allowIndexes = !!length && isLength(length) &&
        (isArray(object) || isArguments(object));

      var index = -1,
          result = [];

      while (++index < propsLength) {
        var key = props[index];
        if ((allowIndexes && isIndex(key, length)) || hasOwnProperty.call(object, key)) {
          result.push(key);
        }
      }
      return result;
    }

    /**
     * Converts `value` to an array-like object if it's not one.
     *
     * @private
     * @param {*} value The value to process.
     * @returns {Array|Object} Returns the array-like object.
     */
    function toIterable(value) {
      if (value == null) {
        return [];
      }
      if (!isArrayLike(value)) {
        return values(value);
      }
      return isObject(value) ? value : Object(value);
    }

    /**
     * Converts `value` to an object if it's not one.
     *
     * @private
     * @param {*} value The value to process.
     * @returns {Object} Returns the object.
     */
    function toObject(value) {
      return isObject(value) ? value : Object(value);
    }

    /**
     * Converts `value` to property path array if it's not one.
     *
     * @private
     * @param {*} value The value to process.
     * @returns {Array} Returns the property path array.
     */
    function toPath(value) {
      if (isArray(value)) {
        return value;
      }
      var result = [];
      baseToString(value).replace(rePropName, function(match, number, quote, string) {
        result.push(quote ? string.replace(reEscapeChar, '$1') : (number || match));
      });
      return result;
    }

    /**
     * Creates a clone of `wrapper`.
     *
     * @private
     * @param {Object} wrapper The wrapper to clone.
     * @returns {Object} Returns the cloned wrapper.
     */
    function wrapperClone(wrapper) {
      return wrapper instanceof LazyWrapper
        ? wrapper.clone()
        : new LodashWrapper(wrapper.__wrapped__, wrapper.__chain__, arrayCopy(wrapper.__actions__));
    }

    /*------------------------------------------------------------------------*/

    /**
     * Creates an array of elements split into groups the length of `size`.
     * If `collection` can't be split evenly, the final chunk will be the remaining
     * elements.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to process.
     * @param {number} [size=1] The length of each chunk.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the new array containing chunks.
     * @example
     *
     * _.chunk(['a', 'b', 'c', 'd'], 2);
     * // => [['a', 'b'], ['c', 'd']]
     *
     * _.chunk(['a', 'b', 'c', 'd'], 3);
     * // => [['a', 'b', 'c'], ['d']]
     */
    function chunk(array, size, guard) {
      if (guard ? isIterateeCall(array, size, guard) : size == null) {
        size = 1;
      } else {
        size = nativeMax(nativeFloor(size) || 1, 1);
      }
      var index = 0,
          length = array ? array.length : 0,
          resIndex = -1,
          result = Array(nativeCeil(length / size));

      while (index < length) {
        result[++resIndex] = baseSlice(array, index, (index += size));
      }
      return result;
    }

    /**
     * Creates an array with all falsey values removed. The values `false`, `null`,
     * `0`, `""`, `undefined`, and `NaN` are falsey.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to compact.
     * @returns {Array} Returns the new array of filtered values.
     * @example
     *
     * _.compact([0, 1, false, 2, '', 3]);
     * // => [1, 2, 3]
     */
    function compact(array) {
      var index = -1,
          length = array ? array.length : 0,
          resIndex = -1,
          result = [];

      while (++index < length) {
        var value = array[index];
        if (value) {
          result[++resIndex] = value;
        }
      }
      return result;
    }

    /**
     * Creates an array of unique `array` values not included in the other
     * provided arrays using [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to inspect.
     * @param {...Array} [values] The arrays of values to exclude.
     * @returns {Array} Returns the new array of filtered values.
     * @example
     *
     * _.difference([1, 2, 3], [4, 2]);
     * // => [1, 3]
     */
    var difference = restParam(function(array, values) {
      return (isObjectLike(array) && isArrayLike(array))
        ? baseDifference(array, baseFlatten(values, false, true))
        : [];
    });

    /**
     * Creates a slice of `array` with `n` elements dropped from the beginning.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {number} [n=1] The number of elements to drop.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.drop([1, 2, 3]);
     * // => [2, 3]
     *
     * _.drop([1, 2, 3], 2);
     * // => [3]
     *
     * _.drop([1, 2, 3], 5);
     * // => []
     *
     * _.drop([1, 2, 3], 0);
     * // => [1, 2, 3]
     */
    function drop(array, n, guard) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (guard ? isIterateeCall(array, n, guard) : n == null) {
        n = 1;
      }
      return baseSlice(array, n < 0 ? 0 : n);
    }

    /**
     * Creates a slice of `array` with `n` elements dropped from the end.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {number} [n=1] The number of elements to drop.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.dropRight([1, 2, 3]);
     * // => [1, 2]
     *
     * _.dropRight([1, 2, 3], 2);
     * // => [1]
     *
     * _.dropRight([1, 2, 3], 5);
     * // => []
     *
     * _.dropRight([1, 2, 3], 0);
     * // => [1, 2, 3]
     */
    function dropRight(array, n, guard) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (guard ? isIterateeCall(array, n, guard) : n == null) {
        n = 1;
      }
      n = length - (+n || 0);
      return baseSlice(array, 0, n < 0 ? 0 : n);
    }

    /**
     * Creates a slice of `array` excluding elements dropped from the end.
     * Elements are dropped until `predicate` returns falsey. The predicate is
     * bound to `thisArg` and invoked with three arguments: (value, index, array).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that match the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.dropRightWhile([1, 2, 3], function(n) {
     *   return n > 1;
     * });
     * // => [1]
     *
     * var users = [
     *   { 'user': 'barney',  'active': true },
     *   { 'user': 'fred',    'active': false },
     *   { 'user': 'pebbles', 'active': false }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.pluck(_.dropRightWhile(users, { 'user': 'pebbles', 'active': false }), 'user');
     * // => ['barney', 'fred']
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.pluck(_.dropRightWhile(users, 'active', false), 'user');
     * // => ['barney']
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.dropRightWhile(users, 'active'), 'user');
     * // => ['barney', 'fred', 'pebbles']
     */
    function dropRightWhile(array, predicate, thisArg) {
      return (array && array.length)
        ? baseWhile(array, getCallback(predicate, thisArg, 3), true, true)
        : [];
    }

    /**
     * Creates a slice of `array` excluding elements dropped from the beginning.
     * Elements are dropped until `predicate` returns falsey. The predicate is
     * bound to `thisArg` and invoked with three arguments: (value, index, array).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.dropWhile([1, 2, 3], function(n) {
     *   return n < 3;
     * });
     * // => [3]
     *
     * var users = [
     *   { 'user': 'barney',  'active': false },
     *   { 'user': 'fred',    'active': false },
     *   { 'user': 'pebbles', 'active': true }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.pluck(_.dropWhile(users, { 'user': 'barney', 'active': false }), 'user');
     * // => ['fred', 'pebbles']
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.pluck(_.dropWhile(users, 'active', false), 'user');
     * // => ['pebbles']
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.dropWhile(users, 'active'), 'user');
     * // => ['barney', 'fred', 'pebbles']
     */
    function dropWhile(array, predicate, thisArg) {
      return (array && array.length)
        ? baseWhile(array, getCallback(predicate, thisArg, 3), true)
        : [];
    }

    /**
     * Fills elements of `array` with `value` from `start` up to, but not
     * including, `end`.
     *
     * **Note:** This method mutates `array`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to fill.
     * @param {*} value The value to fill `array` with.
     * @param {number} [start=0] The start position.
     * @param {number} [end=array.length] The end position.
     * @returns {Array} Returns `array`.
     * @example
     *
     * var array = [1, 2, 3];
     *
     * _.fill(array, 'a');
     * console.log(array);
     * // => ['a', 'a', 'a']
     *
     * _.fill(Array(3), 2);
     * // => [2, 2, 2]
     *
     * _.fill([4, 6, 8], '*', 1, 2);
     * // => [4, '*', 8]
     */
    function fill(array, value, start, end) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (start && typeof start != 'number' && isIterateeCall(array, value, start)) {
        start = 0;
        end = length;
      }
      return baseFill(array, value, start, end);
    }

    /**
     * This method is like `_.find` except that it returns the index of the first
     * element `predicate` returns truthy for instead of the element itself.
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to search.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {number} Returns the index of the found element, else `-1`.
     * @example
     *
     * var users = [
     *   { 'user': 'barney',  'active': false },
     *   { 'user': 'fred',    'active': false },
     *   { 'user': 'pebbles', 'active': true }
     * ];
     *
     * _.findIndex(users, function(chr) {
     *   return chr.user == 'barney';
     * });
     * // => 0
     *
     * // using the `_.matches` callback shorthand
     * _.findIndex(users, { 'user': 'fred', 'active': false });
     * // => 1
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.findIndex(users, 'active', false);
     * // => 0
     *
     * // using the `_.property` callback shorthand
     * _.findIndex(users, 'active');
     * // => 2
     */
    var findIndex = createFindIndex();

    /**
     * This method is like `_.findIndex` except that it iterates over elements
     * of `collection` from right to left.
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to search.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {number} Returns the index of the found element, else `-1`.
     * @example
     *
     * var users = [
     *   { 'user': 'barney',  'active': true },
     *   { 'user': 'fred',    'active': false },
     *   { 'user': 'pebbles', 'active': false }
     * ];
     *
     * _.findLastIndex(users, function(chr) {
     *   return chr.user == 'pebbles';
     * });
     * // => 2
     *
     * // using the `_.matches` callback shorthand
     * _.findLastIndex(users, { 'user': 'barney', 'active': true });
     * // => 0
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.findLastIndex(users, 'active', false);
     * // => 2
     *
     * // using the `_.property` callback shorthand
     * _.findLastIndex(users, 'active');
     * // => 0
     */
    var findLastIndex = createFindIndex(true);

    /**
     * Gets the first element of `array`.
     *
     * @static
     * @memberOf _
     * @alias head
     * @category Array
     * @param {Array} array The array to query.
     * @returns {*} Returns the first element of `array`.
     * @example
     *
     * _.first([1, 2, 3]);
     * // => 1
     *
     * _.first([]);
     * // => undefined
     */
    function first(array) {
      return array ? array[0] : undefined;
    }

    /**
     * Flattens a nested array. If `isDeep` is `true` the array is recursively
     * flattened, otherwise it is only flattened a single level.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to flatten.
     * @param {boolean} [isDeep] Specify a deep flatten.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the new flattened array.
     * @example
     *
     * _.flatten([1, [2, 3, [4]]]);
     * // => [1, 2, 3, [4]]
     *
     * // using `isDeep`
     * _.flatten([1, [2, 3, [4]]], true);
     * // => [1, 2, 3, 4]
     */
    function flatten(array, isDeep, guard) {
      var length = array ? array.length : 0;
      if (guard && isIterateeCall(array, isDeep, guard)) {
        isDeep = false;
      }
      return length ? baseFlatten(array, isDeep) : [];
    }

    /**
     * Recursively flattens a nested array.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to recursively flatten.
     * @returns {Array} Returns the new flattened array.
     * @example
     *
     * _.flattenDeep([1, [2, 3, [4]]]);
     * // => [1, 2, 3, 4]
     */
    function flattenDeep(array) {
      var length = array ? array.length : 0;
      return length ? baseFlatten(array, true) : [];
    }

    /**
     * Gets the index at which the first occurrence of `value` is found in `array`
     * using [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons. If `fromIndex` is negative, it is used as the offset
     * from the end of `array`. If `array` is sorted providing `true` for `fromIndex`
     * performs a faster binary search.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to search.
     * @param {*} value The value to search for.
     * @param {boolean|number} [fromIndex=0] The index to search from or `true`
     *  to perform a binary search on a sorted array.
     * @returns {number} Returns the index of the matched value, else `-1`.
     * @example
     *
     * _.indexOf([1, 2, 1, 2], 2);
     * // => 1
     *
     * // using `fromIndex`
     * _.indexOf([1, 2, 1, 2], 2, 2);
     * // => 3
     *
     * // performing a binary search
     * _.indexOf([1, 1, 2, 2], 2, true);
     * // => 2
     */
    function indexOf(array, value, fromIndex) {
      var length = array ? array.length : 0;
      if (!length) {
        return -1;
      }
      if (typeof fromIndex == 'number') {
        fromIndex = fromIndex < 0 ? nativeMax(length + fromIndex, 0) : fromIndex;
      } else if (fromIndex) {
        var index = binaryIndex(array, value);
        if (index < length &&
            (value === value ? (value === array[index]) : (array[index] !== array[index]))) {
          return index;
        }
        return -1;
      }
      return baseIndexOf(array, value, fromIndex || 0);
    }

    /**
     * Gets all but the last element of `array`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.initial([1, 2, 3]);
     * // => [1, 2]
     */
    function initial(array) {
      return dropRight(array, 1);
    }

    /**
     * Creates an array of unique values that are included in all of the provided
     * arrays using [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {...Array} [arrays] The arrays to inspect.
     * @returns {Array} Returns the new array of shared values.
     * @example
     * _.intersection([1, 2], [4, 2], [2, 1]);
     * // => [2]
     */
    var intersection = restParam(function(arrays) {
      var othLength = arrays.length,
          othIndex = othLength,
          caches = Array(length),
          indexOf = getIndexOf(),
          isCommon = indexOf == baseIndexOf,
          result = [];

      while (othIndex--) {
        var value = arrays[othIndex] = isArrayLike(value = arrays[othIndex]) ? value : [];
        caches[othIndex] = (isCommon && value.length >= 120) ? createCache(othIndex && value) : null;
      }
      var array = arrays[0],
          index = -1,
          length = array ? array.length : 0,
          seen = caches[0];

      outer:
      while (++index < length) {
        value = array[index];
        if ((seen ? cacheIndexOf(seen, value) : indexOf(result, value, 0)) < 0) {
          var othIndex = othLength;
          while (--othIndex) {
            var cache = caches[othIndex];
            if ((cache ? cacheIndexOf(cache, value) : indexOf(arrays[othIndex], value, 0)) < 0) {
              continue outer;
            }
          }
          if (seen) {
            seen.push(value);
          }
          result.push(value);
        }
      }
      return result;
    });

    /**
     * Gets the last element of `array`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @returns {*} Returns the last element of `array`.
     * @example
     *
     * _.last([1, 2, 3]);
     * // => 3
     */
    function last(array) {
      var length = array ? array.length : 0;
      return length ? array[length - 1] : undefined;
    }

    /**
     * This method is like `_.indexOf` except that it iterates over elements of
     * `array` from right to left.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to search.
     * @param {*} value The value to search for.
     * @param {boolean|number} [fromIndex=array.length-1] The index to search from
     *  or `true` to perform a binary search on a sorted array.
     * @returns {number} Returns the index of the matched value, else `-1`.
     * @example
     *
     * _.lastIndexOf([1, 2, 1, 2], 2);
     * // => 3
     *
     * // using `fromIndex`
     * _.lastIndexOf([1, 2, 1, 2], 2, 2);
     * // => 1
     *
     * // performing a binary search
     * _.lastIndexOf([1, 1, 2, 2], 2, true);
     * // => 3
     */
    function lastIndexOf(array, value, fromIndex) {
      var length = array ? array.length : 0;
      if (!length) {
        return -1;
      }
      var index = length;
      if (typeof fromIndex == 'number') {
        index = (fromIndex < 0 ? nativeMax(length + fromIndex, 0) : nativeMin(fromIndex || 0, length - 1)) + 1;
      } else if (fromIndex) {
        index = binaryIndex(array, value, true) - 1;
        var other = array[index];
        if (value === value ? (value === other) : (other !== other)) {
          return index;
        }
        return -1;
      }
      if (value !== value) {
        return indexOfNaN(array, index, true);
      }
      while (index--) {
        if (array[index] === value) {
          return index;
        }
      }
      return -1;
    }

    /**
     * Removes all provided values from `array` using
     * [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons.
     *
     * **Note:** Unlike `_.without`, this method mutates `array`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to modify.
     * @param {...*} [values] The values to remove.
     * @returns {Array} Returns `array`.
     * @example
     *
     * var array = [1, 2, 3, 1, 2, 3];
     *
     * _.pull(array, 2, 3);
     * console.log(array);
     * // => [1, 1]
     */
    function pull() {
      var args = arguments,
          array = args[0];

      if (!(array && array.length)) {
        return array;
      }
      var index = 0,
          indexOf = getIndexOf(),
          length = args.length;

      while (++index < length) {
        var fromIndex = 0,
            value = args[index];

        while ((fromIndex = indexOf(array, value, fromIndex)) > -1) {
          splice.call(array, fromIndex, 1);
        }
      }
      return array;
    }

    /**
     * Removes elements from `array` corresponding to the given indexes and returns
     * an array of the removed elements. Indexes may be specified as an array of
     * indexes or as individual arguments.
     *
     * **Note:** Unlike `_.at`, this method mutates `array`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to modify.
     * @param {...(number|number[])} [indexes] The indexes of elements to remove,
     *  specified as individual indexes or arrays of indexes.
     * @returns {Array} Returns the new array of removed elements.
     * @example
     *
     * var array = [5, 10, 15, 20];
     * var evens = _.pullAt(array, 1, 3);
     *
     * console.log(array);
     * // => [5, 15]
     *
     * console.log(evens);
     * // => [10, 20]
     */
    var pullAt = restParam(function(array, indexes) {
      indexes = baseFlatten(indexes);

      var result = baseAt(array, indexes);
      basePullAt(array, indexes.sort(baseCompareAscending));
      return result;
    });

    /**
     * Removes all elements from `array` that `predicate` returns truthy for
     * and returns an array of the removed elements. The predicate is bound to
     * `thisArg` and invoked with three arguments: (value, index, array).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * **Note:** Unlike `_.filter`, this method mutates `array`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to modify.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the new array of removed elements.
     * @example
     *
     * var array = [1, 2, 3, 4];
     * var evens = _.remove(array, function(n) {
     *   return n % 2 == 0;
     * });
     *
     * console.log(array);
     * // => [1, 3]
     *
     * console.log(evens);
     * // => [2, 4]
     */
    function remove(array, predicate, thisArg) {
      var result = [];
      if (!(array && array.length)) {
        return result;
      }
      var index = -1,
          indexes = [],
          length = array.length;

      predicate = getCallback(predicate, thisArg, 3);
      while (++index < length) {
        var value = array[index];
        if (predicate(value, index, array)) {
          result.push(value);
          indexes.push(index);
        }
      }
      basePullAt(array, indexes);
      return result;
    }

    /**
     * Gets all but the first element of `array`.
     *
     * @static
     * @memberOf _
     * @alias tail
     * @category Array
     * @param {Array} array The array to query.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.rest([1, 2, 3]);
     * // => [2, 3]
     */
    function rest(array) {
      return drop(array, 1);
    }

    /**
     * Creates a slice of `array` from `start` up to, but not including, `end`.
     *
     * **Note:** This method is used instead of `Array#slice` to support node
     * lists in IE < 9 and to ensure dense arrays are returned.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to slice.
     * @param {number} [start=0] The start position.
     * @param {number} [end=array.length] The end position.
     * @returns {Array} Returns the slice of `array`.
     */
    function slice(array, start, end) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (end && typeof end != 'number' && isIterateeCall(array, start, end)) {
        start = 0;
        end = length;
      }
      return baseSlice(array, start, end);
    }

    /**
     * Uses a binary search to determine the lowest index at which `value` should
     * be inserted into `array` in order to maintain its sort order. If an iteratee
     * function is provided it is invoked for `value` and each element of `array`
     * to compute their sort ranking. The iteratee is bound to `thisArg` and
     * invoked with one argument; (value).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The sorted array to inspect.
     * @param {*} value The value to evaluate.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {number} Returns the index at which `value` should be inserted
     *  into `array`.
     * @example
     *
     * _.sortedIndex([30, 50], 40);
     * // => 1
     *
     * _.sortedIndex([4, 4, 5, 5], 5);
     * // => 2
     *
     * var dict = { 'data': { 'thirty': 30, 'forty': 40, 'fifty': 50 } };
     *
     * // using an iteratee function
     * _.sortedIndex(['thirty', 'fifty'], 'forty', function(word) {
     *   return this.data[word];
     * }, dict);
     * // => 1
     *
     * // using the `_.property` callback shorthand
     * _.sortedIndex([{ 'x': 30 }, { 'x': 50 }], { 'x': 40 }, 'x');
     * // => 1
     */
    var sortedIndex = createSortedIndex();

    /**
     * This method is like `_.sortedIndex` except that it returns the highest
     * index at which `value` should be inserted into `array` in order to
     * maintain its sort order.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The sorted array to inspect.
     * @param {*} value The value to evaluate.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {number} Returns the index at which `value` should be inserted
     *  into `array`.
     * @example
     *
     * _.sortedLastIndex([4, 4, 5, 5], 5);
     * // => 4
     */
    var sortedLastIndex = createSortedIndex(true);

    /**
     * Creates a slice of `array` with `n` elements taken from the beginning.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {number} [n=1] The number of elements to take.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.take([1, 2, 3]);
     * // => [1]
     *
     * _.take([1, 2, 3], 2);
     * // => [1, 2]
     *
     * _.take([1, 2, 3], 5);
     * // => [1, 2, 3]
     *
     * _.take([1, 2, 3], 0);
     * // => []
     */
    function take(array, n, guard) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (guard ? isIterateeCall(array, n, guard) : n == null) {
        n = 1;
      }
      return baseSlice(array, 0, n < 0 ? 0 : n);
    }

    /**
     * Creates a slice of `array` with `n` elements taken from the end.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {number} [n=1] The number of elements to take.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.takeRight([1, 2, 3]);
     * // => [3]
     *
     * _.takeRight([1, 2, 3], 2);
     * // => [2, 3]
     *
     * _.takeRight([1, 2, 3], 5);
     * // => [1, 2, 3]
     *
     * _.takeRight([1, 2, 3], 0);
     * // => []
     */
    function takeRight(array, n, guard) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (guard ? isIterateeCall(array, n, guard) : n == null) {
        n = 1;
      }
      n = length - (+n || 0);
      return baseSlice(array, n < 0 ? 0 : n);
    }

    /**
     * Creates a slice of `array` with elements taken from the end. Elements are
     * taken until `predicate` returns falsey. The predicate is bound to `thisArg`
     * and invoked with three arguments: (value, index, array).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.takeRightWhile([1, 2, 3], function(n) {
     *   return n > 1;
     * });
     * // => [2, 3]
     *
     * var users = [
     *   { 'user': 'barney',  'active': true },
     *   { 'user': 'fred',    'active': false },
     *   { 'user': 'pebbles', 'active': false }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.pluck(_.takeRightWhile(users, { 'user': 'pebbles', 'active': false }), 'user');
     * // => ['pebbles']
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.pluck(_.takeRightWhile(users, 'active', false), 'user');
     * // => ['fred', 'pebbles']
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.takeRightWhile(users, 'active'), 'user');
     * // => []
     */
    function takeRightWhile(array, predicate, thisArg) {
      return (array && array.length)
        ? baseWhile(array, getCallback(predicate, thisArg, 3), false, true)
        : [];
    }

    /**
     * Creates a slice of `array` with elements taken from the beginning. Elements
     * are taken until `predicate` returns falsey. The predicate is bound to
     * `thisArg` and invoked with three arguments: (value, index, array).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to query.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the slice of `array`.
     * @example
     *
     * _.takeWhile([1, 2, 3], function(n) {
     *   return n < 3;
     * });
     * // => [1, 2]
     *
     * var users = [
     *   { 'user': 'barney',  'active': false },
     *   { 'user': 'fred',    'active': false},
     *   { 'user': 'pebbles', 'active': true }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.pluck(_.takeWhile(users, { 'user': 'barney', 'active': false }), 'user');
     * // => ['barney']
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.pluck(_.takeWhile(users, 'active', false), 'user');
     * // => ['barney', 'fred']
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.takeWhile(users, 'active'), 'user');
     * // => []
     */
    function takeWhile(array, predicate, thisArg) {
      return (array && array.length)
        ? baseWhile(array, getCallback(predicate, thisArg, 3))
        : [];
    }

    /**
     * Creates an array of unique values, in order, from all of the provided arrays
     * using [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {...Array} [arrays] The arrays to inspect.
     * @returns {Array} Returns the new array of combined values.
     * @example
     *
     * _.union([1, 2], [4, 2], [2, 1]);
     * // => [1, 2, 4]
     */
    var union = restParam(function(arrays) {
      return baseUniq(baseFlatten(arrays, false, true));
    });

    /**
     * Creates a duplicate-free version of an array, using
     * [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons, in which only the first occurence of each element
     * is kept. Providing `true` for `isSorted` performs a faster search algorithm
     * for sorted arrays. If an iteratee function is provided it is invoked for
     * each element in the array to generate the criterion by which uniqueness
     * is computed. The `iteratee` is bound to `thisArg` and invoked with three
     * arguments: (value, index, array).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @alias unique
     * @category Array
     * @param {Array} array The array to inspect.
     * @param {boolean} [isSorted] Specify the array is sorted.
     * @param {Function|Object|string} [iteratee] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array} Returns the new duplicate-value-free array.
     * @example
     *
     * _.uniq([2, 1, 2]);
     * // => [2, 1]
     *
     * // using `isSorted`
     * _.uniq([1, 1, 2], true);
     * // => [1, 2]
     *
     * // using an iteratee function
     * _.uniq([1, 2.5, 1.5, 2], function(n) {
     *   return this.floor(n);
     * }, Math);
     * // => [1, 2.5]
     *
     * // using the `_.property` callback shorthand
     * _.uniq([{ 'x': 1 }, { 'x': 2 }, { 'x': 1 }], 'x');
     * // => [{ 'x': 1 }, { 'x': 2 }]
     */
    function uniq(array, isSorted, iteratee, thisArg) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      if (isSorted != null && typeof isSorted != 'boolean') {
        thisArg = iteratee;
        iteratee = isIterateeCall(array, isSorted, thisArg) ? undefined : isSorted;
        isSorted = false;
      }
      var callback = getCallback();
      if (!(iteratee == null && callback === baseCallback)) {
        iteratee = callback(iteratee, thisArg, 3);
      }
      return (isSorted && getIndexOf() == baseIndexOf)
        ? sortedUniq(array, iteratee)
        : baseUniq(array, iteratee);
    }

    /**
     * This method is like `_.zip` except that it accepts an array of grouped
     * elements and creates an array regrouping the elements to their pre-zip
     * configuration.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array of grouped elements to process.
     * @returns {Array} Returns the new array of regrouped elements.
     * @example
     *
     * var zipped = _.zip(['fred', 'barney'], [30, 40], [true, false]);
     * // => [['fred', 30, true], ['barney', 40, false]]
     *
     * _.unzip(zipped);
     * // => [['fred', 'barney'], [30, 40], [true, false]]
     */
    function unzip(array) {
      if (!(array && array.length)) {
        return [];
      }
      var index = -1,
          length = 0;

      array = arrayFilter(array, function(group) {
        if (isArrayLike(group)) {
          length = nativeMax(group.length, length);
          return true;
        }
      });
      var result = Array(length);
      while (++index < length) {
        result[index] = arrayMap(array, baseProperty(index));
      }
      return result;
    }

    /**
     * This method is like `_.unzip` except that it accepts an iteratee to specify
     * how regrouped values should be combined. The `iteratee` is bound to `thisArg`
     * and invoked with four arguments: (accumulator, value, index, group).
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array of grouped elements to process.
     * @param {Function} [iteratee] The function to combine regrouped values.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array} Returns the new array of regrouped elements.
     * @example
     *
     * var zipped = _.zip([1, 2], [10, 20], [100, 200]);
     * // => [[1, 10, 100], [2, 20, 200]]
     *
     * _.unzipWith(zipped, _.add);
     * // => [3, 30, 300]
     */
    function unzipWith(array, iteratee, thisArg) {
      var length = array ? array.length : 0;
      if (!length) {
        return [];
      }
      var result = unzip(array);
      if (iteratee == null) {
        return result;
      }
      iteratee = bindCallback(iteratee, thisArg, 4);
      return arrayMap(result, function(group) {
        return arrayReduce(group, iteratee, undefined, true);
      });
    }

    /**
     * Creates an array excluding all provided values using
     * [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {Array} array The array to filter.
     * @param {...*} [values] The values to exclude.
     * @returns {Array} Returns the new array of filtered values.
     * @example
     *
     * _.without([1, 2, 1, 3], 1, 2);
     * // => [3]
     */
    var without = restParam(function(array, values) {
      return isArrayLike(array)
        ? baseDifference(array, values)
        : [];
    });

    /**
     * Creates an array of unique values that is the [symmetric difference](https://en.wikipedia.org/wiki/Symmetric_difference)
     * of the provided arrays.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {...Array} [arrays] The arrays to inspect.
     * @returns {Array} Returns the new array of values.
     * @example
     *
     * _.xor([1, 2], [4, 2]);
     * // => [1, 4]
     */
    function xor() {
      var index = -1,
          length = arguments.length;

      while (++index < length) {
        var array = arguments[index];
        if (isArrayLike(array)) {
          var result = result
            ? arrayPush(baseDifference(result, array), baseDifference(array, result))
            : array;
        }
      }
      return result ? baseUniq(result) : [];
    }

    /**
     * Creates an array of grouped elements, the first of which contains the first
     * elements of the given arrays, the second of which contains the second elements
     * of the given arrays, and so on.
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {...Array} [arrays] The arrays to process.
     * @returns {Array} Returns the new array of grouped elements.
     * @example
     *
     * _.zip(['fred', 'barney'], [30, 40], [true, false]);
     * // => [['fred', 30, true], ['barney', 40, false]]
     */
    var zip = restParam(unzip);

    /**
     * The inverse of `_.pairs`; this method returns an object composed from arrays
     * of property names and values. Provide either a single two dimensional array,
     * e.g. `[[key1, value1], [key2, value2]]` or two arrays, one of property names
     * and one of corresponding values.
     *
     * @static
     * @memberOf _
     * @alias object
     * @category Array
     * @param {Array} props The property names.
     * @param {Array} [values=[]] The property values.
     * @returns {Object} Returns the new object.
     * @example
     *
     * _.zipObject([['fred', 30], ['barney', 40]]);
     * // => { 'fred': 30, 'barney': 40 }
     *
     * _.zipObject(['fred', 'barney'], [30, 40]);
     * // => { 'fred': 30, 'barney': 40 }
     */
    function zipObject(props, values) {
      var index = -1,
          length = props ? props.length : 0,
          result = {};

      if (length && !values && !isArray(props[0])) {
        values = [];
      }
      while (++index < length) {
        var key = props[index];
        if (values) {
          result[key] = values[index];
        } else if (key) {
          result[key[0]] = key[1];
        }
      }
      return result;
    }

    /**
     * This method is like `_.zip` except that it accepts an iteratee to specify
     * how grouped values should be combined. The `iteratee` is bound to `thisArg`
     * and invoked with four arguments: (accumulator, value, index, group).
     *
     * @static
     * @memberOf _
     * @category Array
     * @param {...Array} [arrays] The arrays to process.
     * @param {Function} [iteratee] The function to combine grouped values.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array} Returns the new array of grouped elements.
     * @example
     *
     * _.zipWith([1, 2], [10, 20], [100, 200], _.add);
     * // => [111, 222]
     */
    var zipWith = restParam(function(arrays) {
      var length = arrays.length,
          iteratee = length > 2 ? arrays[length - 2] : undefined,
          thisArg = length > 1 ? arrays[length - 1] : undefined;

      if (length > 2 && typeof iteratee == 'function') {
        length -= 2;
      } else {
        iteratee = (length > 1 && typeof thisArg == 'function') ? (--length, thisArg) : undefined;
        thisArg = undefined;
      }
      arrays.length = length;
      return unzipWith(arrays, iteratee, thisArg);
    });

    /*------------------------------------------------------------------------*/

    /**
     * Creates a `lodash` object that wraps `value` with explicit method
     * chaining enabled.
     *
     * @static
     * @memberOf _
     * @category Chain
     * @param {*} value The value to wrap.
     * @returns {Object} Returns the new `lodash` wrapper instance.
     * @example
     *
     * var users = [
     *   { 'user': 'barney',  'age': 36 },
     *   { 'user': 'fred',    'age': 40 },
     *   { 'user': 'pebbles', 'age': 1 }
     * ];
     *
     * var youngest = _.chain(users)
     *   .sortBy('age')
     *   .map(function(chr) {
     *     return chr.user + ' is ' + chr.age;
     *   })
     *   .first()
     *   .value();
     * // => 'pebbles is 1'
     */
    function chain(value) {
      var result = lodash(value);
      result.__chain__ = true;
      return result;
    }

    /**
     * This method invokes `interceptor` and returns `value`. The interceptor is
     * bound to `thisArg` and invoked with one argument; (value). The purpose of
     * this method is to "tap into" a method chain in order to perform operations
     * on intermediate results within the chain.
     *
     * @static
     * @memberOf _
     * @category Chain
     * @param {*} value The value to provide to `interceptor`.
     * @param {Function} interceptor The function to invoke.
     * @param {*} [thisArg] The `this` binding of `interceptor`.
     * @returns {*} Returns `value`.
     * @example
     *
     * _([1, 2, 3])
     *  .tap(function(array) {
     *    array.pop();
     *  })
     *  .reverse()
     *  .value();
     * // => [2, 1]
     */
    function tap(value, interceptor, thisArg) {
      interceptor.call(thisArg, value);
      return value;
    }

    /**
     * This method is like `_.tap` except that it returns the result of `interceptor`.
     *
     * @static
     * @memberOf _
     * @category Chain
     * @param {*} value The value to provide to `interceptor`.
     * @param {Function} interceptor The function to invoke.
     * @param {*} [thisArg] The `this` binding of `interceptor`.
     * @returns {*} Returns the result of `interceptor`.
     * @example
     *
     * _('  abc  ')
     *  .chain()
     *  .trim()
     *  .thru(function(value) {
     *    return [value];
     *  })
     *  .value();
     * // => ['abc']
     */
    function thru(value, interceptor, thisArg) {
      return interceptor.call(thisArg, value);
    }

    /**
     * Enables explicit method chaining on the wrapper object.
     *
     * @name chain
     * @memberOf _
     * @category Chain
     * @returns {Object} Returns the new `lodash` wrapper instance.
     * @example
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36 },
     *   { 'user': 'fred',   'age': 40 }
     * ];
     *
     * // without explicit chaining
     * _(users).first();
     * // => { 'user': 'barney', 'age': 36 }
     *
     * // with explicit chaining
     * _(users).chain()
     *   .first()
     *   .pick('user')
     *   .value();
     * // => { 'user': 'barney' }
     */
    function wrapperChain() {
      return chain(this);
    }

    /**
     * Executes the chained sequence and returns the wrapped result.
     *
     * @name commit
     * @memberOf _
     * @category Chain
     * @returns {Object} Returns the new `lodash` wrapper instance.
     * @example
     *
     * var array = [1, 2];
     * var wrapped = _(array).push(3);
     *
     * console.log(array);
     * // => [1, 2]
     *
     * wrapped = wrapped.commit();
     * console.log(array);
     * // => [1, 2, 3]
     *
     * wrapped.last();
     * // => 3
     *
     * console.log(array);
     * // => [1, 2, 3]
     */
    function wrapperCommit() {
      return new LodashWrapper(this.value(), this.__chain__);
    }

    /**
     * Creates a new array joining a wrapped array with any additional arrays
     * and/or values.
     *
     * @name concat
     * @memberOf _
     * @category Chain
     * @param {...*} [values] The values to concatenate.
     * @returns {Array} Returns the new concatenated array.
     * @example
     *
     * var array = [1];
     * var wrapped = _(array).concat(2, [3], [[4]]);
     *
     * console.log(wrapped.value());
     * // => [1, 2, 3, [4]]
     *
     * console.log(array);
     * // => [1]
     */
    var wrapperConcat = restParam(function(values) {
      values = baseFlatten(values);
      return this.thru(function(array) {
        return arrayConcat(isArray(array) ? array : [toObject(array)], values);
      });
    });

    /**
     * Creates a clone of the chained sequence planting `value` as the wrapped value.
     *
     * @name plant
     * @memberOf _
     * @category Chain
     * @returns {Object} Returns the new `lodash` wrapper instance.
     * @example
     *
     * var array = [1, 2];
     * var wrapped = _(array).map(function(value) {
     *   return Math.pow(value, 2);
     * });
     *
     * var other = [3, 4];
     * var otherWrapped = wrapped.plant(other);
     *
     * otherWrapped.value();
     * // => [9, 16]
     *
     * wrapped.value();
     * // => [1, 4]
     */
    function wrapperPlant(value) {
      var result,
          parent = this;

      while (parent instanceof baseLodash) {
        var clone = wrapperClone(parent);
        if (result) {
          previous.__wrapped__ = clone;
        } else {
          result = clone;
        }
        var previous = clone;
        parent = parent.__wrapped__;
      }
      previous.__wrapped__ = value;
      return result;
    }

    /**
     * Reverses the wrapped array so the first element becomes the last, the
     * second element becomes the second to last, and so on.
     *
     * **Note:** This method mutates the wrapped array.
     *
     * @name reverse
     * @memberOf _
     * @category Chain
     * @returns {Object} Returns the new reversed `lodash` wrapper instance.
     * @example
     *
     * var array = [1, 2, 3];
     *
     * _(array).reverse().value()
     * // => [3, 2, 1]
     *
     * console.log(array);
     * // => [3, 2, 1]
     */
    function wrapperReverse() {
      var value = this.__wrapped__;

      var interceptor = function(value) {
        return (wrapped && wrapped.__dir__ < 0) ? value : value.reverse();
      };
      if (value instanceof LazyWrapper) {
        var wrapped = value;
        if (this.__actions__.length) {
          wrapped = new LazyWrapper(this);
        }
        wrapped = wrapped.reverse();
        wrapped.__actions__.push({ 'func': thru, 'args': [interceptor], 'thisArg': undefined });
        return new LodashWrapper(wrapped, this.__chain__);
      }
      return this.thru(interceptor);
    }

    /**
     * Produces the result of coercing the unwrapped value to a string.
     *
     * @name toString
     * @memberOf _
     * @category Chain
     * @returns {string} Returns the coerced string value.
     * @example
     *
     * _([1, 2, 3]).toString();
     * // => '1,2,3'
     */
    function wrapperToString() {
      return (this.value() + '');
    }

    /**
     * Executes the chained sequence to extract the unwrapped value.
     *
     * @name value
     * @memberOf _
     * @alias run, toJSON, valueOf
     * @category Chain
     * @returns {*} Returns the resolved unwrapped value.
     * @example
     *
     * _([1, 2, 3]).value();
     * // => [1, 2, 3]
     */
    function wrapperValue() {
      return baseWrapperValue(this.__wrapped__, this.__actions__);
    }

    /*------------------------------------------------------------------------*/

    /**
     * Creates an array of elements corresponding to the given keys, or indexes,
     * of `collection`. Keys may be specified as individual arguments or as arrays
     * of keys.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {...(number|number[]|string|string[])} [props] The property names
     *  or indexes of elements to pick, specified individually or in arrays.
     * @returns {Array} Returns the new array of picked elements.
     * @example
     *
     * _.at(['a', 'b', 'c'], [0, 2]);
     * // => ['a', 'c']
     *
     * _.at(['barney', 'fred', 'pebbles'], 0, 2);
     * // => ['barney', 'pebbles']
     */
    var at = restParam(function(collection, props) {
      return baseAt(collection, baseFlatten(props));
    });

    /**
     * Creates an object composed of keys generated from the results of running
     * each element of `collection` through `iteratee`. The corresponding value
     * of each key is the number of times the key was returned by `iteratee`.
     * The `iteratee` is bound to `thisArg` and invoked with three arguments:
     * (value, index|key, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns the composed aggregate object.
     * @example
     *
     * _.countBy([4.3, 6.1, 6.4], function(n) {
     *   return Math.floor(n);
     * });
     * // => { '4': 1, '6': 2 }
     *
     * _.countBy([4.3, 6.1, 6.4], function(n) {
     *   return this.floor(n);
     * }, Math);
     * // => { '4': 1, '6': 2 }
     *
     * _.countBy(['one', 'two', 'three'], 'length');
     * // => { '3': 2, '5': 1 }
     */
    var countBy = createAggregator(function(result, value, key) {
      hasOwnProperty.call(result, key) ? ++result[key] : (result[key] = 1);
    });

    /**
     * Checks if `predicate` returns truthy for **all** elements of `collection`.
     * The predicate is bound to `thisArg` and invoked with three arguments:
     * (value, index|key, collection).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @alias all
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {boolean} Returns `true` if all elements pass the predicate check,
     *  else `false`.
     * @example
     *
     * _.every([true, 1, null, 'yes'], Boolean);
     * // => false
     *
     * var users = [
     *   { 'user': 'barney', 'active': false },
     *   { 'user': 'fred',   'active': false }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.every(users, { 'user': 'barney', 'active': false });
     * // => false
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.every(users, 'active', false);
     * // => true
     *
     * // using the `_.property` callback shorthand
     * _.every(users, 'active');
     * // => false
     */
    function every(collection, predicate, thisArg) {
      var func = isArray(collection) ? arrayEvery : baseEvery;
      if (thisArg && isIterateeCall(collection, predicate, thisArg)) {
        predicate = undefined;
      }
      if (typeof predicate != 'function' || thisArg !== undefined) {
        predicate = getCallback(predicate, thisArg, 3);
      }
      return func(collection, predicate);
    }

    /**
     * Iterates over elements of `collection`, returning an array of all elements
     * `predicate` returns truthy for. The predicate is bound to `thisArg` and
     * invoked with three arguments: (value, index|key, collection).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @alias select
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the new filtered array.
     * @example
     *
     * _.filter([4, 5, 6], function(n) {
     *   return n % 2 == 0;
     * });
     * // => [4, 6]
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36, 'active': true },
     *   { 'user': 'fred',   'age': 40, 'active': false }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.pluck(_.filter(users, { 'age': 36, 'active': true }), 'user');
     * // => ['barney']
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.pluck(_.filter(users, 'active', false), 'user');
     * // => ['fred']
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.filter(users, 'active'), 'user');
     * // => ['barney']
     */
    function filter(collection, predicate, thisArg) {
      var func = isArray(collection) ? arrayFilter : baseFilter;
      predicate = getCallback(predicate, thisArg, 3);
      return func(collection, predicate);
    }

    /**
     * Iterates over elements of `collection`, returning the first element
     * `predicate` returns truthy for. The predicate is bound to `thisArg` and
     * invoked with three arguments: (value, index|key, collection).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @alias detect
     * @category Collection
     * @param {Array|Object|string} collection The collection to search.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {*} Returns the matched element, else `undefined`.
     * @example
     *
     * var users = [
     *   { 'user': 'barney',  'age': 36, 'active': true },
     *   { 'user': 'fred',    'age': 40, 'active': false },
     *   { 'user': 'pebbles', 'age': 1,  'active': true }
     * ];
     *
     * _.result(_.find(users, function(chr) {
     *   return chr.age < 40;
     * }), 'user');
     * // => 'barney'
     *
     * // using the `_.matches` callback shorthand
     * _.result(_.find(users, { 'age': 1, 'active': true }), 'user');
     * // => 'pebbles'
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.result(_.find(users, 'active', false), 'user');
     * // => 'fred'
     *
     * // using the `_.property` callback shorthand
     * _.result(_.find(users, 'active'), 'user');
     * // => 'barney'
     */
    var find = createFind(baseEach);

    /**
     * This method is like `_.find` except that it iterates over elements of
     * `collection` from right to left.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to search.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {*} Returns the matched element, else `undefined`.
     * @example
     *
     * _.findLast([1, 2, 3, 4], function(n) {
     *   return n % 2 == 1;
     * });
     * // => 3
     */
    var findLast = createFind(baseEachRight, true);

    /**
     * Performs a deep comparison between each element in `collection` and the
     * source object, returning the first element that has equivalent property
     * values.
     *
     * **Note:** This method supports comparing arrays, booleans, `Date` objects,
     * numbers, `Object` objects, regexes, and strings. Objects are compared by
     * their own, not inherited, enumerable properties. For comparing a single
     * own or inherited property value see `_.matchesProperty`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to search.
     * @param {Object} source The object of property values to match.
     * @returns {*} Returns the matched element, else `undefined`.
     * @example
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36, 'active': true },
     *   { 'user': 'fred',   'age': 40, 'active': false }
     * ];
     *
     * _.result(_.findWhere(users, { 'age': 36, 'active': true }), 'user');
     * // => 'barney'
     *
     * _.result(_.findWhere(users, { 'age': 40, 'active': false }), 'user');
     * // => 'fred'
     */
    function findWhere(collection, source) {
      return find(collection, baseMatches(source));
    }

    /**
     * Iterates over elements of `collection` invoking `iteratee` for each element.
     * The `iteratee` is bound to `thisArg` and invoked with three arguments:
     * (value, index|key, collection). Iteratee functions may exit iteration early
     * by explicitly returning `false`.
     *
     * **Note:** As with other "Collections" methods, objects with a "length" property
     * are iterated like arrays. To avoid this behavior `_.forIn` or `_.forOwn`
     * may be used for object iteration.
     *
     * @static
     * @memberOf _
     * @alias each
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array|Object|string} Returns `collection`.
     * @example
     *
     * _([1, 2]).forEach(function(n) {
     *   console.log(n);
     * }).value();
     * // => logs each value from left to right and returns the array
     *
     * _.forEach({ 'a': 1, 'b': 2 }, function(n, key) {
     *   console.log(n, key);
     * });
     * // => logs each value-key pair and returns the object (iteration order is not guaranteed)
     */
    var forEach = createForEach(arrayEach, baseEach);

    /**
     * This method is like `_.forEach` except that it iterates over elements of
     * `collection` from right to left.
     *
     * @static
     * @memberOf _
     * @alias eachRight
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array|Object|string} Returns `collection`.
     * @example
     *
     * _([1, 2]).forEachRight(function(n) {
     *   console.log(n);
     * }).value();
     * // => logs each value from right to left and returns the array
     */
    var forEachRight = createForEach(arrayEachRight, baseEachRight);

    /**
     * Creates an object composed of keys generated from the results of running
     * each element of `collection` through `iteratee`. The corresponding value
     * of each key is an array of the elements responsible for generating the key.
     * The `iteratee` is bound to `thisArg` and invoked with three arguments:
     * (value, index|key, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns the composed aggregate object.
     * @example
     *
     * _.groupBy([4.2, 6.1, 6.4], function(n) {
     *   return Math.floor(n);
     * });
     * // => { '4': [4.2], '6': [6.1, 6.4] }
     *
     * _.groupBy([4.2, 6.1, 6.4], function(n) {
     *   return this.floor(n);
     * }, Math);
     * // => { '4': [4.2], '6': [6.1, 6.4] }
     *
     * // using the `_.property` callback shorthand
     * _.groupBy(['one', 'two', 'three'], 'length');
     * // => { '3': ['one', 'two'], '5': ['three'] }
     */
    var groupBy = createAggregator(function(result, value, key) {
      if (hasOwnProperty.call(result, key)) {
        result[key].push(value);
      } else {
        result[key] = [value];
      }
    });

    /**
     * Checks if `value` is in `collection` using
     * [`SameValueZero`](http://ecma-international.org/ecma-262/6.0/#sec-samevaluezero)
     * for equality comparisons. If `fromIndex` is negative, it is used as the offset
     * from the end of `collection`.
     *
     * @static
     * @memberOf _
     * @alias contains, include
     * @category Collection
     * @param {Array|Object|string} collection The collection to search.
     * @param {*} target The value to search for.
     * @param {number} [fromIndex=0] The index to search from.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.reduce`.
     * @returns {boolean} Returns `true` if a matching element is found, else `false`.
     * @example
     *
     * _.includes([1, 2, 3], 1);
     * // => true
     *
     * _.includes([1, 2, 3], 1, 2);
     * // => false
     *
     * _.includes({ 'user': 'fred', 'age': 40 }, 'fred');
     * // => true
     *
     * _.includes('pebbles', 'eb');
     * // => true
     */
    function includes(collection, target, fromIndex, guard) {
      var length = collection ? getLength(collection) : 0;
      if (!isLength(length)) {
        collection = values(collection);
        length = collection.length;
      }
      if (typeof fromIndex != 'number' || (guard && isIterateeCall(target, fromIndex, guard))) {
        fromIndex = 0;
      } else {
        fromIndex = fromIndex < 0 ? nativeMax(length + fromIndex, 0) : (fromIndex || 0);
      }
      return (typeof collection == 'string' || !isArray(collection) && isString(collection))
        ? (fromIndex <= length && collection.indexOf(target, fromIndex) > -1)
        : (!!length && getIndexOf(collection, target, fromIndex) > -1);
    }

    /**
     * Creates an object composed of keys generated from the results of running
     * each element of `collection` through `iteratee`. The corresponding value
     * of each key is the last element responsible for generating the key. The
     * iteratee function is bound to `thisArg` and invoked with three arguments:
     * (value, index|key, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns the composed aggregate object.
     * @example
     *
     * var keyData = [
     *   { 'dir': 'left', 'code': 97 },
     *   { 'dir': 'right', 'code': 100 }
     * ];
     *
     * _.indexBy(keyData, 'dir');
     * // => { 'left': { 'dir': 'left', 'code': 97 }, 'right': { 'dir': 'right', 'code': 100 } }
     *
     * _.indexBy(keyData, function(object) {
     *   return String.fromCharCode(object.code);
     * });
     * // => { 'a': { 'dir': 'left', 'code': 97 }, 'd': { 'dir': 'right', 'code': 100 } }
     *
     * _.indexBy(keyData, function(object) {
     *   return this.fromCharCode(object.code);
     * }, String);
     * // => { 'a': { 'dir': 'left', 'code': 97 }, 'd': { 'dir': 'right', 'code': 100 } }
     */
    var indexBy = createAggregator(function(result, value, key) {
      result[key] = value;
    });

    /**
     * Invokes the method at `path` of each element in `collection`, returning
     * an array of the results of each invoked method. Any additional arguments
     * are provided to each invoked method. If `methodName` is a function it is
     * invoked for, and `this` bound to, each element in `collection`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Array|Function|string} path The path of the method to invoke or
     *  the function invoked per iteration.
     * @param {...*} [args] The arguments to invoke the method with.
     * @returns {Array} Returns the array of results.
     * @example
     *
     * _.invoke([[5, 1, 7], [3, 2, 1]], 'sort');
     * // => [[1, 5, 7], [1, 2, 3]]
     *
     * _.invoke([123, 456], String.prototype.split, '');
     * // => [['1', '2', '3'], ['4', '5', '6']]
     */
    var invoke = restParam(function(collection, path, args) {
      var index = -1,
          isFunc = typeof path == 'function',
          isProp = isKey(path),
          result = isArrayLike(collection) ? Array(collection.length) : [];

      baseEach(collection, function(value) {
        var func = isFunc ? path : ((isProp && value != null) ? value[path] : undefined);
        result[++index] = func ? func.apply(value, args) : invokePath(value, path, args);
      });
      return result;
    });

    /**
     * Creates an array of values by running each element in `collection` through
     * `iteratee`. The `iteratee` is bound to `thisArg` and invoked with three
     * arguments: (value, index|key, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * Many lodash methods are guarded to work as iteratees for methods like
     * `_.every`, `_.filter`, `_.map`, `_.mapValues`, `_.reject`, and `_.some`.
     *
     * The guarded methods are:
     * `ary`, `callback`, `chunk`, `clone`, `create`, `curry`, `curryRight`,
     * `drop`, `dropRight`, `every`, `fill`, `flatten`, `invert`, `max`, `min`,
     * `parseInt`, `slice`, `sortBy`, `take`, `takeRight`, `template`, `trim`,
     * `trimLeft`, `trimRight`, `trunc`, `random`, `range`, `sample`, `some`,
     * `sum`, `uniq`, and `words`
     *
     * @static
     * @memberOf _
     * @alias collect
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array} Returns the new mapped array.
     * @example
     *
     * function timesThree(n) {
     *   return n * 3;
     * }
     *
     * _.map([1, 2], timesThree);
     * // => [3, 6]
     *
     * _.map({ 'a': 1, 'b': 2 }, timesThree);
     * // => [3, 6] (iteration order is not guaranteed)
     *
     * var users = [
     *   { 'user': 'barney' },
     *   { 'user': 'fred' }
     * ];
     *
     * // using the `_.property` callback shorthand
     * _.map(users, 'user');
     * // => ['barney', 'fred']
     */
    function map(collection, iteratee, thisArg) {
      var func = isArray(collection) ? arrayMap : baseMap;
      iteratee = getCallback(iteratee, thisArg, 3);
      return func(collection, iteratee);
    }

    /**
     * Creates an array of elements split into two groups, the first of which
     * contains elements `predicate` returns truthy for, while the second of which
     * contains elements `predicate` returns falsey for. The predicate is bound
     * to `thisArg` and invoked with three arguments: (value, index|key, collection).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the array of grouped elements.
     * @example
     *
     * _.partition([1, 2, 3], function(n) {
     *   return n % 2;
     * });
     * // => [[1, 3], [2]]
     *
     * _.partition([1.2, 2.3, 3.4], function(n) {
     *   return this.floor(n) % 2;
     * }, Math);
     * // => [[1.2, 3.4], [2.3]]
     *
     * var users = [
     *   { 'user': 'barney',  'age': 36, 'active': false },
     *   { 'user': 'fred',    'age': 40, 'active': true },
     *   { 'user': 'pebbles', 'age': 1,  'active': false }
     * ];
     *
     * var mapper = function(array) {
     *   return _.pluck(array, 'user');
     * };
     *
     * // using the `_.matches` callback shorthand
     * _.map(_.partition(users, { 'age': 1, 'active': false }), mapper);
     * // => [['pebbles'], ['barney', 'fred']]
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.map(_.partition(users, 'active', false), mapper);
     * // => [['barney', 'pebbles'], ['fred']]
     *
     * // using the `_.property` callback shorthand
     * _.map(_.partition(users, 'active'), mapper);
     * // => [['fred'], ['barney', 'pebbles']]
     */
    var partition = createAggregator(function(result, value, key) {
      result[key ? 0 : 1].push(value);
    }, function() { return [[], []]; });

    /**
     * Gets the property value of `path` from all elements in `collection`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Array|string} path The path of the property to pluck.
     * @returns {Array} Returns the property values.
     * @example
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36 },
     *   { 'user': 'fred',   'age': 40 }
     * ];
     *
     * _.pluck(users, 'user');
     * // => ['barney', 'fred']
     *
     * var userIndex = _.indexBy(users, 'user');
     * _.pluck(userIndex, 'age');
     * // => [36, 40] (iteration order is not guaranteed)
     */
    function pluck(collection, path) {
      return map(collection, property(path));
    }

    /**
     * Reduces `collection` to a value which is the accumulated result of running
     * each element in `collection` through `iteratee`, where each successive
     * invocation is supplied the return value of the previous. If `accumulator`
     * is not provided the first element of `collection` is used as the initial
     * value. The `iteratee` is bound to `thisArg` and invoked with four arguments:
     * (accumulator, value, index|key, collection).
     *
     * Many lodash methods are guarded to work as iteratees for methods like
     * `_.reduce`, `_.reduceRight`, and `_.transform`.
     *
     * The guarded methods are:
     * `assign`, `defaults`, `defaultsDeep`, `includes`, `merge`, `sortByAll`,
     * and `sortByOrder`
     *
     * @static
     * @memberOf _
     * @alias foldl, inject
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [accumulator] The initial value.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {*} Returns the accumulated value.
     * @example
     *
     * _.reduce([1, 2], function(total, n) {
     *   return total + n;
     * });
     * // => 3
     *
     * _.reduce({ 'a': 1, 'b': 2 }, function(result, n, key) {
     *   result[key] = n * 3;
     *   return result;
     * }, {});
     * // => { 'a': 3, 'b': 6 } (iteration order is not guaranteed)
     */
    var reduce = createReduce(arrayReduce, baseEach);

    /**
     * This method is like `_.reduce` except that it iterates over elements of
     * `collection` from right to left.
     *
     * @static
     * @memberOf _
     * @alias foldr
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [accumulator] The initial value.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {*} Returns the accumulated value.
     * @example
     *
     * var array = [[0, 1], [2, 3], [4, 5]];
     *
     * _.reduceRight(array, function(flattened, other) {
     *   return flattened.concat(other);
     * }, []);
     * // => [4, 5, 2, 3, 0, 1]
     */
    var reduceRight = createReduce(arrayReduceRight, baseEachRight);

    /**
     * The opposite of `_.filter`; this method returns the elements of `collection`
     * that `predicate` does **not** return truthy for.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Array} Returns the new filtered array.
     * @example
     *
     * _.reject([1, 2, 3, 4], function(n) {
     *   return n % 2 == 0;
     * });
     * // => [1, 3]
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36, 'active': false },
     *   { 'user': 'fred',   'age': 40, 'active': true }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.pluck(_.reject(users, { 'age': 40, 'active': true }), 'user');
     * // => ['barney']
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.pluck(_.reject(users, 'active', false), 'user');
     * // => ['fred']
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.reject(users, 'active'), 'user');
     * // => ['barney']
     */
    function reject(collection, predicate, thisArg) {
      var func = isArray(collection) ? arrayFilter : baseFilter;
      predicate = getCallback(predicate, thisArg, 3);
      return func(collection, function(value, index, collection) {
        return !predicate(value, index, collection);
      });
    }

    /**
     * Gets a random element or `n` random elements from a collection.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to sample.
     * @param {number} [n] The number of elements to sample.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {*} Returns the random sample(s).
     * @example
     *
     * _.sample([1, 2, 3, 4]);
     * // => 2
     *
     * _.sample([1, 2, 3, 4], 2);
     * // => [3, 1]
     */
    function sample(collection, n, guard) {
      if (guard ? isIterateeCall(collection, n, guard) : n == null) {
        collection = toIterable(collection);
        var length = collection.length;
        return length > 0 ? collection[baseRandom(0, length - 1)] : undefined;
      }
      var index = -1,
          result = toArray(collection),
          length = result.length,
          lastIndex = length - 1;

      n = nativeMin(n < 0 ? 0 : (+n || 0), length);
      while (++index < n) {
        var rand = baseRandom(index, lastIndex),
            value = result[rand];

        result[rand] = result[index];
        result[index] = value;
      }
      result.length = n;
      return result;
    }

    /**
     * Creates an array of shuffled values, using a version of the
     * [Fisher-Yates shuffle](https://en.wikipedia.org/wiki/Fisher-Yates_shuffle).
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to shuffle.
     * @returns {Array} Returns the new shuffled array.
     * @example
     *
     * _.shuffle([1, 2, 3, 4]);
     * // => [4, 1, 3, 2]
     */
    function shuffle(collection) {
      return sample(collection, POSITIVE_INFINITY);
    }

    /**
     * Gets the size of `collection` by returning its length for array-like
     * values or the number of own enumerable properties for objects.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to inspect.
     * @returns {number} Returns the size of `collection`.
     * @example
     *
     * _.size([1, 2, 3]);
     * // => 3
     *
     * _.size({ 'a': 1, 'b': 2 });
     * // => 2
     *
     * _.size('pebbles');
     * // => 7
     */
    function size(collection) {
      var length = collection ? getLength(collection) : 0;
      return isLength(length) ? length : keys(collection).length;
    }

    /**
     * Checks if `predicate` returns truthy for **any** element of `collection`.
     * The function returns as soon as it finds a passing value and does not iterate
     * over the entire collection. The predicate is bound to `thisArg` and invoked
     * with three arguments: (value, index|key, collection).
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @alias any
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {boolean} Returns `true` if any element passes the predicate check,
     *  else `false`.
     * @example
     *
     * _.some([null, 0, 'yes', false], Boolean);
     * // => true
     *
     * var users = [
     *   { 'user': 'barney', 'active': true },
     *   { 'user': 'fred',   'active': false }
     * ];
     *
     * // using the `_.matches` callback shorthand
     * _.some(users, { 'user': 'barney', 'active': false });
     * // => false
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.some(users, 'active', false);
     * // => true
     *
     * // using the `_.property` callback shorthand
     * _.some(users, 'active');
     * // => true
     */
    function some(collection, predicate, thisArg) {
      var func = isArray(collection) ? arraySome : baseSome;
      if (thisArg && isIterateeCall(collection, predicate, thisArg)) {
        predicate = undefined;
      }
      if (typeof predicate != 'function' || thisArg !== undefined) {
        predicate = getCallback(predicate, thisArg, 3);
      }
      return func(collection, predicate);
    }

    /**
     * Creates an array of elements, sorted in ascending order by the results of
     * running each element in a collection through `iteratee`. This method performs
     * a stable sort, that is, it preserves the original sort order of equal elements.
     * The `iteratee` is bound to `thisArg` and invoked with three arguments:
     * (value, index|key, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array} Returns the new sorted array.
     * @example
     *
     * _.sortBy([1, 2, 3], function(n) {
     *   return Math.sin(n);
     * });
     * // => [3, 1, 2]
     *
     * _.sortBy([1, 2, 3], function(n) {
     *   return this.sin(n);
     * }, Math);
     * // => [3, 1, 2]
     *
     * var users = [
     *   { 'user': 'fred' },
     *   { 'user': 'pebbles' },
     *   { 'user': 'barney' }
     * ];
     *
     * // using the `_.property` callback shorthand
     * _.pluck(_.sortBy(users, 'user'), 'user');
     * // => ['barney', 'fred', 'pebbles']
     */
    function sortBy(collection, iteratee, thisArg) {
      if (collection == null) {
        return [];
      }
      if (thisArg && isIterateeCall(collection, iteratee, thisArg)) {
        iteratee = undefined;
      }
      var index = -1;
      iteratee = getCallback(iteratee, thisArg, 3);

      var result = baseMap(collection, function(value, key, collection) {
        return { 'criteria': iteratee(value, key, collection), 'index': ++index, 'value': value };
      });
      return baseSortBy(result, compareAscending);
    }

    /**
     * This method is like `_.sortBy` except that it can sort by multiple iteratees
     * or property names.
     *
     * If a property name is provided for an iteratee the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If an object is provided for an iteratee the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {...(Function|Function[]|Object|Object[]|string|string[])} iteratees
     *  The iteratees to sort by, specified as individual values or arrays of values.
     * @returns {Array} Returns the new sorted array.
     * @example
     *
     * var users = [
     *   { 'user': 'fred',   'age': 48 },
     *   { 'user': 'barney', 'age': 36 },
     *   { 'user': 'fred',   'age': 42 },
     *   { 'user': 'barney', 'age': 34 }
     * ];
     *
     * _.map(_.sortByAll(users, ['user', 'age']), _.values);
     * // => [['barney', 34], ['barney', 36], ['fred', 42], ['fred', 48]]
     *
     * _.map(_.sortByAll(users, 'user', function(chr) {
     *   return Math.floor(chr.age / 10);
     * }), _.values);
     * // => [['barney', 36], ['barney', 34], ['fred', 48], ['fred', 42]]
     */
    var sortByAll = restParam(function(collection, iteratees) {
      if (collection == null) {
        return [];
      }
      var guard = iteratees[2];
      if (guard && isIterateeCall(iteratees[0], iteratees[1], guard)) {
        iteratees.length = 1;
      }
      return baseSortByOrder(collection, baseFlatten(iteratees), []);
    });

    /**
     * This method is like `_.sortByAll` except that it allows specifying the
     * sort orders of the iteratees to sort by. If `orders` is unspecified, all
     * values are sorted in ascending order. Otherwise, a value is sorted in
     * ascending order if its corresponding order is "asc", and descending if "desc".
     *
     * If a property name is provided for an iteratee the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If an object is provided for an iteratee the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function[]|Object[]|string[]} iteratees The iteratees to sort by.
     * @param {boolean[]} [orders] The sort orders of `iteratees`.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.reduce`.
     * @returns {Array} Returns the new sorted array.
     * @example
     *
     * var users = [
     *   { 'user': 'fred',   'age': 48 },
     *   { 'user': 'barney', 'age': 34 },
     *   { 'user': 'fred',   'age': 42 },
     *   { 'user': 'barney', 'age': 36 }
     * ];
     *
     * // sort by `user` in ascending order and by `age` in descending order
     * _.map(_.sortByOrder(users, ['user', 'age'], ['asc', 'desc']), _.values);
     * // => [['barney', 36], ['barney', 34], ['fred', 48], ['fred', 42]]
     */
    function sortByOrder(collection, iteratees, orders, guard) {
      if (collection == null) {
        return [];
      }
      if (guard && isIterateeCall(iteratees, orders, guard)) {
        orders = undefined;
      }
      if (!isArray(iteratees)) {
        iteratees = iteratees == null ? [] : [iteratees];
      }
      if (!isArray(orders)) {
        orders = orders == null ? [] : [orders];
      }
      return baseSortByOrder(collection, iteratees, orders);
    }

    /**
     * Performs a deep comparison between each element in `collection` and the
     * source object, returning an array of all elements that have equivalent
     * property values.
     *
     * **Note:** This method supports comparing arrays, booleans, `Date` objects,
     * numbers, `Object` objects, regexes, and strings. Objects are compared by
     * their own, not inherited, enumerable properties. For comparing a single
     * own or inherited property value see `_.matchesProperty`.
     *
     * @static
     * @memberOf _
     * @category Collection
     * @param {Array|Object|string} collection The collection to search.
     * @param {Object} source The object of property values to match.
     * @returns {Array} Returns the new filtered array.
     * @example
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36, 'active': false, 'pets': ['hoppy'] },
     *   { 'user': 'fred',   'age': 40, 'active': true, 'pets': ['baby puss', 'dino'] }
     * ];
     *
     * _.pluck(_.where(users, { 'age': 36, 'active': false }), 'user');
     * // => ['barney']
     *
     * _.pluck(_.where(users, { 'pets': ['dino'] }), 'user');
     * // => ['fred']
     */
    function where(collection, source) {
      return filter(collection, baseMatches(source));
    }

    /*------------------------------------------------------------------------*/

    /**
     * Gets the number of milliseconds that have elapsed since the Unix epoch
     * (1 January 1970 00:00:00 UTC).
     *
     * @static
     * @memberOf _
     * @category Date
     * @example
     *
     * _.defer(function(stamp) {
     *   console.log(_.now() - stamp);
     * }, _.now());
     * // => logs the number of milliseconds it took for the deferred function to be invoked
     */
    var now = nativeNow || function() {
      return new Date().getTime();
    };

    /*------------------------------------------------------------------------*/

    /**
     * The opposite of `_.before`; this method creates a function that invokes
     * `func` once it is called `n` or more times.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {number} n The number of calls before `func` is invoked.
     * @param {Function} func The function to restrict.
     * @returns {Function} Returns the new restricted function.
     * @example
     *
     * var saves = ['profile', 'settings'];
     *
     * var done = _.after(saves.length, function() {
     *   console.log('done saving!');
     * });
     *
     * _.forEach(saves, function(type) {
     *   asyncSave({ 'type': type, 'complete': done });
     * });
     * // => logs 'done saving!' after the two async saves have completed
     */
    function after(n, func) {
      if (typeof func != 'function') {
        if (typeof n == 'function') {
          var temp = n;
          n = func;
          func = temp;
        } else {
          throw new TypeError(FUNC_ERROR_TEXT);
        }
      }
      n = nativeIsFinite(n = +n) ? n : 0;
      return function() {
        if (--n < 1) {
          return func.apply(this, arguments);
        }
      };
    }

    /**
     * Creates a function that accepts up to `n` arguments ignoring any
     * additional arguments.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to cap arguments for.
     * @param {number} [n=func.length] The arity cap.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Function} Returns the new function.
     * @example
     *
     * _.map(['6', '8', '10'], _.ary(parseInt, 1));
     * // => [6, 8, 10]
     */
    function ary(func, n, guard) {
      if (guard && isIterateeCall(func, n, guard)) {
        n = undefined;
      }
      n = (func && n == null) ? func.length : nativeMax(+n || 0, 0);
      return createWrapper(func, ARY_FLAG, undefined, undefined, undefined, undefined, n);
    }

    /**
     * Creates a function that invokes `func`, with the `this` binding and arguments
     * of the created function, while it is called less than `n` times. Subsequent
     * calls to the created function return the result of the last `func` invocation.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {number} n The number of calls at which `func` is no longer invoked.
     * @param {Function} func The function to restrict.
     * @returns {Function} Returns the new restricted function.
     * @example
     *
     * jQuery('#add').on('click', _.before(5, addContactToList));
     * // => allows adding up to 4 contacts to the list
     */
    function before(n, func) {
      var result;
      if (typeof func != 'function') {
        if (typeof n == 'function') {
          var temp = n;
          n = func;
          func = temp;
        } else {
          throw new TypeError(FUNC_ERROR_TEXT);
        }
      }
      return function() {
        if (--n > 0) {
          result = func.apply(this, arguments);
        }
        if (n <= 1) {
          func = undefined;
        }
        return result;
      };
    }

    /**
     * Creates a function that invokes `func` with the `this` binding of `thisArg`
     * and prepends any additional `_.bind` arguments to those provided to the
     * bound function.
     *
     * The `_.bind.placeholder` value, which defaults to `_` in monolithic builds,
     * may be used as a placeholder for partially applied arguments.
     *
     * **Note:** Unlike native `Function#bind` this method does not set the "length"
     * property of bound functions.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to bind.
     * @param {*} thisArg The `this` binding of `func`.
     * @param {...*} [partials] The arguments to be partially applied.
     * @returns {Function} Returns the new bound function.
     * @example
     *
     * var greet = function(greeting, punctuation) {
     *   return greeting + ' ' + this.user + punctuation;
     * };
     *
     * var object = { 'user': 'fred' };
     *
     * var bound = _.bind(greet, object, 'hi');
     * bound('!');
     * // => 'hi fred!'
     *
     * // using placeholders
     * var bound = _.bind(greet, object, _, '!');
     * bound('hi');
     * // => 'hi fred!'
     */
    var bind = restParam(function(func, thisArg, partials) {
      var bitmask = BIND_FLAG;
      if (partials.length) {
        var holders = replaceHolders(partials, bind.placeholder);
        bitmask |= PARTIAL_FLAG;
      }
      return createWrapper(func, bitmask, thisArg, partials, holders);
    });

    /**
     * Binds methods of an object to the object itself, overwriting the existing
     * method. Method names may be specified as individual arguments or as arrays
     * of method names. If no method names are provided all enumerable function
     * properties, own and inherited, of `object` are bound.
     *
     * **Note:** This method does not set the "length" property of bound functions.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Object} object The object to bind and assign the bound methods to.
     * @param {...(string|string[])} [methodNames] The object method names to bind,
     *  specified as individual method names or arrays of method names.
     * @returns {Object} Returns `object`.
     * @example
     *
     * var view = {
     *   'label': 'docs',
     *   'onClick': function() {
     *     console.log('clicked ' + this.label);
     *   }
     * };
     *
     * _.bindAll(view);
     * jQuery('#docs').on('click', view.onClick);
     * // => logs 'clicked docs' when the element is clicked
     */
    var bindAll = restParam(function(object, methodNames) {
      methodNames = methodNames.length ? baseFlatten(methodNames) : functions(object);

      var index = -1,
          length = methodNames.length;

      while (++index < length) {
        var key = methodNames[index];
        object[key] = createWrapper(object[key], BIND_FLAG, object);
      }
      return object;
    });

    /**
     * Creates a function that invokes the method at `object[key]` and prepends
     * any additional `_.bindKey` arguments to those provided to the bound function.
     *
     * This method differs from `_.bind` by allowing bound functions to reference
     * methods that may be redefined or don't yet exist.
     * See [Peter Michaux's article](http://peter.michaux.ca/articles/lazy-function-definition-pattern)
     * for more details.
     *
     * The `_.bindKey.placeholder` value, which defaults to `_` in monolithic
     * builds, may be used as a placeholder for partially applied arguments.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Object} object The object the method belongs to.
     * @param {string} key The key of the method.
     * @param {...*} [partials] The arguments to be partially applied.
     * @returns {Function} Returns the new bound function.
     * @example
     *
     * var object = {
     *   'user': 'fred',
     *   'greet': function(greeting, punctuation) {
     *     return greeting + ' ' + this.user + punctuation;
     *   }
     * };
     *
     * var bound = _.bindKey(object, 'greet', 'hi');
     * bound('!');
     * // => 'hi fred!'
     *
     * object.greet = function(greeting, punctuation) {
     *   return greeting + 'ya ' + this.user + punctuation;
     * };
     *
     * bound('!');
     * // => 'hiya fred!'
     *
     * // using placeholders
     * var bound = _.bindKey(object, 'greet', _, '!');
     * bound('hi');
     * // => 'hiya fred!'
     */
    var bindKey = restParam(function(object, key, partials) {
      var bitmask = BIND_FLAG | BIND_KEY_FLAG;
      if (partials.length) {
        var holders = replaceHolders(partials, bindKey.placeholder);
        bitmask |= PARTIAL_FLAG;
      }
      return createWrapper(key, bitmask, object, partials, holders);
    });

    /**
     * Creates a function that accepts one or more arguments of `func` that when
     * called either invokes `func` returning its result, if all `func` arguments
     * have been provided, or returns a function that accepts one or more of the
     * remaining `func` arguments, and so on. The arity of `func` may be specified
     * if `func.length` is not sufficient.
     *
     * The `_.curry.placeholder` value, which defaults to `_` in monolithic builds,
     * may be used as a placeholder for provided arguments.
     *
     * **Note:** This method does not set the "length" property of curried functions.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to curry.
     * @param {number} [arity=func.length] The arity of `func`.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Function} Returns the new curried function.
     * @example
     *
     * var abc = function(a, b, c) {
     *   return [a, b, c];
     * };
     *
     * var curried = _.curry(abc);
     *
     * curried(1)(2)(3);
     * // => [1, 2, 3]
     *
     * curried(1, 2)(3);
     * // => [1, 2, 3]
     *
     * curried(1, 2, 3);
     * // => [1, 2, 3]
     *
     * // using placeholders
     * curried(1)(_, 3)(2);
     * // => [1, 2, 3]
     */
    var curry = createCurry(CURRY_FLAG);

    /**
     * This method is like `_.curry` except that arguments are applied to `func`
     * in the manner of `_.partialRight` instead of `_.partial`.
     *
     * The `_.curryRight.placeholder` value, which defaults to `_` in monolithic
     * builds, may be used as a placeholder for provided arguments.
     *
     * **Note:** This method does not set the "length" property of curried functions.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to curry.
     * @param {number} [arity=func.length] The arity of `func`.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Function} Returns the new curried function.
     * @example
     *
     * var abc = function(a, b, c) {
     *   return [a, b, c];
     * };
     *
     * var curried = _.curryRight(abc);
     *
     * curried(3)(2)(1);
     * // => [1, 2, 3]
     *
     * curried(2, 3)(1);
     * // => [1, 2, 3]
     *
     * curried(1, 2, 3);
     * // => [1, 2, 3]
     *
     * // using placeholders
     * curried(3)(1, _)(2);
     * // => [1, 2, 3]
     */
    var curryRight = createCurry(CURRY_RIGHT_FLAG);

    /**
     * Creates a debounced function that delays invoking `func` until after `wait`
     * milliseconds have elapsed since the last time the debounced function was
     * invoked. The debounced function comes with a `cancel` method to cancel
     * delayed invocations. Provide an options object to indicate that `func`
     * should be invoked on the leading and/or trailing edge of the `wait` timeout.
     * Subsequent calls to the debounced function return the result of the last
     * `func` invocation.
     *
     * **Note:** If `leading` and `trailing` options are `true`, `func` is invoked
     * on the trailing edge of the timeout only if the the debounced function is
     * invoked more than once during the `wait` timeout.
     *
     * See [David Corbacho's article](http://drupalmotion.com/article/debounce-and-throttle-visual-explanation)
     * for details over the differences between `_.debounce` and `_.throttle`.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to debounce.
     * @param {number} [wait=0] The number of milliseconds to delay.
     * @param {Object} [options] The options object.
     * @param {boolean} [options.leading=false] Specify invoking on the leading
     *  edge of the timeout.
     * @param {number} [options.maxWait] The maximum time `func` is allowed to be
     *  delayed before it is invoked.
     * @param {boolean} [options.trailing=true] Specify invoking on the trailing
     *  edge of the timeout.
     * @returns {Function} Returns the new debounced function.
     * @example
     *
     * // avoid costly calculations while the window size is in flux
     * jQuery(window).on('resize', _.debounce(calculateLayout, 150));
     *
     * // invoke `sendMail` when the click event is fired, debouncing subsequent calls
     * jQuery('#postbox').on('click', _.debounce(sendMail, 300, {
     *   'leading': true,
     *   'trailing': false
     * }));
     *
     * // ensure `batchLog` is invoked once after 1 second of debounced calls
     * var source = new EventSource('/stream');
     * jQuery(source).on('message', _.debounce(batchLog, 250, {
     *   'maxWait': 1000
     * }));
     *
     * // cancel a debounced call
     * var todoChanges = _.debounce(batchLog, 1000);
     * Object.observe(models.todo, todoChanges);
     *
     * Object.observe(models, function(changes) {
     *   if (_.find(changes, { 'user': 'todo', 'type': 'delete'})) {
     *     todoChanges.cancel();
     *   }
     * }, ['delete']);
     *
     * // ...at some point `models.todo` is changed
     * models.todo.completed = true;
     *
     * // ...before 1 second has passed `models.todo` is deleted
     * // which cancels the debounced `todoChanges` call
     * delete models.todo;
     */
    function debounce(func, wait, options) {
      var args,
          maxTimeoutId,
          result,
          stamp,
          thisArg,
          timeoutId,
          trailingCall,
          lastCalled = 0,
          maxWait = false,
          trailing = true;

      if (typeof func != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      wait = wait < 0 ? 0 : (+wait || 0);
      if (options === true) {
        var leading = true;
        trailing = false;
      } else if (isObject(options)) {
        leading = !!options.leading;
        maxWait = 'maxWait' in options && nativeMax(+options.maxWait || 0, wait);
        trailing = 'trailing' in options ? !!options.trailing : trailing;
      }

      function cancel() {
        if (timeoutId) {
          clearTimeout(timeoutId);
        }
        if (maxTimeoutId) {
          clearTimeout(maxTimeoutId);
        }
        lastCalled = 0;
        maxTimeoutId = timeoutId = trailingCall = undefined;
      }

      function complete(isCalled, id) {
        if (id) {
          clearTimeout(id);
        }
        maxTimeoutId = timeoutId = trailingCall = undefined;
        if (isCalled) {
          lastCalled = now();
          result = func.apply(thisArg, args);
          if (!timeoutId && !maxTimeoutId) {
            args = thisArg = undefined;
          }
        }
      }

      function delayed() {
        var remaining = wait - (now() - stamp);
        if (remaining <= 0 || remaining > wait) {
          complete(trailingCall, maxTimeoutId);
        } else {
          timeoutId = setTimeout(delayed, remaining);
        }
      }

      function maxDelayed() {
        complete(trailing, timeoutId);
      }

      function debounced() {
        args = arguments;
        stamp = now();
        thisArg = this;
        trailingCall = trailing && (timeoutId || !leading);

        if (maxWait === false) {
          var leadingCall = leading && !timeoutId;
        } else {
          if (!maxTimeoutId && !leading) {
            lastCalled = stamp;
          }
          var remaining = maxWait - (stamp - lastCalled),
              isCalled = remaining <= 0 || remaining > maxWait;

          if (isCalled) {
            if (maxTimeoutId) {
              maxTimeoutId = clearTimeout(maxTimeoutId);
            }
            lastCalled = stamp;
            result = func.apply(thisArg, args);
          }
          else if (!maxTimeoutId) {
            maxTimeoutId = setTimeout(maxDelayed, remaining);
          }
        }
        if (isCalled && timeoutId) {
          timeoutId = clearTimeout(timeoutId);
        }
        else if (!timeoutId && wait !== maxWait) {
          timeoutId = setTimeout(delayed, wait);
        }
        if (leadingCall) {
          isCalled = true;
          result = func.apply(thisArg, args);
        }
        if (isCalled && !timeoutId && !maxTimeoutId) {
          args = thisArg = undefined;
        }
        return result;
      }
      debounced.cancel = cancel;
      return debounced;
    }

    /**
     * Defers invoking the `func` until the current call stack has cleared. Any
     * additional arguments are provided to `func` when it is invoked.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to defer.
     * @param {...*} [args] The arguments to invoke the function with.
     * @returns {number} Returns the timer id.
     * @example
     *
     * _.defer(function(text) {
     *   console.log(text);
     * }, 'deferred');
     * // logs 'deferred' after one or more milliseconds
     */
    var defer = restParam(function(func, args) {
      return baseDelay(func, 1, args);
    });

    /**
     * Invokes `func` after `wait` milliseconds. Any additional arguments are
     * provided to `func` when it is invoked.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to delay.
     * @param {number} wait The number of milliseconds to delay invocation.
     * @param {...*} [args] The arguments to invoke the function with.
     * @returns {number} Returns the timer id.
     * @example
     *
     * _.delay(function(text) {
     *   console.log(text);
     * }, 1000, 'later');
     * // => logs 'later' after one second
     */
    var delay = restParam(function(func, wait, args) {
      return baseDelay(func, wait, args);
    });

    /**
     * Creates a function that returns the result of invoking the provided
     * functions with the `this` binding of the created function, where each
     * successive invocation is supplied the return value of the previous.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {...Function} [funcs] Functions to invoke.
     * @returns {Function} Returns the new function.
     * @example
     *
     * function square(n) {
     *   return n * n;
     * }
     *
     * var addSquare = _.flow(_.add, square);
     * addSquare(1, 2);
     * // => 9
     */
    var flow = createFlow();

    /**
     * This method is like `_.flow` except that it creates a function that
     * invokes the provided functions from right to left.
     *
     * @static
     * @memberOf _
     * @alias backflow, compose
     * @category Function
     * @param {...Function} [funcs] Functions to invoke.
     * @returns {Function} Returns the new function.
     * @example
     *
     * function square(n) {
     *   return n * n;
     * }
     *
     * var addSquare = _.flowRight(square, _.add);
     * addSquare(1, 2);
     * // => 9
     */
    var flowRight = createFlow(true);

    /**
     * Creates a function that memoizes the result of `func`. If `resolver` is
     * provided it determines the cache key for storing the result based on the
     * arguments provided to the memoized function. By default, the first argument
     * provided to the memoized function is coerced to a string and used as the
     * cache key. The `func` is invoked with the `this` binding of the memoized
     * function.
     *
     * **Note:** The cache is exposed as the `cache` property on the memoized
     * function. Its creation may be customized by replacing the `_.memoize.Cache`
     * constructor with one whose instances implement the [`Map`](http://ecma-international.org/ecma-262/6.0/#sec-properties-of-the-map-prototype-object)
     * method interface of `get`, `has`, and `set`.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to have its output memoized.
     * @param {Function} [resolver] The function to resolve the cache key.
     * @returns {Function} Returns the new memoizing function.
     * @example
     *
     * var upperCase = _.memoize(function(string) {
     *   return string.toUpperCase();
     * });
     *
     * upperCase('fred');
     * // => 'FRED'
     *
     * // modifying the result cache
     * upperCase.cache.set('fred', 'BARNEY');
     * upperCase('fred');
     * // => 'BARNEY'
     *
     * // replacing `_.memoize.Cache`
     * var object = { 'user': 'fred' };
     * var other = { 'user': 'barney' };
     * var identity = _.memoize(_.identity);
     *
     * identity(object);
     * // => { 'user': 'fred' }
     * identity(other);
     * // => { 'user': 'fred' }
     *
     * _.memoize.Cache = WeakMap;
     * var identity = _.memoize(_.identity);
     *
     * identity(object);
     * // => { 'user': 'fred' }
     * identity(other);
     * // => { 'user': 'barney' }
     */
    function memoize(func, resolver) {
      if (typeof func != 'function' || (resolver && typeof resolver != 'function')) {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      var memoized = function() {
        var args = arguments,
            key = resolver ? resolver.apply(this, args) : args[0],
            cache = memoized.cache;

        if (cache.has(key)) {
          return cache.get(key);
        }
        var result = func.apply(this, args);
        memoized.cache = cache.set(key, result);
        return result;
      };
      memoized.cache = new memoize.Cache;
      return memoized;
    }

    /**
     * Creates a function that runs each argument through a corresponding
     * transform function.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to wrap.
     * @param {...(Function|Function[])} [transforms] The functions to transform
     * arguments, specified as individual functions or arrays of functions.
     * @returns {Function} Returns the new function.
     * @example
     *
     * function doubled(n) {
     *   return n * 2;
     * }
     *
     * function square(n) {
     *   return n * n;
     * }
     *
     * var modded = _.modArgs(function(x, y) {
     *   return [x, y];
     * }, square, doubled);
     *
     * modded(1, 2);
     * // => [1, 4]
     *
     * modded(5, 10);
     * // => [25, 20]
     */
    var modArgs = restParam(function(func, transforms) {
      transforms = baseFlatten(transforms);
      if (typeof func != 'function' || !arrayEvery(transforms, baseIsFunction)) {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      var length = transforms.length;
      return restParam(function(args) {
        var index = nativeMin(args.length, length);
        while (index--) {
          args[index] = transforms[index](args[index]);
        }
        return func.apply(this, args);
      });
    });

    /**
     * Creates a function that negates the result of the predicate `func`. The
     * `func` predicate is invoked with the `this` binding and arguments of the
     * created function.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} predicate The predicate to negate.
     * @returns {Function} Returns the new function.
     * @example
     *
     * function isEven(n) {
     *   return n % 2 == 0;
     * }
     *
     * _.filter([1, 2, 3, 4, 5, 6], _.negate(isEven));
     * // => [1, 3, 5]
     */
    function negate(predicate) {
      if (typeof predicate != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      return function() {
        return !predicate.apply(this, arguments);
      };
    }

    /**
     * Creates a function that is restricted to invoking `func` once. Repeat calls
     * to the function return the value of the first call. The `func` is invoked
     * with the `this` binding and arguments of the created function.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to restrict.
     * @returns {Function} Returns the new restricted function.
     * @example
     *
     * var initialize = _.once(createApplication);
     * initialize();
     * initialize();
     * // `initialize` invokes `createApplication` once
     */
    function once(func) {
      return before(2, func);
    }

    /**
     * Creates a function that invokes `func` with `partial` arguments prepended
     * to those provided to the new function. This method is like `_.bind` except
     * it does **not** alter the `this` binding.
     *
     * The `_.partial.placeholder` value, which defaults to `_` in monolithic
     * builds, may be used as a placeholder for partially applied arguments.
     *
     * **Note:** This method does not set the "length" property of partially
     * applied functions.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to partially apply arguments to.
     * @param {...*} [partials] The arguments to be partially applied.
     * @returns {Function} Returns the new partially applied function.
     * @example
     *
     * var greet = function(greeting, name) {
     *   return greeting + ' ' + name;
     * };
     *
     * var sayHelloTo = _.partial(greet, 'hello');
     * sayHelloTo('fred');
     * // => 'hello fred'
     *
     * // using placeholders
     * var greetFred = _.partial(greet, _, 'fred');
     * greetFred('hi');
     * // => 'hi fred'
     */
    var partial = createPartial(PARTIAL_FLAG);

    /**
     * This method is like `_.partial` except that partially applied arguments
     * are appended to those provided to the new function.
     *
     * The `_.partialRight.placeholder` value, which defaults to `_` in monolithic
     * builds, may be used as a placeholder for partially applied arguments.
     *
     * **Note:** This method does not set the "length" property of partially
     * applied functions.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to partially apply arguments to.
     * @param {...*} [partials] The arguments to be partially applied.
     * @returns {Function} Returns the new partially applied function.
     * @example
     *
     * var greet = function(greeting, name) {
     *   return greeting + ' ' + name;
     * };
     *
     * var greetFred = _.partialRight(greet, 'fred');
     * greetFred('hi');
     * // => 'hi fred'
     *
     * // using placeholders
     * var sayHelloTo = _.partialRight(greet, 'hello', _);
     * sayHelloTo('fred');
     * // => 'hello fred'
     */
    var partialRight = createPartial(PARTIAL_RIGHT_FLAG);

    /**
     * Creates a function that invokes `func` with arguments arranged according
     * to the specified indexes where the argument value at the first index is
     * provided as the first argument, the argument value at the second index is
     * provided as the second argument, and so on.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to rearrange arguments for.
     * @param {...(number|number[])} indexes The arranged argument indexes,
     *  specified as individual indexes or arrays of indexes.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var rearged = _.rearg(function(a, b, c) {
     *   return [a, b, c];
     * }, 2, 0, 1);
     *
     * rearged('b', 'c', 'a')
     * // => ['a', 'b', 'c']
     *
     * var map = _.rearg(_.map, [1, 0]);
     * map(function(n) {
     *   return n * 3;
     * }, [1, 2, 3]);
     * // => [3, 6, 9]
     */
    var rearg = restParam(function(func, indexes) {
      return createWrapper(func, REARG_FLAG, undefined, undefined, undefined, baseFlatten(indexes));
    });

    /**
     * Creates a function that invokes `func` with the `this` binding of the
     * created function and arguments from `start` and beyond provided as an array.
     *
     * **Note:** This method is based on the [rest parameter](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Functions/rest_parameters).
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to apply a rest parameter to.
     * @param {number} [start=func.length-1] The start position of the rest parameter.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var say = _.restParam(function(what, names) {
     *   return what + ' ' + _.initial(names).join(', ') +
     *     (_.size(names) > 1 ? ', & ' : '') + _.last(names);
     * });
     *
     * say('hello', 'fred', 'barney', 'pebbles');
     * // => 'hello fred, barney, & pebbles'
     */
    function restParam(func, start) {
      if (typeof func != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      start = nativeMax(start === undefined ? (func.length - 1) : (+start || 0), 0);
      return function() {
        var args = arguments,
            index = -1,
            length = nativeMax(args.length - start, 0),
            rest = Array(length);

        while (++index < length) {
          rest[index] = args[start + index];
        }
        switch (start) {
          case 0: return func.call(this, rest);
          case 1: return func.call(this, args[0], rest);
          case 2: return func.call(this, args[0], args[1], rest);
        }
        var otherArgs = Array(start + 1);
        index = -1;
        while (++index < start) {
          otherArgs[index] = args[index];
        }
        otherArgs[start] = rest;
        return func.apply(this, otherArgs);
      };
    }

    /**
     * Creates a function that invokes `func` with the `this` binding of the created
     * function and an array of arguments much like [`Function#apply`](https://es5.github.io/#x15.3.4.3).
     *
     * **Note:** This method is based on the [spread operator](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Spread_operator).
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to spread arguments over.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var say = _.spread(function(who, what) {
     *   return who + ' says ' + what;
     * });
     *
     * say(['fred', 'hello']);
     * // => 'fred says hello'
     *
     * // with a Promise
     * var numbers = Promise.all([
     *   Promise.resolve(40),
     *   Promise.resolve(36)
     * ]);
     *
     * numbers.then(_.spread(function(x, y) {
     *   return x + y;
     * }));
     * // => a Promise of 76
     */
    function spread(func) {
      if (typeof func != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      return function(array) {
        return func.apply(this, array);
      };
    }

    /**
     * Creates a throttled function that only invokes `func` at most once per
     * every `wait` milliseconds. The throttled function comes with a `cancel`
     * method to cancel delayed invocations. Provide an options object to indicate
     * that `func` should be invoked on the leading and/or trailing edge of the
     * `wait` timeout. Subsequent calls to the throttled function return the
     * result of the last `func` call.
     *
     * **Note:** If `leading` and `trailing` options are `true`, `func` is invoked
     * on the trailing edge of the timeout only if the the throttled function is
     * invoked more than once during the `wait` timeout.
     *
     * See [David Corbacho's article](http://drupalmotion.com/article/debounce-and-throttle-visual-explanation)
     * for details over the differences between `_.throttle` and `_.debounce`.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {Function} func The function to throttle.
     * @param {number} [wait=0] The number of milliseconds to throttle invocations to.
     * @param {Object} [options] The options object.
     * @param {boolean} [options.leading=true] Specify invoking on the leading
     *  edge of the timeout.
     * @param {boolean} [options.trailing=true] Specify invoking on the trailing
     *  edge of the timeout.
     * @returns {Function} Returns the new throttled function.
     * @example
     *
     * // avoid excessively updating the position while scrolling
     * jQuery(window).on('scroll', _.throttle(updatePosition, 100));
     *
     * // invoke `renewToken` when the click event is fired, but not more than once every 5 minutes
     * jQuery('.interactive').on('click', _.throttle(renewToken, 300000, {
     *   'trailing': false
     * }));
     *
     * // cancel a trailing throttled call
     * jQuery(window).on('popstate', throttled.cancel);
     */
    function throttle(func, wait, options) {
      var leading = true,
          trailing = true;

      if (typeof func != 'function') {
        throw new TypeError(FUNC_ERROR_TEXT);
      }
      if (options === false) {
        leading = false;
      } else if (isObject(options)) {
        leading = 'leading' in options ? !!options.leading : leading;
        trailing = 'trailing' in options ? !!options.trailing : trailing;
      }
      return debounce(func, wait, { 'leading': leading, 'maxWait': +wait, 'trailing': trailing });
    }

    /**
     * Creates a function that provides `value` to the wrapper function as its
     * first argument. Any additional arguments provided to the function are
     * appended to those provided to the wrapper function. The wrapper is invoked
     * with the `this` binding of the created function.
     *
     * @static
     * @memberOf _
     * @category Function
     * @param {*} value The value to wrap.
     * @param {Function} wrapper The wrapper function.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var p = _.wrap(_.escape, function(func, text) {
     *   return '<p>' + func(text) + '</p>';
     * });
     *
     * p('fred, barney, & pebbles');
     * // => '<p>fred, barney, &amp; pebbles</p>'
     */
    function wrap(value, wrapper) {
      wrapper = wrapper == null ? identity : wrapper;
      return createWrapper(wrapper, PARTIAL_FLAG, undefined, [value], []);
    }

    /*------------------------------------------------------------------------*/

    /**
     * Creates a clone of `value`. If `isDeep` is `true` nested objects are cloned,
     * otherwise they are assigned by reference. If `customizer` is provided it is
     * invoked to produce the cloned values. If `customizer` returns `undefined`
     * cloning is handled by the method instead. The `customizer` is bound to
     * `thisArg` and invoked with two argument; (value [, index|key, object]).
     *
     * **Note:** This method is loosely based on the
     * [structured clone algorithm](http://www.w3.org/TR/html5/infrastructure.html#internal-structured-cloning-algorithm).
     * The enumerable properties of `arguments` objects and objects created by
     * constructors other than `Object` are cloned to plain `Object` objects. An
     * empty object is returned for uncloneable values such as functions, DOM nodes,
     * Maps, Sets, and WeakMaps.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to clone.
     * @param {boolean} [isDeep] Specify a deep clone.
     * @param {Function} [customizer] The function to customize cloning values.
     * @param {*} [thisArg] The `this` binding of `customizer`.
     * @returns {*} Returns the cloned value.
     * @example
     *
     * var users = [
     *   { 'user': 'barney' },
     *   { 'user': 'fred' }
     * ];
     *
     * var shallow = _.clone(users);
     * shallow[0] === users[0];
     * // => true
     *
     * var deep = _.clone(users, true);
     * deep[0] === users[0];
     * // => false
     *
     * // using a customizer callback
     * var el = _.clone(document.body, function(value) {
     *   if (_.isElement(value)) {
     *     return value.cloneNode(false);
     *   }
     * });
     *
     * el === document.body
     * // => false
     * el.nodeName
     * // => BODY
     * el.childNodes.length;
     * // => 0
     */
    function clone(value, isDeep, customizer, thisArg) {
      if (isDeep && typeof isDeep != 'boolean' && isIterateeCall(value, isDeep, customizer)) {
        isDeep = false;
      }
      else if (typeof isDeep == 'function') {
        thisArg = customizer;
        customizer = isDeep;
        isDeep = false;
      }
      return typeof customizer == 'function'
        ? baseClone(value, isDeep, bindCallback(customizer, thisArg, 1))
        : baseClone(value, isDeep);
    }

    /**
     * Creates a deep clone of `value`. If `customizer` is provided it is invoked
     * to produce the cloned values. If `customizer` returns `undefined` cloning
     * is handled by the method instead. The `customizer` is bound to `thisArg`
     * and invoked with two argument; (value [, index|key, object]).
     *
     * **Note:** This method is loosely based on the
     * [structured clone algorithm](http://www.w3.org/TR/html5/infrastructure.html#internal-structured-cloning-algorithm).
     * The enumerable properties of `arguments` objects and objects created by
     * constructors other than `Object` are cloned to plain `Object` objects. An
     * empty object is returned for uncloneable values such as functions, DOM nodes,
     * Maps, Sets, and WeakMaps.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to deep clone.
     * @param {Function} [customizer] The function to customize cloning values.
     * @param {*} [thisArg] The `this` binding of `customizer`.
     * @returns {*} Returns the deep cloned value.
     * @example
     *
     * var users = [
     *   { 'user': 'barney' },
     *   { 'user': 'fred' }
     * ];
     *
     * var deep = _.cloneDeep(users);
     * deep[0] === users[0];
     * // => false
     *
     * // using a customizer callback
     * var el = _.cloneDeep(document.body, function(value) {
     *   if (_.isElement(value)) {
     *     return value.cloneNode(true);
     *   }
     * });
     *
     * el === document.body
     * // => false
     * el.nodeName
     * // => BODY
     * el.childNodes.length;
     * // => 20
     */
    function cloneDeep(value, customizer, thisArg) {
      return typeof customizer == 'function'
        ? baseClone(value, true, bindCallback(customizer, thisArg, 1))
        : baseClone(value, true);
    }

    /**
     * Checks if `value` is greater than `other`.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to compare.
     * @param {*} other The other value to compare.
     * @returns {boolean} Returns `true` if `value` is greater than `other`, else `false`.
     * @example
     *
     * _.gt(3, 1);
     * // => true
     *
     * _.gt(3, 3);
     * // => false
     *
     * _.gt(1, 3);
     * // => false
     */
    function gt(value, other) {
      return value > other;
    }

    /**
     * Checks if `value` is greater than or equal to `other`.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to compare.
     * @param {*} other The other value to compare.
     * @returns {boolean} Returns `true` if `value` is greater than or equal to `other`, else `false`.
     * @example
     *
     * _.gte(3, 1);
     * // => true
     *
     * _.gte(3, 3);
     * // => true
     *
     * _.gte(1, 3);
     * // => false
     */
    function gte(value, other) {
      return value >= other;
    }

    /**
     * Checks if `value` is classified as an `arguments` object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isArguments(function() { return arguments; }());
     * // => true
     *
     * _.isArguments([1, 2, 3]);
     * // => false
     */
    function isArguments(value) {
      return isObjectLike(value) && isArrayLike(value) &&
        hasOwnProperty.call(value, 'callee') && !propertyIsEnumerable.call(value, 'callee');
    }

    /**
     * Checks if `value` is classified as an `Array` object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isArray([1, 2, 3]);
     * // => true
     *
     * _.isArray(function() { return arguments; }());
     * // => false
     */
    var isArray = nativeIsArray || function(value) {
      return isObjectLike(value) && isLength(value.length) && objToString.call(value) == arrayTag;
    };

    /**
     * Checks if `value` is classified as a boolean primitive or object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isBoolean(false);
     * // => true
     *
     * _.isBoolean(null);
     * // => false
     */
    function isBoolean(value) {
      return value === true || value === false || (isObjectLike(value) && objToString.call(value) == boolTag);
    }

    /**
     * Checks if `value` is classified as a `Date` object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isDate(new Date);
     * // => true
     *
     * _.isDate('Mon April 23 2012');
     * // => false
     */
    function isDate(value) {
      return isObjectLike(value) && objToString.call(value) == dateTag;
    }

    /**
     * Checks if `value` is a DOM element.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is a DOM element, else `false`.
     * @example
     *
     * _.isElement(document.body);
     * // => true
     *
     * _.isElement('<body>');
     * // => false
     */
    function isElement(value) {
      return !!value && value.nodeType === 1 && isObjectLike(value) && !isPlainObject(value);
    }

    /**
     * Checks if `value` is empty. A value is considered empty unless it is an
     * `arguments` object, array, string, or jQuery-like collection with a length
     * greater than `0` or an object with own enumerable properties.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {Array|Object|string} value The value to inspect.
     * @returns {boolean} Returns `true` if `value` is empty, else `false`.
     * @example
     *
     * _.isEmpty(null);
     * // => true
     *
     * _.isEmpty(true);
     * // => true
     *
     * _.isEmpty(1);
     * // => true
     *
     * _.isEmpty([1, 2, 3]);
     * // => false
     *
     * _.isEmpty({ 'a': 1 });
     * // => false
     */
    function isEmpty(value) {
      if (value == null) {
        return true;
      }
      if (isArrayLike(value) && (isArray(value) || isString(value) || isArguments(value) ||
          (isObjectLike(value) && isFunction(value.splice)))) {
        return !value.length;
      }
      return !keys(value).length;
    }

    /**
     * Performs a deep comparison between two values to determine if they are
     * equivalent. If `customizer` is provided it is invoked to compare values.
     * If `customizer` returns `undefined` comparisons are handled by the method
     * instead. The `customizer` is bound to `thisArg` and invoked with three
     * arguments: (value, other [, index|key]).
     *
     * **Note:** This method supports comparing arrays, booleans, `Date` objects,
     * numbers, `Object` objects, regexes, and strings. Objects are compared by
     * their own, not inherited, enumerable properties. Functions and DOM nodes
     * are **not** supported. Provide a customizer function to extend support
     * for comparing other values.
     *
     * @static
     * @memberOf _
     * @alias eq
     * @category Lang
     * @param {*} value The value to compare.
     * @param {*} other The other value to compare.
     * @param {Function} [customizer] The function to customize value comparisons.
     * @param {*} [thisArg] The `this` binding of `customizer`.
     * @returns {boolean} Returns `true` if the values are equivalent, else `false`.
     * @example
     *
     * var object = { 'user': 'fred' };
     * var other = { 'user': 'fred' };
     *
     * object == other;
     * // => false
     *
     * _.isEqual(object, other);
     * // => true
     *
     * // using a customizer callback
     * var array = ['hello', 'goodbye'];
     * var other = ['hi', 'goodbye'];
     *
     * _.isEqual(array, other, function(value, other) {
     *   if (_.every([value, other], RegExp.prototype.test, /^h(?:i|ello)$/)) {
     *     return true;
     *   }
     * });
     * // => true
     */
    function isEqual(value, other, customizer, thisArg) {
      customizer = typeof customizer == 'function' ? bindCallback(customizer, thisArg, 3) : undefined;
      var result = customizer ? customizer(value, other) : undefined;
      return  result === undefined ? baseIsEqual(value, other, customizer) : !!result;
    }

    /**
     * Checks if `value` is an `Error`, `EvalError`, `RangeError`, `ReferenceError`,
     * `SyntaxError`, `TypeError`, or `URIError` object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is an error object, else `false`.
     * @example
     *
     * _.isError(new Error);
     * // => true
     *
     * _.isError(Error);
     * // => false
     */
    function isError(value) {
      return isObjectLike(value) && typeof value.message == 'string' && objToString.call(value) == errorTag;
    }

    /**
     * Checks if `value` is a finite primitive number.
     *
     * **Note:** This method is based on [`Number.isFinite`](http://ecma-international.org/ecma-262/6.0/#sec-number.isfinite).
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is a finite number, else `false`.
     * @example
     *
     * _.isFinite(10);
     * // => true
     *
     * _.isFinite('10');
     * // => false
     *
     * _.isFinite(true);
     * // => false
     *
     * _.isFinite(Object(10));
     * // => false
     *
     * _.isFinite(Infinity);
     * // => false
     */
    function isFinite(value) {
      return typeof value == 'number' && nativeIsFinite(value);
    }

    /**
     * Checks if `value` is classified as a `Function` object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isFunction(_);
     * // => true
     *
     * _.isFunction(/abc/);
     * // => false
     */
    function isFunction(value) {
      // The use of `Object#toString` avoids issues with the `typeof` operator
      // in older versions of Chrome and Safari which return 'function' for regexes
      // and Safari 8 equivalents which return 'object' for typed array constructors.
      return isObject(value) && objToString.call(value) == funcTag;
    }

    /**
     * Checks if `value` is the [language type](https://es5.github.io/#x8) of `Object`.
     * (e.g. arrays, functions, objects, regexes, `new Number(0)`, and `new String('')`)
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is an object, else `false`.
     * @example
     *
     * _.isObject({});
     * // => true
     *
     * _.isObject([1, 2, 3]);
     * // => true
     *
     * _.isObject(1);
     * // => false
     */
    function isObject(value) {
      // Avoid a V8 JIT bug in Chrome 19-20.
      // See https://code.google.com/p/v8/issues/detail?id=2291 for more details.
      var type = typeof value;
      return !!value && (type == 'object' || type == 'function');
    }

    /**
     * Performs a deep comparison between `object` and `source` to determine if
     * `object` contains equivalent property values. If `customizer` is provided
     * it is invoked to compare values. If `customizer` returns `undefined`
     * comparisons are handled by the method instead. The `customizer` is bound
     * to `thisArg` and invoked with three arguments: (value, other, index|key).
     *
     * **Note:** This method supports comparing properties of arrays, booleans,
     * `Date` objects, numbers, `Object` objects, regexes, and strings. Functions
     * and DOM nodes are **not** supported. Provide a customizer function to extend
     * support for comparing other values.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {Object} object The object to inspect.
     * @param {Object} source The object of property values to match.
     * @param {Function} [customizer] The function to customize value comparisons.
     * @param {*} [thisArg] The `this` binding of `customizer`.
     * @returns {boolean} Returns `true` if `object` is a match, else `false`.
     * @example
     *
     * var object = { 'user': 'fred', 'age': 40 };
     *
     * _.isMatch(object, { 'age': 40 });
     * // => true
     *
     * _.isMatch(object, { 'age': 36 });
     * // => false
     *
     * // using a customizer callback
     * var object = { 'greeting': 'hello' };
     * var source = { 'greeting': 'hi' };
     *
     * _.isMatch(object, source, function(value, other) {
     *   return _.every([value, other], RegExp.prototype.test, /^h(?:i|ello)$/) || undefined;
     * });
     * // => true
     */
    function isMatch(object, source, customizer, thisArg) {
      customizer = typeof customizer == 'function' ? bindCallback(customizer, thisArg, 3) : undefined;
      return baseIsMatch(object, getMatchData(source), customizer);
    }

    /**
     * Checks if `value` is `NaN`.
     *
     * **Note:** This method is not the same as [`isNaN`](https://es5.github.io/#x15.1.2.4)
     * which returns `true` for `undefined` and other non-numeric values.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is `NaN`, else `false`.
     * @example
     *
     * _.isNaN(NaN);
     * // => true
     *
     * _.isNaN(new Number(NaN));
     * // => true
     *
     * isNaN(undefined);
     * // => true
     *
     * _.isNaN(undefined);
     * // => false
     */
    function isNaN(value) {
      // An `NaN` primitive is the only value that is not equal to itself.
      // Perform the `toStringTag` check first to avoid errors with some host objects in IE.
      return isNumber(value) && value != +value;
    }

    /**
     * Checks if `value` is a native function.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is a native function, else `false`.
     * @example
     *
     * _.isNative(Array.prototype.push);
     * // => true
     *
     * _.isNative(_);
     * // => false
     */
    function isNative(value) {
      if (value == null) {
        return false;
      }
      if (isFunction(value)) {
        return reIsNative.test(fnToString.call(value));
      }
      return isObjectLike(value) && reIsHostCtor.test(value);
    }

    /**
     * Checks if `value` is `null`.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is `null`, else `false`.
     * @example
     *
     * _.isNull(null);
     * // => true
     *
     * _.isNull(void 0);
     * // => false
     */
    function isNull(value) {
      return value === null;
    }

    /**
     * Checks if `value` is classified as a `Number` primitive or object.
     *
     * **Note:** To exclude `Infinity`, `-Infinity`, and `NaN`, which are classified
     * as numbers, use the `_.isFinite` method.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isNumber(8.4);
     * // => true
     *
     * _.isNumber(NaN);
     * // => true
     *
     * _.isNumber('8.4');
     * // => false
     */
    function isNumber(value) {
      return typeof value == 'number' || (isObjectLike(value) && objToString.call(value) == numberTag);
    }

    /**
     * Checks if `value` is a plain object, that is, an object created by the
     * `Object` constructor or one with a `[[Prototype]]` of `null`.
     *
     * **Note:** This method assumes objects created by the `Object` constructor
     * have no inherited enumerable properties.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is a plain object, else `false`.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     * }
     *
     * _.isPlainObject(new Foo);
     * // => false
     *
     * _.isPlainObject([1, 2, 3]);
     * // => false
     *
     * _.isPlainObject({ 'x': 0, 'y': 0 });
     * // => true
     *
     * _.isPlainObject(Object.create(null));
     * // => true
     */
    function isPlainObject(value) {
      var Ctor;

      // Exit early for non `Object` objects.
      if (!(isObjectLike(value) && objToString.call(value) == objectTag && !isArguments(value)) ||
          (!hasOwnProperty.call(value, 'constructor') && (Ctor = value.constructor, typeof Ctor == 'function' && !(Ctor instanceof Ctor)))) {
        return false;
      }
      // IE < 9 iterates inherited properties before own properties. If the first
      // iterated property is an object's own property then there are no inherited
      // enumerable properties.
      var result;
      // In most environments an object's own properties are iterated before
      // its inherited properties. If the last iterated property is an object's
      // own property then there are no inherited enumerable properties.
      baseForIn(value, function(subValue, key) {
        result = key;
      });
      return result === undefined || hasOwnProperty.call(value, result);
    }

    /**
     * Checks if `value` is classified as a `RegExp` object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isRegExp(/abc/);
     * // => true
     *
     * _.isRegExp('/abc/');
     * // => false
     */
    function isRegExp(value) {
      return isObject(value) && objToString.call(value) == regexpTag;
    }

    /**
     * Checks if `value` is classified as a `String` primitive or object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isString('abc');
     * // => true
     *
     * _.isString(1);
     * // => false
     */
    function isString(value) {
      return typeof value == 'string' || (isObjectLike(value) && objToString.call(value) == stringTag);
    }

    /**
     * Checks if `value` is classified as a typed array.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is correctly classified, else `false`.
     * @example
     *
     * _.isTypedArray(new Uint8Array);
     * // => true
     *
     * _.isTypedArray([]);
     * // => false
     */
    function isTypedArray(value) {
      return isObjectLike(value) && isLength(value.length) && !!typedArrayTags[objToString.call(value)];
    }

    /**
     * Checks if `value` is `undefined`.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to check.
     * @returns {boolean} Returns `true` if `value` is `undefined`, else `false`.
     * @example
     *
     * _.isUndefined(void 0);
     * // => true
     *
     * _.isUndefined(null);
     * // => false
     */
    function isUndefined(value) {
      return value === undefined;
    }

    /**
     * Checks if `value` is less than `other`.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to compare.
     * @param {*} other The other value to compare.
     * @returns {boolean} Returns `true` if `value` is less than `other`, else `false`.
     * @example
     *
     * _.lt(1, 3);
     * // => true
     *
     * _.lt(3, 3);
     * // => false
     *
     * _.lt(3, 1);
     * // => false
     */
    function lt(value, other) {
      return value < other;
    }

    /**
     * Checks if `value` is less than or equal to `other`.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to compare.
     * @param {*} other The other value to compare.
     * @returns {boolean} Returns `true` if `value` is less than or equal to `other`, else `false`.
     * @example
     *
     * _.lte(1, 3);
     * // => true
     *
     * _.lte(3, 3);
     * // => true
     *
     * _.lte(3, 1);
     * // => false
     */
    function lte(value, other) {
      return value <= other;
    }

    /**
     * Converts `value` to an array.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to convert.
     * @returns {Array} Returns the converted array.
     * @example
     *
     * (function() {
     *   return _.toArray(arguments).slice(1);
     * }(1, 2, 3));
     * // => [2, 3]
     */
    function toArray(value) {
      var length = value ? getLength(value) : 0;
      if (!isLength(length)) {
        return values(value);
      }
      if (!length) {
        return [];
      }
      return arrayCopy(value);
    }

    /**
     * Converts `value` to a plain object flattening inherited enumerable
     * properties of `value` to own properties of the plain object.
     *
     * @static
     * @memberOf _
     * @category Lang
     * @param {*} value The value to convert.
     * @returns {Object} Returns the converted plain object.
     * @example
     *
     * function Foo() {
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.assign({ 'a': 1 }, new Foo);
     * // => { 'a': 1, 'b': 2 }
     *
     * _.assign({ 'a': 1 }, _.toPlainObject(new Foo));
     * // => { 'a': 1, 'b': 2, 'c': 3 }
     */
    function toPlainObject(value) {
      return baseCopy(value, keysIn(value));
    }

    /*------------------------------------------------------------------------*/

    /**
     * Recursively merges own enumerable properties of the source object(s), that
     * don't resolve to `undefined` into the destination object. Subsequent sources
     * overwrite property assignments of previous sources. If `customizer` is
     * provided it is invoked to produce the merged values of the destination and
     * source properties. If `customizer` returns `undefined` merging is handled
     * by the method instead. The `customizer` is bound to `thisArg` and invoked
     * with five arguments: (objectValue, sourceValue, key, object, source).
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The destination object.
     * @param {...Object} [sources] The source objects.
     * @param {Function} [customizer] The function to customize assigned values.
     * @param {*} [thisArg] The `this` binding of `customizer`.
     * @returns {Object} Returns `object`.
     * @example
     *
     * var users = {
     *   'data': [{ 'user': 'barney' }, { 'user': 'fred' }]
     * };
     *
     * var ages = {
     *   'data': [{ 'age': 36 }, { 'age': 40 }]
     * };
     *
     * _.merge(users, ages);
     * // => { 'data': [{ 'user': 'barney', 'age': 36 }, { 'user': 'fred', 'age': 40 }] }
     *
     * // using a customizer callback
     * var object = {
     *   'fruits': ['apple'],
     *   'vegetables': ['beet']
     * };
     *
     * var other = {
     *   'fruits': ['banana'],
     *   'vegetables': ['carrot']
     * };
     *
     * _.merge(object, other, function(a, b) {
     *   if (_.isArray(a)) {
     *     return a.concat(b);
     *   }
     * });
     * // => { 'fruits': ['apple', 'banana'], 'vegetables': ['beet', 'carrot'] }
     */
    var merge = createAssigner(baseMerge);

    /**
     * Assigns own enumerable properties of source object(s) to the destination
     * object. Subsequent sources overwrite property assignments of previous sources.
     * If `customizer` is provided it is invoked to produce the assigned values.
     * The `customizer` is bound to `thisArg` and invoked with five arguments:
     * (objectValue, sourceValue, key, object, source).
     *
     * **Note:** This method mutates `object` and is based on
     * [`Object.assign`](http://ecma-international.org/ecma-262/6.0/#sec-object.assign).
     *
     * @static
     * @memberOf _
     * @alias extend
     * @category Object
     * @param {Object} object The destination object.
     * @param {...Object} [sources] The source objects.
     * @param {Function} [customizer] The function to customize assigned values.
     * @param {*} [thisArg] The `this` binding of `customizer`.
     * @returns {Object} Returns `object`.
     * @example
     *
     * _.assign({ 'user': 'barney' }, { 'age': 40 }, { 'user': 'fred' });
     * // => { 'user': 'fred', 'age': 40 }
     *
     * // using a customizer callback
     * var defaults = _.partialRight(_.assign, function(value, other) {
     *   return _.isUndefined(value) ? other : value;
     * });
     *
     * defaults({ 'user': 'barney' }, { 'age': 36 }, { 'user': 'fred' });
     * // => { 'user': 'barney', 'age': 36 }
     */
    var assign = createAssigner(function(object, source, customizer) {
      return customizer
        ? assignWith(object, source, customizer)
        : baseAssign(object, source);
    });

    /**
     * Creates an object that inherits from the given `prototype` object. If a
     * `properties` object is provided its own enumerable properties are assigned
     * to the created object.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} prototype The object to inherit from.
     * @param {Object} [properties] The properties to assign to the object.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Object} Returns the new object.
     * @example
     *
     * function Shape() {
     *   this.x = 0;
     *   this.y = 0;
     * }
     *
     * function Circle() {
     *   Shape.call(this);
     * }
     *
     * Circle.prototype = _.create(Shape.prototype, {
     *   'constructor': Circle
     * });
     *
     * var circle = new Circle;
     * circle instanceof Circle;
     * // => true
     *
     * circle instanceof Shape;
     * // => true
     */
    function create(prototype, properties, guard) {
      var result = baseCreate(prototype);
      if (guard && isIterateeCall(prototype, properties, guard)) {
        properties = undefined;
      }
      return properties ? baseAssign(result, properties) : result;
    }

    /**
     * Assigns own enumerable properties of source object(s) to the destination
     * object for all destination properties that resolve to `undefined`. Once a
     * property is set, additional values of the same property are ignored.
     *
     * **Note:** This method mutates `object`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The destination object.
     * @param {...Object} [sources] The source objects.
     * @returns {Object} Returns `object`.
     * @example
     *
     * _.defaults({ 'user': 'barney' }, { 'age': 36 }, { 'user': 'fred' });
     * // => { 'user': 'barney', 'age': 36 }
     */
    var defaults = createDefaults(assign, assignDefaults);

    /**
     * This method is like `_.defaults` except that it recursively assigns
     * default properties.
     *
     * **Note:** This method mutates `object`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The destination object.
     * @param {...Object} [sources] The source objects.
     * @returns {Object} Returns `object`.
     * @example
     *
     * _.defaultsDeep({ 'user': { 'name': 'barney' } }, { 'user': { 'name': 'fred', 'age': 36 } });
     * // => { 'user': { 'name': 'barney', 'age': 36 } }
     *
     */
    var defaultsDeep = createDefaults(merge, mergeDefaults);

    /**
     * This method is like `_.find` except that it returns the key of the first
     * element `predicate` returns truthy for instead of the element itself.
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to search.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {string|undefined} Returns the key of the matched element, else `undefined`.
     * @example
     *
     * var users = {
     *   'barney':  { 'age': 36, 'active': true },
     *   'fred':    { 'age': 40, 'active': false },
     *   'pebbles': { 'age': 1,  'active': true }
     * };
     *
     * _.findKey(users, function(chr) {
     *   return chr.age < 40;
     * });
     * // => 'barney' (iteration order is not guaranteed)
     *
     * // using the `_.matches` callback shorthand
     * _.findKey(users, { 'age': 1, 'active': true });
     * // => 'pebbles'
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.findKey(users, 'active', false);
     * // => 'fred'
     *
     * // using the `_.property` callback shorthand
     * _.findKey(users, 'active');
     * // => 'barney'
     */
    var findKey = createFindKey(baseForOwn);

    /**
     * This method is like `_.findKey` except that it iterates over elements of
     * a collection in the opposite order.
     *
     * If a property name is provided for `predicate` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `predicate` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to search.
     * @param {Function|Object|string} [predicate=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {string|undefined} Returns the key of the matched element, else `undefined`.
     * @example
     *
     * var users = {
     *   'barney':  { 'age': 36, 'active': true },
     *   'fred':    { 'age': 40, 'active': false },
     *   'pebbles': { 'age': 1,  'active': true }
     * };
     *
     * _.findLastKey(users, function(chr) {
     *   return chr.age < 40;
     * });
     * // => returns `pebbles` assuming `_.findKey` returns `barney`
     *
     * // using the `_.matches` callback shorthand
     * _.findLastKey(users, { 'age': 36, 'active': true });
     * // => 'barney'
     *
     * // using the `_.matchesProperty` callback shorthand
     * _.findLastKey(users, 'active', false);
     * // => 'fred'
     *
     * // using the `_.property` callback shorthand
     * _.findLastKey(users, 'active');
     * // => 'pebbles'
     */
    var findLastKey = createFindKey(baseForOwnRight);

    /**
     * Iterates over own and inherited enumerable properties of an object invoking
     * `iteratee` for each property. The `iteratee` is bound to `thisArg` and invoked
     * with three arguments: (value, key, object). Iteratee functions may exit
     * iteration early by explicitly returning `false`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns `object`.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.forIn(new Foo, function(value, key) {
     *   console.log(key);
     * });
     * // => logs 'a', 'b', and 'c' (iteration order is not guaranteed)
     */
    var forIn = createForIn(baseFor);

    /**
     * This method is like `_.forIn` except that it iterates over properties of
     * `object` in the opposite order.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns `object`.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.forInRight(new Foo, function(value, key) {
     *   console.log(key);
     * });
     * // => logs 'c', 'b', and 'a' assuming `_.forIn ` logs 'a', 'b', and 'c'
     */
    var forInRight = createForIn(baseForRight);

    /**
     * Iterates over own enumerable properties of an object invoking `iteratee`
     * for each property. The `iteratee` is bound to `thisArg` and invoked with
     * three arguments: (value, key, object). Iteratee functions may exit iteration
     * early by explicitly returning `false`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns `object`.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.forOwn(new Foo, function(value, key) {
     *   console.log(key);
     * });
     * // => logs 'a' and 'b' (iteration order is not guaranteed)
     */
    var forOwn = createForOwn(baseForOwn);

    /**
     * This method is like `_.forOwn` except that it iterates over properties of
     * `object` in the opposite order.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns `object`.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.forOwnRight(new Foo, function(value, key) {
     *   console.log(key);
     * });
     * // => logs 'b' and 'a' assuming `_.forOwn` logs 'a' and 'b'
     */
    var forOwnRight = createForOwn(baseForOwnRight);

    /**
     * Creates an array of function property names from all enumerable properties,
     * own and inherited, of `object`.
     *
     * @static
     * @memberOf _
     * @alias methods
     * @category Object
     * @param {Object} object The object to inspect.
     * @returns {Array} Returns the new array of property names.
     * @example
     *
     * _.functions(_);
     * // => ['after', 'ary', 'assign', ...]
     */
    function functions(object) {
      return baseFunctions(object, keysIn(object));
    }

    /**
     * Gets the property value at `path` of `object`. If the resolved value is
     * `undefined` the `defaultValue` is used in its place.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @param {Array|string} path The path of the property to get.
     * @param {*} [defaultValue] The value returned if the resolved value is `undefined`.
     * @returns {*} Returns the resolved value.
     * @example
     *
     * var object = { 'a': [{ 'b': { 'c': 3 } }] };
     *
     * _.get(object, 'a[0].b.c');
     * // => 3
     *
     * _.get(object, ['a', '0', 'b', 'c']);
     * // => 3
     *
     * _.get(object, 'a.b.c', 'default');
     * // => 'default'
     */
    function get(object, path, defaultValue) {
      var result = object == null ? undefined : baseGet(object, toPath(path), path + '');
      return result === undefined ? defaultValue : result;
    }

    /**
     * Checks if `path` is a direct property.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @param {Array|string} path The path to check.
     * @returns {boolean} Returns `true` if `path` is a direct property, else `false`.
     * @example
     *
     * var object = { 'a': { 'b': { 'c': 3 } } };
     *
     * _.has(object, 'a');
     * // => true
     *
     * _.has(object, 'a.b.c');
     * // => true
     *
     * _.has(object, ['a', 'b', 'c']);
     * // => true
     */
    function has(object, path) {
      if (object == null) {
        return false;
      }
      var result = hasOwnProperty.call(object, path);
      if (!result && !isKey(path)) {
        path = toPath(path);
        object = path.length == 1 ? object : baseGet(object, baseSlice(path, 0, -1));
        if (object == null) {
          return false;
        }
        path = last(path);
        result = hasOwnProperty.call(object, path);
      }
      return result || (isLength(object.length) && isIndex(path, object.length) &&
        (isArray(object) || isArguments(object)));
    }

    /**
     * Creates an object composed of the inverted keys and values of `object`.
     * If `object` contains duplicate values, subsequent values overwrite property
     * assignments of previous values unless `multiValue` is `true`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to invert.
     * @param {boolean} [multiValue] Allow multiple values per key.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Object} Returns the new inverted object.
     * @example
     *
     * var object = { 'a': 1, 'b': 2, 'c': 1 };
     *
     * _.invert(object);
     * // => { '1': 'c', '2': 'b' }
     *
     * // with `multiValue`
     * _.invert(object, true);
     * // => { '1': ['a', 'c'], '2': ['b'] }
     */
    function invert(object, multiValue, guard) {
      if (guard && isIterateeCall(object, multiValue, guard)) {
        multiValue = undefined;
      }
      var index = -1,
          props = keys(object),
          length = props.length,
          result = {};

      while (++index < length) {
        var key = props[index],
            value = object[key];

        if (multiValue) {
          if (hasOwnProperty.call(result, value)) {
            result[value].push(key);
          } else {
            result[value] = [key];
          }
        }
        else {
          result[value] = key;
        }
      }
      return result;
    }

    /**
     * Creates an array of the own enumerable property names of `object`.
     *
     * **Note:** Non-object values are coerced to objects. See the
     * [ES spec](http://ecma-international.org/ecma-262/6.0/#sec-object.keys)
     * for more details.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @returns {Array} Returns the array of property names.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.keys(new Foo);
     * // => ['a', 'b'] (iteration order is not guaranteed)
     *
     * _.keys('hi');
     * // => ['0', '1']
     */
    var keys = !nativeKeys ? shimKeys : function(object) {
      var Ctor = object == null ? undefined : object.constructor;
      if ((typeof Ctor == 'function' && Ctor.prototype === object) ||
          (typeof object != 'function' && isArrayLike(object))) {
        return shimKeys(object);
      }
      return isObject(object) ? nativeKeys(object) : [];
    };

    /**
     * Creates an array of the own and inherited enumerable property names of `object`.
     *
     * **Note:** Non-object values are coerced to objects.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @returns {Array} Returns the array of property names.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.keysIn(new Foo);
     * // => ['a', 'b', 'c'] (iteration order is not guaranteed)
     */
    function keysIn(object) {
      if (object == null) {
        return [];
      }
      if (!isObject(object)) {
        object = Object(object);
      }
      var length = object.length;
      length = (length && isLength(length) &&
        (isArray(object) || isArguments(object)) && length) || 0;

      var Ctor = object.constructor,
          index = -1,
          isProto = typeof Ctor == 'function' && Ctor.prototype === object,
          result = Array(length),
          skipIndexes = length > 0;

      while (++index < length) {
        result[index] = (index + '');
      }
      for (var key in object) {
        if (!(skipIndexes && isIndex(key, length)) &&
            !(key == 'constructor' && (isProto || !hasOwnProperty.call(object, key)))) {
          result.push(key);
        }
      }
      return result;
    }

    /**
     * The opposite of `_.mapValues`; this method creates an object with the
     * same values as `object` and keys generated by running each own enumerable
     * property of `object` through `iteratee`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns the new mapped object.
     * @example
     *
     * _.mapKeys({ 'a': 1, 'b': 2 }, function(value, key) {
     *   return key + value;
     * });
     * // => { 'a1': 1, 'b2': 2 }
     */
    var mapKeys = createObjectMapper(true);

    /**
     * Creates an object with the same keys as `object` and values generated by
     * running each own enumerable property of `object` through `iteratee`. The
     * iteratee function is bound to `thisArg` and invoked with three arguments:
     * (value, key, object).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to iterate over.
     * @param {Function|Object|string} [iteratee=_.identity] The function invoked
     *  per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Object} Returns the new mapped object.
     * @example
     *
     * _.mapValues({ 'a': 1, 'b': 2 }, function(n) {
     *   return n * 3;
     * });
     * // => { 'a': 3, 'b': 6 }
     *
     * var users = {
     *   'fred':    { 'user': 'fred',    'age': 40 },
     *   'pebbles': { 'user': 'pebbles', 'age': 1 }
     * };
     *
     * // using the `_.property` callback shorthand
     * _.mapValues(users, 'age');
     * // => { 'fred': 40, 'pebbles': 1 } (iteration order is not guaranteed)
     */
    var mapValues = createObjectMapper();

    /**
     * The opposite of `_.pick`; this method creates an object composed of the
     * own and inherited enumerable properties of `object` that are not omitted.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The source object.
     * @param {Function|...(string|string[])} [predicate] The function invoked per
     *  iteration or property names to omit, specified as individual property
     *  names or arrays of property names.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Object} Returns the new object.
     * @example
     *
     * var object = { 'user': 'fred', 'age': 40 };
     *
     * _.omit(object, 'age');
     * // => { 'user': 'fred' }
     *
     * _.omit(object, _.isNumber);
     * // => { 'user': 'fred' }
     */
    var omit = restParam(function(object, props) {
      if (object == null) {
        return {};
      }
      if (typeof props[0] != 'function') {
        var props = arrayMap(baseFlatten(props), String);
        return pickByArray(object, baseDifference(keysIn(object), props));
      }
      var predicate = bindCallback(props[0], props[1], 3);
      return pickByCallback(object, function(value, key, object) {
        return !predicate(value, key, object);
      });
    });

    /**
     * Creates a two dimensional array of the key-value pairs for `object`,
     * e.g. `[[key1, value1], [key2, value2]]`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @returns {Array} Returns the new array of key-value pairs.
     * @example
     *
     * _.pairs({ 'barney': 36, 'fred': 40 });
     * // => [['barney', 36], ['fred', 40]] (iteration order is not guaranteed)
     */
    function pairs(object) {
      object = toObject(object);

      var index = -1,
          props = keys(object),
          length = props.length,
          result = Array(length);

      while (++index < length) {
        var key = props[index];
        result[index] = [key, object[key]];
      }
      return result;
    }

    /**
     * Creates an object composed of the picked `object` properties. Property
     * names may be specified as individual arguments or as arrays of property
     * names. If `predicate` is provided it is invoked for each property of `object`
     * picking the properties `predicate` returns truthy for. The predicate is
     * bound to `thisArg` and invoked with three arguments: (value, key, object).
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The source object.
     * @param {Function|...(string|string[])} [predicate] The function invoked per
     *  iteration or property names to pick, specified as individual property
     *  names or arrays of property names.
     * @param {*} [thisArg] The `this` binding of `predicate`.
     * @returns {Object} Returns the new object.
     * @example
     *
     * var object = { 'user': 'fred', 'age': 40 };
     *
     * _.pick(object, 'user');
     * // => { 'user': 'fred' }
     *
     * _.pick(object, _.isString);
     * // => { 'user': 'fred' }
     */
    var pick = restParam(function(object, props) {
      if (object == null) {
        return {};
      }
      return typeof props[0] == 'function'
        ? pickByCallback(object, bindCallback(props[0], props[1], 3))
        : pickByArray(object, baseFlatten(props));
    });

    /**
     * This method is like `_.get` except that if the resolved value is a function
     * it is invoked with the `this` binding of its parent object and its result
     * is returned.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @param {Array|string} path The path of the property to resolve.
     * @param {*} [defaultValue] The value returned if the resolved value is `undefined`.
     * @returns {*} Returns the resolved value.
     * @example
     *
     * var object = { 'a': [{ 'b': { 'c1': 3, 'c2': _.constant(4) } }] };
     *
     * _.result(object, 'a[0].b.c1');
     * // => 3
     *
     * _.result(object, 'a[0].b.c2');
     * // => 4
     *
     * _.result(object, 'a.b.c', 'default');
     * // => 'default'
     *
     * _.result(object, 'a.b.c', _.constant('default'));
     * // => 'default'
     */
    function result(object, path, defaultValue) {
      var result = object == null ? undefined : object[path];
      if (result === undefined) {
        if (object != null && !isKey(path, object)) {
          path = toPath(path);
          object = path.length == 1 ? object : baseGet(object, baseSlice(path, 0, -1));
          result = object == null ? undefined : object[last(path)];
        }
        result = result === undefined ? defaultValue : result;
      }
      return isFunction(result) ? result.call(object) : result;
    }

    /**
     * Sets the property value of `path` on `object`. If a portion of `path`
     * does not exist it is created.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to augment.
     * @param {Array|string} path The path of the property to set.
     * @param {*} value The value to set.
     * @returns {Object} Returns `object`.
     * @example
     *
     * var object = { 'a': [{ 'b': { 'c': 3 } }] };
     *
     * _.set(object, 'a[0].b.c', 4);
     * console.log(object.a[0].b.c);
     * // => 4
     *
     * _.set(object, 'x[0].y.z', 5);
     * console.log(object.x[0].y.z);
     * // => 5
     */
    function set(object, path, value) {
      if (object == null) {
        return object;
      }
      var pathKey = (path + '');
      path = (object[pathKey] != null || isKey(path, object)) ? [pathKey] : toPath(path);

      var index = -1,
          length = path.length,
          lastIndex = length - 1,
          nested = object;

      while (nested != null && ++index < length) {
        var key = path[index];
        if (isObject(nested)) {
          if (index == lastIndex) {
            nested[key] = value;
          } else if (nested[key] == null) {
            nested[key] = isIndex(path[index + 1]) ? [] : {};
          }
        }
        nested = nested[key];
      }
      return object;
    }

    /**
     * An alternative to `_.reduce`; this method transforms `object` to a new
     * `accumulator` object which is the result of running each of its own enumerable
     * properties through `iteratee`, with each invocation potentially mutating
     * the `accumulator` object. The `iteratee` is bound to `thisArg` and invoked
     * with four arguments: (accumulator, value, key, object). Iteratee functions
     * may exit iteration early by explicitly returning `false`.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Array|Object} object The object to iterate over.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [accumulator] The custom accumulator value.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {*} Returns the accumulated value.
     * @example
     *
     * _.transform([2, 3, 4], function(result, n) {
     *   result.push(n *= n);
     *   return n % 2 == 0;
     * });
     * // => [4, 9]
     *
     * _.transform({ 'a': 1, 'b': 2 }, function(result, n, key) {
     *   result[key] = n * 3;
     * });
     * // => { 'a': 3, 'b': 6 }
     */
    function transform(object, iteratee, accumulator, thisArg) {
      var isArr = isArray(object) || isTypedArray(object);
      iteratee = getCallback(iteratee, thisArg, 4);

      if (accumulator == null) {
        if (isArr || isObject(object)) {
          var Ctor = object.constructor;
          if (isArr) {
            accumulator = isArray(object) ? new Ctor : [];
          } else {
            accumulator = baseCreate(isFunction(Ctor) ? Ctor.prototype : undefined);
          }
        } else {
          accumulator = {};
        }
      }
      (isArr ? arrayEach : baseForOwn)(object, function(value, index, object) {
        return iteratee(accumulator, value, index, object);
      });
      return accumulator;
    }

    /**
     * Creates an array of the own enumerable property values of `object`.
     *
     * **Note:** Non-object values are coerced to objects.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @returns {Array} Returns the array of property values.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.values(new Foo);
     * // => [1, 2] (iteration order is not guaranteed)
     *
     * _.values('hi');
     * // => ['h', 'i']
     */
    function values(object) {
      return baseValues(object, keys(object));
    }

    /**
     * Creates an array of the own and inherited enumerable property values
     * of `object`.
     *
     * **Note:** Non-object values are coerced to objects.
     *
     * @static
     * @memberOf _
     * @category Object
     * @param {Object} object The object to query.
     * @returns {Array} Returns the array of property values.
     * @example
     *
     * function Foo() {
     *   this.a = 1;
     *   this.b = 2;
     * }
     *
     * Foo.prototype.c = 3;
     *
     * _.valuesIn(new Foo);
     * // => [1, 2, 3] (iteration order is not guaranteed)
     */
    function valuesIn(object) {
      return baseValues(object, keysIn(object));
    }

    /*------------------------------------------------------------------------*/

    /**
     * Checks if `n` is between `start` and up to but not including, `end`. If
     * `end` is not specified it is set to `start` with `start` then set to `0`.
     *
     * @static
     * @memberOf _
     * @category Number
     * @param {number} n The number to check.
     * @param {number} [start=0] The start of the range.
     * @param {number} end The end of the range.
     * @returns {boolean} Returns `true` if `n` is in the range, else `false`.
     * @example
     *
     * _.inRange(3, 2, 4);
     * // => true
     *
     * _.inRange(4, 8);
     * // => true
     *
     * _.inRange(4, 2);
     * // => false
     *
     * _.inRange(2, 2);
     * // => false
     *
     * _.inRange(1.2, 2);
     * // => true
     *
     * _.inRange(5.2, 4);
     * // => false
     */
    function inRange(value, start, end) {
      start = +start || 0;
      if (end === undefined) {
        end = start;
        start = 0;
      } else {
        end = +end || 0;
      }
      return value >= nativeMin(start, end) && value < nativeMax(start, end);
    }

    /**
     * Produces a random number between `min` and `max` (inclusive). If only one
     * argument is provided a number between `0` and the given number is returned.
     * If `floating` is `true`, or either `min` or `max` are floats, a floating-point
     * number is returned instead of an integer.
     *
     * @static
     * @memberOf _
     * @category Number
     * @param {number} [min=0] The minimum possible value.
     * @param {number} [max=1] The maximum possible value.
     * @param {boolean} [floating] Specify returning a floating-point number.
     * @returns {number} Returns the random number.
     * @example
     *
     * _.random(0, 5);
     * // => an integer between 0 and 5
     *
     * _.random(5);
     * // => also an integer between 0 and 5
     *
     * _.random(5, true);
     * // => a floating-point number between 0 and 5
     *
     * _.random(1.2, 5.2);
     * // => a floating-point number between 1.2 and 5.2
     */
    function random(min, max, floating) {
      if (floating && isIterateeCall(min, max, floating)) {
        max = floating = undefined;
      }
      var noMin = min == null,
          noMax = max == null;

      if (floating == null) {
        if (noMax && typeof min == 'boolean') {
          floating = min;
          min = 1;
        }
        else if (typeof max == 'boolean') {
          floating = max;
          noMax = true;
        }
      }
      if (noMin && noMax) {
        max = 1;
        noMax = false;
      }
      min = +min || 0;
      if (noMax) {
        max = min;
        min = 0;
      } else {
        max = +max || 0;
      }
      if (floating || min % 1 || max % 1) {
        var rand = nativeRandom();
        return nativeMin(min + (rand * (max - min + parseFloat('1e-' + ((rand + '').length - 1)))), max);
      }
      return baseRandom(min, max);
    }

    /*------------------------------------------------------------------------*/

    /**
     * Converts `string` to [camel case](https://en.wikipedia.org/wiki/CamelCase).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to convert.
     * @returns {string} Returns the camel cased string.
     * @example
     *
     * _.camelCase('Foo Bar');
     * // => 'fooBar'
     *
     * _.camelCase('--foo-bar');
     * // => 'fooBar'
     *
     * _.camelCase('__foo_bar__');
     * // => 'fooBar'
     */
    var camelCase = createCompounder(function(result, word, index) {
      word = word.toLowerCase();
      return result + (index ? (word.charAt(0).toUpperCase() + word.slice(1)) : word);
    });

    /**
     * Capitalizes the first character of `string`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to capitalize.
     * @returns {string} Returns the capitalized string.
     * @example
     *
     * _.capitalize('fred');
     * // => 'Fred'
     */
    function capitalize(string) {
      string = baseToString(string);
      return string && (string.charAt(0).toUpperCase() + string.slice(1));
    }

    /**
     * Deburrs `string` by converting [latin-1 supplementary letters](https://en.wikipedia.org/wiki/Latin-1_Supplement_(Unicode_block)#Character_table)
     * to basic latin letters and removing [combining diacritical marks](https://en.wikipedia.org/wiki/Combining_Diacritical_Marks).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to deburr.
     * @returns {string} Returns the deburred string.
     * @example
     *
     * _.deburr('déjà vu');
     * // => 'deja vu'
     */
    function deburr(string) {
      string = baseToString(string);
      return string && string.replace(reLatin1, deburrLetter).replace(reComboMark, '');
    }

    /**
     * Checks if `string` ends with the given target string.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to search.
     * @param {string} [target] The string to search for.
     * @param {number} [position=string.length] The position to search from.
     * @returns {boolean} Returns `true` if `string` ends with `target`, else `false`.
     * @example
     *
     * _.endsWith('abc', 'c');
     * // => true
     *
     * _.endsWith('abc', 'b');
     * // => false
     *
     * _.endsWith('abc', 'b', 2);
     * // => true
     */
    function endsWith(string, target, position) {
      string = baseToString(string);
      target = (target + '');

      var length = string.length;
      position = position === undefined
        ? length
        : nativeMin(position < 0 ? 0 : (+position || 0), length);

      position -= target.length;
      return position >= 0 && string.indexOf(target, position) == position;
    }

    /**
     * Converts the characters "&", "<", ">", '"', "'", and "\`", in `string` to
     * their corresponding HTML entities.
     *
     * **Note:** No other characters are escaped. To escape additional characters
     * use a third-party library like [_he_](https://mths.be/he).
     *
     * Though the ">" character is escaped for symmetry, characters like
     * ">" and "/" don't need escaping in HTML and have no special meaning
     * unless they're part of a tag or unquoted attribute value.
     * See [Mathias Bynens's article](https://mathiasbynens.be/notes/ambiguous-ampersands)
     * (under "semi-related fun fact") for more details.
     *
     * Backticks are escaped because in Internet Explorer < 9, they can break out
     * of attribute values or HTML comments. See [#59](https://html5sec.org/#59),
     * [#102](https://html5sec.org/#102), [#108](https://html5sec.org/#108), and
     * [#133](https://html5sec.org/#133) of the [HTML5 Security Cheatsheet](https://html5sec.org/)
     * for more details.
     *
     * When working with HTML you should always [quote attribute values](http://wonko.com/post/html-escaping)
     * to reduce XSS vectors.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to escape.
     * @returns {string} Returns the escaped string.
     * @example
     *
     * _.escape('fred, barney, & pebbles');
     * // => 'fred, barney, &amp; pebbles'
     */
    function escape(string) {
      // Reset `lastIndex` because in IE < 9 `String#replace` does not.
      string = baseToString(string);
      return (string && reHasUnescapedHtml.test(string))
        ? string.replace(reUnescapedHtml, escapeHtmlChar)
        : string;
    }

    /**
     * Escapes the `RegExp` special characters "\", "/", "^", "$", ".", "|", "?",
     * "*", "+", "(", ")", "[", "]", "{" and "}" in `string`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to escape.
     * @returns {string} Returns the escaped string.
     * @example
     *
     * _.escapeRegExp('[lodash](https://lodash.com/)');
     * // => '\[lodash\]\(https:\/\/lodash\.com\/\)'
     */
    function escapeRegExp(string) {
      string = baseToString(string);
      return (string && reHasRegExpChars.test(string))
        ? string.replace(reRegExpChars, escapeRegExpChar)
        : (string || '(?:)');
    }

    /**
     * Converts `string` to [kebab case](https://en.wikipedia.org/wiki/Letter_case#Special_case_styles).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to convert.
     * @returns {string} Returns the kebab cased string.
     * @example
     *
     * _.kebabCase('Foo Bar');
     * // => 'foo-bar'
     *
     * _.kebabCase('fooBar');
     * // => 'foo-bar'
     *
     * _.kebabCase('__foo_bar__');
     * // => 'foo-bar'
     */
    var kebabCase = createCompounder(function(result, word, index) {
      return result + (index ? '-' : '') + word.toLowerCase();
    });

    /**
     * Pads `string` on the left and right sides if it's shorter than `length`.
     * Padding characters are truncated if they can't be evenly divided by `length`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to pad.
     * @param {number} [length=0] The padding length.
     * @param {string} [chars=' '] The string used as padding.
     * @returns {string} Returns the padded string.
     * @example
     *
     * _.pad('abc', 8);
     * // => '  abc   '
     *
     * _.pad('abc', 8, '_-');
     * // => '_-abc_-_'
     *
     * _.pad('abc', 3);
     * // => 'abc'
     */
    function pad(string, length, chars) {
      string = baseToString(string);
      length = +length;

      var strLength = string.length;
      if (strLength >= length || !nativeIsFinite(length)) {
        return string;
      }
      var mid = (length - strLength) / 2,
          leftLength = nativeFloor(mid),
          rightLength = nativeCeil(mid);

      chars = createPadding('', rightLength, chars);
      return chars.slice(0, leftLength) + string + chars;
    }

    /**
     * Pads `string` on the left side if it's shorter than `length`. Padding
     * characters are truncated if they exceed `length`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to pad.
     * @param {number} [length=0] The padding length.
     * @param {string} [chars=' '] The string used as padding.
     * @returns {string} Returns the padded string.
     * @example
     *
     * _.padLeft('abc', 6);
     * // => '   abc'
     *
     * _.padLeft('abc', 6, '_-');
     * // => '_-_abc'
     *
     * _.padLeft('abc', 3);
     * // => 'abc'
     */
    var padLeft = createPadDir();

    /**
     * Pads `string` on the right side if it's shorter than `length`. Padding
     * characters are truncated if they exceed `length`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to pad.
     * @param {number} [length=0] The padding length.
     * @param {string} [chars=' '] The string used as padding.
     * @returns {string} Returns the padded string.
     * @example
     *
     * _.padRight('abc', 6);
     * // => 'abc   '
     *
     * _.padRight('abc', 6, '_-');
     * // => 'abc_-_'
     *
     * _.padRight('abc', 3);
     * // => 'abc'
     */
    var padRight = createPadDir(true);

    /**
     * Converts `string` to an integer of the specified radix. If `radix` is
     * `undefined` or `0`, a `radix` of `10` is used unless `value` is a hexadecimal,
     * in which case a `radix` of `16` is used.
     *
     * **Note:** This method aligns with the [ES5 implementation](https://es5.github.io/#E)
     * of `parseInt`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} string The string to convert.
     * @param {number} [radix] The radix to interpret `value` by.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {number} Returns the converted integer.
     * @example
     *
     * _.parseInt('08');
     * // => 8
     *
     * _.map(['6', '08', '10'], _.parseInt);
     * // => [6, 8, 10]
     */
    function parseInt(string, radix, guard) {
      // Firefox < 21 and Opera < 15 follow ES3 for `parseInt`.
      // Chrome fails to trim leading <BOM> whitespace characters.
      // See https://code.google.com/p/v8/issues/detail?id=3109 for more details.
      if (guard ? isIterateeCall(string, radix, guard) : radix == null) {
        radix = 0;
      } else if (radix) {
        radix = +radix;
      }
      string = trim(string);
      return nativeParseInt(string, radix || (reHasHexPrefix.test(string) ? 16 : 10));
    }

    /**
     * Repeats the given string `n` times.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to repeat.
     * @param {number} [n=0] The number of times to repeat the string.
     * @returns {string} Returns the repeated string.
     * @example
     *
     * _.repeat('*', 3);
     * // => '***'
     *
     * _.repeat('abc', 2);
     * // => 'abcabc'
     *
     * _.repeat('abc', 0);
     * // => ''
     */
    function repeat(string, n) {
      var result = '';
      string = baseToString(string);
      n = +n;
      if (n < 1 || !string || !nativeIsFinite(n)) {
        return result;
      }
      // Leverage the exponentiation by squaring algorithm for a faster repeat.
      // See https://en.wikipedia.org/wiki/Exponentiation_by_squaring for more details.
      do {
        if (n % 2) {
          result += string;
        }
        n = nativeFloor(n / 2);
        string += string;
      } while (n);

      return result;
    }

    /**
     * Converts `string` to [snake case](https://en.wikipedia.org/wiki/Snake_case).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to convert.
     * @returns {string} Returns the snake cased string.
     * @example
     *
     * _.snakeCase('Foo Bar');
     * // => 'foo_bar'
     *
     * _.snakeCase('fooBar');
     * // => 'foo_bar'
     *
     * _.snakeCase('--foo-bar');
     * // => 'foo_bar'
     */
    var snakeCase = createCompounder(function(result, word, index) {
      return result + (index ? '_' : '') + word.toLowerCase();
    });

    /**
     * Converts `string` to [start case](https://en.wikipedia.org/wiki/Letter_case#Stylistic_or_specialised_usage).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to convert.
     * @returns {string} Returns the start cased string.
     * @example
     *
     * _.startCase('--foo-bar');
     * // => 'Foo Bar'
     *
     * _.startCase('fooBar');
     * // => 'Foo Bar'
     *
     * _.startCase('__foo_bar__');
     * // => 'Foo Bar'
     */
    var startCase = createCompounder(function(result, word, index) {
      return result + (index ? ' ' : '') + (word.charAt(0).toUpperCase() + word.slice(1));
    });

    /**
     * Checks if `string` starts with the given target string.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to search.
     * @param {string} [target] The string to search for.
     * @param {number} [position=0] The position to search from.
     * @returns {boolean} Returns `true` if `string` starts with `target`, else `false`.
     * @example
     *
     * _.startsWith('abc', 'a');
     * // => true
     *
     * _.startsWith('abc', 'b');
     * // => false
     *
     * _.startsWith('abc', 'b', 1);
     * // => true
     */
    function startsWith(string, target, position) {
      string = baseToString(string);
      position = position == null
        ? 0
        : nativeMin(position < 0 ? 0 : (+position || 0), string.length);

      return string.lastIndexOf(target, position) == position;
    }

    /**
     * Creates a compiled template function that can interpolate data properties
     * in "interpolate" delimiters, HTML-escape interpolated data properties in
     * "escape" delimiters, and execute JavaScript in "evaluate" delimiters. Data
     * properties may be accessed as free variables in the template. If a setting
     * object is provided it takes precedence over `_.templateSettings` values.
     *
     * **Note:** In the development build `_.template` utilizes
     * [sourceURLs](http://www.html5rocks.com/en/tutorials/developertools/sourcemaps/#toc-sourceurl)
     * for easier debugging.
     *
     * For more information on precompiling templates see
     * [lodash's custom builds documentation](https://lodash.com/custom-builds).
     *
     * For more information on Chrome extension sandboxes see
     * [Chrome's extensions documentation](https://developer.chrome.com/extensions/sandboxingEval).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The template string.
     * @param {Object} [options] The options object.
     * @param {RegExp} [options.escape] The HTML "escape" delimiter.
     * @param {RegExp} [options.evaluate] The "evaluate" delimiter.
     * @param {Object} [options.imports] An object to import into the template as free variables.
     * @param {RegExp} [options.interpolate] The "interpolate" delimiter.
     * @param {string} [options.sourceURL] The sourceURL of the template's compiled source.
     * @param {string} [options.variable] The data object variable name.
     * @param- {Object} [otherOptions] Enables the legacy `options` param signature.
     * @returns {Function} Returns the compiled template function.
     * @example
     *
     * // using the "interpolate" delimiter to create a compiled template
     * var compiled = _.template('hello <%= user %>!');
     * compiled({ 'user': 'fred' });
     * // => 'hello fred!'
     *
     * // using the HTML "escape" delimiter to escape data property values
     * var compiled = _.template('<b><%- value %></b>');
     * compiled({ 'value': '<script>' });
     * // => '<b>&lt;script&gt;</b>'
     *
     * // using the "evaluate" delimiter to execute JavaScript and generate HTML
     * var compiled = _.template('<% _.forEach(users, function(user) { %><li><%- user %></li><% }); %>');
     * compiled({ 'users': ['fred', 'barney'] });
     * // => '<li>fred</li><li>barney</li>'
     *
     * // using the internal `print` function in "evaluate" delimiters
     * var compiled = _.template('<% print("hello " + user); %>!');
     * compiled({ 'user': 'barney' });
     * // => 'hello barney!'
     *
     * // using the ES delimiter as an alternative to the default "interpolate" delimiter
     * var compiled = _.template('hello ${ user }!');
     * compiled({ 'user': 'pebbles' });
     * // => 'hello pebbles!'
     *
     * // using custom template delimiters
     * _.templateSettings.interpolate = /{{([\s\S]+?)}}/g;
     * var compiled = _.template('hello {{ user }}!');
     * compiled({ 'user': 'mustache' });
     * // => 'hello mustache!'
     *
     * // using backslashes to treat delimiters as plain text
     * var compiled = _.template('<%= "\\<%- value %\\>" %>');
     * compiled({ 'value': 'ignored' });
     * // => '<%- value %>'
     *
     * // using the `imports` option to import `jQuery` as `jq`
     * var text = '<% jq.each(users, function(user) { %><li><%- user %></li><% }); %>';
     * var compiled = _.template(text, { 'imports': { 'jq': jQuery } });
     * compiled({ 'users': ['fred', 'barney'] });
     * // => '<li>fred</li><li>barney</li>'
     *
     * // using the `sourceURL` option to specify a custom sourceURL for the template
     * var compiled = _.template('hello <%= user %>!', { 'sourceURL': '/basic/greeting.jst' });
     * compiled(data);
     * // => find the source of "greeting.jst" under the Sources tab or Resources panel of the web inspector
     *
     * // using the `variable` option to ensure a with-statement isn't used in the compiled template
     * var compiled = _.template('hi <%= data.user %>!', { 'variable': 'data' });
     * compiled.source;
     * // => function(data) {
     * //   var __t, __p = '';
     * //   __p += 'hi ' + ((__t = ( data.user )) == null ? '' : __t) + '!';
     * //   return __p;
     * // }
     *
     * // using the `source` property to inline compiled templates for meaningful
     * // line numbers in error messages and a stack trace
     * fs.writeFileSync(path.join(cwd, 'jst.js'), '\
     *   var JST = {\
     *     "main": ' + _.template(mainText).source + '\
     *   };\
     * ');
     */
    function template(string, options, otherOptions) {
      // Based on John Resig's `tmpl` implementation (http://ejohn.org/blog/javascript-micro-templating/)
      // and Laura Doktorova's doT.js (https://github.com/olado/doT).
      var settings = lodash.templateSettings;

      if (otherOptions && isIterateeCall(string, options, otherOptions)) {
        options = otherOptions = undefined;
      }
      string = baseToString(string);
      options = assignWith(baseAssign({}, otherOptions || options), settings, assignOwnDefaults);

      var imports = assignWith(baseAssign({}, options.imports), settings.imports, assignOwnDefaults),
          importsKeys = keys(imports),
          importsValues = baseValues(imports, importsKeys);

      var isEscaping,
          isEvaluating,
          index = 0,
          interpolate = options.interpolate || reNoMatch,
          source = "__p += '";

      // Compile the regexp to match each delimiter.
      var reDelimiters = RegExp(
        (options.escape || reNoMatch).source + '|' +
        interpolate.source + '|' +
        (interpolate === reInterpolate ? reEsTemplate : reNoMatch).source + '|' +
        (options.evaluate || reNoMatch).source + '|$'
      , 'g');

      // Use a sourceURL for easier debugging.
      var sourceURL = '//# sourceURL=' +
        ('sourceURL' in options
          ? options.sourceURL
          : ('lodash.templateSources[' + (++templateCounter) + ']')
        ) + '\n';

      string.replace(reDelimiters, function(match, escapeValue, interpolateValue, esTemplateValue, evaluateValue, offset) {
        interpolateValue || (interpolateValue = esTemplateValue);

        // Escape characters that can't be included in string literals.
        source += string.slice(index, offset).replace(reUnescapedString, escapeStringChar);

        // Replace delimiters with snippets.
        if (escapeValue) {
          isEscaping = true;
          source += "' +\n__e(" + escapeValue + ") +\n'";
        }
        if (evaluateValue) {
          isEvaluating = true;
          source += "';\n" + evaluateValue + ";\n__p += '";
        }
        if (interpolateValue) {
          source += "' +\n((__t = (" + interpolateValue + ")) == null ? '' : __t) +\n'";
        }
        index = offset + match.length;

        // The JS engine embedded in Adobe products requires returning the `match`
        // string in order to produce the correct `offset` value.
        return match;
      });

      source += "';\n";

      // If `variable` is not specified wrap a with-statement around the generated
      // code to add the data object to the top of the scope chain.
      var variable = options.variable;
      if (!variable) {
        source = 'with (obj) {\n' + source + '\n}\n';
      }
      // Cleanup code by stripping empty strings.
      source = (isEvaluating ? source.replace(reEmptyStringLeading, '') : source)
        .replace(reEmptyStringMiddle, '$1')
        .replace(reEmptyStringTrailing, '$1;');

      // Frame code as the function body.
      source = 'function(' + (variable || 'obj') + ') {\n' +
        (variable
          ? ''
          : 'obj || (obj = {});\n'
        ) +
        "var __t, __p = ''" +
        (isEscaping
           ? ', __e = _.escape'
           : ''
        ) +
        (isEvaluating
          ? ', __j = Array.prototype.join;\n' +
            "function print() { __p += __j.call(arguments, '') }\n"
          : ';\n'
        ) +
        source +
        'return __p\n}';

      var result = attempt(function() {
        return Function(importsKeys, sourceURL + 'return ' + source).apply(undefined, importsValues);
      });

      // Provide the compiled function's source by its `toString` method or
      // the `source` property as a convenience for inlining compiled templates.
      result.source = source;
      if (isError(result)) {
        throw result;
      }
      return result;
    }

    /**
     * Removes leading and trailing whitespace or specified characters from `string`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to trim.
     * @param {string} [chars=whitespace] The characters to trim.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {string} Returns the trimmed string.
     * @example
     *
     * _.trim('  abc  ');
     * // => 'abc'
     *
     * _.trim('-_-abc-_-', '_-');
     * // => 'abc'
     *
     * _.map(['  foo  ', '  bar  '], _.trim);
     * // => ['foo', 'bar']
     */
    function trim(string, chars, guard) {
      var value = string;
      string = baseToString(string);
      if (!string) {
        return string;
      }
      if (guard ? isIterateeCall(value, chars, guard) : chars == null) {
        return string.slice(trimmedLeftIndex(string), trimmedRightIndex(string) + 1);
      }
      chars = (chars + '');
      return string.slice(charsLeftIndex(string, chars), charsRightIndex(string, chars) + 1);
    }

    /**
     * Removes leading whitespace or specified characters from `string`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to trim.
     * @param {string} [chars=whitespace] The characters to trim.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {string} Returns the trimmed string.
     * @example
     *
     * _.trimLeft('  abc  ');
     * // => 'abc  '
     *
     * _.trimLeft('-_-abc-_-', '_-');
     * // => 'abc-_-'
     */
    function trimLeft(string, chars, guard) {
      var value = string;
      string = baseToString(string);
      if (!string) {
        return string;
      }
      if (guard ? isIterateeCall(value, chars, guard) : chars == null) {
        return string.slice(trimmedLeftIndex(string));
      }
      return string.slice(charsLeftIndex(string, (chars + '')));
    }

    /**
     * Removes trailing whitespace or specified characters from `string`.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to trim.
     * @param {string} [chars=whitespace] The characters to trim.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {string} Returns the trimmed string.
     * @example
     *
     * _.trimRight('  abc  ');
     * // => '  abc'
     *
     * _.trimRight('-_-abc-_-', '_-');
     * // => '-_-abc'
     */
    function trimRight(string, chars, guard) {
      var value = string;
      string = baseToString(string);
      if (!string) {
        return string;
      }
      if (guard ? isIterateeCall(value, chars, guard) : chars == null) {
        return string.slice(0, trimmedRightIndex(string) + 1);
      }
      return string.slice(0, charsRightIndex(string, (chars + '')) + 1);
    }

    /**
     * Truncates `string` if it's longer than the given maximum string length.
     * The last characters of the truncated string are replaced with the omission
     * string which defaults to "...".
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to truncate.
     * @param {Object|number} [options] The options object or maximum string length.
     * @param {number} [options.length=30] The maximum string length.
     * @param {string} [options.omission='...'] The string to indicate text is omitted.
     * @param {RegExp|string} [options.separator] The separator pattern to truncate to.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {string} Returns the truncated string.
     * @example
     *
     * _.trunc('hi-diddly-ho there, neighborino');
     * // => 'hi-diddly-ho there, neighbo...'
     *
     * _.trunc('hi-diddly-ho there, neighborino', 24);
     * // => 'hi-diddly-ho there, n...'
     *
     * _.trunc('hi-diddly-ho there, neighborino', {
     *   'length': 24,
     *   'separator': ' '
     * });
     * // => 'hi-diddly-ho there,...'
     *
     * _.trunc('hi-diddly-ho there, neighborino', {
     *   'length': 24,
     *   'separator': /,? +/
     * });
     * // => 'hi-diddly-ho there...'
     *
     * _.trunc('hi-diddly-ho there, neighborino', {
     *   'omission': ' [...]'
     * });
     * // => 'hi-diddly-ho there, neig [...]'
     */
    function trunc(string, options, guard) {
      if (guard && isIterateeCall(string, options, guard)) {
        options = undefined;
      }
      var length = DEFAULT_TRUNC_LENGTH,
          omission = DEFAULT_TRUNC_OMISSION;

      if (options != null) {
        if (isObject(options)) {
          var separator = 'separator' in options ? options.separator : separator;
          length = 'length' in options ? (+options.length || 0) : length;
          omission = 'omission' in options ? baseToString(options.omission) : omission;
        } else {
          length = +options || 0;
        }
      }
      string = baseToString(string);
      if (length >= string.length) {
        return string;
      }
      var end = length - omission.length;
      if (end < 1) {
        return omission;
      }
      var result = string.slice(0, end);
      if (separator == null) {
        return result + omission;
      }
      if (isRegExp(separator)) {
        if (string.slice(end).search(separator)) {
          var match,
              newEnd,
              substring = string.slice(0, end);

          if (!separator.global) {
            separator = RegExp(separator.source, (reFlags.exec(separator) || '') + 'g');
          }
          separator.lastIndex = 0;
          while ((match = separator.exec(substring))) {
            newEnd = match.index;
          }
          result = result.slice(0, newEnd == null ? end : newEnd);
        }
      } else if (string.indexOf(separator, end) != end) {
        var index = result.lastIndexOf(separator);
        if (index > -1) {
          result = result.slice(0, index);
        }
      }
      return result + omission;
    }

    /**
     * The inverse of `_.escape`; this method converts the HTML entities
     * `&amp;`, `&lt;`, `&gt;`, `&quot;`, `&#39;`, and `&#96;` in `string` to their
     * corresponding characters.
     *
     * **Note:** No other HTML entities are unescaped. To unescape additional HTML
     * entities use a third-party library like [_he_](https://mths.be/he).
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to unescape.
     * @returns {string} Returns the unescaped string.
     * @example
     *
     * _.unescape('fred, barney, &amp; pebbles');
     * // => 'fred, barney, & pebbles'
     */
    function unescape(string) {
      string = baseToString(string);
      return (string && reHasEscapedHtml.test(string))
        ? string.replace(reEscapedHtml, unescapeHtmlChar)
        : string;
    }

    /**
     * Splits `string` into an array of its words.
     *
     * @static
     * @memberOf _
     * @category String
     * @param {string} [string=''] The string to inspect.
     * @param {RegExp|string} [pattern] The pattern to match words.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Array} Returns the words of `string`.
     * @example
     *
     * _.words('fred, barney, & pebbles');
     * // => ['fred', 'barney', 'pebbles']
     *
     * _.words('fred, barney, & pebbles', /[^, ]+/g);
     * // => ['fred', 'barney', '&', 'pebbles']
     */
    function words(string, pattern, guard) {
      if (guard && isIterateeCall(string, pattern, guard)) {
        pattern = undefined;
      }
      string = baseToString(string);
      return string.match(pattern || reWords) || [];
    }

    /*------------------------------------------------------------------------*/

    /**
     * Attempts to invoke `func`, returning either the result or the caught error
     * object. Any additional arguments are provided to `func` when it is invoked.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Function} func The function to attempt.
     * @returns {*} Returns the `func` result or error object.
     * @example
     *
     * // avoid throwing errors for invalid selectors
     * var elements = _.attempt(function(selector) {
     *   return document.querySelectorAll(selector);
     * }, '>_>');
     *
     * if (_.isError(elements)) {
     *   elements = [];
     * }
     */
    var attempt = restParam(function(func, args) {
      try {
        return func.apply(undefined, args);
      } catch(e) {
        return isError(e) ? e : new Error(e);
      }
    });

    /**
     * Creates a function that invokes `func` with the `this` binding of `thisArg`
     * and arguments of the created function. If `func` is a property name the
     * created callback returns the property value for a given element. If `func`
     * is an object the created callback returns `true` for elements that contain
     * the equivalent object properties, otherwise it returns `false`.
     *
     * @static
     * @memberOf _
     * @alias iteratee
     * @category Utility
     * @param {*} [func=_.identity] The value to convert to a callback.
     * @param {*} [thisArg] The `this` binding of `func`.
     * @param- {Object} [guard] Enables use as a callback for functions like `_.map`.
     * @returns {Function} Returns the callback.
     * @example
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36 },
     *   { 'user': 'fred',   'age': 40 }
     * ];
     *
     * // wrap to create custom callback shorthands
     * _.callback = _.wrap(_.callback, function(callback, func, thisArg) {
     *   var match = /^(.+?)__([gl]t)(.+)$/.exec(func);
     *   if (!match) {
     *     return callback(func, thisArg);
     *   }
     *   return function(object) {
     *     return match[2] == 'gt'
     *       ? object[match[1]] > match[3]
     *       : object[match[1]] < match[3];
     *   };
     * });
     *
     * _.filter(users, 'age__gt36');
     * // => [{ 'user': 'fred', 'age': 40 }]
     */
    function callback(func, thisArg, guard) {
      if (guard && isIterateeCall(func, thisArg, guard)) {
        thisArg = undefined;
      }
      return isObjectLike(func)
        ? matches(func)
        : baseCallback(func, thisArg);
    }

    /**
     * Creates a function that returns `value`.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {*} value The value to return from the new function.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var object = { 'user': 'fred' };
     * var getter = _.constant(object);
     *
     * getter() === object;
     * // => true
     */
    function constant(value) {
      return function() {
        return value;
      };
    }

    /**
     * This method returns the first argument provided to it.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {*} value Any value.
     * @returns {*} Returns `value`.
     * @example
     *
     * var object = { 'user': 'fred' };
     *
     * _.identity(object) === object;
     * // => true
     */
    function identity(value) {
      return value;
    }

    /**
     * Creates a function that performs a deep comparison between a given object
     * and `source`, returning `true` if the given object has equivalent property
     * values, else `false`.
     *
     * **Note:** This method supports comparing arrays, booleans, `Date` objects,
     * numbers, `Object` objects, regexes, and strings. Objects are compared by
     * their own, not inherited, enumerable properties. For comparing a single
     * own or inherited property value see `_.matchesProperty`.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Object} source The object of property values to match.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36, 'active': true },
     *   { 'user': 'fred',   'age': 40, 'active': false }
     * ];
     *
     * _.filter(users, _.matches({ 'age': 40, 'active': false }));
     * // => [{ 'user': 'fred', 'age': 40, 'active': false }]
     */
    function matches(source) {
      return baseMatches(baseClone(source, true));
    }

    /**
     * Creates a function that compares the property value of `path` on a given
     * object to `value`.
     *
     * **Note:** This method supports comparing arrays, booleans, `Date` objects,
     * numbers, `Object` objects, regexes, and strings. Objects are compared by
     * their own, not inherited, enumerable properties.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Array|string} path The path of the property to get.
     * @param {*} srcValue The value to match.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var users = [
     *   { 'user': 'barney' },
     *   { 'user': 'fred' }
     * ];
     *
     * _.find(users, _.matchesProperty('user', 'fred'));
     * // => { 'user': 'fred' }
     */
    function matchesProperty(path, srcValue) {
      return baseMatchesProperty(path, baseClone(srcValue, true));
    }

    /**
     * Creates a function that invokes the method at `path` on a given object.
     * Any additional arguments are provided to the invoked method.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Array|string} path The path of the method to invoke.
     * @param {...*} [args] The arguments to invoke the method with.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var objects = [
     *   { 'a': { 'b': { 'c': _.constant(2) } } },
     *   { 'a': { 'b': { 'c': _.constant(1) } } }
     * ];
     *
     * _.map(objects, _.method('a.b.c'));
     * // => [2, 1]
     *
     * _.invoke(_.sortBy(objects, _.method(['a', 'b', 'c'])), 'a.b.c');
     * // => [1, 2]
     */
    var method = restParam(function(path, args) {
      return function(object) {
        return invokePath(object, path, args);
      };
    });

    /**
     * The opposite of `_.method`; this method creates a function that invokes
     * the method at a given path on `object`. Any additional arguments are
     * provided to the invoked method.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Object} object The object to query.
     * @param {...*} [args] The arguments to invoke the method with.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var array = _.times(3, _.constant),
     *     object = { 'a': array, 'b': array, 'c': array };
     *
     * _.map(['a[2]', 'c[0]'], _.methodOf(object));
     * // => [2, 0]
     *
     * _.map([['a', '2'], ['c', '0']], _.methodOf(object));
     * // => [2, 0]
     */
    var methodOf = restParam(function(object, args) {
      return function(path) {
        return invokePath(object, path, args);
      };
    });

    /**
     * Adds all own enumerable function properties of a source object to the
     * destination object. If `object` is a function then methods are added to
     * its prototype as well.
     *
     * **Note:** Use `_.runInContext` to create a pristine `lodash` function to
     * avoid conflicts caused by modifying the original.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Function|Object} [object=lodash] The destination object.
     * @param {Object} source The object of functions to add.
     * @param {Object} [options] The options object.
     * @param {boolean} [options.chain=true] Specify whether the functions added
     *  are chainable.
     * @returns {Function|Object} Returns `object`.
     * @example
     *
     * function vowels(string) {
     *   return _.filter(string, function(v) {
     *     return /[aeiou]/i.test(v);
     *   });
     * }
     *
     * _.mixin({ 'vowels': vowels });
     * _.vowels('fred');
     * // => ['e']
     *
     * _('fred').vowels().value();
     * // => ['e']
     *
     * _.mixin({ 'vowels': vowels }, { 'chain': false });
     * _('fred').vowels();
     * // => ['e']
     */
    function mixin(object, source, options) {
      if (options == null) {
        var isObj = isObject(source),
            props = isObj ? keys(source) : undefined,
            methodNames = (props && props.length) ? baseFunctions(source, props) : undefined;

        if (!(methodNames ? methodNames.length : isObj)) {
          methodNames = false;
          options = source;
          source = object;
          object = this;
        }
      }
      if (!methodNames) {
        methodNames = baseFunctions(source, keys(source));
      }
      var chain = true,
          index = -1,
          isFunc = isFunction(object),
          length = methodNames.length;

      if (options === false) {
        chain = false;
      } else if (isObject(options) && 'chain' in options) {
        chain = options.chain;
      }
      while (++index < length) {
        var methodName = methodNames[index],
            func = source[methodName];

        object[methodName] = func;
        if (isFunc) {
          object.prototype[methodName] = (function(func) {
            return function() {
              var chainAll = this.__chain__;
              if (chain || chainAll) {
                var result = object(this.__wrapped__),
                    actions = result.__actions__ = arrayCopy(this.__actions__);

                actions.push({ 'func': func, 'args': arguments, 'thisArg': object });
                result.__chain__ = chainAll;
                return result;
              }
              return func.apply(object, arrayPush([this.value()], arguments));
            };
          }(func));
        }
      }
      return object;
    }

    /**
     * Reverts the `_` variable to its previous value and returns a reference to
     * the `lodash` function.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @returns {Function} Returns the `lodash` function.
     * @example
     *
     * var lodash = _.noConflict();
     */
    function noConflict() {
      root._ = oldDash;
      return this;
    }

    /**
     * A no-operation function that returns `undefined` regardless of the
     * arguments it receives.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @example
     *
     * var object = { 'user': 'fred' };
     *
     * _.noop(object) === undefined;
     * // => true
     */
    function noop() {
      // No operation performed.
    }

    /**
     * Creates a function that returns the property value at `path` on a
     * given object.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Array|string} path The path of the property to get.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var objects = [
     *   { 'a': { 'b': { 'c': 2 } } },
     *   { 'a': { 'b': { 'c': 1 } } }
     * ];
     *
     * _.map(objects, _.property('a.b.c'));
     * // => [2, 1]
     *
     * _.pluck(_.sortBy(objects, _.property(['a', 'b', 'c'])), 'a.b.c');
     * // => [1, 2]
     */
    function property(path) {
      return isKey(path) ? baseProperty(path) : basePropertyDeep(path);
    }

    /**
     * The opposite of `_.property`; this method creates a function that returns
     * the property value at a given path on `object`.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {Object} object The object to query.
     * @returns {Function} Returns the new function.
     * @example
     *
     * var array = [0, 1, 2],
     *     object = { 'a': array, 'b': array, 'c': array };
     *
     * _.map(['a[2]', 'c[0]'], _.propertyOf(object));
     * // => [2, 0]
     *
     * _.map([['a', '2'], ['c', '0']], _.propertyOf(object));
     * // => [2, 0]
     */
    function propertyOf(object) {
      return function(path) {
        return baseGet(object, toPath(path), path + '');
      };
    }

    /**
     * Creates an array of numbers (positive and/or negative) progressing from
     * `start` up to, but not including, `end`. If `end` is not specified it is
     * set to `start` with `start` then set to `0`. If `end` is less than `start`
     * a zero-length range is created unless a negative `step` is specified.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {number} [start=0] The start of the range.
     * @param {number} end The end of the range.
     * @param {number} [step=1] The value to increment or decrement by.
     * @returns {Array} Returns the new array of numbers.
     * @example
     *
     * _.range(4);
     * // => [0, 1, 2, 3]
     *
     * _.range(1, 5);
     * // => [1, 2, 3, 4]
     *
     * _.range(0, 20, 5);
     * // => [0, 5, 10, 15]
     *
     * _.range(0, -4, -1);
     * // => [0, -1, -2, -3]
     *
     * _.range(1, 4, 0);
     * // => [1, 1, 1]
     *
     * _.range(0);
     * // => []
     */
    function range(start, end, step) {
      if (step && isIterateeCall(start, end, step)) {
        end = step = undefined;
      }
      start = +start || 0;
      step = step == null ? 1 : (+step || 0);

      if (end == null) {
        end = start;
        start = 0;
      } else {
        end = +end || 0;
      }
      // Use `Array(length)` so engines like Chakra and V8 avoid slower modes.
      // See https://youtu.be/XAqIpGU8ZZk#t=17m25s for more details.
      var index = -1,
          length = nativeMax(nativeCeil((end - start) / (step || 1)), 0),
          result = Array(length);

      while (++index < length) {
        result[index] = start;
        start += step;
      }
      return result;
    }

    /**
     * Invokes the iteratee function `n` times, returning an array of the results
     * of each invocation. The `iteratee` is bound to `thisArg` and invoked with
     * one argument; (index).
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {number} n The number of times to invoke `iteratee`.
     * @param {Function} [iteratee=_.identity] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {Array} Returns the array of results.
     * @example
     *
     * var diceRolls = _.times(3, _.partial(_.random, 1, 6, false));
     * // => [3, 6, 4]
     *
     * _.times(3, function(n) {
     *   mage.castSpell(n);
     * });
     * // => invokes `mage.castSpell(n)` three times with `n` of `0`, `1`, and `2`
     *
     * _.times(3, function(n) {
     *   this.cast(n);
     * }, mage);
     * // => also invokes `mage.castSpell(n)` three times
     */
    function times(n, iteratee, thisArg) {
      n = nativeFloor(n);

      // Exit early to avoid a JSC JIT bug in Safari 8
      // where `Array(0)` is treated as `Array(1)`.
      if (n < 1 || !nativeIsFinite(n)) {
        return [];
      }
      var index = -1,
          result = Array(nativeMin(n, MAX_ARRAY_LENGTH));

      iteratee = bindCallback(iteratee, thisArg, 1);
      while (++index < n) {
        if (index < MAX_ARRAY_LENGTH) {
          result[index] = iteratee(index);
        } else {
          iteratee(index);
        }
      }
      return result;
    }

    /**
     * Generates a unique ID. If `prefix` is provided the ID is appended to it.
     *
     * @static
     * @memberOf _
     * @category Utility
     * @param {string} [prefix] The value to prefix the ID with.
     * @returns {string} Returns the unique ID.
     * @example
     *
     * _.uniqueId('contact_');
     * // => 'contact_104'
     *
     * _.uniqueId();
     * // => '105'
     */
    function uniqueId(prefix) {
      var id = ++idCounter;
      return baseToString(prefix) + id;
    }

    /*------------------------------------------------------------------------*/

    /**
     * Adds two numbers.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {number} augend The first number to add.
     * @param {number} addend The second number to add.
     * @returns {number} Returns the sum.
     * @example
     *
     * _.add(6, 4);
     * // => 10
     */
    function add(augend, addend) {
      return (+augend || 0) + (+addend || 0);
    }

    /**
     * Calculates `n` rounded up to `precision`.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {number} n The number to round up.
     * @param {number} [precision=0] The precision to round up to.
     * @returns {number} Returns the rounded up number.
     * @example
     *
     * _.ceil(4.006);
     * // => 5
     *
     * _.ceil(6.004, 2);
     * // => 6.01
     *
     * _.ceil(6040, -2);
     * // => 6100
     */
    var ceil = createRound('ceil');

    /**
     * Calculates `n` rounded down to `precision`.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {number} n The number to round down.
     * @param {number} [precision=0] The precision to round down to.
     * @returns {number} Returns the rounded down number.
     * @example
     *
     * _.floor(4.006);
     * // => 4
     *
     * _.floor(0.046, 2);
     * // => 0.04
     *
     * _.floor(4060, -2);
     * // => 4000
     */
    var floor = createRound('floor');

    /**
     * Gets the maximum value of `collection`. If `collection` is empty or falsey
     * `-Infinity` is returned. If an iteratee function is provided it is invoked
     * for each value in `collection` to generate the criterion by which the value
     * is ranked. The `iteratee` is bound to `thisArg` and invoked with three
     * arguments: (value, index, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {*} Returns the maximum value.
     * @example
     *
     * _.max([4, 2, 8, 6]);
     * // => 8
     *
     * _.max([]);
     * // => -Infinity
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36 },
     *   { 'user': 'fred',   'age': 40 }
     * ];
     *
     * _.max(users, function(chr) {
     *   return chr.age;
     * });
     * // => { 'user': 'fred', 'age': 40 }
     *
     * // using the `_.property` callback shorthand
     * _.max(users, 'age');
     * // => { 'user': 'fred', 'age': 40 }
     */
    var max = createExtremum(gt, NEGATIVE_INFINITY);

    /**
     * Gets the minimum value of `collection`. If `collection` is empty or falsey
     * `Infinity` is returned. If an iteratee function is provided it is invoked
     * for each value in `collection` to generate the criterion by which the value
     * is ranked. The `iteratee` is bound to `thisArg` and invoked with three
     * arguments: (value, index, collection).
     *
     * If a property name is provided for `iteratee` the created `_.property`
     * style callback returns the property value of the given element.
     *
     * If a value is also provided for `thisArg` the created `_.matchesProperty`
     * style callback returns `true` for elements that have a matching property
     * value, else `false`.
     *
     * If an object is provided for `iteratee` the created `_.matches` style
     * callback returns `true` for elements that have the properties of the given
     * object, else `false`.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {*} Returns the minimum value.
     * @example
     *
     * _.min([4, 2, 8, 6]);
     * // => 2
     *
     * _.min([]);
     * // => Infinity
     *
     * var users = [
     *   { 'user': 'barney', 'age': 36 },
     *   { 'user': 'fred',   'age': 40 }
     * ];
     *
     * _.min(users, function(chr) {
     *   return chr.age;
     * });
     * // => { 'user': 'barney', 'age': 36 }
     *
     * // using the `_.property` callback shorthand
     * _.min(users, 'age');
     * // => { 'user': 'barney', 'age': 36 }
     */
    var min = createExtremum(lt, POSITIVE_INFINITY);

    /**
     * Calculates `n` rounded to `precision`.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {number} n The number to round.
     * @param {number} [precision=0] The precision to round to.
     * @returns {number} Returns the rounded number.
     * @example
     *
     * _.round(4.006);
     * // => 4
     *
     * _.round(4.006, 2);
     * // => 4.01
     *
     * _.round(4060, -2);
     * // => 4100
     */
    var round = createRound('round');

    /**
     * Gets the sum of the values in `collection`.
     *
     * @static
     * @memberOf _
     * @category Math
     * @param {Array|Object|string} collection The collection to iterate over.
     * @param {Function|Object|string} [iteratee] The function invoked per iteration.
     * @param {*} [thisArg] The `this` binding of `iteratee`.
     * @returns {number} Returns the sum.
     * @example
     *
     * _.sum([4, 6]);
     * // => 10
     *
     * _.sum({ 'a': 4, 'b': 6 });
     * // => 10
     *
     * var objects = [
     *   { 'n': 4 },
     *   { 'n': 6 }
     * ];
     *
     * _.sum(objects, function(object) {
     *   return object.n;
     * });
     * // => 10
     *
     * // using the `_.property` callback shorthand
     * _.sum(objects, 'n');
     * // => 10
     */
    function sum(collection, iteratee, thisArg) {
      if (thisArg && isIterateeCall(collection, iteratee, thisArg)) {
        iteratee = undefined;
      }
      iteratee = getCallback(iteratee, thisArg, 3);
      return iteratee.length == 1
        ? arraySum(isArray(collection) ? collection : toIterable(collection), iteratee)
        : baseSum(collection, iteratee);
    }

    /*------------------------------------------------------------------------*/

    // Ensure wrappers are instances of `baseLodash`.
    lodash.prototype = baseLodash.prototype;

    LodashWrapper.prototype = baseCreate(baseLodash.prototype);
    LodashWrapper.prototype.constructor = LodashWrapper;

    LazyWrapper.prototype = baseCreate(baseLodash.prototype);
    LazyWrapper.prototype.constructor = LazyWrapper;

    // Add functions to the `Map` cache.
    MapCache.prototype['delete'] = mapDelete;
    MapCache.prototype.get = mapGet;
    MapCache.prototype.has = mapHas;
    MapCache.prototype.set = mapSet;

    // Add functions to the `Set` cache.
    SetCache.prototype.push = cachePush;

    // Assign cache to `_.memoize`.
    memoize.Cache = MapCache;

    // Add functions that return wrapped values when chaining.
    lodash.after = after;
    lodash.ary = ary;
    lodash.assign = assign;
    lodash.at = at;
    lodash.before = before;
    lodash.bind = bind;
    lodash.bindAll = bindAll;
    lodash.bindKey = bindKey;
    lodash.callback = callback;
    lodash.chain = chain;
    lodash.chunk = chunk;
    lodash.compact = compact;
    lodash.constant = constant;
    lodash.countBy = countBy;
    lodash.create = create;
    lodash.curry = curry;
    lodash.curryRight = curryRight;
    lodash.debounce = debounce;
    lodash.defaults = defaults;
    lodash.defaultsDeep = defaultsDeep;
    lodash.defer = defer;
    lodash.delay = delay;
    lodash.difference = difference;
    lodash.drop = drop;
    lodash.dropRight = dropRight;
    lodash.dropRightWhile = dropRightWhile;
    lodash.dropWhile = dropWhile;
    lodash.fill = fill;
    lodash.filter = filter;
    lodash.flatten = flatten;
    lodash.flattenDeep = flattenDeep;
    lodash.flow = flow;
    lodash.flowRight = flowRight;
    lodash.forEach = forEach;
    lodash.forEachRight = forEachRight;
    lodash.forIn = forIn;
    lodash.forInRight = forInRight;
    lodash.forOwn = forOwn;
    lodash.forOwnRight = forOwnRight;
    lodash.functions = functions;
    lodash.groupBy = groupBy;
    lodash.indexBy = indexBy;
    lodash.initial = initial;
    lodash.intersection = intersection;
    lodash.invert = invert;
    lodash.invoke = invoke;
    lodash.keys = keys;
    lodash.keysIn = keysIn;
    lodash.map = map;
    lodash.mapKeys = mapKeys;
    lodash.mapValues = mapValues;
    lodash.matches = matches;
    lodash.matchesProperty = matchesProperty;
    lodash.memoize = memoize;
    lodash.merge = merge;
    lodash.method = method;
    lodash.methodOf = methodOf;
    lodash.mixin = mixin;
    lodash.modArgs = modArgs;
    lodash.negate = negate;
    lodash.omit = omit;
    lodash.once = once;
    lodash.pairs = pairs;
    lodash.partial = partial;
    lodash.partialRight = partialRight;
    lodash.partition = partition;
    lodash.pick = pick;
    lodash.pluck = pluck;
    lodash.property = property;
    lodash.propertyOf = propertyOf;
    lodash.pull = pull;
    lodash.pullAt = pullAt;
    lodash.range = range;
    lodash.rearg = rearg;
    lodash.reject = reject;
    lodash.remove = remove;
    lodash.rest = rest;
    lodash.restParam = restParam;
    lodash.set = set;
    lodash.shuffle = shuffle;
    lodash.slice = slice;
    lodash.sortBy = sortBy;
    lodash.sortByAll = sortByAll;
    lodash.sortByOrder = sortByOrder;
    lodash.spread = spread;
    lodash.take = take;
    lodash.takeRight = takeRight;
    lodash.takeRightWhile = takeRightWhile;
    lodash.takeWhile = takeWhile;
    lodash.tap = tap;
    lodash.throttle = throttle;
    lodash.thru = thru;
    lodash.times = times;
    lodash.toArray = toArray;
    lodash.toPlainObject = toPlainObject;
    lodash.transform = transform;
    lodash.union = union;
    lodash.uniq = uniq;
    lodash.unzip = unzip;
    lodash.unzipWith = unzipWith;
    lodash.values = values;
    lodash.valuesIn = valuesIn;
    lodash.where = where;
    lodash.without = without;
    lodash.wrap = wrap;
    lodash.xor = xor;
    lodash.zip = zip;
    lodash.zipObject = zipObject;
    lodash.zipWith = zipWith;

    // Add aliases.
    lodash.backflow = flowRight;
    lodash.collect = map;
    lodash.compose = flowRight;
    lodash.each = forEach;
    lodash.eachRight = forEachRight;
    lodash.extend = assign;
    lodash.iteratee = callback;
    lodash.methods = functions;
    lodash.object = zipObject;
    lodash.select = filter;
    lodash.tail = rest;
    lodash.unique = uniq;

    // Add functions to `lodash.prototype`.
    mixin(lodash, lodash);

    /*------------------------------------------------------------------------*/

    // Add functions that return unwrapped values when chaining.
    lodash.add = add;
    lodash.attempt = attempt;
    lodash.camelCase = camelCase;
    lodash.capitalize = capitalize;
    lodash.ceil = ceil;
    lodash.clone = clone;
    lodash.cloneDeep = cloneDeep;
    lodash.deburr = deburr;
    lodash.endsWith = endsWith;
    lodash.escape = escape;
    lodash.escapeRegExp = escapeRegExp;
    lodash.every = every;
    lodash.find = find;
    lodash.findIndex = findIndex;
    lodash.findKey = findKey;
    lodash.findLast = findLast;
    lodash.findLastIndex = findLastIndex;
    lodash.findLastKey = findLastKey;
    lodash.findWhere = findWhere;
    lodash.first = first;
    lodash.floor = floor;
    lodash.get = get;
    lodash.gt = gt;
    lodash.gte = gte;
    lodash.has = has;
    lodash.identity = identity;
    lodash.includes = includes;
    lodash.indexOf = indexOf;
    lodash.inRange = inRange;
    lodash.isArguments = isArguments;
    lodash.isArray = isArray;
    lodash.isBoolean = isBoolean;
    lodash.isDate = isDate;
    lodash.isElement = isElement;
    lodash.isEmpty = isEmpty;
    lodash.isEqual = isEqual;
    lodash.isError = isError;
    lodash.isFinite = isFinite;
    lodash.isFunction = isFunction;
    lodash.isMatch = isMatch;
    lodash.isNaN = isNaN;
    lodash.isNative = isNative;
    lodash.isNull = isNull;
    lodash.isNumber = isNumber;
    lodash.isObject = isObject;
    lodash.isPlainObject = isPlainObject;
    lodash.isRegExp = isRegExp;
    lodash.isString = isString;
    lodash.isTypedArray = isTypedArray;
    lodash.isUndefined = isUndefined;
    lodash.kebabCase = kebabCase;
    lodash.last = last;
    lodash.lastIndexOf = lastIndexOf;
    lodash.lt = lt;
    lodash.lte = lte;
    lodash.max = max;
    lodash.min = min;
    lodash.noConflict = noConflict;
    lodash.noop = noop;
    lodash.now = now;
    lodash.pad = pad;
    lodash.padLeft = padLeft;
    lodash.padRight = padRight;
    lodash.parseInt = parseInt;
    lodash.random = random;
    lodash.reduce = reduce;
    lodash.reduceRight = reduceRight;
    lodash.repeat = repeat;
    lodash.result = result;
    lodash.round = round;
    lodash.runInContext = runInContext;
    lodash.size = size;
    lodash.snakeCase = snakeCase;
    lodash.some = some;
    lodash.sortedIndex = sortedIndex;
    lodash.sortedLastIndex = sortedLastIndex;
    lodash.startCase = startCase;
    lodash.startsWith = startsWith;
    lodash.sum = sum;
    lodash.template = template;
    lodash.trim = trim;
    lodash.trimLeft = trimLeft;
    lodash.trimRight = trimRight;
    lodash.trunc = trunc;
    lodash.unescape = unescape;
    lodash.uniqueId = uniqueId;
    lodash.words = words;

    // Add aliases.
    lodash.all = every;
    lodash.any = some;
    lodash.contains = includes;
    lodash.eq = isEqual;
    lodash.detect = find;
    lodash.foldl = reduce;
    lodash.foldr = reduceRight;
    lodash.head = first;
    lodash.include = includes;
    lodash.inject = reduce;

    mixin(lodash, (function() {
      var source = {};
      baseForOwn(lodash, function(func, methodName) {
        if (!lodash.prototype[methodName]) {
          source[methodName] = func;
        }
      });
      return source;
    }()), false);

    /*------------------------------------------------------------------------*/

    // Add functions capable of returning wrapped and unwrapped values when chaining.
    lodash.sample = sample;

    lodash.prototype.sample = function(n) {
      if (!this.__chain__ && n == null) {
        return sample(this.value());
      }
      return this.thru(function(value) {
        return sample(value, n);
      });
    };

    /*------------------------------------------------------------------------*/

    /**
     * The semantic version number.
     *
     * @static
     * @memberOf _
     * @type string
     */
    lodash.VERSION = VERSION;

    // Assign default placeholders.
    arrayEach(['bind', 'bindKey', 'curry', 'curryRight', 'partial', 'partialRight'], function(methodName) {
      lodash[methodName].placeholder = lodash;
    });

    // Add `LazyWrapper` methods for `_.drop` and `_.take` variants.
    arrayEach(['drop', 'take'], function(methodName, index) {
      LazyWrapper.prototype[methodName] = function(n) {
        var filtered = this.__filtered__;
        if (filtered && !index) {
          return new LazyWrapper(this);
        }
        n = n == null ? 1 : nativeMax(nativeFloor(n) || 0, 0);

        var result = this.clone();
        if (filtered) {
          result.__takeCount__ = nativeMin(result.__takeCount__, n);
        } else {
          result.__views__.push({ 'size': n, 'type': methodName + (result.__dir__ < 0 ? 'Right' : '') });
        }
        return result;
      };

      LazyWrapper.prototype[methodName + 'Right'] = function(n) {
        return this.reverse()[methodName](n).reverse();
      };
    });

    // Add `LazyWrapper` methods that accept an `iteratee` value.
    arrayEach(['filter', 'map', 'takeWhile'], function(methodName, index) {
      var type = index + 1,
          isFilter = type != LAZY_MAP_FLAG;

      LazyWrapper.prototype[methodName] = function(iteratee, thisArg) {
        var result = this.clone();
        result.__iteratees__.push({ 'iteratee': getCallback(iteratee, thisArg, 1), 'type': type });
        result.__filtered__ = result.__filtered__ || isFilter;
        return result;
      };
    });

    // Add `LazyWrapper` methods for `_.first` and `_.last`.
    arrayEach(['first', 'last'], function(methodName, index) {
      var takeName = 'take' + (index ? 'Right' : '');

      LazyWrapper.prototype[methodName] = function() {
        return this[takeName](1).value()[0];
      };
    });

    // Add `LazyWrapper` methods for `_.initial` and `_.rest`.
    arrayEach(['initial', 'rest'], function(methodName, index) {
      var dropName = 'drop' + (index ? '' : 'Right');

      LazyWrapper.prototype[methodName] = function() {
        return this.__filtered__ ? new LazyWrapper(this) : this[dropName](1);
      };
    });

    // Add `LazyWrapper` methods for `_.pluck` and `_.where`.
    arrayEach(['pluck', 'where'], function(methodName, index) {
      var operationName = index ? 'filter' : 'map',
          createCallback = index ? baseMatches : property;

      LazyWrapper.prototype[methodName] = function(value) {
        return this[operationName](createCallback(value));
      };
    });

    LazyWrapper.prototype.compact = function() {
      return this.filter(identity);
    };

    LazyWrapper.prototype.reject = function(predicate, thisArg) {
      predicate = getCallback(predicate, thisArg, 1);
      return this.filter(function(value) {
        return !predicate(value);
      });
    };

    LazyWrapper.prototype.slice = function(start, end) {
      start = start == null ? 0 : (+start || 0);

      var result = this;
      if (result.__filtered__ && (start > 0 || end < 0)) {
        return new LazyWrapper(result);
      }
      if (start < 0) {
        result = result.takeRight(-start);
      } else if (start) {
        result = result.drop(start);
      }
      if (end !== undefined) {
        end = (+end || 0);
        result = end < 0 ? result.dropRight(-end) : result.take(end - start);
      }
      return result;
    };

    LazyWrapper.prototype.takeRightWhile = function(predicate, thisArg) {
      return this.reverse().takeWhile(predicate, thisArg).reverse();
    };

    LazyWrapper.prototype.toArray = function() {
      return this.take(POSITIVE_INFINITY);
    };

    // Add `LazyWrapper` methods to `lodash.prototype`.
    baseForOwn(LazyWrapper.prototype, function(func, methodName) {
      var checkIteratee = /^(?:filter|map|reject)|While$/.test(methodName),
          retUnwrapped = /^(?:first|last)$/.test(methodName),
          lodashFunc = lodash[retUnwrapped ? ('take' + (methodName == 'last' ? 'Right' : '')) : methodName];

      if (!lodashFunc) {
        return;
      }
      lodash.prototype[methodName] = function() {
        var args = retUnwrapped ? [1] : arguments,
            chainAll = this.__chain__,
            value = this.__wrapped__,
            isHybrid = !!this.__actions__.length,
            isLazy = value instanceof LazyWrapper,
            iteratee = args[0],
            useLazy = isLazy || isArray(value);

        if (useLazy && checkIteratee && typeof iteratee == 'function' && iteratee.length != 1) {
          // Avoid lazy use if the iteratee has a "length" value other than `1`.
          isLazy = useLazy = false;
        }
        var interceptor = function(value) {
          return (retUnwrapped && chainAll)
            ? lodashFunc(value, 1)[0]
            : lodashFunc.apply(undefined, arrayPush([value], args));
        };

        var action = { 'func': thru, 'args': [interceptor], 'thisArg': undefined },
            onlyLazy = isLazy && !isHybrid;

        if (retUnwrapped && !chainAll) {
          if (onlyLazy) {
            value = value.clone();
            value.__actions__.push(action);
            return func.call(value);
          }
          return lodashFunc.call(undefined, this.value())[0];
        }
        if (!retUnwrapped && useLazy) {
          value = onlyLazy ? value : new LazyWrapper(this);
          var result = func.apply(value, args);
          result.__actions__.push(action);
          return new LodashWrapper(result, chainAll);
        }
        return this.thru(interceptor);
      };
    });

    // Add `Array` and `String` methods to `lodash.prototype`.
    arrayEach(['join', 'pop', 'push', 'replace', 'shift', 'sort', 'splice', 'split', 'unshift'], function(methodName) {
      var func = (/^(?:replace|split)$/.test(methodName) ? stringProto : arrayProto)[methodName],
          chainName = /^(?:push|sort|unshift)$/.test(methodName) ? 'tap' : 'thru',
          retUnwrapped = /^(?:join|pop|replace|shift)$/.test(methodName);

      lodash.prototype[methodName] = function() {
        var args = arguments;
        if (retUnwrapped && !this.__chain__) {
          return func.apply(this.value(), args);
        }
        return this[chainName](function(value) {
          return func.apply(value, args);
        });
      };
    });

    // Map minified function names to their real names.
    baseForOwn(LazyWrapper.prototype, function(func, methodName) {
      var lodashFunc = lodash[methodName];
      if (lodashFunc) {
        var key = lodashFunc.name,
            names = realNames[key] || (realNames[key] = []);

        names.push({ 'name': methodName, 'func': lodashFunc });
      }
    });

    realNames[createHybridWrapper(undefined, BIND_KEY_FLAG).name] = [{ 'name': 'wrapper', 'func': undefined }];

    // Add functions to the lazy wrapper.
    LazyWrapper.prototype.clone = lazyClone;
    LazyWrapper.prototype.reverse = lazyReverse;
    LazyWrapper.prototype.value = lazyValue;

    // Add chaining functions to the `lodash` wrapper.
    lodash.prototype.chain = wrapperChain;
    lodash.prototype.commit = wrapperCommit;
    lodash.prototype.concat = wrapperConcat;
    lodash.prototype.plant = wrapperPlant;
    lodash.prototype.reverse = wrapperReverse;
    lodash.prototype.toString = wrapperToString;
    lodash.prototype.run = lodash.prototype.toJSON = lodash.prototype.valueOf = lodash.prototype.value = wrapperValue;

    // Add function aliases to the `lodash` wrapper.
    lodash.prototype.collect = lodash.prototype.map;
    lodash.prototype.head = lodash.prototype.first;
    lodash.prototype.select = lodash.prototype.filter;
    lodash.prototype.tail = lodash.prototype.rest;

    return lodash;
  }

  /*--------------------------------------------------------------------------*/

  // Export lodash.
  var _ = runInContext();

  // Some AMD build optimizers like r.js check for condition patterns like the following:
  if (typeof define == 'function' && typeof define.amd == 'object' && define.amd) {
    // Expose lodash to the global object when an AMD loader is present to avoid
    // errors in cases where lodash is loaded by a script tag and not intended
    // as an AMD module. See http://requirejs.org/docs/errors.html#mismatch for
    // more details.
    root._ = _;

    // Define as an anonymous module so, through path mapping, it can be
    // referenced as the "underscore" module.
    define(function() {
      return _;
    });
  }
  // Check for `exports` after `define` in case a build optimizer adds an `exports` object.
  else if (freeExports && freeModule) {
    // Export for Node.js or RingoJS.
    if (moduleExports) {
      (freeModule.exports = _)._ = _;
    }
    // Export for Rhino with CommonJS support.
    else {
      freeExports._ = _;
    }
  }
  else {
    // Export for a browser or Rhino.
    root._ = _;
  }
}.call(this));

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{}],6:[function(require,module,exports){

},{}],7:[function(require,module,exports){
(function (global){
/*!
 * The buffer module from node.js, for the browser.
 *
 * @author   Feross Aboukhadijeh <feross@feross.org> <http://feross.org>
 * @license  MIT
 */
/* eslint-disable no-proto */

'use strict'

var base64 = require('base64-js')
var ieee754 = require('ieee754')
var isArray = require('isarray')

exports.Buffer = Buffer
exports.SlowBuffer = SlowBuffer
exports.INSPECT_MAX_BYTES = 50
Buffer.poolSize = 8192 // not used by this implementation

var rootParent = {}

/**
 * If `Buffer.TYPED_ARRAY_SUPPORT`:
 *   === true    Use Uint8Array implementation (fastest)
 *   === false   Use Object implementation (most compatible, even IE6)
 *
 * Browsers that support typed arrays are IE 10+, Firefox 4+, Chrome 7+, Safari 5.1+,
 * Opera 11.6+, iOS 4.2+.
 *
 * Due to various browser bugs, sometimes the Object implementation will be used even
 * when the browser supports typed arrays.
 *
 * Note:
 *
 *   - Firefox 4-29 lacks support for adding new properties to `Uint8Array` instances,
 *     See: https://bugzilla.mozilla.org/show_bug.cgi?id=695438.
 *
 *   - Safari 5-7 lacks support for changing the `Object.prototype.constructor` property
 *     on objects.
 *
 *   - Chrome 9-10 is missing the `TypedArray.prototype.subarray` function.
 *
 *   - IE10 has a broken `TypedArray.prototype.subarray` function which returns arrays of
 *     incorrect length in some situations.

 * We detect these buggy browsers and set `Buffer.TYPED_ARRAY_SUPPORT` to `false` so they
 * get the Object implementation, which is slower but behaves correctly.
 */
Buffer.TYPED_ARRAY_SUPPORT = global.TYPED_ARRAY_SUPPORT !== undefined
  ? global.TYPED_ARRAY_SUPPORT
  : typedArraySupport()

function typedArraySupport () {
  function Bar () {}
  try {
    var arr = new Uint8Array(1)
    arr.foo = function () { return 42 }
    arr.constructor = Bar
    return arr.foo() === 42 && // typed array instances can be augmented
        arr.constructor === Bar && // constructor can be set
        typeof arr.subarray === 'function' && // chrome 9-10 lack `subarray`
        arr.subarray(1, 1).byteLength === 0 // ie10 has broken `subarray`
  } catch (e) {
    return false
  }
}

function kMaxLength () {
  return Buffer.TYPED_ARRAY_SUPPORT
    ? 0x7fffffff
    : 0x3fffffff
}

/**
 * Class: Buffer
 * =============
 *
 * The Buffer constructor returns instances of `Uint8Array` that are augmented
 * with function properties for all the node `Buffer` API functions. We use
 * `Uint8Array` so that square bracket notation works as expected -- it returns
 * a single octet.
 *
 * By augmenting the instances, we can avoid modifying the `Uint8Array`
 * prototype.
 */
function Buffer (arg) {
  if (!(this instanceof Buffer)) {
    // Avoid going through an ArgumentsAdaptorTrampoline in the common case.
    if (arguments.length > 1) return new Buffer(arg, arguments[1])
    return new Buffer(arg)
  }

  if (!Buffer.TYPED_ARRAY_SUPPORT) {
    this.length = 0
    this.parent = undefined
  }

  // Common case.
  if (typeof arg === 'number') {
    return fromNumber(this, arg)
  }

  // Slightly less common case.
  if (typeof arg === 'string') {
    return fromString(this, arg, arguments.length > 1 ? arguments[1] : 'utf8')
  }

  // Unusual.
  return fromObject(this, arg)
}

Buff = Buffer;

function fromNumber (that, length) {
  that = allocate(that, length < 0 ? 0 : checked(length) | 0)
  if (!Buffer.TYPED_ARRAY_SUPPORT) {
    for (var i = 0; i < length; i++) {
      that[i] = 0
    }
  }
  return that
}

function fromString (that, string, encoding) {
  if (typeof encoding !== 'string' || encoding === '') encoding = 'utf8'

  // Assumption: byteLength() return value is always < kMaxLength.
  var length = byteLength(string, encoding) | 0
  that = allocate(that, length)

  that.write(string, encoding)
  return that
}

function fromObject (that, object) {
  if (Buffer.isBuffer(object)) return fromBuffer(that, object)

  if (isArray(object)) return fromArray(that, object)

  if (object == null) {
    throw new TypeError('must start with number, buffer, array or string')
  }

  if (typeof ArrayBuffer !== 'undefined') {
    if (object.buffer instanceof ArrayBuffer) {
      return fromTypedArray(that, object)
    }
    if (object instanceof ArrayBuffer) {
      return fromArrayBuffer(that, object)
    }
  }

  if (object.length) return fromArrayLike(that, object)

  return fromJsonObject(that, object)
}

function fromBuffer (that, buffer) {
  var length = checked(buffer.length) | 0
  that = allocate(that, length)
  buffer.copy(that, 0, 0, length)
  return that
}

function fromArray (that, array) {
  var length = checked(array.length) | 0
  that = allocate(that, length)
  for (var i = 0; i < length; i += 1) {
    that[i] = array[i] & 255
  }
  return that
}

// Duplicate of fromArray() to keep fromArray() monomorphic.
function fromTypedArray (that, array) {
  var length = checked(array.length) | 0
  that = allocate(that, length)
  // Truncating the elements is probably not what people expect from typed
  // arrays with BYTES_PER_ELEMENT > 1 but it's compatible with the behavior
  // of the old Buffer constructor.
  for (var i = 0; i < length; i += 1) {
    that[i] = array[i] & 255
  }
  return that
}

function fromArrayBuffer (that, array) {
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    // Return an augmented `Uint8Array` instance, for best performance
    array.byteLength
    that = Buffer._augment(new Uint8Array(array))
  } else {
    // Fallback: Return an object instance of the Buffer class
    that = fromTypedArray(that, new Uint8Array(array))
  }
  return that
}

function fromArrayLike (that, array) {
  var length = checked(array.length) | 0
  that = allocate(that, length)
  for (var i = 0; i < length; i += 1) {
    that[i] = array[i] & 255
  }
  return that
}

// Deserialize { type: 'Buffer', data: [1,2,3,...] } into a Buffer object.
// Returns a zero-length buffer for inputs that don't conform to the spec.
function fromJsonObject (that, object) {
  var array
  var length = 0

  if (object.type === 'Buffer' && isArray(object.data)) {
    array = object.data
    length = checked(array.length) | 0
  }
  that = allocate(that, length)

  for (var i = 0; i < length; i += 1) {
    that[i] = array[i] & 255
  }
  return that
}

if (Buffer.TYPED_ARRAY_SUPPORT) {
  Buffer.prototype.__proto__ = Uint8Array.prototype
  Buffer.__proto__ = Uint8Array
} else {
  // pre-set for values that may exist in the future
  Buffer.prototype.length = undefined
  Buffer.prototype.parent = undefined
}

function allocate (that, length) {
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    // Return an augmented `Uint8Array` instance, for best performance
    that = Buffer._augment(new Uint8Array(length))
    that.__proto__ = Buffer.prototype
  } else {
    // Fallback: Return an object instance of the Buffer class
    that.length = length
    that._isBuffer = true
  }

  var fromPool = length !== 0 && length <= Buffer.poolSize >>> 1
  if (fromPool) that.parent = rootParent

  return that
}

function checked (length) {
  // Note: cannot use `length < kMaxLength` here because that fails when
  // length is NaN (which is otherwise coerced to zero.)
  if (length >= kMaxLength()) {
    throw new RangeError('Attempt to allocate Buffer larger than maximum ' +
                         'size: 0x' + kMaxLength().toString(16) + ' bytes')
  }
  return length | 0
}

function SlowBuffer (subject, encoding) {
  if (!(this instanceof SlowBuffer)) return new SlowBuffer(subject, encoding)

  var buf = new Buffer(subject, encoding)
  delete buf.parent
  return buf
}

Buffer.isBuffer = function isBuffer (b) {
  return !!(b != null && b._isBuffer)
}

Buffer.compare = function compare (a, b) {
  if (!Buffer.isBuffer(a) || !Buffer.isBuffer(b)) {
    throw new TypeError('Arguments must be Buffers')
  }

  if (a === b) return 0

  var x = a.length
  var y = b.length

  var i = 0
  var len = Math.min(x, y)
  while (i < len) {
    if (a[i] !== b[i]) break

    ++i
  }

  if (i !== len) {
    x = a[i]
    y = b[i]
  }

  if (x < y) return -1
  if (y < x) return 1
  return 0
}

Buffer.isEncoding = function isEncoding (encoding) {
  switch (String(encoding).toLowerCase()) {
    case 'hex':
    case 'utf8':
    case 'utf-8':
    case 'ascii':
    case 'binary':
    case 'base64':
    case 'raw':
    case 'ucs2':
    case 'ucs-2':
    case 'utf16le':
    case 'utf-16le':
      return true
    default:
      return false
  }
}

Buffer.concat = function concat (list, length) {
  if (!isArray(list)) throw new TypeError('list argument must be an Array of Buffers.')

  if (list.length === 0) {
    return new Buffer(0)
  }

  var i
  if (length === undefined) {
    length = 0
    for (i = 0; i < list.length; i++) {
      length += list[i].length
    }
  }

  var buf = new Buffer(length)
  var pos = 0
  for (i = 0; i < list.length; i++) {
    var item = list[i]
    item.copy(buf, pos)
    pos += item.length
  }
  return buf
}

function byteLength (string, encoding) {
  if (typeof string !== 'string') string = '' + string

  var len = string.length
  if (len === 0) return 0

  // Use a for loop to avoid recursion
  var loweredCase = false
  for (;;) {
    switch (encoding) {
      case 'ascii':
      case 'binary':
      // Deprecated
      case 'raw':
      case 'raws':
        return len
      case 'utf8':
      case 'utf-8':
        return utf8ToBytes(string).length
      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return len * 2
      case 'hex':
        return len >>> 1
      case 'base64':
        return base64ToBytes(string).length
      default:
        if (loweredCase) return utf8ToBytes(string).length // assume utf8
        encoding = ('' + encoding).toLowerCase()
        loweredCase = true
    }
  }
}
Buffer.byteLength = byteLength

function slowToString (encoding, start, end) {
  var loweredCase = false

  start = start | 0
  end = end === undefined || end === Infinity ? this.length : end | 0

  if (!encoding) encoding = 'utf8'
  if (start < 0) start = 0
  if (end > this.length) end = this.length
  if (end <= start) return ''

  while (true) {
    switch (encoding) {
      case 'hex':
        return hexSlice(this, start, end)

      case 'utf8':
      case 'utf-8':
        return utf8Slice(this, start, end)

      case 'ascii':
        return asciiSlice(this, start, end)

      case 'binary':
        return binarySlice(this, start, end)

      case 'base64':
        return base64Slice(this, start, end)

      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return utf16leSlice(this, start, end)

      default:
        if (loweredCase) throw new TypeError('Unknown encoding: ' + encoding)
        encoding = (encoding + '').toLowerCase()
        loweredCase = true
    }
  }
}

Buffer.prototype.toString = function toString () {
  var length = this.length | 0
  if (length === 0) return ''
  if (arguments.length === 0) return utf8Slice(this, 0, length)
  return slowToString.apply(this, arguments)
}

Buffer.prototype.equals = function equals (b) {
  if (!Buffer.isBuffer(b)) throw new TypeError('Argument must be a Buffer')
  if (this === b) return true
  return Buffer.compare(this, b) === 0
}

Buffer.prototype.inspect = function inspect () {
  var str = ''
  var max = exports.INSPECT_MAX_BYTES
  if (this.length > 0) {
    str = this.toString('hex', 0, max).match(/.{2}/g).join(' ')
    if (this.length > max) str += ' ... '
  }
  return '<Buffer ' + str + '>'
}

Buffer.prototype.compare = function compare (b) {
  if (!Buffer.isBuffer(b)) throw new TypeError('Argument must be a Buffer')
  if (this === b) return 0
  return Buffer.compare(this, b)
}

Buffer.prototype.indexOf = function indexOf (val, byteOffset) {
  if (byteOffset > 0x7fffffff) byteOffset = 0x7fffffff
  else if (byteOffset < -0x80000000) byteOffset = -0x80000000
  byteOffset >>= 0

  if (this.length === 0) return -1
  if (byteOffset >= this.length) return -1

  // Negative offsets start from the end of the buffer
  if (byteOffset < 0) byteOffset = Math.max(this.length + byteOffset, 0)

  if (typeof val === 'string') {
    if (val.length === 0) return -1 // special case: looking for empty string always fails
    return String.prototype.indexOf.call(this, val, byteOffset)
  }
  if (Buffer.isBuffer(val)) {
    return arrayIndexOf(this, val, byteOffset)
  }
  if (typeof val === 'number') {
    if (Buffer.TYPED_ARRAY_SUPPORT && Uint8Array.prototype.indexOf === 'function') {
      return Uint8Array.prototype.indexOf.call(this, val, byteOffset)
    }
    return arrayIndexOf(this, [ val ], byteOffset)
  }

  function arrayIndexOf (arr, val, byteOffset) {
    var foundIndex = -1
    for (var i = 0; byteOffset + i < arr.length; i++) {
      if (arr[byteOffset + i] === val[foundIndex === -1 ? 0 : i - foundIndex]) {
        if (foundIndex === -1) foundIndex = i
        if (i - foundIndex + 1 === val.length) return byteOffset + foundIndex
      } else {
        foundIndex = -1
      }
    }
    return -1
  }

  throw new TypeError('val must be string, number or Buffer')
}

// `get` is deprecated
Buffer.prototype.get = function get (offset) {
  console.log('.get() is deprecated. Access using array indexes instead.')
  return this.readUInt8(offset)
}

// `set` is deprecated
Buffer.prototype.set = function set (v, offset) {
  console.log('.set() is deprecated. Access using array indexes instead.')
  return this.writeUInt8(v, offset)
}

function hexWrite (buf, string, offset, length) {
  offset = Number(offset) || 0
  var remaining = buf.length - offset
  if (!length) {
    length = remaining
  } else {
    length = Number(length)
    if (length > remaining) {
      length = remaining
    }
  }

  // must be an even number of digits
  var strLen = string.length
  if (strLen % 2 !== 0) throw new Error('Invalid hex string')

  if (length > strLen / 2) {
    length = strLen / 2
  }
  for (var i = 0; i < length; i++) {
    var parsed = parseInt(string.substr(i * 2, 2), 16)
    if (isNaN(parsed)) throw new Error('Invalid hex string')
    buf[offset + i] = parsed
  }
  return i
}

function utf8Write (buf, string, offset, length) {
  return blitBuffer(utf8ToBytes(string, buf.length - offset), buf, offset, length)
}

function asciiWrite (buf, string, offset, length) {
  return blitBuffer(asciiToBytes(string), buf, offset, length)
}

function binaryWrite (buf, string, offset, length) {
  return asciiWrite(buf, string, offset, length)
}

function base64Write (buf, string, offset, length) {
  return blitBuffer(base64ToBytes(string), buf, offset, length)
}

function ucs2Write (buf, string, offset, length) {
  return blitBuffer(utf16leToBytes(string, buf.length - offset), buf, offset, length)
}

Buffer.prototype.write = function write (string, offset, length, encoding) {
  // Buffer#write(string)
  if (offset === undefined) {
    encoding = 'utf8'
    length = this.length
    offset = 0
  // Buffer#write(string, encoding)
  } else if (length === undefined && typeof offset === 'string') {
    encoding = offset
    length = this.length
    offset = 0
  // Buffer#write(string, offset[, length][, encoding])
  } else if (isFinite(offset)) {
    offset = offset | 0
    if (isFinite(length)) {
      length = length | 0
      if (encoding === undefined) encoding = 'utf8'
    } else {
      encoding = length
      length = undefined
    }
  // legacy write(string, encoding, offset, length) - remove in v0.13
  } else {
    var swap = encoding
    encoding = offset
    offset = length | 0
    length = swap
  }

  var remaining = this.length - offset
  if (length === undefined || length > remaining) length = remaining

  if ((string.length > 0 && (length < 0 || offset < 0)) || offset > this.length) {
    throw new RangeError('attempt to write outside buffer bounds')
  }

  if (!encoding) encoding = 'utf8'

  var loweredCase = false
  for (;;) {
    switch (encoding) {
      case 'hex':
        return hexWrite(this, string, offset, length)

      case 'utf8':
      case 'utf-8':
        return utf8Write(this, string, offset, length)

      case 'ascii':
        return asciiWrite(this, string, offset, length)

      case 'binary':
        return binaryWrite(this, string, offset, length)

      case 'base64':
        // Warning: maxLength not taken into account in base64Write
        return base64Write(this, string, offset, length)

      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return ucs2Write(this, string, offset, length)

      default:
        if (loweredCase) throw new TypeError('Unknown encoding: ' + encoding)
        encoding = ('' + encoding).toLowerCase()
        loweredCase = true
    }
  }
}

Buffer.prototype.toJSON = function toJSON () {
  return {
    type: 'Buffer',
    data: Array.prototype.slice.call(this._arr || this, 0)
  }
}

function base64Slice (buf, start, end) {
  if (start === 0 && end === buf.length) {
    return base64.fromByteArray(buf)
  } else {
    return base64.fromByteArray(buf.slice(start, end))
  }
}

function utf8Slice (buf, start, end) {
  end = Math.min(buf.length, end)
  var res = []

  var i = start
  while (i < end) {
    var firstByte = buf[i]
    var codePoint = null
    var bytesPerSequence = (firstByte > 0xEF) ? 4
      : (firstByte > 0xDF) ? 3
      : (firstByte > 0xBF) ? 2
      : 1

    if (i + bytesPerSequence <= end) {
      var secondByte, thirdByte, fourthByte, tempCodePoint

      switch (bytesPerSequence) {
        case 1:
          if (firstByte < 0x80) {
            codePoint = firstByte
          }
          break
        case 2:
          secondByte = buf[i + 1]
          if ((secondByte & 0xC0) === 0x80) {
            tempCodePoint = (firstByte & 0x1F) << 0x6 | (secondByte & 0x3F)
            if (tempCodePoint > 0x7F) {
              codePoint = tempCodePoint
            }
          }
          break
        case 3:
          secondByte = buf[i + 1]
          thirdByte = buf[i + 2]
          if ((secondByte & 0xC0) === 0x80 && (thirdByte & 0xC0) === 0x80) {
            tempCodePoint = (firstByte & 0xF) << 0xC | (secondByte & 0x3F) << 0x6 | (thirdByte & 0x3F)
            if (tempCodePoint > 0x7FF && (tempCodePoint < 0xD800 || tempCodePoint > 0xDFFF)) {
              codePoint = tempCodePoint
            }
          }
          break
        case 4:
          secondByte = buf[i + 1]
          thirdByte = buf[i + 2]
          fourthByte = buf[i + 3]
          if ((secondByte & 0xC0) === 0x80 && (thirdByte & 0xC0) === 0x80 && (fourthByte & 0xC0) === 0x80) {
            tempCodePoint = (firstByte & 0xF) << 0x12 | (secondByte & 0x3F) << 0xC | (thirdByte & 0x3F) << 0x6 | (fourthByte & 0x3F)
            if (tempCodePoint > 0xFFFF && tempCodePoint < 0x110000) {
              codePoint = tempCodePoint
            }
          }
      }
    }

    if (codePoint === null) {
      // we did not generate a valid codePoint so insert a
      // replacement char (U+FFFD) and advance only 1 byte
      codePoint = 0xFFFD
      bytesPerSequence = 1
    } else if (codePoint > 0xFFFF) {
      // encode to utf16 (surrogate pair dance)
      codePoint -= 0x10000
      res.push(codePoint >>> 10 & 0x3FF | 0xD800)
      codePoint = 0xDC00 | codePoint & 0x3FF
    }

    res.push(codePoint)
    i += bytesPerSequence
  }

  return decodeCodePointsArray(res)
}

// Based on http://stackoverflow.com/a/22747272/680742, the browser with
// the lowest limit is Chrome, with 0x10000 args.
// We go 1 magnitude less, for safety
var MAX_ARGUMENTS_LENGTH = 0x1000

function decodeCodePointsArray (codePoints) {
  var len = codePoints.length
  if (len <= MAX_ARGUMENTS_LENGTH) {
    return String.fromCharCode.apply(String, codePoints) // avoid extra slice()
  }

  // Decode in chunks to avoid "call stack size exceeded".
  var res = ''
  var i = 0
  while (i < len) {
    res += String.fromCharCode.apply(
      String,
      codePoints.slice(i, i += MAX_ARGUMENTS_LENGTH)
    )
  }
  return res
}

function asciiSlice (buf, start, end) {
  var ret = ''
  end = Math.min(buf.length, end)

  for (var i = start; i < end; i++) {
    ret += String.fromCharCode(buf[i] & 0x7F)
  }
  return ret
}

function binarySlice (buf, start, end) {
  var ret = ''
  end = Math.min(buf.length, end)

  for (var i = start; i < end; i++) {
    ret += String.fromCharCode(buf[i])
  }
  return ret
}

function hexSlice (buf, start, end) {
  var len = buf.length

  if (!start || start < 0) start = 0
  if (!end || end < 0 || end > len) end = len

  var out = ''
  for (var i = start; i < end; i++) {
    out += toHex(buf[i])
  }
  return out
}

function utf16leSlice (buf, start, end) {
  var bytes = buf.slice(start, end)
  var res = ''
  for (var i = 0; i < bytes.length; i += 2) {
    res += String.fromCharCode(bytes[i] + bytes[i + 1] * 256)
  }
  return res
}

Buffer.prototype.slice = function slice (start, end) {
  var len = this.length
  start = ~~start
  end = end === undefined ? len : ~~end

  if (start < 0) {
    start += len
    if (start < 0) start = 0
  } else if (start > len) {
    start = len
  }

  if (end < 0) {
    end += len
    if (end < 0) end = 0
  } else if (end > len) {
    end = len
  }

  if (end < start) end = start

  var newBuf
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    newBuf = Buffer._augment(this.subarray(start, end))
  } else {
    var sliceLen = end - start
    newBuf = new Buffer(sliceLen, undefined)
    for (var i = 0; i < sliceLen; i++) {
      newBuf[i] = this[i + start]
    }
  }

  if (newBuf.length) newBuf.parent = this.parent || this

  return newBuf
}

/*
 * Need to make sure that buffer isn't trying to write out of bounds.
 */
function checkOffset (offset, ext, length) {
  if ((offset % 1) !== 0 || offset < 0) throw new RangeError('offset is not uint')
  if (offset + ext > length) throw new RangeError('Trying to access beyond buffer length')
}

Buffer.prototype.readUIntLE = function readUIntLE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkOffset(offset, byteLength, this.length)

  var val = this[offset]
  var mul = 1
  var i = 0
  while (++i < byteLength && (mul *= 0x100)) {
    val += this[offset + i] * mul
  }

  return val
}

Buffer.prototype.readUIntBE = function readUIntBE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) {
    checkOffset(offset, byteLength, this.length)
  }

  var val = this[offset + --byteLength]
  var mul = 1
  while (byteLength > 0 && (mul *= 0x100)) {
    val += this[offset + --byteLength] * mul
  }

  return val
}

Buffer.prototype.readUInt8 = function readUInt8 (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 1, this.length)
  return this[offset]
}

Buffer.prototype.readUInt16LE = function readUInt16LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  return this[offset] | (this[offset + 1] << 8)
}

Buffer.prototype.readUInt16BE = function readUInt16BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  return (this[offset] << 8) | this[offset + 1]
}

Buffer.prototype.readUInt32LE = function readUInt32LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return ((this[offset]) |
      (this[offset + 1] << 8) |
      (this[offset + 2] << 16)) +
      (this[offset + 3] * 0x1000000)
}

Buffer.prototype.readUInt32BE = function readUInt32BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return (this[offset] * 0x1000000) +
    ((this[offset + 1] << 16) |
    (this[offset + 2] << 8) |
    this[offset + 3])
}

Buffer.prototype.readIntLE = function readIntLE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkOffset(offset, byteLength, this.length)

  var val = this[offset]
  var mul = 1
  var i = 0
  while (++i < byteLength && (mul *= 0x100)) {
    val += this[offset + i] * mul
  }
  mul *= 0x80

  if (val >= mul) val -= Math.pow(2, 8 * byteLength)

  return val
}

Buffer.prototype.readIntBE = function readIntBE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkOffset(offset, byteLength, this.length)

  var i = byteLength
  var mul = 1
  var val = this[offset + --i]
  while (i > 0 && (mul *= 0x100)) {
    val += this[offset + --i] * mul
  }
  mul *= 0x80

  if (val >= mul) val -= Math.pow(2, 8 * byteLength)

  return val
}

Buffer.prototype.readInt8 = function readInt8 (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 1, this.length)
  if (!(this[offset] & 0x80)) return (this[offset])
  return ((0xff - this[offset] + 1) * -1)
}

Buffer.prototype.readInt16LE = function readInt16LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  var val = this[offset] | (this[offset + 1] << 8)
  return (val & 0x8000) ? val | 0xFFFF0000 : val
}

Buffer.prototype.readInt16BE = function readInt16BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  var val = this[offset + 1] | (this[offset] << 8)
  return (val & 0x8000) ? val | 0xFFFF0000 : val
}

Buffer.prototype.readInt32LE = function readInt32LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return (this[offset]) |
    (this[offset + 1] << 8) |
    (this[offset + 2] << 16) |
    (this[offset + 3] << 24)
}

Buffer.prototype.readInt32BE = function readInt32BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return (this[offset] << 24) |
    (this[offset + 1] << 16) |
    (this[offset + 2] << 8) |
    (this[offset + 3])
}

Buffer.prototype.readFloatLE = function readFloatLE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)
  return ieee754.read(this, offset, true, 23, 4)
}

Buffer.prototype.readFloatBE = function readFloatBE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)
  return ieee754.read(this, offset, false, 23, 4)
}

Buffer.prototype.readDoubleLE = function readDoubleLE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 8, this.length)
  return ieee754.read(this, offset, true, 52, 8)
}

Buffer.prototype.readDoubleBE = function readDoubleBE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 8, this.length)
  return ieee754.read(this, offset, false, 52, 8)
}

function checkInt (buf, value, offset, ext, max, min) {
  if (!Buffer.isBuffer(buf)) throw new TypeError('buffer must be a Buffer instance')
  if (value > max || value < min) throw new RangeError('value is out of bounds')
  if (offset + ext > buf.length) throw new RangeError('index out of range')
}

Buffer.prototype.writeUIntLE = function writeUIntLE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkInt(this, value, offset, byteLength, Math.pow(2, 8 * byteLength), 0)

  var mul = 1
  var i = 0
  this[offset] = value & 0xFF
  while (++i < byteLength && (mul *= 0x100)) {
    this[offset + i] = (value / mul) & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeUIntBE = function writeUIntBE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkInt(this, value, offset, byteLength, Math.pow(2, 8 * byteLength), 0)

  var i = byteLength - 1
  var mul = 1
  this[offset + i] = value & 0xFF
  while (--i >= 0 && (mul *= 0x100)) {
    this[offset + i] = (value / mul) & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeUInt8 = function writeUInt8 (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 1, 0xff, 0)
  if (!Buffer.TYPED_ARRAY_SUPPORT) value = Math.floor(value)
  this[offset] = (value & 0xff)
  return offset + 1
}

function objectWriteUInt16 (buf, value, offset, littleEndian) {
  if (value < 0) value = 0xffff + value + 1
  for (var i = 0, j = Math.min(buf.length - offset, 2); i < j; i++) {
    buf[offset + i] = (value & (0xff << (8 * (littleEndian ? i : 1 - i)))) >>>
      (littleEndian ? i : 1 - i) * 8
  }
}

Buffer.prototype.writeUInt16LE = function writeUInt16LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0xffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value & 0xff)
    this[offset + 1] = (value >>> 8)
  } else {
    objectWriteUInt16(this, value, offset, true)
  }
  return offset + 2
}

Buffer.prototype.writeUInt16BE = function writeUInt16BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0xffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 8)
    this[offset + 1] = (value & 0xff)
  } else {
    objectWriteUInt16(this, value, offset, false)
  }
  return offset + 2
}

function objectWriteUInt32 (buf, value, offset, littleEndian) {
  if (value < 0) value = 0xffffffff + value + 1
  for (var i = 0, j = Math.min(buf.length - offset, 4); i < j; i++) {
    buf[offset + i] = (value >>> (littleEndian ? i : 3 - i) * 8) & 0xff
  }
}

Buffer.prototype.writeUInt32LE = function writeUInt32LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0xffffffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset + 3] = (value >>> 24)
    this[offset + 2] = (value >>> 16)
    this[offset + 1] = (value >>> 8)
    this[offset] = (value & 0xff)
  } else {
    objectWriteUInt32(this, value, offset, true)
  }
  return offset + 4
}

Buffer.prototype.writeUInt32BE = function writeUInt32BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0xffffffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 24)
    this[offset + 1] = (value >>> 16)
    this[offset + 2] = (value >>> 8)
    this[offset + 3] = (value & 0xff)
  } else {
    objectWriteUInt32(this, value, offset, false)
  }
  return offset + 4
}

Buffer.prototype.writeIntLE = function writeIntLE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) {
    var limit = Math.pow(2, 8 * byteLength - 1)

    checkInt(this, value, offset, byteLength, limit - 1, -limit)
  }

  var i = 0
  var mul = 1
  var sub = value < 0 ? 1 : 0
  this[offset] = value & 0xFF
  while (++i < byteLength && (mul *= 0x100)) {
    this[offset + i] = ((value / mul) >> 0) - sub & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeIntBE = function writeIntBE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) {
    var limit = Math.pow(2, 8 * byteLength - 1)

    checkInt(this, value, offset, byteLength, limit - 1, -limit)
  }

  var i = byteLength - 1
  var mul = 1
  var sub = value < 0 ? 1 : 0
  this[offset + i] = value & 0xFF
  while (--i >= 0 && (mul *= 0x100)) {
    this[offset + i] = ((value / mul) >> 0) - sub & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeInt8 = function writeInt8 (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 1, 0x7f, -0x80)
  if (!Buffer.TYPED_ARRAY_SUPPORT) value = Math.floor(value)
  if (value < 0) value = 0xff + value + 1
  this[offset] = (value & 0xff)
  return offset + 1
}

Buffer.prototype.writeInt16LE = function writeInt16LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0x7fff, -0x8000)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value & 0xff)
    this[offset + 1] = (value >>> 8)
  } else {
    objectWriteUInt16(this, value, offset, true)
  }
  return offset + 2
}

Buffer.prototype.writeInt16BE = function writeInt16BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0x7fff, -0x8000)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 8)
    this[offset + 1] = (value & 0xff)
  } else {
    objectWriteUInt16(this, value, offset, false)
  }
  return offset + 2
}

Buffer.prototype.writeInt32LE = function writeInt32LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0x7fffffff, -0x80000000)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value & 0xff)
    this[offset + 1] = (value >>> 8)
    this[offset + 2] = (value >>> 16)
    this[offset + 3] = (value >>> 24)
  } else {
    objectWriteUInt32(this, value, offset, true)
  }
  return offset + 4
}

Buffer.prototype.writeInt32BE = function writeInt32BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0x7fffffff, -0x80000000)
  if (value < 0) value = 0xffffffff + value + 1
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 24)
    this[offset + 1] = (value >>> 16)
    this[offset + 2] = (value >>> 8)
    this[offset + 3] = (value & 0xff)
  } else {
    objectWriteUInt32(this, value, offset, false)
  }
  return offset + 4
}

function checkIEEE754 (buf, value, offset, ext, max, min) {
  if (value > max || value < min) throw new RangeError('value is out of bounds')
  if (offset + ext > buf.length) throw new RangeError('index out of range')
  if (offset < 0) throw new RangeError('index out of range')
}

function writeFloat (buf, value, offset, littleEndian, noAssert) {
  if (!noAssert) {
    checkIEEE754(buf, value, offset, 4, 3.4028234663852886e+38, -3.4028234663852886e+38)
  }
  ieee754.write(buf, value, offset, littleEndian, 23, 4)
  return offset + 4
}

Buffer.prototype.writeFloatLE = function writeFloatLE (value, offset, noAssert) {
  return writeFloat(this, value, offset, true, noAssert)
}

Buffer.prototype.writeFloatBE = function writeFloatBE (value, offset, noAssert) {
  return writeFloat(this, value, offset, false, noAssert)
}

function writeDouble (buf, value, offset, littleEndian, noAssert) {
  if (!noAssert) {
    checkIEEE754(buf, value, offset, 8, 1.7976931348623157E+308, -1.7976931348623157E+308)
  }
  ieee754.write(buf, value, offset, littleEndian, 52, 8)
  return offset + 8
}

Buffer.prototype.writeDoubleLE = function writeDoubleLE (value, offset, noAssert) {
  return writeDouble(this, value, offset, true, noAssert)
}

Buffer.prototype.writeDoubleBE = function writeDoubleBE (value, offset, noAssert) {
  return writeDouble(this, value, offset, false, noAssert)
}

// copy(targetBuffer, targetStart=0, sourceStart=0, sourceEnd=buffer.length)
Buffer.prototype.copy = function copy (target, targetStart, start, end) {
  if (!start) start = 0
  if (!end && end !== 0) end = this.length
  if (targetStart >= target.length) targetStart = target.length
  if (!targetStart) targetStart = 0
  if (end > 0 && end < start) end = start

  // Copy 0 bytes; we're done
  if (end === start) return 0
  if (target.length === 0 || this.length === 0) return 0

  // Fatal error conditions
  if (targetStart < 0) {
    throw new RangeError('targetStart out of bounds')
  }
  if (start < 0 || start >= this.length) throw new RangeError('sourceStart out of bounds')
  if (end < 0) throw new RangeError('sourceEnd out of bounds')

  // Are we oob?
  if (end > this.length) end = this.length
  if (target.length - targetStart < end - start) {
    end = target.length - targetStart + start
  }

  var len = end - start
  var i

  if (this === target && start < targetStart && targetStart < end) {
    // descending copy from end
    for (i = len - 1; i >= 0; i--) {
      target[i + targetStart] = this[i + start]
    }
  } else if (len < 1000 || !Buffer.TYPED_ARRAY_SUPPORT) {
    // ascending copy from start
    for (i = 0; i < len; i++) {
      target[i + targetStart] = this[i + start]
    }
  } else {
    target._set(this.subarray(start, start + len), targetStart)
  }

  return len
}

// fill(value, start=0, end=buffer.length)
Buffer.prototype.fill = function fill (value, start, end) {
  if (!value) value = 0
  if (!start) start = 0
  if (!end) end = this.length

  if (end < start) throw new RangeError('end < start')

  // Fill 0 bytes; we're done
  if (end === start) return
  if (this.length === 0) return

  if (start < 0 || start >= this.length) throw new RangeError('start out of bounds')
  if (end < 0 || end > this.length) throw new RangeError('end out of bounds')

  var i
  if (typeof value === 'number') {
    for (i = start; i < end; i++) {
      this[i] = value
    }
  } else {
    var bytes = utf8ToBytes(value.toString())
    var len = bytes.length
    for (i = start; i < end; i++) {
      this[i] = bytes[i % len]
    }
  }

  return this
}

/**
 * Creates a new `ArrayBuffer` with the *copied* memory of the buffer instance.
 * Added in Node 0.12. Only available in browsers that support ArrayBuffer.
 */
Buffer.prototype.toArrayBuffer = function toArrayBuffer () {
  if (typeof Uint8Array !== 'undefined') {
    if (Buffer.TYPED_ARRAY_SUPPORT) {
      return (new Buffer(this)).buffer
    } else {
      var buf = new Uint8Array(this.length)
      for (var i = 0, len = buf.length; i < len; i += 1) {
        buf[i] = this[i]
      }
      return buf.buffer
    }
  } else {
    throw new TypeError('Buffer.toArrayBuffer not supported in this browser')
  }
}

// HELPER FUNCTIONS
// ================

var BP = Buffer.prototype

/**
 * Augment a Uint8Array *instance* (not the Uint8Array class!) with Buffer methods
 */
Buffer._augment = function _augment (arr) {
  arr.constructor = Buffer
  arr._isBuffer = true

  // save reference to original Uint8Array set method before overwriting
  arr._set = arr.set

  // deprecated
  arr.get = BP.get
  arr.set = BP.set

  arr.write = BP.write
  arr.toString = BP.toString
  arr.toLocaleString = BP.toString
  arr.toJSON = BP.toJSON
  arr.equals = BP.equals
  arr.compare = BP.compare
  arr.indexOf = BP.indexOf
  arr.copy = BP.copy
  arr.slice = BP.slice
  arr.readUIntLE = BP.readUIntLE
  arr.readUIntBE = BP.readUIntBE
  arr.readUInt8 = BP.readUInt8
  arr.readUInt16LE = BP.readUInt16LE
  arr.readUInt16BE = BP.readUInt16BE
  arr.readUInt32LE = BP.readUInt32LE
  arr.readUInt32BE = BP.readUInt32BE
  arr.readIntLE = BP.readIntLE
  arr.readIntBE = BP.readIntBE
  arr.readInt8 = BP.readInt8
  arr.readInt16LE = BP.readInt16LE
  arr.readInt16BE = BP.readInt16BE
  arr.readInt32LE = BP.readInt32LE
  arr.readInt32BE = BP.readInt32BE
  arr.readFloatLE = BP.readFloatLE
  arr.readFloatBE = BP.readFloatBE
  arr.readDoubleLE = BP.readDoubleLE
  arr.readDoubleBE = BP.readDoubleBE
  arr.writeUInt8 = BP.writeUInt8
  arr.writeUIntLE = BP.writeUIntLE
  arr.writeUIntBE = BP.writeUIntBE
  arr.writeUInt16LE = BP.writeUInt16LE
  arr.writeUInt16BE = BP.writeUInt16BE
  arr.writeUInt32LE = BP.writeUInt32LE
  arr.writeUInt32BE = BP.writeUInt32BE
  arr.writeIntLE = BP.writeIntLE
  arr.writeIntBE = BP.writeIntBE
  arr.writeInt8 = BP.writeInt8
  arr.writeInt16LE = BP.writeInt16LE
  arr.writeInt16BE = BP.writeInt16BE
  arr.writeInt32LE = BP.writeInt32LE
  arr.writeInt32BE = BP.writeInt32BE
  arr.writeFloatLE = BP.writeFloatLE
  arr.writeFloatBE = BP.writeFloatBE
  arr.writeDoubleLE = BP.writeDoubleLE
  arr.writeDoubleBE = BP.writeDoubleBE
  arr.fill = BP.fill
  arr.inspect = BP.inspect
  arr.toArrayBuffer = BP.toArrayBuffer

  return arr
}

var INVALID_BASE64_RE = /[^+\/0-9A-Za-z-_]/g

function base64clean (str) {
  // Node strips out invalid characters like \n and \t from the string, base64-js does not
  str = stringtrim(str).replace(INVALID_BASE64_RE, '')
  // Node converts strings with length < 2 to ''
  if (str.length < 2) return ''
  // Node allows for non-padded base64 strings (missing trailing ===), base64-js does not
  while (str.length % 4 !== 0) {
    str = str + '='
  }
  return str
}

function stringtrim (str) {
  if (str.trim) return str.trim()
  return str.replace(/^\s+|\s+$/g, '')
}

function toHex (n) {
  if (n < 16) return '0' + n.toString(16)
  return n.toString(16)
}

function utf8ToBytes (string, units) {
  units = units || Infinity
  var codePoint
  var length = string.length
  var leadSurrogate = null
  var bytes = []

  for (var i = 0; i < length; i++) {
    codePoint = string.charCodeAt(i)

    // is surrogate component
    if (codePoint > 0xD7FF && codePoint < 0xE000) {
      // last char was a lead
      if (!leadSurrogate) {
        // no lead yet
        if (codePoint > 0xDBFF) {
          // unexpected trail
          if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
          continue
        } else if (i + 1 === length) {
          // unpaired lead
          if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
          continue
        }

        // valid lead
        leadSurrogate = codePoint

        continue
      }

      // 2 leads in a row
      if (codePoint < 0xDC00) {
        if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
        leadSurrogate = codePoint
        continue
      }

      // valid surrogate pair
      codePoint = (leadSurrogate - 0xD800 << 10 | codePoint - 0xDC00) + 0x10000
    } else if (leadSurrogate) {
      // valid bmp char, but last char was a lead
      if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
    }

    leadSurrogate = null

    // encode utf8
    if (codePoint < 0x80) {
      if ((units -= 1) < 0) break
      bytes.push(codePoint)
    } else if (codePoint < 0x800) {
      if ((units -= 2) < 0) break
      bytes.push(
        codePoint >> 0x6 | 0xC0,
        codePoint & 0x3F | 0x80
      )
    } else if (codePoint < 0x10000) {
      if ((units -= 3) < 0) break
      bytes.push(
        codePoint >> 0xC | 0xE0,
        codePoint >> 0x6 & 0x3F | 0x80,
        codePoint & 0x3F | 0x80
      )
    } else if (codePoint < 0x110000) {
      if ((units -= 4) < 0) break
      bytes.push(
        codePoint >> 0x12 | 0xF0,
        codePoint >> 0xC & 0x3F | 0x80,
        codePoint >> 0x6 & 0x3F | 0x80,
        codePoint & 0x3F | 0x80
      )
    } else {
      throw new Error('Invalid code point')
    }
  }

  return bytes
}

function asciiToBytes (str) {
  var byteArray = []
  for (var i = 0; i < str.length; i++) {
    // Node's code seems to be doing this and not & 0x7F..
    byteArray.push(str.charCodeAt(i) & 0xFF)
  }
  return byteArray
}

function utf16leToBytes (str, units) {
  var c, hi, lo
  var byteArray = []
  for (var i = 0; i < str.length; i++) {
    if ((units -= 2) < 0) break

    c = str.charCodeAt(i)
    hi = c >> 8
    lo = c % 256
    byteArray.push(lo)
    byteArray.push(hi)
  }

  return byteArray
}

function base64ToBytes (str) {
  return base64.toByteArray(base64clean(str))
}

function blitBuffer (src, dst, offset, length) {
  for (var i = 0; i < length; i++) {
    if ((i + offset >= dst.length) || (i >= src.length)) break
    dst[i + offset] = src[i]
  }
  return i
}

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{"base64-js":2,"ieee754":11,"isarray":15}],8:[function(require,module,exports){
(function (Buffer){
var Writable = require('readable-stream').Writable
var inherits = require('inherits')

if (typeof Uint8Array === 'undefined') {
  var U8 = require('typedarray').Uint8Array
} else {
  var U8 = Uint8Array
}

function ConcatStream(opts, cb) {
  if (!(this instanceof ConcatStream)) return new ConcatStream(opts, cb)

  if (typeof opts === 'function') {
    cb = opts
    opts = {}
  }
  if (!opts) opts = {}

  var encoding = opts.encoding
  var shouldInferEncoding = false

  if (!encoding) {
    shouldInferEncoding = true
  } else {
    encoding =  String(encoding).toLowerCase()
    if (encoding === 'u8' || encoding === 'uint8') {
      encoding = 'uint8array'
    }
  }

  Writable.call(this, { objectMode: true })

  this.encoding = encoding
  this.shouldInferEncoding = shouldInferEncoding

  if (cb) this.on('finish', function () { cb(this.getBody()) })
  this.body = []
}

module.exports = ConcatStream
inherits(ConcatStream, Writable)

ConcatStream.prototype._write = function(chunk, enc, next) {
  this.body.push(chunk)
  next()
}

ConcatStream.prototype.inferEncoding = function (buff) {
  var firstBuffer = buff === undefined ? this.body[0] : buff;
  if (Buffer.isBuffer(firstBuffer)) return 'buffer'
  if (typeof Uint8Array !== 'undefined' && firstBuffer instanceof Uint8Array) return 'uint8array'
  if (Array.isArray(firstBuffer)) return 'array'
  if (typeof firstBuffer === 'string') return 'string'
  if (Object.prototype.toString.call(firstBuffer) === "[object Object]") return 'object'
  return 'buffer'
}

ConcatStream.prototype.getBody = function () {
  if (!this.encoding && this.body.length === 0) return []
  if (this.shouldInferEncoding) this.encoding = this.inferEncoding()
  if (this.encoding === 'array') return arrayConcat(this.body)
  if (this.encoding === 'string') return stringConcat(this.body)
  if (this.encoding === 'buffer') return bufferConcat(this.body)
  if (this.encoding === 'uint8array') return u8Concat(this.body)
  return this.body
}

var isArray = Array.isArray || function (arr) {
  return Object.prototype.toString.call(arr) == '[object Array]'
}

function isArrayish (arr) {
  return /Array\]$/.test(Object.prototype.toString.call(arr))
}

function stringConcat (parts) {
  var strings = []
  var needsToString = false
  for (var i = 0; i < parts.length; i++) {
    var p = parts[i]
    if (typeof p === 'string') {
      strings.push(p)
    } else if (Buffer.isBuffer(p)) {
      strings.push(p)
    } else {
      strings.push(Buffer(p))
    }
  }
  if (Buffer.isBuffer(parts[0])) {
    strings = Buffer.concat(strings)
    strings = strings.toString('utf8')
  } else {
    strings = strings.join('')
  }
  return strings
}

function bufferConcat (parts) {
  var bufs = []
  for (var i = 0; i < parts.length; i++) {
    var p = parts[i]
    if (Buffer.isBuffer(p)) {
      bufs.push(p)
    } else if (typeof p === 'string' || isArrayish(p)
    || (p && typeof p.subarray === 'function')) {
      bufs.push(Buffer(p))
    } else bufs.push(Buffer(String(p)))
  }
  return Buffer.concat(bufs)
}

function arrayConcat (parts) {
  var res = []
  for (var i = 0; i < parts.length; i++) {
    res.push.apply(res, parts[i])
  }
  return res
}

function u8Concat (parts) {
  var len = 0
  for (var i = 0; i < parts.length; i++) {
    if (typeof parts[i] === 'string') {
      parts[i] = Buffer(parts[i])
    }
    len += parts[i].length
  }
  var u8 = new U8(len)
  for (var i = 0, offset = 0; i < parts.length; i++) {
    var part = parts[i]
    for (var j = 0; j < part.length; j++) {
      u8[offset++] = part[j]
    }
  }
  return u8
}

}).call(this,require("buffer").Buffer)
},{"buffer":7,"inherits":12,"readable-stream":23,"typedarray":25}],9:[function(require,module,exports){
(function (Buffer){
// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

// NOTE: These type checking functions intentionally don't use `instanceof`
// because it is fragile and can be easily faked with `Object.create()`.

function isArray(arg) {
  if (Array.isArray) {
    return Array.isArray(arg);
  }
  return objectToString(arg) === '[object Array]';
}
exports.isArray = isArray;

function isBoolean(arg) {
  return typeof arg === 'boolean';
}
exports.isBoolean = isBoolean;

function isNull(arg) {
  return arg === null;
}
exports.isNull = isNull;

function isNullOrUndefined(arg) {
  return arg == null;
}
exports.isNullOrUndefined = isNullOrUndefined;

function isNumber(arg) {
  return typeof arg === 'number';
}
exports.isNumber = isNumber;

function isString(arg) {
  return typeof arg === 'string';
}
exports.isString = isString;

function isSymbol(arg) {
  return typeof arg === 'symbol';
}
exports.isSymbol = isSymbol;

function isUndefined(arg) {
  return arg === void 0;
}
exports.isUndefined = isUndefined;

function isRegExp(re) {
  return objectToString(re) === '[object RegExp]';
}
exports.isRegExp = isRegExp;

function isObject(arg) {
  return typeof arg === 'object' && arg !== null;
}
exports.isObject = isObject;

function isDate(d) {
  return objectToString(d) === '[object Date]';
}
exports.isDate = isDate;

function isError(e) {
  return (objectToString(e) === '[object Error]' || e instanceof Error);
}
exports.isError = isError;

function isFunction(arg) {
  return typeof arg === 'function';
}
exports.isFunction = isFunction;

function isPrimitive(arg) {
  return arg === null ||
         typeof arg === 'boolean' ||
         typeof arg === 'number' ||
         typeof arg === 'string' ||
         typeof arg === 'symbol' ||  // ES6 symbol
         typeof arg === 'undefined';
}
exports.isPrimitive = isPrimitive;

exports.isBuffer = Buffer.isBuffer;

function objectToString(o) {
  return Object.prototype.toString.call(o);
}

}).call(this,{"isBuffer":require("../../is-buffer/index.js")})
},{"../../is-buffer/index.js":14}],10:[function(require,module,exports){
// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

function EventEmitter() {
  this._events = this._events || {};
  this._maxListeners = this._maxListeners || undefined;
}
module.exports = EventEmitter;

// Backwards-compat with node 0.10.x
EventEmitter.EventEmitter = EventEmitter;

EventEmitter.prototype._events = undefined;
EventEmitter.prototype._maxListeners = undefined;

// By default EventEmitters will print a warning if more than 10 listeners are
// added to it. This is a useful default which helps finding memory leaks.
EventEmitter.defaultMaxListeners = 10;

// Obviously not all Emitters should be limited to 10. This function allows
// that to be increased. Set to zero for unlimited.
EventEmitter.prototype.setMaxListeners = function(n) {
  if (!isNumber(n) || n < 0 || isNaN(n))
    throw TypeError('n must be a positive number');
  this._maxListeners = n;
  return this;
};

EventEmitter.prototype.emit = function(type) {
  var er, handler, len, args, i, listeners;

  if (!this._events)
    this._events = {};

  // If there is no 'error' event listener then throw.
  if (type === 'error') {
    if (!this._events.error ||
        (isObject(this._events.error) && !this._events.error.length)) {
      er = arguments[1];
      if (er instanceof Error) {
        throw er; // Unhandled 'error' event
      }
      throw TypeError('Uncaught, unspecified "error" event.');
    }
  }

  handler = this._events[type];

  if (isUndefined(handler))
    return false;

  if (isFunction(handler)) {
    switch (arguments.length) {
      // fast cases
      case 1:
        handler.call(this);
        break;
      case 2:
        handler.call(this, arguments[1]);
        break;
      case 3:
        handler.call(this, arguments[1], arguments[2]);
        break;
      // slower
      default:
        len = arguments.length;
        args = new Array(len - 1);
        for (i = 1; i < len; i++)
          args[i - 1] = arguments[i];
        handler.apply(this, args);
    }
  } else if (isObject(handler)) {
    len = arguments.length;
    args = new Array(len - 1);
    for (i = 1; i < len; i++)
      args[i - 1] = arguments[i];

    listeners = handler.slice();
    len = listeners.length;
    for (i = 0; i < len; i++)
      listeners[i].apply(this, args);
  }

  return true;
};

EventEmitter.prototype.addListener = function(type, listener) {
  var m;

  if (!isFunction(listener))
    throw TypeError('listener must be a function');

  if (!this._events)
    this._events = {};

  // To avoid recursion in the case that type === "newListener"! Before
  // adding it to the listeners, first emit "newListener".
  if (this._events.newListener)
    this.emit('newListener', type,
              isFunction(listener.listener) ?
              listener.listener : listener);

  if (!this._events[type])
    // Optimize the case of one listener. Don't need the extra array object.
    this._events[type] = listener;
  else if (isObject(this._events[type]))
    // If we've already got an array, just append.
    this._events[type].push(listener);
  else
    // Adding the second element, need to change to array.
    this._events[type] = [this._events[type], listener];

  // Check for listener leak
  if (isObject(this._events[type]) && !this._events[type].warned) {
    var m;
    if (!isUndefined(this._maxListeners)) {
      m = this._maxListeners;
    } else {
      m = EventEmitter.defaultMaxListeners;
    }

    if (m && m > 0 && this._events[type].length > m) {
      this._events[type].warned = true;
      console.error('(node) warning: possible EventEmitter memory ' +
                    'leak detected. %d listeners added. ' +
                    'Use emitter.setMaxListeners() to increase limit.',
                    this._events[type].length);
      if (typeof console.trace === 'function') {
        // not supported in IE 10
        console.trace();
      }
    }
  }

  return this;
};

EventEmitter.prototype.on = EventEmitter.prototype.addListener;

EventEmitter.prototype.once = function(type, listener) {
  if (!isFunction(listener))
    throw TypeError('listener must be a function');

  var fired = false;

  function g() {
    this.removeListener(type, g);

    if (!fired) {
      fired = true;
      listener.apply(this, arguments);
    }
  }

  g.listener = listener;
  this.on(type, g);

  return this;
};

// emits a 'removeListener' event iff the listener was removed
EventEmitter.prototype.removeListener = function(type, listener) {
  var list, position, length, i;

  if (!isFunction(listener))
    throw TypeError('listener must be a function');

  if (!this._events || !this._events[type])
    return this;

  list = this._events[type];
  length = list.length;
  position = -1;

  if (list === listener ||
      (isFunction(list.listener) && list.listener === listener)) {
    delete this._events[type];
    if (this._events.removeListener)
      this.emit('removeListener', type, listener);

  } else if (isObject(list)) {
    for (i = length; i-- > 0;) {
      if (list[i] === listener ||
          (list[i].listener && list[i].listener === listener)) {
        position = i;
        break;
      }
    }

    if (position < 0)
      return this;

    if (list.length === 1) {
      list.length = 0;
      delete this._events[type];
    } else {
      list.splice(position, 1);
    }

    if (this._events.removeListener)
      this.emit('removeListener', type, listener);
  }

  return this;
};

EventEmitter.prototype.removeAllListeners = function(type) {
  var key, listeners;

  if (!this._events)
    return this;

  // not listening for removeListener, no need to emit
  if (!this._events.removeListener) {
    if (arguments.length === 0)
      this._events = {};
    else if (this._events[type])
      delete this._events[type];
    return this;
  }

  // emit removeListener for all listeners on all events
  if (arguments.length === 0) {
    for (key in this._events) {
      if (key === 'removeListener') continue;
      this.removeAllListeners(key);
    }
    this.removeAllListeners('removeListener');
    this._events = {};
    return this;
  }

  listeners = this._events[type];

  if (isFunction(listeners)) {
    this.removeListener(type, listeners);
  } else {
    // LIFO order
    while (listeners.length)
      this.removeListener(type, listeners[listeners.length - 1]);
  }
  delete this._events[type];

  return this;
};

EventEmitter.prototype.listeners = function(type) {
  var ret;
  if (!this._events || !this._events[type])
    ret = [];
  else if (isFunction(this._events[type]))
    ret = [this._events[type]];
  else
    ret = this._events[type].slice();
  return ret;
};

EventEmitter.listenerCount = function(emitter, type) {
  var ret;
  if (!emitter._events || !emitter._events[type])
    ret = 0;
  else if (isFunction(emitter._events[type]))
    ret = 1;
  else
    ret = emitter._events[type].length;
  return ret;
};

function isFunction(arg) {
  return typeof arg === 'function';
}

function isNumber(arg) {
  return typeof arg === 'number';
}

function isObject(arg) {
  return typeof arg === 'object' && arg !== null;
}

function isUndefined(arg) {
  return arg === void 0;
}

},{}],11:[function(require,module,exports){
exports.read = function (buffer, offset, isLE, mLen, nBytes) {
  var e, m
  var eLen = nBytes * 8 - mLen - 1
  var eMax = (1 << eLen) - 1
  var eBias = eMax >> 1
  var nBits = -7
  var i = isLE ? (nBytes - 1) : 0
  var d = isLE ? -1 : 1
  var s = buffer[offset + i]

  i += d

  e = s & ((1 << (-nBits)) - 1)
  s >>= (-nBits)
  nBits += eLen
  for (; nBits > 0; e = e * 256 + buffer[offset + i], i += d, nBits -= 8) {}

  m = e & ((1 << (-nBits)) - 1)
  e >>= (-nBits)
  nBits += mLen
  for (; nBits > 0; m = m * 256 + buffer[offset + i], i += d, nBits -= 8) {}

  if (e === 0) {
    e = 1 - eBias
  } else if (e === eMax) {
    return m ? NaN : ((s ? -1 : 1) * Infinity)
  } else {
    m = m + Math.pow(2, mLen)
    e = e - eBias
  }
  return (s ? -1 : 1) * m * Math.pow(2, e - mLen)
}

exports.write = function (buffer, value, offset, isLE, mLen, nBytes) {
  var e, m, c
  var eLen = nBytes * 8 - mLen - 1
  var eMax = (1 << eLen) - 1
  var eBias = eMax >> 1
  var rt = (mLen === 23 ? Math.pow(2, -24) - Math.pow(2, -77) : 0)
  var i = isLE ? 0 : (nBytes - 1)
  var d = isLE ? 1 : -1
  var s = value < 0 || (value === 0 && 1 / value < 0) ? 1 : 0

  value = Math.abs(value)

  if (isNaN(value) || value === Infinity) {
    m = isNaN(value) ? 1 : 0
    e = eMax
  } else {
    e = Math.floor(Math.log(value) / Math.LN2)
    if (value * (c = Math.pow(2, -e)) < 1) {
      e--
      c *= 2
    }
    if (e + eBias >= 1) {
      value += rt / c
    } else {
      value += rt * Math.pow(2, 1 - eBias)
    }
    if (value * c >= 2) {
      e++
      c /= 2
    }

    if (e + eBias >= eMax) {
      m = 0
      e = eMax
    } else if (e + eBias >= 1) {
      m = (value * c - 1) * Math.pow(2, mLen)
      e = e + eBias
    } else {
      m = value * Math.pow(2, eBias - 1) * Math.pow(2, mLen)
      e = 0
    }
  }

  for (; mLen >= 8; buffer[offset + i] = m & 0xff, i += d, m /= 256, mLen -= 8) {}

  e = (e << mLen) | m
  eLen += mLen
  for (; eLen > 0; buffer[offset + i] = e & 0xff, i += d, e /= 256, eLen -= 8) {}

  buffer[offset + i - d] |= s * 128
}

},{}],12:[function(require,module,exports){
if (typeof Object.create === 'function') {
  // implementation from standard node.js 'util' module
  module.exports = function inherits(ctor, superCtor) {
    ctor.super_ = superCtor
    ctor.prototype = Object.create(superCtor.prototype, {
      constructor: {
        value: ctor,
        enumerable: false,
        writable: true,
        configurable: true
      }
    });
  };
} else {
  // old school shim for old browsers
  module.exports = function inherits(ctor, superCtor) {
    ctor.super_ = superCtor
    var TempCtor = function () {}
    TempCtor.prototype = superCtor.prototype
    ctor.prototype = new TempCtor()
    ctor.prototype.constructor = ctor
  }
}

},{}],13:[function(require,module,exports){
(function (global){
(function(e){if(typeof exports==="object"&&typeof module!=="undefined"){module.exports=e()}else if(typeof define==="function"&&define.amd){define([],e)}else{var t;if(typeof window!=="undefined"){t=window}else if(typeof global!=="undefined"){t=global}else if(typeof self!=="undefined"){t=self}else{t=this}t.ipfsAPI=e()}})(function(){var e,t,r;return function n(e,t,r){function i(o,s){if(!t[o]){if(!e[o]){var f=typeof require=="function"&&require;if(!s&&f)return f(o,!0);if(a)return a(o,!0);var u=new Error("Cannot find module '"+o+"'");throw u.code="MODULE_NOT_FOUND",u}var c=t[o]={exports:{}};e[o][0].call(c.exports,function(t){var r=e[o][1][t];return i(r?r:t)},c,c.exports,n,e,t,r)}return t[o].exports}var a=typeof require=="function"&&require;for(var o=0;o<r.length;o++)i(r[o]);return i}({1:[function(e,t,r){"use strict";var n=e("arr-flatten");var i=[].slice;function a(e,t){var r=arguments.length;var a=e.length,o=-1;var s=[],t;if(r===1){return e}if(r>2){t=n(i.call(arguments,1))}while(++o<a){if(!~t.indexOf(e[o])){s.push(e[o])}}return s}t.exports=a},{"arr-flatten":2}],2:[function(e,t,r){"use strict";t.exports=function i(e){return n(e,[])};function n(e,t){var r=e.length;var i=-1;while(r--){var a=e[++i];if(Array.isArray(a)){n(a,t)}else{t.push(a)}}return t}},{}],3:[function(e,t,r){"use strict";t.exports=function n(e){if(!Array.isArray(e)){throw new TypeError("array-unique expects an array.")}var t=e.length;var r=-1;while(r++<t){var n=r+1;for(;n<e.length;++n){if(e[r]===e[n]){e.splice(n--,1)}}}return e}},{}],4:[function(e,t,r){var n=e("util/");var i=Array.prototype.slice;var a=Object.prototype.hasOwnProperty;var o=t.exports=l;o.AssertionError=function b(e){this.name="AssertionError";this.actual=e.actual;this.expected=e.expected;this.operator=e.operator;if(e.message){this.message=e.message;this.generatedMessage=false}else{this.message=u(this);this.generatedMessage=true}var t=e.stackStartFunction||c;if(Error.captureStackTrace){Error.captureStackTrace(this,t)}else{var r=new Error;if(r.stack){var n=r.stack;var i=t.name;var a=n.indexOf("\n"+i);if(a>=0){var o=n.indexOf("\n",a+1);n=n.substring(o+1)}this.stack=n}}};n.inherits(o.AssertionError,Error);function s(e,t){if(n.isUndefined(t)){return""+t}if(n.isNumber(t)&&!isFinite(t)){return t.toString()}if(n.isFunction(t)||n.isRegExp(t)){return t.toString()}return t}function f(e,t){if(n.isString(e)){return e.length<t?e:e.slice(0,t)}else{return e}}function u(e){return f(JSON.stringify(e.actual,s),128)+" "+e.operator+" "+f(JSON.stringify(e.expected,s),128)}function c(e,t,r,n,i){throw new o.AssertionError({message:r,actual:e,expected:t,operator:n,stackStartFunction:i})}o.fail=c;function l(e,t){if(!e)c(e,true,t,"==",o.ok)}o.ok=l;o.equal=function m(e,t,r){if(e!=t)c(e,t,r,"==",o.equal)};o.notEqual=function _(e,t,r){if(e==t){c(e,t,r,"!=",o.notEqual)}};o.deepEqual=function w(e,t,r){if(!h(e,t)){c(e,t,r,"deepEqual",o.deepEqual)}};function h(e,t){if(e===t){return true}else if(n.isBuffer(e)&&n.isBuffer(t)){if(e.length!=t.length)return false;for(var r=0;r<e.length;r++){if(e[r]!==t[r])return false}return true}else if(n.isDate(e)&&n.isDate(t)){return e.getTime()===t.getTime()}else if(n.isRegExp(e)&&n.isRegExp(t)){return e.source===t.source&&e.global===t.global&&e.multiline===t.multiline&&e.lastIndex===t.lastIndex&&e.ignoreCase===t.ignoreCase}else if(!n.isObject(e)&&!n.isObject(t)){return e==t}else{return d(e,t)}}function p(e){return Object.prototype.toString.call(e)=="[object Arguments]"}function d(e,t){if(n.isNullOrUndefined(e)||n.isNullOrUndefined(t))return false;if(e.prototype!==t.prototype)return false;if(n.isPrimitive(e)||n.isPrimitive(t)){return e===t}var r=p(e),a=p(t);if(r&&!a||!r&&a)return false;if(r){e=i.call(e);t=i.call(t);return h(e,t)}var o=y(e),s=y(t),f,u;if(o.length!=s.length)return false;o.sort();s.sort();for(u=o.length-1;u>=0;u--){if(o[u]!=s[u])return false}for(u=o.length-1;u>=0;u--){f=o[u];if(!h(e[f],t[f]))return false}return true}o.notDeepEqual=function E(e,t,r){if(h(e,t)){c(e,t,r,"notDeepEqual",o.notDeepEqual)}};o.strictEqual=function S(e,t,r){if(e!==t){c(e,t,r,"===",o.strictEqual)}};o.notStrictEqual=function x(e,t,r){if(e===t){c(e,t,r,"!==",o.notStrictEqual)}};function v(e,t){if(!e||!t){return false}if(Object.prototype.toString.call(t)=="[object RegExp]"){return t.test(e)}else if(e instanceof t){return true}else if(t.call({},e)===true){return true}return false}function g(e,t,r,i){var a;if(n.isString(r)){i=r;r=null}try{t()}catch(o){a=o}i=(r&&r.name?" ("+r.name+").":".")+(i?" "+i:".");if(e&&!a){c(a,r,"Missing expected exception"+i)}if(!e&&v(a,r)){c(a,r,"Got unwanted exception"+i)}if(e&&a&&r&&!v(a,r)||!e&&a){throw a}}o.throws=function(e,t,r){g.apply(this,[true].concat(i.call(arguments)))};o.doesNotThrow=function(e,t){g.apply(this,[false].concat(i.call(arguments)))};o.ifError=function(e){if(e){throw e}};var y=Object.keys||function(e){var t=[];for(var r in e){if(a.call(e,r))t.push(r)}return t}},{"util/":148}],5:[function(e,t,r){t.exports=n;function n(e,t,r){if(e instanceof RegExp)e=i(e,r);if(t instanceof RegExp)t=i(t,r);var n=a(e,t,r);return n&&{start:n[0],end:n[1],pre:r.slice(0,n[0]),body:r.slice(n[0]+e.length,n[1]),post:r.slice(n[1]+t.length)}}function i(e,t){var r=t.match(e);return r?r[0]:null}n.range=a;function a(e,t,r){var n,i,a,o,s;var f=r.indexOf(e);var u=r.indexOf(t,f+1);var c=f;if(f>=0&&u>0){n=[];a=r.length;while(c<r.length&&c>=0&&!s){if(c==f){n.push(c);f=r.indexOf(e,c+1)}else if(n.length==1){s=[n.pop(),u]}else{i=n.pop();if(i<a){a=i;o=u}u=r.indexOf(t,c+1)}c=f<u&&f>=0?f:u}if(n.length){s=[a,o]}}return s}},{}],6:[function(e,t,r){t.exports=function n(e){var t={};var r=e.length;var n=e.charAt(0);for(var i=0;i<e.length;i++){t[e.charAt(i)]=i}function a(t){if(t.length===0)return"";var n=[0];for(var i=0;i<t.length;++i){for(var a=0,o=t[i];a<n.length;++a){o+=n[a]<<8;n[a]=o%r;o=o/r|0}while(o>0){n.push(o%r);o=o/r|0}}for(var s=0;t[s]===0&&s<t.length-1;++s){n.push(0)}for(var f=0,u=n.length-1;f<=u;++f,--u){var c=e[n[f]];n[f]=e[n[u]];n[u]=c}return n.join("")}function o(e){if(e.length===0)return[];var i=[0];for(var a=0;a<e.length;a++){var o=t[e[a]];if(o===undefined)throw new Error("Non-base"+r+" character");for(var s=0,f=o;s<i.length;++s){f+=i[s]*r;i[s]=f&255;f>>=8}while(f>0){i.push(f&255);f>>=8}}for(var u=0;e[u]===n&&u<e.length-1;++u){i.push(0)}return i.reverse()}return{encode:a,decode:o}}},{}],7:[function(e,t,r){var n="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";(function(e){"use strict";var t=typeof Uint8Array!=="undefined"?Uint8Array:Array;var r="+".charCodeAt(0);var i="/".charCodeAt(0);var a="0".charCodeAt(0);var o="a".charCodeAt(0);var s="A".charCodeAt(0);var f="-".charCodeAt(0);var u="_".charCodeAt(0);function c(e){var t=e.charCodeAt(0);if(t===r||t===f)return 62;if(t===i||t===u)return 63;if(t<a)return-1;if(t<a+10)return t-a+26+26;if(t<s+26)return t-s;if(t<o+26)return t-o+26}function l(e){var r,n,i,a,o,s;if(e.length%4>0){throw new Error("Invalid string. Length must be a multiple of 4")}var f=e.length;o="="===e.charAt(f-2)?2:"="===e.charAt(f-1)?1:0;s=new t(e.length*3/4-o);i=o>0?e.length-4:e.length;var u=0;function l(e){s[u++]=e}for(r=0,n=0;r<i;r+=4,n+=3){a=c(e.charAt(r))<<18|c(e.charAt(r+1))<<12|c(e.charAt(r+2))<<6|c(e.charAt(r+3));l((a&16711680)>>16);l((a&65280)>>8);l(a&255)}if(o===2){a=c(e.charAt(r))<<2|c(e.charAt(r+1))>>4;l(a&255)}else if(o===1){a=c(e.charAt(r))<<10|c(e.charAt(r+1))<<4|c(e.charAt(r+2))>>2;l(a>>8&255);l(a&255)}return s}function h(e){var t,r=e.length%3,i="",a,o;function s(e){return n.charAt(e)}function f(e){return s(e>>18&63)+s(e>>12&63)+s(e>>6&63)+s(e&63)}for(t=0,o=e.length-r;t<o;t+=3){a=(e[t]<<16)+(e[t+1]<<8)+e[t+2];i+=f(a)}switch(r){case 1:a=e[e.length-1];i+=s(a>>2);i+=s(a<<4&63);i+="==";break;case 2:a=(e[e.length-2]<<8)+e[e.length-1];i+=s(a>>10);i+=s(a>>4&63);i+=s(a<<2&63);i+="=";break}return i}e.toByteArray=l;e.fromByteArray=h})(typeof r==="undefined"?this.base64js={}:r)},{}],8:[function(e,t,r){var n=e("concat-map");var i=e("balanced-match");t.exports=d;var a="\0SLASH"+Math.random()+"\0";var o="\0OPEN"+Math.random()+"\0";var s="\0CLOSE"+Math.random()+"\0";var f="\0COMMA"+Math.random()+"\0";var u="\0PERIOD"+Math.random()+"\0";function c(e){return parseInt(e,10)==e?parseInt(e,10):e.charCodeAt(0)}function l(e){return e.split("\\\\").join(a).split("\\{").join(o).split("\\}").join(s).split("\\,").join(f).split("\\.").join(u)}function h(e){return e.split(a).join("\\").split(o).join("{").split(s).join("}").split(f).join(",").split(u).join(".")}function p(e){if(!e)return[""];var t=[];var r=i("{","}",e);if(!r)return e.split(",");var n=r.pre;var a=r.body;var o=r.post;var s=n.split(",");s[s.length-1]+="{"+a+"}";var f=p(o);if(o.length){s[s.length-1]+=f.shift();s.push.apply(s,f)}t.push.apply(t,s);return t}function d(e){if(!e)return[];return _(l(e),true).map(h)}function v(e){return e}function g(e){return"{"+e+"}"}function y(e){return/^-?0\d/.test(e)}function b(e,t){return e<=t}function m(e,t){return e>=t}function _(e,t){var r=[];var a=i("{","}",e);if(!a||/\$$/.test(a.pre))return[e];var o=/^-?\d+\.\.-?\d+(?:\.\.-?\d+)?$/.test(a.body);var f=/^[a-zA-Z]\.\.[a-zA-Z](?:\.\.-?\d+)?$/.test(a.body);var u=o||f;var l=/^(.*,)+(.+)?$/.test(a.body);if(!u&&!l){if(a.post.match(/,.*\}/)){e=a.pre+"{"+a.body+s+a.post;return _(e)}return[e]}var h;if(u){h=a.body.split(/\.\./)}else{h=p(a.body);if(h.length===1){h=_(h[0],false).map(g);if(h.length===1){var d=a.post.length?_(a.post,false):[""];return d.map(function(e){return a.pre+h[0]+e})}}}var v=a.pre;var d=a.post.length?_(a.post,false):[""];var w;if(u){var E=c(h[0]);var S=c(h[1]);var x=Math.max(h[0].length,h[1].length);var O=h.length==3?Math.abs(c(h[2])):1;var j=b;var A=S<E;if(A){O*=-1;j=m}var k=h.some(y);w=[];for(var R=E;j(R,S);R+=O){var T;if(f){T=String.fromCharCode(R);if(T==="\\")T=""}else{T=String(R);if(k){var L=x-T.length;if(L>0){var I=new Array(L+1).join("0");if(R<0)T="-"+I+T.slice(1);else T=I+T}}}w.push(T)}}else{w=n(h,function(e){return _(e,false)})}for(var C=0;C<w.length;C++){for(var P=0;P<d.length;P++){var M=v+w[C]+d[P];if(!t||u||M)r.push(M)}}return r}},{"balanced-match":5,"concat-map":17}],9:[function(e,t,r){"use strict";var n=e("expand-range");var i=e("repeat-element");var a=e("preserve");t.exports=function(e,t){if(typeof e!=="string"){throw new Error("braces expects a string")}return o(e,t)};function o(e,t,r){if(e===""){return[]}if(!Array.isArray(t)){r=t;t=[]}var i=r||{};t=t||[];if(typeof i.nodupes==="undefined"){i.nodupes=true}var E=i.fn;var x;if(typeof i==="function"){E=i;i={}}if(!(_ instanceof RegExp)){_=g()}var O=e.match(_)||[];var j=O[0];switch(j){case"\\,":return v(e,t,i);case"\\.":return p(e,t,i);case"/.":return d(e,t,i);case" ":return l(e);case"{,}":return s(e,i,o);case"{}":return u(e,t,i);case"\\{":case"\\}":return h(e,t,i);case"${":if(!/\{[^{]+\{/.test(e)){return t.concat(e)}else{x=true;e=a.before(e,b())}}if(!(m instanceof RegExp)){m=y()}var A=m.exec(e);if(A==null){return[e]}var k=A[1];var R=A[2];if(R===""){return[e]}var T,L;if(R.indexOf("..")!==-1){T=n(R,i,E)||R.split(",");L=T.length}else if(R[0]==='"'||R[0]==="'"){return t.concat(e.split(/['"]/).join(""))}else{T=R.split(",");if(i.makeRe){return o(e.replace(k,f(T,"|")),i)}L=T.length;if(L===1&&i.bash){T[0]=f(T[0],"\\")}}var I=T.length;var C=0,P;while(I--){var M=T[C++];if(/(\.[^.\/])/.test(M)){if(L>1){return T}else{return[e]}}P=w(e,k,M);if(/\{[^{}]+?\}/.test(P)){t=o(P,t,i)}else if(P!==""){if(i.nodupes&&t.indexOf(P)!==-1){continue}t.push(x?a.after(P):P)}}if(i.strict){return S(t,c)}return t}function s(e,t,r){if(typeof t==="function"){r=t;t=null}var n=t||{};var a="__ESC_EXP__";var o=0;var s;var f=e.split("{,}");if(n.nodupes){return r(f.join(""),n)}o=f.length-1;s=r(f.join(a),n);var u=s.length;var c=[];var l=0;while(u--){var h=s[l++];var p=h.indexOf(a);if(p===-1){c.push(h)}else{h=h.split("__ESC_EXP__").join("");if(!!h&&n.nodupes!==false){c.push(h)}else{var d=Math.pow(2,o);c.push.apply(c,i(h,d))}}}return c}function f(e,t){if(t==="|"){return"("+e.join(t)+")"}if(t===","){return"{"+e.join(t)+"}"}if(t==="-"){return"["+e.join(t)+"]"}if(t==="\\"){return"\\{"+e+"\\}"}}function u(e,t,r){return o(e.split("{}").join("\\{\\}"),t,r)}function c(e){return!!e&&e!=="\\"}function l(e){var t=e.split(" ");var r=t.length;var n=[];var i=0;while(r--){n.push.apply(n,o(t[i++]))}return n}function h(e,t,r){if(!/\{[^{]+\{/.test(e)){return t.concat(e.split("\\").join(""))}else{e=e.split("\\{").join("__LT_BRACE__");e=e.split("\\}").join("__RT_BRACE__");return E(o(e,t,r),function(e){e=e.split("__LT_BRACE__").join("{");return e.split("__RT_BRACE__").join("}")})}}function p(e,t,r){if(!/[^\\]\..+\\\./.test(e)){return t.concat(e.split("\\").join(""))}else{e=e.split("\\.").join("__ESC_DOT__");return E(o(e,t,r),function(e){return e.split("__ESC_DOT__").join(".")})}}function d(e,t,r){e=e.split("/.").join("__ESC_PATH__");return E(o(e,t,r),function(e){return e.split("__ESC_PATH__").join("/.")})}function v(e,t,r){if(!/\w,/.test(e)){return t.concat(e.split("\\").join(""))}else{e=e.split("\\,").join("__ESC_COMMA__");return E(o(e,t,r),function(e){return e.split("__ESC_COMMA__").join(",")})}}function g(){return/\${|( (?=[{,}])|(?=[{,}]) )|{}|{,}|\\,(?=.*[{}])|\/\.(?=.*[{}])|\\\.(?={)|\\{|\\}/}function y(){return/.*(\\?\{([^}]+)\})/}function b(){return/\$\{([^}]+)\}/}var m;var _;function w(e,t,r){var n=e.indexOf(t);return e.substr(0,n)+r+e.substr(n+t.length)}function E(e,t){if(e==null){return[]}var r=e.length;var n=new Array(r);var i=-1;while(++i<r){n[i]=t(e[i],i,e)}return n}function S(e,t){if(e==null)return[];if(typeof t!=="function"){throw new TypeError("braces: filter expects a callback function.")}var r=e.length;var n=e.slice();var i=0;while(r--){if(!t(e[r],i++)){n.splice(r,1)}}return n}},{"expand-range":24,preserve:110,"repeat-element":129}],10:[function(e,t,r){},{}],11:[function(e,t,r){arguments[4][10][0].apply(r,arguments)},{dup:10}],12:[function(e,t,r){var n=e("base-x");var i="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";var a=n(i);t.exports={encode:a.encode,decode:a.decode}},{"base-x":6}],13:[function(e,t,r){(function(t){"use strict";var n=e("base64-js");var i=e("ieee754");var a=e("isarray");r.Buffer=u;r.SlowBuffer=w;r.INSPECT_MAX_BYTES=50;u.poolSize=8192;var o={};u.TYPED_ARRAY_SUPPORT=t.TYPED_ARRAY_SUPPORT!==undefined?t.TYPED_ARRAY_SUPPORT:s();function s(){function e(){}try{var t=new Uint8Array(1);t.foo=function(){return 42};t.constructor=e;return t.foo()===42&&t.constructor===e&&typeof t.subarray==="function"&&t.subarray(1,1).byteLength===0}catch(r){return false}}function f(){return u.TYPED_ARRAY_SUPPORT?2147483647:1073741823}function u(e){if(!(this instanceof u)){if(arguments.length>1)return new u(e,arguments[1]);return new u(e)}if(!u.TYPED_ARRAY_SUPPORT){this.length=0;this.parent=undefined}if(typeof e==="number"){return c(this,e)}if(typeof e==="string"){return l(this,e,arguments.length>1?arguments[1]:"utf8")}return h(this,e)}function c(e,t){e=m(e,t<0?0:_(t)|0);if(!u.TYPED_ARRAY_SUPPORT){for(var r=0;r<t;r++){e[r]=0}}return e}function l(e,t,r){if(typeof r!=="string"||r==="")r="utf8";var n=E(t,r)|0;e=m(e,n);e.write(t,r);return e}function h(e,t){if(u.isBuffer(t))return p(e,t);if(a(t))return d(e,t);if(t==null){throw new TypeError("must start with number, buffer, array or string")}if(typeof ArrayBuffer!=="undefined"){if(t.buffer instanceof ArrayBuffer){return v(e,t)}if(t instanceof ArrayBuffer){return g(e,t)}}if(t.length)return y(e,t);return b(e,t)}function p(e,t){var r=_(t.length)|0;e=m(e,r);t.copy(e,0,0,r);return e}function d(e,t){var r=_(t.length)|0;e=m(e,r);for(var n=0;n<r;n+=1){e[n]=t[n]&255}return e}function v(e,t){var r=_(t.length)|0;e=m(e,r);for(var n=0;n<r;n+=1){e[n]=t[n]&255}return e}function g(e,t){if(u.TYPED_ARRAY_SUPPORT){t.byteLength;e=u._augment(new Uint8Array(t))}else{e=v(e,new Uint8Array(t))}return e}function y(e,t){var r=_(t.length)|0;e=m(e,r);for(var n=0;n<r;n+=1){e[n]=t[n]&255}return e}function b(e,t){var r;var n=0;if(t.type==="Buffer"&&a(t.data)){r=t.data;n=_(r.length)|0}e=m(e,n);for(var i=0;i<n;i+=1){e[i]=r[i]&255}return e}if(u.TYPED_ARRAY_SUPPORT){u.prototype.__proto__=Uint8Array.prototype;u.__proto__=Uint8Array}else{u.prototype.length=undefined;u.prototype.parent=undefined}function m(e,t){if(u.TYPED_ARRAY_SUPPORT){e=u._augment(new Uint8Array(t));e.__proto__=u.prototype}else{e.length=t;e._isBuffer=true}var r=t!==0&&t<=u.poolSize>>>1;if(r)e.parent=o;return e}function _(e){if(e>=f()){throw new RangeError("Attempt to allocate Buffer larger than maximum "+"size: 0x"+f().toString(16)+" bytes")}return e|0}function w(e,t){if(!(this instanceof w))return new w(e,t);var r=new u(e,t);delete r.parent;return r}u.isBuffer=function te(e){return!!(e!=null&&e._isBuffer)};u.compare=function re(e,t){if(!u.isBuffer(e)||!u.isBuffer(t)){throw new TypeError("Arguments must be Buffers")}if(e===t)return 0;var r=e.length;var n=t.length;var i=0;var a=Math.min(r,n);while(i<a){if(e[i]!==t[i])break;++i}if(i!==a){r=e[i];n=t[i]}if(r<n)return-1;if(n<r)return 1;return 0};u.isEncoding=function ne(e){switch(String(e).toLowerCase()){case"hex":case"utf8":case"utf-8":case"ascii":case"binary":case"base64":case"raw":case"ucs2":case"ucs-2":case"utf16le":case"utf-16le":return true;default:return false}};u.concat=function ie(e,t){if(!a(e))throw new TypeError("list argument must be an Array of Buffers.");if(e.length===0){return new u(0)}var r;if(t===undefined){t=0;for(r=0;r<e.length;r++){t+=e[r].length}}var n=new u(t);var i=0;for(r=0;r<e.length;r++){var o=e[r];o.copy(n,i);i+=o.length}return n};function E(e,t){if(typeof e!=="string")e=""+e;var r=e.length;if(r===0)return 0;var n=false;for(;;){switch(t){case"ascii":case"binary":case"raw":case"raws":return r;case"utf8":case"utf-8":return V(e).length;case"ucs2":case"ucs-2":case"utf16le":case"utf-16le":return r*2;case"hex":return r>>>1;case"base64":return Z(e).length;default:if(n)return V(e).length;t=(""+t).toLowerCase();n=true}}}u.byteLength=E;function S(e,t,r){var n=false;t=t|0;r=r===undefined||r===Infinity?this.length:r|0;if(!e)e="utf8";if(t<0)t=0;if(r>this.length)r=this.length;if(r<=t)return"";while(true){switch(e){case"hex":return N(this,t,r);case"utf8":case"utf-8":return L(this,t,r);case"ascii":return P(this,t,r);case"binary":return M(this,t,r);case"base64":return T(this,t,r);case"ucs2":case"ucs-2":case"utf16le":case"utf-16le":return B(this,t,r);default:if(n)throw new TypeError("Unknown encoding: "+e);e=(e+"").toLowerCase();n=true}}}u.prototype.toString=function ae(){var e=this.length|0;if(e===0)return"";if(arguments.length===0)return L(this,0,e);return S.apply(this,arguments)};u.prototype.equals=function oe(e){if(!u.isBuffer(e))throw new TypeError("Argument must be a Buffer");if(this===e)return true;return u.compare(this,e)===0};u.prototype.inspect=function se(){var e="";var t=r.INSPECT_MAX_BYTES;if(this.length>0){e=this.toString("hex",0,t).match(/.{2}/g).join(" ");if(this.length>t)e+=" ... "}return"<Buffer "+e+">"};u.prototype.compare=function fe(e){if(!u.isBuffer(e))throw new TypeError("Argument must be a Buffer");if(this===e)return 0;return u.compare(this,e)};u.prototype.indexOf=function ue(e,t){if(t>2147483647)t=2147483647;else if(t<-2147483648)t=-2147483648;t>>=0;if(this.length===0)return-1;if(t>=this.length)return-1;if(t<0)t=Math.max(this.length+t,0);if(typeof e==="string"){if(e.length===0)return-1;return String.prototype.indexOf.call(this,e,t)}if(u.isBuffer(e)){return r(this,e,t)}if(typeof e==="number"){if(u.TYPED_ARRAY_SUPPORT&&Uint8Array.prototype.indexOf==="function"){return Uint8Array.prototype.indexOf.call(this,e,t)}return r(this,[e],t)}function r(e,t,r){var n=-1;for(var i=0;r+i<e.length;i++){if(e[r+i]===t[n===-1?0:i-n]){if(n===-1)n=i;if(i-n+1===t.length)return r+n}else{n=-1}}return-1}throw new TypeError("val must be string, number or Buffer")};u.prototype.get=function ce(e){console.log(".get() is deprecated. Access using array indexes instead.");return this.readUInt8(e)};u.prototype.set=function le(e,t){console.log(".set() is deprecated. Access using array indexes instead.");return this.writeUInt8(e,t)};function x(e,t,r,n){r=Number(r)||0;var i=e.length-r;if(!n){n=i}else{n=Number(n);if(n>i){n=i}}var a=t.length;if(a%2!==0)throw new Error("Invalid hex string");if(n>a/2){n=a/2}for(var o=0;o<n;o++){var s=parseInt(t.substr(o*2,2),16);if(isNaN(s))throw new Error("Invalid hex string");e[r+o]=s}return o}function O(e,t,r,n){return ee(V(t,e.length-r),e,r,n)}function j(e,t,r,n){return ee(Q(t),e,r,n)}function A(e,t,r,n){return j(e,t,r,n)}function k(e,t,r,n){return ee(Z(t),e,r,n)}function R(e,t,r,n){return ee(J(t,e.length-r),e,r,n)}u.prototype.write=function he(e,t,r,n){if(t===undefined){n="utf8";r=this.length;t=0}else if(r===undefined&&typeof t==="string"){n=t;r=this.length;t=0}else if(isFinite(t)){t=t|0;if(isFinite(r)){r=r|0;if(n===undefined)n="utf8"}else{n=r;r=undefined}}else{var i=n;n=t;t=r|0;r=i}var a=this.length-t;if(r===undefined||r>a)r=a;if(e.length>0&&(r<0||t<0)||t>this.length){throw new RangeError("attempt to write outside buffer bounds")}if(!n)n="utf8";var o=false;for(;;){switch(n){case"hex":return x(this,e,t,r);case"utf8":case"utf-8":return O(this,e,t,r);case"ascii":return j(this,e,t,r);case"binary":return A(this,e,t,r);case"base64":return k(this,e,t,r);case"ucs2":case"ucs-2":case"utf16le":case"utf-16le":return R(this,e,t,r);default:if(o)throw new TypeError("Unknown encoding: "+n);n=(""+n).toLowerCase();o=true}}};u.prototype.toJSON=function pe(){return{type:"Buffer",data:Array.prototype.slice.call(this._arr||this,0)}};function T(e,t,r){if(t===0&&r===e.length){return n.fromByteArray(e)}else{return n.fromByteArray(e.slice(t,r))}}function L(e,t,r){r=Math.min(e.length,r);var n=[];var i=t;while(i<r){var a=e[i];var o=null;var s=a>239?4:a>223?3:a>191?2:1;if(i+s<=r){var f,u,c,l;switch(s){case 1:if(a<128){o=a}break;case 2:f=e[i+1];if((f&192)===128){l=(a&31)<<6|f&63;if(l>127){o=l}}break;case 3:f=e[i+1];u=e[i+2];if((f&192)===128&&(u&192)===128){l=(a&15)<<12|(f&63)<<6|u&63;if(l>2047&&(l<55296||l>57343)){o=l}}break;case 4:f=e[i+1];u=e[i+2];c=e[i+3];if((f&192)===128&&(u&192)===128&&(c&192)===128){l=(a&15)<<18|(f&63)<<12|(u&63)<<6|c&63;if(l>65535&&l<1114112){o=l}}}}if(o===null){o=65533;s=1}else if(o>65535){o-=65536;n.push(o>>>10&1023|55296);o=56320|o&1023}n.push(o);i+=s}return C(n)}var I=4096;function C(e){var t=e.length;if(t<=I){return String.fromCharCode.apply(String,e)}var r="";var n=0;while(n<t){r+=String.fromCharCode.apply(String,e.slice(n,n+=I))}return r}function P(e,t,r){var n="";r=Math.min(e.length,r);for(var i=t;i<r;i++){n+=String.fromCharCode(e[i]&127)}return n}function M(e,t,r){var n="";r=Math.min(e.length,r);for(var i=t;i<r;i++){n+=String.fromCharCode(e[i])}return n}function N(e,t,r){var n=e.length;if(!t||t<0)t=0;if(!r||r<0||r>n)r=n;var i="";for(var a=t;a<r;a++){i+=X(e[a])}return i}function B(e,t,r){var n=e.slice(t,r);var i="";for(var a=0;a<n.length;a+=2){i+=String.fromCharCode(n[a]+n[a+1]*256)}return i}u.prototype.slice=function de(e,t){var r=this.length;e=~~e;t=t===undefined?r:~~t;if(e<0){e+=r;if(e<0)e=0}else if(e>r){e=r}if(t<0){t+=r;if(t<0)t=0}else if(t>r){t=r}if(t<e)t=e;var n;if(u.TYPED_ARRAY_SUPPORT){n=u._augment(this.subarray(e,t))}else{var i=t-e;n=new u(i,undefined);for(var a=0;a<i;a++){n[a]=this[a+e]}}if(n.length)n.parent=this.parent||this;return n};function D(e,t,r){if(e%1!==0||e<0)throw new RangeError("offset is not uint");if(e+t>r)throw new RangeError("Trying to access beyond buffer length")}u.prototype.readUIntLE=function ve(e,t,r){e=e|0;t=t|0;if(!r)D(e,t,this.length);var n=this[e];var i=1;var a=0;while(++a<t&&(i*=256)){n+=this[e+a]*i}return n};u.prototype.readUIntBE=function ge(e,t,r){e=e|0;t=t|0;if(!r){D(e,t,this.length)}var n=this[e+--t];var i=1;while(t>0&&(i*=256)){n+=this[e+--t]*i}return n};u.prototype.readUInt8=function ye(e,t){if(!t)D(e,1,this.length);return this[e]};u.prototype.readUInt16LE=function be(e,t){if(!t)D(e,2,this.length);return this[e]|this[e+1]<<8};u.prototype.readUInt16BE=function me(e,t){if(!t)D(e,2,this.length);return this[e]<<8|this[e+1]};u.prototype.readUInt32LE=function _e(e,t){if(!t)D(e,4,this.length);return(this[e]|this[e+1]<<8|this[e+2]<<16)+this[e+3]*16777216};u.prototype.readUInt32BE=function we(e,t){if(!t)D(e,4,this.length);return this[e]*16777216+(this[e+1]<<16|this[e+2]<<8|this[e+3])};u.prototype.readIntLE=function Ee(e,t,r){e=e|0;t=t|0;if(!r)D(e,t,this.length);var n=this[e];var i=1;var a=0;while(++a<t&&(i*=256)){n+=this[e+a]*i}i*=128;if(n>=i)n-=Math.pow(2,8*t);return n};u.prototype.readIntBE=function Se(e,t,r){e=e|0;t=t|0;if(!r)D(e,t,this.length);var n=t;var i=1;var a=this[e+--n];while(n>0&&(i*=256)){a+=this[e+--n]*i}i*=128;if(a>=i)a-=Math.pow(2,8*t);return a};u.prototype.readInt8=function xe(e,t){if(!t)D(e,1,this.length);if(!(this[e]&128))return this[e];return(255-this[e]+1)*-1};u.prototype.readInt16LE=function Oe(e,t){if(!t)D(e,2,this.length);var r=this[e]|this[e+1]<<8;return r&32768?r|4294901760:r};u.prototype.readInt16BE=function je(e,t){if(!t)D(e,2,this.length);var r=this[e+1]|this[e]<<8;return r&32768?r|4294901760:r};u.prototype.readInt32LE=function Ae(e,t){if(!t)D(e,4,this.length);return this[e]|this[e+1]<<8|this[e+2]<<16|this[e+3]<<24};u.prototype.readInt32BE=function ke(e,t){if(!t)D(e,4,this.length);return this[e]<<24|this[e+1]<<16|this[e+2]<<8|this[e+3]};u.prototype.readFloatLE=function Re(e,t){if(!t)D(e,4,this.length);return i.read(this,e,true,23,4)};u.prototype.readFloatBE=function Te(e,t){if(!t)D(e,4,this.length);return i.read(this,e,false,23,4)};u.prototype.readDoubleLE=function Le(e,t){if(!t)D(e,8,this.length);return i.read(this,e,true,52,8)};u.prototype.readDoubleBE=function Ie(e,t){if(!t)D(e,8,this.length);return i.read(this,e,false,52,8)};function U(e,t,r,n,i,a){if(!u.isBuffer(e))throw new TypeError("buffer must be a Buffer instance");if(t>i||t<a)throw new RangeError("value is out of bounds");if(r+n>e.length)throw new RangeError("index out of range")}u.prototype.writeUIntLE=function Ce(e,t,r,n){e=+e;t=t|0;r=r|0;if(!n)U(this,e,t,r,Math.pow(2,8*r),0);var i=1;var a=0;this[t]=e&255;while(++a<r&&(i*=256)){this[t+a]=e/i&255}return t+r};u.prototype.writeUIntBE=function Pe(e,t,r,n){e=+e;t=t|0;r=r|0;if(!n)U(this,e,t,r,Math.pow(2,8*r),0);var i=r-1;var a=1;this[t+i]=e&255;while(--i>=0&&(a*=256)){this[t+i]=e/a&255}return t+r};u.prototype.writeUInt8=function Me(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,1,255,0);if(!u.TYPED_ARRAY_SUPPORT)e=Math.floor(e);this[t]=e&255;return t+1};function F(e,t,r,n){if(t<0)t=65535+t+1;for(var i=0,a=Math.min(e.length-r,2);i<a;i++){e[r+i]=(t&255<<8*(n?i:1-i))>>>(n?i:1-i)*8}}u.prototype.writeUInt16LE=function Ne(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,2,65535,0);if(u.TYPED_ARRAY_SUPPORT){this[t]=e&255;this[t+1]=e>>>8}else{F(this,e,t,true)}return t+2};u.prototype.writeUInt16BE=function Be(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,2,65535,0);if(u.TYPED_ARRAY_SUPPORT){this[t]=e>>>8;this[t+1]=e&255}else{F(this,e,t,false)}return t+2};function q(e,t,r,n){if(t<0)t=4294967295+t+1;for(var i=0,a=Math.min(e.length-r,4);i<a;i++){e[r+i]=t>>>(n?i:3-i)*8&255}}u.prototype.writeUInt32LE=function De(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,4,4294967295,0);if(u.TYPED_ARRAY_SUPPORT){this[t+3]=e>>>24;this[t+2]=e>>>16;this[t+1]=e>>>8;this[t]=e&255}else{q(this,e,t,true)}return t+4};u.prototype.writeUInt32BE=function Ue(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,4,4294967295,0);if(u.TYPED_ARRAY_SUPPORT){this[t]=e>>>24;this[t+1]=e>>>16;this[t+2]=e>>>8;this[t+3]=e&255}else{q(this,e,t,false)}return t+4};u.prototype.writeIntLE=function Fe(e,t,r,n){e=+e;t=t|0;if(!n){var i=Math.pow(2,8*r-1);U(this,e,t,r,i-1,-i)}var a=0;var o=1;var s=e<0?1:0;this[t]=e&255;while(++a<r&&(o*=256)){this[t+a]=(e/o>>0)-s&255}return t+r};u.prototype.writeIntBE=function qe(e,t,r,n){e=+e;t=t|0;if(!n){var i=Math.pow(2,8*r-1);U(this,e,t,r,i-1,-i)}var a=r-1;var o=1;var s=e<0?1:0;this[t+a]=e&255;while(--a>=0&&(o*=256)){this[t+a]=(e/o>>0)-s&255}return t+r};u.prototype.writeInt8=function Ge(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,1,127,-128);if(!u.TYPED_ARRAY_SUPPORT)e=Math.floor(e);if(e<0)e=255+e+1;this[t]=e&255;return t+1};u.prototype.writeInt16LE=function $e(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,2,32767,-32768);if(u.TYPED_ARRAY_SUPPORT){this[t]=e&255;this[t+1]=e>>>8}else{F(this,e,t,true)}return t+2};u.prototype.writeInt16BE=function We(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,2,32767,-32768);if(u.TYPED_ARRAY_SUPPORT){this[t]=e>>>8;this[t+1]=e&255}else{F(this,e,t,false)}return t+2};u.prototype.writeInt32LE=function He(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,4,2147483647,-2147483648);if(u.TYPED_ARRAY_SUPPORT){this[t]=e&255;this[t+1]=e>>>8;this[t+2]=e>>>16;this[t+3]=e>>>24}else{q(this,e,t,true)}return t+4};u.prototype.writeInt32BE=function Ye(e,t,r){e=+e;t=t|0;if(!r)U(this,e,t,4,2147483647,-2147483648);if(e<0)e=4294967295+e+1;if(u.TYPED_ARRAY_SUPPORT){this[t]=e>>>24;this[t+1]=e>>>16;this[t+2]=e>>>8;this[t+3]=e&255}else{q(this,e,t,false)}return t+4};function G(e,t,r,n,i,a){if(t>i||t<a)throw new RangeError("value is out of bounds");if(r+n>e.length)throw new RangeError("index out of range");if(r<0)throw new RangeError("index out of range")}function $(e,t,r,n,a){if(!a){G(e,t,r,4,3.4028234663852886e38,-3.4028234663852886e38)}i.write(e,t,r,n,23,4);return r+4}u.prototype.writeFloatLE=function ze(e,t,r){return $(this,e,t,true,r)};u.prototype.writeFloatBE=function Ke(e,t,r){return $(this,e,t,false,r)};function W(e,t,r,n,a){if(!a){G(e,t,r,8,1.7976931348623157e308,-1.7976931348623157e308)}i.write(e,t,r,n,52,8);return r+8}u.prototype.writeDoubleLE=function Xe(e,t,r){return W(this,e,t,true,r)};u.prototype.writeDoubleBE=function Ve(e,t,r){return W(this,e,t,false,r)};u.prototype.copy=function Qe(e,t,r,n){if(!r)r=0;if(!n&&n!==0)n=this.length;if(t>=e.length)t=e.length;if(!t)t=0;if(n>0&&n<r)n=r;if(n===r)return 0;if(e.length===0||this.length===0)return 0;if(t<0){throw new RangeError("targetStart out of bounds")}if(r<0||r>=this.length)throw new RangeError("sourceStart out of bounds");if(n<0)throw new RangeError("sourceEnd out of bounds");if(n>this.length)n=this.length;if(e.length-t<n-r){n=e.length-t+r}var i=n-r;var a;if(this===e&&r<t&&t<n){for(a=i-1;a>=0;a--){e[a+t]=this[a+r]}}else if(i<1e3||!u.TYPED_ARRAY_SUPPORT){for(a=0;a<i;a++){e[a+t]=this[a+r]}}else{e._set(this.subarray(r,r+i),t)}return i};u.prototype.fill=function Je(e,t,r){if(!e)e=0;if(!t)t=0;if(!r)r=this.length;if(r<t)throw new RangeError("end < start");if(r===t)return;if(this.length===0)return;if(t<0||t>=this.length)throw new RangeError("start out of bounds");if(r<0||r>this.length)throw new RangeError("end out of bounds");var n;if(typeof e==="number"){for(n=t;n<r;n++){this[n]=e}}else{var i=V(e.toString());var a=i.length;for(n=t;n<r;n++){this[n]=i[n%a]}}return this};u.prototype.toArrayBuffer=function Ze(){if(typeof Uint8Array!=="undefined"){if(u.TYPED_ARRAY_SUPPORT){return new u(this).buffer}else{var e=new Uint8Array(this.length);for(var t=0,r=e.length;t<r;t+=1){e[t]=this[t]}return e.buffer}}else{throw new TypeError("Buffer.toArrayBuffer not supported in this browser")}};var H=u.prototype;u._augment=function et(e){e.constructor=u;e._isBuffer=true;e._set=e.set;e.get=H.get;e.set=H.set;e.write=H.write;e.toString=H.toString;e.toLocaleString=H.toString;e.toJSON=H.toJSON;e.equals=H.equals;e.compare=H.compare;e.indexOf=H.indexOf;e.copy=H.copy;e.slice=H.slice;e.readUIntLE=H.readUIntLE;e.readUIntBE=H.readUIntBE;e.readUInt8=H.readUInt8;e.readUInt16LE=H.readUInt16LE;e.readUInt16BE=H.readUInt16BE;e.readUInt32LE=H.readUInt32LE;e.readUInt32BE=H.readUInt32BE;e.readIntLE=H.readIntLE;e.readIntBE=H.readIntBE;e.readInt8=H.readInt8;e.readInt16LE=H.readInt16LE;e.readInt16BE=H.readInt16BE;e.readInt32LE=H.readInt32LE;e.readInt32BE=H.readInt32BE;e.readFloatLE=H.readFloatLE;e.readFloatBE=H.readFloatBE;e.readDoubleLE=H.readDoubleLE;e.readDoubleBE=H.readDoubleBE;e.writeUInt8=H.writeUInt8;e.writeUIntLE=H.writeUIntLE;e.writeUIntBE=H.writeUIntBE;e.writeUInt16LE=H.writeUInt16LE;e.writeUInt16BE=H.writeUInt16BE;e.writeUInt32LE=H.writeUInt32LE;e.writeUInt32BE=H.writeUInt32BE;e.writeIntLE=H.writeIntLE;e.writeIntBE=H.writeIntBE;e.writeInt8=H.writeInt8;e.writeInt16LE=H.writeInt16LE;e.writeInt16BE=H.writeInt16BE;e.writeInt32LE=H.writeInt32LE;e.writeInt32BE=H.writeInt32BE;
e.writeFloatLE=H.writeFloatLE;e.writeFloatBE=H.writeFloatBE;e.writeDoubleLE=H.writeDoubleLE;e.writeDoubleBE=H.writeDoubleBE;e.fill=H.fill;e.inspect=H.inspect;e.toArrayBuffer=H.toArrayBuffer;return e};var Y=/[^+\/0-9A-Za-z-_]/g;function z(e){e=K(e).replace(Y,"");if(e.length<2)return"";while(e.length%4!==0){e=e+"="}return e}function K(e){if(e.trim)return e.trim();return e.replace(/^\s+|\s+$/g,"")}function X(e){if(e<16)return"0"+e.toString(16);return e.toString(16)}function V(e,t){t=t||Infinity;var r;var n=e.length;var i=null;var a=[];for(var o=0;o<n;o++){r=e.charCodeAt(o);if(r>55295&&r<57344){if(!i){if(r>56319){if((t-=3)>-1)a.push(239,191,189);continue}else if(o+1===n){if((t-=3)>-1)a.push(239,191,189);continue}i=r;continue}if(r<56320){if((t-=3)>-1)a.push(239,191,189);i=r;continue}r=(i-55296<<10|r-56320)+65536}else if(i){if((t-=3)>-1)a.push(239,191,189)}i=null;if(r<128){if((t-=1)<0)break;a.push(r)}else if(r<2048){if((t-=2)<0)break;a.push(r>>6|192,r&63|128)}else if(r<65536){if((t-=3)<0)break;a.push(r>>12|224,r>>6&63|128,r&63|128)}else if(r<1114112){if((t-=4)<0)break;a.push(r>>18|240,r>>12&63|128,r>>6&63|128,r&63|128)}else{throw new Error("Invalid code point")}}return a}function Q(e){var t=[];for(var r=0;r<e.length;r++){t.push(e.charCodeAt(r)&255)}return t}function J(e,t){var r,n,i;var a=[];for(var o=0;o<e.length;o++){if((t-=2)<0)break;r=e.charCodeAt(o);n=r>>8;i=r%256;a.push(i);a.push(n)}return a}function Z(e){return n.toByteArray(z(e))}function ee(e,t,r,n){for(var i=0;i<n;i++){if(i+r>=t.length||i>=e.length)break;t[i+r]=e[i]}return i}}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{"base64-js":7,ieee754:54,isarray:70}],14:[function(e,t,r){t.exports={100:"Continue",101:"Switching Protocols",102:"Processing",200:"OK",201:"Created",202:"Accepted",203:"Non-Authoritative Information",204:"No Content",205:"Reset Content",206:"Partial Content",207:"Multi-Status",300:"Multiple Choices",301:"Moved Permanently",302:"Moved Temporarily",303:"See Other",304:"Not Modified",305:"Use Proxy",307:"Temporary Redirect",308:"Permanent Redirect",400:"Bad Request",401:"Unauthorized",402:"Payment Required",403:"Forbidden",404:"Not Found",405:"Method Not Allowed",406:"Not Acceptable",407:"Proxy Authentication Required",408:"Request Time-out",409:"Conflict",410:"Gone",411:"Length Required",412:"Precondition Failed",413:"Request Entity Too Large",414:"Request-URI Too Large",415:"Unsupported Media Type",416:"Requested Range Not Satisfiable",417:"Expectation Failed",418:"I'm a teapot",422:"Unprocessable Entity",423:"Locked",424:"Failed Dependency",425:"Unordered Collection",426:"Upgrade Required",428:"Precondition Required",429:"Too Many Requests",431:"Request Header Fields Too Large",500:"Internal Server Error",501:"Not Implemented",502:"Bad Gateway",503:"Service Unavailable",504:"Gateway Time-out",505:"HTTP Version Not Supported",506:"Variant Also Negotiates",507:"Insufficient Storage",509:"Bandwidth Limit Exceeded",510:"Not Extended",511:"Network Authentication Required"}},{}],15:[function(e,t,r){var n=e("fs").Stats;t.exports=i;function i(e){var t=new n;Object.keys(e).forEach(function(r){t[r]=e[r]});return t}},{fs:11}],16:[function(e,t,r){(function(e){var r=function(){"use strict";function t(r,n,i,a){var s;if(typeof n==="object"){i=n.depth;a=n.prototype;s=n.filter;n=n.circular}var f=[];var u=[];var c=typeof e!="undefined";if(typeof n=="undefined")n=true;if(typeof i=="undefined")i=Infinity;function l(r,i){if(r===null)return null;if(i==0)return r;var s;var h;if(typeof r!="object"){return r}if(t.__isArray(r)){s=[]}else if(t.__isRegExp(r)){s=new RegExp(r.source,o(r));if(r.lastIndex)s.lastIndex=r.lastIndex}else if(t.__isDate(r)){s=new Date(r.getTime())}else if(c&&e.isBuffer(r)){s=new e(r.length);r.copy(s);return s}else{if(typeof a=="undefined"){h=Object.getPrototypeOf(r);s=Object.create(h)}else{s=Object.create(a);h=a}}if(n){var p=f.indexOf(r);if(p!=-1){return u[p]}f.push(r);u.push(s)}for(var d in r){var v;if(h){v=Object.getOwnPropertyDescriptor(h,d)}if(v&&v.set==null){continue}s[d]=l(r[d],i-1)}return s}return l(r,i)}t.clonePrototype=function s(e){if(e===null)return null;var t=function(){};t.prototype=e;return new t};function r(e){return Object.prototype.toString.call(e)}t.__objToStr=r;function n(e){return typeof e==="object"&&r(e)==="[object Date]"}t.__isDate=n;function i(e){return typeof e==="object"&&r(e)==="[object Array]"}t.__isArray=i;function a(e){return typeof e==="object"&&r(e)==="[object RegExp]"}t.__isRegExp=a;function o(e){var t="";if(e.global)t+="g";if(e.ignoreCase)t+="i";if(e.multiline)t+="m";return t}t.__getRegExpFlags=o;return t}();if(typeof t==="object"&&t.exports){t.exports=r}}).call(this,e("buffer").Buffer)},{buffer:13}],17:[function(e,t,r){t.exports=function(e,t){var r=[];for(var i=0;i<e.length;i++){var a=t(e[i],i);if(n(a))r.push.apply(r,a);else r.push(a)}return r};var n=Array.isArray||function(e){return Object.prototype.toString.call(e)==="[object Array]"}},{}],18:[function(e,t,r){t.exports={O_RDONLY:0,O_WRONLY:1,O_RDWR:2,S_IFMT:61440,S_IFREG:32768,S_IFDIR:16384,S_IFCHR:8192,S_IFBLK:24576,S_IFIFO:4096,S_IFLNK:40960,S_IFSOCK:49152,O_CREAT:512,O_EXCL:2048,O_NOCTTY:131072,O_TRUNC:1024,O_APPEND:8,O_DIRECTORY:1048576,O_NOFOLLOW:256,O_SYNC:128,O_SYMLINK:2097152,S_IRWXU:448,S_IRUSR:256,S_IWUSR:128,S_IXUSR:64,S_IRWXG:56,S_IRGRP:32,S_IWGRP:16,S_IXGRP:8,S_IRWXO:7,S_IROTH:4,S_IWOTH:2,S_IXOTH:1,E2BIG:7,EACCES:13,EADDRINUSE:48,EADDRNOTAVAIL:49,EAFNOSUPPORT:47,EAGAIN:35,EALREADY:37,EBADF:9,EBADMSG:94,EBUSY:16,ECANCELED:89,ECHILD:10,ECONNABORTED:53,ECONNREFUSED:61,ECONNRESET:54,EDEADLK:11,EDESTADDRREQ:39,EDOM:33,EDQUOT:69,EEXIST:17,EFAULT:14,EFBIG:27,EHOSTUNREACH:65,EIDRM:90,EILSEQ:92,EINPROGRESS:36,EINTR:4,EINVAL:22,EIO:5,EISCONN:56,EISDIR:21,ELOOP:62,EMFILE:24,EMLINK:31,EMSGSIZE:40,EMULTIHOP:95,ENAMETOOLONG:63,ENETDOWN:50,ENETRESET:52,ENETUNREACH:51,ENFILE:23,ENOBUFS:55,ENODATA:96,ENODEV:19,ENOENT:2,ENOEXEC:8,ENOLCK:77,ENOLINK:97,ENOMEM:12,ENOMSG:91,ENOPROTOOPT:42,ENOSPC:28,ENOSR:98,ENOSTR:99,ENOSYS:78,ENOTCONN:57,ENOTDIR:20,ENOTEMPTY:66,ENOTSOCK:38,ENOTSUP:45,ENOTTY:25,ENXIO:6,EOPNOTSUPP:102,EOVERFLOW:84,EPERM:1,EPIPE:32,EPROTO:100,EPROTONOSUPPORT:43,EPROTOTYPE:41,ERANGE:34,EROFS:30,ESPIPE:29,ESRCH:3,ESTALE:70,ETIME:101,ETIMEDOUT:60,ETXTBSY:26,EWOULDBLOCK:35,EXDEV:18,SIGHUP:1,SIGINT:2,SIGQUIT:3,SIGILL:4,SIGTRAP:5,SIGABRT:6,SIGIOT:6,SIGBUS:10,SIGFPE:8,SIGKILL:9,SIGUSR1:30,SIGSEGV:11,SIGUSR2:31,SIGPIPE:13,SIGALRM:14,SIGTERM:15,SIGCHLD:20,SIGCONT:19,SIGSTOP:17,SIGTSTP:18,SIGTTIN:21,SIGTTOU:22,SIGURG:16,SIGXCPU:24,SIGXFSZ:25,SIGVTALRM:26,SIGPROF:27,SIGWINCH:28,SIGIO:23,SIGSYS:12,SSL_OP_ALL:2147486719,SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION:262144,SSL_OP_CIPHER_SERVER_PREFERENCE:4194304,SSL_OP_CISCO_ANYCONNECT:32768,SSL_OP_COOKIE_EXCHANGE:8192,SSL_OP_CRYPTOPRO_TLSEXT_BUG:2147483648,SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS:2048,SSL_OP_EPHEMERAL_RSA:2097152,SSL_OP_LEGACY_SERVER_CONNECT:4,SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER:32,SSL_OP_MICROSOFT_SESS_ID_BUG:1,SSL_OP_MSIE_SSLV2_RSA_PADDING:64,SSL_OP_NETSCAPE_CA_DN_BUG:536870912,SSL_OP_NETSCAPE_CHALLENGE_BUG:2,SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG:1073741824,SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG:8,SSL_OP_NO_COMPRESSION:131072,SSL_OP_NO_QUERY_MTU:4096,SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION:65536,SSL_OP_NO_SSLv2:16777216,SSL_OP_NO_SSLv3:33554432,SSL_OP_NO_TICKET:16384,SSL_OP_NO_TLSv1:67108864,SSL_OP_NO_TLSv1_1:268435456,SSL_OP_NO_TLSv1_2:134217728,SSL_OP_PKCS1_CHECK_1:0,SSL_OP_PKCS1_CHECK_2:0,SSL_OP_SINGLE_DH_USE:1048576,SSL_OP_SINGLE_ECDH_USE:524288,SSL_OP_SSLEAY_080_CLIENT_DH_BUG:128,SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG:16,SSL_OP_TLS_BLOCK_PADDING_BUG:512,SSL_OP_TLS_D5_BUG:256,SSL_OP_TLS_ROLLBACK_BUG:8388608,NPN_ENABLED:1}},{}],19:[function(e,t,r){(function(e){function t(e){if(Array.isArray){return Array.isArray(e)}return g(e)==="[object Array]"}r.isArray=t;function n(e){return typeof e==="boolean"}r.isBoolean=n;function i(e){return e===null}r.isNull=i;function a(e){return e==null}r.isNullOrUndefined=a;function o(e){return typeof e==="number"}r.isNumber=o;function s(e){return typeof e==="string"}r.isString=s;function f(e){return typeof e==="symbol"}r.isSymbol=f;function u(e){return e===void 0}r.isUndefined=u;function c(e){return g(e)==="[object RegExp]"}r.isRegExp=c;function l(e){return typeof e==="object"&&e!==null}r.isObject=l;function h(e){return g(e)==="[object Date]"}r.isDate=h;function p(e){return g(e)==="[object Error]"||e instanceof Error}r.isError=p;function d(e){return typeof e==="function"}r.isFunction=d;function v(e){return e===null||typeof e==="boolean"||typeof e==="number"||typeof e==="string"||typeof e==="symbol"||typeof e==="undefined"}r.isPrimitive=v;r.isBuffer=e.isBuffer;function g(e){return Object.prototype.toString.call(e)}}).call(this,{isBuffer:e("../../is-buffer/index.js")})},{"../../is-buffer/index.js":59}],20:[function(e,t,r){(function(r,n){var i=e("readable-stream");var a=e("end-of-stream");var o=e("inherits");var s=new n([0]);var f=function(e,t){if(e._corked)e.once("uncork",t);else t()};var u=function(e,t){return function(r){if(r)e.destroy(r.message==="premature close"?null:r);else if(t&&!e._ended)e.end()}};var c=function(e,t){if(!e)return t();if(e._writableState&&e._writableState.finished)return t();if(e._writableState)return e.end(t);e.end();t()};var l=function(e){return new i.Readable({objectMode:true,highWaterMark:16}).wrap(e)};var h=function(e,t,r){if(!(this instanceof h))return new h(e,t,r);i.Duplex.call(this,r);this._writable=null;this._readable=null;this._readable2=null;this._forwardDestroy=!r||r.destroy!==false;this._forwardEnd=!r||r.end!==false;this._corked=1;this._ondrain=null;this._drained=false;this._forwarding=false;this._unwrite=null;this._unread=null;this._ended=false;this.destroyed=false;if(e)this.setWritable(e);if(t)this.setReadable(t)};o(h,i.Duplex);h.obj=function(e,t,r){if(!r)r={};r.objectMode=true;r.highWaterMark=16;return new h(e,t,r)};h.prototype.cork=function(){if(++this._corked===1)this.emit("cork")};h.prototype.uncork=function(){if(this._corked&&--this._corked===0)this.emit("uncork")};h.prototype.setWritable=function(e){if(this._unwrite)this._unwrite();if(this.destroyed){if(e&&e.destroy)e.destroy();return}if(e===null||e===false){this.end();return}var t=this;var n=a(e,{writable:true,readable:false},u(this,this._forwardEnd));var i=function(){var e=t._ondrain;t._ondrain=null;if(e)e()};var o=function(){t._writable.removeListener("drain",i);n()};if(this._unwrite)r.nextTick(i);this._writable=e;this._writable.on("drain",i);this._unwrite=o;this.uncork()};h.prototype.setReadable=function(e){if(this._unread)this._unread();if(this.destroyed){if(e&&e.destroy)e.destroy();return}if(e===null||e===false){this.push(null);this.resume();return}var t=this;var r=a(e,{writable:false,readable:true},u(this));var n=function(){t._forward()};var i=function(){t.push(null)};var o=function(){t._readable2.removeListener("readable",n);t._readable2.removeListener("end",i);r()};this._drained=true;this._readable=e;this._readable2=e._readableState?e:l(e);this._readable2.on("readable",n);this._readable2.on("end",i);this._unread=o;this._forward()};h.prototype._read=function(){this._drained=true;this._forward()};h.prototype._forward=function(){if(this._forwarding||!this._readable2||!this._drained)return;this._forwarding=true;var e;var t=this._readable2._readableState;while((e=this._readable2.read(t.buffer.length?t.buffer[0].length:t.length))!==null){this._drained=this.push(e)}this._forwarding=false};h.prototype.destroy=function(e){if(this.destroyed)return;this.destroyed=true;var t=this;r.nextTick(function(){t._destroy(e)})};h.prototype._destroy=function(e){if(e){var t=this._ondrain;this._ondrain=null;if(t)t(e);else this.emit("error",e)}if(this._forwardDestroy){if(this._readable&&this._readable.destroy)this._readable.destroy();if(this._writable&&this._writable.destroy)this._writable.destroy()}this.emit("close")};h.prototype._write=function(e,t,r){if(this.destroyed)return r();if(this._corked)return f(this,this._write.bind(this,e,t,r));if(e===s)return this._finish(r);if(!this._writable)return r();if(this._writable.write(e)===false)this._ondrain=r;else r()};h.prototype._finish=function(e){var t=this;this.emit("preend");f(this,function(){c(t._forwardEnd&&t._writable,function(){if(t._writableState.prefinished===false)t._writableState.prefinished=true;t.emit("prefinish");f(t,e)})})};h.prototype.end=function(e,t,r){if(typeof e==="function")return this.end(null,null,e);if(typeof t==="function")return this.end(e,null,t);this._ended=true;if(e)this.write(e);if(!this._writableState.ending)this.write(s);return i.Writable.prototype.end.call(this,r)};t.exports=h}).call(this,e("_process"),e("buffer").Buffer)},{_process:112,buffer:13,"end-of-stream":21,inherits:57,"readable-stream":125}],21:[function(e,t,r){var n=e("once");var i=function(){};var a=function(e){return e.setHeader&&typeof e.abort==="function"};var o=function(e,t,r){if(typeof t==="function")return o(e,null,t);if(!t)t={};r=n(r||i);var s=e._writableState;var f=e._readableState;var u=t.readable||t.readable!==false&&e.readable;var c=t.writable||t.writable!==false&&e.writable;var l=function(){if(!e.writable)h()};var h=function(){c=false;if(!u)r()};var p=function(){u=false;if(!c)r()};var d=function(){if(u&&!(f&&f.ended))return r(new Error("premature close"));if(c&&!(s&&s.ended))return r(new Error("premature close"))};var v=function(){e.req.on("finish",h)};if(a(e)){e.on("complete",h);e.on("abort",d);if(e.req)v();else e.on("request",v)}else if(c&&!s){e.on("end",l);e.on("close",l)}e.on("end",p);e.on("finish",h);if(t.error!==false)e.on("error",r);e.on("close",d);return function(){e.removeListener("complete",h);e.removeListener("abort",d);e.removeListener("request",v);if(e.req)e.req.removeListener("finish",h);e.removeListener("end",l);e.removeListener("close",l);e.removeListener("finish",h);e.removeListener("end",p);e.removeListener("error",r);e.removeListener("close",d)}};t.exports=o},{once:104}],22:[function(e,t,r){function n(){this._events=this._events||{};this._maxListeners=this._maxListeners||undefined}t.exports=n;n.EventEmitter=n;n.prototype._events=undefined;n.prototype._maxListeners=undefined;n.defaultMaxListeners=10;n.prototype.setMaxListeners=function(e){if(!a(e)||e<0||isNaN(e))throw TypeError("n must be a positive number");this._maxListeners=e;return this};n.prototype.emit=function(e){var t,r,n,a,f,u;if(!this._events)this._events={};if(e==="error"){if(!this._events.error||o(this._events.error)&&!this._events.error.length){t=arguments[1];if(t instanceof Error){throw t}throw TypeError('Uncaught, unspecified "error" event.')}}r=this._events[e];if(s(r))return false;if(i(r)){switch(arguments.length){case 1:r.call(this);break;case 2:r.call(this,arguments[1]);break;case 3:r.call(this,arguments[1],arguments[2]);break;default:n=arguments.length;a=new Array(n-1);for(f=1;f<n;f++)a[f-1]=arguments[f];r.apply(this,a)}}else if(o(r)){n=arguments.length;a=new Array(n-1);for(f=1;f<n;f++)a[f-1]=arguments[f];u=r.slice();n=u.length;for(f=0;f<n;f++)u[f].apply(this,a)}return true};n.prototype.addListener=function(e,t){var r;if(!i(t))throw TypeError("listener must be a function");if(!this._events)this._events={};if(this._events.newListener)this.emit("newListener",e,i(t.listener)?t.listener:t);if(!this._events[e])this._events[e]=t;else if(o(this._events[e]))this._events[e].push(t);else this._events[e]=[this._events[e],t];if(o(this._events[e])&&!this._events[e].warned){var r;if(!s(this._maxListeners)){r=this._maxListeners}else{r=n.defaultMaxListeners}if(r&&r>0&&this._events[e].length>r){this._events[e].warned=true;console.error("(node) warning: possible EventEmitter memory "+"leak detected. %d listeners added. "+"Use emitter.setMaxListeners() to increase limit.",this._events[e].length);if(typeof console.trace==="function"){console.trace()}}}return this};n.prototype.on=n.prototype.addListener;n.prototype.once=function(e,t){if(!i(t))throw TypeError("listener must be a function");var r=false;function n(){this.removeListener(e,n);if(!r){r=true;t.apply(this,arguments)}}n.listener=t;this.on(e,n);return this};n.prototype.removeListener=function(e,t){var r,n,a,s;if(!i(t))throw TypeError("listener must be a function");if(!this._events||!this._events[e])return this;r=this._events[e];a=r.length;n=-1;if(r===t||i(r.listener)&&r.listener===t){delete this._events[e];if(this._events.removeListener)this.emit("removeListener",e,t)}else if(o(r)){for(s=a;s-- >0;){if(r[s]===t||r[s].listener&&r[s].listener===t){n=s;break}}if(n<0)return this;if(r.length===1){r.length=0;delete this._events[e]}else{r.splice(n,1)}if(this._events.removeListener)this.emit("removeListener",e,t)}return this};n.prototype.removeAllListeners=function(e){var t,r;if(!this._events)return this;if(!this._events.removeListener){if(arguments.length===0)this._events={};else if(this._events[e])delete this._events[e];return this}if(arguments.length===0){for(t in this._events){if(t==="removeListener")continue;this.removeAllListeners(t)}this.removeAllListeners("removeListener");this._events={};return this}r=this._events[e];if(i(r)){this.removeListener(e,r)}else{while(r.length)this.removeListener(e,r[r.length-1])}delete this._events[e];return this};n.prototype.listeners=function(e){var t;if(!this._events||!this._events[e])t=[];else if(i(this._events[e]))t=[this._events[e]];else t=this._events[e].slice();return t};n.listenerCount=function(e,t){var r;if(!e._events||!e._events[t])r=0;else if(i(e._events[t]))r=1;else r=e._events[t].length;return r};function i(e){return typeof e==="function"}function a(e){return typeof e==="number"}function o(e){return typeof e==="object"&&e!==null}function s(e){return e===void 0}},{}],23:[function(e,t,r){"use strict";var n=e("is-posix-bracket");var i={alnum:"a-zA-Z0-9",alpha:"a-zA-Z",blank:" \\t",cntrl:"\\x00-\\x1F\\x7F",digit:"0-9",graph:"\\x21-\\x7E",lower:"a-z",print:"\\x20-\\x7E",punct:"-!\"#$%&'()\\*+,./:;<=>?@[\\]^_`{|}~",space:" \\t\\r\\n\\v\\f",upper:"A-Z",word:"A-Za-z0-9_",xdigit:"A-Fa-f0-9"};t.exports=a;function a(e){if(!n(e)){return e}var t=false;if(e.indexOf("[^")!==-1){t=true;e=e.split("[^").join("[")}if(e.indexOf("[!")!==-1){t=true;e=e.split("[!").join("[")}var r=e.split("[");var a=e.split("]");var o=r.length!==a.length;var s=e.split(/(?::\]\[:|\[?\[:|:\]\]?)/);var f=s.length,u=0;var c="",l="";var h=[];while(f--){var p=s[u++];if(p==="^[!"||p==="[!"){p="";t=true}var d=t?"^":"";var v=i[p];if(v){h.push("["+d+v+"]")}else if(p){if(/^\[?\w-\w\]?$/.test(p)){if(u===s.length){h.push("["+d+p)}else if(u===1){h.push(d+p+"]")}else{h.push(d+p)}}else{if(u===1){l+=p}else if(u===s.length){c+=p}else{h.push("["+d+p+"]")}}}}var g=h.join("|");var y=h.length||1;if(y>1){g="(?:"+g+")";y=1}if(l){y++;if(l.charAt(0)==="["){if(o){l="\\["+l.slice(1)}else{l+="]"}}g=l+g}if(c){y++;if(c.slice(-1)==="]"){if(o){c=c.slice(0,c.length-1)+"\\]"}else{c="["+c}}g+=c}if(y>1){g=g.split("][").join("]|[");if(g.indexOf("|")!==-1&&!/\(\?/.test(g)){g="(?:"+g+")"}}g=g.replace(/\[+=|=\]+/g,"\\b");return g}a.makeRe=function(e){try{return new RegExp(a(e))}catch(t){}};a.isMatch=function(e,t){try{return a.makeRe(t).test(e)}catch(r){return false}};a.match=function(e,t){var r=e.length,n=0;var i=e.slice();var o=a.makeRe(t);while(n<r){var s=e[n++];if(!o.test(s)){continue}i.splice(n,1)}return i}},{"is-posix-bracket":66}],24:[function(e,t,r){"use strict";var n=e("fill-range");t.exports=function i(e,t,r){if(typeof e!=="string"){throw new TypeError("expand-range expects a string.")}if(typeof t==="function"){r=t;t={}}if(typeof t==="boolean"){t={};t.makeRe=true}var i=t||{};var a=e.split("..");var o=a.length;if(o>3){return e}if(o===1){return a}if(typeof r==="boolean"&&r===true){i.makeRe=true}a.push(i);return n.apply(null,a.concat(r))}},{"fill-range":29}],25:[function(e,t,r){"use strict";var n=e("is-extendable");t.exports=function o(e){if(!n(e)){e={}}var t=arguments.length;for(var r=1;r<t;r++){var a=arguments[r];if(n(a)){i(e,a)}}return e};function i(e,t){for(var r in t){if(a(t,r)){e[r]=t[r]}}}function a(e,t){return Object.prototype.hasOwnProperty.call(e,t)}},{"is-extendable":62}],26:[function(e,t,r){"use strict";var n=Object.prototype.hasOwnProperty;var i=Object.prototype.toString;var a=function s(e){if(typeof Array.isArray==="function"){return Array.isArray(e)}return i.call(e)==="[object Array]"};var o=function f(e){if(!e||i.call(e)!=="[object Object]"){return false}var t=n.call(e,"constructor");var r=e.constructor&&e.constructor.prototype&&n.call(e.constructor.prototype,"isPrototypeOf");if(e.constructor&&!t&&!r){return false}var a;for(a in e){}return typeof a==="undefined"||n.call(e,a)};t.exports=function u(){var e,t,r,n,i,s,f=arguments[0],c=1,l=arguments.length,h=false;if(typeof f==="boolean"){h=f;f=arguments[1]||{};c=2}else if(typeof f!=="object"&&typeof f!=="function"||f==null){f={}}for(;c<l;++c){e=arguments[c];if(e!=null){for(t in e){r=f[t];n=e[t];if(f!==n){if(h&&n&&(o(n)||(i=a(n)))){if(i){i=false;s=r&&a(r)?r:[]}else{s=r&&o(r)?r:{}}f[t]=u(h,s,n)}else if(typeof n!=="undefined"){f[t]=n}}}}}return f}},{}],27:[function(e,t,r){"use strict";var n=e("is-extglob");var i,a={};t.exports=o;function o(e,t){t=t||{};var r={},n=0;e=e.replace(/!\(([^\w*()])/g,"$1!(");e=e.replace(/([*\/])\.!\([*]\)/g,function(e,t){if(t==="/"){return f("\\/[^.]+")}return f("[^.]+")});var o=e+String(!!t.regex)+String(!!t.contains)+String(!!t.escape);if(a.hasOwnProperty(o)){return a[o]}if(!(i instanceof RegExp)){i=u()}t.negate=false;var c;while(c=i.exec(e)){var h=c[1];var p=c[3];if(h==="!"){t.negate=true}var d="__EXTGLOB_"+n++ +"__";r[d]=s(p,h,t.escape);e=e.split(c[0]).join(d)}var v=Object.keys(r);var g=v.length;while(g--){var y=v[g];e=e.split(y).join(r[y])}var b=t.regex?l(e,t.contains,t.negate):e;b=b.split(".").join("\\.");return a[o]=b}function s(e,t,r){if(r)e=f(e);switch(t){case"!":return"(?!"+e+")[^/]"+(r?"%%%~":"*?");case"@":return"(?:"+e+")";case"+":return"(?:"+e+")+";case"*":return"(?:"+e+")"+(r?"%%":"*");case"?":return"(?:"+e+"|)";default:return e}}function f(e){e=e.split("*").join("[^/]%%%~");e=e.split(".").join("\\.");return e}function u(){return/(\\?[@?!+*$]\\?)(\(([^()]*?)\))/}function c(e){return"(?!^"+e+").*$"}function l(e,t,r){var n=t?"^":"";var i=t?"$":"";e="(?:"+e+")"+i;if(r){e=n+c(e)}return new RegExp(n+e)}},{"is-extglob":63}],28:[function(e,t,r){t.exports=function n(){return/([^\\\/]+)$/}},{}],29:[function(e,t,r){"use strict";var n=e("isobject");var i=e("is-number");var a=e("randomatic");var o=e("repeat-string");var s=e("repeat-element");t.exports=f;function f(e,t,r,o,f){if(e==null||t==null){throw new Error("fill-range expects the first and second args to be strings.")}if(typeof r==="function"){f=r;o={};r=null}if(typeof o==="function"){f=o;o={}}if(n(r)){o=r;r=""}var c,g=false,w="";var S=o||{};if(typeof S.silent==="undefined"){S.silent=true}r=r||S.step;var x=e,O=t;t=t.toString()==="-0"?0:t;if(S.optimize||S.makeRe){r=r?r+="~":r;c=true;g=true;w="~"}if(typeof r==="string"){var j=y().exec(r);if(j){var A=j.index;var k=j[0];if(k==="+"){return s(e,t)}else if(k==="?"){return[a(e,t)]}else if(k===">"){r=r.substr(0,A)+r.substr(A+1);c=true}else if(k==="|"){r=r.substr(0,A)+r.substr(A+1);c=true;g=true;w=k}else if(k==="~"){r=r.substr(0,A)+r.substr(A+1);c=true;g=true;w=k}}else if(!i(r)){if(!S.silent){throw new TypeError("fill-range: invalid step.")}return null}}if(/[.&*()[\]^%$#@!]/.test(e)||/[.&*()[\]^%$#@!]/.test(t)){if(!S.silent){throw new RangeError("fill-range: invalid range arguments.")}return null}if(!b(e)||!b(t)||m(e)||m(t)){if(!S.silent){throw new RangeError("fill-range: invalid range arguments.")}return null}var R=i(_(e));var T=i(_(t));if(!R&&T||R&&!T){if(!S.silent){throw new TypeError("fill-range: first range argument is incompatible with second.")}return null}var L=R;var I=p(r);if(L){e=+e;t=+t}else{e=e.charCodeAt(0);t=t.charCodeAt(0)}var C=e>t;if(e<0||t<0){c=false;g=false}var P=E(x,O);var M,N,B=[];var D=0;if(g){if(l(e,t,I,L,P,S)){if(w==="|"||w==="~"){w=h(e,t,I,L,C)}return u([x,O],w,S)}}while(C?e>=t:e<=t){if(P&&L){N=P(e)}if(typeof f==="function"){M=f(e,L,N,D++)}else if(!L){if(g&&v(e)){M=null}else{M=String.fromCharCode(e)}}else{M=d(e,N)}if(M!==null)B.push(M);if(C){e-=I}else{e+=I}}if((g||c)&&!S.noexpand){if(w==="|"||w==="~"){w=h(e,t,I,L,C)}if(B.length===1||e<0||t<0){return B}return u(B,w,S)}return B}function u(e,t,r){if(t==="~"){t="-"}var n=e.join(t);var i=r&&r.regexPrefix;if(t==="|"){n=i?i+n:n;n="("+n+")"}if(t==="-"){n=i&&i==="^"?i+n:n;n="["+n+"]"}return[n]}function c(e,t,r,n,i){if(i){return false}if(n){return e<=9&&t<=9}if(e<t){return r===1}return false}function l(e,t,r,n,i,a){if(n&&(e>9||t>9)){return false}return!i&&r===1&&e<t}function h(e,t,r,n,i){var a=c(e,t,r,n,i);if(!a){return"|"}return"~"}function p(e){return Math.abs(e>>0)||1}function d(e,t){var r=t?t+e:e;if(t&&e.toString().charAt(0)==="-"){r="-"+t+e.toString().substr(1)}return r.toString()}function v(e){var t=g(e);return t==="\\"||t==="["||t==="]"||t==="^"||t==="("||t===")"||t==="`"}function g(e){return String.fromCharCode(e)}function y(){return/\?|>|\||\+|\~/g}function b(e){return/[a-z0-9]/i.test(e)}function m(e){return/[a-z][0-9]|[0-9][a-z]/i.test(e)}function _(e){if(/^-*0+$/.test(e.toString())){return"0"}return e}function w(e){return/[^.]\.|^-*0+[0-9]/.test(e)}function E(e,t){if(w(e)||w(t)){var r=S(e);var n=S(t);var i=r>=n?r:n;return function(e){return o("0",i-S(e))}}return false}function S(e){return e.toString().length}},{"is-number":65,isobject:71,randomatic:117,"repeat-element":129,"repeat-string":130}],30:[function(e,t,r){"use strict";t.exports=function n(e,t,r){for(var n in e){if(t.call(r,e[n],n,e)===false){break}}}},{}],31:[function(e,t,r){"use strict";var n=e("for-in");var i=Object.prototype.hasOwnProperty;t.exports=function a(e,t,r){n(e,function(n,a){if(i.call(e,a)){return t.call(r,e[a],a,e)}})}},{"for-in":30}],32:[function(e,t,r){var n=Object.prototype.hasOwnProperty;var i=Object.prototype.toString;t.exports=function a(e,t,r){if(i.call(t)!=="[object Function]"){throw new TypeError("iterator must be a function")}var a=e.length;if(a===+a){for(var o=0;o<a;o++){t.call(r,e[o],o,e)}}else{for(var s in e){if(n.call(e,s)){t.call(r,e[s],s,e)}}}}},{}],33:[function(e,t,r){(function(r){"use strict";var n=e("util");var i=e("events").EventEmitter;var a=e("fs");var o=e("path");var s=e("globule");var f=e("./helper");var u=e("timers").setImmediate;if(typeof u!=="function"){u=r.nextTick}var c=10;function l(e,t,n){var a=this;i.call(a);if(typeof t==="function"){n=t;t={}}t=t||{};t.mark=true;t.interval=t.interval||100;t.debounceDelay=t.debounceDelay||500;t.cwd=t.cwd||r.cwd();this.options=t;n=n||function(){};this._watched=Object.create(null);this._watchers=Object.create(null);this._pollers=Object.create(null);this._patterns=[];this._cached=Object.create(null);if(this.options.maxListeners){this.setMaxListeners(this.options.maxListeners);l.super_.prototype.setMaxListeners(this.options.maxListeners);delete this.options.maxListeners}if(e){this.add(e,n)}this._keepalive=setInterval(function(){},200);return this}n.inherits(l,i);t.exports=function h(e,t,r){return new l(e,t,r)};t.exports.Gaze=l;l.prototype.emit=function(){var e=this;var t=arguments;var r=t[0];var n=t[1];var i;if(r.slice(-2)!=="ed"){l.super_.prototype.emit.apply(e,t);return this}if(r==="added"){Object.keys(this._cached).forEach(function(n){if(e._cached[n].indexOf("deleted")!==-1){t[0]=r="renamed";[].push.call(t,n);delete e._cached[n];return false}})}var u=this._cached[n]||[];if(u.indexOf(r)===-1){f.objectPush(e._cached,n,r);clearTimeout(i);i=setTimeout(function(){delete e._cached[n]},this.options.debounceDelay);l.super_.prototype.emit.apply(e,t);l.super_.prototype.emit.apply(e,["all",r].concat([].slice.call(t,1)))}if(r==="added"){if(f.isDir(n)){a.readdirSync(n).map(function(e){return o.join(n,e)}).filter(function(t){return s.isMatch(e._patterns,t,e.options)}).forEach(function(t){e.emit("added",t)})}}return this};l.prototype.close=function(e){var t=this;e=e===false?false:true;Object.keys(t._watchers).forEach(function(e){t._watchers[e].close()});t._watchers=Object.create(null);Object.keys(this._watched).forEach(function(e){t._unpollDir(e)});if(e){t._watched=Object.create(null);setTimeout(function(){t.emit("end");t.removeAllListeners();clearInterval(t._keepalive)},c+100)}return t};l.prototype.add=function(e,t){if(typeof e==="string"){e=[e]}this._patterns=f.unique.apply(null,[this._patterns,e]);e=s.find(this._patterns,this.options);this._addToWatched(e);this.close(false);this._initWatched(t)};l.prototype._internalAdd=function(e,t){var r=[];if(f.isDir(e)){r=[f.markDir(e)].concat(s.find(this._patterns,this.options))}else{if(s.isMatch(this._patterns,e,this.options)){r=[e]}}if(r.length>0){this._addToWatched(r);this.close(false);this._initWatched(t)}};l.prototype.remove=function(e){var t=this;if(this._watched[e]){this._unpollDir(e);delete this._watched[e]}else{Object.keys(this._watched).forEach(function(r){var n=t._watched[r].indexOf(e);if(n!==-1){t._unpollFile(e);t._watched[r].splice(n,1);return false}})}if(this._watchers[e]){this._watchers[e].close()}return this};l.prototype.watched=function(){return this._watched};l.prototype.relative=function(e,t){var n=this;var i=Object.create(null);var a,s,u;var c=this.options.cwd||r.cwd();if(e===""){e="."}e=f.markDir(e);t=t||false;Object.keys(this._watched).forEach(function(e){a=o.relative(c,e)+o.sep;if(a===o.sep){a="."}u=t?f.unixifyPathSep(a):a;i[u]=n._watched[e].map(function(e){s=o.relative(o.join(c,a)||"",e||"");if(f.isDir(e)){s=f.markDir(s)}if(t){s=f.unixifyPathSep(s)}return s})});if(e&&t){e=f.unixifyPathSep(e)}return e?i[e]||[]:i};l.prototype._addToWatched=function(e){for(var t=0;t<e.length;t++){var r=e[t];var n=o.resolve(this.options.cwd,r);var i=f.isDir(r)?n:o.dirname(n);i=f.markDir(i);if(f.isDir(r)&&!(n in this._watched)){f.objectPush(this._watched,n,[])}if(r.slice(-1)==="/"){n+=o.sep}f.objectPush(this._watched,o.dirname(n)+o.sep,n);var s=a.readdirSync(i);for(var u=0;u<s.length;u++){var c=o.join(i,s[u]);if(a.lstatSync(c).isDirectory()){f.objectPush(this._watched,i,c+o.sep)}}}return this};l.prototype._watchDir=function(e,t){var r=this;var n;try{this._watchers[e]=a.watch(e,function(i){clearTimeout(n);n=setTimeout(function(){if(e in r._watchers&&a.existsSync(e)){t(null,e)}},c+100)})}catch(i){return this._handleError(i)}return this};l.prototype._unpollFile=function(e){if(this._pollers[e]){a.unwatchFile(e,this._pollers[e]);delete this._pollers[e]}return this};l.prototype._unpollDir=function(e){this._unpollFile(e);for(var t=0;t<this._watched[e].length;t++){this._unpollFile(this._watched[e][t])}};l.prototype._pollFile=function(e,t){var r={persistent:true,interval:this.options.interval};if(!this._pollers[e]){this._pollers[e]=function(r,n){t(null,e)};try{a.watchFile(e,r,this._pollers[e])}catch(n){return this._handleError(n)}}return this};l.prototype._initWatched=function(e){var t=this;var n=this.options.cwd||r.cwd();var i=Object.keys(t._watched);if(i.length<1){u(function(){t.emit("ready",t);if(e){e.call(t,null,t)}t.emit("nomatch")});return}f.forEachSeries(i,function(e,r){e=e||"";var i=t._watched[e];t._watchDir(e,function(r,i){var s=n===e?".":o.relative(n,e);s=s||"";a.readdir(i,function(r,n){if(r){return t.emit("error",r)}if(!n){return}try{n=n.map(function(t){if(a.existsSync(o.join(e,t))&&a.lstatSync(o.join(e,t)).isDirectory()){return t+o.sep}else{return t}})}catch(r){}var i=t.relative(s);i.filter(function(e){return n.indexOf(e)<0}).forEach(function(r){if(!f.isDir(r)){var n=o.join(e,r);t.remove(n);t.emit("deleted",n)}});n.filter(function(e){return i.indexOf(e)<0}).forEach(function(r){var n=o.join(s,r);t._internalAdd(n,function(){t.emit("added",o.join(e,r))})})})});i.forEach(function(e){if(f.isDir(e)){return}t._pollFile(e,function(e,r){if(a.existsSync(r)){t.emit("changed",r)}})});r()},function(){setTimeout(function(){t.emit("ready",t);if(e){e.call(t,null,t)}},c+100)})};l.prototype._handleError=function(e){if(e.code==="EMFILE"){return this.emit("error",new Error("EMFILE: Too many opened files."))}return this.emit("error",e)}}).call(this,e("_process"));
},{"./helper":34,_process:112,events:22,fs:11,globule:49,path:108,timers:142,util:148}],34:[function(e,t,r){(function(r){"use strict";var n=e("path");var i=t.exports={};i.isDir=function a(e){if(typeof e!=="string"){return false}return e.slice(-n.sep.length)===n.sep};i.objectPush=function o(e,t,r){if(e[t]==null){e[t]=[]}if(Array.isArray(r)){e[t]=e[t].concat(r)}else if(r){e[t].push(r)}return e[t]=i.unique(e[t])};i.markDir=function s(e){if(typeof e==="string"&&e.slice(-n.sep.length)!==n.sep&&e!=="."){e+=n.sep}return e};i.unixifyPathSep=function f(e){return r.platform==="win32"?String(e).replace(/\\/g,"/"):e};i.unique=function u(){var e=Array.prototype.concat.apply(Array.prototype,arguments);var t=[];for(var r=0;r<e.length;r++){if(t.indexOf(e[r])===-1){t.push(e[r])}}return t};i.forEachSeries=function c(e,t,r){if(!e.length){return r()}var n=0;var i=function(){t(e[n],function(t){if(t){r(t);r=function(){}}else{n+=1;if(n===e.length){r(null)}else{i()}}})};i()}}).call(this,e("_process"))},{_process:112,path:108}],35:[function(e,t,r){"use strict";var n=e("path");var i=e("glob-parent");var a=e("is-glob");t.exports=function s(e){if(typeof e!=="string"){throw new TypeError("glob-base expects a string.")}var t={};t.base=i(e);t.isGlob=a(e);if(t.base!=="."){t.glob=e.substr(t.base.length);if(t.glob.charAt(0)==="/"){t.glob=t.glob.substr(1)}}else{t.glob=e}if(!t.isGlob){t.base=o(e);t.glob=t.base!=="."?e.substr(t.base.length):e}if(t.glob.substr(0,2)==="./"){t.glob=t.glob.substr(2)}if(t.glob.charAt(0)==="/"){t.glob=t.glob.substr(1)}return t};function o(e){if(e.slice(-1)==="/")return e;return n.dirname(e)}},{"glob-parent":36,"is-glob":64,path:108}],36:[function(e,t,r){"use strict";var n=e("path");var i=e("is-glob");t.exports=function a(e){e+="a";do{e=n.dirname(e)}while(i(e));return e}},{"is-glob":64,path:108}],37:[function(e,t,r){(function(r){"use strict";var n=e("through2");var i=e("ordered-read-streams");var a=e("unique-stream");var o=e("glob");var s=e("micromatch");var f=e("to-absolute-glob");var u=e("glob-parent");var c=e("path");var l=e("extend");var h={createStream:function(e,t,r){e=f(e,r);var i=l({},r);delete i.root;var a=new o.Glob(e,i);var s=r.base||u(e)+c.sep;var h=n.obj(r,t.length?v:undefined);var d=false;a.on("error",h.emit.bind(h,"error"));a.once("end",function(){if(r.allowEmpty!==true&&!d&&y(a)){h.emit("error",new Error("File not found with singular glob: "+e))}h.end()});a.on("match",function(e){d=true;h.write({cwd:r.cwd,base:s,path:c.normalize(e)})});return h;function v(e,r,n){var i=p.bind(null,e);if(t.every(i)){n(null,e)}else{n()}}},create:function(e,t){if(!t){t={}}if(typeof t.cwd!=="string"){t.cwd=r.cwd()}if(typeof t.dot!=="boolean"){t.dot=false}if(typeof t.silent!=="boolean"){t.silent=true}if(typeof t.nonull!=="boolean"){t.nonull=false}if(typeof t.cwdbase!=="boolean"){t.cwdbase=false}if(t.cwdbase){t.base=t.cwd}if(!Array.isArray(e)){e=[e]}var n=[];var o=[];var u=l({},t);delete u.root;e.forEach(function(e,r){if(typeof e!=="string"&&!(e instanceof RegExp)){throw new Error("Invalid glob at index "+r)}var i=d(e)?o:n;if(i===o&&typeof e==="string"){var a=f(e,t);e=s.matcher(a,u)}i.push({index:r,glob:e})});if(n.length===0){throw new Error("Missing positive glob")}if(n.length===1){return m(n[0])}var c=n.map(m);var p=new i(c);var y=a("path");var b=p.pipe(y);p.on("error",function(e){b.emit("error",e)});return b;function m(e){var r=o.filter(v(e.index)).map(g);return h.createStream(e.glob,r,t)}}};function p(e,t){if(typeof t==="function"){return t(e.path)}if(t instanceof RegExp){return t.test(e.path)}}function d(e){if(typeof e==="string"){return e[0]==="!"}if(e instanceof RegExp){return true}}function v(e){return function(t){return t.index>e}}function g(e){return e.glob}function y(e){var t=e.minimatch.set;if(t.length!==1){return false}return t[0].every(function r(e){return typeof e==="string"})}t.exports=h}).call(this,e("_process"))},{_process:112,extend:26,glob:47,"glob-parent":36,micromatch:87,"ordered-read-streams":105,path:108,through2:44,"to-absolute-glob":143,"unique-stream":144}],38:[function(e,t,r){t.exports=Array.isArray||function(e){return Object.prototype.toString.call(e)=="[object Array]"}},{}],39:[function(e,t,r){(function(r){t.exports=s;var n=Object.keys||function(e){var t=[];for(var r in e)t.push(r);return t};var i=e("core-util-is");i.inherits=e("inherits");var a=e("./_stream_readable");var o=e("./_stream_writable");i.inherits(s,a);u(n(o.prototype),function(e){if(!s.prototype[e])s.prototype[e]=o.prototype[e]});function s(e){if(!(this instanceof s))return new s(e);a.call(this,e);o.call(this,e);if(e&&e.readable===false)this.readable=false;if(e&&e.writable===false)this.writable=false;this.allowHalfOpen=true;if(e&&e.allowHalfOpen===false)this.allowHalfOpen=false;this.once("end",f)}function f(){if(this.allowHalfOpen||this._writableState.ended)return;r.nextTick(this.end.bind(this))}function u(e,t){for(var r=0,n=e.length;r<n;r++){t(e[r],r)}}}).call(this,e("_process"))},{"./_stream_readable":40,"./_stream_writable":42,_process:112,"core-util-is":19,inherits:57}],40:[function(e,t,r){(function(r){t.exports=c;var n=e("isarray");var i=e("buffer").Buffer;c.ReadableState=u;var a=e("events").EventEmitter;if(!a.listenerCount)a.listenerCount=function(e,t){return e.listeners(t).length};var o=e("stream");var s=e("core-util-is");s.inherits=e("inherits");var f;s.inherits(c,o);function u(t,r){t=t||{};var n=t.highWaterMark;this.highWaterMark=n||n===0?n:16*1024;this.highWaterMark=~~this.highWaterMark;this.buffer=[];this.length=0;this.pipes=null;this.pipesCount=0;this.flowing=false;this.ended=false;this.endEmitted=false;this.reading=false;this.calledRead=false;this.sync=true;this.needReadable=false;this.emittedReadable=false;this.readableListening=false;this.objectMode=!!t.objectMode;this.defaultEncoding=t.defaultEncoding||"utf8";this.ranOut=false;this.awaitDrain=0;this.readingMore=false;this.decoder=null;this.encoding=null;if(t.encoding){if(!f)f=e("string_decoder/").StringDecoder;this.decoder=new f(t.encoding);this.encoding=t.encoding}}function c(e){if(!(this instanceof c))return new c(e);this._readableState=new u(e,this);this.readable=true;o.call(this)}c.prototype.push=function(e,t){var r=this._readableState;if(typeof e==="string"&&!r.objectMode){t=t||r.defaultEncoding;if(t!==r.encoding){e=new i(e,t);t=""}}return l(this,r,e,t,false)};c.prototype.unshift=function(e){var t=this._readableState;return l(this,t,e,"",true)};function l(e,t,r,n,i){var a=g(t,r);if(a){e.emit("error",a)}else if(r===null||r===undefined){t.reading=false;if(!t.ended)y(e,t)}else if(t.objectMode||r&&r.length>0){if(t.ended&&!i){var o=new Error("stream.push() after EOF");e.emit("error",o)}else if(t.endEmitted&&i){var o=new Error("stream.unshift() after end event");e.emit("error",o)}else{if(t.decoder&&!i&&!n)r=t.decoder.write(r);t.length+=t.objectMode?1:r.length;if(i){t.buffer.unshift(r)}else{t.reading=false;t.buffer.push(r)}if(t.needReadable)b(e);_(e,t)}}else if(!i){t.reading=false}return h(t)}function h(e){return!e.ended&&(e.needReadable||e.length<e.highWaterMark||e.length===0)}c.prototype.setEncoding=function(t){if(!f)f=e("string_decoder/").StringDecoder;this._readableState.decoder=new f(t);this._readableState.encoding=t};var p=8388608;function d(e){if(e>=p){e=p}else{e--;for(var t=1;t<32;t<<=1)e|=e>>t;e++}return e}function v(e,t){if(t.length===0&&t.ended)return 0;if(t.objectMode)return e===0?0:1;if(e===null||isNaN(e)){if(t.flowing&&t.buffer.length)return t.buffer[0].length;else return t.length}if(e<=0)return 0;if(e>t.highWaterMark)t.highWaterMark=d(e);if(e>t.length){if(!t.ended){t.needReadable=true;return 0}else return t.length}return e}c.prototype.read=function(e){var t=this._readableState;t.calledRead=true;var r=e;var n;if(typeof e!=="number"||e>0)t.emittedReadable=false;if(e===0&&t.needReadable&&(t.length>=t.highWaterMark||t.ended)){b(this);return null}e=v(e,t);if(e===0&&t.ended){n=null;if(t.length>0&&t.decoder){n=j(e,t);t.length-=n.length}if(t.length===0)A(this);return n}var i=t.needReadable;if(t.length-e<=t.highWaterMark)i=true;if(t.ended||t.reading)i=false;if(i){t.reading=true;t.sync=true;if(t.length===0)t.needReadable=true;this._read(t.highWaterMark);t.sync=false}if(i&&!t.reading)e=v(r,t);if(e>0)n=j(e,t);else n=null;if(n===null){t.needReadable=true;e=0}t.length-=e;if(t.length===0&&!t.ended)t.needReadable=true;if(t.ended&&!t.endEmitted&&t.length===0)A(this);return n};function g(e,t){var r=null;if(!i.isBuffer(t)&&"string"!==typeof t&&t!==null&&t!==undefined&&!e.objectMode){r=new TypeError("Invalid non-string/buffer chunk")}return r}function y(e,t){if(t.decoder&&!t.ended){var r=t.decoder.end();if(r&&r.length){t.buffer.push(r);t.length+=t.objectMode?1:r.length}}t.ended=true;if(t.length>0)b(e);else A(e)}function b(e){var t=e._readableState;t.needReadable=false;if(t.emittedReadable)return;t.emittedReadable=true;if(t.sync)r.nextTick(function(){m(e)});else m(e)}function m(e){e.emit("readable")}function _(e,t){if(!t.readingMore){t.readingMore=true;r.nextTick(function(){w(e,t)})}}function w(e,t){var r=t.length;while(!t.reading&&!t.flowing&&!t.ended&&t.length<t.highWaterMark){e.read(0);if(r===t.length)break;else r=t.length}t.readingMore=false}c.prototype._read=function(e){this.emit("error",new Error("not implemented"))};c.prototype.pipe=function(e,t){var i=this;var o=this._readableState;switch(o.pipesCount){case 0:o.pipes=e;break;case 1:o.pipes=[o.pipes,e];break;default:o.pipes.push(e);break}o.pipesCount+=1;var s=(!t||t.end!==false)&&e!==r.stdout&&e!==r.stderr;var f=s?c:h;if(o.endEmitted)r.nextTick(f);else i.once("end",f);e.on("unpipe",u);function u(e){if(e!==i)return;h()}function c(){e.end()}var l=E(i);e.on("drain",l);function h(){e.removeListener("close",d);e.removeListener("finish",v);e.removeListener("drain",l);e.removeListener("error",p);e.removeListener("unpipe",u);i.removeListener("end",c);i.removeListener("end",h);if(!e._writableState||e._writableState.needDrain)l()}function p(t){g();e.removeListener("error",p);if(a.listenerCount(e,"error")===0)e.emit("error",t)}if(!e._events||!e._events.error)e.on("error",p);else if(n(e._events.error))e._events.error.unshift(p);else e._events.error=[p,e._events.error];function d(){e.removeListener("finish",v);g()}e.once("close",d);function v(){e.removeListener("close",d);g()}e.once("finish",v);function g(){i.unpipe(e)}e.emit("pipe",i);if(!o.flowing){this.on("readable",x);o.flowing=true;r.nextTick(function(){S(i)})}return e};function E(e){return function(){var t=this;var r=e._readableState;r.awaitDrain--;if(r.awaitDrain===0)S(e)}}function S(e){var t=e._readableState;var r;t.awaitDrain=0;function n(e,n,i){var a=e.write(r);if(false===a){t.awaitDrain++}}while(t.pipesCount&&null!==(r=e.read())){if(t.pipesCount===1)n(t.pipes,0,null);else k(t.pipes,n);e.emit("data",r);if(t.awaitDrain>0)return}if(t.pipesCount===0){t.flowing=false;if(a.listenerCount(e,"data")>0)O(e);return}t.ranOut=true}function x(){if(this._readableState.ranOut){this._readableState.ranOut=false;S(this)}}c.prototype.unpipe=function(e){var t=this._readableState;if(t.pipesCount===0)return this;if(t.pipesCount===1){if(e&&e!==t.pipes)return this;if(!e)e=t.pipes;t.pipes=null;t.pipesCount=0;this.removeListener("readable",x);t.flowing=false;if(e)e.emit("unpipe",this);return this}if(!e){var r=t.pipes;var n=t.pipesCount;t.pipes=null;t.pipesCount=0;this.removeListener("readable",x);t.flowing=false;for(var i=0;i<n;i++)r[i].emit("unpipe",this);return this}var i=R(t.pipes,e);if(i===-1)return this;t.pipes.splice(i,1);t.pipesCount-=1;if(t.pipesCount===1)t.pipes=t.pipes[0];e.emit("unpipe",this);return this};c.prototype.on=function(e,t){var r=o.prototype.on.call(this,e,t);if(e==="data"&&!this._readableState.flowing)O(this);if(e==="readable"&&this.readable){var n=this._readableState;if(!n.readableListening){n.readableListening=true;n.emittedReadable=false;n.needReadable=true;if(!n.reading){this.read(0)}else if(n.length){b(this,n)}}}return r};c.prototype.addListener=c.prototype.on;c.prototype.resume=function(){O(this);this.read(0);this.emit("resume")};c.prototype.pause=function(){O(this,true);this.emit("pause")};function O(e,t){var n=e._readableState;if(n.flowing){throw new Error("Cannot switch to old mode now.")}var i=t||false;var a=false;e.readable=true;e.pipe=o.prototype.pipe;e.on=e.addListener=o.prototype.on;e.on("readable",function(){a=true;var t;while(!i&&null!==(t=e.read()))e.emit("data",t);if(t===null){a=false;e._readableState.needReadable=true}});e.pause=function(){i=true;this.emit("pause")};e.resume=function(){i=false;if(a)r.nextTick(function(){e.emit("readable")});else this.read(0);this.emit("resume")};e.emit("readable")}c.prototype.wrap=function(e){var t=this._readableState;var r=false;var n=this;e.on("end",function(){if(t.decoder&&!t.ended){var e=t.decoder.end();if(e&&e.length)n.push(e)}n.push(null)});e.on("data",function(i){if(t.decoder)i=t.decoder.write(i);if(t.objectMode&&(i===null||i===undefined))return;else if(!t.objectMode&&(!i||!i.length))return;var a=n.push(i);if(!a){r=true;e.pause()}});for(var i in e){if(typeof e[i]==="function"&&typeof this[i]==="undefined"){this[i]=function(t){return function(){return e[t].apply(e,arguments)}}(i)}}var a=["error","close","destroy","pause","resume"];k(a,function(t){e.on(t,n.emit.bind(n,t))});n._read=function(t){if(r){r=false;e.resume()}};return n};c._fromList=j;function j(e,t){var r=t.buffer;var n=t.length;var a=!!t.decoder;var o=!!t.objectMode;var s;if(r.length===0)return null;if(n===0)s=null;else if(o)s=r.shift();else if(!e||e>=n){if(a)s=r.join("");else s=i.concat(r,n);r.length=0}else{if(e<r[0].length){var f=r[0];s=f.slice(0,e);r[0]=f.slice(e)}else if(e===r[0].length){s=r.shift()}else{if(a)s="";else s=new i(e);var u=0;for(var c=0,l=r.length;c<l&&u<e;c++){var f=r[0];var h=Math.min(e-u,f.length);if(a)s+=f.slice(0,h);else f.copy(s,u,0,h);if(h<f.length)r[0]=f.slice(h);else r.shift();u+=h}}}return s}function A(e){var t=e._readableState;if(t.length>0)throw new Error("endReadable called on non-empty stream");if(!t.endEmitted&&t.calledRead){t.ended=true;r.nextTick(function(){if(!t.endEmitted&&t.length===0){t.endEmitted=true;e.readable=false;e.emit("end")}})}}function k(e,t){for(var r=0,n=e.length;r<n;r++){t(e[r],r)}}function R(e,t){for(var r=0,n=e.length;r<n;r++){if(e[r]===t)return r}return-1}}).call(this,e("_process"))},{_process:112,buffer:13,"core-util-is":19,events:22,inherits:57,isarray:38,stream:134,"string_decoder/":139}],41:[function(e,t,r){t.exports=s;var n=e("./_stream_duplex");var i=e("core-util-is");i.inherits=e("inherits");i.inherits(s,n);function a(e,t){this.afterTransform=function(e,r){return o(t,e,r)};this.needTransform=false;this.transforming=false;this.writecb=null;this.writechunk=null}function o(e,t,r){var n=e._transformState;n.transforming=false;var i=n.writecb;if(!i)return e.emit("error",new Error("no writecb in Transform class"));n.writechunk=null;n.writecb=null;if(r!==null&&r!==undefined)e.push(r);if(i)i(t);var a=e._readableState;a.reading=false;if(a.needReadable||a.length<a.highWaterMark){e._read(a.highWaterMark)}}function s(e){if(!(this instanceof s))return new s(e);n.call(this,e);var t=this._transformState=new a(e,this);var r=this;this._readableState.needReadable=true;this._readableState.sync=false;this.once("finish",function(){if("function"===typeof this._flush)this._flush(function(e){f(r,e)});else f(r)})}s.prototype.push=function(e,t){this._transformState.needTransform=false;return n.prototype.push.call(this,e,t)};s.prototype._transform=function(e,t,r){throw new Error("not implemented")};s.prototype._write=function(e,t,r){var n=this._transformState;n.writecb=r;n.writechunk=e;n.writeencoding=t;if(!n.transforming){var i=this._readableState;if(n.needTransform||i.needReadable||i.length<i.highWaterMark)this._read(i.highWaterMark)}};s.prototype._read=function(e){var t=this._transformState;if(t.writechunk!==null&&t.writecb&&!t.transforming){t.transforming=true;this._transform(t.writechunk,t.writeencoding,t.afterTransform)}else{t.needTransform=true}};function f(e,t){if(t)return e.emit("error",t);var r=e._writableState;var n=e._readableState;var i=e._transformState;if(r.length)throw new Error("calling transform done when ws.length != 0");if(i.transforming)throw new Error("calling transform done when still transforming");return e.push(null)}},{"./_stream_duplex":39,"core-util-is":19,inherits:57}],42:[function(e,t,r){(function(r){t.exports=f;var n=e("buffer").Buffer;f.WritableState=s;var i=e("core-util-is");i.inherits=e("inherits");var a=e("stream");i.inherits(f,a);function o(e,t,r){this.chunk=e;this.encoding=t;this.callback=r}function s(e,t){e=e||{};var r=e.highWaterMark;this.highWaterMark=r||r===0?r:16*1024;this.objectMode=!!e.objectMode;this.highWaterMark=~~this.highWaterMark;this.needDrain=false;this.ending=false;this.ended=false;this.finished=false;var n=e.decodeStrings===false;this.decodeStrings=!n;this.defaultEncoding=e.defaultEncoding||"utf8";this.length=0;this.writing=false;this.sync=true;this.bufferProcessing=false;this.onwrite=function(e){g(t,e)};this.writecb=null;this.writelen=0;this.buffer=[];this.errorEmitted=false}function f(t){var r=e("./_stream_duplex");if(!(this instanceof f)&&!(this instanceof r))return new f(t);this._writableState=new s(t,this);this.writable=true;a.call(this)}f.prototype.pipe=function(){this.emit("error",new Error("Cannot pipe. Not readable."))};function u(e,t,n){var i=new Error("write after end");e.emit("error",i);r.nextTick(function(){n(i)})}function c(e,t,i,a){var o=true;if(!n.isBuffer(i)&&"string"!==typeof i&&i!==null&&i!==undefined&&!t.objectMode){var s=new TypeError("Invalid non-string/buffer chunk");e.emit("error",s);r.nextTick(function(){a(s)});o=false}return o}f.prototype.write=function(e,t,r){var i=this._writableState;var a=false;if(typeof t==="function"){r=t;t=null}if(n.isBuffer(e))t="buffer";else if(!t)t=i.defaultEncoding;if(typeof r!=="function")r=function(){};if(i.ended)u(this,i,r);else if(c(this,i,e,r))a=h(this,i,e,t,r);return a};function l(e,t,r){if(!e.objectMode&&e.decodeStrings!==false&&typeof t==="string"){t=new n(t,r)}return t}function h(e,t,r,i,a){r=l(t,r,i);if(n.isBuffer(r))i="buffer";var s=t.objectMode?1:r.length;t.length+=s;var f=t.length<t.highWaterMark;if(!f)t.needDrain=true;if(t.writing)t.buffer.push(new o(r,i,a));else p(e,t,s,r,i,a);return f}function p(e,t,r,n,i,a){t.writelen=r;t.writecb=a;t.writing=true;t.sync=true;e._write(n,i,t.onwrite);t.sync=false}function d(e,t,n,i,a){if(n)r.nextTick(function(){a(i)});else a(i);e._writableState.errorEmitted=true;e.emit("error",i)}function v(e){e.writing=false;e.writecb=null;e.length-=e.writelen;e.writelen=0}function g(e,t){var n=e._writableState;var i=n.sync;var a=n.writecb;v(n);if(t)d(e,n,i,t,a);else{var o=_(e,n);if(!o&&!n.bufferProcessing&&n.buffer.length)m(e,n);if(i){r.nextTick(function(){y(e,n,o,a)})}else{y(e,n,o,a)}}}function y(e,t,r,n){if(!r)b(e,t);n();if(r)w(e,t)}function b(e,t){if(t.length===0&&t.needDrain){t.needDrain=false;e.emit("drain")}}function m(e,t){t.bufferProcessing=true;for(var r=0;r<t.buffer.length;r++){var n=t.buffer[r];var i=n.chunk;var a=n.encoding;var o=n.callback;var s=t.objectMode?1:i.length;p(e,t,s,i,a,o);if(t.writing){r++;break}}t.bufferProcessing=false;if(r<t.buffer.length)t.buffer=t.buffer.slice(r);else t.buffer.length=0}f.prototype._write=function(e,t,r){r(new Error("not implemented"))};f.prototype.end=function(e,t,r){var n=this._writableState;if(typeof e==="function"){r=e;e=null;t=null}else if(typeof t==="function"){r=t;t=null}if(typeof e!=="undefined"&&e!==null)this.write(e,t);if(!n.ending&&!n.finished)E(this,n,r)};function _(e,t){return t.ending&&t.length===0&&!t.finished&&!t.writing}function w(e,t){var r=_(e,t);if(r){t.finished=true;e.emit("finish")}return r}function E(e,t,n){t.ending=true;w(e,t);if(n){if(t.finished)r.nextTick(n);else e.once("finish",n)}t.ended=true}}).call(this,e("_process"))},{"./_stream_duplex":39,_process:112,buffer:13,"core-util-is":19,inherits:57,stream:134}],43:[function(e,t,r){t.exports=e("./lib/_stream_transform.js")},{"./lib/_stream_transform.js":41}],44:[function(e,t,r){(function(r){var n=e("readable-stream/transform"),i=e("util").inherits,a=e("xtend");function o(e){n.call(this,e);this._destroyed=false}i(o,n);o.prototype.destroy=function(e){if(this._destroyed)return;this._destroyed=true;var t=this;r.nextTick(function(){if(e)t.emit("error",e);t.emit("close")})};function s(e,t,r){r(null,e)}function f(e){return function(t,r,n){if(typeof t=="function"){n=r;r=t;t={}}if(typeof r!="function")r=s;if(typeof n!="function")n=null;return e(t,r,n)}}t.exports=f(function(e,t,r){var n=new o(e);n._transform=t;if(r)n._flush=r;return n});t.exports.ctor=f(function(e,t,r){function n(t){if(!(this instanceof n))return new n(t);this.options=a(e,t);o.call(this,this.options)}i(n,o);n.prototype._transform=t;if(r)n.prototype._flush=r;return n});t.exports.obj=f(function(e,t,r){var n=new o(a({objectMode:true,highWaterMark:16},e));n._transform=t;if(r)n._flush=r;return n})}).call(this,e("_process"))},{_process:112,"readable-stream/transform":43,util:148,xtend:188}],45:[function(e,t,r){var n=e("gaze");var i=e("events").EventEmitter;function a(e,t){return function(r,n){if(r)e.emit("error",r);n.on("all",function(r,n,i){var a={type:r,path:n};if(i)a.old=i;e.emit("change",a);if(t)t()})}}t.exports=function(e,t,r){var o=new i;if(typeof t==="function"){r=t;t={}}var s=n(e,t,a(o,r));s.on("end",o.emit.bind(o,"end"));s.on("error",o.emit.bind(o,"error"));s.on("ready",o.emit.bind(o,"ready"));s.on("nomatch",o.emit.bind(o,"nomatch"));o.end=function(){return s.close()};o.add=function(e,t){return s.add(e,a(o,t))};o.remove=function(e){return s.remove(e)};o._watcher=s;return o}},{events:22,gaze:33}],46:[function(e,t,r){(function(t){r.alphasort=u;r.alphasorti=f;r.setopts=h;r.ownProp=n;r.makeAbs=g;r.finish=d;r.mark=v;r.isIgnored=y;r.childrenIgnored=b;function n(e,t){return Object.prototype.hasOwnProperty.call(e,t)}var i=e("path");var a=e("minimatch");var o=e("path-is-absolute");var s=a.Minimatch;function f(e,t){return e.toLowerCase().localeCompare(t.toLowerCase())}function u(e,t){return e.localeCompare(t)}function c(e,t){e.ignore=t.ignore||[];if(!Array.isArray(e.ignore))e.ignore=[e.ignore];if(e.ignore.length){e.ignore=e.ignore.map(l)}}function l(e){var t=null;if(e.slice(-3)==="/**"){var r=e.replace(/(\/\*\*)+$/,"");t=new s(r)}return{matcher:new s(e),gmatcher:t}}function h(e,r,a){if(!a)a={};if(a.matchBase&&-1===r.indexOf("/")){if(a.noglobstar){throw new Error("base matching requires globstar")}r="**/"+r}e.silent=!!a.silent;e.pattern=r;e.strict=a.strict!==false;e.realpath=!!a.realpath;e.realpathCache=a.realpathCache||Object.create(null);e.follow=!!a.follow;e.dot=!!a.dot;e.mark=!!a.mark;e.nodir=!!a.nodir;if(e.nodir)e.mark=true;e.sync=!!a.sync;e.nounique=!!a.nounique;e.nonull=!!a.nonull;e.nosort=!!a.nosort;e.nocase=!!a.nocase;e.stat=!!a.stat;e.noprocess=!!a.noprocess;e.maxLength=a.maxLength||Infinity;e.cache=a.cache||Object.create(null);e.statCache=a.statCache||Object.create(null);e.symlinks=a.symlinks||Object.create(null);c(e,a);e.changedCwd=false;var o=t.cwd();if(!n(a,"cwd"))e.cwd=o;else{e.cwd=a.cwd;e.changedCwd=i.resolve(a.cwd)!==o}e.root=a.root||i.resolve(e.cwd,"/");e.root=i.resolve(e.root);if(t.platform==="win32")e.root=e.root.replace(/\\/g,"/");e.nomount=!!a.nomount;a.nonegate=a.nonegate===false?false:true;a.nocomment=a.nocomment===false?false:true;p(a);e.minimatch=new s(r,a);e.options=e.minimatch.options}r.deprecationWarned;function p(e){if(!e.nonegate||!e.nocomment){if(t.noDeprecation!==true&&!r.deprecationWarned){var n="glob WARNING: comments and negation will be disabled in v6";if(t.throwDeprecation)throw new Error(n);else if(t.traceDeprecation)console.trace(n);else console.error(n);r.deprecationWarned=true}}}function d(e){var t=e.nounique;var r=t?[]:Object.create(null);for(var n=0,i=e.matches.length;n<i;n++){var a=e.matches[n];if(!a||Object.keys(a).length===0){if(e.nonull){var o=e.minimatch.globSet[n];if(t)r.push(o);else r[o]=true}}else{var s=Object.keys(a);if(t)r.push.apply(r,s);else s.forEach(function(e){r[e]=true})}}if(!t)r=Object.keys(r);if(!e.nosort)r=r.sort(e.nocase?f:u);if(e.mark){for(var n=0;n<r.length;n++){r[n]=e._mark(r[n])}if(e.nodir){r=r.filter(function(e){return!/\/$/.test(e)})}}if(e.ignore.length)r=r.filter(function(t){return!y(e,t)});e.found=r}function v(e,t){var r=g(e,t);var n=e.cache[r];var i=t;if(n){var a=n==="DIR"||Array.isArray(n);var o=t.slice(-1)==="/";if(a&&!o)i+="/";else if(!a&&o)i=i.slice(0,-1);if(i!==t){var s=g(e,i);e.statCache[s]=e.statCache[r];e.cache[s]=e.cache[r]}}return i}function g(e,t){var r=t;if(t.charAt(0)==="/"){r=i.join(e.root,t)}else if(o(t)||t===""){r=t}else if(e.changedCwd){r=i.resolve(e.cwd,t)}else{r=i.resolve(t)}return r}function y(e,t){if(!e.ignore.length)return false;return e.ignore.some(function(e){return e.matcher.match(t)||!!(e.gmatcher&&e.gmatcher.match(t))})}function b(e,t){if(!e.ignore.length)return false;return e.ignore.some(function(e){return!!(e.gmatcher&&e.gmatcher.match(t))})}}).call(this,e("_process"))},{_process:112,minimatch:92,path:108,"path-is-absolute":109}],47:[function(e,t,r){(function(r){t.exports=E;var n=e("fs");var i=e("minimatch");var a=i.Minimatch;var o=e("inherits");var s=e("events").EventEmitter;var f=e("path");var u=e("assert");var c=e("path-is-absolute");var l=e("./sync.js");var h=e("./common.js");var p=h.alphasort;var d=h.alphasorti;var v=h.setopts;var g=h.ownProp;var y=e("inflight");var b=e("util");var m=h.childrenIgnored;var _=h.isIgnored;var w=e("once");function E(e,t,r){if(typeof t==="function")r=t,t={};if(!t)t={};if(t.sync){if(r)throw new TypeError("callback provided to sync glob");return l(e,t)}return new x(e,t,r)}E.sync=l;var S=E.GlobSync=l.GlobSync;E.glob=E;E.hasMagic=function(e,t){var r=b._extend({},t);r.noprocess=true;var n=new x(e,r);var i=n.minimatch.set;if(i.length>1)return true;for(var a=0;a<i[0].length;a++){if(typeof i[0][a]!=="string")return true}return false};E.Glob=x;o(x,s);function x(e,t,r){if(typeof t==="function"){r=t;t=null}if(t&&t.sync){if(r)throw new TypeError("callback provided to sync glob");return new S(e,t)}if(!(this instanceof x))return new x(e,t,r);v(this,e,t);this._didRealPath=false;var n=this.minimatch.set.length;this.matches=new Array(n);if(typeof r==="function"){r=w(r);this.on("error",r);this.on("end",function(e){r(null,e)})}var i=this;var n=this.minimatch.set.length;this._processing=0;this.matches=new Array(n);this._emitQueue=[];this._processQueue=[];this.paused=false;if(this.noprocess)return this;if(n===0)return o();for(var a=0;a<n;a++){this._process(this.minimatch.set[a],a,false,o)}function o(){--i._processing;if(i._processing<=0)i._finish()}}x.prototype._finish=function(){u(this instanceof x);if(this.aborted)return;if(this.realpath&&!this._didRealpath)return this._realpath();h.finish(this);this.emit("end",this.found)};x.prototype._realpath=function(){if(this._didRealpath)return;this._didRealpath=true;var e=this.matches.length;if(e===0)return this._finish();var t=this;for(var r=0;r<this.matches.length;r++)this._realpathSet(r,n);function n(){if(--e===0)t._finish()}};x.prototype._realpathSet=function(e,t){var r=this.matches[e];if(!r)return t();var i=Object.keys(r);var a=this;var o=i.length;if(o===0)return t();var s=this.matches[e]=Object.create(null);i.forEach(function(r,i){r=a._makeAbs(r);n.realpath(r,a.realpathCache,function(n,i){if(!n)s[i]=true;else if(n.syscall==="stat")s[r]=true;else a.emit("error",n);if(--o===0){a.matches[e]=s;t()}})})};x.prototype._mark=function(e){return h.mark(this,e)};x.prototype._makeAbs=function(e){return h.makeAbs(this,e)};x.prototype.abort=function(){this.aborted=true;this.emit("abort")};x.prototype.pause=function(){if(!this.paused){this.paused=true;this.emit("pause")}};x.prototype.resume=function(){if(this.paused){this.emit("resume");this.paused=false;if(this._emitQueue.length){var e=this._emitQueue.slice(0);this._emitQueue.length=0;for(var t=0;t<e.length;t++){var r=e[t];this._emitMatch(r[0],r[1])}}if(this._processQueue.length){var n=this._processQueue.slice(0);this._processQueue.length=0;for(var t=0;t<n.length;t++){var i=n[t];this._processing--;this._process(i[0],i[1],i[2],i[3])}}}};x.prototype._process=function(e,t,r,n){u(this instanceof x);u(typeof n==="function");if(this.aborted)return;this._processing++;if(this.paused){this._processQueue.push([e,t,r,n]);return}var a=0;while(typeof e[a]==="string"){a++}var o;switch(a){case e.length:this._processSimple(e.join("/"),t,n);return;case 0:o=null;break;default:o=e.slice(0,a).join("/");break}var s=e.slice(a);var f;if(o===null)f=".";else if(c(o)||c(e.join("/"))){if(!o||!c(o))o="/"+o;f=o}else f=o;var l=this._makeAbs(f);if(m(this,f))return n();var h=s[0]===i.GLOBSTAR;if(h)this._processGlobStar(o,f,l,s,t,r,n);else this._processReaddir(o,f,l,s,t,r,n)};x.prototype._processReaddir=function(e,t,r,n,i,a,o){var s=this;this._readdir(r,a,function(f,u){return s._processReaddir2(e,t,r,n,i,a,u,o)})};x.prototype._processReaddir2=function(e,t,r,n,i,a,o,s){if(!o)return s();var u=n[0];var c=!!this.minimatch.negate;var l=u._glob;var h=this.dot||l.charAt(0)===".";var p=[];for(var d=0;d<o.length;d++){var v=o[d];if(v.charAt(0)!=="."||h){var g;if(c&&!e){g=!v.match(u)}else{g=v.match(u)}if(g)p.push(v)}}var y=p.length;if(y===0)return s();if(n.length===1&&!this.mark&&!this.stat){if(!this.matches[i])this.matches[i]=Object.create(null);for(var d=0;d<y;d++){var v=p[d];if(e){if(e!=="/")v=e+"/"+v;else v=e+v}if(v.charAt(0)==="/"&&!this.nomount){v=f.join(this.root,v)}this._emitMatch(i,v)}return s()}n.shift();for(var d=0;d<y;d++){var v=p[d];var b;if(e){if(e!=="/")v=e+"/"+v;else v=e+v}this._process([v].concat(n),i,a,s)}s()};x.prototype._emitMatch=function(e,t){if(this.aborted)return;if(this.matches[e][t])return;if(_(this,t))return;if(this.paused){this._emitQueue.push([e,t]);return}var r=this._makeAbs(t);if(this.nodir){var n=this.cache[r];if(n==="DIR"||Array.isArray(n))return}if(this.mark)t=this._mark(t);this.matches[e][t]=true;var i=this.statCache[r];if(i)this.emit("stat",t,i);this.emit("match",t)};x.prototype._readdirInGlobStar=function(e,t){if(this.aborted)return;if(this.follow)return this._readdir(e,false,t);var r="lstat\0"+e;var i=this;var a=y(r,o);if(a)n.lstat(e,a);function o(r,n){if(r)return t();var a=n.isSymbolicLink();i.symlinks[e]=a;if(!a&&!n.isDirectory()){i.cache[e]="FILE";t()}else i._readdir(e,false,t)}};x.prototype._readdir=function(e,t,r){if(this.aborted)return;r=y("readdir\0"+e+"\0"+t,r);if(!r)return;if(t&&!g(this.symlinks,e))return this._readdirInGlobStar(e,r);if(g(this.cache,e)){var i=this.cache[e];if(!i||i==="FILE")return r();if(Array.isArray(i))return r(null,i)}var a=this;n.readdir(e,O(this,e,r))};function O(e,t,r){return function(n,i){if(n)e._readdirError(t,n,r);else e._readdirEntries(t,i,r)}}x.prototype._readdirEntries=function(e,t,r){if(this.aborted)return;if(!this.mark&&!this.stat){for(var n=0;n<t.length;n++){var i=t[n];if(e==="/")i=e+i;else i=e+"/"+i;this.cache[i]=true}}this.cache[e]=t;return r(null,t)};x.prototype._readdirError=function(e,t,r){if(this.aborted)return;switch(t.code){case"ENOTSUP":case"ENOTDIR":this.cache[this._makeAbs(e)]="FILE";break;case"ENOENT":case"ELOOP":case"ENAMETOOLONG":case"UNKNOWN":this.cache[this._makeAbs(e)]=false;break;default:this.cache[this._makeAbs(e)]=false;if(this.strict){this.emit("error",t);this.abort()}if(!this.silent)console.error("glob error",t);break}return r()};x.prototype._processGlobStar=function(e,t,r,n,i,a,o){var s=this;this._readdir(r,a,function(f,u){s._processGlobStar2(e,t,r,n,i,a,u,o)})};x.prototype._processGlobStar2=function(e,t,r,n,i,a,o,s){if(!o)return s();var f=n.slice(1);var u=e?[e]:[];var c=u.concat(f);this._process(c,i,false,s);var l=this.symlinks[r];var h=o.length;if(l&&a)return s();for(var p=0;p<h;p++){var d=o[p];if(d.charAt(0)==="."&&!this.dot)continue;var v=u.concat(o[p],f);this._process(v,i,true,s);var g=u.concat(o[p],n);this._process(g,i,true,s)}s()};x.prototype._processSimple=function(e,t,r){var n=this;this._stat(e,function(i,a){n._processSimple2(e,t,i,a,r)})};x.prototype._processSimple2=function(e,t,n,i,a){if(!this.matches[t])this.matches[t]=Object.create(null);
if(!i)return a();if(e&&c(e)&&!this.nomount){var o=/[\/\\]$/.test(e);if(e.charAt(0)==="/"){e=f.join(this.root,e)}else{e=f.resolve(this.root,e);if(o)e+="/"}}if(r.platform==="win32")e=e.replace(/\\/g,"/");this._emitMatch(t,e);a()};x.prototype._stat=function(e,t){var r=this._makeAbs(e);var i=e.slice(-1)==="/";if(e.length>this.maxLength)return t();if(!this.stat&&g(this.cache,r)){var a=this.cache[r];if(Array.isArray(a))a="DIR";if(!i||a==="DIR")return t(null,a);if(i&&a==="FILE")return t()}var o;var s=this.statCache[r];if(s!==undefined){if(s===false)return t(null,s);else{var f=s.isDirectory()?"DIR":"FILE";if(i&&f==="FILE")return t();else return t(null,f,s)}}var u=this;var c=y("stat\0"+r,l);if(c)n.lstat(r,c);function l(i,a){if(a&&a.isSymbolicLink()){return n.stat(r,function(n,i){if(n)u._stat2(e,r,null,a,t);else u._stat2(e,r,n,i,t)})}else{u._stat2(e,r,i,a,t)}}};x.prototype._stat2=function(e,t,r,n,i){if(r){this.statCache[t]=false;return i()}var a=e.slice(-1)==="/";this.statCache[t]=n;if(t.slice(-1)==="/"&&!n.isDirectory())return i(null,false,n);var o=n.isDirectory()?"DIR":"FILE";this.cache[t]=this.cache[t]||o;if(a&&o!=="DIR")return i();return i(null,o,n)}}).call(this,e("_process"))},{"./common.js":46,"./sync.js":48,_process:112,assert:4,events:22,fs:11,inflight:56,inherits:57,minimatch:92,once:104,path:108,"path-is-absolute":109,util:148}],48:[function(e,t,r){(function(r){t.exports=y;y.GlobSync=b;var n=e("fs");var i=e("minimatch");var a=i.Minimatch;var o=e("./glob.js").Glob;var s=e("util");var f=e("path");var u=e("assert");var c=e("path-is-absolute");var l=e("./common.js");var h=l.alphasort;var p=l.alphasorti;var d=l.setopts;var v=l.ownProp;var g=l.childrenIgnored;function y(e,t){if(typeof t==="function"||arguments.length===3)throw new TypeError("callback provided to sync glob\n"+"See: https://github.com/isaacs/node-glob/issues/167");return new b(e,t).found}function b(e,t){if(!e)throw new Error("must provide pattern");if(typeof t==="function"||arguments.length===3)throw new TypeError("callback provided to sync glob\n"+"See: https://github.com/isaacs/node-glob/issues/167");if(!(this instanceof b))return new b(e,t);d(this,e,t);if(this.noprocess)return this;var r=this.minimatch.set.length;this.matches=new Array(r);for(var n=0;n<r;n++){this._process(this.minimatch.set[n],n,false)}this._finish()}b.prototype._finish=function(){u(this instanceof b);if(this.realpath){var e=this;this.matches.forEach(function(t,r){var i=e.matches[r]=Object.create(null);for(var a in t){try{a=e._makeAbs(a);var o=n.realpathSync(a,e.realpathCache);i[o]=true}catch(s){if(s.syscall==="stat")i[e._makeAbs(a)]=true;else throw s}}})}l.finish(this)};b.prototype._process=function(e,t,r){u(this instanceof b);var n=0;while(typeof e[n]==="string"){n++}var a;switch(n){case e.length:this._processSimple(e.join("/"),t);return;case 0:a=null;break;default:a=e.slice(0,n).join("/");break}var o=e.slice(n);var s;if(a===null)s=".";else if(c(a)||c(e.join("/"))){if(!a||!c(a))a="/"+a;s=a}else s=a;var f=this._makeAbs(s);if(g(this,s))return;var l=o[0]===i.GLOBSTAR;if(l)this._processGlobStar(a,s,f,o,t,r);else this._processReaddir(a,s,f,o,t,r)};b.prototype._processReaddir=function(e,t,r,n,i,a){var o=this._readdir(r,a);if(!o)return;var s=n[0];var u=!!this.minimatch.negate;var c=s._glob;var l=this.dot||c.charAt(0)===".";var h=[];for(var p=0;p<o.length;p++){var d=o[p];if(d.charAt(0)!=="."||l){var v;if(u&&!e){v=!d.match(s)}else{v=d.match(s)}if(v)h.push(d)}}var g=h.length;if(g===0)return;if(n.length===1&&!this.mark&&!this.stat){if(!this.matches[i])this.matches[i]=Object.create(null);for(var p=0;p<g;p++){var d=h[p];if(e){if(e.slice(-1)!=="/")d=e+"/"+d;else d=e+d}if(d.charAt(0)==="/"&&!this.nomount){d=f.join(this.root,d)}this.matches[i][d]=true}return}n.shift();for(var p=0;p<g;p++){var d=h[p];var y;if(e)y=[e,d];else y=[d];this._process(y.concat(n),i,a)}};b.prototype._emitMatch=function(e,t){var r=this._makeAbs(t);if(this.mark)t=this._mark(t);if(this.matches[e][t])return;if(this.nodir){var n=this.cache[this._makeAbs(t)];if(n==="DIR"||Array.isArray(n))return}this.matches[e][t]=true;if(this.stat)this._stat(t)};b.prototype._readdirInGlobStar=function(e){if(this.follow)return this._readdir(e,false);var t;var r;var i;try{r=n.lstatSync(e)}catch(a){return null}var o=r.isSymbolicLink();this.symlinks[e]=o;if(!o&&!r.isDirectory())this.cache[e]="FILE";else t=this._readdir(e,false);return t};b.prototype._readdir=function(e,t){var r;if(t&&!v(this.symlinks,e))return this._readdirInGlobStar(e);if(v(this.cache,e)){var i=this.cache[e];if(!i||i==="FILE")return null;if(Array.isArray(i))return i}try{return this._readdirEntries(e,n.readdirSync(e))}catch(a){this._readdirError(e,a);return null}};b.prototype._readdirEntries=function(e,t){if(!this.mark&&!this.stat){for(var r=0;r<t.length;r++){var n=t[r];if(e==="/")n=e+n;else n=e+"/"+n;this.cache[n]=true}}this.cache[e]=t;return t};b.prototype._readdirError=function(e,t){switch(t.code){case"ENOTSUP":case"ENOTDIR":this.cache[this._makeAbs(e)]="FILE";break;case"ENOENT":case"ELOOP":case"ENAMETOOLONG":case"UNKNOWN":this.cache[this._makeAbs(e)]=false;break;default:this.cache[this._makeAbs(e)]=false;if(this.strict)throw t;if(!this.silent)console.error("glob error",t);break}};b.prototype._processGlobStar=function(e,t,r,n,i,a){var o=this._readdir(r,a);if(!o)return;var s=n.slice(1);var f=e?[e]:[];var u=f.concat(s);this._process(u,i,false);var c=o.length;var l=this.symlinks[r];if(l&&a)return;for(var h=0;h<c;h++){var p=o[h];if(p.charAt(0)==="."&&!this.dot)continue;var d=f.concat(o[h],s);this._process(d,i,true);var v=f.concat(o[h],n);this._process(v,i,true)}};b.prototype._processSimple=function(e,t){var n=this._stat(e);if(!this.matches[t])this.matches[t]=Object.create(null);if(!n)return;if(e&&c(e)&&!this.nomount){var i=/[\/\\]$/.test(e);if(e.charAt(0)==="/"){e=f.join(this.root,e)}else{e=f.resolve(this.root,e);if(i)e+="/"}}if(r.platform==="win32")e=e.replace(/\\/g,"/");this.matches[t][e]=true};b.prototype._stat=function(e){var t=this._makeAbs(e);var r=e.slice(-1)==="/";if(e.length>this.maxLength)return false;if(!this.stat&&v(this.cache,t)){var i=this.cache[t];if(Array.isArray(i))i="DIR";if(!r||i==="DIR")return i;if(r&&i==="FILE")return false}var a;var o=this.statCache[t];if(!o){var s;try{s=n.lstatSync(t)}catch(f){return false}if(s.isSymbolicLink()){try{o=n.statSync(t)}catch(f){o=s}}else{o=s}}this.statCache[t]=o;var i=o.isDirectory()?"DIR":"FILE";this.cache[t]=this.cache[t]||i;if(r&&i!=="DIR")return false;return i};b.prototype._mark=function(e){return l.mark(this,e)};b.prototype._makeAbs=function(e){return l.makeAbs(this,e)}}).call(this,e("_process"))},{"./common.js":46,"./glob.js":47,_process:112,assert:4,fs:11,minimatch:92,path:108,"path-is-absolute":109,util:148}],49:[function(e,t,r){"use strict";var n=e("fs");var i=e("path");var a=e("lodash");var o=e("glob");var s=e("minimatch");var f=r;function u(e,t){return a.flatten(e).reduce(function(e,r){if(r.indexOf("!")===0){r=r.slice(1);return a.difference(e,t(r))}else{return a.union(e,t(r))}},[])}f.match=function(e,t,r){if(e==null||t==null){return[]}if(!a.isArray(e)){e=[e]}if(!a.isArray(t)){t=[t]}if(e.length===0||t.length===0){return[]}return u(e,function(e){return s.match(t,e,r||{})})};f.isMatch=function(){return f.match.apply(null,arguments).length>0};f.find=function(){var e=a.toArray(arguments);var t=a.isPlainObject(e[e.length-1])?e.pop():{};var r=a.isArray(e[0])?e[0]:e;if(r.length===0){return[]}var s=t.srcBase||t.cwd;var f=a.extend({},t);if(s){f.cwd=s}var c=u(r,function(e){return o.sync(e,f)});if(s&&t.prefixBase){c=c.map(function(e){return i.join(s,e)})}if(t.filter){c=c.filter(function(e){if(s&&!t.prefixBase){e=i.join(s,e)}try{if(a.isFunction(t.filter)){return t.filter(e,t)}else{return n.statSync(e)[t.filter]()}}catch(r){return false}})}return c};var c=/[\/\\]/g;var l={first:/(\.[^\/]*)?$/,last:/(\.[^\/\.]*)?$/};function h(e,t){if(t.flatten){e=i.basename(e)}if(t.ext){e=e.replace(l[t.extDot],t.ext)}if(t.destBase){e=i.join(t.destBase,e)}return e}f.mapping=function(e,t){if(e==null){return[]}t=a.defaults({},t,{extDot:"first",rename:h});var r=[];var n={};e.forEach(function(e){var a=t.rename(e,t);if(t.srcBase){e=i.join(t.srcBase,e)}a=a.replace(c,"/");e=e.replace(c,"/");if(n[a]){n[a].src.push(e)}else{r.push({src:[e],dest:a});n[a]=r[r.length-1]}});return r};f.findMapping=function(e,t){return f.mapping(f.find(e,t),t)}},{fs:11,glob:50,lodash:84,minimatch:52,path:108}],50:[function(e,t,r){(function(r){t.exports=l;var n=e("graceful-fs"),i=e("minimatch"),a=i.Minimatch,o=e("inherits"),s=e("events").EventEmitter,f=e("path"),u={},c=e("assert").ok;function l(e,t,r){if(typeof t==="function")r=t,t={};if(!t)t={};if(typeof t==="number"){h();return}var n=new d(e,t,r);return n.sync?n.found:n}l.fnmatch=h;function h(){throw new Error("glob's interface has changed. Please see the docs.")}l.sync=p;function p(e,t){if(typeof t==="number"){h();return}t=t||{};t.sync=true;return l(e,t)}l.Glob=d;o(d,s);function d(e,t,n){if(!(this instanceof d)){return new d(e,t,n)}if(typeof n==="function"){this.on("error",n);this.on("end",function(e){n(null,e)})}t=t||{};this.EOF={};this._emitQueue=[];this.maxDepth=t.maxDepth||1e3;this.maxLength=t.maxLength||Infinity;this.statCache=t.statCache||{};this.changedCwd=false;var i=r.cwd();if(!t.hasOwnProperty("cwd"))this.cwd=i;else{this.cwd=t.cwd;this.changedCwd=f.resolve(t.cwd)!==i}this.root=t.root||f.resolve(this.cwd,"/");this.root=f.resolve(this.root);if(r.platform==="win32")this.root=this.root.replace(/\\/g,"/");this.nomount=!!t.nomount;if(!e){throw new Error("must provide pattern")}if(t.matchBase&&-1===e.indexOf("/")){if(t.noglobstar){throw new Error("base matching requires globstar")}e="**/"+e}this.strict=t.strict!==false;this.dot=!!t.dot;this.mark=!!t.mark;this.sync=!!t.sync;this.nounique=!!t.nounique;this.nonull=!!t.nonull;this.nosort=!!t.nosort;this.nocase=!!t.nocase;this.stat=!!t.stat;this.debug=!!t.debug||!!t.globDebug;if(this.debug)this.log=console.error;this.silent=!!t.silent;var o=this.minimatch=new a(e,t);this.options=o.options;e=this.pattern=o.pattern;this.error=null;this.aborted=false;s.call(this);var u=this.minimatch.set.length;this.matches=new Array(u);this.minimatch.set.forEach(c.bind(this));function c(e,t,r){this._process(e,0,t,function(e){if(e)this.emit("error",e);if(--u<=0)this._finish()})}}d.prototype.log=function(){};d.prototype._finish=function(){c(this instanceof d);var e=this.nounique,t=e?[]:{};for(var r=0,n=this.matches.length;r<n;r++){var i=this.matches[r];this.log("matches[%d] =",r,i);if(!i){if(this.nonull){var a=this.minimatch.globSet[r];if(e)t.push(a);else t[a]=true}}else{var o=Object.keys(i);if(e)t.push.apply(t,o);else o.forEach(function(e){t[e]=true})}}if(!e)t=Object.keys(t);if(!this.nosort){t=t.sort(this.nocase?v:g)}if(this.mark){t=t.map(function(e){var t=this.statCache[e];if(!t)return e;var r=Array.isArray(t)||t===2;if(r&&e.slice(-1)!=="/"){return e+"/"}if(!r&&e.slice(-1)==="/"){return e.replace(/\/+$/,"")}return e},this)}this.log("emitting end",t);this.EOF=this.found=t;this.emitMatch(this.EOF)};function v(e,t){e=e.toLowerCase();t=t.toLowerCase();return g(e,t)}function g(e,t){return e>t?1:e<t?-1:0}d.prototype.abort=function(){this.aborted=true;this.emit("abort")};d.prototype.pause=function(){if(this.paused)return;if(this.sync)this.emit("error",new Error("Can't pause/resume sync glob"));this.paused=true;this.emit("pause")};d.prototype.resume=function(){if(!this.paused)return;if(this.sync)this.emit("error",new Error("Can't pause/resume sync glob"));this.paused=false;this.emit("resume");this._processEmitQueue()};d.prototype.emitMatch=function(e){this._emitQueue.push(e);this._processEmitQueue()};d.prototype._processEmitQueue=function(e){while(!this._processingEmitQueue&&!this.paused){this._processingEmitQueue=true;var e=this._emitQueue.shift();if(!e){this._processingEmitQueue=false;break}this.log("emit!",e===this.EOF?"end":"match");this.emit(e===this.EOF?"end":"match",e);this._processingEmitQueue=false}};d.prototype._process=function(e,t,n,a){c(this instanceof d);var o=function h(e,t){c(this instanceof d);if(this.paused){if(!this._processQueue){this._processQueue=[];this.once("resume",function(){var e=this._processQueue;this._processQueue=null;e.forEach(function(e){e()})})}this._processQueue.push(a.bind(this,e,t))}else{a.call(this,e,t)}}.bind(this);if(this.aborted)return o();if(t>this.maxDepth)return o();var s=0;while(typeof e[s]==="string"){s++}var u;switch(s){case e.length:u=e.join("/");this._stat(u,function(e,t){if(e){if(u&&y(u)&&!this.nomount){if(u.charAt(0)==="/"){u=f.join(this.root,u)}else{u=f.resolve(this.root,u)}}if(r.platform==="win32")u=u.replace(/\\/g,"/");this.matches[n]=this.matches[n]||{};this.matches[n][u]=true;this.emitMatch(u)}return o()});return;case 0:u=null;break;default:u=e.slice(0,s);u=u.join("/");break}var l;if(u===null)l=".";else if(y(u)||y(e.join("/"))){if(!u||!y(u)){u=f.join("/",u)}l=u=f.resolve(u);this.log("absolute: ",u,this.root,e,l)}else{l=u}this.log("readdir(%j)",l,this.cwd,this.root);return this._readdir(l,function(a,c){if(a){return o()}if(e[s]===i.GLOBSTAR){var l=[e.slice(0,s).concat(e.slice(s+1))];c.forEach(function(t){if(t.charAt(0)==="."&&!this.dot)return;l.push(e.slice(0,s).concat(t).concat(e.slice(s+1)));l.push(e.slice(0,s).concat(t).concat(e.slice(s)))},this);var h=l.length,p=null;l.forEach(function(e){this._process(e,t+1,n,function(e){if(p)return;if(e)return o(p=e);if(--h<=0)return o()})},this);return}var d=e[s];if(typeof d==="string"){var v=c.indexOf(d)!==-1;c=v?c[d]:[]}else{var g=e[s]._glob,y=this.dot||g.charAt(0)===".";c=c.filter(function(t){return(t.charAt(0)!=="."||y)&&(typeof e[s]==="string"&&t===e[s]||t.match(e[s]))})}if(s===e.length-1&&!this.mark&&!this.stat){c.forEach(function(e){if(u){if(u!=="/")e=u+"/"+e;else e=u+e}if(e.charAt(0)==="/"&&!this.nomount){e=f.join(this.root,e)}if(r.platform==="win32")e=e.replace(/\\/g,"/");this.matches[n]=this.matches[n]||{};this.matches[n][e]=true;this.emitMatch(e)},this);return o.call(this)}var h=c.length,p=null;if(h===0)return o();c.forEach(function(r){var i=e.slice(0,s).concat(r).concat(e.slice(s+1));this._process(i,t+1,n,function(e){if(p)return;if(e)return o(p=e);if(--h===0)return o.call(this)})},this)})};d.prototype._stat=function(e,t){c(this instanceof d);var i=e;if(e.charAt(0)==="/"){i=f.join(this.root,e)}else if(this.changedCwd){i=f.resolve(this.cwd,e)}this.log("stat",[this.cwd,e,"=",i]);if(e.length>this.maxLength){var a=new Error("Path name too long");a.code="ENAMETOOLONG";a.path=e;return this._afterStat(e,i,t,a)}if(this.statCache.hasOwnProperty(e)){var o=this.statCache[e],s=o&&(Array.isArray(o)||o===2);if(this.sync)return t.call(this,!!o,s);return r.nextTick(t.bind(this,!!o,s))}if(this.sync){var a,u;try{u=n.statSync(i)}catch(l){a=l}this._afterStat(e,i,t,a,u)}else{n.stat(i,this._afterStat.bind(this,e,i,t))}};d.prototype._afterStat=function(e,t,r,n,i){var a;c(this instanceof d);if(t.slice(-1)==="/"&&i&&!i.isDirectory()){this.log("should be ENOTDIR, fake it");n=new Error("ENOTDIR, not a directory '"+t+"'");n.path=t;n.code="ENOTDIR";i=null}if(n||!i){a=false}else{a=i.isDirectory()?2:1}this.statCache[e]=this.statCache[e]||a;r.call(this,!!a,a===2)};d.prototype._readdir=function(e,t){c(this instanceof d);var i=e;if(e.charAt(0)==="/"){i=f.join(this.root,e)}else if(y(e)){i=e}else if(this.changedCwd){i=f.resolve(this.cwd,e)}this.log("readdir",[this.cwd,e,i]);if(e.length>this.maxLength){var a=new Error("Path name too long");a.code="ENAMETOOLONG";a.path=e;return this._afterReaddir(e,i,t,a)}if(this.statCache.hasOwnProperty(e)){var o=this.statCache[e];if(Array.isArray(o)){if(this.sync)return t.call(this,null,o);return r.nextTick(t.bind(this,null,o))}if(!o||o===1){var s=o?"ENOTDIR":"ENOENT",a=new Error((o?"Not a directory":"Not found")+": "+e);a.path=e;a.code=s;this.log(e,a);if(this.sync)return t.call(this,a);return r.nextTick(t.bind(this,a))}}if(this.sync){var a,u;try{u=n.readdirSync(i)}catch(l){a=l}return this._afterReaddir(e,i,t,a,u)}n.readdir(i,this._afterReaddir.bind(this,e,i,t))};d.prototype._afterReaddir=function(e,t,r,n,i){c(this instanceof d);if(i&&!n){this.statCache[e]=i;if(!this.mark&&!this.stat){i.forEach(function(t){if(e==="/")t=e+t;else t=e+"/"+t;this.statCache[t]=true},this)}return r.call(this,n,i)}if(n)switch(n.code){case"ENOTDIR":this.statCache[e]=1;return r.call(this,n);case"ENOENT":case"ELOOP":case"ENAMETOOLONG":case"UNKNOWN":this.statCache[e]=false;return r.call(this,n);default:this.statCache[e]=false;if(this.strict)this.emit("error",n);if(!this.silent)console.error("glob error",n);return r.call(this,n)}};var y=r.platform==="win32"?b:m;function b(e){if(m(e))return true;var t=/^([a-zA-Z]:|[\\\/]{2}[^\\\/]+[\\\/]+[^\\\/]+)?([\\\/])?([\s\S]*?)$/,r=t.exec(e),n=r[1]||"",i=n&&n.charAt(1)!==":",a=!!r[2]||i;return a}function m(e){return e.charAt(0)==="/"||e===""}}).call(this,e("_process"))},{_process:112,assert:4,events:22,"graceful-fs":53,inherits:51,minimatch:52,path:108}],51:[function(e,t,r){t.exports=n;function n(e,t,r){r=r||{};var n={};[e.prototype,r].forEach(function(e){Object.getOwnPropertyNames(e).forEach(function(t){n[t]=Object.getOwnPropertyDescriptor(e,t)})});e.prototype=Object.create(t.prototype,n);e.super=t}},{}],52:[function(e,t,r){(function(r){(function(e,t,r,n){if(r)r.exports=b;else t.minimatch=b;if(!e){e=function(e){switch(e){case"sigmund":return function t(e){return JSON.stringify(e)};case"path":return{basename:function(e){e=e.split(/[\/\\]/);var t=e.pop();if(!t)t=e.pop();return t}};case"lru-cache":return function r(){var e={};var t=0;this.set=function(r,n){t++;if(t>=100)e={};e[r]=n};this.get=function(t){return e[t]}}}}}b.Minimatch=m;var i=e("lru-cache"),a=b.cache=new i({max:100}),o=b.GLOBSTAR=m.GLOBSTAR={},s=e("sigmund");var f=e("path"),u="[^/]",c=u+"*?",l="(?:(?!(?:\\/|^)(?:\\.{1,2})($|\\/)).)*?",h="(?:(?!(?:\\/|^)\\.).)*?",p=d("().*{}+?[]^$\\!");function d(e){return e.split("").reduce(function(e,t){e[t]=true;return e},{})}var v=/\/+/;b.filter=g;function g(e,t){t=t||{};return function(r,n,i){return b(r,e,t)}}function y(e,t){e=e||{};t=t||{};var r={};Object.keys(t).forEach(function(e){r[e]=t[e]});Object.keys(e).forEach(function(t){r[t]=e[t]});return r}b.defaults=function(e){if(!e||!Object.keys(e).length)return b;var t=b;var r=function n(r,i,a){return t.minimatch(r,i,y(e,a))};r.Minimatch=function i(r,n){return new t.Minimatch(r,y(e,n))};return r};m.defaults=function(e){if(!e||!Object.keys(e).length)return m;return b.defaults(e).Minimatch};function b(e,t,r){if(typeof t!=="string"){throw new TypeError("glob pattern string required")}if(!r)r={};if(!r.nocomment&&t.charAt(0)==="#"){return false}if(t.trim()==="")return e==="";return new m(t,r).match(e)}function m(e,t){if(!(this instanceof m)){return new m(e,t,a)}if(typeof e!=="string"){throw new TypeError("glob pattern string required")}if(!t)t={};e=e.trim();if(n==="win32"){e=e.split("\\").join("/")}var r=e+"\n"+s(t);var i=b.cache.get(r);if(i)return i;b.cache.set(r,this);this.options=t;this.set=[];this.pattern=e;this.regexp=null;this.negate=false;this.comment=false;this.empty=false;this.make()}m.prototype.debug=function(){};m.prototype.make=_;function _(){if(this._made)return;var e=this.pattern;var t=this.options;if(!t.nocomment&&e.charAt(0)==="#"){this.comment=true;return}if(!e){this.empty=true;return}this.parseNegate();var r=this.globSet=this.braceExpand();if(t.debug)this.debug=console.error;this.debug(this.pattern,r);r=this.globParts=r.map(function(e){return e.split(v)});this.debug(this.pattern,r);r=r.map(function(e,t,r){return e.map(this.parse,this)},this);this.debug(this.pattern,r);r=r.filter(function(e){return-1===e.indexOf(false)});this.debug(this.pattern,r);this.set=r}m.prototype.parseNegate=w;function w(){var e=this.pattern,t=false,r=this.options,n=0;if(r.nonegate)return;for(var i=0,a=e.length;i<a&&e.charAt(i)==="!";i++){t=!t;n++}if(n)this.pattern=e.substr(n);this.negate=t}b.braceExpand=function(e,t){return new m(e,t).braceExpand()};m.prototype.braceExpand=E;function E(e,t){t=t||this.options;e=typeof e==="undefined"?this.pattern:e;if(typeof e==="undefined"){throw new Error("undefined pattern")}if(t.nobrace||!e.match(/\{.*\}/)){return[e]}var r=false;if(e.charAt(0)!=="{"){this.debug(e);var n=null;for(var i=0,a=e.length;i<a;i++){var o=e.charAt(i);this.debug(i,o);if(o==="\\"){r=!r}else if(o==="{"&&!r){n=e.substr(0,i);break}}if(n===null){this.debug("no sets");return[e]}var s=E.call(this,e.substr(i),t);return s.map(function(e){return n+e})}var f=e.match(/^\{(-?[0-9]+)\.\.(-?[0-9]+)\}/);if(f){this.debug("numset",f[1],f[2]);var u=E.call(this,e.substr(f[0].length),t),c=+f[1],l=+f[2],h=c>l?-1:1,p=[];for(var i=c;i!=l+h;i+=h){for(var d=0,v=u.length;d<v;d++){p.push(i+u[d])}}return p}var i=1,g=1,p=[],y="",b=false,r=false;function m(){p.push(y);y=""}this.debug("Entering for");e:for(i=1,a=e.length;i<a;i++){var o=e.charAt(i);this.debug("",i,o);if(r){r=false;y+="\\"+o}else{switch(o){case"\\":r=true;continue;case"{":g++;y+="{";continue;case"}":g--;if(g===0){m();i++;break e}else{y+=o;continue}case",":if(g===1){m()}else{y+=o}continue;default:y+=o;continue}}}if(g!==0){this.debug("didn't close",e);return E.call(this,"\\"+e,t)}this.debug("set",p);this.debug("suffix",e.substr(i));var u=E.call(this,e.substr(i),t);var _=p.length===1;this.debug("set pre-expanded",p);p=p.map(function(e){return E.call(this,e,t)},this);this.debug("set expanded",p);p=p.reduce(function(e,t){return e.concat(t)});if(_){p=p.map(function(e){return"{"+e+"}"})}var w=[];for(var i=0,a=p.length;i<a;i++){for(var d=0,v=u.length;d<v;d++){w.push(p[i]+u[d])}}return w}m.prototype.parse=x;var S={};function x(e,t){var r=this.options;if(!r.noglobstar&&e==="**")return o;if(e==="")return"";var n="",i=!!r.nocase,a=false,s=[],f,l,h=false,d=-1,v=-1,g=e.charAt(0)==="."?"":r.dot?"(?!(?:^|\\/)\\.{1,2}(?:$|\\/))":"(?!\\.)",y=this;function b(){if(l){switch(l){case"*":n+=c;i=true;break;case"?":n+=u;i=true;break;default:n+="\\"+l;break}y.debug("clearStateChar %j %j",l,n);l=false}}for(var m=0,_=e.length,w;m<_&&(w=e.charAt(m));m++){this.debug("%s\t%s %s %j",e,m,n,w);if(a&&p[w]){n+="\\"+w;a=false;continue}e:switch(w){case"/":return false;case"\\":b();a=true;continue;case"?":case"*":case"+":case"@":case"!":this.debug("%s\t%s %s %j <-- stateChar",e,m,n,w);if(h){this.debug("  in class");if(w==="!"&&m===v+1)w="^";n+=w;continue}y.debug("call clearStateChar %j",l);b();l=w;if(r.noext)b();continue;case"(":if(h){n+="(";continue}if(!l){n+="\\(";continue}f=l;s.push({type:f,start:m-1,reStart:n.length});n+=l==="!"?"(?:(?!":"(?:";this.debug("plType %j %j",l,n);l=false;continue;case")":if(h||!s.length){n+="\\)";continue}b();i=true;n+=")";f=s.pop().type;switch(f){case"!":n+="[^/]*?)";break;case"?":case"+":case"*":n+=f;case"@":break}continue;case"|":if(h||!s.length||a){n+="\\|";a=false;continue}b();n+="|";continue;case"[":b();if(h){n+="\\"+w;continue}h=true;v=m;d=n.length;n+=w;continue;case"]":if(m===v+1||!h){n+="\\"+w;a=false;continue}i=true;h=false;n+=w;continue;default:b();if(a){a=false}else if(p[w]&&!(w==="^"&&h)){n+="\\"}n+=w}}if(h){var E=e.substr(v+1),x=this.parse(E,S);n=n.substr(0,d)+"\\["+x[0];i=i||x[1]}var O;while(O=s.pop()){var j=n.slice(O.reStart+3);j=j.replace(/((?:\\{2})*)(\\?)\|/g,function(e,t,r){if(!r){r="\\"}return t+t+r+"|"});this.debug("tail=%j\n   %s",j,j);var k=O.type==="*"?c:O.type==="?"?u:"\\"+O.type;i=true;n=n.slice(0,O.reStart)+k+"\\("+j}b();if(a){n+="\\\\"}var R=false;switch(n.charAt(0)){case".":case"[":case"(":R=true}if(n!==""&&i)n="(?=.)"+n;if(R)n=g+n;if(t===S){return[n,i]}if(!i){return A(e)}var T=r.nocase?"i":"",L=new RegExp("^"+n+"$",T);L._glob=e;L._src=n;return L}b.makeRe=function(e,t){return new m(e,t||{}).makeRe()};m.prototype.makeRe=O;function O(){if(this.regexp||this.regexp===false)return this.regexp;var e=this.set;if(!e.length)return this.regexp=false;var t=this.options;var r=t.noglobstar?c:t.dot?l:h,n=t.nocase?"i":"";var i=e.map(function(e){return e.map(function(e){return e===o?r:typeof e==="string"?k(e):e._src}).join("\\/")}).join("|");i="^(?:"+i+")$";if(this.negate)i="^(?!"+i+").*$";try{return this.regexp=new RegExp(i,n)}catch(a){return this.regexp=false}}b.match=function(e,t,r){var n=new m(t,r);e=e.filter(function(e){return n.match(e)});if(r.nonull&&!e.length){e.push(t)}return e};m.prototype.match=j;function j(e,t){this.debug("match",e,this.pattern);if(this.comment)return false;if(this.empty)return e==="";if(e==="/"&&t)return true;var r=this.options;if(n==="win32"){e=e.split("\\").join("/")}e=e.split(v);this.debug(this.pattern,"split",e);var i=this.set;this.debug(this.pattern,"set",i);var a=f.basename(e.join("/")).split("/");for(var o=0,s=i.length;o<s;o++){var u=i[o],c=e;if(r.matchBase&&u.length===1){c=a}var l=this.matchOne(c,u,t);if(l){if(r.flipNegate)return true;return!this.negate}}if(r.flipNegate)return false;return this.negate}m.prototype.matchOne=function(e,t,r){var n=this.options;this.debug("matchOne",{"this":this,file:e,pattern:t});this.debug("matchOne",e.length,t.length);for(var i=0,a=0,s=e.length,f=t.length;i<s&&a<f;i++,a++){this.debug("matchOne loop");var u=t[a],c=e[i];this.debug(t,u,c);if(u===false)return false;if(u===o){this.debug("GLOBSTAR",[t,u,c]);var l=i,h=a+1;if(h===f){this.debug("** at the end");for(;i<s;i++){if(e[i]==="."||e[i]===".."||!n.dot&&e[i].charAt(0)===".")return false}return true}e:while(l<s){var p=e[l];this.debug("\nglobstar while",e,l,t,h,p);if(this.matchOne(e.slice(l),t.slice(h),r)){this.debug("globstar found match!",l,s,p);return true}else{if(p==="."||p===".."||!n.dot&&p.charAt(0)==="."){this.debug("dot detected!",e,l,t,h);break e}this.debug("globstar swallow a segment, and continue");l++}}if(r){this.debug("\n>>> no match, partial?",e,l,t,h);if(l===s)return true}return false}var d;if(typeof u==="string"){if(n.nocase){d=c.toLowerCase()===u.toLowerCase()}else{d=c===u}this.debug("string match",u,c,d)}else{d=c.match(u);this.debug("pattern match",u,c,d)}if(!d)return false}if(i===s&&a===f){return true}else if(i===s){return r}else if(a===f){var v=i===s-1&&e[i]==="";return v}throw new Error("wtf?")};function A(e){return e.replace(/\\(.)/g,"$1")}function k(e){return e.replace(/[-[\]{}()*+?.,\\^$|#\s]/g,"\\$&")}})(typeof e==="function"?e:null,this,typeof t==="object"?t:null,typeof r==="object"?r.platform:"win32")}).call(this,e("_process"))},{_process:112,"lru-cache":85,path:108,sigmund:133}],53:[function(e,t,r){(function(n){var i=r=t.exports={};i._originalFs=e("fs");Object.getOwnPropertyNames(i._originalFs).forEach(function(e){var t=Object.getOwnPropertyDescriptor(i._originalFs,e);Object.defineProperty(i,e,t)});var a=[],o=e("constants");i._curOpen=0;i.MIN_MAX_OPEN=64;i.MAX_OPEN=1024;function s(e,t,r,n){this.path=e;this.flags=t;this.mode=r;this.cb=n}function f(){}i.open=u;function u(e,t,r,n){if(typeof r==="function")n=r,r=null;if(typeof n!=="function")n=f;if(i._curOpen>=i.MAX_OPEN){a.push(new s(e,t,r,n));setTimeout(h);return}c(e,t,r,function(a,o){if(a&&a.code==="EMFILE"&&i._curOpen>i.MIN_MAX_OPEN){i.MAX_OPEN=i._curOpen-1;return i.open(e,t,r,n)}n(a,o)})}function c(e,t,r,n){n=n||f;i._curOpen++;i._originalFs.open.call(i,e,t,r,function(e,t){if(e)l();n(e,t)})}i.openSync=function(e,t,r){var n;n=i._originalFs.openSync.call(i,e,t,r);i._curOpen++;return n};function l(){i._curOpen--;h()}function h(){while(i._curOpen<i.MAX_OPEN){var e=a.shift();if(!e)return;switch(e.constructor.name){case"OpenReq":c(e.path,e.flags||"r",e.mode||511,e.cb);break;case"ReaddirReq":d(e.path,e.cb);break;case"ReadFileReq":y(e.path,e.options,e.cb);break;case"WriteFileReq":_(e.path,e.data,e.options,e.cb);break;default:throw new Error("Unknown req type: "+e.constructor.name)}}}i.close=function(e,t){t=t||f;i._originalFs.close.call(i,e,function(e){l();t(e)})};i.closeSync=function(e){try{return i._originalFs.closeSync.call(i,e)}finally{l()}};i.readdir=p;function p(e,t){if(i._curOpen>=i.MAX_OPEN){a.push(new v(e,t));setTimeout(h);return}d(e,function(r,n){if(r&&r.code==="EMFILE"&&i._curOpen>i.MIN_MAX_OPEN){i.MAX_OPEN=i._curOpen-1;return i.readdir(e,t)}t(r,n)})}function d(e,t){t=t||f;i._curOpen++;i._originalFs.readdir.call(i,e,function(e,r){l();t(e,r)})}function v(e,t){this.path=e;this.cb=t}i.readFile=g;function g(e,t,r){if(typeof t==="function")r=t,t=null;if(typeof r!=="function")r=f;if(i._curOpen>=i.MAX_OPEN){a.push(new b(e,t,r));setTimeout(h);return}y(e,t,function(n,a){if(n&&n.code==="EMFILE"&&i._curOpen>i.MIN_MAX_OPEN){i.MAX_OPEN=i._curOpen-1;return i.readFile(e,t,r)}r(n,a)})}function y(e,t,r){r=r||f;i._curOpen++;i._originalFs.readFile.call(i,e,t,function(e,t){l();r(e,t)})}function b(e,t,r){this.path=e;this.options=t;this.cb=r}i.writeFile=m;function m(e,t,r,n){if(typeof r==="function")n=r,r=null;if(typeof n!=="function")n=f;if(i._curOpen>=i.MAX_OPEN){a.push(new w(e,t,r,n));setTimeout(h);return}_(e,t,r,function(a){if(a&&a.code==="EMFILE"&&i._curOpen>i.MIN_MAX_OPEN){i.MAX_OPEN=i._curOpen-1;return i.writeFile(e,t,r,n)}n(a)})}function _(e,t,r,n){n=n||f;i._curOpen++;i._originalFs.writeFile.call(i,e,t,r,function(e){l();n(e)})}function w(e,t,r,n){this.path=e;this.data=t;this.options=r;this.cb=n}var o=e("constants");if(o.hasOwnProperty("O_SYMLINK")&&n.version.match(/^v0\.6\.[0-2]|^v0\.5\./)){i.lchmod=function(e,t,r){r=r||f;i.open(e,o.O_WRONLY|o.O_SYMLINK,t,function(e,n){if(e){r(e);return}i.fchmod(n,t,function(e){i.close(n,function(t){r(e||t)})})})};i.lchmodSync=function(e,t){var r=i.openSync(e,o.O_WRONLY|o.O_SYMLINK,t);var n,a;try{var s=i.fchmodSync(r,t)}catch(f){n=f}try{i.closeSync(r)}catch(f){a=f}if(n||a)throw n||a;return s}}if(!i.lutimes){if(o.hasOwnProperty("O_SYMLINK")){i.lutimes=function(e,t,r,n){i.open(e,o.O_SYMLINK,function(e,a){n=n||f;if(e)return n(e);i.futimes(a,t,r,function(e){i.close(a,function(t){return n(e||t)})})})};i.lutimesSync=function(e,t,r){var n=i.openSync(e,o.O_SYMLINK),a,s,f;try{var f=i.futimesSync(n,t,r)}catch(u){a=u}try{i.closeSync(n)}catch(u){s=u}if(a||s)throw a||s;return f}}else if(i.utimensat&&o.hasOwnProperty("AT_SYMLINK_NOFOLLOW")){i.lutimes=function(e,t,r,n){i.utimensat(e,t,r,o.AT_SYMLINK_NOFOLLOW,n)};i.lutimesSync=function(e,t,r){return i.utimensatSync(e,t,r,o.AT_SYMLINK_NOFOLLOW)}}else{i.lutimes=function(e,t,r,i){n.nextTick(i)};i.lutimesSync=function(){}}}i.chown=E(i.chown);i.fchown=E(i.fchown);i.lchown=E(i.lchown);i.chownSync=S(i.chownSync);i.fchownSync=S(i.fchownSync);i.lchownSync=S(i.lchownSync);function E(e){if(!e)return e;return function(t,r,n,a){return e.call(i,t,r,n,function(e,t){if(x(e))e=null;a(e,t)})}}function S(e){if(!e)return e;return function(t,r,n){try{return e.call(i,t,r,n)}catch(a){if(!x(a))throw a}}}function x(e){if(!e||(!n.getuid||n.getuid()!==0)&&(e.code==="EINVAL"||e.code==="EPERM"))return true}if(!i.lchmod){i.lchmod=function(e,t,r){n.nextTick(r)};i.lchmodSync=function(){}}if(!i.lchown){i.lchown=function(e,t,r,i){n.nextTick(i)};i.lchownSync=function(){}}if(n.platform==="win32"){var O=i.rename;i.rename=function k(e,t,r){var n=Date.now();O(e,t,function i(a){if(a&&(a.code==="EACCES"||a.code==="EPERM")&&Date.now()-n<1e3){return O(e,t,i)}r(a)})}}var j=i.read;i.read=function(e,t,r,n,a,o){var s;if(o&&typeof o==="function"){var f=0;s=function(u,c,l){if(u&&u.code==="EAGAIN"&&f<10){f++;return j.call(i,e,t,r,n,a,s)}o.apply(this,arguments)}}return j.call(i,e,t,r,n,a,s)};var A=i.readSync;i.readSync=function(e,t,r,n,a){var o=0;while(true){try{return A.call(i,e,t,r,n,a)}catch(s){if(s.code==="EAGAIN"&&o<10){o++;continue}throw s}}}}).call(this,e("_process"))},{_process:112,constants:18,fs:11}],54:[function(e,t,r){r.read=function(e,t,r,n,i){var a,o;var s=i*8-n-1;var f=(1<<s)-1;var u=f>>1;var c=-7;var l=r?i-1:0;var h=r?-1:1;var p=e[t+l];l+=h;a=p&(1<<-c)-1;p>>=-c;c+=s;for(;c>0;a=a*256+e[t+l],l+=h,c-=8){}o=a&(1<<-c)-1;a>>=-c;c+=n;for(;c>0;o=o*256+e[t+l],l+=h,c-=8){}if(a===0){a=1-u}else if(a===f){return o?NaN:(p?-1:1)*Infinity}else{o=o+Math.pow(2,n);a=a-u}return(p?-1:1)*o*Math.pow(2,a-n)};r.write=function(e,t,r,n,i,a){var o,s,f;var u=a*8-i-1;var c=(1<<u)-1;var l=c>>1;var h=i===23?Math.pow(2,-24)-Math.pow(2,-77):0;var p=n?0:a-1;var d=n?1:-1;var v=t<0||t===0&&1/t<0?1:0;t=Math.abs(t);if(isNaN(t)||t===Infinity){s=isNaN(t)?1:0;o=c}else{o=Math.floor(Math.log(t)/Math.LN2);if(t*(f=Math.pow(2,-o))<1){
o--;f*=2}if(o+l>=1){t+=h/f}else{t+=h*Math.pow(2,1-l)}if(t*f>=2){o++;f/=2}if(o+l>=c){s=0;o=c}else if(o+l>=1){s=(t*f-1)*Math.pow(2,i);o=o+l}else{s=t*Math.pow(2,l-1)*Math.pow(2,i);o=0}}for(;i>=8;e[r+p]=s&255,p+=d,s/=256,i-=8){}o=o<<i|s;u+=i;for(;u>0;e[r+p]=o&255,p+=d,o/=256,u-=8){}e[r+p-d]|=v*128}},{}],55:[function(e,t,r){var n=[].indexOf;t.exports=function(e,t){if(n)return e.indexOf(t);for(var r=0;r<e.length;++r){if(e[r]===t)return r}return-1}},{}],56:[function(e,t,r){(function(r){var n=e("wrappy");var i=Object.create(null);var a=e("once");t.exports=n(o);function o(e,t){if(i[e]){i[e].push(t);return null}else{i[e]=[t];return s(e)}}function s(e){return a(function t(){var n=i[e];var a=n.length;var o=f(arguments);for(var s=0;s<a;s++){n[s].apply(null,o)}if(n.length>a){n.splice(0,a);r.nextTick(function(){t.apply(null,o)})}else{delete i[e]}})}function f(e){var t=e.length;var r=[];for(var n=0;n<t;n++)r[n]=e[n];return r}}).call(this,e("_process"))},{_process:112,once:104,wrappy:187}],57:[function(e,t,r){if(typeof Object.create==="function"){t.exports=function n(e,t){e.super_=t;e.prototype=Object.create(t.prototype,{constructor:{value:e,enumerable:false,writable:true,configurable:true}})}}else{t.exports=function i(e,t){e.super_=t;var r=function(){};r.prototype=t.prototype;e.prototype=new r;e.prototype.constructor=e}}},{}],58:[function(e,t,r){"use strict";var n=r;var i=e("buffer").Buffer;var a=e("os");n.toBuffer=function(e,t,r){r=~~r;var n;if(this.isV4Format(e)){n=t||new i(r+4);e.split(/\./g).map(function(e){n[r++]=parseInt(e,10)&255})}else if(this.isV6Format(e)){var a=e.split(":",8);var o;for(o=0;o<a.length;o++){var s=this.isV4Format(a[o]);var f;if(s){f=this.toBuffer(a[o]);a[o]=f.slice(0,2).toString("hex")}if(f&&++o<8){a.splice(o,0,f.slice(2,4).toString("hex"))}}if(a[0]===""){while(a.length<8)a.unshift("0")}else if(a[a.length-1]===""){while(a.length<8)a.push("0")}else if(a.length<8){for(o=0;o<a.length&&a[o]!=="";o++);var u=[o,1];for(o=9-a.length;o>0;o--){u.push("0")}a.splice.apply(a,u)}n=t||new i(r+16);for(o=0;o<a.length;o++){var c=parseInt(a[o],16);n[r++]=c>>8&255;n[r++]=c&255}}if(!n){throw Error("Invalid ip address: "+e)}return n};n.toString=function(e,t,r){t=~~t;r=r||e.length-t;var n=[];if(r===4){for(var i=0;i<r;i++){n.push(e[t+i])}n=n.join(".")}else if(r===16){for(var i=0;i<r;i+=2){n.push(e.readUInt16BE(t+i).toString(16))}n=n.join(":");n=n.replace(/(^|:)0(:0)*:0(:|$)/,"$1::$3");n=n.replace(/:{3,4}/,"::")}return n};var o=/^(\d{1,3}\.){3,3}\d{1,3}$/;var s=/^(::)?(((\d{1,3}\.){3}(\d{1,3}){1})?([0-9a-f]){0,4}:{0,2}){1,8}(::)?$/i;n.isV4Format=function(e){return o.test(e)};n.isV6Format=function(e){return s.test(e)};function f(e){return e?e.toLowerCase():"ipv4"}n.fromPrefixLen=function(e,t){if(e>32){t="ipv6"}else{t=f(t)}var r=4;if(t==="ipv6"){r=16}var a=new i(r);for(var o=0,s=a.length;o<s;++o){var u=8;if(e<8){u=e}e-=u;a[o]=~(255>>u)}return n.toString(a)};n.mask=function(e,t){e=n.toBuffer(e);t=n.toBuffer(t);var r=new i(Math.max(e.length,t.length));if(e.length===t.length){for(var a=0;a<e.length;a++){r[a]=e[a]&t[a]}}else if(t.length===4){for(var a=0;a<t.length;a++){r[a]=e[e.length-4+a]&t[a]}}else{for(var a=0;a<r.length-6;a++){r[a]=0}r[10]=255;r[11]=255;for(var a=0;a<e.length;a++){r[a+12]=e[a]&t[a+12]}}return n.toString(r)};n.cidr=function(e){var t=e.split("/");var r=t[0];if(t.length!==2)throw new Error("invalid CIDR subnet: "+r);var i=n.fromPrefixLen(parseInt(t[1],10));return n.mask(r,i)};n.subnet=function(e,t){var r=n.toLong(n.mask(e,t));var i=n.toBuffer(t);var a=0;for(var o=0;o<i.length;o++){if(i[o]===255){a+=8}else{var s=i[o]&255;while(s){s=s<<1&255;a++}}}var f=Math.pow(2,32-a);return{networkAddress:n.fromLong(r),firstAddress:f<=2?n.fromLong(r):n.fromLong(r+1),lastAddress:f<=2?n.fromLong(r+f-1):n.fromLong(r+f-2),broadcastAddress:n.fromLong(r+f-1),subnetMask:t,subnetMaskLength:a,numHosts:f<=2?f:f-2,length:f,contains:function(e){return r===n.toLong(n.mask(e,t))}}};n.cidrSubnet=function(e){var t=e.split("/");var r=t[0];if(t.length!==2)throw new Error("invalid CIDR subnet: "+r);var i=n.fromPrefixLen(parseInt(t[1],10));return n.subnet(r,i)};n.not=function(e){var t=n.toBuffer(e);for(var r=0;r<t.length;r++){t[r]=255^t[r]}return n.toString(t)};n.or=function(e,t){e=n.toBuffer(e);t=n.toBuffer(t);if(e.length===t.length){for(var r=0;r<e.length;++r){e[r]|=t[r]}return n.toString(e)}else{var i=e;var a=t;if(t.length>e.length){i=t;a=e}var o=i.length-a.length;for(var r=o;r<i.length;++r){i[r]|=a[r-o]}return n.toString(i)}};n.isEqual=function(e,t){e=n.toBuffer(e);t=n.toBuffer(t);if(e.length===t.length){for(var r=0;r<e.length;r++){if(e[r]!==t[r])return false}return true}if(t.length===4){var i=t;t=e;e=i}for(var r=0;r<10;r++){if(t[r]!==0)return false}var a=t.readUInt16BE(10);if(a!==0&&a!==65535)return false;for(var r=0;r<4;r++){if(e[r]!==t[r+12])return false}return true};n.isPrivate=function(e){return/^(::f{4}:)?10\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})$/i.test(e)||/^(::f{4}:)?192\.168\.([0-9]{1,3})\.([0-9]{1,3})$/i.test(e)||/^(::f{4}:)?172\.(1[6-9]|2\d|30|31)\.([0-9]{1,3})\.([0-9]{1,3})$/i.test(e)||/^(::f{4}:)?127\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})$/i.test(e)||/^(::f{4}:)?169\.254\.([0-9]{1,3})\.([0-9]{1,3})$/i.test(e)||/^f[cd][0-9a-f]{2}:/i.test(e)||/^fe80:/i.test(e)||/^::1$/.test(e)||/^::$/.test(e)};n.isPublic=function(e){return!n.isPrivate(e)};n.isLoopback=function(e){return/^(::f{4}:)?127\.([0-9]{1,3})\.([0-9]{1,3})\.([0-9]{1,3})/.test(e)||/^fe80::1$/.test(e)||/^::1$/.test(e)||/^::$/.test(e)};n.loopback=function(e){e=f(e);if(e!=="ipv4"&&e!=="ipv6"){throw new Error("family must be ipv4 or ipv6")}return e==="ipv4"?"127.0.0.1":"fe80::1"};n.address=function(e,t){var r=a.networkInterfaces();var i;t=f(t);if(e&&e!=="private"&&e!=="public"){var o=r[e].filter(function(e){var r=e.family.toLowerCase();return r===t});if(o.length===0)return undefined;return o[0].address}var i=Object.keys(r).map(function(i){var a=r[i].filter(function(r){r.family=r.family.toLowerCase();if(r.family!==t||n.isLoopback(r.address)){return false}else if(!e){return true}return e==="public"?n.isPrivate(r.address):n.isPublic(r.address)});return a.length?a[0].address:undefined}).filter(Boolean);return!i.length?n.loopback(t):i[0]};n.toLong=function(e){var t=0;e.split(".").forEach(function(e){t<<=8;t+=parseInt(e)});return t>>>0};n.fromLong=function(e){return(e>>>24)+"."+(e>>16&255)+"."+(e>>8&255)+"."+(e&255)}},{buffer:13,os:106}],59:[function(e,t,r){t.exports=function(e){return!!(e!=null&&(e._isBuffer||e.constructor&&typeof e.constructor.isBuffer==="function"&&e.constructor.isBuffer(e)))}},{}],60:[function(e,t,r){t.exports=function(e){if(e.charCodeAt(0)===46&&e.indexOf("/",1)===-1){return true}var t=e.lastIndexOf("/");return t!==-1?e.charCodeAt(t+1)===46:false}},{}],61:[function(e,t,r){"use strict";var n=e("is-primitive");t.exports=function i(e,t){if(!e&&!t){return true}if(!e&&t||e&&!t){return false}var r=0,i=0,a;for(a in t){i++;if(!n(t[a])||!e.hasOwnProperty(a)||e[a]!==t[a]){return false}}for(a in e){r++}return r===i}},{"is-primitive":67}],62:[function(e,t,r){"use strict";t.exports=function n(e){return typeof e!=="undefined"&&e!==null&&(typeof e==="object"||typeof e==="function")}},{}],63:[function(e,t,r){t.exports=function n(e){return typeof e==="string"&&/[@?!+*]\(/.test(e)}},{}],64:[function(e,t,r){var n=e("is-extglob");t.exports=function i(e){return typeof e==="string"&&(/[*!?{}(|)[\]]/.test(e)||n(e))}},{"is-extglob":63}],65:[function(e,t,r){"use strict";var n=e("kind-of");t.exports=function i(e){var t=n(e);if(t!=="number"&&t!=="string"){return false}var r=+e;return r-r+1>=0&&e!==""}},{"kind-of":76}],66:[function(e,t,r){t.exports=function n(e){return typeof e==="string"&&/\[([:.=+])(?:[^\[\]]|)+\1\]/.test(e)}},{}],67:[function(e,t,r){"use strict";t.exports=function n(e){return e==null||typeof e!=="function"&&typeof e!=="object"}},{}],68:[function(e,t,r){"use strict";var n=t.exports=function(e){return e!==null&&typeof e==="object"&&typeof e.pipe==="function"};n.writable=function(e){return n(e)&&e.writable!==false&&typeof e._write==="function"&&typeof e._writableState==="object"};n.readable=function(e){return n(e)&&e.readable!==false&&typeof e._read==="function"&&typeof e._readableState==="object"};n.duplex=function(e){return n.writable(e)&&n.readable(e)};n.transform=function(e){return n.duplex(e)&&typeof e._transform==="function"&&typeof e._transformState==="object"}},{}],69:[function(e,t,r){"use strict";t.exports=function i(e){if(typeof e==="string"&&e.length>0){return true}if(Array.isArray(e)){return e.length!==0&&n(e)}return false};function n(e){var t=e.length;while(t--){if(typeof e[t]!=="string"||e[t].length<=0){return false}}return true}},{}],70:[function(e,t,r){var n={}.toString;t.exports=Array.isArray||function(e){return n.call(e)=="[object Array]"}},{}],71:[function(e,t,r){"use strict";var n=e("isarray");t.exports=function i(e){return e!=null&&typeof e==="object"&&n(e)===false}},{isarray:70}],72:[function(e,t,r){var n=typeof JSON!=="undefined"?JSON:e("jsonify");t.exports=function(e,t){if(!t)t={};if(typeof t==="function")t={cmp:t};var r=t.space||"";if(typeof r==="number")r=Array(r+1).join(" ");var o=typeof t.cycles==="boolean"?t.cycles:false;var s=t.replacer||function(e,t){return t};var f=t.cmp&&function(e){return function(t){return function(r,n){var i={key:r,value:t[r]};var a={key:n,value:t[n]};return e(i,a)}}}(t.cmp);var u=[];return function c(e,t,l,h){var p=r?"\n"+new Array(h+1).join(r):"";var d=r?": ":":";if(l&&l.toJSON&&typeof l.toJSON==="function"){l=l.toJSON()}l=s.call(e,t,l);if(l===undefined){return}if(typeof l!=="object"||l===null){return n.stringify(l)}if(i(l)){var v=[];for(var g=0;g<l.length;g++){var y=c(l,g,l[g],h+1)||n.stringify(null);v.push(p+r+y)}return"["+v.join(",")+p+"]"}else{if(u.indexOf(l)!==-1){if(o)return n.stringify("__cycle__");throw new TypeError("Converting circular structure to JSON")}else u.push(l);var b=a(l).sort(f&&f(l));var v=[];for(var g=0;g<b.length;g++){var t=b[g];var m=c(l,t,l[t],h+1);if(!m)continue;var _=n.stringify(t)+d+m;v.push(p+r+_)}u.splice(u.indexOf(l),1);return"{"+v.join(",")+p+"}"}}({"":e},"",e,0)};var i=Array.isArray||function(e){return{}.toString.call(e)==="[object Array]"};var a=Object.keys||function(e){var t=Object.prototype.hasOwnProperty||function(){return true};var r=[];for(var n in e){if(t.call(e,n))r.push(n)}return r}},{jsonify:73}],73:[function(e,t,r){r.parse=e("./lib/parse");r.stringify=e("./lib/stringify")},{"./lib/parse":74,"./lib/stringify":75}],74:[function(e,t,r){var n,i,a={'"':'"',"\\":"\\","/":"/",b:"\b",f:"\f",n:"\n",r:"\r",t:"\t"},o,s=function(e){throw{name:"SyntaxError",message:e,at:n,text:o}},f=function(e){if(e&&e!==i){s("Expected '"+e+"' instead of '"+i+"'")}i=o.charAt(n);n+=1;return i},u=function(){var e,t="";if(i==="-"){t="-";f("-")}while(i>="0"&&i<="9"){t+=i;f()}if(i==="."){t+=".";while(f()&&i>="0"&&i<="9"){t+=i}}if(i==="e"||i==="E"){t+=i;f();if(i==="-"||i==="+"){t+=i;f()}while(i>="0"&&i<="9"){t+=i;f()}}e=+t;if(!isFinite(e)){s("Bad number")}else{return e}},c=function(){var e,t,r="",n;if(i==='"'){while(f()){if(i==='"'){f();return r}else if(i==="\\"){f();if(i==="u"){n=0;for(t=0;t<4;t+=1){e=parseInt(f(),16);if(!isFinite(e)){break}n=n*16+e}r+=String.fromCharCode(n)}else if(typeof a[i]==="string"){r+=a[i]}else{break}}else{r+=i}}}s("Bad string")},l=function(){while(i&&i<=" "){f()}},h=function(){switch(i){case"t":f("t");f("r");f("u");f("e");return true;case"f":f("f");f("a");f("l");f("s");f("e");return false;case"n":f("n");f("u");f("l");f("l");return null}s("Unexpected '"+i+"'")},p,d=function(){var e=[];if(i==="["){f("[");l();if(i==="]"){f("]");return e}while(i){e.push(p());l();if(i==="]"){f("]");return e}f(",");l()}}s("Bad array")},v=function(){var e,t={};if(i==="{"){f("{");l();if(i==="}"){f("}");return t}while(i){e=c();l();f(":");if(Object.hasOwnProperty.call(t,e)){s('Duplicate key "'+e+'"')}t[e]=p();l();if(i==="}"){f("}");return t}f(",");l()}}s("Bad object")};p=function(){l();switch(i){case"{":return v();case"[":return d();case'"':return c();case"-":return u();default:return i>="0"&&i<="9"?u():h()}};t.exports=function(e,t){var r;o=e;n=0;i=" ";r=p();l();if(i){s("Syntax error")}return typeof t==="function"?function a(e,r){var n,i,o=e[r];if(o&&typeof o==="object"){for(n in o){if(Object.prototype.hasOwnProperty.call(o,n)){i=a(o,n);if(i!==undefined){o[n]=i}else{delete o[n]}}}}return t.call(e,r,o)}({"":r},""):r}},{}],75:[function(e,t,r){var n=/[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,i=/[\\\"\x00-\x1f\x7f-\x9f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,a,o,s={"\b":"\\b","\t":"\\t","\n":"\\n","\f":"\\f","\r":"\\r",'"':'\\"',"\\":"\\\\"},f;function u(e){i.lastIndex=0;return i.test(e)?'"'+e.replace(i,function(e){var t=s[e];return typeof t==="string"?t:"\\u"+("0000"+e.charCodeAt(0).toString(16)).slice(-4)})+'"':'"'+e+'"'}function c(e,t){var r,n,i,s,l=a,h,p=t[e];if(p&&typeof p==="object"&&typeof p.toJSON==="function"){p=p.toJSON(e)}if(typeof f==="function"){p=f.call(t,e,p)}switch(typeof p){case"string":return u(p);case"number":return isFinite(p)?String(p):"null";case"boolean":case"null":return String(p);case"object":if(!p)return"null";a+=o;h=[];if(Object.prototype.toString.apply(p)==="[object Array]"){s=p.length;for(r=0;r<s;r+=1){h[r]=c(r,p)||"null"}i=h.length===0?"[]":a?"[\n"+a+h.join(",\n"+a)+"\n"+l+"]":"["+h.join(",")+"]";a=l;return i}if(f&&typeof f==="object"){s=f.length;for(r=0;r<s;r+=1){n=f[r];if(typeof n==="string"){i=c(n,p);if(i){h.push(u(n)+(a?": ":":")+i)}}}}else{for(n in p){if(Object.prototype.hasOwnProperty.call(p,n)){i=c(n,p);if(i){h.push(u(n)+(a?": ":":")+i)}}}}i=h.length===0?"{}":a?"{\n"+a+h.join(",\n"+a)+"\n"+l+"}":"{"+h.join(",")+"}";a=l;return i}}t.exports=function(e,t,r){var n;a="";o="";if(typeof r==="number"){for(n=0;n<r;n+=1){o+=" "}}else if(typeof r==="string"){o=r}f=t;if(t&&typeof t!=="function"&&(typeof t!=="object"||typeof t.length!=="number")){throw new Error("JSON.stringify")}return c("",{"":e})}},{}],76:[function(e,t,r){(function(r){var n=e("is-buffer");var i=Object.prototype.toString;t.exports=function a(e){if(typeof e==="undefined"){return"undefined"}if(e===null){return"null"}if(e===true||e===false||e instanceof Boolean){return"boolean"}if(typeof e==="string"||e instanceof String){return"string"}if(typeof e==="number"||e instanceof Number){return"number"}if(typeof e==="function"||e instanceof Function){return"function"}if(typeof Array.isArray!=="undefined"&&Array.isArray(e)){return"array"}if(e instanceof RegExp){return"regexp"}if(e instanceof Date){return"date"}var t=i.call(e);if(t==="[object RegExp]"){return"regexp"}if(t==="[object Date]"){return"date"}if(t==="[object Arguments]"){return"arguments"}if(typeof r!=="undefined"&&n(e)){return"buffer"}if(t==="[object Set]"){return"set"}if(t==="[object WeakSet]"){return"weakset"}if(t==="[object Map]"){return"map"}if(t==="[object WeakMap]"){return"weakmap"}if(t==="[object Symbol]"){return"symbol"}if(t==="[object Int8Array]"){return"int8array"}if(t==="[object Uint8Array]"){return"uint8array"}if(t==="[object Uint8ClampedArray]"){return"uint8clampedarray"}if(t==="[object Int16Array]"){return"int16array"}if(t==="[object Uint16Array]"){return"uint16array"}if(t==="[object Int32Array]"){return"int32array"}if(t==="[object Uint32Array]"){return"uint32array"}if(t==="[object Float32Array]"){return"float32array"}if(t==="[object Float64Array]"){return"float64array"}return"object"}}).call(this,e("buffer").Buffer)},{buffer:13,"is-buffer":59}],77:[function(e,t,r){var n=9007199254740991;var i="[object Arguments]",a="[object Function]",o="[object GeneratorFunction]",s="[object String]";var f=/^(?:0|[1-9]\d*)$/;function u(e,t){var r=-1,n=Array(e);while(++r<e){n[r]=t(r)}return n}var c=Object.prototype;var l=c.hasOwnProperty;var h=c.toString;var p=c.propertyIsEnumerable;var d=Object.getPrototypeOf,v=Object.keys;var g=E(b);var y=S();function b(e,t){return e&&y(e,t,D)}function m(e,t){return l.call(e,t)||typeof e=="object"&&t in e&&O(e)===null}function _(e){return v(Object(e))}function w(e){return function(t){return t==null?undefined:t[e]}}function E(e,t){return function(r,n){if(r==null){return r}if(!L(r)){return e(r,n)}var i=r.length,a=t?i:-1,o=Object(r);while(t?a--:++a<i){if(n(o[a],a,o)===false){break}}return r}}function S(e){return function(t,r,n){var i=-1,a=Object(t),o=n(t),s=o.length;while(s--){var f=o[e?s:++i];if(r(a[f],f,a)===false){break}}return t}}var x=w("length");function O(e){return d(Object(e))}function j(e){var t=e?e.length:undefined;if(P(t)&&(T(e)||B(e)||R(e))){return u(t,String)}return null}function A(e,t){t=t==null?n:t;return!!t&&(typeof e=="number"||f.test(e))&&(e>-1&&e%1==0&&e<t)}function k(e){var t=e&&e.constructor,r=typeof t=="function"&&t.prototype||c;return e===r}function R(e){return I(e)&&l.call(e,"callee")&&(!p.call(e,"callee")||h.call(e)==i)}var T=Array.isArray;function L(e){return e!=null&&P(x(e))&&!C(e)}function I(e){return N(e)&&L(e)}function C(e){var t=M(e)?h.call(e):"";return t==a||t==o}function P(e){return typeof e=="number"&&e>-1&&e%1==0&&e<=n}function M(e){var t=typeof e;return!!e&&(t=="object"||t=="function")}function N(e){return!!e&&typeof e=="object"}function B(e){return typeof e=="string"||!T(e)&&N(e)&&h.call(e)==s}function D(e){var t=k(e);if(!(t||L(e))){return _(e)}var r=j(e),n=!!r,i=r||[],a=i.length;for(var o in e){if(m(e,o)&&!(n&&(o=="length"||A(o,a)))&&!(t&&o=="constructor")){i.push(o)}}return i}t.exports=g},{}],78:[function(e,t,r){var n=e("lodash._baseeach");function i(e,t){var r=[];n(e,function(e,n,i){if(t(e,n,i)){r.push(e)}});return r}t.exports=i},{"lodash._baseeach":77}],79:[function(e,t,r){(function(n){var i=e("lodash._stringtopath");var a=200;var o="__lodash_hash_undefined__";var s=1,f=2;var u=1/0,c=9007199254740991;var l="[object Arguments]",h="[object Array]",p="[object Boolean]",d="[object Date]",v="[object Error]",g="[object Function]",y="[object GeneratorFunction]",b="[object Map]",m="[object Number]",_="[object Object]",w="[object Promise]",E="[object RegExp]",S="[object Set]",x="[object String]",O="[object Symbol]",j="[object WeakMap]";var A="[object ArrayBuffer]",k="[object DataView]",R="[object Float32Array]",T="[object Float64Array]",L="[object Int8Array]",I="[object Int16Array]",C="[object Int32Array]",P="[object Uint8Array]",M="[object Uint8ClampedArray]",N="[object Uint16Array]",B="[object Uint32Array]";var D=/\.|\[(?:[^[\]]*|(["'])(?:(?!\1)[^\\]|\\.)*?\1)\]/,U=/^\w*$/;var F=/[\\^$.*+?()[\]{}|]/g;var q=/^\[object .+?Constructor\]$/;var G=/^(?:0|[1-9]\d*)$/;var $={};$[R]=$[T]=$[L]=$[I]=$[C]=$[P]=$[M]=$[N]=$[B]=true;$[l]=$[h]=$[A]=$[p]=$[k]=$[d]=$[v]=$[g]=$[b]=$[m]=$[_]=$[E]=$[S]=$[x]=$[j]=false;var W={"function":true,object:true};var H=W[typeof r]&&r&&!r.nodeType?r:undefined;var Y=W[typeof t]&&t&&!t.nodeType?t:undefined;var z=re(H&&Y&&typeof n=="object"&&n);var K=re(W[typeof self]&&self);var X=re(W[typeof window]&&window);var V=re(W[typeof this]&&this);var Q=z||X!==(V&&V.window)&&X||K||V||Function("return this")();function J(e,t){var r=-1,n=e.length,i=Array(n);while(++r<n){i[r]=t(e[r],r,e)}return i}function Z(e,t){var r=-1,n=e.length;while(++r<n){if(t(e[r],r,e)){return true}}return false}function ee(e,t){var r=-1,n=Array(e);while(++r<e){n[r]=t(r)}return n}function te(e,t){return J(t,function(t){return[t,e[t]]})}function re(e){return e&&e.Object===Object?e:null}function ne(e){var t=false;if(e!=null&&typeof e.toString!="function"){try{t=!!(e+"")}catch(r){}}return t}function ie(e){var t=-1,r=Array(e.size);e.forEach(function(e,n){r[++t]=[n,e]});return r}function ae(e){var t=-1,r=Array(e.size);e.forEach(function(e){r[++t]=e});return r}function oe(e){var t=-1,r=Array(e.size);e.forEach(function(e){r[++t]=[e,e]});return r}var se=Array.prototype,fe=Object.prototype;var ue=Function.prototype.toString;var ce=fe.hasOwnProperty;var le=fe.toString;var he=RegExp("^"+ue.call(ce).replace(F,"\\$&").replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g,"$1.*?")+"$");var pe=Q.Symbol,de=Q.Uint8Array,ve=fe.propertyIsEnumerable,ge=se.splice;var ye=Object.getPrototypeOf,be=Object.keys;var me=jt(Q,"DataView"),_e=jt(Q,"Map"),we=jt(Q,"Promise"),Ee=jt(Q,"Set"),Se=jt(Q,"WeakMap"),xe=jt(Object,"create");var Oe=Dt(me),je=Dt(_e),Ae=Dt(we),ke=Dt(Ee),Re=Dt(Se);var Te=pe?pe.prototype:undefined,Le=Te?Te.valueOf:undefined;function Ie(e){var t=-1,r=e?e.length:0;this.clear();while(++t<r){var n=e[t];this.set(n[0],n[1])}}function Ce(){this.__data__=xe?xe(null):{}}function Pe(e){return this.has(e)&&delete this.__data__[e]}function Me(e){var t=this.__data__;if(xe){var r=t[e];return r===o?undefined:r}return ce.call(t,e)?t[e]:undefined}function Ne(e){var t=this.__data__;return xe?t[e]!==undefined:ce.call(t,e)}function Be(e,t){var r=this.__data__;r[e]=xe&&t===undefined?o:t;return this}Ie.prototype.clear=Ce;Ie.prototype["delete"]=Pe;Ie.prototype.get=Me;Ie.prototype.has=Ne;Ie.prototype.set=Be;function De(e){var t=-1,r=e?e.length:0;this.clear();while(++t<r){var n=e[t];this.set(n[0],n[1])}}function Ue(){this.__data__=[]}function Fe(e){var t=this.__data__,r=at(t,e);if(r<0){return false}var n=t.length-1;if(r==n){t.pop()}else{ge.call(t,r,1)}return true}function qe(e){var t=this.__data__,r=at(t,e);return r<0?undefined:t[r][1]}function Ge(e){return at(this.__data__,e)>-1}function $e(e,t){var r=this.__data__,n=at(r,e);if(n<0){r.push([e,t])}else{r[n][1]=t}return this}De.prototype.clear=Ue;De.prototype["delete"]=Fe;De.prototype.get=qe;De.prototype.has=Ge;De.prototype.set=$e;function We(e){var t=-1,r=e?e.length:0;this.clear();while(++t<r){var n=e[t];this.set(n[0],n[1])}}function He(){this.__data__={hash:new Ie,map:new(_e||De),string:new Ie}}function Ye(e){return xt(this,e)["delete"](e)}function ze(e){return xt(this,e).get(e)}function Ke(e){return xt(this,e).has(e)}function Xe(e,t){xt(this,e).set(e,t);return this}We.prototype.clear=He;We.prototype["delete"]=Ye;We.prototype.get=ze;We.prototype.has=Ke;We.prototype.set=Xe;function Ve(e){var t=-1,r=e?e.length:0;this.__data__=new We;while(++t<r){this.add(e[t])}}function Qe(e){this.__data__.set(e,o);return this}function Je(e){return this.__data__.has(e)}Ve.prototype.add=Ve.prototype.push=Qe;Ve.prototype.has=Je;function Ze(e){this.__data__=new De(e)}function et(){this.__data__=new De}function tt(e){return this.__data__["delete"](e)}function rt(e){return this.__data__.get(e)}function nt(e){return this.__data__.has(e)}function it(e,t){var r=this.__data__;if(r instanceof De&&r.__data__.length==a){r=this.__data__=new We(r.__data__)}r.set(e,t);return this}Ze.prototype.clear=et;Ze.prototype["delete"]=tt;Ze.prototype.get=rt;Ze.prototype.has=nt;Ze.prototype.set=it;function at(e,t){var r=e.length;while(r--){if(Ut(e[r][0],t)){return r}}return-1}function ot(e,t){t=It(t,e)?[t]:bt(t);var r=0,n=t.length;while(e!=null&&r<n){e=e[Bt(t[r++])]}return r&&r==n?e:undefined}function st(e,t){return ce.call(e,t)||typeof e=="object"&&t in e&&At(e)===null}function ft(e,t){return t in Object(e)}function ut(e,t,r,n,i){if(e===t){return true}if(e==null||t==null||!Yt(e)&&!zt(t)){return e!==e&&t!==t}return ct(e,t,ut,r,n,i)}function ct(e,t,r,n,i,a){var o=qt(e),s=qt(t),u=h,c=h;if(!o){u=kt(e);u=u==l?_:u}if(!s){c=kt(t);c=c==l?_:c}var p=u==_&&!ne(e),d=c==_&&!ne(t),v=u==c;if(v&&!p){a||(a=new Ze);return o||Qt(e)?_t(e,t,r,n,i,a):wt(e,t,u,r,n,i,a)}if(!(i&f)){var g=p&&ce.call(e,"__wrapped__"),y=d&&ce.call(t,"__wrapped__");if(g||y){var b=g?e.value():e,m=y?t.value():t;a||(a=new Ze);return r(b,m,n,i,a)}}if(!v){return false}a||(a=new Ze);return Et(e,t,r,n,i,a)}function lt(e,t,r,n){var i=r.length,a=i,o=!n;if(e==null){return!a}e=Object(e);while(i--){var u=r[i];if(o&&u[2]?u[1]!==e[u[0]]:!(u[0]in e)){return false}}while(++i<a){u=r[i];var c=u[0],l=e[c],h=u[1];if(o&&u[2]){if(l===undefined&&!(c in e)){return false}}else{var p=new Ze;if(n){var d=n(l,h,c,e,t,p)}if(!(d===undefined?ut(h,l,n,s|f,p):d)){return false}}}return true}function ht(e){if(typeof e=="function"){return e}if(e==null){return rr}if(typeof e=="object"){return qt(e)?vt(e[0],e[1]):dt(e)}return nr(e)}function pt(e){return be(Object(e))}function dt(e){var t=Ot(e);if(t.length==1&&t[0][2]){return Nt(t[0][0],t[0][1])}return function(r){return r===e||lt(r,e,t)}}function vt(e,t){if(It(e)&&Mt(t)){return Nt(Bt(e),t)}return function(r){var n=Jt(r,e);return n===undefined&&n===t?Zt(r,e):ut(t,n,undefined,s|f)}}function gt(e){return function(t){return t==null?undefined:t[e]}}function yt(e){return function(t){return ot(t,e)}}function bt(e){return qt(e)?e:i(e)}function mt(e){return function(t){var r=kt(t);if(r==b){return ie(t)}if(r==S){return oe(t)}return te(t,e(t))}}function _t(e,t,r,n,i,a){var o=i&f,u=e.length,c=t.length;if(u!=c&&!(o&&c>u)){return false}var l=a.get(e);if(l){return l==t}var h=-1,p=true,d=i&s?new Ve:undefined;a.set(e,t);while(++h<u){var v=e[h],g=t[h];if(n){var y=o?n(g,v,h,t,e,a):n(v,g,h,e,t,a)}if(y!==undefined){if(y){continue}p=false;break}if(d){if(!Z(t,function(e,t){if(!d.has(t)&&(v===e||r(v,e,n,i,a))){return d.add(t)}})){p=false;break}}else if(!(v===g||r(v,g,n,i,a))){p=false;break}}a["delete"](e);return p}function wt(e,t,r,n,i,a,o){switch(r){case k:if(e.byteLength!=t.byteLength||e.byteOffset!=t.byteOffset){return false}e=e.buffer;t=t.buffer;case A:if(e.byteLength!=t.byteLength||!n(new de(e),new de(t))){return false}return true;case p:case d:return+e==+t;case v:return e.name==t.name&&e.message==t.message;case m:return e!=+e?t!=+t:e==+t;case E:case x:return e==t+"";case b:var u=ie;case S:var c=a&f;u||(u=ae);if(e.size!=t.size&&!c){return false}var l=o.get(e);if(l){return l==t}a|=s;o.set(e,t);return _t(u(e),u(t),n,i,a,o);case O:if(Le){return Le.call(e)==Le.call(t)}}return false}function Et(e,t,r,n,i,a){var o=i&f,s=er(e),u=s.length,c=er(t),l=c.length;if(u!=l&&!o){return false}var h=u;while(h--){var p=s[h];if(!(o?p in t:st(t,p))){return false}}var d=a.get(e);if(d){return d==t}var v=true;a.set(e,t);var g=o;while(++h<u){p=s[h];var y=e[p],b=t[p];if(n){var m=o?n(b,y,p,t,e,a):n(y,b,p,e,t,a)}if(!(m===undefined?y===b||r(y,b,n,i,a):m)){v=false;break}g||(g=p=="constructor")}if(v&&!g){var _=e.constructor,w=t.constructor;if(_!=w&&("constructor"in e&&"constructor"in t)&&!(typeof _=="function"&&_ instanceof _&&typeof w=="function"&&w instanceof w)){v=false}}a["delete"](e);return v}var St=gt("length");function xt(e,t){var r=e.__data__;return Ct(t)?r[typeof t=="string"?"string":"hash"]:r.map}function Ot(e){var t=tr(e),r=t.length;while(r--){t[r][2]=Mt(t[r][1])}return t}function jt(e,t){var r=e[t];return Kt(r)?r:undefined}function At(e){return ye(Object(e))}function kt(e){return le.call(e)}if(me&&kt(new me(new ArrayBuffer(1)))!=k||_e&&kt(new _e)!=b||we&&kt(we.resolve())!=w||Ee&&kt(new Ee)!=S||Se&&kt(new Se)!=j){kt=function(e){var t=le.call(e),r=t==_?e.constructor:undefined,n=r?Dt(r):undefined;if(n){switch(n){case Oe:return k;case je:return b;case Ae:return w;case ke:return S;case Re:return j}}return t}}function Rt(e,t,r){t=It(t,e)?[t]:bt(t);var n,i=-1,a=t.length;while(++i<a){var o=Bt(t[i]);if(!(n=e!=null&&r(e,o))){break}e=e[o]}if(n){return n}var a=e?e.length:0;return!!a&&Ht(a)&&Lt(o,a)&&(qt(e)||Xt(e)||Ft(e))}function Tt(e){var t=e?e.length:undefined;if(Ht(t)&&(qt(e)||Xt(e)||Ft(e))){return ee(t,String)}return null}function Lt(e,t){t=t==null?c:t;return!!t&&(typeof e=="number"||G.test(e))&&(e>-1&&e%1==0&&e<t)}function It(e,t){if(qt(e)){return false}var r=typeof e;if(r=="number"||r=="symbol"||r=="boolean"||e==null||Vt(e)){return true}return U.test(e)||!D.test(e)||t!=null&&e in Object(t)}function Ct(e){var t=typeof e;return t=="string"||t=="number"||t=="symbol"||t=="boolean"?e!=="__proto__":e===null}function Pt(e){var t=e&&e.constructor,r=typeof t=="function"&&t.prototype||fe;return e===r}function Mt(e){return e===e&&!Yt(e)}function Nt(e,t){return function(r){if(r==null){return false}return r[e]===t&&(t!==undefined||e in Object(r))}}function Bt(e){if(typeof e=="string"||Vt(e)){return e}var t=e+"";return t=="0"&&1/e==-u?"-0":t}function Dt(e){if(e!=null){try{return ue.call(e)}catch(t){}try{return e+""}catch(t){}}return""}function Ut(e,t){return e===t||e!==e&&t!==t}function Ft(e){return $t(e)&&ce.call(e,"callee")&&(!ve.call(e,"callee")||le.call(e)==l)}var qt=Array.isArray;function Gt(e){return e!=null&&Ht(St(e))&&!Wt(e)}function $t(e){return zt(e)&&Gt(e)}function Wt(e){var t=Yt(e)?le.call(e):"";return t==g||t==y}function Ht(e){return typeof e=="number"&&e>-1&&e%1==0&&e<=c}function Yt(e){var t=typeof e;return!!e&&(t=="object"||t=="function")}function zt(e){return!!e&&typeof e=="object"}function Kt(e){if(!Yt(e)){return false}var t=Wt(e)||ne(e)?he:q;return t.test(Dt(e))}function Xt(e){return typeof e=="string"||!qt(e)&&zt(e)&&le.call(e)==x}function Vt(e){return typeof e=="symbol"||zt(e)&&le.call(e)==O}function Qt(e){return zt(e)&&Ht(e.length)&&!!$[le.call(e)]}function Jt(e,t,r){var n=e==null?undefined:ot(e,t);return n===undefined?r:n}function Zt(e,t){return e!=null&&Rt(e,t,ft)}function er(e){var t=Pt(e);if(!(t||Gt(e))){return pt(e)}var r=Tt(e),n=!!r,i=r||[],a=i.length;for(var o in e){if(st(e,o)&&!(n&&(o=="length"||Lt(o,a)))&&!(t&&o=="constructor")){i.push(o)}}return i}var tr=mt(er);function rr(e){return e}function nr(e){return It(e)?gt(Bt(e)):yt(e)}t.exports=ht}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{"lodash._stringtopath":81}],80:[function(e,t,r){(function(e){var n=1/0;var i="[object Symbol]";var a={"function":true,object:true};var o=a[typeof r]&&r&&!r.nodeType?r:undefined;var s=a[typeof t]&&t&&!t.nodeType?t:undefined;var f=p(o&&s&&typeof e=="object"&&e);var u=p(a[typeof self]&&self);var c=p(a[typeof window]&&window);var l=p(a[typeof this]&&this);var h=f||c!==(l&&l.window)&&c||u||l||Function("return this")();function p(e){return e&&e.Object===Object?e:null}var d=Object.prototype;var v=d.toString;var g=h.Symbol;var y=g?g.prototype:undefined,b=y?y.toString:undefined;function m(e){if(typeof e=="string"){return e}if(w(e)){return b?b.call(e):""}var t=e+"";return t=="0"&&1/e==-n?"-0":t}function _(e){return!!e&&typeof e=="object"}function w(e){return typeof e=="symbol"||_(e)&&v.call(e)==i}t.exports=m}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{}],81:[function(e,t,r){(function(n){var i=e("lodash._basetostring");var a="Expected a function";var o="__lodash_hash_undefined__";var s="[object Function]",f="[object GeneratorFunction]";var u=/[^.[\]]+|\[(?:(-?\d+(?:\.\d+)?)|(["'])((?:(?!\2)[^\\]|\\.)*?)\2)\]/g;var c=/[\\^$.*+?()[\]{}|]/g;var l=/\\(\\)?/g;var h=/^\[object .+?Constructor\]$/;var p={"function":true,object:true};var d=p[typeof r]&&r&&!r.nodeType?r:undefined;var v=p[typeof t]&&t&&!t.nodeType?t:undefined;var g=w(d&&v&&typeof n=="object"&&n);var y=w(p[typeof self]&&self);var b=w(p[typeof window]&&window);var m=w(p[typeof this]&&this);var _=g||b!==(m&&m.window)&&b||y||m||Function("return this")();function w(e){return e&&e.Object===Object?e:null}function E(e){var t=false;if(e!=null&&typeof e.toString!="function"){try{t=!!(e+"")}catch(r){}}return t}var S=Array.prototype,x=Object.prototype;var O=Function.prototype.toString;var j=x.hasOwnProperty;var A=x.toString;var k=RegExp("^"+O.call(j).replace(c,"\\$&").replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g,"$1.*?")+"$");var R=S.splice;var T=J(_,"Map"),L=J(Object,"create");function I(e){var t=-1,r=e?e.length:0;this.clear();while(++t<r){var n=e[t];this.set(n[0],n[1])}}function C(){this.__data__=L?L(null):{}}function P(e){return this.has(e)&&delete this.__data__[e]}function M(e){var t=this.__data__;if(L){var r=t[e];return r===o?undefined:r}return j.call(t,e)?t[e]:undefined}function N(e){var t=this.__data__;return L?t[e]!==undefined:j.call(t,e)}function B(e,t){var r=this.__data__;r[e]=L&&t===undefined?o:t;return this}I.prototype.clear=C;I.prototype["delete"]=P;I.prototype.get=M;I.prototype.has=N;I.prototype.set=B;function D(e){var t=-1,r=e?e.length:0;this.clear();while(++t<r){var n=e[t];this.set(n[0],n[1])}}function U(){this.__data__=[]}function F(e){var t=this.__data__,r=V(t,e);if(r<0){
return false}var n=t.length-1;if(r==n){t.pop()}else{R.call(t,r,1)}return true}function q(e){var t=this.__data__,r=V(t,e);return r<0?undefined:t[r][1]}function G(e){return V(this.__data__,e)>-1}function $(e,t){var r=this.__data__,n=V(r,e);if(n<0){r.push([e,t])}else{r[n][1]=t}return this}D.prototype.clear=U;D.prototype["delete"]=F;D.prototype.get=q;D.prototype.has=G;D.prototype.set=$;function W(e){var t=-1,r=e?e.length:0;this.clear();while(++t<r){var n=e[t];this.set(n[0],n[1])}}function H(){this.__data__={hash:new I,map:new(T||D),string:new I}}function Y(e){return Q(this,e)["delete"](e)}function z(e){return Q(this,e).get(e)}function K(e){return Q(this,e).has(e)}function X(e,t){Q(this,e).set(e,t);return this}W.prototype.clear=H;W.prototype["delete"]=Y;W.prototype.get=z;W.prototype.has=K;W.prototype.set=X;function V(e,t){var r=e.length;while(r--){if(ne(e[r][0],t)){return r}}return-1}function Q(e,t){var r=e.__data__;return Z(t)?r[typeof t=="string"?"string":"hash"]:r.map}function J(e,t){var r=e[t];return oe(r)?r:undefined}function Z(e){var t=typeof e;return t=="string"||t=="number"||t=="symbol"||t=="boolean"?e!=="__proto__":e===null}var ee=re(function(e){var t=[];se(e).replace(u,function(e,r,n,i){t.push(n?i.replace(l,"$1"):r||e)});return t});function te(e){if(e!=null){try{return O.call(e)}catch(t){}try{return e+""}catch(t){}}return""}function re(e,t){if(typeof e!="function"||t&&typeof t!="function"){throw new TypeError(a)}var r=function(){var n=arguments,i=t?t.apply(this,n):n[0],a=r.cache;if(a.has(i)){return a.get(i)}var o=e.apply(this,n);r.cache=a.set(i,o);return o};r.cache=new(re.Cache||W);return r}re.Cache=W;function ne(e,t){return e===t||e!==e&&t!==t}function ie(e){var t=ae(e)?A.call(e):"";return t==s||t==f}function ae(e){var t=typeof e;return!!e&&(t=="object"||t=="function")}function oe(e){if(!ae(e)){return false}var t=ie(e)||E(e)?k:h;return t.test(te(e))}function se(e){return e==null?"":i(e)}t.exports=ee}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{"lodash._basetostring":80}],82:[function(e,t,r){var n=e("lodash._basefilter"),i=e("lodash._baseiteratee");function a(e,t){var r=-1,n=e.length,i=0,a=[];while(++r<n){var o=e[r];if(t(o,r,e)){a[i++]=o}}return a}function o(e,t){var r=s(e)?a:n;return r(e,i(t,3))}var s=Array.isArray;t.exports=o},{"lodash._basefilter":78,"lodash._baseiteratee":79}],83:[function(e,t,r){var n=e("lodash._baseeach"),i=e("lodash._baseiteratee");var a=9007199254740991;var o="[object Function]",s="[object GeneratorFunction]";function f(e,t){var r=-1,n=e.length,i=Array(n);while(++r<n){i[r]=t(e[r],r,e)}return i}var u=Object.prototype;var c=u.toString;function l(e,t){var r=-1,i=g(e)?Array(e.length):[];n(e,function(e,n,a){i[++r]=t(e,n,a)});return i}function h(e){return function(t){return t==null?undefined:t[e]}}var p=h("length");function d(e,t){var r=v(e)?f:l;return r(e,i(t,3))}var v=Array.isArray;function g(e){return e!=null&&b(p(e))&&!y(e)}function y(e){var t=m(e)?c.call(e):"";return t==o||t==s}function b(e){return typeof e=="number"&&e>-1&&e%1==0&&e<=a}function m(e){var t=typeof e;return!!e&&(t=="object"||t=="function")}t.exports=d},{"lodash._baseeach":77,"lodash._baseiteratee":79}],84:[function(t,r,n){(function(t){(function(i,a){var o=typeof n=="object"&&n;var s=typeof r=="object"&&r&&r.exports==o&&r;var f=typeof t=="object"&&t;if(f.global===f){i=f}var u=[],c={};var l=0;var h=c;var p=30;var d=i._;var v=/&(?:amp|lt|gt|quot|#39);/g;var g=/\b__p \+= '';/g,y=/\b(__p \+=) '' \+/g,b=/(__e\(.*?\)|\b__t\)) \+\n'';/g;var m=/\w*$/;var _=RegExp("^"+(c.valueOf+"").replace(/[.*+?^${}()|[\]\\]/g,"\\$&").replace(/valueOf|for [^\]]+/g,".+?")+"$");var w=/\$\{([^\\}]*(?:\\.[^\\}]*)*)\}/g;var E=/<%=([\s\S]+?)%>/g;var S=/($^)/;var x=/[&<>"']/g;var O=/['\n\r\t\u2028\u2029\\]/g;var j=0;var A=Math.ceil,k=u.concat,R=Math.floor,T=_.test(T=Object.getPrototypeOf)&&T,L=c.hasOwnProperty,I=u.push,C=c.toString;var P=_.test(P=we.bind)&&P,M=_.test(M=Array.isArray)&&M,N=i.isFinite,B=i.isNaN,D=_.test(D=Object.keys)&&D,U=Math.max,F=Math.min,q=Math.random;var G="[object Arguments]",$="[object Array]",W="[object Boolean]",H="[object Date]",Y="[object Function]",z="[object Number]",K="[object Object]",X="[object RegExp]",V="[object String]";var Q=!!i.attachEvent,J=P&&!/\n|true/.test(P+Q);var Z=P&&!J;var ee=D&&(Q||J);var te={};te[Y]=false;te[G]=te[$]=te[W]=te[H]=te[z]=te[K]=te[X]=te[V]=true;var re={};re[$]=Array;re[W]=Boolean;re[H]=Date;re[K]=Object;re[z]=Number;re[X]=RegExp;re[V]=String;var ne={"boolean":false,"function":true,object:true,number:false,string:false,undefined:false};var ie={"\\":"\\","'":"'","\n":"n","\r":"r","\t":"t","\u2028":"u2028","\u2029":"u2029"};function ae(e){if(e&&typeof e=="object"&&e.__wrapped__){return e}if(!(this instanceof ae)){return new ae(e)}this.__wrapped__=e}ae.templateSettings={escape:/<%-([\s\S]+?)%>/g,evaluate:/<%([\s\S]+?)%>/g,interpolate:E,variable:"",imports:{_:ae}};var oe=function(e){var t="var index, iterable = "+e.firstArg+", result = iterable;\nif (!iterable) return result;\n"+e.top+";\n";if(e.arrays){t+="var length = iterable.length; index = -1;\nif ("+e.arrays+") {\n  while (++index < length) {\n    "+e.loop+"\n  }\n}\nelse {  "}if(e.isKeysFast&&e.useHas){t+="\n  var ownIndex = -1,\n      ownProps = objectTypes[typeof iterable] ? nativeKeys(iterable) : [],\n      length = ownProps.length;\n\n  while (++ownIndex < length) {\n    index = ownProps[ownIndex];\n    "+e.loop+"\n  }  "}else{t+="\n  for (index in iterable) {";if(e.useHas){t+="\n    if (";if(e.useHas){t+="hasOwnProperty.call(iterable, index)"}t+=") {    "}t+=e.loop+";    ";if(e.useHas){t+="\n    }"}t+="\n  }  "}if(e.arrays){t+="\n}"}t+=e.bottom+";\nreturn result";return t};var se={args:"object, source, guard",top:"var args = arguments,\n"+"    argsIndex = 0,\n"+"    argsLength = typeof guard == 'number' ? 2 : args.length;\n"+"while (++argsIndex < argsLength) {\n"+"  iterable = args[argsIndex];\n"+"  if (iterable && objectTypes[typeof iterable]) {",loop:"if (typeof result[index] == 'undefined') result[index] = iterable[index]",bottom:"  }\n}"};var fe={args:"collection, callback, thisArg",top:"callback = callback && typeof thisArg == 'undefined' ? callback : createCallback(callback, thisArg)",arrays:"typeof length == 'number'",loop:"if (callback(iterable[index], index, collection) === false) return result"};var ue={top:"if (!objectTypes[typeof iterable]) return result;\n"+fe.top,arrays:false};function ce(e,t,r){t||(t=0);var n=e.length,i=n-t>=(r||p);if(i){var a={},o=t-1;while(++o<n){var s=e[o]+"";(L.call(a,s)?a[s]:a[s]=[]).push(e[o])}}return function(r){if(i){var n=r+"";return L.call(a,n)&&Lt(a[n],r)>-1}return Lt(e,r,t)>-1}}function le(e){return e.charCodeAt(0)}function he(e,t){var r=e.index,n=t.index;e=e.criteria;t=t.criteria;if(e!==t){if(e>t||typeof e=="undefined"){return 1}if(e<t||typeof t=="undefined"){return-1}}return r<n?-1:1}function pe(e,t,r,n){var i=He(e),a=!r,o=t;if(a){r=t}if(!i){t=e}function s(){var f=arguments,u=a?this:t;if(!i){e=t[o]}if(r.length){f=f.length?(f=we(f),n?f.concat(r):r.concat(f)):r}if(this instanceof s){_e.prototype=e.prototype;u=new _e;_e.prototype=null;var c=e.apply(u,f);return Ye(c)?c:u}return e.apply(u,f)}return s}function de(e,t,r){if(e==null){return ar}var n=typeof e;if(n!="function"){if(n!="object"){return function(t){return t[e]}}var i=Ae(e);return function(t){var r=i.length,n=false;while(r--){if(!(n=$e(t[i[r]],e[i[r]],h))){break}}return n}}if(typeof t!="undefined"){if(r===1){return function(r){return e.call(t,r)}}if(r===2){return function(r,n){return e.call(t,r,n)}}if(r===4){return function(r,n,i,a){return e.call(t,r,n,i,a)}}return function(r,n,i){return e.call(t,r,n,i)}}return e}function ve(){var e={isKeysFast:ee,arrays:"isArray(iterable)",bottom:"",loop:"",top:"",useHas:true};for(var t,r=0;t=arguments[r];r++){for(var n in t){e[n]=t[n]}}var i=e.args;e.firstArg=/^[^,]+/.exec(i)[0];var a=Function("createCallback, hasOwnProperty, isArguments, isArray, isString, "+"objectTypes, nativeKeys","return function("+i+") {\n"+oe(e)+"\n}");return a(de,L,Se,je,Je,ne,D)}var ge=ve(fe);function ye(e){return"\\"+ie[e]}function be(e){return Te[e]}function me(e){return typeof e.toString!="function"&&typeof(e+"")=="string"}function _e(){}function we(e,t,r){t||(t=0);if(typeof r=="undefined"){r=e?e.length:0}var n=-1,i=r-t||0,a=Array(i<0?0:i);while(++n<i){a[n]=e[t+n]}return a}function Ee(e){return Le[e]}function Se(e){return C.call(e)==G}var xe=ve(fe,ue,{useHas:false});var Oe=ve(fe,ue);var je=M||function(e){return e instanceof Array||C.call(e)==$};var Ae=!D?Re:function(e){if(!Ye(e)){return[]}return D(e)};function ke(e){var t=false;if(!(e&&typeof e=="object")||Se(e)){return t}var r=e.constructor;if(!He(r)||r instanceof r){xe(e,function(e,r){t=r});return t===false||L.call(e,t)}return t}function Re(e){var t=[];Oe(e,function(e,r){t.push(r)});return t}var Te={"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"};var Le=De(Te);var Ie=ve(se,{top:se.top.replace(";",";\n"+"if (argsLength > 3 && typeof args[argsLength - 2] == 'function') {\n"+"  var callback = createCallback(args[--argsLength - 1], args[argsLength--], 2);\n"+"} else if (argsLength > 2 && typeof args[argsLength - 1] == 'function') {\n"+"  callback = args[--argsLength];\n"+"}"),loop:"result[index] = callback ? callback(result[index], iterable[index]) : iterable[index]"});function Ce(e,t,r,n,i,o){var s=e;if(typeof t=="function"){n=r;r=t;t=false}if(typeof r=="function"){r=typeof n=="undefined"?r:de(r,n,1);s=r(s);var f=typeof s!="undefined";if(!f){s=e}}var u=Ye(s);if(u){var c=C.call(s);if(!te[c]){return s}var l=je(s)}if(!u||!t){return u&&!f?l?we(s):Ie({},s):s}var h=re[c];switch(c){case W:case H:return f?s:new h((+s));case z:case V:return f?s:new h(s);case X:return f?s:h(s.source,m.exec(s))}i||(i=[]);o||(o=[]);var p=i.length;while(p--){if(i[p]==e){return o[p]}}if(!f){s=l?h(s.length):{};if(l){if(L.call(e,"index")){s.index=e.index}if(L.call(e,"input")){s.input=e.input}}}i.push(e);o.push(s);(l?lt:Oe)(f?s:e,function(e,n){s[n]=Ce(e,t,r,a,i,o)});return s}function Pe(e,t,r){return Ce(e,true,t,r)}var Me=ve(se);function Ne(e){var t=[];xe(e,function(e,r){if(He(e)){t.push(r)}});return t.sort()}function Be(e,t){return e?L.call(e,t):false}function De(e){var t=-1,r=Ae(e),n=r.length,i={};while(++t<n){var a=r[t];i[e[a]]=a}return i}function Ue(e){return e===true||e===false||C.call(e)==W}function Fe(e){return e instanceof Date||C.call(e)==H}function qe(e){return e?e.nodeType===1:false}function Ge(e){var t=true;if(!e){return t}var r=C.call(e),n=e.length;if(r==$||r==V||r==G||r==K&&typeof n=="number"&&He(e.splice)){return!n}Oe(e,function(){return t=false});return t}function $e(e,t,r,n,i,a){var o=r===h;if(r&&!o){r=typeof n=="undefined"?r:de(r,n,2);var s=r(e,t);if(typeof s!="undefined"){return!!s}}if(e===t){return e!==0||1/e==1/t}var f=typeof e,u=typeof t;if(e===e&&(!e||f!="function"&&f!="object")&&(!t||u!="function"&&u!="object")){return false}if(e==null||t==null){return e===t}var c=C.call(e),l=C.call(t);if(c==G){c=K}if(l==G){l=K}if(c!=l){return false}switch(c){case W:case H:return+e==+t;case z:return e!=+e?t!=+t:e==0?1/e==1/t:e==+t;case X:case V:return e==t+""}var p=c==$;if(!p){if(e.__wrapped__||t.__wrapped__){return $e(e.__wrapped__||e,t.__wrapped__||t,r,n,i,a)}if(c!=K){return false}var d=e.constructor,v=t.constructor;if(d!=v&&!(He(d)&&d instanceof d&&He(v)&&v instanceof v)){return false}}i||(i=[]);a||(a=[]);var g=i.length;while(g--){if(i[g]==e){return a[g]==t}}var y=0;s=true;i.push(e);a.push(t);if(p){g=e.length;y=t.length;s=y==e.length;if(!s&&!o){return s}while(y--){var b=g,m=t[y];if(o){while(b--){if(s=$e(e[b],m,r,n,i,a)){break}}}else if(!(s=$e(e[y],m,r,n,i,a))){break}}return s}xe(t,function(t,o,f){if(L.call(f,o)){y++;return s=L.call(e,o)&&$e(e[o],t,r,n,i,a)}});if(s&&!o){xe(e,function(e,t,r){if(L.call(r,t)){return s=--y>-1}})}return s}function We(e){return N(e)&&!B(parseFloat(e))}function He(e){return typeof e=="function"}if(He(/x/)){He=function(e){return e instanceof Function||C.call(e)==Y}}function Ye(e){return e?ne[typeof e]:false}function ze(e){return Xe(e)&&e!=+e}function Ke(e){return e===null}function Xe(e){return typeof e=="number"||C.call(e)==z}var Ve=!T?ke:function(e){if(!(e&&typeof e=="object")){return false}var t=e.valueOf,r=typeof t=="function"&&(r=T(t))&&T(r);return r?e==r||T(e)==r&&!Se(e):ke(e)};function Qe(e){return e instanceof RegExp||C.call(e)==X}function Je(e){return typeof e=="string"||C.call(e)==V}function Ze(e){return typeof e=="undefined"}function et(e,t,r){var n=arguments,i=0,a=2;if(!Ye(e)){return e}if(r===h){var o=n[3],s=n[4],f=n[5]}else{s=[];f=[];if(typeof r!="number"){a=n.length}if(a>3&&typeof n[a-2]=="function"){o=de(n[--a-1],n[a--],2)}else if(a>2&&typeof n[a-1]=="function"){o=n[--a]}}while(++i<a){(je(n[i])?lt:Oe)(n[i],function(t,r){var n,i,a=t,u=e[r];if(t&&((i=je(t))||Ve(t))){var c=s.length;while(c--){if(n=s[c]==t){u=f[c];break}}if(!n){u=i?je(u)?u:[]:Ve(u)?u:{};if(o){a=o(u,t);if(typeof a!="undefined"){u=a}}s.push(t);f.push(u);if(!o){u=et(u,t,h,o,s,f)}}}else{if(o){a=o(u,t);if(typeof a=="undefined"){a=t}}if(typeof a!="undefined"){u=a}}e[r]=u})}return e}function tt(e,t,r){var n=typeof t=="function",i={};if(n){t=de(t,r)}else{var a=k.apply(u,arguments)}xe(e,function(e,r,o){if(n?!t(e,r,o):Lt(a,r,1)<0){i[r]=e}});return i}function rt(e){var t=-1,r=Ae(e),n=r.length,i=Array(n);while(++t<n){var a=r[t];i[t]=[a,e[a]]}return i}function nt(e,t,r){var n={};if(typeof t!="function"){var i=0,a=k.apply(u,arguments),o=Ye(e)?a.length:0;while(++i<o){var s=a[i];if(s in e){n[s]=e[s]}}}else{t=de(t,r);xe(e,function(e,r,i){if(t(e,r,i)){n[r]=e}})}return n}function it(e){var t=-1,r=Ae(e),n=r.length,i=Array(n);while(++t<n){i[t]=e[r[t]]}return i}function at(e){var t=-1,r=k.apply(u,we(arguments,1)),n=r.length,i=Array(n);while(++t<n){i[t]=e[r[t]]}return i}function ot(e,t,r){var n=-1,i=e?e.length:0,a=false;r=(r<0?U(0,i+r):r)||0;if(typeof i=="number"){a=(Je(e)?e.indexOf(t,r):Lt(e,t,r))>-1}else{ge(e,function(e){if(++n>=r){return!(a=e===t)}})}return a}function st(e,t,r){var n={};t=de(t,r);lt(e,function(e,r,i){r=t(e,r,i)+"";L.call(n,r)?n[r]++:n[r]=1});return n}function ft(e,t,r){var n=true;t=de(t,r);if(je(e)){var i=-1,a=e.length;while(++i<a){if(!(n=!!t(e[i],i,e))){break}}}else{ge(e,function(e,r,i){return n=!!t(e,r,i)})}return n}function ut(e,t,r){var n=[];t=de(t,r);if(je(e)){var i=-1,a=e.length;while(++i<a){var o=e[i];if(t(o,i,e)){n.push(o)}}}else{ge(e,function(e,r,i){if(t(e,r,i)){n.push(e)}})}return n}function ct(e,t,r){var n;t=de(t,r);lt(e,function(e,r,i){if(t(e,r,i)){n=e;return false}});return n}function lt(e,t,r){if(t&&typeof r=="undefined"&&je(e)){var n=-1,i=e.length;while(++n<i){if(t(e[n],n,e)===false){break}}}else{ge(e,t,r)}return e}function ht(e,t,r){var n={};t=de(t,r);lt(e,function(e,r,i){r=t(e,r,i)+"";(L.call(n,r)?n[r]:n[r]=[]).push(e)});return n}function pt(e,t){var r=we(arguments,2),n=-1,i=typeof t=="function",a=e?e.length:0,o=Array(typeof a=="number"?a:0);lt(e,function(e){o[++n]=(i?t:e[t]).apply(e,r)});return o}function dt(e,t,r){var n=-1,i=e?e.length:0,a=Array(typeof i=="number"?i:0);t=de(t,r);if(je(e)){while(++n<i){a[n]=t(e[n],n,e)}}else{ge(e,function(e,r,i){a[++n]=t(e,r,i)})}return a}function vt(e,t,r){var n=-Infinity,i=n;if(!t&&je(e)){var a=-1,o=e.length;while(++a<o){var s=e[a];if(s>i){i=s}}}else{t=!t&&Je(e)?le:de(t,r);ge(e,function(e,r,a){var o=t(e,r,a);if(o>n){n=o;i=e}})}return i}function gt(e,t,r){var n=Infinity,i=n;if(!t&&je(e)){var a=-1,o=e.length;while(++a<o){var s=e[a];if(s<i){i=s}}}else{t=!t&&Je(e)?le:de(t,r);ge(e,function(e,r,a){var o=t(e,r,a);if(o<n){n=o;i=e}})}return i}var yt=dt;function bt(e,t,r,n){var i=arguments.length<3;t=de(t,n,4);if(je(e)){var a=-1,o=e.length;if(i){r=e[++a]}while(++a<o){r=t(r,e[a],a,e)}}else{ge(e,function(e,n,a){r=i?(i=false,e):t(r,e,n,a)})}return r}function mt(e,t,r,n){var i=e,a=e?e.length:0,o=arguments.length<3;if(typeof a!="number"){var s=Ae(e);a=s.length}t=de(t,n,4);lt(e,function(e,n,f){n=s?s[--a]:--a;r=o?(o=false,i[n]):t(r,i[n],n,f)});return r}function _t(e,t,r){t=de(t,r);return ut(e,function(e,r,n){return!t(e,r,n)})}function wt(e){var t=-1,r=e?e.length:0,n=Array(typeof r=="number"?r:0);lt(e,function(e){var r=R(q()*(++t+1));n[t]=n[r];n[r]=e});return n}function Et(e){var t=e?e.length:0;return typeof t=="number"?t:Ae(e).length}function St(e,t,r){var n;t=de(t,r);if(je(e)){var i=-1,a=e.length;while(++i<a){if(n=t(e[i],i,e)){break}}}else{ge(e,function(e,r,i){return!(n=t(e,r,i))})}return!!n}function xt(e,t,r){var n=-1,i=e?e.length:0,a=Array(typeof i=="number"?i:0);t=de(t,r);lt(e,function(e,r,i){a[++n]={criteria:t(e,r,i),index:n,value:e}});i=a.length;a.sort(he);while(i--){a[i]=a[i].value}return a}function Ot(e){if(e&&typeof e.length=="number"){return we(e)}return it(e)}var jt=ut;function At(e){var t=-1,r=e?e.length:0,n=[];while(++t<r){var i=e[t];if(i){n.push(i)}}return n}function kt(e){var t=-1,r=e?e.length:0,n=k.apply(u,arguments),i=ce(n,r),a=[];while(++t<r){var o=e[t];if(!i(o)){a.push(o)}}return a}function Rt(e,t,r){if(e){var n=0,i=e.length;if(typeof t!="number"&&t!=null){var a=-1;t=de(t,r);while(++a<i&&t(e[a],a,e)){n++}}else{n=t;if(n==null||r){return e[0]}}return we(e,0,F(U(0,n),i))}}function Tt(e,t){var r=-1,n=e?e.length:0,i=[];while(++r<n){var a=e[r];if(je(a)){I.apply(i,t?a:Tt(a))}else{i.push(a)}}return i}function Lt(e,t,r){var n=-1,i=e?e.length:0;if(typeof r=="number"){n=(r<0?U(0,i+r):r||0)-1}else if(r){n=Ut(e,t);return e[n]===t?n:-1}while(++n<i){if(e[n]===t){return n}}return-1}function It(e,t,r){if(!e){return[]}var n=0,i=e.length;if(typeof t!="number"&&t!=null){var a=i;t=de(t,r);while(a--&&t(e[a],a,e)){n++}}else{n=t==null||r?1:t||n}return we(e,0,F(U(0,i-n),i))}function Ct(e){var t=arguments,r=t.length,n={0:{}},i=-1,a=e?e.length:0,o=a>=100,s=[],f=s;e:while(++i<a){var u=e[i];if(o){var c=u+"";var l=L.call(n[0],c)?!(f=n[0][c]):f=n[0][c]=[]}if(l||Lt(f,u)<0){if(o){f.push(u)}var h=r;while(--h){if(!(n[h]||(n[h]=ce(t[h],0,100)))(u)){continue e}}s.push(u)}}return s}function Pt(e,t,r){if(e){var n=0,i=e.length;if(typeof t!="number"&&t!=null){var a=i;t=de(t,r);while(a--&&t(e[a],a,e)){n++}}else{n=t;if(n==null||r){return e[i-1]}}return we(e,U(0,i-n))}}function Mt(e,t,r){var n=e?e.length:0;if(typeof r=="number"){n=(r<0?U(0,n+r):F(r,n-1))+1}while(n--){if(e[n]===t){return n}}return-1}function Nt(e,t){var r=-1,n=e?e.length:0,i={};while(++r<n){var a=e[r];if(t){i[a]=t[r]}else{i[a[0]]=a[1]}}return i}function Bt(e,t,r){e=+e||0;r=+r||1;if(t==null){t=e;e=0}var n=-1,i=U(0,A((t-e)/r)),a=Array(i);while(++n<i){a[n]=e;e+=r}return a}function Dt(e,t,r){if(typeof t!="number"&&t!=null){var n=0,i=-1,a=e?e.length:0;t=de(t,r);while(++i<a&&t(e[i],i,e)){n++}}else{n=t==null||r?1:U(0,t)}return we(e,n)}function Ut(e,t,r,n){var i=0,a=e?e.length:i;r=r?de(r,n,1):ar;t=r(t);while(i<a){var o=i+a>>>1;r(e[o])<t?i=o+1:a=o}return i}function Ft(){return qt(k.apply(u,arguments))}function qt(e,t,r,n){var i=-1,a=e?e.length:0,o=[],s=o;if(typeof t=="function"){n=r;r=t;t=false}var f=!t&&a>=75;if(f){var u={}}if(r){s=[];r=de(r,n)}while(++i<a){var c=e[i],l=r?r(c,i,e):c;if(f){var h=l+"";var p=L.call(u,h)?!(s=u[h]):s=u[h]=[]}if(t?!i||s[s.length-1]!==l:p||Lt(s,l)<0){if(r||f){s.push(l)}o.push(c)}}return o}function Gt(e){var t=-1,r=e?e.length:0,n=ce(arguments,1),i=[];while(++t<r){var a=e[t];if(!n(a)){i.push(a)}}return i}function $t(e){var t=-1,r=e?vt(yt(arguments,"length")):0,n=Array(r);while(++t<r){n[t]=yt(arguments,t)}return n}function Wt(e,t){if(e<1){return t()}return function(){if(--e<1){return t.apply(this,arguments)}}}function Ht(e,t){return Z||P&&arguments.length>2?P.call.apply(P,arguments):pe(e,t,we(arguments,2))}function Yt(e){var t=k.apply(u,arguments),r=t.length>1?0:(t=Ne(e),-1),n=t.length;while(++r<n){var i=t[r];e[i]=Ht(e[i],e)}return e}function zt(e,t){return pe(e,t,we(arguments,2))}function Kt(){var e=arguments;return function(){var t=arguments,r=e.length;while(r--){t=[e[r].apply(this,t)]}return t[0]}}function Xt(e,t,r){var n,i,a,o;function s(){o=null;if(!r){i=e.apply(a,n)}}return function(){var f=r&&!o;n=arguments;a=this;clearTimeout(o);o=setTimeout(s,t);if(f){i=e.apply(a,n)}return i}}function Vt(e,t){var r=we(arguments,2);return setTimeout(function(){e.apply(a,r)},t)}function Qt(e){var t=we(arguments,1);return setTimeout(function(){e.apply(a,t)},1)}if(J&&s&&typeof setImmediate=="function"){Qt=Ht(setImmediate,i)}function Jt(e,t){var r={};return function(){var n=(t?t.apply(this,arguments):arguments[0])+"";return L.call(r,n)?r[n]:r[n]=e.apply(this,arguments)}}function Zt(e){var t,r;return function(){if(t){return r}t=true;r=e.apply(this,arguments);e=null;return r}}function er(e){return pe(e,we(arguments,1))}function tr(e){return pe(e,we(arguments,1),null,h)}function rr(e,t){var r,n,i,a,o=0;function s(){o=new Date;a=null;n=e.apply(i,r)}return function(){var f=new Date,u=t-(f-o);r=arguments;i=this;if(u<=0){clearTimeout(a);a=null;o=f;n=e.apply(i,r)}else if(!a){a=setTimeout(s,u)}return n}}function nr(e,t){return function(){var r=[e];I.apply(r,arguments);return t.apply(this,r)}}function ir(e){return e==null?"":(e+"").replace(x,be)}function ar(e){return e}function or(e){lt(Ne(e),function(t){var r=ae[t]=e[t];ae.prototype[t]=function(){var e=[this.__wrapped__];I.apply(e,arguments);return new ae(r.apply(ae,e))}})}function sr(){i._=d;return this}function fr(e,t){if(e==null&&t==null){t=1}e=+e||0;if(t==null){t=e;e=0}return e+R(q()*((+t||0)-e+1))}function ur(e,t){var r=e?e[t]:a;return He(r)?e[t]():r}function cr(e,t,r){var n=ae.templateSettings;e||(e="");r=Me({},r,n);var i=Me({},r.imports,n.imports),o=Ae(i),s=it(i);var f,u=0,c=r.interpolate||S,l="__p += '";var h=RegExp((r.escape||S).source+"|"+c.source+"|"+(c===E?w:S).source+"|"+(r.evaluate||S).source+"|$","g");e.replace(h,function(t,r,n,i,a,o){n||(n=i);l+=e.slice(u,o).replace(O,ye);if(r){l+="' +\n__e("+r+") +\n'"}if(a){f=true;l+="';\n"+a+";\n__p += '"}if(n){l+="' +\n((__t = ("+n+")) == null ? '' : __t) +\n'"}u=o+t.length;return t});l+="';\n";var p=r.variable,d=p;if(!d){p="obj";l="with ("+p+") {\n"+l+"\n}\n"}l=(f?l.replace(g,""):l).replace(y,"$1").replace(b,"$1;");l="function("+p+") {\n"+(d?"":p+" || ("+p+" = {});\n")+"var __t, __p = '', __e = _.escape"+(f?", __j = Array.prototype.join;\n"+"function print() { __p += __j.call(arguments, '') }\n":";\n")+l+"return __p\n}";var v="\n/*\n//@ sourceURL="+(r.sourceURL||"/lodash/template/source["+j++ +"]")+"\n*/";try{var m=Function(o,"return "+l+v).apply(a,s)}catch(_){_.source=l;throw _}if(t){return m(t)}m.source=l;return m}function lr(e,t,r){e=+e||0;var n=-1,i=Array(e);while(++n<e){i[n]=t.call(r,n)}return i}function hr(e){return e==null?"":(e+"").replace(v,Ee)}function pr(e){var t=++l;return(e==null?"":e+"")+t}function dr(e,t){t(e);return e}function vr(){return this.__wrapped__+""}function gr(){return this.__wrapped__}ae.after=Wt;ae.assign=Ie;ae.at=at;ae.bind=Ht;ae.bindAll=Yt;ae.bindKey=zt;ae.compact=At;ae.compose=Kt;ae.countBy=st;ae.debounce=Xt;ae.defaults=Me;ae.defer=Qt;ae.delay=Vt;ae.difference=kt;ae.filter=ut;ae.flatten=Tt;ae.forEach=lt;ae.forIn=xe;ae.forOwn=Oe;ae.functions=Ne;ae.groupBy=ht;ae.initial=It;ae.intersection=Ct;ae.invert=De;ae.invoke=pt;ae.keys=Ae;ae.map=dt;ae.max=vt;ae.memoize=Jt;ae.merge=et;ae.min=gt;ae.object=Nt;ae.omit=tt;ae.once=Zt;ae.pairs=rt;ae.partial=er;ae.partialRight=tr;ae.pick=nt;ae.pluck=yt;ae.range=Bt;ae.reject=_t;ae.rest=Dt;ae.shuffle=wt;ae.sortBy=xt;ae.tap=dr;ae.throttle=rr;ae.times=lr;ae.toArray=Ot;ae.union=Ft;ae.uniq=qt;ae.values=it;ae.where=jt;ae.without=Gt;ae.wrap=nr;ae.zip=$t;ae.collect=dt;ae.drop=Dt;ae.each=lt;ae.extend=Ie;ae.methods=Ne;ae.select=ut;ae.tail=Dt;ae.unique=qt;or(ae);ae.clone=Ce;ae.cloneDeep=Pe;ae.contains=ot;ae.escape=ir;ae.every=ft;ae.find=ct;ae.has=Be;ae.identity=ar;ae.indexOf=Lt;ae.isArguments=Se;ae.isArray=je;ae.isBoolean=Ue;ae.isDate=Fe;ae.isElement=qe;ae.isEmpty=Ge;ae.isEqual=$e;ae.isFinite=We;ae.isFunction=He;ae.isNaN=ze;ae.isNull=Ke;ae.isNumber=Xe;ae.isObject=Ye;ae.isPlainObject=Ve;ae.isRegExp=Qe;ae.isString=Je;ae.isUndefined=Ze;ae.lastIndexOf=Mt;ae.mixin=or;ae.noConflict=sr;ae.random=fr;ae.reduce=bt;ae.reduceRight=mt;ae.result=ur;ae.size=Et;ae.some=St;ae.sortedIndex=Ut;ae.template=cr;ae.unescape=hr;ae.uniqueId=pr;ae.all=ft;ae.any=St;ae.detect=ct;ae.foldl=bt;ae.foldr=mt;ae.include=ot;ae.inject=bt;Oe(ae,function(e,t){if(!ae.prototype[t]){ae.prototype[t]=function(){var t=[this.__wrapped__];I.apply(t,arguments);return e.apply(ae,t)}}});ae.first=Rt;ae.last=Pt;ae.take=Rt;ae.head=Rt;Oe(ae,function(e,t){if(!ae.prototype[t]){ae.prototype[t]=function(t,r){var n=e(this.__wrapped__,t,r);return t==null||r&&typeof t!="function"?n:new ae(n)}}});ae.VERSION="1.0.2";ae.prototype.toString=vr;ae.prototype.value=gr;ae.prototype.valueOf=gr;ge(["join","pop","shift"],function(e){var t=u[e];ae.prototype[e]=function(){return t.apply(this.__wrapped__,arguments)}});ge(["push","reverse","sort","unshift"],function(e){var t=u[e];ae.prototype[e]=function(){t.apply(this.__wrapped__,arguments);return this}});ge(["concat","slice","splice"],function(e){var t=u[e];ae.prototype[e]=function(){return new ae(t.apply(this.__wrapped__,arguments))}});if(typeof e=="function"&&typeof e.amd=="object"&&e.amd){i._=ae;e(function(){return ae})}else if(o){if(s){(s.exports=ae)._=ae}else{o._=ae}}else{i._=ae}})(this)}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{}],85:[function(e,t,r){(function(){if(typeof t==="object"&&t.exports){t.exports=a}else{this.LRUCache=a}function e(e,t){return Object.prototype.hasOwnProperty.call(e,t)}function r(){return 1}var n=false;function i(e){if(!n&&typeof e!=="string"&&typeof e!=="number"){n=true;console.error(new TypeError("LRU: key must be a string or number. Almost certainly a bug! "+typeof e).stack)}}function a(e){if(!(this instanceof a))return new a(e);if(typeof e==="number")e={max:e};if(!e)e={};this._max=e.max;if(!this._max||!(typeof this._max==="number")||this._max<=0)this._max=Infinity;this._lengthCalculator=e.length||r;if(typeof this._lengthCalculator!=="function")this._lengthCalculator=r;this._allowStale=e.stale||false;this._maxAge=e.maxAge||null;this._dispose=e.dispose;this.reset()}Object.defineProperty(a.prototype,"max",{set:function(e){if(!e||!(typeof e==="number")||e<=0)e=Infinity;this._max=e;if(this._length>this._max)u(this)},get:function(){return this._max},enumerable:true});Object.defineProperty(a.prototype,"lengthCalculator",{set:function(e){if(typeof e!=="function"){this._lengthCalculator=r;this._length=this._itemCount;for(var t in this._cache){this._cache[t].length=1}}else{this._lengthCalculator=e;this._length=0;for(var t in this._cache){this._cache[t].length=this._lengthCalculator(this._cache[t].value);this._length+=this._cache[t].length}}if(this._length>this._max)u(this)},get:function(){return this._lengthCalculator},enumerable:true});Object.defineProperty(a.prototype,"length",{get:function(){return this._length},enumerable:true});Object.defineProperty(a.prototype,"itemCount",{get:function(){return this._itemCount},enumerable:true});a.prototype.forEach=function(e,t){t=t||this;var r=0;var n=this._itemCount;for(var i=this._mru-1;i>=0&&r<n;i--)if(this._lruList[i]){r++;var a=this._lruList[i];if(s(this,a)){l(this,a);if(!this._allowStale)a=undefined}if(a){e.call(t,a.value,a.key,this)}}};a.prototype.keys=function(){var e=new Array(this._itemCount);var t=0;for(var r=this._mru-1;r>=0&&t<this._itemCount;r--)if(this._lruList[r]){var n=this._lruList[r];e[t++]=n.key}return e};a.prototype.values=function(){var e=new Array(this._itemCount);var t=0;for(var r=this._mru-1;r>=0&&t<this._itemCount;r--)if(this._lruList[r]){var n=this._lruList[r];e[t++]=n.value}return e};a.prototype.reset=function(){if(this._dispose&&this._cache){for(var e in this._cache){this._dispose(e,this._cache[e].value)}}this._cache=Object.create(null);this._lruList=Object.create(null);this._mru=0;this._lru=0;this._length=0;this._itemCount=0};a.prototype.dump=function(){var e=[];var t=0;for(var r=this._mru-1;r>=0&&t<this._itemCount;r--)if(this._lruList[r]){var n=this._lruList[r];if(!s(this,n)){++t;e.push({k:n.key,v:n.value,e:n.now+(n.maxAge||0)})}}return e};a.prototype.dumpLru=function(){return this._lruList};a.prototype.set=function(t,r,n){n=n||this._maxAge;i(t);var a=n?Date.now():0;var o=this._lengthCalculator(r);if(e(this._cache,t)){if(o>this._max){l(this,this._cache[t]);return false}if(this._dispose)this._dispose(t,this._cache[t].value);this._cache[t].now=a;this._cache[t].maxAge=n;this._cache[t].value=r;this._length+=o-this._cache[t].length;this._cache[t].length=o;this.get(t);if(this._length>this._max)u(this);return true}var s=new h(t,r,(this._mru++),o,a,n);if(s.length>this._max){if(this._dispose)this._dispose(t,r);return false}this._length+=s.length;this._lruList[s.lu]=this._cache[t]=s;this._itemCount++;if(this._length>this._max)u(this);return true};a.prototype.has=function(t){i(t);if(!e(this._cache,t))return false;var r=this._cache[t];if(s(this,r)){return false}return true};a.prototype.get=function(e){i(e);return o(this,e,true)};a.prototype.peek=function(e){i(e);return o(this,e,false)};a.prototype.pop=function(){var e=this._lruList[this._lru];l(this,e);return e||null};a.prototype.del=function(e){i(e);l(this,this._cache[e])};a.prototype.load=function(e){this.reset();var t=Date.now();for(var r=e.length-1;r>=0;r--){var n=e[r];i(n.k);var a=n.e||0;if(a===0){this.set(n.k,n.v)}else{var o=a-t;if(o>0)this.set(n.k,n.v,o)}}};function o(e,t,r){i(t);var n=e._cache[t];if(n){if(s(e,n)){l(e,n);if(!e._allowStale)n=undefined}else{if(r)f(e,n)}if(n)n=n.value}return n}function s(e,t){if(!t||!t.maxAge&&!e._maxAge)return false;var r=false;var n=Date.now()-t.now;if(t.maxAge){r=n>t.maxAge}else{r=e._maxAge&&n>e._maxAge}return r}function f(e,t){c(e,t);t.lu=e._mru++;e._lruList[t.lu]=t}function u(e){while(e._lru<e._mru&&e._length>e._max)l(e,e._lruList[e._lru])}function c(e,t){delete e._lruList[t.lu];while(e._lru<e._mru&&!e._lruList[e._lru])e._lru++}function l(e,t){if(t){if(e._dispose)e._dispose(t.key,t.value);e._length-=t.length;e._itemCount--;delete e._cache[t.key];c(e,t)}}function h(e,t,r,n,i,a){this.key=e;this.value=t;this.lu=r;this.length=n;this.now=i;if(a)this.maxAge=a}})()},{}],86:[function(e,t,r){"use strict";var n=e("readable-stream/passthrough");t.exports=function(){var e=[];var t=new n({objectMode:true});t.setMaxListeners(0);t.add=r;t.isEmpty=i;t.on("unpipe",a);Array.prototype.slice.call(arguments).forEach(r);return t;function r(n){if(Array.isArray(n)){n.forEach(r);return this}e.push(n);n.once("end",a.bind(null,n));n.pipe(t,{end:false});return this}function i(){return e.length==0}function a(r){e=e.filter(function(e){return e!==r});if(!e.length&&t.readable){t.end()}}}},{"readable-stream/passthrough":124}],87:[function(e,t,r){"use strict";var n=e("./lib/expand");var i=e("./lib/utils");function a(e,t,r){if(!e||!t)return[];r=r||{};if(typeof r.cache==="undefined"){r.cache=true}if(!Array.isArray(t)){return o(e,t,r)}var n=t.length,a=0;var s=[],f=[];while(n--){var u=t[a++];if(typeof u==="string"&&u.charCodeAt(0)===33){s.push.apply(s,o(e,u.slice(1),r))}else{f.push.apply(f,o(e,u,r))}}return i.diff(f,s)}function o(e,t,r){if(i.typeOf(e)!=="string"&&!Array.isArray(e)){throw new Error(g("match","files","a string or array"))}e=i.arrayify(e);r=r||{};var n=r.negate||false;var o=t;if(typeof t==="string"){n=t.charAt(0)==="!";if(n){t=t.slice(1)}if(r.nonegate===true){n=false}}var s=h(t,r);var f=e.length,u=0;var c=[];while(u<f){var l=e[u++];var p=i.unixify(l,r);if(!s(p)){continue}c.push(p)}if(c.length===0){if(r.failglob===true){throw new Error('micromatch.match() found no matches for: "'+o+'".')}if(r.nonull||r.nullglob){c.push(i.unescapeGlob(o))}}if(n){c=i.diff(e,c)}if(r.ignore&&r.ignore.length){t=r.ignore;r=i.omit(r,["ignore"]);c=i.diff(c,a(c,t,r))}if(r.nodupes){return i.unique(c)}return c}function s(e,t){if(!Array.isArray(e)&&typeof e!=="string"){throw new TypeError(g("filter","patterns","a string or array"))}e=i.arrayify(e);var r=e.length,n=0;var a=Array(r);while(n<r){a[n]=h(e[n++],t)}return function(e){if(e==null)return[];var r=a.length,n=0;var o=true;e=i.unixify(e,t);
while(n<r){var s=a[n++];if(!s(e)){o=false;break}}return o}}function f(e,t,r){if(typeof e!=="string"){throw new TypeError(g("isMatch","filepath","a string"))}e=i.unixify(e,r);if(i.typeOf(t)==="object"){return h(e,t)}return h(t,r)(e)}function u(e,t,r){if(typeof e!=="string"){throw new TypeError(g("contains","pattern","a string"))}r=r||{};r.contains=t!=="";e=i.unixify(e,r);if(r.contains&&!i.isGlob(t)){return e.indexOf(t)!==-1}return h(t,r)(e)}function c(e,t,r){if(!Array.isArray(t)&&typeof t!=="string"){throw new TypeError(g("any","patterns","a string or array"))}t=i.arrayify(t);var n=t.length;e=i.unixify(e,r);while(n--){var a=h(t[n],r);if(a(e)){return true}}return false}function l(e,t,r){if(i.typeOf(e)!=="object"){throw new TypeError(g("matchKeys","first argument","an object"))}var n=h(t,r);var a={};for(var o in e){if(e.hasOwnProperty(o)&&n(o)){a[o]=e[o]}}return a}function h(e,t){if(typeof e==="function"){return e}if(e instanceof RegExp){return function(t){return e.test(t)}}if(typeof e!=="string"){throw new TypeError(g("matcher","pattern","a string, regex, or function"))}e=i.unixify(e,t);if(!i.isGlob(e)){return i.matchPath(e,t)}var r=v(e,t);if(t&&t.matchBase){return i.hasFilename(r,t)}return function(e){e=i.unixify(e,t);return r.test(e)}}function p(e,t){var r=Object.create(t||{});var i=r.flags||"";if(r.nocase&&i.indexOf("i")===-1){i+="i"}var a=n(e,r);r.negated=r.negated||a.negated;r.negate=r.negated;e=d(a.pattern,r);var o;try{o=new RegExp(e,i);return o}catch(s){s.reason="micromatch invalid regex: ("+o+")";if(r.strict)throw new SyntaxError(s)}return/$^/}function d(e,t){var r=t&&!t.contains?"^":"";var n=t&&!t.contains?"$":"";e="(?:"+e+")"+n;if(t&&t.negate){return r+("(?!^"+e+").*$")}return r+e}function v(e,t){if(i.typeOf(e)!=="string"){throw new Error(g("makeRe","glob","a string"))}return i.cache(p,e,t)}function g(e,t,r){return"micromatch."+e+"(): "+t+" should be "+r+"."}a.any=c;a.braces=a.braceExpand=i.braces;a.contains=u;a.expand=n;a.filter=s;a.isMatch=f;a.makeRe=v;a.match=o;a.matcher=h;a.matchKeys=l;t.exports=a},{"./lib/expand":89,"./lib/utils":91}],88:[function(e,t,r){"use strict";var n={},i,a;function o(e,t){return Object.keys(e).reduce(function(r,n){var i=t?t+n:n;r[e[n]]=i;return r},{})}n.escapeRegex={"?":/\?/g,"@":/\@/g,"!":/\!/g,"+":/\+/g,"*":/\*/g,"(":/\(/g,")":/\)/g,"[":/\[/g,"]":/\]/g};n.ESC={"?":"__UNESC_QMRK__","@":"__UNESC_AMPE__","!":"__UNESC_EXCL__","+":"__UNESC_PLUS__","*":"__UNESC_STAR__",",":"__UNESC_COMMA__","(":"__UNESC_LTPAREN__",")":"__UNESC_RTPAREN__","[":"__UNESC_LTBRACK__","]":"__UNESC_RTBRACK__"};n.UNESC=i||(i=o(n.ESC,"\\"));n.ESC_TEMP={"?":"__TEMP_QMRK__","@":"__TEMP_AMPE__","!":"__TEMP_EXCL__","*":"__TEMP_STAR__","+":"__TEMP_PLUS__",",":"__TEMP_COMMA__","(":"__TEMP_LTPAREN__",")":"__TEMP_RTPAREN__","[":"__TEMP_LTBRACK__","]":"__TEMP_RTBRACK__"};n.TEMP=a||(a=o(n.ESC_TEMP));t.exports=n},{}],89:[function(e,t,r){"use strict";var n=e("./utils");var i=e("./glob");t.exports=a;function a(e,t){if(typeof e!=="string"){throw new TypeError("micromatch.expand(): argument should be a string.")}var r=new i(e,t||{});var a=r.options;if(!n.isGlob(e)){r.pattern=r.pattern.replace(/([\/.])/g,"\\$1");return r}r.pattern=r.pattern.replace(/(\+)(?!\()/g,"\\$1");r.pattern=r.pattern.split("$").join("\\$");if(typeof a.braces!=="boolean"&&typeof a.nobraces!=="boolean"){a.braces=true}if(r.pattern===".*"){return{pattern:"\\."+c,tokens:h,options:a}}if(r.pattern==="*"){return{pattern:v(a.dot),tokens:h,options:a}}r.parse();var h=r.tokens;h.is.negated=a.negated;if((a.dotfiles===true||h.is.dotfile)&&a.dot!==false){a.dotfiles=true;a.dot=true}if((a.dotdirs===true||h.is.dotdir)&&a.dot!==false){a.dotdirs=true;a.dot=true}if(/[{,]\./.test(r.pattern)){a.makeRe=false;a.dot=true}if(a.nonegate!==true){a.negated=r.negated}if(r.pattern.charAt(0)==="."&&r.pattern.charAt(1)!=="/"){r.pattern="\\"+r.pattern}r.track("before braces");if(h.is.braces){r.braces()}r.track("after braces");r.track("before extglob");if(h.is.extglob){r.extglob()}r.track("after extglob");r.track("before brackets");if(h.is.brackets){r.brackets()}r.track("after brackets");r._replace("[!","[^");r._replace("(?","(%~");r._replace(/\[\]/,"\\[\\]");r._replace("/[","/"+(a.dot?p:l)+"[",true);r._replace("/?","/"+(a.dot?p:l)+"[^/]",true);r._replace("/.","/(?=.)\\.",true);r._replace(/^(\w):([\\\/]+?)/gi,"(?=.)$1:$2",true);if(r.pattern.indexOf("[^")!==-1){r.pattern=s(r.pattern)}if(a.globstar!==false&&r.pattern==="**"){r.pattern=g(a.dot)}else{r._replace(/(\/\*)+/g,function(e){var t=e.length/2;if(t===1){return e}return"(?:\\/*){"+t+"}"});r.pattern=f(r.pattern,"[","]");r.escape(r.pattern);if(h.is.globstar){r.pattern=o(r.pattern,"/**");r.pattern=o(r.pattern,"**/");r._replace("/**/","(?:/"+g(a.dot)+"/|/)",true);r._replace(/\*{2,}/g,"**");r._replace(/(\w+)\*(?!\/)/g,"$1[^/]*?",true);r._replace(/\*\*\/\*(\w)/g,g(a.dot)+"\\/"+(a.dot?p:l)+"[^/]*?$1",true);if(a.dot!==true){r._replace(/\*\*\/(.)/g,"(?:**\\/|)$1")}if(h.path.dirname!==""||/,\*\*|\*\*,/.test(r.orig)){r._replace("**",g(a.dot),true)}}r._replace(/\/\*$/,"\\/"+v(a.dot),true);r._replace(/(?!\/)\*$/,c,true);r._replace(/([^\/]+)\*/,"$1"+v(true),true);r._replace("*",v(a.dot),true);r._replace("?.","?\\.",true);r._replace("?:","?:",true);r._replace(/\?+/g,function(e){var t=e.length;if(t===1){return u}return u+"{"+t+"}"});r._replace(/\.([*\w]+)/g,"\\.$1");r._replace(/\[\^[\\\/]+\]/g,u);r._replace(/\/+/g,"\\/");r._replace(/\\{2,}/g,"\\")}r.unescape(r.pattern);r._replace("__UNESC_STAR__","*");r._replace("?.","?\\.");r._replace("[^\\/]",u);if(r.pattern.length>1){if(/^[\[?*]/.test(r.pattern)){r.pattern=(a.dot?p:l)+r.pattern}}return r}function o(e,t){var r=e.split(t);var n=r[0]==="";var i=r[r.length-1]==="";r=r.filter(Boolean);if(n)r.unshift("");if(i)r.push("");return r.join(t)}function s(e){return e.replace(/\[\^([^\]]*?)\]/g,function(e,t){if(t.indexOf("/")===-1){t="\\/"+t}return"[^"+t+"]"})}function f(e,t,r){var n=e.split(t);var i=n.join("").length;var a=e.split(r).join("").length;if(i!==a){e=n.join("\\"+t);return e.split(r).join("\\"+r)}return e}var u="[^/]";var c=u+"*?";var l="(?!\\.)(?=.)";var h="(?:\\/|^)\\.{1,2}($|\\/)";var p="(?!"+h+")(?=.)";var d="(?:(?!"+h+").)*?";function v(e){return e?"(?!"+h+")(?=.)"+c:l+c}function g(e){if(e){return d}return"(?:(?!(?:\\/|^)\\.).)*?"}},{"./glob":90,"./utils":91}],90:[function(e,t,r){"use strict";var n=e("./chars");var i=e("./utils");var a=t.exports=function f(e,t){if(!(this instanceof f)){return new f(e,t)}this.options=t||{};this.pattern=e;this.history=[];this.tokens={};this.init(e)};a.prototype.init=function(e){this.orig=e;this.negated=this.isNegated();this.options.track=this.options.track||false;this.options.makeRe=true};a.prototype.track=function(e){if(this.options.track){this.history.push({msg:e,pattern:this.pattern})}};a.prototype.isNegated=function(){if(this.pattern.charCodeAt(0)===33){this.pattern=this.pattern.slice(1);return true}return false};a.prototype.braces=function(){if(this.options.nobraces!==true&&this.options.nobrace!==true){var e=this.pattern.match(/[\{\(\[]/g);var t=this.pattern.match(/[\}\)\]]/g);if(e&&t&&e.length!==t.length){this.options.makeRe=false}var r=i.braces(this.pattern,this.options);this.pattern=r.join("|")}};a.prototype.brackets=function(){if(this.options.nobrackets!==true){this.pattern=i.brackets(this.pattern)}};a.prototype.extglob=function(){if(this.options.noextglob===true)return;if(i.isExtglob(this.pattern)){this.pattern=i.extglob(this.pattern,{escape:true})}};a.prototype.parse=function(e){this.tokens=i.parseGlob(e||this.pattern,true);return this.tokens};a.prototype._replace=function(e,t,r){this.track('before (find): "'+e+'" (replace with): "'+t+'"');if(r)t=o(t);if(e&&t&&typeof e==="string"){this.pattern=this.pattern.split(e).join(t)}else{this.pattern=this.pattern.replace(e,t)}this.track("after")};a.prototype.escape=function(e){this.track("before escape: ");var t=/["\\](['"]?[^"'\\]['"]?)/g;this.pattern=e.replace(t,function(e,t){var r=n.ESC;var i=r&&r[t];if(i){return i}if(/[a-z]/i.test(e)){return e.split("\\").join("")}return e});this.track("after escape: ")};a.prototype.unescape=function(e){var t=/__([A-Z]+)_([A-Z]+)__/g;this.pattern=e.replace(t,function(e,t){return n[t][e]});this.pattern=s(this.pattern)};function o(e){e=e.split("?").join("%~");e=e.split("*").join("%%");return e}function s(e){e=e.split("%~").join("?");e=e.split("%%").join("*");return e}},{"./chars":88,"./utils":91}],91:[function(e,t,r){(function(r){"use strict";var n=r&&r.platform==="win32";var i=e("path");var a=e("filename-regex");var o=t.exports;o.diff=e("arr-diff");o.unique=e("array-unique");o.braces=e("braces");o.brackets=e("expand-brackets");o.extglob=e("extglob");o.isExtglob=e("is-extglob");o.isGlob=e("is-glob");o.typeOf=e("kind-of");o.normalize=e("normalize-path");o.omit=e("object.omit");o.parseGlob=e("parse-glob");o.cache=e("regex-cache");o.filename=function s(e){var t=e.match(a());return t&&t[0]};o.isPath=function f(e,t){t=t||{};return function(r){var n=o.unixify(r,t);if(t.nocase){return e.toLowerCase()===n.toLowerCase()}return e===n}};o.hasPath=function u(e,t){return function(r){return o.unixify(e,t).indexOf(r)!==-1}};o.matchPath=function c(e,t){var r=t&&t.contains?o.hasPath(e,t):o.isPath(e,t);return r};o.hasFilename=function l(e){return function(t){var r=o.filename(t);return r&&e.test(r)}};o.arrayify=function h(e){return!Array.isArray(e)?[e]:e};o.unixify=function p(e,t){if(t&&t.unixify===false)return e;if(t&&t.unixify===true||n||i.sep==="\\"){return o.normalize(e,false)}if(t&&t.unescape===true){return e?e.toString().replace(/\\(\w)/g,"$1"):""}return e};o.escapePath=function d(e){return e.replace(/[\\.]/g,"\\$&")};o.unescapeGlob=function v(e){return e.replace(/[\\"']/g,"")};o.escapeRe=function g(e){return e.replace(/[-[\\$*+?.#^\s{}(|)\]]/g,"\\$&")};t.exports=o}).call(this,e("_process"))},{_process:112,"arr-diff":1,"array-unique":3,braces:9,"expand-brackets":23,extglob:27,"filename-regex":28,"is-extglob":63,"is-glob":64,"kind-of":76,"normalize-path":99,"object.omit":103,"parse-glob":107,path:108,"regex-cache":128}],92:[function(e,t,r){t.exports=g;g.Minimatch=y;var n={sep:"/"};try{n=e("path")}catch(i){}var a=g.GLOBSTAR=y.GLOBSTAR={};var o=e("brace-expansion");var s="[^/]";var f=s+"*?";var u="(?:(?!(?:\\/|^)(?:\\.{1,2})($|\\/)).)*?";var c="(?:(?!(?:\\/|^)\\.).)*?";var l=h("().*{}+?[]^$\\!");function h(e){return e.split("").reduce(function(e,t){e[t]=true;return e},{})}var p=/\/+/;g.filter=d;function d(e,t){t=t||{};return function(r,n,i){return g(r,e,t)}}function v(e,t){e=e||{};t=t||{};var r={};Object.keys(t).forEach(function(e){r[e]=t[e]});Object.keys(e).forEach(function(t){r[t]=e[t]});return r}g.defaults=function(e){if(!e||!Object.keys(e).length)return g;var t=g;var r=function n(r,i,a){return t.minimatch(r,i,v(e,a))};r.Minimatch=function i(r,n){return new t.Minimatch(r,v(e,n))};return r};y.defaults=function(e){if(!e||!Object.keys(e).length)return y;return g.defaults(e).Minimatch};function g(e,t,r){if(typeof t!=="string"){throw new TypeError("glob pattern string required")}if(!r)r={};if(!r.nocomment&&t.charAt(0)==="#"){return false}if(t.trim()==="")return e==="";return new y(t,r).match(e)}function y(e,t){if(!(this instanceof y)){return new y(e,t)}if(typeof e!=="string"){throw new TypeError("glob pattern string required")}if(!t)t={};e=e.trim();if(n.sep!=="/"){e=e.split(n.sep).join("/")}this.options=t;this.set=[];this.pattern=e;this.regexp=null;this.negate=false;this.comment=false;this.empty=false;this.make()}y.prototype.debug=function(){};y.prototype.make=b;function b(){if(this._made)return;var e=this.pattern;var t=this.options;if(!t.nocomment&&e.charAt(0)==="#"){this.comment=true;return}if(!e){this.empty=true;return}this.parseNegate();var r=this.globSet=this.braceExpand();if(t.debug)this.debug=console.error;this.debug(this.pattern,r);r=this.globParts=r.map(function(e){return e.split(p)});this.debug(this.pattern,r);r=r.map(function(e,t,r){return e.map(this.parse,this)},this);this.debug(this.pattern,r);r=r.filter(function(e){return e.indexOf(false)===-1});this.debug(this.pattern,r);this.set=r}y.prototype.parseNegate=m;function m(){var e=this.pattern;var t=false;var r=this.options;var n=0;if(r.nonegate)return;for(var i=0,a=e.length;i<a&&e.charAt(i)==="!";i++){t=!t;n++}if(n)this.pattern=e.substr(n);this.negate=t}g.braceExpand=function(e,t){return _(e,t)};y.prototype.braceExpand=_;function _(e,t){if(!t){if(this instanceof y){t=this.options}else{t={}}}e=typeof e==="undefined"?this.pattern:e;if(typeof e==="undefined"){throw new TypeError("undefined pattern")}if(t.nobrace||!e.match(/\{.*\}/)){return[e]}return o(e)}y.prototype.parse=E;var w={};function E(e,t){if(e.length>1024*64){throw new TypeError("pattern is too long")}var r=this.options;if(!r.noglobstar&&e==="**")return a;if(e==="")return"";var n="";var i=!!r.nocase;var o=false;var u=[];var c=[];var h;var p;var d=false;var v=-1;var g=-1;var y=e.charAt(0)==="."?"":r.dot?"(?!(?:^|\\/)\\.{1,2}(?:$|\\/))":"(?!\\.)";var b=this;function m(){if(p){switch(p){case"*":n+=f;i=true;break;case"?":n+=s;i=true;break;default:n+="\\"+p;break}b.debug("clearStateChar %j %j",p,n);p=false}}for(var _=0,E=e.length,S;_<E&&(S=e.charAt(_));_++){this.debug("%s\t%s %s %j",e,_,n,S);if(o&&l[S]){n+="\\"+S;o=false;continue}switch(S){case"/":return false;case"\\":m();o=true;continue;case"?":case"*":case"+":case"@":case"!":this.debug("%s\t%s %s %j <-- stateChar",e,_,n,S);if(d){this.debug("  in class");if(S==="!"&&_===g+1)S="^";n+=S;continue}b.debug("call clearStateChar %j",p);m();p=S;if(r.noext)m();continue;case"(":if(d){n+="(";continue}if(!p){n+="\\(";continue}h=p;u.push({type:h,start:_-1,reStart:n.length});n+=p==="!"?"(?:(?!(?:":"(?:";this.debug("plType %j %j",p,n);p=false;continue;case")":if(d||!u.length){n+="\\)";continue}m();i=true;n+=")";var x=u.pop();h=x.type;switch(h){case"!":c.push(x);n+=")[^/]*?)";x.reEnd=n.length;break;case"?":case"+":case"*":n+=h;break;case"@":break}continue;case"|":if(d||!u.length||o){n+="\\|";o=false;continue}m();n+="|";continue;case"[":m();if(d){n+="\\"+S;continue}d=true;g=_;v=n.length;n+=S;continue;case"]":if(_===g+1||!d){n+="\\"+S;o=false;continue}if(d){var j=e.substring(g+1,_);try{RegExp("["+j+"]")}catch(A){var k=this.parse(j,w);n=n.substr(0,v)+"\\["+k[0]+"\\]";i=i||k[1];d=false;continue}}i=true;d=false;n+=S;continue;default:m();if(o){o=false}else if(l[S]&&!(S==="^"&&d)){n+="\\"}n+=S}}if(d){j=e.substr(g+1);k=this.parse(j,w);n=n.substr(0,v)+"\\["+k[0];i=i||k[1]}for(x=u.pop();x;x=u.pop()){var R=n.slice(x.reStart+3);R=R.replace(/((?:\\{2}){0,64})(\\?)\|/g,function(e,t,r){if(!r){r="\\"}return t+t+r+"|"});this.debug("tail=%j\n   %s",R,R);var T=x.type==="*"?f:x.type==="?"?s:"\\"+x.type;i=true;n=n.slice(0,x.reStart)+T+"\\("+R}m();if(o){n+="\\\\"}var L=false;switch(n.charAt(0)){case".":case"[":case"(":L=true}for(var I=c.length-1;I>-1;I--){var C=c[I];var P=n.slice(0,C.reStart);var M=n.slice(C.reStart,C.reEnd-8);var N=n.slice(C.reEnd-8,C.reEnd);var B=n.slice(C.reEnd);N+=B;var D=P.split("(").length-1;var U=B;for(_=0;_<D;_++){U=U.replace(/\)[+*?]?/,"")}B=U;var F="";if(B===""&&t!==w){F="$"}var q=P+M+B+F+N;n=q}if(n!==""&&i){n="(?=.)"+n}if(L){n=y+n}if(t===w){return[n,i]}if(!i){return O(e)}var G=r.nocase?"i":"";try{var $=new RegExp("^"+n+"$",G)}catch(A){return new RegExp("$.")}$._glob=e;$._src=n;return $}g.makeRe=function(e,t){return new y(e,t||{}).makeRe()};y.prototype.makeRe=S;function S(){if(this.regexp||this.regexp===false)return this.regexp;var e=this.set;if(!e.length){this.regexp=false;return this.regexp}var t=this.options;var r=t.noglobstar?f:t.dot?u:c;var n=t.nocase?"i":"";var i=e.map(function(e){return e.map(function(e){return e===a?r:typeof e==="string"?j(e):e._src}).join("\\/")}).join("|");i="^(?:"+i+")$";if(this.negate)i="^(?!"+i+").*$";try{this.regexp=new RegExp(i,n)}catch(o){this.regexp=false}return this.regexp}g.match=function(e,t,r){r=r||{};var n=new y(t,r);e=e.filter(function(e){return n.match(e)});if(n.options.nonull&&!e.length){e.push(t)}return e};y.prototype.match=x;function x(e,t){this.debug("match",e,this.pattern);if(this.comment)return false;if(this.empty)return e==="";if(e==="/"&&t)return true;var r=this.options;if(n.sep!=="/"){e=e.split(n.sep).join("/")}e=e.split(p);this.debug(this.pattern,"split",e);var i=this.set;this.debug(this.pattern,"set",i);var a;var o;for(o=e.length-1;o>=0;o--){a=e[o];if(a)break}for(o=0;o<i.length;o++){var s=i[o];var f=e;if(r.matchBase&&s.length===1){f=[a]}var u=this.matchOne(f,s,t);if(u){if(r.flipNegate)return true;return!this.negate}}if(r.flipNegate)return false;return this.negate}y.prototype.matchOne=function(e,t,r){var n=this.options;this.debug("matchOne",{"this":this,file:e,pattern:t});this.debug("matchOne",e.length,t.length);for(var i=0,o=0,s=e.length,f=t.length;i<s&&o<f;i++,o++){this.debug("matchOne loop");var u=t[o];var c=e[i];this.debug(t,u,c);if(u===false)return false;if(u===a){this.debug("GLOBSTAR",[t,u,c]);var l=i;var h=o+1;if(h===f){this.debug("** at the end");for(;i<s;i++){if(e[i]==="."||e[i]===".."||!n.dot&&e[i].charAt(0)===".")return false}return true}while(l<s){var p=e[l];this.debug("\nglobstar while",e,l,t,h,p);if(this.matchOne(e.slice(l),t.slice(h),r)){this.debug("globstar found match!",l,s,p);return true}else{if(p==="."||p===".."||!n.dot&&p.charAt(0)==="."){this.debug("dot detected!",e,l,t,h);break}this.debug("globstar swallow a segment, and continue");l++}}if(r){this.debug("\n>>> no match, partial?",e,l,t,h);if(l===s)return true}return false}var d;if(typeof u==="string"){if(n.nocase){d=c.toLowerCase()===u.toLowerCase()}else{d=c===u}this.debug("string match",u,c,d)}else{d=c.match(u);this.debug("pattern match",u,c,d)}if(!d)return false}if(i===s&&o===f){return true}else if(i===s){return r}else if(o===f){var v=i===s-1&&e[i]==="";return v}throw new Error("wtf?")};function O(e){return e.replace(/\\(.)/g,"$1")}function j(e){return e.replace(/[-[\]{}()*+?.,\\^$|#\s]/g,"\\$&")}},{"brace-expansion":8,path:108}],93:[function(e,t,r){(function(r){var n=e("path");var i=e("fs");var a=parseInt("0777",8);t.exports=o.mkdirp=o.mkdirP=o;function o(e,t,s,f){if(typeof t==="function"){s=t;t={}}else if(!t||typeof t!=="object"){t={mode:t}}var u=t.mode;var c=t.fs||i;if(u===undefined){u=a&~r.umask()}if(!f)f=null;var l=s||function(){};e=n.resolve(e);c.mkdir(e,u,function(r){if(!r){f=f||e;return l(null,f)}switch(r.code){case"ENOENT":o(n.dirname(e),t,function(r,n){if(r)l(r,n);else o(e,t,l,n)});break;default:c.stat(e,function(e,t){if(e||!t.isDirectory())l(r,f);else l(null,f)});break}})}o.sync=function s(e,t,o){if(!t||typeof t!=="object"){t={mode:t}}var f=t.mode;var u=t.fs||i;if(f===undefined){f=a&~r.umask()}if(!o)o=null;e=n.resolve(e);try{u.mkdirSync(e,f);o=o||e}catch(c){switch(c.code){case"ENOENT":o=s(n.dirname(e),t,o);s(e,t,o);break;default:var l;try{l=u.statSync(e)}catch(h){throw c}if(!l.isDirectory())throw c;break}}return o}}).call(this,e("_process"))},{_process:112,fs:11,path:108}],94:[function(e,t,r){(function(r){"use strict";var n=e("lodash.map");var i=e("lodash.filter");var a=e("./convert");var o=e("./protocols");var s=e("varint");t.exports={stringToStringTuples:f,stringTuplesToString:u,tuplesToStringTuples:l,stringTuplesToTuples:c,bufferToTuples:d,tuplesToBuffer:h,bufferToString:v,stringToBuffer:g,fromString:y,fromBuffer:b,validateBuffer:m,isValidBuffer:_,cleanPath:w,ParseError:E,protoFromTuple:S,sizeForAddr:p};function f(e){var t=[];var r=e.split("/").slice(1);if(r.length===1&&r[0]===""){return[]}for(var n=0;n<r.length;n++){var i=r[n];var a=o(i);if(a.size===0){t.push([i]);continue}n++;if(n>=r.length){throw E("invalid address: "+e)}t.push([i,r[n]])}return t}function u(e){var t=[];n(e,function(e){var r=S(e);t.push(r.name);if(e.length>1){t.push(e[1])}});return"/"+t.join("/")}function c(e){return n(e,function(e){if(!Array.isArray(e)){e=[e]}var t=S(e);if(e.length>1){return[t.code,a.toBuffer(t.code,e[1])]}return[t.code]})}function l(e){return n(e,function(e){var t=S(e);if(e.length>1){return[t.code,a.toString(t.code,e[1])]}return[t.code]})}function h(e){return b(r.concat(n(e,function(e){var t=S(e);var n=new r(s.encode(t.code));if(e.length>1){n=r.concat([n,e[1]])}return n})))}function p(e,t){if(e.size>0){return e.size/8}else if(e.size===0){return 0}else{var r=s.decode(t);return r+s.decode.bytes}}function d(e){var t=[];var r=0;while(r<e.length){var n=s.decode(e,r);var i=s.decode.bytes;var a=o(n);var f=p(a,e.slice(r+i));if(f===0){t.push([n]);r+=i;continue}var u=e.slice(r+i,r+i+f);r+=f+i;if(r>e.length){throw E("Invalid address buffer: "+e.toString("hex"))}t.push([n,u])}return t}function v(e){var t=d(e);var r=l(t);return u(r)}function g(e){e=w(e);var t=f(e);var r=c(t);return h(r)}function y(e){return g(e)}function b(e){var t=m(e);if(t)throw t;return new r(e)}function m(e){try{d(e)}catch(t){return t}}function _(e){return m(e)===undefined}function w(e){return"/"+i(e.trim().split("/")).join("/")}function E(e){return new Error("Error parsing address: "+e)}function S(e){var t=o(e[0]);return t}}).call(this,e("buffer").Buffer)},{"./convert":95,"./protocols":97,buffer:13,"lodash.filter":82,"lodash.map":83,varint:151}],95:[function(e,t,r){(function(r){"use strict";var n=e("ip");var i=e("./protocols");var a=e("bs58");var o=e("varint");t.exports=s;function s(e,t){if(t instanceof r){return s.toString(e,t)}else{return s.toBuffer(e,t)}}s.toString=function h(e,t){e=i(e);switch(e.code){case 4:case 41:return n.toString(t);case 6:case 17:case 33:case 132:return u(t);case 421:return l(t);default:return t.toString("hex")}};s.toBuffer=function p(e,t){e=i(e);switch(e.code){case 4:case 41:return n.toBuffer(t);case 6:case 17:case 33:case 132:return f(parseInt(t,10));case 421:return c(t);default:return new r(t,"hex")}};function f(e){var t=new r(2);t.writeUInt16BE(e,0);return t}function u(e){return e.readUInt16BE(0)}function c(e){var t=new r(a.decode(e));var n=new r(o.encode(t.length));return r.concat([n,t])}function l(e){var t=o.decode(e);var r=e.slice(o.decode.bytes);if(r.length!==t){throw new Error("inconsistent lengths")}return a.encode(r)}}).call(this,e("buffer").Buffer)},{"./protocols":97,bs58:12,buffer:13,ip:58,varint:151}],96:[function(e,t,r){(function(n){"use strict";var i=e("lodash.map");var a=e("xtend");var o=e("./codec");var s=e("./protocols");var f=new Error("Sorry, Not Implemented Yet.");var u=e("varint");r=t.exports=c;function c(e){if(!(this instanceof c)){return new c(e)}if(!e){e=""}if(e instanceof n){this.buffer=o.fromBuffer(e)}else if(typeof e==="string"||e instanceof String){this.buffer=o.fromString(e)}else if(e.buffer&&e.protos&&e.protoCodes){this.buffer=o.fromBuffer(e.buffer)}else{throw new Error("addr must be a string, Buffer, or another Multiaddr")}}c.prototype.toString=function l(){return o.bufferToString(this.buffer)};c.prototype.toOptions=function h(){var e={};var t=this.toString().split("/");e.family=t[1]==="ip4"?"ipv4":"ipv6";e.host=t[2];e.port=t[4];return e};c.prototype.inspect=function p(){return"<Multiaddr "+this.buffer.toString("hex")+" - "+o.bufferToString(this.buffer)+">"};c.prototype.protos=function d(){return i(this.protoCodes(),function(e){return a(s(e))})};c.prototype.protoCodes=function v(){var e=[];var t=this.buffer;var r=0;while(r<t.length){var n=u.decode(t,r);var i=u.decode.bytes;var a=s(n);var f=o.sizeForAddr(a,t.slice(r+i));r+=f+i;e.push(n)}return e};c.prototype.protoNames=function g(){return i(this.protos(),function(e){return e.name})};c.prototype.tuples=function y(){return o.bufferToTuples(this.buffer)};c.prototype.stringTuples=function b(){var e=o.bufferToTuples(this.buffer);return o.tuplesToStringTuples(e)};c.prototype.encapsulate=function m(e){e=c(e);return c(this.toString()+e.toString())};c.prototype.decapsulate=function _(e){e=e.toString();var t=this.toString();var r=t.lastIndexOf(e);if(r<0){throw new Error("Address "+this+" does not contain subaddress: "+e)}return c(t.slice(0,r))};c.prototype.equals=function w(e){return this.buffer.equals(e.buffer)};c.prototype.nodeAddress=function E(){if(!this.isThinWaistAddress()){throw new Error('Multiaddr must be "thin waist" address for nodeAddress.')}var e=this.protoCodes();var t=this.toString().split("/").slice(1);return{family:e[0]===41?"IPv6":"IPv4",address:t[1],port:t[3]}};c.fromNodeAddress=function S(e,t){if(!e)throw new Error("requires node address object");if(!t)throw new Error("requires transport protocol");var r=e.family==="IPv6"?"ip6":"ip4";return c("/"+[r,e.address,t,e.port].join("/"))};c.prototype.isThinWaistAddress=function x(e){var t=(e||this).protos();if(t.length!==2){return false}if(t[0].code!==4&&t[0].code!==41){return false}if(t[1].code!==6&&t[1].code!==17){return false}return true};c.prototype.fromStupidString=function O(e){throw f};c.protocols=s}).call(this,e("buffer").Buffer)},{"./codec":94,"./protocols":97,buffer:13,"lodash.map":83,varint:151,xtend:188}],97:[function(e,t,r){"use strict";var n=e("lodash.map");t.exports=i;function i(e){if(typeof e==="number"){if(i.codes[e]){return i.codes[e]}throw new Error("no protocol with code: "+e)}else if(typeof e==="string"||e instanceof String){if(i.names[e]){return i.names[e]}throw new Error("no protocol with name: "+e)}throw new Error("invalid protocol id type: "+e)}i.lengthPrefixedVarSize=-1;i.table=[[4,32,"ip4"],[6,16,"tcp"],[17,16,"udp"],[33,16,"dccp"],[41,128,"ip6"],[132,16,"sctp"],[302,0,"utp"],[421,i.lengthPrefixedVarSize,"ipfs"],[480,0,"http"],[443,0,"https"],[477,0,"websockets"]];i.names={};i.codes={};n(i.table,function(e){var t=a.apply(this,e);i.codes[t.code]=t;i.names[t.name]=t});i.object=a;function a(e,t,r){return{code:e,size:t,name:r}}},{"lodash.map":83}],98:[function(e,t,r){var n=e("sandwich-stream").SandwichStream;var i=e("stream");var a=e("inherits");var o=e("is-stream");var s="\r\n";t.exports=f;function f(e){if(!this instanceof f){return new f(e)}this.boundary=e||Math.random().toString(36).slice(2);n.call(this,{head:"--"+this.boundary+s,tail:s+"--"+this.boundary+"--",separator:s+"--"+this.boundary+s});this._add=this.add;this.add=this.addPart}a(f,n);f.prototype.addPart=function(e){e=e||{};var t=new i.PassThrough;if(e.headers){for(var r in e.headers){var n=e.headers[r];t.write(r+": "+n+s)}}t.write(s);if(o(e.body)){e.body.pipe(t)}else{t.end(e.body)}this._add(t)}},{inherits:57,"is-stream":68,"sandwich-stream":132,stream:134}],99:[function(e,t,r){t.exports=function n(e,t){if(typeof e!=="string"){throw new TypeError("expected a string")}e=e.replace(/[\\\/]+/g,"/");if(t!==false){e=e.replace(/\/$/,"")}return e}},{}],100:[function(e,t,r){"use strict";var n=Object.prototype.propertyIsEnumerable;function i(e){if(e==null){throw new TypeError("Object.assign cannot be called with null or undefined")}return Object(e)}function a(e){var t=Object.getOwnPropertyNames(e);if(Object.getOwnPropertySymbols){t=t.concat(Object.getOwnPropertySymbols(e))}return t.filter(function(t){return n.call(e,t)})}t.exports=Object.assign||function(e,t){var r;var n;var o=i(e);for(var s=1;s<arguments.length;s++){r=arguments[s];n=a(Object(r));for(var f=0;f<n.length;f++){o[n[f]]=r[n[f]]}}return o}},{}],101:[function(e,t,r){"use strict";var n=Object.prototype.hasOwnProperty;var i=Object.prototype.toString;var a=Array.prototype.slice;var o=e("./isArguments");var s=!{toString:null}.propertyIsEnumerable("toString");var f=function(){}.propertyIsEnumerable("prototype");var u=["toString","toLocaleString","valueOf","hasOwnProperty","isPrototypeOf","propertyIsEnumerable","constructor"];var c=function(e){var t=e.constructor;return t&&t.prototype===e};var l={$console:true,$frame:true,$frameElement:true,$frames:true,$parent:true,$self:true,$webkitIndexedDB:true,$webkitStorageInfo:true,$window:true};var h=function(){if(typeof window==="undefined"){return false}for(var e in window){try{if(!l["$"+e]&&n.call(window,e)&&window[e]!==null&&typeof window[e]==="object"){try{c(window[e])}catch(t){return true}}}catch(t){return true}}return false}();var p=function(e){if(typeof window==="undefined"||!h){return c(e)}try{return c(e)}catch(t){return false}};var d=function v(e){var t=e!==null&&typeof e==="object";var r=i.call(e)==="[object Function]";var a=o(e);var c=t&&i.call(e)==="[object String]";var l=[];if(!t&&!r&&!a){throw new TypeError("Object.keys called on a non-object")}var h=f&&r;if(c&&e.length>0&&!n.call(e,0)){for(var d=0;d<e.length;++d){l.push(String(d))}}if(a&&e.length>0){for(var v=0;v<e.length;++v){l.push(String(v))}}else{for(var g in e){if(!(h&&g==="prototype")&&n.call(e,g)){l.push(String(g))}}}if(s){var y=p(e);for(var b=0;b<u.length;++b){if(!(y&&u[b]==="constructor")&&n.call(e,u[b])){l.push(u[b])}}}return l};d.shim=function g(){if(Object.keys){var e=function(){return(Object.keys(arguments)||"").length===2}(1,2);if(!e){var t=Object.keys;Object.keys=function r(e){if(o(e)){return t(a.call(e))}else{return t(e)}}}}else{Object.keys=d}return Object.keys||d};t.exports=d},{"./isArguments":102}],102:[function(e,t,r){"use strict";var n=Object.prototype.toString;t.exports=function i(e){var t=n.call(e);var r=t==="[object Arguments]";if(!r){r=t!=="[object Array]"&&e!==null&&typeof e==="object"&&typeof e.length==="number"&&e.length>=0&&n.call(e.callee)==="[object Function]"}return r}},{}],103:[function(e,t,r){"use strict";var n=e("is-extendable");var i=e("for-own");t.exports=function a(e,t){if(!n(e))return{};var t=[].concat.apply([],[].slice.call(arguments,1));var r=t[t.length-1];var a={},o;if(typeof r==="function"){o=t.pop()}var s=typeof o==="function";if(!t.length&&!s){return e}i(e,function(r,n){if(t.indexOf(n)===-1){if(!s){a[n]=r}else if(o(r,n,e)){a[n]=r}}});return a}},{"for-own":31,"is-extendable":62}],104:[function(e,t,r){var n=e("wrappy");t.exports=n(i);i.proto=i(function(){Object.defineProperty(Function.prototype,"once",{value:function(){return i(this)},configurable:true})});function i(e){var t=function(){if(t.called)return t.value;t.called=true;return t.value=e.apply(this,arguments)};t.called=false;return t}},{wrappy:187}],105:[function(e,t,r){var n=e("readable-stream/readable");var i=e("is-stream").readable;var a=e("util");function o(e,t){if(!i(t))throw new Error("All input streams must be readable");var r=this;t._buffer=[];t.on("readable",function(){var n=t.read();if(n===null)return;if(this===e[0])r.push(n);else this._buffer.push(n)});t.on("end",function(){for(var t=e[0];t&&t._readableState.ended;t=e[0]){while(t._buffer.length)r.push(t._buffer.shift());e.shift()}if(!e.length)r.push(null)});t.on("error",this.emit.bind(this,"error"));e.push(t)}function s(e,t){if(!(this instanceof s)){return new s(e,t)}e=e||[];t=t||{};t.objectMode=true;n.call(this,t);if(!Array.isArray(e))e=[e];if(!e.length)return this.push(null);var r=o.bind(this,[]);e.forEach(function(e){if(Array.isArray(e))e.forEach(r);else r(e)})}a.inherits(s,n);s.prototype._read=function(){};t.exports=s},{"is-stream":68,"readable-stream/readable":125,util:148}],106:[function(e,t,r){r.endianness=function(){return"LE"};r.hostname=function(){if(typeof location!=="undefined"){return location.hostname}else return""};r.loadavg=function(){return[]};r.uptime=function(){return 0};r.freemem=function(){return Number.MAX_VALUE};r.totalmem=function(){return Number.MAX_VALUE};r.cpus=function(){return[]};r.type=function(){return"Browser"};r.release=function(){if(typeof navigator!=="undefined"){return navigator.appVersion}return""};r.networkInterfaces=r.getNetworkInterfaces=function(){return{}};r.arch=function(){return"javascript"};r.platform=function(){return"browser"};r.tmpdir=r.tmpDir=function(){return"/tmp"};r.EOL="\n"},{}],107:[function(e,t,r){"use strict";var n=e("is-glob");var i=e("glob-base");var a=e("is-extglob");var o=e("is-dotfile");var s=t.exports.cache={};t.exports=function p(e){if(s.hasOwnProperty(e)){return s[e]}var t={};t.orig=e;t.is={};e=c(e);var r=i(e);t.is.glob=r.isGlob;t.glob=r.glob;t.base=r.base;var l=/([^\/]*)$/.exec(e);t.path={};t.path.dirname="";t.path.basename=l[1]||"";t.path.dirname=e.split(t.path.basename).join("")||"";var p=(t.path.basename||"").split(".")||"";t.path.filename=p[0]||"";t.path.extname=p.slice(1).join(".")||"";t.path.ext="";if(n(t.path.dirname)&&!t.path.basename){if(!/\/$/.test(t.glob)){t.path.basename=t.glob}t.path.dirname=t.base;
}if(e.indexOf("/")===-1&&!t.is.globstar){t.path.dirname="";t.path.basename=t.orig}var d=t.path.basename.indexOf(".");if(d!==-1){t.path.filename=t.path.basename.slice(0,d);t.path.extname=t.path.basename.slice(d)}if(t.path.extname.charAt(0)==="."){var v=t.path.extname.split(".");t.path.ext=v[v.length-1]}t.glob=h(t.glob);t.path.dirname=h(t.path.dirname);t.path.basename=h(t.path.basename);t.path.filename=h(t.path.filename);t.path.extname=h(t.path.extname);var g=e&&t.is.glob;t.is.negated=e&&e.charAt(0)==="!";t.is.extglob=e&&a(e);t.is.braces=u(g,e,"{");t.is.brackets=u(g,e,"[:");t.is.globstar=u(g,e,"**");t.is.dotfile=o(t.path.basename)||o(t.path.filename);t.is.dotdir=f(t.path.dirname);return s[e]=t};function f(e){if(e.indexOf("/.")!==-1){return true}if(e.charAt(0)==="."&&e.charAt(1)!=="/"){return true}return false}function u(e,t,r){return e&&t.indexOf(r)!==-1}function c(e){var t=/\{([^{}]*?)}|\(([^()]*?)\)|\[([^\[\]]*?)\]/g;return e.replace(t,function(e,t,r,n){var i=t||r||n;if(!i){return e}return e.split(i).join(l(i))})}function l(e){e=e.split("/").join("__SLASH__");e=e.split(".").join("__DOT__");return e}function h(e){e=e.split("__SLASH__").join("/");e=e.split("__DOT__").join(".");return e}},{"glob-base":35,"is-dotfile":60,"is-extglob":63,"is-glob":64}],108:[function(e,t,r){(function(e){function t(e,t){var r=0;for(var n=e.length-1;n>=0;n--){var i=e[n];if(i==="."){e.splice(n,1)}else if(i===".."){e.splice(n,1);r++}else if(r){e.splice(n,1);r--}}if(t){for(;r--;r){e.unshift("..")}}return e}var n=/^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;var i=function(e){return n.exec(e).slice(1)};r.resolve=function(){var r="",n=false;for(var i=arguments.length-1;i>=-1&&!n;i--){var o=i>=0?arguments[i]:e.cwd();if(typeof o!=="string"){throw new TypeError("Arguments to path.resolve must be strings")}else if(!o){continue}r=o+"/"+r;n=o.charAt(0)==="/"}r=t(a(r.split("/"),function(e){return!!e}),!n).join("/");return(n?"/":"")+r||"."};r.normalize=function(e){var n=r.isAbsolute(e),i=o(e,-1)==="/";e=t(a(e.split("/"),function(e){return!!e}),!n).join("/");if(!e&&!n){e="."}if(e&&i){e+="/"}return(n?"/":"")+e};r.isAbsolute=function(e){return e.charAt(0)==="/"};r.join=function(){var e=Array.prototype.slice.call(arguments,0);return r.normalize(a(e,function(e,t){if(typeof e!=="string"){throw new TypeError("Arguments to path.join must be strings")}return e}).join("/"))};r.relative=function(e,t){e=r.resolve(e).substr(1);t=r.resolve(t).substr(1);function n(e){var t=0;for(;t<e.length;t++){if(e[t]!=="")break}var r=e.length-1;for(;r>=0;r--){if(e[r]!=="")break}if(t>r)return[];return e.slice(t,r-t+1)}var i=n(e.split("/"));var a=n(t.split("/"));var o=Math.min(i.length,a.length);var s=o;for(var f=0;f<o;f++){if(i[f]!==a[f]){s=f;break}}var u=[];for(var f=s;f<i.length;f++){u.push("..")}u=u.concat(a.slice(s));return u.join("/")};r.sep="/";r.delimiter=":";r.dirname=function(e){var t=i(e),r=t[0],n=t[1];if(!r&&!n){return"."}if(n){n=n.substr(0,n.length-1)}return r+n};r.basename=function(e,t){var r=i(e)[2];if(t&&r.substr(-1*t.length)===t){r=r.substr(0,r.length-t.length)}return r};r.extname=function(e){return i(e)[3]};function a(e,t){if(e.filter)return e.filter(t);var r=[];for(var n=0;n<e.length;n++){if(t(e[n],n,e))r.push(e[n])}return r}var o="ab".substr(-1)==="b"?function(e,t,r){return e.substr(t,r)}:function(e,t,r){if(t<0)t=e.length+t;return e.substr(t,r)}}).call(this,e("_process"))},{_process:112}],109:[function(e,t,r){(function(e){"use strict";function r(e){return e.charAt(0)==="/"}function n(e){var t=/^([a-zA-Z]:|[\\\/]{2}[^\\\/]+[\\\/]+[^\\\/]+)?([\\\/])?([\s\S]*?)$/;var r=t.exec(e);var n=r[1]||"";var i=!!n&&n.charAt(1)!==":";return!!r[2]||i}t.exports=e.platform==="win32"?n:r;t.exports.posix=r;t.exports.win32=n}).call(this,e("_process"))},{_process:112}],110:[function(e,t,r){"use strict";r.before=function a(e,t){return e.replace(t,function(e){var t=n();i[t]=e;return"__ID"+t+"__"})};r.after=function o(e){return e.replace(/__ID(.{5})__/g,function(e,t){return i[t]})};function n(){return Math.random().toString().slice(2,7)}var i={}},{}],111:[function(e,t,r){(function(e){"use strict";if(!e.version||e.version.indexOf("v0.")===0||e.version.indexOf("v1.")===0&&e.version.indexOf("v1.8.")!==0){t.exports=r}else{t.exports=e.nextTick}function r(t,r,n,i){if(typeof t!=="function"){throw new TypeError('"callback" argument must be a function')}var a=arguments.length;var o,s;switch(a){case 0:case 1:return e.nextTick(t);case 2:return e.nextTick(function f(){t.call(null,r)});case 3:return e.nextTick(function u(){t.call(null,r,n)});case 4:return e.nextTick(function c(){t.call(null,r,n,i)});default:o=new Array(a-1);s=0;while(s<o.length){o[s++]=arguments[s]}return e.nextTick(function l(){t.apply(null,o)})}}}).call(this,e("_process"))},{_process:112}],112:[function(e,t,r){var n=t.exports={};var i;var a;(function(){try{i=setTimeout}catch(e){i=function(){throw new Error("setTimeout is not defined")}}try{a=clearTimeout}catch(e){a=function(){throw new Error("clearTimeout is not defined")}}})();var o=[];var s=false;var f;var u=-1;function c(){if(!s||!f){return}s=false;if(f.length){o=f.concat(o)}else{u=-1}if(o.length){l()}}function l(){if(s){return}var e=i(c);s=true;var t=o.length;while(t){f=o;o=[];while(++u<t){if(f){f[u].run()}}u=-1;t=o.length}f=null;s=false;a(e)}n.nextTick=function(e){var t=new Array(arguments.length-1);if(arguments.length>1){for(var r=1;r<arguments.length;r++){t[r-1]=arguments[r]}}o.push(new h(e,t));if(o.length===1&&!s){i(l,0)}};function h(e,t){this.fun=e;this.array=t}h.prototype.run=function(){this.fun.apply(null,this.array)};n.title="browser";n.browser=true;n.env={};n.argv=[];n.version="";n.versions={};function p(){}n.on=p;n.addListener=p;n.once=p;n.off=p;n.removeListener=p;n.removeAllListeners=p;n.emit=p;n.binding=function(e){throw new Error("process.binding is not supported")};n.cwd=function(){return"/"};n.chdir=function(e){throw new Error("process.chdir is not supported")};n.umask=function(){return 0}},{}],113:[function(t,r,n){(function(t){(function(i){var a=typeof n=="object"&&n&&!n.nodeType&&n;var o=typeof r=="object"&&r&&!r.nodeType&&r;var s=typeof t=="object"&&t;if(s.global===s||s.window===s||s.self===s){i=s}var f,u=2147483647,c=36,l=1,h=26,p=38,d=700,v=72,g=128,y="-",b=/^xn--/,m=/[^\x20-\x7E]/,_=/[\x2E\u3002\uFF0E\uFF61]/g,w={overflow:"Overflow: input needs wider integers to process","not-basic":"Illegal input >= 0x80 (not a basic code point)","invalid-input":"Invalid input"},E=c-l,S=Math.floor,x=String.fromCharCode,O;function j(e){throw new RangeError(w[e])}function A(e,t){var r=e.length;var n=[];while(r--){n[r]=t(e[r])}return n}function k(e,t){var r=e.split("@");var n="";if(r.length>1){n=r[0]+"@";e=r[1]}e=e.replace(_,".");var i=e.split(".");var a=A(i,t).join(".");return n+a}function R(e){var t=[],r=0,n=e.length,i,a;while(r<n){i=e.charCodeAt(r++);if(i>=55296&&i<=56319&&r<n){a=e.charCodeAt(r++);if((a&64512)==56320){t.push(((i&1023)<<10)+(a&1023)+65536)}else{t.push(i);r--}}else{t.push(i)}}return t}function T(e){return A(e,function(e){var t="";if(e>65535){e-=65536;t+=x(e>>>10&1023|55296);e=56320|e&1023}t+=x(e);return t}).join("")}function L(e){if(e-48<10){return e-22}if(e-65<26){return e-65}if(e-97<26){return e-97}return c}function I(e,t){return e+22+75*(e<26)-((t!=0)<<5)}function C(e,t,r){var n=0;e=r?S(e/d):e>>1;e+=S(e/t);for(;e>E*h>>1;n+=c){e=S(e/E)}return S(n+(E+1)*e/(e+p))}function P(e){var t=[],r=e.length,n,i=0,a=g,o=v,s,f,p,d,b,m,_,w,E;s=e.lastIndexOf(y);if(s<0){s=0}for(f=0;f<s;++f){if(e.charCodeAt(f)>=128){j("not-basic")}t.push(e.charCodeAt(f))}for(p=s>0?s+1:0;p<r;){for(d=i,b=1,m=c;;m+=c){if(p>=r){j("invalid-input")}_=L(e.charCodeAt(p++));if(_>=c||_>S((u-i)/b)){j("overflow")}i+=_*b;w=m<=o?l:m>=o+h?h:m-o;if(_<w){break}E=c-w;if(b>S(u/E)){j("overflow")}b*=E}n=t.length+1;o=C(i-d,n,d==0);if(S(i/n)>u-a){j("overflow")}a+=S(i/n);i%=n;t.splice(i++,0,a)}return T(t)}function M(e){var t,r,n,i,a,o,s,f,p,d,b,m=[],_,w,E,O;e=R(e);_=e.length;t=g;r=0;a=v;for(o=0;o<_;++o){b=e[o];if(b<128){m.push(x(b))}}n=i=m.length;if(i){m.push(y)}while(n<_){for(s=u,o=0;o<_;++o){b=e[o];if(b>=t&&b<s){s=b}}w=n+1;if(s-t>S((u-r)/w)){j("overflow")}r+=(s-t)*w;t=s;for(o=0;o<_;++o){b=e[o];if(b<t&&++r>u){j("overflow")}if(b==t){for(f=r,p=c;;p+=c){d=p<=a?l:p>=a+h?h:p-a;if(f<d){break}O=f-d;E=c-d;m.push(x(I(d+O%E,0)));f=S(O/E)}m.push(x(I(f,0)));a=C(r,w,n==i);r=0;++n}}++r;++t}return m.join("")}function N(e){return k(e,function(e){return b.test(e)?P(e.slice(4).toLowerCase()):e})}function B(e){return k(e,function(e){return m.test(e)?"xn--"+M(e):e})}f={version:"1.4.1",ucs2:{decode:R,encode:T},decode:P,encode:M,toASCII:B,toUnicode:N};if(typeof e=="function"&&typeof e.amd=="object"&&e.amd){e("punycode",function(){return f})}else if(a&&o){if(r.exports==a){o.exports=f}else{for(O in f){f.hasOwnProperty(O)&&(a[O]=f[O])}}}else{i.punycode=f}})(this)}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{}],114:[function(e,t,r){"use strict";function n(e,t){return Object.prototype.hasOwnProperty.call(e,t)}t.exports=function(e,t,r,a){t=t||"&";r=r||"=";var o={};if(typeof e!=="string"||e.length===0){return o}var s=/\+/g;e=e.split(t);var f=1e3;if(a&&typeof a.maxKeys==="number"){f=a.maxKeys}var u=e.length;if(f>0&&u>f){u=f}for(var c=0;c<u;++c){var l=e[c].replace(s,"%20"),h=l.indexOf(r),p,d,v,g;if(h>=0){p=l.substr(0,h);d=l.substr(h+1)}else{p=l;d=""}v=decodeURIComponent(p);g=decodeURIComponent(d);if(!n(o,v)){o[v]=g}else if(i(o[v])){o[v].push(g)}else{o[v]=[o[v],g]}}return o};var i=Array.isArray||function(e){return Object.prototype.toString.call(e)==="[object Array]"}},{}],115:[function(e,t,r){"use strict";var n=function(e){switch(typeof e){case"string":return e;case"boolean":return e?"true":"false";case"number":return isFinite(e)?e:"";default:return""}};t.exports=function(e,t,r,s){t=t||"&";r=r||"=";if(e===null){e=undefined}if(typeof e==="object"){return a(o(e),function(o){var s=encodeURIComponent(n(o))+r;if(i(e[o])){return a(e[o],function(e){return s+encodeURIComponent(n(e))}).join(t)}else{return s+encodeURIComponent(n(e[o]))}}).join(t)}if(!s)return"";return encodeURIComponent(n(s))+r+encodeURIComponent(n(e))};var i=Array.isArray||function(e){return Object.prototype.toString.call(e)==="[object Array]"};function a(e,t){if(e.map)return e.map(t);var r=[];for(var n=0;n<e.length;n++){r.push(t(e[n],n))}return r}var o=Object.keys||function(e){var t=[];for(var r in e){if(Object.prototype.hasOwnProperty.call(e,r))t.push(r)}return t}},{}],116:[function(e,t,r){"use strict";r.decode=r.parse=e("./decode");r.encode=r.stringify=e("./encode")},{"./decode":114,"./encode":115}],117:[function(e,t,r){"use strict";var n=e("is-number");var i=e("kind-of");t.exports=o;var a={lower:"abcdefghijklmnopqrstuvwxyz",upper:"ABCDEFGHIJKLMNOPQRSTUVWXYZ",number:"0123456789",special:"~!@#$%^&()_+-={}[];',."};a.all=a.lower+a.upper+a.number;function o(e,t,r){if(typeof e==="undefined"){throw new Error("randomatic expects a string or number.")}var o=false;if(arguments.length===1){if(typeof e==="string"){t=e.length}else if(n(e)){r={};t=e;e="*"}}if(i(t)==="object"&&t.hasOwnProperty("chars")){r=t;e=r.chars;t=e.length;o=true}var s=r||{};var f="";var u="";if(e.indexOf("?")!==-1)f+=s.chars;if(e.indexOf("a")!==-1)f+=a.lower;if(e.indexOf("A")!==-1)f+=a.upper;if(e.indexOf("0")!==-1)f+=a.number;if(e.indexOf("!")!==-1)f+=a.special;if(e.indexOf("*")!==-1)f+=a.all;if(o)f+=e;while(t--){u+=f.charAt(parseInt(Math.random()*f.length,10))}return u}},{"is-number":65,"kind-of":76}],118:[function(e,t,r){t.exports=e("./lib/_stream_duplex.js")},{"./lib/_stream_duplex.js":119}],119:[function(e,t,r){"use strict";var n=Object.keys||function(e){var t=[];for(var r in e){t.push(r)}return t};t.exports=l;var i=e("process-nextick-args");var a=e("core-util-is");a.inherits=e("inherits");var o=e("./_stream_readable");var s=e("./_stream_writable");a.inherits(l,o);var f=n(s.prototype);for(var u=0;u<f.length;u++){var c=f[u];if(!l.prototype[c])l.prototype[c]=s.prototype[c]}function l(e){if(!(this instanceof l))return new l(e);o.call(this,e);s.call(this,e);if(e&&e.readable===false)this.readable=false;if(e&&e.writable===false)this.writable=false;this.allowHalfOpen=true;if(e&&e.allowHalfOpen===false)this.allowHalfOpen=false;this.once("end",h)}function h(){if(this.allowHalfOpen||this._writableState.ended)return;i(p,this)}function p(e){e.end()}function d(e,t){for(var r=0,n=e.length;r<n;r++){t(e[r],r)}}},{"./_stream_readable":121,"./_stream_writable":123,"core-util-is":19,inherits:57,"process-nextick-args":111}],120:[function(e,t,r){"use strict";t.exports=a;var n=e("./_stream_transform");var i=e("core-util-is");i.inherits=e("inherits");i.inherits(a,n);function a(e){if(!(this instanceof a))return new a(e);n.call(this,e)}a.prototype._transform=function(e,t,r){r(null,e)}},{"./_stream_transform":122,"core-util-is":19,inherits:57}],121:[function(e,t,r){(function(r){"use strict";t.exports=v;var n=e("process-nextick-args");var i=e("isarray");var a=e("buffer").Buffer;v.ReadableState=d;var o=e("events");var s=function(e,t){return e.listeners(t).length};var f;(function(){try{f=e("st"+"ream")}catch(t){}finally{if(!f)f=e("events").EventEmitter}})();var a=e("buffer").Buffer;var u=e("core-util-is");u.inherits=e("inherits");var c=e("util");var l=undefined;if(c&&c.debuglog){l=c.debuglog("stream")}else{l=function(){}}var h;u.inherits(v,f);var p;function d(t,r){p=p||e("./_stream_duplex");t=t||{};this.objectMode=!!t.objectMode;if(r instanceof p)this.objectMode=this.objectMode||!!t.readableObjectMode;var n=t.highWaterMark;var i=this.objectMode?16:16*1024;this.highWaterMark=n||n===0?n:i;this.highWaterMark=~~this.highWaterMark;this.buffer=[];this.length=0;this.pipes=null;this.pipesCount=0;this.flowing=null;this.ended=false;this.endEmitted=false;this.reading=false;this.sync=true;this.needReadable=false;this.emittedReadable=false;this.readableListening=false;this.resumeScheduled=false;this.defaultEncoding=t.defaultEncoding||"utf8";this.ranOut=false;this.awaitDrain=0;this.readingMore=false;this.decoder=null;this.encoding=null;if(t.encoding){if(!h)h=e("string_decoder/").StringDecoder;this.decoder=new h(t.encoding);this.encoding=t.encoding}}var p;function v(t){p=p||e("./_stream_duplex");if(!(this instanceof v))return new v(t);this._readableState=new d(t,this);this.readable=true;if(t&&typeof t.read==="function")this._read=t.read;f.call(this)}v.prototype.push=function(e,t){var r=this._readableState;if(!r.objectMode&&typeof e==="string"){t=t||r.defaultEncoding;if(t!==r.encoding){e=new a(e,t);t=""}}return g(this,r,e,t,false)};v.prototype.unshift=function(e){var t=this._readableState;return g(this,t,e,"",true)};v.prototype.isPaused=function(){return this._readableState.flowing===false};function g(e,t,r,n,i){var a=w(t,r);if(a){e.emit("error",a)}else if(r===null){t.reading=false;E(e,t)}else if(t.objectMode||r&&r.length>0){if(t.ended&&!i){var o=new Error("stream.push() after EOF");e.emit("error",o)}else if(t.endEmitted&&i){var o=new Error("stream.unshift() after end event");e.emit("error",o)}else{var s;if(t.decoder&&!i&&!n){r=t.decoder.write(r);s=!t.objectMode&&r.length===0}if(!i)t.reading=false;if(!s){if(t.flowing&&t.length===0&&!t.sync){e.emit("data",r);e.read(0)}else{t.length+=t.objectMode?1:r.length;if(i)t.buffer.unshift(r);else t.buffer.push(r);if(t.needReadable)S(e)}}O(e,t)}}else if(!i){t.reading=false}return y(t)}function y(e){return!e.ended&&(e.needReadable||e.length<e.highWaterMark||e.length===0)}v.prototype.setEncoding=function(t){if(!h)h=e("string_decoder/").StringDecoder;this._readableState.decoder=new h(t);this._readableState.encoding=t;return this};var b=8388608;function m(e){if(e>=b){e=b}else{e--;e|=e>>>1;e|=e>>>2;e|=e>>>4;e|=e>>>8;e|=e>>>16;e++}return e}function _(e,t){if(t.length===0&&t.ended)return 0;if(t.objectMode)return e===0?0:1;if(e===null||isNaN(e)){if(t.flowing&&t.buffer.length)return t.buffer[0].length;else return t.length}if(e<=0)return 0;if(e>t.highWaterMark)t.highWaterMark=m(e);if(e>t.length){if(!t.ended){t.needReadable=true;return 0}else{return t.length}}return e}v.prototype.read=function(e){l("read",e);var t=this._readableState;var r=e;if(typeof e!=="number"||e>0)t.emittedReadable=false;if(e===0&&t.needReadable&&(t.length>=t.highWaterMark||t.ended)){l("read: emitReadable",t.length,t.ended);if(t.length===0&&t.ended)C(this);else S(this);return null}e=_(e,t);if(e===0&&t.ended){if(t.length===0)C(this);return null}var n=t.needReadable;l("need readable",n);if(t.length===0||t.length-e<t.highWaterMark){n=true;l("length less than watermark",n)}if(t.ended||t.reading){n=false;l("reading or ended",n)}if(n){l("do read");t.reading=true;t.sync=true;if(t.length===0)t.needReadable=true;this._read(t.highWaterMark);t.sync=false}if(n&&!t.reading)e=_(r,t);var i;if(e>0)i=I(e,t);else i=null;if(i===null){t.needReadable=true;e=0}t.length-=e;if(t.length===0&&!t.ended)t.needReadable=true;if(r!==e&&t.ended&&t.length===0)C(this);if(i!==null)this.emit("data",i);return i};function w(e,t){var r=null;if(!a.isBuffer(t)&&typeof t!=="string"&&t!==null&&t!==undefined&&!e.objectMode){r=new TypeError("Invalid non-string/buffer chunk")}return r}function E(e,t){if(t.ended)return;if(t.decoder){var r=t.decoder.end();if(r&&r.length){t.buffer.push(r);t.length+=t.objectMode?1:r.length}}t.ended=true;S(e)}function S(e){var t=e._readableState;t.needReadable=false;if(!t.emittedReadable){l("emitReadable",t.flowing);t.emittedReadable=true;if(t.sync)n(x,e);else x(e)}}function x(e){l("emit readable");e.emit("readable");L(e)}function O(e,t){if(!t.readingMore){t.readingMore=true;n(j,e,t)}}function j(e,t){var r=t.length;while(!t.reading&&!t.flowing&&!t.ended&&t.length<t.highWaterMark){l("maybeReadMore read 0");e.read(0);if(r===t.length)break;else r=t.length}t.readingMore=false}v.prototype._read=function(e){this.emit("error",new Error("not implemented"))};v.prototype.pipe=function(e,t){var a=this;var o=this._readableState;switch(o.pipesCount){case 0:o.pipes=e;break;case 1:o.pipes=[o.pipes,e];break;default:o.pipes.push(e);break}o.pipesCount+=1;l("pipe count=%d opts=%j",o.pipesCount,t);var f=(!t||t.end!==false)&&e!==r.stdout&&e!==r.stderr;var u=f?h:v;if(o.endEmitted)n(u);else a.once("end",u);e.on("unpipe",c);function c(e){l("onunpipe");if(e===a){v()}}function h(){l("onend");e.end()}var p=A(a);e.on("drain",p);var d=false;function v(){l("cleanup");e.removeListener("close",b);e.removeListener("finish",m);e.removeListener("drain",p);e.removeListener("error",y);e.removeListener("unpipe",c);a.removeListener("end",h);a.removeListener("end",v);a.removeListener("data",g);d=true;if(o.awaitDrain&&(!e._writableState||e._writableState.needDrain))p()}a.on("data",g);function g(t){l("ondata");var r=e.write(t);if(false===r){if(o.pipesCount===1&&o.pipes[0]===e&&a.listenerCount("data")===1&&!d){l("false write response, pause",a._readableState.awaitDrain);a._readableState.awaitDrain++}a.pause()}}function y(t){l("onerror",t);_();e.removeListener("error",y);if(s(e,"error")===0)e.emit("error",t)}if(!e._events||!e._events.error)e.on("error",y);else if(i(e._events.error))e._events.error.unshift(y);else e._events.error=[y,e._events.error];function b(){e.removeListener("finish",m);_()}e.once("close",b);function m(){l("onfinish");e.removeListener("close",b);_()}e.once("finish",m);function _(){l("unpipe");a.unpipe(e)}e.emit("pipe",a);if(!o.flowing){l("pipe resume");a.resume()}return e};function A(e){return function(){var t=e._readableState;l("pipeOnDrain",t.awaitDrain);if(t.awaitDrain)t.awaitDrain--;if(t.awaitDrain===0&&s(e,"data")){t.flowing=true;L(e)}}}v.prototype.unpipe=function(e){var t=this._readableState;if(t.pipesCount===0)return this;if(t.pipesCount===1){if(e&&e!==t.pipes)return this;if(!e)e=t.pipes;t.pipes=null;t.pipesCount=0;t.flowing=false;if(e)e.emit("unpipe",this);return this}if(!e){var r=t.pipes;var n=t.pipesCount;t.pipes=null;t.pipesCount=0;t.flowing=false;for(var i=0;i<n;i++){r[i].emit("unpipe",this)}return this}var a=N(t.pipes,e);if(a===-1)return this;t.pipes.splice(a,1);t.pipesCount-=1;if(t.pipesCount===1)t.pipes=t.pipes[0];e.emit("unpipe",this);return this};v.prototype.on=function(e,t){var r=f.prototype.on.call(this,e,t);if(e==="data"&&false!==this._readableState.flowing){this.resume()}if(e==="readable"&&!this._readableState.endEmitted){var i=this._readableState;if(!i.readableListening){i.readableListening=true;i.emittedReadable=false;i.needReadable=true;if(!i.reading){n(k,this)}else if(i.length){S(this,i)}}}return r};v.prototype.addListener=v.prototype.on;function k(e){l("readable nexttick read 0");e.read(0)}v.prototype.resume=function(){var e=this._readableState;if(!e.flowing){l("resume");e.flowing=true;R(this,e)}return this};function R(e,t){if(!t.resumeScheduled){t.resumeScheduled=true;n(T,e,t)}}function T(e,t){if(!t.reading){l("resume read 0");e.read(0)}t.resumeScheduled=false;e.emit("resume");L(e);if(t.flowing&&!t.reading)e.read(0)}v.prototype.pause=function(){l("call pause flowing=%j",this._readableState.flowing);if(false!==this._readableState.flowing){l("pause");this._readableState.flowing=false;this.emit("pause")}return this};function L(e){var t=e._readableState;l("flow",t.flowing);if(t.flowing){do{var r=e.read()}while(null!==r&&t.flowing)}}v.prototype.wrap=function(e){var t=this._readableState;var r=false;var n=this;e.on("end",function(){l("wrapped end");if(t.decoder&&!t.ended){var e=t.decoder.end();if(e&&e.length)n.push(e)}n.push(null)});e.on("data",function(i){l("wrapped data");if(t.decoder)i=t.decoder.write(i);if(t.objectMode&&(i===null||i===undefined))return;else if(!t.objectMode&&(!i||!i.length))return;var a=n.push(i);if(!a){r=true;e.pause()}});for(var i in e){if(this[i]===undefined&&typeof e[i]==="function"){this[i]=function(t){return function(){return e[t].apply(e,arguments)}}(i)}}var a=["error","close","destroy","pause","resume"];M(a,function(t){e.on(t,n.emit.bind(n,t))});n._read=function(t){l("wrapped _read",t);if(r){r=false;e.resume()}};return n};v._fromList=I;function I(e,t){var r=t.buffer;var n=t.length;var i=!!t.decoder;var o=!!t.objectMode;var s;if(r.length===0)return null;if(n===0)s=null;else if(o)s=r.shift();else if(!e||e>=n){if(i)s=r.join("");else if(r.length===1)s=r[0];else s=a.concat(r,n);r.length=0}else{if(e<r[0].length){var f=r[0];s=f.slice(0,e);r[0]=f.slice(e)}else if(e===r[0].length){s=r.shift()}else{if(i)s="";else s=new a(e);var u=0;for(var c=0,l=r.length;c<l&&u<e;c++){var f=r[0];var h=Math.min(e-u,f.length);if(i)s+=f.slice(0,h);else f.copy(s,u,0,h);if(h<f.length)r[0]=f.slice(h);else r.shift();u+=h}}}return s}function C(e){var t=e._readableState;if(t.length>0)throw new Error("endReadable called on non-empty stream");if(!t.endEmitted){t.ended=true;n(P,t,e)}}function P(e,t){if(!e.endEmitted&&e.length===0){e.endEmitted=true;t.readable=false;t.emit("end")}}function M(e,t){for(var r=0,n=e.length;r<n;r++){t(e[r],r)}}function N(e,t){for(var r=0,n=e.length;r<n;r++){if(e[r]===t)return r}return-1}}).call(this,e("_process"))},{"./_stream_duplex":119,_process:112,buffer:13,"core-util-is":19,events:22,inherits:57,isarray:70,"process-nextick-args":111,"string_decoder/":139,util:10}],122:[function(e,t,r){"use strict";t.exports=s;var n=e("./_stream_duplex");var i=e("core-util-is");i.inherits=e("inherits");i.inherits(s,n);function a(e){this.afterTransform=function(t,r){return o(e,t,r)};this.needTransform=false;this.transforming=false;this.writecb=null;this.writechunk=null;this.writeencoding=null}function o(e,t,r){var n=e._transformState;n.transforming=false;var i=n.writecb;if(!i)return e.emit("error",new Error("no writecb in Transform class"));n.writechunk=null;n.writecb=null;if(r!==null&&r!==undefined)e.push(r);i(t);var a=e._readableState;a.reading=false;if(a.needReadable||a.length<a.highWaterMark){e._read(a.highWaterMark)}}function s(e){if(!(this instanceof s))return new s(e);n.call(this,e);this._transformState=new a(this);var t=this;this._readableState.needReadable=true;this._readableState.sync=false;if(e){if(typeof e.transform==="function")this._transform=e.transform;if(typeof e.flush==="function")this._flush=e.flush}this.once("prefinish",function(){if(typeof this._flush==="function")this._flush(function(e){f(t,e)});else f(t)})}s.prototype.push=function(e,t){this._transformState.needTransform=false;return n.prototype.push.call(this,e,t)};s.prototype._transform=function(e,t,r){throw new Error("not implemented")};s.prototype._write=function(e,t,r){var n=this._transformState;n.writecb=r;n.writechunk=e;n.writeencoding=t;if(!n.transforming){var i=this._readableState;if(n.needTransform||i.needReadable||i.length<i.highWaterMark)this._read(i.highWaterMark)}};s.prototype._read=function(e){var t=this._transformState;if(t.writechunk!==null&&t.writecb&&!t.transforming){t.transforming=true;this._transform(t.writechunk,t.writeencoding,t.afterTransform)}else{t.needTransform=true}};function f(e,t){if(t)return e.emit("error",t);var r=e._writableState;var n=e._transformState;if(r.length)throw new Error("calling transform done when ws.length != 0");if(n.transforming)throw new Error("calling transform done when still transforming");return e.push(null)}},{"./_stream_duplex":119,"core-util-is":19,inherits:57}],123:[function(e,t,r){(function(r){"use strict";t.exports=p;var n=e("process-nextick-args");var i=!r.browser&&["v0.10","v0.9."].indexOf(r.version.slice(0,5))>-1?setImmediate:n;var a=e("buffer").Buffer;p.WritableState=h;var o=e("core-util-is");o.inherits=e("inherits");var s={deprecate:e("util-deprecate")};var f;(function(){try{f=e("st"+"ream")}catch(t){}finally{if(!f)f=e("events").EventEmitter}})();var a=e("buffer").Buffer;o.inherits(p,f);function u(){}function c(e,t,r){this.chunk=e;this.encoding=t;this.callback=r;this.next=null}var l;function h(t,r){l=l||e("./_stream_duplex");t=t||{};this.objectMode=!!t.objectMode;if(r instanceof l)this.objectMode=this.objectMode||!!t.writableObjectMode;var n=t.highWaterMark;var i=this.objectMode?16:16*1024;this.highWaterMark=n||n===0?n:i;this.highWaterMark=~~this.highWaterMark;this.needDrain=false;this.ending=false;this.ended=false;this.finished=false;var a=t.decodeStrings===false;this.decodeStrings=!a;this.defaultEncoding=t.defaultEncoding||"utf8";this.length=0;this.writing=false;this.corked=0;this.sync=true;this.bufferProcessing=false;this.onwrite=function(e){w(r,e)};this.writecb=null;this.writelen=0;this.bufferedRequest=null;this.lastBufferedRequest=null;this.pendingcb=0;this.prefinished=false;this.errorEmitted=false;this.bufferedRequestCount=0;this.corkedRequestsFree=new R(this);this.corkedRequestsFree.next=new R(this)}h.prototype.getBuffer=function T(){var e=this.bufferedRequest;var t=[];while(e){t.push(e);e=e.next}return t};(function(){try{Object.defineProperty(h.prototype,"buffer",{get:s.deprecate(function(){return this.getBuffer()},"_writableState.buffer is deprecated. Use _writableState.getBuffer "+"instead.")})}catch(e){}})();var l;function p(t){l=l||e("./_stream_duplex");if(!(this instanceof p)&&!(this instanceof l))return new p(t);this._writableState=new h(t,this);this.writable=true;if(t){if(typeof t.write==="function")this._write=t.write;if(typeof t.writev==="function")this._writev=t.writev}f.call(this)}p.prototype.pipe=function(){this.emit("error",new Error("Cannot pipe. Not readable."))};function d(e,t){var r=new Error("write after end");e.emit("error",r);n(t,r)}function v(e,t,r,i){var o=true;if(!a.isBuffer(r)&&typeof r!=="string"&&r!==null&&r!==undefined&&!t.objectMode){var s=new TypeError("Invalid non-string/buffer chunk");e.emit("error",s);n(i,s);o=false}return o}p.prototype.write=function(e,t,r){var n=this._writableState;var i=false;if(typeof t==="function"){r=t;t=null}if(a.isBuffer(e))t="buffer";else if(!t)t=n.defaultEncoding;if(typeof r!=="function")r=u;if(n.ended)d(this,r);else if(v(this,n,e,r)){n.pendingcb++;i=y(this,n,e,t,r)}return i};p.prototype.cork=function(){var e=this._writableState;e.corked++};p.prototype.uncork=function(){var e=this._writableState;if(e.corked){e.corked--;if(!e.writing&&!e.corked&&!e.finished&&!e.bufferProcessing&&e.bufferedRequest)x(this,e)}};p.prototype.setDefaultEncoding=function L(e){if(typeof e==="string")e=e.toLowerCase();if(!(["hex","utf8","utf-8","ascii","binary","base64","ucs2","ucs-2","utf16le","utf-16le","raw"].indexOf((e+"").toLowerCase())>-1))throw new TypeError("Unknown encoding: "+e);this._writableState.defaultEncoding=e};function g(e,t,r){if(!e.objectMode&&e.decodeStrings!==false&&typeof t==="string"){t=new a(t,r)}return t}function y(e,t,r,n,i){r=g(t,r,n);if(a.isBuffer(r))n="buffer";var o=t.objectMode?1:r.length;t.length+=o;var s=t.length<t.highWaterMark;if(!s)t.needDrain=true;if(t.writing||t.corked){var f=t.lastBufferedRequest;t.lastBufferedRequest=new c(r,n,i);if(f){f.next=t.lastBufferedRequest}else{t.bufferedRequest=t.lastBufferedRequest}t.bufferedRequestCount+=1}else{b(e,t,false,o,r,n,i)}return s}function b(e,t,r,n,i,a,o){t.writelen=n;t.writecb=o;t.writing=true;t.sync=true;if(r)e._writev(i,t.onwrite);else e._write(i,a,t.onwrite);t.sync=false}function m(e,t,r,i,a){--t.pendingcb;if(r)n(a,i);else a(i);e._writableState.errorEmitted=true;e.emit("error",i)}function _(e){e.writing=false;e.writecb=null;e.length-=e.writelen;e.writelen=0}function w(e,t){var r=e._writableState;var n=r.sync;var a=r.writecb;_(r);if(t)m(e,r,n,t,a);else{var o=O(r);if(!o&&!r.corked&&!r.bufferProcessing&&r.bufferedRequest){x(e,r)}if(n){i(E,e,r,o,a)}else{E(e,r,o,a)}}}function E(e,t,r,n){if(!r)S(e,t);t.pendingcb--;n();A(e,t)}function S(e,t){if(t.length===0&&t.needDrain){t.needDrain=false;e.emit("drain")}}function x(e,t){t.bufferProcessing=true;var r=t.bufferedRequest;if(e._writev&&r&&r.next){var n=t.bufferedRequestCount;var i=new Array(n);var a=t.corkedRequestsFree;a.entry=r;var o=0;while(r){i[o]=r;r=r.next;o+=1}b(e,t,true,t.length,i,"",a.finish);t.pendingcb++;t.lastBufferedRequest=null;t.corkedRequestsFree=a.next;a.next=null}else{while(r){var s=r.chunk;var f=r.encoding;var u=r.callback;var c=t.objectMode?1:s.length;b(e,t,false,c,s,f,u);r=r.next;if(t.writing){break}}if(r===null)t.lastBufferedRequest=null}t.bufferedRequestCount=0;t.bufferedRequest=r;t.bufferProcessing=false}p.prototype._write=function(e,t,r){r(new Error("not implemented"))};p.prototype._writev=null;p.prototype.end=function(e,t,r){var n=this._writableState;if(typeof e==="function"){r=e;e=null;t=null}else if(typeof t==="function"){r=t;t=null}if(e!==null&&e!==undefined)this.write(e,t);if(n.corked){n.corked=1;this.uncork()}if(!n.ending&&!n.finished)k(this,n,r)};function O(e){return e.ending&&e.length===0&&e.bufferedRequest===null&&!e.finished&&!e.writing}function j(e,t){if(!t.prefinished){t.prefinished=true;e.emit("prefinish")}}function A(e,t){var r=O(t);if(r){if(t.pendingcb===0){j(e,t);t.finished=true;e.emit("finish")}else{j(e,t)}}return r}function k(e,t,r){t.ending=true;A(e,t);if(r){if(t.finished)n(r);else e.once("finish",r)}t.ended=true;e.writable=false}function R(e){var t=this;this.next=null;this.entry=null;this.finish=function(r){var n=t.entry;t.entry=null;while(n){var i=n.callback;e.pendingcb--;i(r);n=n.next}if(e.corkedRequestsFree){e.corkedRequestsFree.next=t}else{e.corkedRequestsFree=t}}}}).call(this,e("_process"))},{"./_stream_duplex":119,_process:112,buffer:13,"core-util-is":19,events:22,inherits:57,"process-nextick-args":111,"util-deprecate":146}],124:[function(e,t,r){t.exports=e("./lib/_stream_passthrough.js")},{"./lib/_stream_passthrough.js":120}],125:[function(e,t,r){var n=function(){try{return e("st"+"ream")}catch(t){}}();r=t.exports=e("./lib/_stream_readable.js");r.Stream=n||r;r.Readable=r;r.Writable=e("./lib/_stream_writable.js");r.Duplex=e("./lib/_stream_duplex.js");r.Transform=e("./lib/_stream_transform.js");r.PassThrough=e("./lib/_stream_passthrough.js")},{"./lib/_stream_duplex.js":119,"./lib/_stream_passthrough.js":120,"./lib/_stream_readable.js":121,"./lib/_stream_transform.js":122,"./lib/_stream_writable.js":123}],126:[function(e,t,r){arguments[4][43][0].apply(r,arguments)},{"./lib/_stream_transform.js":122,dup:43}],127:[function(e,t,r){t.exports=e("./lib/_stream_writable.js")},{"./lib/_stream_writable.js":123}],128:[function(e,t,r){
"use strict";var n=e("is-primitive");var i=e("is-equal-shallow");var a={};var o={};t.exports=s;function s(e,t,r){var n="_default_",s,u;if(!t&&!r){if(typeof e!=="function"){return e}return a[n]||(a[n]=e(t))}var c=typeof t==="string";if(c){if(!r){return a[t]||(a[t]=e(t))}n=t}else{r=t}u=o[n];if(u&&i(u.opts,r)){return u.regex}f(n,r,s=e(t,r));return s}function f(e,t,r){o[e]={regex:r,opts:t}}t.exports.cache=o;t.exports.basic=a},{"is-equal-shallow":61,"is-primitive":67}],129:[function(e,t,r){"use strict";t.exports=function n(e,t){var r=new Array(t);for(var n=0;n<t;n++){r[n]=e}return r}},{}],130:[function(e,t,r){"use strict";var n="";var i;t.exports=a;function a(e,t){if(typeof e!=="string"){throw new TypeError("repeat-string expects a string.")}if(t===1)return e;if(t===2)return e+e;var r=e.length*t;if(i!==e||typeof i==="undefined"){i=e;n=""}while(r>n.length&&t>0){if(t&1){n+=e}t>>=1;if(!t)break;e+=e}return n.substr(0,r)}},{}],131:[function(e,t,r){var n=e("path");t.exports=function(e,t){if(typeof e!=="string")return e;if(e.length===0)return e;var r=n.basename(e,n.extname(e))+t;return n.join(n.dirname(e),r)}},{path:108}],132:[function(e,t,r){var n=e("stream").Readable;var i=e("stream").PassThrough;function a(e){n.call(this,e);e=e||{};this._streamsActive=false;this._streamsAdded=false;this._streams=[];this._currentStream=undefined;this._errorsEmitted=false;if(e.head){this._head=e.head}if(e.tail){this._tail=e.tail}if(e.separator){this._separator=e.separator}}a.prototype=Object.create(n.prototype,{constructor:a});a.prototype._read=function(){if(!this._streamsActive){this._streamsActive=true;this._pushHead();this._streamNextStream()}};a.prototype.add=function(e){if(!this._streamsActive){this._streamsAdded=true;this._streams.push(e);e.on("error",this._substreamOnError.bind(this))}else{throw new Error("SandwichStream error adding new stream while streaming")}};a.prototype._substreamOnError=function(e){this._errorsEmitted=true;this.emit("error",e)};a.prototype._pushHead=function(){if(this._head){this.push(this._head)}};a.prototype._streamNextStream=function(){if(this._nextStream()){this._bindCurrentStreamEvents()}else{this._pushTail();this.push(null)}};a.prototype._nextStream=function(){this._currentStream=this._streams.shift();return this._currentStream!==undefined};a.prototype._bindCurrentStreamEvents=function(){this._currentStream.on("readable",this._currentStreamOnReadable.bind(this));this._currentStream.on("end",this._currentStreamOnEnd.bind(this))};a.prototype._currentStreamOnReadable=function(){this.push(this._currentStream.read()||"")};a.prototype._currentStreamOnEnd=function(){this._pushSeparator();this._streamNextStream()};a.prototype._pushSeparator=function(){if(this._streams.length>0&&this._separator){this.push(this._separator)}};a.prototype._pushTail=function(){if(this._tail){this.push(this._tail)}};function o(e){var t=new a(e);return t}o.SandwichStream=a;t.exports=o},{stream:134}],133:[function(e,t,r){t.exports=n;function n(e,t){t=t||10;var r=[];var n="";var i=RegExp;function a(e,o){if(o>t)return;if(typeof e==="function"||typeof e==="undefined"){return}if(typeof e!=="object"||!e||e instanceof i){n+=e;return}if(r.indexOf(e)!==-1||o===t)return;r.push(e);n+="{";Object.keys(e).forEach(function(t,r,i){if(t.charAt(0)==="_")return;var s=typeof e[t];if(s==="function"||s==="undefined")return;n+=t;a(e[t],o+1)})}a(e,0);return n}},{}],134:[function(e,t,r){t.exports=a;var n=e("events").EventEmitter;var i=e("inherits");i(a,n);a.Readable=e("readable-stream/readable.js");a.Writable=e("readable-stream/writable.js");a.Duplex=e("readable-stream/duplex.js");a.Transform=e("readable-stream/transform.js");a.PassThrough=e("readable-stream/passthrough.js");a.Stream=a;function a(){n.call(this)}a.prototype.pipe=function(e,t){var r=this;function i(t){if(e.writable){if(false===e.write(t)&&r.pause){r.pause()}}}r.on("data",i);function a(){if(r.readable&&r.resume){r.resume()}}e.on("drain",a);if(!e._isStdio&&(!t||t.end!==false)){r.on("end",s);r.on("close",f)}var o=false;function s(){if(o)return;o=true;e.end()}function f(){if(o)return;o=true;if(typeof e.destroy==="function")e.destroy()}function u(e){c();if(n.listenerCount(this,"error")===0){throw e}}r.on("error",u);e.on("error",u);function c(){r.removeListener("data",i);e.removeListener("drain",a);r.removeListener("end",s);r.removeListener("close",f);r.removeListener("error",u);e.removeListener("error",u);r.removeListener("end",c);r.removeListener("close",c);e.removeListener("close",c)}r.on("end",c);r.on("close",c);e.on("close",c);e.emit("pipe",r);return e}},{events:22,inherits:57,"readable-stream/duplex.js":118,"readable-stream/passthrough.js":124,"readable-stream/readable.js":125,"readable-stream/transform.js":126,"readable-stream/writable.js":127}],135:[function(e,t,r){var n=e("./lib/request");var i=e("xtend");var a=e("builtin-status-codes");var o=e("url");var s=r;s.request=function(e,t){if(typeof e==="string")e=o.parse(e);else e=i(e);var r=e.protocol||"";var a=e.hostname||e.host;var s=e.port;var f=e.path||"/";if(a&&a.indexOf(":")!==-1)a="["+a+"]";e.url=(a?r+"//"+a:"")+(s?":"+s:"")+f;e.method=(e.method||"GET").toUpperCase();e.headers=e.headers||{};var u=new n(e);if(t)u.on("response",t);return u};s.get=function f(e,t){var r=s.request(e,t);r.end();return r};s.Agent=function(){};s.Agent.defaultMaxSockets=4;s.STATUS_CODES=a;s.METHODS=["CHECKOUT","CONNECT","COPY","DELETE","GET","HEAD","LOCK","M-SEARCH","MERGE","MKACTIVITY","MKCOL","MOVE","NOTIFY","OPTIONS","PATCH","POST","PROPFIND","PROPPATCH","PURGE","PUT","REPORT","SEARCH","SUBSCRIBE","TRACE","UNLOCK","UNSUBSCRIBE"]},{"./lib/request":137,"builtin-status-codes":14,url:145,xtend:188}],136:[function(e,t,r){(function(e){r.fetch=s(e.fetch)&&s(e.ReadableByteStream);r.blobConstructor=false;try{new Blob([new ArrayBuffer(1)]);r.blobConstructor=true}catch(t){}var n=new e.XMLHttpRequest;n.open("GET",e.location.host?"/":"https://example.com");function i(e){try{n.responseType=e;return n.responseType===e}catch(t){}return false}var a=typeof e.ArrayBuffer!=="undefined";var o=a&&s(e.ArrayBuffer.prototype.slice);r.arraybuffer=a&&i("arraybuffer");r.msstream=!r.fetch&&o&&i("ms-stream");r.mozchunkedarraybuffer=!r.fetch&&a&&i("moz-chunked-arraybuffer");r.overrideMimeType=s(n.overrideMimeType);r.vbArray=s(e.VBArray);function s(e){return typeof e==="function"}n=null}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{}],137:[function(e,t,r){(function(r,n,i){var a=e("./capability");var o=e("foreach");var s=e("indexof");var f=e("inherits");var u=e("object-keys");var c=e("./response");var l=e("stream");var h=c.IncomingMessage;var p=c.readyStates;function d(e){if(a.fetch){return"fetch"}else if(a.mozchunkedarraybuffer){return"moz-chunked-arraybuffer"}else if(a.msstream){return"ms-stream"}else if(a.arraybuffer&&e){return"arraybuffer"}else if(a.vbArray&&e){return"text:vbarray"}else{return"text"}}var v=t.exports=function(e){var t=this;l.Writable.call(t);t._opts=e;t._body=[];t._headers={};if(e.auth)t.setHeader("Authorization","Basic "+new i(e.auth).toString("base64"));o(u(e.headers),function(r){t.setHeader(r,e.headers[r])});var r;if(e.mode==="prefer-streaming"){r=false}else if(e.mode==="allow-wrong-content-type"){r=!a.overrideMimeType}else if(!e.mode||e.mode==="default"||e.mode==="prefer-fast"){r=true}else{throw new Error("Invalid value for opts.mode")}t._mode=d(r);t.on("finish",function(){t._onFinish()})};f(v,l.Writable);v.prototype.setHeader=function(e,t){var r=this;var n=e.toLowerCase();if(s(y,n)!==-1)return;r._headers[n]={name:e,value:t}};v.prototype.getHeader=function(e){var t=this;return t._headers[e.toLowerCase()].value};v.prototype.removeHeader=function(e){var t=this;delete t._headers[e.toLowerCase()]};v.prototype._onFinish=function(){var e=this;if(e._destroyed)return;var t=e._opts;var s=e._headers;var f;if(t.method==="POST"||t.method==="PUT"){if(a.blobConstructor){f=new n.Blob(e._body.map(function(e){return e.toArrayBuffer()}),{type:(s["content-type"]||{}).value||""})}else{f=i.concat(e._body).toString()}}if(e._mode==="fetch"){var c=u(s).map(function(e){return[s[e].name,s[e].value]});n.fetch(e._opts.url,{method:e._opts.method,headers:c,body:f,mode:"cors",credentials:t.withCredentials?"include":"same-origin"}).then(function(t){e._fetchResponse=t;e._connect()}).then(undefined,function(t){e.emit("error",t)})}else{var l=e._xhr=new n.XMLHttpRequest;try{l.open(e._opts.method,e._opts.url,true)}catch(h){r.nextTick(function(){e.emit("error",h)});return}if("responseType"in l)l.responseType=e._mode.split(":")[0];if("withCredentials"in l)l.withCredentials=!!t.withCredentials;if(e._mode==="text"&&"overrideMimeType"in l)l.overrideMimeType("text/plain; charset=x-user-defined");o(u(s),function(e){l.setRequestHeader(s[e].name,s[e].value)});e._response=null;l.onreadystatechange=function(){switch(l.readyState){case p.LOADING:case p.DONE:e._onXHRProgress();break}};if(e._mode==="moz-chunked-arraybuffer"){l.onprogress=function(){e._onXHRProgress()}}l.onerror=function(){if(e._destroyed)return;e.emit("error",new Error("XHR error"))};try{l.send(f)}catch(h){r.nextTick(function(){e.emit("error",h)});return}}};function g(e){try{return e.status!==null}catch(t){return false}}v.prototype._onXHRProgress=function(){var e=this;if(!g(e._xhr)||e._destroyed)return;if(!e._response)e._connect();e._response._onXHRProgress()};v.prototype._connect=function(){var e=this;if(e._destroyed)return;e._response=new h(e._xhr,e._fetchResponse,e._mode);e.emit("response",e._response)};v.prototype._write=function(e,t,r){var n=this;n._body.push(e);r()};v.prototype.abort=v.prototype.destroy=function(){var e=this;e._destroyed=true;if(e._response)e._response._destroyed=true;if(e._xhr)e._xhr.abort()};v.prototype.end=function(e,t,r){var n=this;if(typeof e==="function"){r=e;e=undefined}l.Writable.prototype.end.call(n,e,t,r)};v.prototype.flushHeaders=function(){};v.prototype.setTimeout=function(){};v.prototype.setNoDelay=function(){};v.prototype.setSocketKeepAlive=function(){};var y=["accept-charset","accept-encoding","access-control-request-headers","access-control-request-method","connection","content-length","cookie","cookie2","date","dnt","expect","host","keep-alive","origin","referer","te","trailer","transfer-encoding","upgrade","user-agent","via"]}).call(this,e("_process"),typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{},e("buffer").Buffer)},{"./capability":136,"./response":138,_process:112,buffer:13,foreach:32,indexof:55,inherits:57,"object-keys":101,stream:134}],138:[function(e,t,r){(function(t,n,i){var a=e("./capability");var o=e("foreach");var s=e("inherits");var f=e("stream");var u=r.readyStates={UNSENT:0,OPENED:1,HEADERS_RECEIVED:2,LOADING:3,DONE:4};var c=r.IncomingMessage=function(e,r,n){var s=this;f.Readable.call(s);s._mode=n;s.headers={};s.rawHeaders=[];s.trailers={};s.rawTrailers=[];s.on("end",function(){t.nextTick(function(){s.emit("close")})});if(n==="fetch"){s._fetchResponse=r;s.statusCode=r.status;s.statusMessage=r.statusText;for(var u,c,l=r.headers[Symbol.iterator]();u=(c=l.next()).value,!c.done;){s.headers[u[0].toLowerCase()]=u[1];s.rawHeaders.push(u[0],u[1])}var h=r.body.getReader();function p(){h.read().then(function(e){if(s._destroyed)return;if(e.done){s.push(null);return}s.push(new i(e.value));p()})}p()}else{s._xhr=e;s._pos=0;s.statusCode=e.status;s.statusMessage=e.statusText;var d=e.getAllResponseHeaders().split(/\r?\n/);o(d,function(e){var t=e.match(/^([^:]+):\s*(.*)/);if(t){var r=t[1].toLowerCase();if(s.headers[r]!==undefined)s.headers[r]+=", "+t[2];else s.headers[r]=t[2];s.rawHeaders.push(t[1],t[2])}});s._charset="x-user-defined";if(!a.overrideMimeType){var v=s.rawHeaders["mime-type"];if(v){var g=v.match(/;\s*charset=([^;])(;|$)/);if(g){s._charset=g[1].toLowerCase()}}if(!s._charset)s._charset="utf-8"}}};s(c,f.Readable);c.prototype._read=function(){};c.prototype._onXHRProgress=function(){var e=this;var t=e._xhr;var r=null;switch(e._mode){case"text:vbarray":if(t.readyState!==u.DONE)break;try{r=new n.VBArray(t.responseBody).toArray()}catch(a){}if(r!==null){e.push(new i(r));break}case"text":try{r=t.responseText}catch(a){e._mode="text:vbarray";break}if(r.length>e._pos){var o=r.substr(e._pos);if(e._charset==="x-user-defined"){var s=new i(o.length);for(var f=0;f<o.length;f++)s[f]=o.charCodeAt(f)&255;e.push(s)}else{e.push(o,e._charset)}e._pos=r.length}break;case"arraybuffer":if(t.readyState!==u.DONE)break;r=t.response;e.push(new i(new Uint8Array(r)));break;case"moz-chunked-arraybuffer":r=t.response;if(t.readyState!==u.LOADING||!r)break;e.push(new i(new Uint8Array(r)));break;case"ms-stream":r=t.response;if(t.readyState!==u.LOADING)break;var c=new n.MSStreamReader;c.onprogress=function(){if(c.result.byteLength>e._pos){e.push(new i(new Uint8Array(c.result.slice(e._pos))));e._pos=c.result.byteLength}};c.onload=function(){e.push(null)};c.readAsArrayBuffer(r);break}if(e._xhr.readyState===u.DONE&&e._mode!=="ms-stream"){e.push(null)}}}).call(this,e("_process"),typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{},e("buffer").Buffer)},{"./capability":136,_process:112,buffer:13,foreach:32,inherits:57,stream:134}],139:[function(e,t,r){var n=e("buffer").Buffer;var i=n.isEncoding||function(e){switch(e&&e.toLowerCase()){case"hex":case"utf8":case"utf-8":case"ascii":case"binary":case"base64":case"ucs2":case"ucs-2":case"utf16le":case"utf-16le":case"raw":return true;default:return false}};function a(e){if(e&&!i(e)){throw new Error("Unknown encoding: "+e)}}var o=r.StringDecoder=function(e){this.encoding=(e||"utf8").toLowerCase().replace(/[-_]/,"");a(e);switch(this.encoding){case"utf8":this.surrogateSize=3;break;case"ucs2":case"utf16le":this.surrogateSize=2;this.detectIncompleteChar=f;break;case"base64":this.surrogateSize=3;this.detectIncompleteChar=u;break;default:this.write=s;return}this.charBuffer=new n(6);this.charReceived=0;this.charLength=0};o.prototype.write=function(e){var t="";while(this.charLength){var r=e.length>=this.charLength-this.charReceived?this.charLength-this.charReceived:e.length;e.copy(this.charBuffer,this.charReceived,0,r);this.charReceived+=r;if(this.charReceived<this.charLength){return""}e=e.slice(r,e.length);t=this.charBuffer.slice(0,this.charLength).toString(this.encoding);var n=t.charCodeAt(t.length-1);if(n>=55296&&n<=56319){this.charLength+=this.surrogateSize;t="";continue}this.charReceived=this.charLength=0;if(e.length===0){return t}break}this.detectIncompleteChar(e);var i=e.length;if(this.charLength){e.copy(this.charBuffer,0,e.length-this.charReceived,i);i-=this.charReceived}t+=e.toString(this.encoding,0,i);var i=t.length-1;var n=t.charCodeAt(i);if(n>=55296&&n<=56319){var a=this.surrogateSize;this.charLength+=a;this.charReceived+=a;this.charBuffer.copy(this.charBuffer,a,0,a);e.copy(this.charBuffer,0,0,a);return t.substring(0,i)}return t};o.prototype.detectIncompleteChar=function(e){var t=e.length>=3?3:e.length;for(;t>0;t--){var r=e[e.length-t];if(t==1&&r>>5==6){this.charLength=2;break}if(t<=2&&r>>4==14){this.charLength=3;break}if(t<=3&&r>>3==30){this.charLength=4;break}}this.charReceived=t};o.prototype.end=function(e){var t="";if(e&&e.length)t=this.write(e);if(this.charReceived){var r=this.charReceived;var n=this.charBuffer;var i=this.encoding;t+=n.slice(0,r).toString(i)}return t};function s(e){return e.toString(this.encoding)}function f(e){this.charReceived=e.length%2;this.charLength=this.charReceived?2:0}function u(e){this.charReceived=e.length%3;this.charLength=this.charReceived?3:0}},{buffer:13}],140:[function(e,t,r){"use strict";t.exports=s;t.exports.ctor=a;t.exports.objCtor=o;t.exports.obj=f;var n=e("through2");var i=e("xtend");function a(e,t){if(typeof e=="function"){t=e;e={}}var r=n.ctor(e,function(e,r,n){if(this.options.wantStrings)e=e.toString();if(t.call(this,e,this._index++))this.push(e);return n()});r.prototype._index=0;return r}function o(e,t){if(typeof e==="function"){t=e;e={}}e=i({objectMode:true,highWaterMark:16},e);return a(e,t)}function s(e,t){return a(e,t)()}function f(e,t){if(typeof e==="function"){t=e;e={}}e=i({objectMode:true,highWaterMark:16},e);return s(e,t)}},{through2:141,xtend:188}],141:[function(e,t,r){arguments[4][44][0].apply(r,arguments)},{_process:112,dup:44,"readable-stream/transform":126,util:148,xtend:188}],142:[function(e,t,r){var n=e("process/browser.js").nextTick;var i=Function.prototype.apply;var a=Array.prototype.slice;var o={};var s=0;r.setTimeout=function(){return new f(i.call(setTimeout,window,arguments),clearTimeout)};r.setInterval=function(){return new f(i.call(setInterval,window,arguments),clearInterval)};r.clearTimeout=r.clearInterval=function(e){e.close()};function f(e,t){this._id=e;this._clearFn=t}f.prototype.unref=f.prototype.ref=function(){};f.prototype.close=function(){this._clearFn.call(window,this._id)};r.enroll=function(e,t){clearTimeout(e._idleTimeoutId);e._idleTimeout=t};r.unenroll=function(e){clearTimeout(e._idleTimeoutId);e._idleTimeout=-1};r._unrefActive=r.active=function(e){clearTimeout(e._idleTimeoutId);var t=e._idleTimeout;if(t>=0){e._idleTimeoutId=setTimeout(function r(){if(e._onTimeout)e._onTimeout()},t)}};r.setImmediate=typeof setImmediate==="function"?setImmediate:function(e){var t=s++;var i=arguments.length<2?false:a.call(arguments,1);o[t]=true;n(function f(){if(o[t]){if(i){e.apply(null,i)}else{e.call(null)}r.clearImmediate(t)}});return t};r.clearImmediate=typeof clearImmediate==="function"?clearImmediate:function(e){delete o[e]}},{"process/browser.js":112}],143:[function(e,t,r){(function(r){"use strict";var n=e("path");var i=e("extend-shallow");t.exports=function(e,t){var a=i({},t);a.cwd=a.cwd?n.resolve(a.cwd):r.cwd();var o=e.charAt(0);var s=e.slice(-1);var f=o==="!";if(f)e=e.slice(1);if(a.root&&e.charAt(0)==="/"){e=n.join(n.resolve(a.root),"."+e)}else{e=n.resolve(a.cwd,e)}if(s==="/"&&e.slice(-1)!=="/"){e+="/"}return f?"!"+e:e}}).call(this,e("_process"))},{_process:112,"extend-shallow":25,path:108}],144:[function(e,t,r){(function(r){"use strict";var n=e("through2-filter").obj;var i=e("json-stable-stringify");var a;if(typeof r.Set==="function"){a=r.Set}else{a=function(){this.keys=[];this.has=function(e){return this.keys.indexOf(e)!==-1},this.add=function(e){this.keys.push(e)}}}function o(e){return function(t){return t[e]}}t.exports=s;function s(e,t){t=t||new a;var r=i;if(typeof e==="string"){r=o(e)}else if(typeof e==="function"){r=e}return n(function(e){var n=r(e);if(t.has(n)){return false}t.add(n);return true})}}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{"json-stable-stringify":72,"through2-filter":140}],145:[function(e,t,r){var n=e("punycode");r.parse=m;r.resolve=w;r.resolveObject=E;r.format=_;r.Url=i;function i(){this.protocol=null;this.slashes=null;this.auth=null;this.host=null;this.port=null;this.hostname=null;this.hash=null;this.search=null;this.query=null;this.pathname=null;this.path=null;this.href=null}var a=/^([a-z0-9.+-]+:)/i,o=/:[0-9]*$/,s=["<",">",'"',"`"," ","\r","\n","\t"],f=["{","}","|","\\","^","`"].concat(s),u=["'"].concat(f),c=["%","/","?",";","#"].concat(u),l=["/","?","#"],h=255,p=/^[a-z0-9A-Z_-]{0,63}$/,d=/^([a-z0-9A-Z_-]{0,63})(.*)$/,v={javascript:true,"javascript:":true},g={javascript:true,"javascript:":true},y={http:true,https:true,ftp:true,gopher:true,file:true,"http:":true,"https:":true,"ftp:":true,"gopher:":true,"file:":true},b=e("querystring");function m(e,t,r){if(e&&x(e)&&e instanceof i)return e;var n=new i;n.parse(e,t,r);return n}i.prototype.parse=function(e,t,r){if(!S(e)){throw new TypeError("Parameter 'url' must be a string, not "+typeof e)}var i=e;i=i.trim();var o=a.exec(i);if(o){o=o[0];var s=o.toLowerCase();this.protocol=s;i=i.substr(o.length)}if(r||o||i.match(/^\/\/[^@\/]+@[^@\/]+/)){var f=i.substr(0,2)==="//";if(f&&!(o&&g[o])){i=i.substr(2);this.slashes=true}}if(!g[o]&&(f||o&&!y[o])){var m=-1;for(var _=0;_<l.length;_++){var w=i.indexOf(l[_]);if(w!==-1&&(m===-1||w<m))m=w}var E,x;if(m===-1){x=i.lastIndexOf("@")}else{x=i.lastIndexOf("@",m)}if(x!==-1){E=i.slice(0,x);i=i.slice(x+1);this.auth=decodeURIComponent(E)}m=-1;for(var _=0;_<c.length;_++){var w=i.indexOf(c[_]);if(w!==-1&&(m===-1||w<m))m=w}if(m===-1)m=i.length;this.host=i.slice(0,m);i=i.slice(m);this.parseHost();this.hostname=this.hostname||"";var O=this.hostname[0]==="["&&this.hostname[this.hostname.length-1]==="]";if(!O){var j=this.hostname.split(/\./);for(var _=0,A=j.length;_<A;_++){var k=j[_];if(!k)continue;if(!k.match(p)){var R="";for(var T=0,L=k.length;T<L;T++){if(k.charCodeAt(T)>127){R+="x"}else{R+=k[T]}}if(!R.match(p)){var I=j.slice(0,_);var C=j.slice(_+1);var P=k.match(d);if(P){I.push(P[1]);C.unshift(P[2])}if(C.length){i="/"+C.join(".")+i}this.hostname=I.join(".");break}}}}if(this.hostname.length>h){this.hostname=""}else{this.hostname=this.hostname.toLowerCase()}if(!O){var M=this.hostname.split(".");var N=[];for(var _=0;_<M.length;++_){var B=M[_];N.push(B.match(/[^A-Za-z0-9_-]/)?"xn--"+n.encode(B):B)}this.hostname=N.join(".")}var D=this.port?":"+this.port:"";var U=this.hostname||"";this.host=U+D;this.href+=this.host;if(O){this.hostname=this.hostname.substr(1,this.hostname.length-2);if(i[0]!=="/"){i="/"+i}}}if(!v[s]){for(var _=0,A=u.length;_<A;_++){var F=u[_];var q=encodeURIComponent(F);if(q===F){q=escape(F)}i=i.split(F).join(q)}}var G=i.indexOf("#");if(G!==-1){this.hash=i.substr(G);i=i.slice(0,G)}var $=i.indexOf("?");if($!==-1){this.search=i.substr($);this.query=i.substr($+1);if(t){this.query=b.parse(this.query)}i=i.slice(0,$)}else if(t){this.search="";this.query={}}if(i)this.pathname=i;if(y[s]&&this.hostname&&!this.pathname){this.pathname="/"}if(this.pathname||this.search){var D=this.pathname||"";var B=this.search||"";this.path=D+B}this.href=this.format();return this};function _(e){if(S(e))e=m(e);if(!(e instanceof i))return i.prototype.format.call(e);return e.format()}i.prototype.format=function(){var e=this.auth||"";if(e){e=encodeURIComponent(e);e=e.replace(/%3A/i,":");e+="@"}var t=this.protocol||"",r=this.pathname||"",n=this.hash||"",i=false,a="";if(this.host){i=e+this.host}else if(this.hostname){i=e+(this.hostname.indexOf(":")===-1?this.hostname:"["+this.hostname+"]");if(this.port){i+=":"+this.port}}if(this.query&&x(this.query)&&Object.keys(this.query).length){a=b.stringify(this.query)}var o=this.search||a&&"?"+a||"";if(t&&t.substr(-1)!==":")t+=":";if(this.slashes||(!t||y[t])&&i!==false){i="//"+(i||"");if(r&&r.charAt(0)!=="/")r="/"+r}else if(!i){i=""}if(n&&n.charAt(0)!=="#")n="#"+n;if(o&&o.charAt(0)!=="?")o="?"+o;r=r.replace(/[?#]/g,function(e){return encodeURIComponent(e)});o=o.replace("#","%23");return t+i+r+o+n};function w(e,t){return m(e,false,true).resolve(t)}i.prototype.resolve=function(e){return this.resolveObject(m(e,false,true)).format()};function E(e,t){if(!e)return t;return m(e,false,true).resolveObject(t)}i.prototype.resolveObject=function(e){if(S(e)){var t=new i;t.parse(e,false,true);e=t}var r=new i;Object.keys(this).forEach(function(e){r[e]=this[e]},this);r.hash=e.hash;if(e.href===""){r.href=r.format();return r}if(e.slashes&&!e.protocol){Object.keys(e).forEach(function(t){if(t!=="protocol")r[t]=e[t]});if(y[r.protocol]&&r.hostname&&!r.pathname){r.path=r.pathname="/"}r.href=r.format();return r}if(e.protocol&&e.protocol!==r.protocol){if(!y[e.protocol]){Object.keys(e).forEach(function(t){r[t]=e[t]});r.href=r.format();return r}r.protocol=e.protocol;if(!e.host&&!g[e.protocol]){var n=(e.pathname||"").split("/");while(n.length&&!(e.host=n.shift()));if(!e.host)e.host="";if(!e.hostname)e.hostname="";if(n[0]!=="")n.unshift("");if(n.length<2)n.unshift("");r.pathname=n.join("/")}else{r.pathname=e.pathname}r.search=e.search;r.query=e.query;r.host=e.host||"";r.auth=e.auth;r.hostname=e.hostname||e.host;r.port=e.port;if(r.pathname||r.search){var a=r.pathname||"";var o=r.search||"";r.path=a+o}r.slashes=r.slashes||e.slashes;r.href=r.format();return r}var s=r.pathname&&r.pathname.charAt(0)==="/",f=e.host||e.pathname&&e.pathname.charAt(0)==="/",u=f||s||r.host&&e.pathname,c=u,l=r.pathname&&r.pathname.split("/")||[],n=e.pathname&&e.pathname.split("/")||[],h=r.protocol&&!y[r.protocol];if(h){r.hostname="";r.port=null;if(r.host){if(l[0]==="")l[0]=r.host;else l.unshift(r.host)}r.host="";if(e.protocol){e.hostname=null;e.port=null;if(e.host){if(n[0]==="")n[0]=e.host;else n.unshift(e.host)}e.host=null}u=u&&(n[0]===""||l[0]==="")}if(f){r.host=e.host||e.host===""?e.host:r.host;r.hostname=e.hostname||e.hostname===""?e.hostname:r.hostname;r.search=e.search;r.query=e.query;l=n}else if(n.length){if(!l)l=[];l.pop();l=l.concat(n);r.search=e.search;r.query=e.query}else if(!j(e.search)){if(h){r.hostname=r.host=l.shift();var p=r.host&&r.host.indexOf("@")>0?r.host.split("@"):false;if(p){r.auth=p.shift();r.host=r.hostname=p.shift()}}r.search=e.search;r.query=e.query;if(!O(r.pathname)||!O(r.search)){r.path=(r.pathname?r.pathname:"")+(r.search?r.search:"")}r.href=r.format();return r}if(!l.length){r.pathname=null;if(r.search){r.path="/"+r.search}else{r.path=null}r.href=r.format();return r}var d=l.slice(-1)[0];var v=(r.host||e.host)&&(d==="."||d==="..")||d==="";var b=0;for(var m=l.length;m>=0;m--){d=l[m];if(d=="."){l.splice(m,1)}else if(d===".."){l.splice(m,1);b++}else if(b){l.splice(m,1);b--}}if(!u&&!c){for(;b--;b){l.unshift("..")}}if(u&&l[0]!==""&&(!l[0]||l[0].charAt(0)!=="/")){l.unshift("")}if(v&&l.join("/").substr(-1)!=="/"){l.push("")}var _=l[0]===""||l[0]&&l[0].charAt(0)==="/";if(h){r.hostname=r.host=_?"":l.length?l.shift():"";var p=r.host&&r.host.indexOf("@")>0?r.host.split("@"):false;if(p){r.auth=p.shift();r.host=r.hostname=p.shift()}}u=u||r.host&&l.length;if(u&&!_){l.unshift("")}if(!l.length){r.pathname=null;r.path=null}else{r.pathname=l.join("/")}if(!O(r.pathname)||!O(r.search)){r.path=(r.pathname?r.pathname:"")+(r.search?r.search:"")}r.auth=e.auth||r.auth;r.slashes=r.slashes||e.slashes;r.href=r.format();return r};i.prototype.parseHost=function(){var e=this.host;var t=o.exec(e);if(t){t=t[0];if(t!==":"){this.port=t.substr(1)}e=e.substr(0,e.length-t.length)}if(e)this.hostname=e};function S(e){return typeof e==="string"}function x(e){return typeof e==="object"&&e!==null}function O(e){return e===null}function j(e){return e==null}},{punycode:113,querystring:116}],146:[function(e,t,r){(function(e){t.exports=r;function r(e,t){if(n("noDeprecation")){return e}var r=false;function i(){if(!r){if(n("throwDeprecation")){throw new Error(t)}else if(n("traceDeprecation")){console.trace(t)}else{console.warn(t)}r=true}return e.apply(this,arguments)}return i}function n(t){try{if(!e.localStorage)return false}catch(r){return false}var n=e.localStorage[t];if(null==n)return false;return String(n).toLowerCase()==="true"}}).call(this,typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{}],147:[function(e,t,r){t.exports=function n(e){return e&&typeof e==="object"&&typeof e.copy==="function"&&typeof e.fill==="function"&&typeof e.readUInt8==="function"}},{}],148:[function(e,t,r){(function(t,n){var i=/%[sdj%]/g;r.format=function(e){if(!E(e)){var t=[];for(var r=0;r<arguments.length;r++){t.push(s(arguments[r]))}return t.join(" ")}var r=1;var n=arguments;var a=n.length;var o=String(e).replace(i,function(e){if(e==="%%")return"%";if(r>=a)return e;switch(e){case"%s":return String(n[r++]);case"%d":return Number(n[r++]);case"%j":try{return JSON.stringify(n[r++])}catch(t){return"[Circular]"}default:return e}});for(var f=n[r];r<a;f=n[++r]){if(m(f)||!j(f)){o+=" "+f}else{o+=" "+s(f)}}return o};r.deprecate=function(e,i){if(x(n.process)){return function(){return r.deprecate(e,i).apply(this,arguments)}}if(t.noDeprecation===true){return e}var a=false;function o(){if(!a){if(t.throwDeprecation){throw new Error(i)}else if(t.traceDeprecation){console.trace(i)}else{console.error(i)}a=true}return e.apply(this,arguments)}return o};var a={};var o;r.debuglog=function(e){if(x(o))o=t.env.NODE_DEBUG||"";e=e.toUpperCase();if(!a[e]){if(new RegExp("\\b"+e+"\\b","i").test(o)){var n=t.pid;a[e]=function(){var t=r.format.apply(r,arguments);console.error("%s %d: %s",e,n,t)}}else{a[e]=function(){}}}return a[e]};function s(e,t){var n={seen:[],stylize:u};if(arguments.length>=3)n.depth=arguments[2];if(arguments.length>=4)n.colors=arguments[3];if(b(t)){n.showHidden=t}else if(t){r._extend(n,t)}if(x(n.showHidden))n.showHidden=false;if(x(n.depth))n.depth=2;if(x(n.colors))n.colors=false;if(x(n.customInspect))n.customInspect=true;if(n.colors)n.stylize=f;return l(n,e,n.depth)}r.inspect=s;s.colors={bold:[1,22],italic:[3,23],underline:[4,24],inverse:[7,27],white:[37,39],grey:[90,39],black:[30,39],blue:[34,39],cyan:[36,39],green:[32,39],magenta:[35,39],red:[31,39],yellow:[33,39]};s.styles={special:"cyan",number:"yellow","boolean":"yellow",undefined:"grey","null":"bold",string:"green",date:"magenta",regexp:"red"};function f(e,t){var r=s.styles[t];if(r){return"["+s.colors[r][0]+"m"+e+"["+s.colors[r][1]+"m"}else{return e}}function u(e,t){return e}function c(e){var t={};e.forEach(function(e,r){t[e]=true});return t}function l(e,t,n){if(e.customInspect&&t&&R(t.inspect)&&t.inspect!==r.inspect&&!(t.constructor&&t.constructor.prototype===t)){var i=t.inspect(n,e);if(!E(i)){i=l(e,i,n)}return i}var a=h(e,t);if(a){return a}var o=Object.keys(t);var s=c(o);if(e.showHidden){o=Object.getOwnPropertyNames(t)}if(k(t)&&(o.indexOf("message")>=0||o.indexOf("description")>=0)){return p(t)}if(o.length===0){if(R(t)){var f=t.name?": "+t.name:"";return e.stylize("[Function"+f+"]","special")}if(O(t)){return e.stylize(RegExp.prototype.toString.call(t),"regexp")}if(A(t)){return e.stylize(Date.prototype.toString.call(t),"date")}if(k(t)){return p(t)}}var u="",b=false,m=["{","}"];if(y(t)){b=true;m=["[","]"]}if(R(t)){var _=t.name?": "+t.name:"";u=" [Function"+_+"]"}if(O(t)){u=" "+RegExp.prototype.toString.call(t)}if(A(t)){u=" "+Date.prototype.toUTCString.call(t)}if(k(t)){u=" "+p(t)}if(o.length===0&&(!b||t.length==0)){return m[0]+u+m[1]}if(n<0){if(O(t)){return e.stylize(RegExp.prototype.toString.call(t),"regexp")}else{return e.stylize("[Object]","special")}}e.seen.push(t);var w;if(b){w=d(e,t,n,s,o)}else{w=o.map(function(r){return v(e,t,n,s,r,b)})}e.seen.pop();return g(w,u,m)}function h(e,t){if(x(t))return e.stylize("undefined","undefined");if(E(t)){var r="'"+JSON.stringify(t).replace(/^"|"$/g,"").replace(/'/g,"\\'").replace(/\\"/g,'"')+"'";return e.stylize(r,"string")}if(w(t))return e.stylize(""+t,"number");if(b(t))return e.stylize(""+t,"boolean");if(m(t))return e.stylize("null","null")}function p(e){return"["+Error.prototype.toString.call(e)+"]"}function d(e,t,r,n,i){var a=[];for(var o=0,s=t.length;o<s;++o){if(M(t,String(o))){a.push(v(e,t,r,n,String(o),true))}else{a.push("")}}i.forEach(function(i){if(!i.match(/^\d+$/)){a.push(v(e,t,r,n,i,true))}});return a}function v(e,t,r,n,i,a){var o,s,f;f=Object.getOwnPropertyDescriptor(t,i)||{value:t[i]};if(f.get){if(f.set){s=e.stylize("[Getter/Setter]","special")}else{s=e.stylize("[Getter]","special")}}else{if(f.set){s=e.stylize("[Setter]","special")}}if(!M(n,i)){o="["+i+"]"}if(!s){if(e.seen.indexOf(f.value)<0){if(m(r)){s=l(e,f.value,null)}else{s=l(e,f.value,r-1)}if(s.indexOf("\n")>-1){if(a){s=s.split("\n").map(function(e){return"  "+e}).join("\n").substr(2)}else{s="\n"+s.split("\n").map(function(e){return"   "+e}).join("\n")}}}else{s=e.stylize("[Circular]","special")}}if(x(o)){if(a&&i.match(/^\d+$/)){return s}o=JSON.stringify(""+i);if(o.match(/^"([a-zA-Z_][a-zA-Z_0-9]*)"$/)){o=o.substr(1,o.length-2);o=e.stylize(o,"name")}else{o=o.replace(/'/g,"\\'").replace(/\\"/g,'"').replace(/(^"|"$)/g,"'");o=e.stylize(o,"string")}}return o+": "+s}function g(e,t,r){var n=0;var i=e.reduce(function(e,t){n++;if(t.indexOf("\n")>=0)n++;return e+t.replace(/\u001b\[\d\d?m/g,"").length+1},0);if(i>60){return r[0]+(t===""?"":t+"\n ")+" "+e.join(",\n  ")+" "+r[1]}return r[0]+t+" "+e.join(", ")+" "+r[1]}function y(e){return Array.isArray(e)}r.isArray=y;function b(e){return typeof e==="boolean"}r.isBoolean=b;function m(e){return e===null;
}r.isNull=m;function _(e){return e==null}r.isNullOrUndefined=_;function w(e){return typeof e==="number"}r.isNumber=w;function E(e){return typeof e==="string"}r.isString=E;function S(e){return typeof e==="symbol"}r.isSymbol=S;function x(e){return e===void 0}r.isUndefined=x;function O(e){return j(e)&&L(e)==="[object RegExp]"}r.isRegExp=O;function j(e){return typeof e==="object"&&e!==null}r.isObject=j;function A(e){return j(e)&&L(e)==="[object Date]"}r.isDate=A;function k(e){return j(e)&&(L(e)==="[object Error]"||e instanceof Error)}r.isError=k;function R(e){return typeof e==="function"}r.isFunction=R;function T(e){return e===null||typeof e==="boolean"||typeof e==="number"||typeof e==="string"||typeof e==="symbol"||typeof e==="undefined"}r.isPrimitive=T;r.isBuffer=e("./support/isBuffer");function L(e){return Object.prototype.toString.call(e)}function I(e){return e<10?"0"+e.toString(10):e.toString(10)}var C=["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"];function P(){var e=new Date;var t=[I(e.getHours()),I(e.getMinutes()),I(e.getSeconds())].join(":");return[e.getDate(),C[e.getMonth()],t].join(" ")}r.log=function(){console.log("%s - %s",P(),r.format.apply(r,arguments))};r.inherits=e("inherits");r._extend=function(e,t){if(!t||!j(t))return e;var r=Object.keys(t);var n=r.length;while(n--){e[r[n]]=t[r[n]]}return e};function M(e,t){return Object.prototype.hasOwnProperty.call(e,t)}}).call(this,e("_process"),typeof global!=="undefined"?global:typeof self!=="undefined"?self:typeof window!=="undefined"?window:{})},{"./support/isBuffer":147,_process:112,inherits:57}],149:[function(e,t,r){t.exports=a;var n=128,i=127;function a(e,t){var r=0,t=t||0,o=0,s=t,f,u=e.length;do{if(s>=u){a.bytes=0;a.bytesRead=0;return undefined}f=e[s++];r+=o<28?(f&i)<<o:(f&i)*Math.pow(2,o);o+=7}while(f>=n);a.bytes=s-t;return r}},{}],150:[function(e,t,r){t.exports=s;var n=128,i=127,a=~i,o=Math.pow(2,31);function s(e,t,r){t=t||[];r=r||0;var i=r;while(e>=o){t[r++]=e&255|n;e/=128}while(e&a){t[r++]=e&255|n;e>>>=7}t[r]=e|0;s.bytes=r-i+1;return t}},{}],151:[function(e,t,r){t.exports={encode:e("./encode.js"),decode:e("./decode.js"),encodingLength:e("./length.js")}},{"./decode.js":149,"./encode.js":150,"./length.js":152}],152:[function(e,t,r){var n=Math.pow(2,7);var i=Math.pow(2,14);var a=Math.pow(2,21);var o=Math.pow(2,28);var s=Math.pow(2,35);var f=Math.pow(2,42);var u=Math.pow(2,49);var c=Math.pow(2,56);var l=Math.pow(2,63);t.exports=function(e){return e<n?1:e<i?2:e<a?3:e<o?4:e<s?5:e<f?6:e<u?7:e<c?8:e<l?9:10}},{}],153:[function(e,t,r){"use strict";t.exports={src:e("./lib/src"),dest:e("./lib/dest"),symlink:e("./lib/symlink"),watch:e("glob-watcher")}},{"./lib/dest":154,"./lib/src":165,"./lib/symlink":167,"glob-watcher":45}],154:[function(e,t,r){"use strict";var n=e("through2");var i=e("../prepareWrite");var a=e("./writeContents");function o(e,t){if(!t){t={}}function r(r,n,o){i(e,r,t,function(e,t){if(e){return o(e)}a(t,r,o)})}return n.obj(r)}t.exports=o},{"../prepareWrite":160,"./writeContents":155,through2:141}],155:[function(e,t,r){"use strict";var n=e("./writeDir");var i=e("./writeStream");var a=e("./writeBuffer");function o(e,t,r){if(t.isDirectory()){return n(e,t,s)}if(t.isStream()){return i(e,t,s)}if(t.isBuffer()){return a(e,t,s)}if(t.isNull()){return o()}function o(e){r(e,t)}function s(r){if(f(r)){return o(r)}if(!t.stat||typeof t.stat.mode!=="number"){return o()}fs.stat(e,function(r,n){if(r){return o(r)}var i=n.mode&parseInt("0777",8);var a=t.stat.mode&parseInt("0777",8);if(i===a){return o()}fs.chmod(e,a,o)})}function f(e){if(!e){return false}else if(e.code==="EEXIST"&&t.flag==="wx"){return false}return true}}t.exports=o},{"./writeBuffer":156,"./writeDir":157,"./writeStream":158}],156:[function(e,t,r){"use strict";function n(e,t,r){var n={mode:t.stat.mode,flag:t.flag};fs.writeFile(e,t.contents,n,r)}t.exports=n},{}],157:[function(e,t,r){"use strict";var n=e("mkdirp");function i(e,t,r){n(e,t.stat.mode,r)}t.exports=i},{mkdirp:93}],158:[function(e,t,r){"use strict";var n=e("../../src/getContents/streamFile");function i(e,t,r){var i={mode:t.stat.mode,flag:t.flag};var a=fs.createWriteStream(e,i);t.contents.once("error",s);a.once("error",s);a.once("finish",o);t.contents.pipe(a);function o(){n(t,{},s)}function s(e){t.contents.removeListener("error",r);a.removeListener("error",r);a.removeListener("finish",o);r(e)}}t.exports=i},{"../../src/getContents/streamFile":164}],159:[function(e,t,r){"use strict";var n=e("through2-filter");t.exports=function(e){var t=typeof e==="number"||e instanceof Number||e instanceof Date;if(!t){throw new Error("expected since option to be a date or a number")}return n.obj(function(t){return t.stat&&t.stat.mtime>e})}},{"through2-filter":140}],160:[function(e,t,r){(function(r){"use strict";var n=e("object-assign");var i=e("path");var a=e("mkdirp");function o(e,t){if(typeof e!=="string"&&typeof e!=="function"){return null}return typeof e==="string"?e:e(t)}function s(e,t,s,f){var u=n({cwd:r.cwd(),mode:t.stat?t.stat.mode:null,dirMode:null,overwrite:true},s);u.flag=u.overwrite?"w":"wx";var c=i.resolve(u.cwd);var l=o(e,t);if(!l){throw new Error("Invalid output folder")}var h=u.base?o(u.base,t):i.resolve(c,l);if(!h){throw new Error("Invalid base option")}var p=i.resolve(h,t.relative);var d=i.dirname(p);t.stat=t.stat||new fs.Stats;t.stat.mode=u.mode;t.flag=u.flag;t.cwd=c;t.base=h;t.path=p;a(d,u.dirMode,function(e){if(e){return f(e)}f(null,p)})}t.exports=s}).call(this,e("_process"))},{_process:112,mkdirp:93,"object-assign":100,path:108}],161:[function(e,t,r){"use strict";function n(e,t,r){fs.readFile(e.path,function(t,n){if(t){return r(t)}e.contents=n;r(null,e)})}t.exports=n},{}],162:[function(e,t,r){"use strict";var n=e("through2");var i=e("./readDir");var a=e("./bufferFile");var o=e("./streamFile");function s(e){return n.obj(function(t,r,n){if(t.isDirectory()){return i(t,e,n)}if(e.buffer!==false){return a(t,e,n)}return o(t,e,n)})}t.exports=s},{"./bufferFile":161,"./readDir":163,"./streamFile":164,through2:141}],163:[function(e,t,r){"use strict";function n(e,t,r){r(null,e)}t.exports=n},{}],164:[function(e,t,r){"use strict";function n(e,t,r){e.contents=fs.createReadStream(e.path);r(null,e)}t.exports=n},{}],165:[function(e,t,r){"use strict";var n=e("object-assign");var i=e("through2");var a=e("glob-stream");var o=e("vinyl");var s=e("duplexify");var f=e("merge-stream");var u=e("../filterSince");var c=e("is-valid-glob");var l=e("./getContents");var h=e("./resolveSymlinks");function p(e,t,r){r(null,new o(e))}function d(e,t){var r=n({read:true,buffer:true,sourcemaps:false,passthrough:false},t);var o;if(!c(e)){throw new Error("Invalid glob argument: "+e)}var d=a.create(e,r);var v=d.pipe(h()).pipe(i.obj(p));if(r.since!=null){v=v.pipe(u(r.since))}if(r.read!==false){v=v.pipe(l(r))}if(r.passthrough===true){o=i.obj();v=s.obj(o,f(v,o))}d.on("error",v.emit.bind(v,"error"));return v}t.exports=d},{"../filterSince":159,"./getContents":162,"./resolveSymlinks":166,duplexify:20,"glob-stream":37,"is-valid-glob":69,"merge-stream":169,"object-assign":100,through2:141,vinyl:181}],166:[function(e,t,r){"use strict";var n=e("through2");var i=e("path");function a(){return n.obj(o)}function o(e,t,r){fs.lstat(e.path,function(n,a){if(n){return r(n)}e.stat=a;if(!a.isSymbolicLink()){return r(null,e)}fs.realpath(e.path,function(n,a){if(n){return r(n)}e.base=i.dirname(a);e.path=a;o(e,t,r)})})}t.exports=a},{path:108,through2:141}],167:[function(e,t,r){"use strict";var n=e("through2");var i=e("../prepareWrite");function a(e,t){function r(r,n,a){var o=r.path;var s=r.isDirectory()?"dir":"file";i(e,r,t,function(e,t){if(e){return a(e)}fs.symlink(o,t,s,function(e){if(e&&e.code!=="EEXIST"){return a(e)}a(null,r)})})}var a=n.obj(r);a.resume();return a}t.exports=a},{"../prepareWrite":160,through2:141}],168:[function(e,t,r){arguments[4][38][0].apply(r,arguments)},{dup:38}],169:[function(e,t,r){"use strict";var n=e("through2");t.exports=function(){var e=[];var t=n.obj();t.setMaxListeners(0);t.add=r;t.isEmpty=i;t.on("unpipe",a);Array.prototype.slice.call(arguments).forEach(r);return t;function r(n){if(Array.isArray(n)){n.forEach(r);return this}e.push(n);n.once("end",a.bind(null,n));n.pipe(t,{end:false});return this}function i(){return e.length==0}function a(r){e=e.filter(function(e){return e!==r});if(!e.length&&t.readable){t.end()}}}},{through2:170}],170:[function(e,t,r){arguments[4][44][0].apply(r,arguments)},{_process:112,dup:44,"readable-stream/transform":175,util:148,xtend:188}],171:[function(e,t,r){arguments[4][39][0].apply(r,arguments)},{"./_stream_readable":172,"./_stream_writable":174,_process:112,"core-util-is":19,dup:39,inherits:57}],172:[function(e,t,r){arguments[4][40][0].apply(r,arguments)},{_process:112,buffer:13,"core-util-is":19,dup:40,events:22,inherits:57,isarray:168,stream:134,"string_decoder/":139}],173:[function(e,t,r){arguments[4][41][0].apply(r,arguments)},{"./_stream_duplex":171,"core-util-is":19,dup:41,inherits:57}],174:[function(e,t,r){arguments[4][42][0].apply(r,arguments)},{"./_stream_duplex":171,_process:112,buffer:13,"core-util-is":19,dup:42,inherits:57,stream:134}],175:[function(e,t,r){arguments[4][43][0].apply(r,arguments)},{"./lib/_stream_transform.js":173,dup:43}],176:[function(e,t,r){var n=e("path");t.exports=i;function i(e,t){var r={paths:[],named:{},unnamed:[]};function i(e){if(!r.named[e]){r.named[e]={children:[]}}return r.named[e]}e.on("data",function(a){if(t===null){e.on("data",function(){});return}if(a.path){var o=i(a.path);o.file=a;var s=i(n.dirname(a.path));if(o!==s)s.children.push(o);r.paths.push(a.path)}else{r.unnamed.push({file:a,children:[]})}});e.on("error",function(e){t&&t(e);t=null});e.on("end",function(){t&&t(null,r);t=null})}},{path:108}],177:[function(e,t,r){var n=t.exports={};n.randomString=i;n.cleanPath=a;function i(){return Math.random().toString(36).slice(2)+Math.random().toString(36).slice(2)+Math.random().toString(36).slice(2)+Math.random().toString(36).slice(2)}function a(e,t){if(!e)return"";if(!t)return e;if(t[t.length-1]!="/"){t+="/"}e=e.replace(t,"");e=e.replace(/[\/]+/g,"/");return e}},{}],178:[function(e,t,r){var n=e("./mp2v_flat");var i=e("./mp2v_tree");var a=t.exports=i;a.flat=n;a.tree=i},{"./mp2v_flat":179,"./mp2v_tree":180}],179:[function(e,t,r){var n=e("multipart-stream");var i=e("duplexify");var a=e("stream");var o=e("./common");l=o.randomString;t.exports=s;function s(e){e=e||{};e.boundary=e.boundary||l();var t=new a.Writable({objectMode:true});var r=new a.PassThrough({objectMode:true});var o=new n(e.boundary);t._write=function(e,t,r){f(o,e,r)};t.on("finish",function(){o.pipe(r)});var s=i.obj(t,r);s.boundary=e.boundary;return s}function f(e,t,r){var n=t.contents;if(n===null)n=u();e.addPart({body:t.contents,headers:c(t)});r(null)}function u(){var e=new a.PassThrough({objectMode:true});e.write(null);return e}function c(e){var t=o.cleanPath(e.path,e.base);var r={};r["Content-Disposition"]='file; filename="'+t+'"';if(e.isDirectory()){r["Content-Type"]="text/directory"}else{r["Content-Type"]="application/octet-stream"}return r}function l(){return Math.random().toString(36).slice(2)+Math.random().toString(36).slice(2)+Math.random().toString(36).slice(2)+Math.random().toString(36).slice(2)}},{"./common":177,duplexify:20,"multipart-stream":98,stream:134}],180:[function(e,t,r){var n=e("multipart-stream");var i=e("duplexify");var a=e("stream");var o=e("path");var s=e("./collect");var f=e("./common");var u=f.randomString;t.exports=c;function c(e){e=e||{};e.boundary=e.boundary||u();var t=new a.PassThrough({objectMode:true});var r=new a.PassThrough({objectMode:true});var n=i.obj(r,t);n.boundary=e.boundary;s(r,function(r,i){if(r){t.emit("error",r);return}try{var a=l(e.boundary,i);n.multipartHdr="Content-Type: multipart/mixed; boundary="+a.boundary;if(e.writeHeader){t.write(n.multipartHdr+"\r\n");t.write("\r\n")}a.pipe(t)}catch(o){t.emit("error",o)}});return n}function l(e,t){var r=[];t.paths.sort();for(var i=0;i<t.paths.length;i++){var a=t.paths[i];var o=p(t,a);if(!o)continue;r.push({body:o,headers:g(t.named[a])})}for(var i=0;i<t.unnamed.length;i++){var s=t.unnamed[i];var o=d(t,s);if(!o)continue;r.push({body:o,headers:g(s)})}if(r.length==0){var o=h("--"+e+"--\r\n");o.boundary=e;return o}var f=new n(e);for(var i=0;i<r.length;i++){f.addPart(r[i])}return f}function h(e){var t=new a.PassThrough;t.end(e);return t}function p(e,t){var r=e.named[t];if(!r){throw new Error("no object for path. lib error.")}if(!r.file){return}if(r.done)return null;r.done=true;return d(e,r)}function d(e,t){if(t.file.isDirectory()){return v(e,t)}return t.file.contents}function v(e,t){t.boundary=u();if(!t.children||t.children.length<1){return h("--"+t.boundary+"--\r\n")}var r=new n(t.boundary);for(var i=0;i<t.children.length;i++){var a=t.children[i];if(!a.file){throw new Error("child has no file. lib error")}var o=p(e,a.file.path);r.addPart({body:o,headers:g(a)})}return r}function g(e){var t=f.cleanPath(e.file.path,e.file.base);var r={};r["Content-Disposition"]='file; filename="'+t+'"';if(e.file.isDirectory()){r["Content-Type"]="multipart/mixed; boundary="+e.boundary}else{r["Content-Type"]="application/octet-stream"}return r}},{"./collect":176,"./common":177,duplexify:20,"multipart-stream":98,path:108,stream:134}],181:[function(e,t,r){(function(r){var n=e("path");var i=e("clone");var a=e("clone-stats");var o=e("./lib/cloneBuffer");var s=e("./lib/isBuffer");var f=e("./lib/isStream");var u=e("./lib/isNull");var c=e("./lib/inspectStream");var l=e("stream");var h=e("replace-ext");function p(e){if(!e)e={};var t=e.path?[e.path]:e.history;this.history=t||[];this.cwd=e.cwd||r.cwd();this.base=e.base||this.cwd;this.stat=e.stat||null;this.contents=e.contents||null;this._isVinyl=true}p.prototype.isBuffer=function(){return s(this.contents)};p.prototype.isStream=function(){return f(this.contents)};p.prototype.isNull=function(){return u(this.contents)};p.prototype.isDirectory=function(){return this.isNull()&&this.stat&&this.stat.isDirectory()};p.prototype.clone=function(e){if(typeof e==="boolean"){e={deep:e,contents:true}}else if(!e){e={deep:true,contents:true}}else{e.deep=e.deep===true;e.contents=e.contents!==false}var t;if(this.isStream()){t=this.contents.pipe(new l.PassThrough);this.contents=this.contents.pipe(new l.PassThrough)}else if(this.isBuffer()){t=e.contents?o(this.contents):this.contents}var r=new p({cwd:this.cwd,base:this.base,stat:this.stat?a(this.stat):null,history:this.history.slice(),contents:t});Object.keys(this).forEach(function(t){if(t==="_contents"||t==="stat"||t==="history"||t==="path"||t==="base"||t==="cwd"){return}r[t]=e.deep?i(this[t],true):this[t]},this);return r};p.prototype.pipe=function(e,t){if(!t)t={};if(typeof t.end==="undefined")t.end=true;if(this.isStream()){return this.contents.pipe(e,t)}if(this.isBuffer()){if(t.end){e.end(this.contents)}else{e.write(this.contents)}return e}if(t.end)e.end();return e};p.prototype.inspect=function(){var e=[];var t=this.base&&this.path?this.relative:this.path;if(t){e.push('"'+t+'"')}if(this.isBuffer()){e.push(this.contents.inspect())}if(this.isStream()){e.push(c(this.contents))}return"<File "+e.join(" ")+">"};p.isVinyl=function(e){return e&&e._isVinyl===true};Object.defineProperty(p.prototype,"contents",{get:function(){return this._contents},set:function(e){if(!s(e)&&!f(e)&&!u(e)){throw new Error("File.contents can only be a Buffer, a Stream, or null.")}this._contents=e}});Object.defineProperty(p.prototype,"relative",{get:function(){if(!this.base)throw new Error("No base specified! Can not get relative.");if(!this.path)throw new Error("No path specified! Can not get relative.");return n.relative(this.base,this.path)},set:function(){throw new Error("File.relative is generated from the base and path attributes. Do not modify it.")}});Object.defineProperty(p.prototype,"dirname",{get:function(){if(!this.path)throw new Error("No path specified! Can not get dirname.");return n.dirname(this.path)},set:function(e){if(!this.path)throw new Error("No path specified! Can not set dirname.");this.path=n.join(e,n.basename(this.path))}});Object.defineProperty(p.prototype,"basename",{get:function(){if(!this.path)throw new Error("No path specified! Can not get basename.");return n.basename(this.path)},set:function(e){if(!this.path)throw new Error("No path specified! Can not set basename.");this.path=n.join(n.dirname(this.path),e)}});Object.defineProperty(p.prototype,"extname",{get:function(){if(!this.path)throw new Error("No path specified! Can not get extname.");return n.extname(this.path)},set:function(e){if(!this.path)throw new Error("No path specified! Can not set extname.");this.path=h(this.path,e)}});Object.defineProperty(p.prototype,"path",{get:function(){return this.history[this.history.length-1]},set:function(e){if(typeof e!=="string")throw new Error("path should be string");if(e&&e!==this.path){this.history.push(e)}}});t.exports=p}).call(this,e("_process"))},{"./lib/cloneBuffer":182,"./lib/inspectStream":183,"./lib/isBuffer":184,"./lib/isNull":185,"./lib/isStream":186,_process:112,clone:16,"clone-stats":15,path:108,"replace-ext":131,stream:134}],182:[function(e,t,r){var n=e("buffer").Buffer;t.exports=function(e){var t=new n(e.length);e.copy(t);return t}},{buffer:13}],183:[function(e,t,r){var n=e("./isStream");t.exports=function(e){if(!n(e))return;var t=e.constructor.name;if(t==="Stream")t="";return"<"+t+"Stream>"}},{"./isStream":186}],184:[function(e,t,r){t.exports=e("buffer").Buffer.isBuffer},{buffer:13}],185:[function(e,t,r){t.exports=function(e){return e===null}},{}],186:[function(e,t,r){var n=e("stream").Stream;t.exports=function(e){return!!e&&e instanceof n}},{stream:134}],187:[function(e,t,r){t.exports=n;function n(e,t){if(e&&t)return n(e)(t);if(typeof e!=="function")throw new TypeError("need wrapper function");Object.keys(e).forEach(function(t){r[t]=e[t]});return r;function r(){var t=new Array(arguments.length);for(var r=0;r<t.length;r++){t[r]=arguments[r]}var n=e.apply(this,t);var i=t[t.length-1];if(typeof n==="function"&&n!==i){Object.keys(i).forEach(function(e){n[e]=i[e]})}return n}}},{}],188:[function(e,t,r){t.exports=i;var n=Object.prototype.hasOwnProperty;function i(){var e={};for(var t=0;t<arguments.length;t++){var r=arguments[t];for(var i in r){if(n.call(r,i)){e[i]=r[i]}}}return e}},{}],189:[function(e,t,r){t.exports={name:"ipfs-api",version:"2.3.2",description:"A client library for the IPFS API",main:"src/index.js",dependencies:{brfs:"^1.4.0","merge-stream":"^1.0.0",multiaddr:"^1.0.0","multipart-stream":"^2.0.0",vinyl:"^0.5.1","vinyl-fs-browser":"^0.1.0","vinyl-multipart-stream":"^1.2.5"},browserify:{transform:["brfs"]},repository:{type:"git",url:"https://github.com/ipfs/node-ipfs-api"},devDependencies:{browserify:"^11.0.0","ipfsd-ctl":"^0.3.3",mocha:"^2.2.5","pre-commit":"^1.0.6",standard:"^3.3.2","uglify-js":"^2.4.24"},scripts:{test:"./node_modules/.bin/mocha",lint:"./node_modules/.bin/standard --format",build:"./node_modules/.bin/browserify -t brfs -s ipfsAPI -e ./src/index.js | tee dist/ipfsapi.js | ./node_modules/.bin/uglifyjs -m > dist/ipfsapi.min.js"},"pre-commit":["lint"],keywords:["ipfs"],author:"Matt Bell <mappum@gmail.com>",contributors:["Travis Person <travis.person@gmail.com>","Jeromy Jonson <why@ipfs.io>"],license:"MIT",bugs:{url:"https://github.com/ipfs/node-ipfs-api/issues"},homepage:"https://github.com/ipfs/node-ipfs-api"}},{}],190:[function(e,t,r){var n=e("../package.json");r=t.exports={"api-path":"/api/v0/","user-agent":"/node-"+n.name+"/"+n.version+"/",host:"localhost",port:"5001",protocol:"http"}},{"../package.json":189}],191:[function(e,t,r){(function(n){var i=e("vinyl");var a=e("vinyl-fs-browser");var o=e("vinyl-multipart-stream");var s=e("stream");var f=e("merge-stream");r=t.exports=u;function u(e,t){if(!e)return null;if(!Array.isArray(e))e=[e];var r=new f;var n=new s.PassThrough({objectMode:true});r.add(n);for(var i=0;i<e.length;i++){var u=e[i];if(typeof u==="string"){r.add(a.src(u,{buffer:false}));if(t.r||t.recursive){r.add(a.src(u+"/**/*",{buffer:false}))}}else{n.push(c(u))}}n.end();return r.pipe(o())}function c(e){if(e instanceof i){return e}var t={cwd:"/",base:"/",path:""};if(e.contents&&e.path){t.path=e.path;t.cwd=e.cwd||t.cwd;t.base=e.base||t.base;t.contents=e.contents}else{t.contents=e}t.contents=l(t.contents);return new i(t)}function l(e){if(n.isBuffer(e))return e;if(typeof e==="string")return e;if(e instanceof s.Stream)return e;if(typeof e.pipe==="function"){var t=new s.PassThrough;return e.pipe(t)}throw new Error("vinyl will not accept: "+e)}}).call(this,{isBuffer:e("../node_modules/is-buffer/index.js")})},{"../node_modules/is-buffer/index.js":59,"merge-stream":86,stream:134,vinyl:181,"vinyl-fs-browser":153,"vinyl-multipart-stream":178}],192:[function(e,t,r){(function(n){var i=e("multiaddr");var a=e("./config");var o=e("./request-api");r=t.exports=s;function s(e,t,r){var f=this;if(!(f instanceof s)){return new s(e,t,r)}try{var u=i(e).nodeAddress();a.host=u.address;a.port=u.port}catch(c){if(typeof e==="string"){a.host=e;a.port=t&&typeof t!=="object"?t:a.port}}var l=arguments.length;while(!r&&l-- >0){r=arguments[l];if(r)break}Object.assign(a,r);if(!a.host&&window&&window.location){var h=window.location.host.split(":");a.host=h[0];a.port=h[1]}function p(e){return function(t,r){if(typeof t==="function"){r=t;t={}}return o(e,null,t,null,r)}}function d(e){return function(t,r,n){if(typeof r==="function"){n=r;r={}}return o(e,t,r,null,n)}}f.send=o;f.add=function(e,t,r){if(typeof t==="function"&&r===undefined){r=t;t={}}return o("add",null,t,e,r)};f.cat=d("cat");f.ls=d("ls");f.config={get:d("config"),set:function(e,t,r,n){if(typeof r==="function"){n=r;r={}}return o("config",[e,t],r,null,n)},show:function(e){return o("config/show",null,null,null,true,e)},replace:function(e,t){return o("config/replace",null,null,e,t)}};f.update={apply:p("update"),check:p("update/check"),log:p("update/log")};f.version=p("version");f.commands=p("commands");f.mount=function(e,t,r){if(typeof e==="function"){r=e;e=null}else if(typeof t==="function"){r=t;t=null}var n={};if(e)n.f=e;if(t)n.n=t;return o("mount",null,n,null,r)};f.diag={net:p("diag/net")};f.block={get:d("block/get"),put:function(e,t){if(Array.isArray(e)){return t(null,new Error("block.put() only accepts 1 file"))}return o("block/put",null,null,e,t)}};f.object={get:d("object/get"),put:function(e,t,r){if(typeof t==="function"){return r(null,new Error("Must specify an object encoding ('json' or 'protobuf')"))}return o("object/put",t,null,e,r)},data:d("object/data"),stat:d("object/stat"),links:d("object/links")};f.swarm={peers:p("swarm/peers"),connect:d("swarm/peers")};f.ping=function(e,t){return o("ping",e,{n:1},null,function(e,r){if(e)return t(e,null);t(null,r[1])})};f.id=function(e,t){if(typeof e==="function"){t=e;e=null}return o("id",e,null,null,t)};f.pin={add:function(e,t,r){if(typeof t==="function"){r=t;t=null}o("pin/add",e,t,null,r)},remove:function(e,t,r){if(typeof t==="function"){r=t;t=null}o("pin/rm",e,t,null,r)},list:function(e,t){if(typeof e==="function"){t=e;e=null}var r=null;if(e)r={type:e};return o("pin/ls",null,r,null,t)}};f.gateway={enable:p("gateway/enable"),disable:p("gateway/disable")};f.log={tail:function(e){return o("log/tail",null,{enc:"text"},null,true,e)}};f.name={publish:d("name/publish"),resolve:d("name/resolve")};f.Buffer=n;f.refs=d("refs");f.refs.local=p("refs/local");f.dht={findprovs:d("dht/findprovs"),get:function(e,t,r){if(typeof t==="function"&&!r){r=t;t=null}return o("dht/get",e,t,null,function(e,t){if(e)return r(e);if(!t)return r(new Error("empty response"));if(t.length===0)return r(new Error("no value returned for key"));if(t[0].Type===5){r(null,t[0].Extra)}else{r(t)}})},put:function(e,t,r,n){if(typeof r==="function"&&!n){n=r;r=null}return o("dht/put",[e,t],r,null,n)}}}}).call(this,e("buffer").Buffer)},{"./config":190,"./request-api":193,buffer:13,multiaddr:96}],193:[function(e,t,r){var n=e("http");var i=e("querystring");var a=e("./get-files-stream");var o=e("./config");r=t.exports=s;function s(e,t,r,s,f,u){var c,l,h;h="application/json";if(Array.isArray(e))e=e.join("/");r=r||{};if(t&&!Array.isArray(t))t=[t];if(t)r.arg=t;r["stream-channels"]=true;c=i.stringify(r);if(s){l=a(s,r);if(!l.boundary){throw new Error("no boundary in multipart stream")}h="multipart/form-data; boundary="+l.boundary}if(typeof f==="function"){u=f;f=false}var p=o.protocol.slice(-1)===":"?o.protocol:o.protocol+":";var d={method:s?"POST":"GET",host:o.host,port:o.port,protocol:p,path:o["api-path"]+e+"?"+c,headers:{"User-Agent":o["user-agent"],"Content-Type":h},withCredentials:false};var v=n.request(d,function(e){var t="";var r=[];var n=!!e.headers&&!!e.headers["x-stream-output"];var i=!!e.headers&&!!e.headers["x-chunked-output"];if(n&&!f)return u(null,e);if(i&&f)return u(null,e);e.on("data",function(e){if(!i){t+=e.toString("binary");return t}try{var n=JSON.parse(e.toString());r.push(n)}catch(a){i=false;t+=e.toString("binary")}});e.on("end",function(){var n;if(!i){try{n=JSON.parse(t);t=n}catch(a){}}else{t=r}if(e.statusCode>=400||!e.statusCode){if(!t)t=new Error;return u(t,null)}return u(null,t)});e.on("error",function(e){return u(e,null)})});v.on("error",function(e){return u(e,null)});if(l){l.pipe(v)}else{v.end()}return v}},{"./config":190,"./get-files-stream":191,http:135,querystring:116}]},{},[192])(192)});

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{}],14:[function(require,module,exports){
/**
 * Determine if an object is Buffer
 *
 * Author:   Feross Aboukhadijeh <feross@feross.org> <http://feross.org>
 * License:  MIT
 *
 * `npm install is-buffer`
 */

module.exports = function (obj) {
  return !!(obj != null &&
    (obj._isBuffer || // For Safari 5-7 (missing Object.prototype.constructor)
      (obj.constructor &&
      typeof obj.constructor.isBuffer === 'function' &&
      obj.constructor.isBuffer(obj))
    ))
}

},{}],15:[function(require,module,exports){
var toString = {}.toString;

module.exports = Array.isArray || function (arr) {
  return toString.call(arr) == '[object Array]';
};

},{}],16:[function(require,module,exports){
(function (process){
'use strict';

if (!process.version ||
    process.version.indexOf('v0.') === 0 ||
    process.version.indexOf('v1.') === 0 && process.version.indexOf('v1.8.') !== 0) {
  module.exports = nextTick;
} else {
  module.exports = process.nextTick;
}

function nextTick(fn, arg1, arg2, arg3) {
  if (typeof fn !== 'function') {
    throw new TypeError('"callback" argument must be a function');
  }
  var len = arguments.length;
  var args, i;
  switch (len) {
  case 0:
  case 1:
    return process.nextTick(fn);
  case 2:
    return process.nextTick(function afterTickOne() {
      fn.call(null, arg1);
    });
  case 3:
    return process.nextTick(function afterTickTwo() {
      fn.call(null, arg1, arg2);
    });
  case 4:
    return process.nextTick(function afterTickThree() {
      fn.call(null, arg1, arg2, arg3);
    });
  default:
    args = new Array(len - 1);
    i = 0;
    while (i < args.length) {
      args[i++] = arguments[i];
    }
    return process.nextTick(function afterTick() {
      fn.apply(null, args);
    });
  }
}

}).call(this,require('_process'))
},{"_process":17}],17:[function(require,module,exports){
// shim for using process in browser

var process = module.exports = {};

// cached from whatever global is present so that test runners that stub it
// don't break things.  But we need to wrap it in a try catch in case it is
// wrapped in strict mode code which doesn't define any globals.  It's inside a
// function because try/catches deoptimize in certain engines.

var cachedSetTimeout;
var cachedClearTimeout;

(function () {
  try {
    cachedSetTimeout = setTimeout;
  } catch (e) {
    cachedSetTimeout = function () {
      throw new Error('setTimeout is not defined');
    }
  }
  try {
    cachedClearTimeout = clearTimeout;
  } catch (e) {
    cachedClearTimeout = function () {
      throw new Error('clearTimeout is not defined');
    }
  }
} ())
var queue = [];
var draining = false;
var currentQueue;
var queueIndex = -1;

function cleanUpNextTick() {
    if (!draining || !currentQueue) {
        return;
    }
    draining = false;
    if (currentQueue.length) {
        queue = currentQueue.concat(queue);
    } else {
        queueIndex = -1;
    }
    if (queue.length) {
        drainQueue();
    }
}

function drainQueue() {
    if (draining) {
        return;
    }
    var timeout = cachedSetTimeout(cleanUpNextTick);
    draining = true;

    var len = queue.length;
    while(len) {
        currentQueue = queue;
        queue = [];
        while (++queueIndex < len) {
            if (currentQueue) {
                currentQueue[queueIndex].run();
            }
        }
        queueIndex = -1;
        len = queue.length;
    }
    currentQueue = null;
    draining = false;
    cachedClearTimeout(timeout);
}

process.nextTick = function (fun) {
    var args = new Array(arguments.length - 1);
    if (arguments.length > 1) {
        for (var i = 1; i < arguments.length; i++) {
            args[i - 1] = arguments[i];
        }
    }
    queue.push(new Item(fun, args));
    if (queue.length === 1 && !draining) {
        cachedSetTimeout(drainQueue, 0);
    }
};

// v8 likes predictible objects
function Item(fun, array) {
    this.fun = fun;
    this.array = array;
}
Item.prototype.run = function () {
    this.fun.apply(null, this.array);
};
process.title = 'browser';
process.browser = true;
process.env = {};
process.argv = [];
process.version = ''; // empty string to avoid regexp issues
process.versions = {};

function noop() {}

process.on = noop;
process.addListener = noop;
process.once = noop;
process.off = noop;
process.removeListener = noop;
process.removeAllListeners = noop;
process.emit = noop;

process.binding = function (name) {
    throw new Error('process.binding is not supported');
};

process.cwd = function () { return '/' };
process.chdir = function (dir) {
    throw new Error('process.chdir is not supported');
};
process.umask = function() { return 0; };

},{}],18:[function(require,module,exports){
// a duplex stream is just a stream that is both readable and writable.
// Since JS doesn't have multiple prototypal inheritance, this class
// prototypally inherits from Readable, and then parasitically from
// Writable.

'use strict';

/*<replacement>*/

var objectKeys = Object.keys || function (obj) {
  var keys = [];
  for (var key in obj) {
    keys.push(key);
  }return keys;
};
/*</replacement>*/

module.exports = Duplex;

/*<replacement>*/
var processNextTick = require('process-nextick-args');
/*</replacement>*/

/*<replacement>*/
var util = require('core-util-is');
util.inherits = require('inherits');
/*</replacement>*/

var Readable = require('./_stream_readable');
var Writable = require('./_stream_writable');

util.inherits(Duplex, Readable);

var keys = objectKeys(Writable.prototype);
for (var v = 0; v < keys.length; v++) {
  var method = keys[v];
  if (!Duplex.prototype[method]) Duplex.prototype[method] = Writable.prototype[method];
}

function Duplex(options) {
  if (!(this instanceof Duplex)) return new Duplex(options);

  Readable.call(this, options);
  Writable.call(this, options);

  if (options && options.readable === false) this.readable = false;

  if (options && options.writable === false) this.writable = false;

  this.allowHalfOpen = true;
  if (options && options.allowHalfOpen === false) this.allowHalfOpen = false;

  this.once('end', onend);
}

// the no-half-open enforcer
function onend() {
  // if we allow half-open state, or if the writable side ended,
  // then we're ok.
  if (this.allowHalfOpen || this._writableState.ended) return;

  // no more data can be written.
  // But allow more writes to happen in this tick.
  processNextTick(onEndNT, this);
}

function onEndNT(self) {
  self.end();
}

function forEach(xs, f) {
  for (var i = 0, l = xs.length; i < l; i++) {
    f(xs[i], i);
  }
}
},{"./_stream_readable":20,"./_stream_writable":22,"core-util-is":9,"inherits":12,"process-nextick-args":16}],19:[function(require,module,exports){
// a passthrough stream.
// basically just the most minimal sort of Transform stream.
// Every written chunk gets output as-is.

'use strict';

module.exports = PassThrough;

var Transform = require('./_stream_transform');

/*<replacement>*/
var util = require('core-util-is');
util.inherits = require('inherits');
/*</replacement>*/

util.inherits(PassThrough, Transform);

function PassThrough(options) {
  if (!(this instanceof PassThrough)) return new PassThrough(options);

  Transform.call(this, options);
}

PassThrough.prototype._transform = function (chunk, encoding, cb) {
  cb(null, chunk);
};
},{"./_stream_transform":21,"core-util-is":9,"inherits":12}],20:[function(require,module,exports){
(function (process){
'use strict';

module.exports = Readable;

/*<replacement>*/
var processNextTick = require('process-nextick-args');
/*</replacement>*/

/*<replacement>*/
var isArray = require('isarray');
/*</replacement>*/

/*<replacement>*/
var Buffer = require('buffer').Buffer;
/*</replacement>*/

Readable.ReadableState = ReadableState;

var EE = require('events');

/*<replacement>*/
var EElistenerCount = function (emitter, type) {
  return emitter.listeners(type).length;
};
/*</replacement>*/

/*<replacement>*/
var Stream;
(function () {
  try {
    Stream = require('st' + 'ream');
  } catch (_) {} finally {
    if (!Stream) Stream = require('events').EventEmitter;
  }
})();
/*</replacement>*/

var Buffer = require('buffer').Buffer;

/*<replacement>*/
var util = require('core-util-is');
util.inherits = require('inherits');
/*</replacement>*/

/*<replacement>*/
var debugUtil = require('util');
var debug = undefined;
if (debugUtil && debugUtil.debuglog) {
  debug = debugUtil.debuglog('stream');
} else {
  debug = function () {};
}
/*</replacement>*/

var StringDecoder;

util.inherits(Readable, Stream);

var Duplex;
function ReadableState(options, stream) {
  Duplex = Duplex || require('./_stream_duplex');

  options = options || {};

  // object stream flag. Used to make read(n) ignore n and to
  // make all the buffer merging and length checks go away
  this.objectMode = !!options.objectMode;

  if (stream instanceof Duplex) this.objectMode = this.objectMode || !!options.readableObjectMode;

  // the point at which it stops calling _read() to fill the buffer
  // Note: 0 is a valid value, means "don't call _read preemptively ever"
  var hwm = options.highWaterMark;
  var defaultHwm = this.objectMode ? 16 : 16 * 1024;
  this.highWaterMark = hwm || hwm === 0 ? hwm : defaultHwm;

  // cast to ints.
  this.highWaterMark = ~ ~this.highWaterMark;

  this.buffer = [];
  this.length = 0;
  this.pipes = null;
  this.pipesCount = 0;
  this.flowing = null;
  this.ended = false;
  this.endEmitted = false;
  this.reading = false;

  // a flag to be able to tell if the onwrite cb is called immediately,
  // or on a later tick.  We set this to true at first, because any
  // actions that shouldn't happen until "later" should generally also
  // not happen before the first write call.
  this.sync = true;

  // whenever we return null, then we set a flag to say
  // that we're awaiting a 'readable' event emission.
  this.needReadable = false;
  this.emittedReadable = false;
  this.readableListening = false;
  this.resumeScheduled = false;

  // Crypto is kind of old and crusty.  Historically, its default string
  // encoding is 'binary' so we have to make this configurable.
  // Everything else in the universe uses 'utf8', though.
  this.defaultEncoding = options.defaultEncoding || 'utf8';

  // when piping, we only care about 'readable' events that happen
  // after read()ing all the bytes and not getting any pushback.
  this.ranOut = false;

  // the number of writers that are awaiting a drain event in .pipe()s
  this.awaitDrain = 0;

  // if true, a maybeReadMore has been scheduled
  this.readingMore = false;

  this.decoder = null;
  this.encoding = null;
  if (options.encoding) {
    if (!StringDecoder) StringDecoder = require('string_decoder/').StringDecoder;
    this.decoder = new StringDecoder(options.encoding);
    this.encoding = options.encoding;
  }
}

var Duplex;
function Readable(options) {
  Duplex = Duplex || require('./_stream_duplex');

  if (!(this instanceof Readable)) return new Readable(options);

  this._readableState = new ReadableState(options, this);

  // legacy
  this.readable = true;

  if (options && typeof options.read === 'function') this._read = options.read;

  Stream.call(this);
}

// Manually shove something into the read() buffer.
// This returns true if the highWaterMark has not been hit yet,
// similar to how Writable.write() returns true if you should
// write() some more.
Readable.prototype.push = function (chunk, encoding) {
  var state = this._readableState;

  if (!state.objectMode && typeof chunk === 'string') {
    encoding = encoding || state.defaultEncoding;
    if (encoding !== state.encoding) {
      chunk = new Buffer(chunk, encoding);
      encoding = '';
    }
  }

  return readableAddChunk(this, state, chunk, encoding, false);
};

// Unshift should *always* be something directly out of read()
Readable.prototype.unshift = function (chunk) {
  var state = this._readableState;
  return readableAddChunk(this, state, chunk, '', true);
};

Readable.prototype.isPaused = function () {
  return this._readableState.flowing === false;
};

function readableAddChunk(stream, state, chunk, encoding, addToFront) {
  var er = chunkInvalid(state, chunk);
  if (er) {
    stream.emit('error', er);
  } else if (chunk === null) {
    state.reading = false;
    onEofChunk(stream, state);
  } else if (state.objectMode || chunk && chunk.length > 0) {
    if (state.ended && !addToFront) {
      var e = new Error('stream.push() after EOF');
      stream.emit('error', e);
    } else if (state.endEmitted && addToFront) {
      var e = new Error('stream.unshift() after end event');
      stream.emit('error', e);
    } else {
      var skipAdd;
      if (state.decoder && !addToFront && !encoding) {
        chunk = state.decoder.write(chunk);
        skipAdd = !state.objectMode && chunk.length === 0;
      }

      if (!addToFront) state.reading = false;

      // Don't add to the buffer if we've decoded to an empty string chunk and
      // we're not in object mode
      if (!skipAdd) {
        // if we want the data now, just emit it.
        if (state.flowing && state.length === 0 && !state.sync) {
          stream.emit('data', chunk);
          stream.read(0);
        } else {
          // update the buffer info.
          state.length += state.objectMode ? 1 : chunk.length;
          if (addToFront) state.buffer.unshift(chunk);else state.buffer.push(chunk);

          if (state.needReadable) emitReadable(stream);
        }
      }

      maybeReadMore(stream, state);
    }
  } else if (!addToFront) {
    state.reading = false;
  }

  return needMoreData(state);
}

// if it's past the high water mark, we can push in some more.
// Also, if we have no data yet, we can stand some
// more bytes.  This is to work around cases where hwm=0,
// such as the repl.  Also, if the push() triggered a
// readable event, and the user called read(largeNumber) such that
// needReadable was set, then we ought to push more, so that another
// 'readable' event will be triggered.
function needMoreData(state) {
  return !state.ended && (state.needReadable || state.length < state.highWaterMark || state.length === 0);
}

// backwards compatibility.
Readable.prototype.setEncoding = function (enc) {
  if (!StringDecoder) StringDecoder = require('string_decoder/').StringDecoder;
  this._readableState.decoder = new StringDecoder(enc);
  this._readableState.encoding = enc;
  return this;
};

// Don't raise the hwm > 8MB
var MAX_HWM = 0x800000;
function computeNewHighWaterMark(n) {
  if (n >= MAX_HWM) {
    n = MAX_HWM;
  } else {
    // Get the next highest power of 2
    n--;
    n |= n >>> 1;
    n |= n >>> 2;
    n |= n >>> 4;
    n |= n >>> 8;
    n |= n >>> 16;
    n++;
  }
  return n;
}

function howMuchToRead(n, state) {
  if (state.length === 0 && state.ended) return 0;

  if (state.objectMode) return n === 0 ? 0 : 1;

  if (n === null || isNaN(n)) {
    // only flow one buffer at a time
    if (state.flowing && state.buffer.length) return state.buffer[0].length;else return state.length;
  }

  if (n <= 0) return 0;

  // If we're asking for more than the target buffer level,
  // then raise the water mark.  Bump up to the next highest
  // power of 2, to prevent increasing it excessively in tiny
  // amounts.
  if (n > state.highWaterMark) state.highWaterMark = computeNewHighWaterMark(n);

  // don't have that much.  return null, unless we've ended.
  if (n > state.length) {
    if (!state.ended) {
      state.needReadable = true;
      return 0;
    } else {
      return state.length;
    }
  }

  return n;
}

// you can override either this method, or the async _read(n) below.
Readable.prototype.read = function (n) {
  debug('read', n);
  var state = this._readableState;
  var nOrig = n;

  if (typeof n !== 'number' || n > 0) state.emittedReadable = false;

  // if we're doing read(0) to trigger a readable event, but we
  // already have a bunch of data in the buffer, then just trigger
  // the 'readable' event and move on.
  if (n === 0 && state.needReadable && (state.length >= state.highWaterMark || state.ended)) {
    debug('read: emitReadable', state.length, state.ended);
    if (state.length === 0 && state.ended) endReadable(this);else emitReadable(this);
    return null;
  }

  n = howMuchToRead(n, state);

  // if we've ended, and we're now clear, then finish it up.
  if (n === 0 && state.ended) {
    if (state.length === 0) endReadable(this);
    return null;
  }

  // All the actual chunk generation logic needs to be
  // *below* the call to _read.  The reason is that in certain
  // synthetic stream cases, such as passthrough streams, _read
  // may be a completely synchronous operation which may change
  // the state of the read buffer, providing enough data when
  // before there was *not* enough.
  //
  // So, the steps are:
  // 1. Figure out what the state of things will be after we do
  // a read from the buffer.
  //
  // 2. If that resulting state will trigger a _read, then call _read.
  // Note that this may be asynchronous, or synchronous.  Yes, it is
  // deeply ugly to write APIs this way, but that still doesn't mean
  // that the Readable class should behave improperly, as streams are
  // designed to be sync/async agnostic.
  // Take note if the _read call is sync or async (ie, if the read call
  // has returned yet), so that we know whether or not it's safe to emit
  // 'readable' etc.
  //
  // 3. Actually pull the requested chunks out of the buffer and return.

  // if we need a readable event, then we need to do some reading.
  var doRead = state.needReadable;
  debug('need readable', doRead);

  // if we currently have less than the highWaterMark, then also read some
  if (state.length === 0 || state.length - n < state.highWaterMark) {
    doRead = true;
    debug('length less than watermark', doRead);
  }

  // however, if we've ended, then there's no point, and if we're already
  // reading, then it's unnecessary.
  if (state.ended || state.reading) {
    doRead = false;
    debug('reading or ended', doRead);
  }

  if (doRead) {
    debug('do read');
    state.reading = true;
    state.sync = true;
    // if the length is currently zero, then we *need* a readable event.
    if (state.length === 0) state.needReadable = true;
    // call internal read method
    this._read(state.highWaterMark);
    state.sync = false;
  }

  // If _read pushed data synchronously, then `reading` will be false,
  // and we need to re-evaluate how much data we can return to the user.
  if (doRead && !state.reading) n = howMuchToRead(nOrig, state);

  var ret;
  if (n > 0) ret = fromList(n, state);else ret = null;

  if (ret === null) {
    state.needReadable = true;
    n = 0;
  }

  state.length -= n;

  // If we have nothing in the buffer, then we want to know
  // as soon as we *do* get something into the buffer.
  if (state.length === 0 && !state.ended) state.needReadable = true;

  // If we tried to read() past the EOF, then emit end on the next tick.
  if (nOrig !== n && state.ended && state.length === 0) endReadable(this);

  if (ret !== null) this.emit('data', ret);

  return ret;
};

function chunkInvalid(state, chunk) {
  var er = null;
  if (!Buffer.isBuffer(chunk) && typeof chunk !== 'string' && chunk !== null && chunk !== undefined && !state.objectMode) {
    er = new TypeError('Invalid non-string/buffer chunk');
  }
  return er;
}

function onEofChunk(stream, state) {
  if (state.ended) return;
  if (state.decoder) {
    var chunk = state.decoder.end();
    if (chunk && chunk.length) {
      state.buffer.push(chunk);
      state.length += state.objectMode ? 1 : chunk.length;
    }
  }
  state.ended = true;

  // emit 'readable' now to make sure it gets picked up.
  emitReadable(stream);
}

// Don't emit readable right away in sync mode, because this can trigger
// another read() call => stack overflow.  This way, it might trigger
// a nextTick recursion warning, but that's not so bad.
function emitReadable(stream) {
  var state = stream._readableState;
  state.needReadable = false;
  if (!state.emittedReadable) {
    debug('emitReadable', state.flowing);
    state.emittedReadable = true;
    if (state.sync) processNextTick(emitReadable_, stream);else emitReadable_(stream);
  }
}

function emitReadable_(stream) {
  debug('emit readable');
  stream.emit('readable');
  flow(stream);
}

// at this point, the user has presumably seen the 'readable' event,
// and called read() to consume some data.  that may have triggered
// in turn another _read(n) call, in which case reading = true if
// it's in progress.
// However, if we're not ended, or reading, and the length < hwm,
// then go ahead and try to read some more preemptively.
function maybeReadMore(stream, state) {
  if (!state.readingMore) {
    state.readingMore = true;
    processNextTick(maybeReadMore_, stream, state);
  }
}

function maybeReadMore_(stream, state) {
  var len = state.length;
  while (!state.reading && !state.flowing && !state.ended && state.length < state.highWaterMark) {
    debug('maybeReadMore read 0');
    stream.read(0);
    if (len === state.length)
      // didn't get any data, stop spinning.
      break;else len = state.length;
  }
  state.readingMore = false;
}

// abstract method.  to be overridden in specific implementation classes.
// call cb(er, data) where data is <= n in length.
// for virtual (non-string, non-buffer) streams, "length" is somewhat
// arbitrary, and perhaps not very meaningful.
Readable.prototype._read = function (n) {
  this.emit('error', new Error('not implemented'));
};

Readable.prototype.pipe = function (dest, pipeOpts) {
  var src = this;
  var state = this._readableState;

  switch (state.pipesCount) {
    case 0:
      state.pipes = dest;
      break;
    case 1:
      state.pipes = [state.pipes, dest];
      break;
    default:
      state.pipes.push(dest);
      break;
  }
  state.pipesCount += 1;
  debug('pipe count=%d opts=%j', state.pipesCount, pipeOpts);

  var doEnd = (!pipeOpts || pipeOpts.end !== false) && dest !== process.stdout && dest !== process.stderr;

  var endFn = doEnd ? onend : cleanup;
  if (state.endEmitted) processNextTick(endFn);else src.once('end', endFn);

  dest.on('unpipe', onunpipe);
  function onunpipe(readable) {
    debug('onunpipe');
    if (readable === src) {
      cleanup();
    }
  }

  function onend() {
    debug('onend');
    dest.end();
  }

  // when the dest drains, it reduces the awaitDrain counter
  // on the source.  This would be more elegant with a .once()
  // handler in flow(), but adding and removing repeatedly is
  // too slow.
  var ondrain = pipeOnDrain(src);
  dest.on('drain', ondrain);

  var cleanedUp = false;
  function cleanup() {
    debug('cleanup');
    // cleanup event handlers once the pipe is broken
    dest.removeListener('close', onclose);
    dest.removeListener('finish', onfinish);
    dest.removeListener('drain', ondrain);
    dest.removeListener('error', onerror);
    dest.removeListener('unpipe', onunpipe);
    src.removeListener('end', onend);
    src.removeListener('end', cleanup);
    src.removeListener('data', ondata);

    cleanedUp = true;

    // if the reader is waiting for a drain event from this
    // specific writer, then it would cause it to never start
    // flowing again.
    // So, if this is awaiting a drain, then we just call it now.
    // If we don't know, then assume that we are waiting for one.
    if (state.awaitDrain && (!dest._writableState || dest._writableState.needDrain)) ondrain();
  }

  src.on('data', ondata);
  function ondata(chunk) {
    debug('ondata');
    var ret = dest.write(chunk);
    if (false === ret) {
      // If the user unpiped during `dest.write()`, it is possible
      // to get stuck in a permanently paused state if that write
      // also returned false.
      if (state.pipesCount === 1 && state.pipes[0] === dest && src.listenerCount('data') === 1 && !cleanedUp) {
        debug('false write response, pause', src._readableState.awaitDrain);
        src._readableState.awaitDrain++;
      }
      src.pause();
    }
  }

  // if the dest has an error, then stop piping into it.
  // however, don't suppress the throwing behavior for this.
  function onerror(er) {
    debug('onerror', er);
    unpipe();
    dest.removeListener('error', onerror);
    if (EElistenerCount(dest, 'error') === 0) dest.emit('error', er);
  }
  // This is a brutally ugly hack to make sure that our error handler
  // is attached before any userland ones.  NEVER DO THIS.
  if (!dest._events || !dest._events.error) dest.on('error', onerror);else if (isArray(dest._events.error)) dest._events.error.unshift(onerror);else dest._events.error = [onerror, dest._events.error];

  // Both close and finish should trigger unpipe, but only once.
  function onclose() {
    dest.removeListener('finish', onfinish);
    unpipe();
  }
  dest.once('close', onclose);
  function onfinish() {
    debug('onfinish');
    dest.removeListener('close', onclose);
    unpipe();
  }
  dest.once('finish', onfinish);

  function unpipe() {
    debug('unpipe');
    src.unpipe(dest);
  }

  // tell the dest that it's being piped to
  dest.emit('pipe', src);

  // start the flow if it hasn't been started already.
  if (!state.flowing) {
    debug('pipe resume');
    src.resume();
  }

  return dest;
};

function pipeOnDrain(src) {
  return function () {
    var state = src._readableState;
    debug('pipeOnDrain', state.awaitDrain);
    if (state.awaitDrain) state.awaitDrain--;
    if (state.awaitDrain === 0 && EElistenerCount(src, 'data')) {
      state.flowing = true;
      flow(src);
    }
  };
}

Readable.prototype.unpipe = function (dest) {
  var state = this._readableState;

  // if we're not piping anywhere, then do nothing.
  if (state.pipesCount === 0) return this;

  // just one destination.  most common case.
  if (state.pipesCount === 1) {
    // passed in one, but it's not the right one.
    if (dest && dest !== state.pipes) return this;

    if (!dest) dest = state.pipes;

    // got a match.
    state.pipes = null;
    state.pipesCount = 0;
    state.flowing = false;
    if (dest) dest.emit('unpipe', this);
    return this;
  }

  // slow case. multiple pipe destinations.

  if (!dest) {
    // remove all.
    var dests = state.pipes;
    var len = state.pipesCount;
    state.pipes = null;
    state.pipesCount = 0;
    state.flowing = false;

    for (var _i = 0; _i < len; _i++) {
      dests[_i].emit('unpipe', this);
    }return this;
  }

  // try to find the right one.
  var i = indexOf(state.pipes, dest);
  if (i === -1) return this;

  state.pipes.splice(i, 1);
  state.pipesCount -= 1;
  if (state.pipesCount === 1) state.pipes = state.pipes[0];

  dest.emit('unpipe', this);

  return this;
};

// set up data events if they are asked for
// Ensure readable listeners eventually get something
Readable.prototype.on = function (ev, fn) {
  var res = Stream.prototype.on.call(this, ev, fn);

  // If listening to data, and it has not explicitly been paused,
  // then call resume to start the flow of data on the next tick.
  if (ev === 'data' && false !== this._readableState.flowing) {
    this.resume();
  }

  if (ev === 'readable' && !this._readableState.endEmitted) {
    var state = this._readableState;
    if (!state.readableListening) {
      state.readableListening = true;
      state.emittedReadable = false;
      state.needReadable = true;
      if (!state.reading) {
        processNextTick(nReadingNextTick, this);
      } else if (state.length) {
        emitReadable(this, state);
      }
    }
  }

  return res;
};
Readable.prototype.addListener = Readable.prototype.on;

function nReadingNextTick(self) {
  debug('readable nexttick read 0');
  self.read(0);
}

// pause() and resume() are remnants of the legacy readable stream API
// If the user uses them, then switch into old mode.
Readable.prototype.resume = function () {
  var state = this._readableState;
  if (!state.flowing) {
    debug('resume');
    state.flowing = true;
    resume(this, state);
  }
  return this;
};

function resume(stream, state) {
  if (!state.resumeScheduled) {
    state.resumeScheduled = true;
    processNextTick(resume_, stream, state);
  }
}

function resume_(stream, state) {
  if (!state.reading) {
    debug('resume read 0');
    stream.read(0);
  }

  state.resumeScheduled = false;
  stream.emit('resume');
  flow(stream);
  if (state.flowing && !state.reading) stream.read(0);
}

Readable.prototype.pause = function () {
  debug('call pause flowing=%j', this._readableState.flowing);
  if (false !== this._readableState.flowing) {
    debug('pause');
    this._readableState.flowing = false;
    this.emit('pause');
  }
  return this;
};

function flow(stream) {
  var state = stream._readableState;
  debug('flow', state.flowing);
  if (state.flowing) {
    do {
      var chunk = stream.read();
    } while (null !== chunk && state.flowing);
  }
}

// wrap an old-style stream as the async data source.
// This is *not* part of the readable stream interface.
// It is an ugly unfortunate mess of history.
Readable.prototype.wrap = function (stream) {
  var state = this._readableState;
  var paused = false;

  var self = this;
  stream.on('end', function () {
    debug('wrapped end');
    if (state.decoder && !state.ended) {
      var chunk = state.decoder.end();
      if (chunk && chunk.length) self.push(chunk);
    }

    self.push(null);
  });

  stream.on('data', function (chunk) {
    debug('wrapped data');
    if (state.decoder) chunk = state.decoder.write(chunk);

    // don't skip over falsy values in objectMode
    if (state.objectMode && (chunk === null || chunk === undefined)) return;else if (!state.objectMode && (!chunk || !chunk.length)) return;

    var ret = self.push(chunk);
    if (!ret) {
      paused = true;
      stream.pause();
    }
  });

  // proxy all the other methods.
  // important when wrapping filters and duplexes.
  for (var i in stream) {
    if (this[i] === undefined && typeof stream[i] === 'function') {
      this[i] = function (method) {
        return function () {
          return stream[method].apply(stream, arguments);
        };
      }(i);
    }
  }

  // proxy certain important events.
  var events = ['error', 'close', 'destroy', 'pause', 'resume'];
  forEach(events, function (ev) {
    stream.on(ev, self.emit.bind(self, ev));
  });

  // when we try to consume some more bytes, simply unpause the
  // underlying stream.
  self._read = function (n) {
    debug('wrapped _read', n);
    if (paused) {
      paused = false;
      stream.resume();
    }
  };

  return self;
};

// exposed for testing purposes only.
Readable._fromList = fromList;

// Pluck off n bytes from an array of buffers.
// Length is the combined lengths of all the buffers in the list.
function fromList(n, state) {
  var list = state.buffer;
  var length = state.length;
  var stringMode = !!state.decoder;
  var objectMode = !!state.objectMode;
  var ret;

  // nothing in the list, definitely empty.
  if (list.length === 0) return null;

  if (length === 0) ret = null;else if (objectMode) ret = list.shift();else if (!n || n >= length) {
    // read it all, truncate the array.
    if (stringMode) ret = list.join('');else if (list.length === 1) ret = list[0];else ret = Buffer.concat(list, length);
    list.length = 0;
  } else {
    // read just some of it.
    if (n < list[0].length) {
      // just take a part of the first list item.
      // slice is the same for buffers and strings.
      var buf = list[0];
      ret = buf.slice(0, n);
      list[0] = buf.slice(n);
    } else if (n === list[0].length) {
      // first list is a perfect match
      ret = list.shift();
    } else {
      // complex case.
      // we have enough to cover it, but it spans past the first buffer.
      if (stringMode) ret = '';else ret = new Buffer(n);

      var c = 0;
      for (var i = 0, l = list.length; i < l && c < n; i++) {
        var buf = list[0];
        var cpy = Math.min(n - c, buf.length);

        if (stringMode) ret += buf.slice(0, cpy);else buf.copy(ret, c, 0, cpy);

        if (cpy < buf.length) list[0] = buf.slice(cpy);else list.shift();

        c += cpy;
      }
    }
  }

  return ret;
}

function endReadable(stream) {
  var state = stream._readableState;

  // If we get here before consuming all the bytes, then that is a
  // bug in node.  Should never happen.
  if (state.length > 0) throw new Error('endReadable called on non-empty stream');

  if (!state.endEmitted) {
    state.ended = true;
    processNextTick(endReadableNT, state, stream);
  }
}

function endReadableNT(state, stream) {
  // Check that we didn't get one last unshift.
  if (!state.endEmitted && state.length === 0) {
    state.endEmitted = true;
    stream.readable = false;
    stream.emit('end');
  }
}

function forEach(xs, f) {
  for (var i = 0, l = xs.length; i < l; i++) {
    f(xs[i], i);
  }
}

function indexOf(xs, x) {
  for (var i = 0, l = xs.length; i < l; i++) {
    if (xs[i] === x) return i;
  }
  return -1;
}
}).call(this,require('_process'))
},{"./_stream_duplex":18,"_process":17,"buffer":7,"core-util-is":9,"events":10,"inherits":12,"isarray":15,"process-nextick-args":16,"string_decoder/":24,"util":6}],21:[function(require,module,exports){
// a transform stream is a readable/writable stream where you do
// something with the data.  Sometimes it's called a "filter",
// but that's not a great name for it, since that implies a thing where
// some bits pass through, and others are simply ignored.  (That would
// be a valid example of a transform, of course.)
//
// While the output is causally related to the input, it's not a
// necessarily symmetric or synchronous transformation.  For example,
// a zlib stream might take multiple plain-text writes(), and then
// emit a single compressed chunk some time in the future.
//
// Here's how this works:
//
// The Transform stream has all the aspects of the readable and writable
// stream classes.  When you write(chunk), that calls _write(chunk,cb)
// internally, and returns false if there's a lot of pending writes
// buffered up.  When you call read(), that calls _read(n) until
// there's enough pending readable data buffered up.
//
// In a transform stream, the written data is placed in a buffer.  When
// _read(n) is called, it transforms the queued up data, calling the
// buffered _write cb's as it consumes chunks.  If consuming a single
// written chunk would result in multiple output chunks, then the first
// outputted bit calls the readcb, and subsequent chunks just go into
// the read buffer, and will cause it to emit 'readable' if necessary.
//
// This way, back-pressure is actually determined by the reading side,
// since _read has to be called to start processing a new chunk.  However,
// a pathological inflate type of transform can cause excessive buffering
// here.  For example, imagine a stream where every byte of input is
// interpreted as an integer from 0-255, and then results in that many
// bytes of output.  Writing the 4 bytes {ff,ff,ff,ff} would result in
// 1kb of data being output.  In this case, you could write a very small
// amount of input, and end up with a very large amount of output.  In
// such a pathological inflating mechanism, there'd be no way to tell
// the system to stop doing the transform.  A single 4MB write could
// cause the system to run out of memory.
//
// However, even in such a pathological case, only a single written chunk
// would be consumed, and then the rest would wait (un-transformed) until
// the results of the previous transformed chunk were consumed.

'use strict';

module.exports = Transform;

var Duplex = require('./_stream_duplex');

/*<replacement>*/
var util = require('core-util-is');
util.inherits = require('inherits');
/*</replacement>*/

util.inherits(Transform, Duplex);

function TransformState(stream) {
  this.afterTransform = function (er, data) {
    return afterTransform(stream, er, data);
  };

  this.needTransform = false;
  this.transforming = false;
  this.writecb = null;
  this.writechunk = null;
  this.writeencoding = null;
}

function afterTransform(stream, er, data) {
  var ts = stream._transformState;
  ts.transforming = false;

  var cb = ts.writecb;

  if (!cb) return stream.emit('error', new Error('no writecb in Transform class'));

  ts.writechunk = null;
  ts.writecb = null;

  if (data !== null && data !== undefined) stream.push(data);

  cb(er);

  var rs = stream._readableState;
  rs.reading = false;
  if (rs.needReadable || rs.length < rs.highWaterMark) {
    stream._read(rs.highWaterMark);
  }
}

function Transform(options) {
  if (!(this instanceof Transform)) return new Transform(options);

  Duplex.call(this, options);

  this._transformState = new TransformState(this);

  // when the writable side finishes, then flush out anything remaining.
  var stream = this;

  // start out asking for a readable event once data is transformed.
  this._readableState.needReadable = true;

  // we have implemented the _read method, and done the other things
  // that Readable wants before the first _read call, so unset the
  // sync guard flag.
  this._readableState.sync = false;

  if (options) {
    if (typeof options.transform === 'function') this._transform = options.transform;

    if (typeof options.flush === 'function') this._flush = options.flush;
  }

  this.once('prefinish', function () {
    if (typeof this._flush === 'function') this._flush(function (er) {
      done(stream, er);
    });else done(stream);
  });
}

Transform.prototype.push = function (chunk, encoding) {
  this._transformState.needTransform = false;
  return Duplex.prototype.push.call(this, chunk, encoding);
};

// This is the part where you do stuff!
// override this function in implementation classes.
// 'chunk' is an input chunk.
//
// Call `push(newChunk)` to pass along transformed output
// to the readable side.  You may call 'push' zero or more times.
//
// Call `cb(err)` when you are done with this chunk.  If you pass
// an error, then that'll put the hurt on the whole operation.  If you
// never call cb(), then you'll never get another chunk.
Transform.prototype._transform = function (chunk, encoding, cb) {
  throw new Error('not implemented');
};

Transform.prototype._write = function (chunk, encoding, cb) {
  var ts = this._transformState;
  ts.writecb = cb;
  ts.writechunk = chunk;
  ts.writeencoding = encoding;
  if (!ts.transforming) {
    var rs = this._readableState;
    if (ts.needTransform || rs.needReadable || rs.length < rs.highWaterMark) this._read(rs.highWaterMark);
  }
};

// Doesn't matter what the args are here.
// _transform does all the work.
// That we got here means that the readable side wants more data.
Transform.prototype._read = function (n) {
  var ts = this._transformState;

  if (ts.writechunk !== null && ts.writecb && !ts.transforming) {
    ts.transforming = true;
    this._transform(ts.writechunk, ts.writeencoding, ts.afterTransform);
  } else {
    // mark that we need a transform, so that any data that comes in
    // will get processed, now that we've asked for it.
    ts.needTransform = true;
  }
};

function done(stream, er) {
  if (er) return stream.emit('error', er);

  // if there's nothing in the write buffer, then that means
  // that nothing more will ever be provided
  var ws = stream._writableState;
  var ts = stream._transformState;

  if (ws.length) throw new Error('calling transform done when ws.length != 0');

  if (ts.transforming) throw new Error('calling transform done when still transforming');

  return stream.push(null);
}
},{"./_stream_duplex":18,"core-util-is":9,"inherits":12}],22:[function(require,module,exports){
(function (process){
// A bit simpler than readable streams.
// Implement an async ._write(chunk, encoding, cb), and it'll handle all
// the drain event emission and buffering.

'use strict';

module.exports = Writable;

/*<replacement>*/
var processNextTick = require('process-nextick-args');
/*</replacement>*/

/*<replacement>*/
var asyncWrite = !process.browser && ['v0.10', 'v0.9.'].indexOf(process.version.slice(0, 5)) > -1 ? setImmediate : processNextTick;
/*</replacement>*/

/*<replacement>*/
var Buffer = require('buffer').Buffer;
/*</replacement>*/

Writable.WritableState = WritableState;

/*<replacement>*/
var util = require('core-util-is');
util.inherits = require('inherits');
/*</replacement>*/

/*<replacement>*/
var internalUtil = {
  deprecate: require('util-deprecate')
};
/*</replacement>*/

/*<replacement>*/
var Stream;
(function () {
  try {
    Stream = require('st' + 'ream');
  } catch (_) {} finally {
    if (!Stream) Stream = require('events').EventEmitter;
  }
})();
/*</replacement>*/

var Buffer = require('buffer').Buffer;

util.inherits(Writable, Stream);

function nop() {}

function WriteReq(chunk, encoding, cb) {
  this.chunk = chunk;
  this.encoding = encoding;
  this.callback = cb;
  this.next = null;
}

var Duplex;
function WritableState(options, stream) {
  Duplex = Duplex || require('./_stream_duplex');

  options = options || {};

  // object stream flag to indicate whether or not this stream
  // contains buffers or objects.
  this.objectMode = !!options.objectMode;

  if (stream instanceof Duplex) this.objectMode = this.objectMode || !!options.writableObjectMode;

  // the point at which write() starts returning false
  // Note: 0 is a valid value, means that we always return false if
  // the entire buffer is not flushed immediately on write()
  var hwm = options.highWaterMark;
  var defaultHwm = this.objectMode ? 16 : 16 * 1024;
  this.highWaterMark = hwm || hwm === 0 ? hwm : defaultHwm;

  // cast to ints.
  this.highWaterMark = ~ ~this.highWaterMark;

  this.needDrain = false;
  // at the start of calling end()
  this.ending = false;
  // when end() has been called, and returned
  this.ended = false;
  // when 'finish' is emitted
  this.finished = false;

  // should we decode strings into buffers before passing to _write?
  // this is here so that some node-core streams can optimize string
  // handling at a lower level.
  var noDecode = options.decodeStrings === false;
  this.decodeStrings = !noDecode;

  // Crypto is kind of old and crusty.  Historically, its default string
  // encoding is 'binary' so we have to make this configurable.
  // Everything else in the universe uses 'utf8', though.
  this.defaultEncoding = options.defaultEncoding || 'utf8';

  // not an actual buffer we keep track of, but a measurement
  // of how much we're waiting to get pushed to some underlying
  // socket or file.
  this.length = 0;

  // a flag to see when we're in the middle of a write.
  this.writing = false;

  // when true all writes will be buffered until .uncork() call
  this.corked = 0;

  // a flag to be able to tell if the onwrite cb is called immediately,
  // or on a later tick.  We set this to true at first, because any
  // actions that shouldn't happen until "later" should generally also
  // not happen before the first write call.
  this.sync = true;

  // a flag to know if we're processing previously buffered items, which
  // may call the _write() callback in the same tick, so that we don't
  // end up in an overlapped onwrite situation.
  this.bufferProcessing = false;

  // the callback that's passed to _write(chunk,cb)
  this.onwrite = function (er) {
    onwrite(stream, er);
  };

  // the callback that the user supplies to write(chunk,encoding,cb)
  this.writecb = null;

  // the amount that is being written when _write is called.
  this.writelen = 0;

  this.bufferedRequest = null;
  this.lastBufferedRequest = null;

  // number of pending user-supplied write callbacks
  // this must be 0 before 'finish' can be emitted
  this.pendingcb = 0;

  // emit prefinish if the only thing we're waiting for is _write cbs
  // This is relevant for synchronous Transform streams
  this.prefinished = false;

  // True if the error was already emitted and should not be thrown again
  this.errorEmitted = false;

  // count buffered requests
  this.bufferedRequestCount = 0;

  // create the two objects needed to store the corked requests
  // they are not a linked list, as no new elements are inserted in there
  this.corkedRequestsFree = new CorkedRequest(this);
  this.corkedRequestsFree.next = new CorkedRequest(this);
}

WritableState.prototype.getBuffer = function writableStateGetBuffer() {
  var current = this.bufferedRequest;
  var out = [];
  while (current) {
    out.push(current);
    current = current.next;
  }
  return out;
};

(function () {
  try {
    Object.defineProperty(WritableState.prototype, 'buffer', {
      get: internalUtil.deprecate(function () {
        return this.getBuffer();
      }, '_writableState.buffer is deprecated. Use _writableState.getBuffer ' + 'instead.')
    });
  } catch (_) {}
})();

var Duplex;
function Writable(options) {
  Duplex = Duplex || require('./_stream_duplex');

  // Writable ctor is applied to Duplexes, though they're not
  // instanceof Writable, they're instanceof Readable.
  if (!(this instanceof Writable) && !(this instanceof Duplex)) return new Writable(options);

  this._writableState = new WritableState(options, this);

  // legacy.
  this.writable = true;

  if (options) {
    if (typeof options.write === 'function') this._write = options.write;

    if (typeof options.writev === 'function') this._writev = options.writev;
  }

  Stream.call(this);
}

// Otherwise people can pipe Writable streams, which is just wrong.
Writable.prototype.pipe = function () {
  this.emit('error', new Error('Cannot pipe. Not readable.'));
};

function writeAfterEnd(stream, cb) {
  var er = new Error('write after end');
  // TODO: defer error events consistently everywhere, not just the cb
  stream.emit('error', er);
  processNextTick(cb, er);
}

// If we get something that is not a buffer, string, null, or undefined,
// and we're not in objectMode, then that's an error.
// Otherwise stream chunks are all considered to be of length=1, and the
// watermarks determine how many objects to keep in the buffer, rather than
// how many bytes or characters.
function validChunk(stream, state, chunk, cb) {
  var valid = true;

  if (!Buffer.isBuffer(chunk) && typeof chunk !== 'string' && chunk !== null && chunk !== undefined && !state.objectMode) {
    var er = new TypeError('Invalid non-string/buffer chunk');
    stream.emit('error', er);
    processNextTick(cb, er);
    valid = false;
  }
  return valid;
}

Writable.prototype.write = function (chunk, encoding, cb) {
  var state = this._writableState;
  var ret = false;

  if (typeof encoding === 'function') {
    cb = encoding;
    encoding = null;
  }

  if (Buffer.isBuffer(chunk)) encoding = 'buffer';else if (!encoding) encoding = state.defaultEncoding;

  if (typeof cb !== 'function') cb = nop;

  if (state.ended) writeAfterEnd(this, cb);else if (validChunk(this, state, chunk, cb)) {
    state.pendingcb++;
    ret = writeOrBuffer(this, state, chunk, encoding, cb);
  }

  return ret;
};

Writable.prototype.cork = function () {
  var state = this._writableState;

  state.corked++;
};

Writable.prototype.uncork = function () {
  var state = this._writableState;

  if (state.corked) {
    state.corked--;

    if (!state.writing && !state.corked && !state.finished && !state.bufferProcessing && state.bufferedRequest) clearBuffer(this, state);
  }
};

Writable.prototype.setDefaultEncoding = function setDefaultEncoding(encoding) {
  // node::ParseEncoding() requires lower case.
  if (typeof encoding === 'string') encoding = encoding.toLowerCase();
  if (!(['hex', 'utf8', 'utf-8', 'ascii', 'binary', 'base64', 'ucs2', 'ucs-2', 'utf16le', 'utf-16le', 'raw'].indexOf((encoding + '').toLowerCase()) > -1)) throw new TypeError('Unknown encoding: ' + encoding);
  this._writableState.defaultEncoding = encoding;
};

function decodeChunk(state, chunk, encoding) {
  if (!state.objectMode && state.decodeStrings !== false && typeof chunk === 'string') {
    chunk = new Buffer(chunk, encoding);
  }
  return chunk;
}

// if we're already writing something, then just put this
// in the queue, and wait our turn.  Otherwise, call _write
// If we return false, then we need a drain event, so set that flag.
function writeOrBuffer(stream, state, chunk, encoding, cb) {
  chunk = decodeChunk(state, chunk, encoding);

  if (Buffer.isBuffer(chunk)) encoding = 'buffer';
  var len = state.objectMode ? 1 : chunk.length;

  state.length += len;

  var ret = state.length < state.highWaterMark;
  // we must ensure that previous needDrain will not be reset to false.
  if (!ret) state.needDrain = true;

  if (state.writing || state.corked) {
    var last = state.lastBufferedRequest;
    state.lastBufferedRequest = new WriteReq(chunk, encoding, cb);
    if (last) {
      last.next = state.lastBufferedRequest;
    } else {
      state.bufferedRequest = state.lastBufferedRequest;
    }
    state.bufferedRequestCount += 1;
  } else {
    doWrite(stream, state, false, len, chunk, encoding, cb);
  }

  return ret;
}

function doWrite(stream, state, writev, len, chunk, encoding, cb) {
  state.writelen = len;
  state.writecb = cb;
  state.writing = true;
  state.sync = true;
  if (writev) stream._writev(chunk, state.onwrite);else stream._write(chunk, encoding, state.onwrite);
  state.sync = false;
}

function onwriteError(stream, state, sync, er, cb) {
  --state.pendingcb;
  if (sync) processNextTick(cb, er);else cb(er);

  stream._writableState.errorEmitted = true;
  stream.emit('error', er);
}

function onwriteStateUpdate(state) {
  state.writing = false;
  state.writecb = null;
  state.length -= state.writelen;
  state.writelen = 0;
}

function onwrite(stream, er) {
  var state = stream._writableState;
  var sync = state.sync;
  var cb = state.writecb;

  onwriteStateUpdate(state);

  if (er) onwriteError(stream, state, sync, er, cb);else {
    // Check if we're actually ready to finish, but don't emit yet
    var finished = needFinish(state);

    if (!finished && !state.corked && !state.bufferProcessing && state.bufferedRequest) {
      clearBuffer(stream, state);
    }

    if (sync) {
      /*<replacement>*/
      asyncWrite(afterWrite, stream, state, finished, cb);
      /*</replacement>*/
    } else {
        afterWrite(stream, state, finished, cb);
      }
  }
}

function afterWrite(stream, state, finished, cb) {
  if (!finished) onwriteDrain(stream, state);
  state.pendingcb--;
  cb();
  finishMaybe(stream, state);
}

// Must force callback to be called on nextTick, so that we don't
// emit 'drain' before the write() consumer gets the 'false' return
// value, and has a chance to attach a 'drain' listener.
function onwriteDrain(stream, state) {
  if (state.length === 0 && state.needDrain) {
    state.needDrain = false;
    stream.emit('drain');
  }
}

// if there's something in the buffer waiting, then process it
function clearBuffer(stream, state) {
  state.bufferProcessing = true;
  var entry = state.bufferedRequest;

  if (stream._writev && entry && entry.next) {
    // Fast case, write everything using _writev()
    var l = state.bufferedRequestCount;
    var buffer = new Array(l);
    var holder = state.corkedRequestsFree;
    holder.entry = entry;

    var count = 0;
    while (entry) {
      buffer[count] = entry;
      entry = entry.next;
      count += 1;
    }

    doWrite(stream, state, true, state.length, buffer, '', holder.finish);

    // doWrite is always async, defer these to save a bit of time
    // as the hot path ends with doWrite
    state.pendingcb++;
    state.lastBufferedRequest = null;
    state.corkedRequestsFree = holder.next;
    holder.next = null;
  } else {
    // Slow case, write chunks one-by-one
    while (entry) {
      var chunk = entry.chunk;
      var encoding = entry.encoding;
      var cb = entry.callback;
      var len = state.objectMode ? 1 : chunk.length;

      doWrite(stream, state, false, len, chunk, encoding, cb);
      entry = entry.next;
      // if we didn't call the onwrite immediately, then
      // it means that we need to wait until it does.
      // also, that means that the chunk and cb are currently
      // being processed, so move the buffer counter past them.
      if (state.writing) {
        break;
      }
    }

    if (entry === null) state.lastBufferedRequest = null;
  }

  state.bufferedRequestCount = 0;
  state.bufferedRequest = entry;
  state.bufferProcessing = false;
}

Writable.prototype._write = function (chunk, encoding, cb) {
  cb(new Error('not implemented'));
};

Writable.prototype._writev = null;

Writable.prototype.end = function (chunk, encoding, cb) {
  var state = this._writableState;

  if (typeof chunk === 'function') {
    cb = chunk;
    chunk = null;
    encoding = null;
  } else if (typeof encoding === 'function') {
    cb = encoding;
    encoding = null;
  }

  if (chunk !== null && chunk !== undefined) this.write(chunk, encoding);

  // .end() fully uncorks
  if (state.corked) {
    state.corked = 1;
    this.uncork();
  }

  // ignore unnecessary end() calls.
  if (!state.ending && !state.finished) endWritable(this, state, cb);
};

function needFinish(state) {
  return state.ending && state.length === 0 && state.bufferedRequest === null && !state.finished && !state.writing;
}

function prefinish(stream, state) {
  if (!state.prefinished) {
    state.prefinished = true;
    stream.emit('prefinish');
  }
}

function finishMaybe(stream, state) {
  var need = needFinish(state);
  if (need) {
    if (state.pendingcb === 0) {
      prefinish(stream, state);
      state.finished = true;
      stream.emit('finish');
    } else {
      prefinish(stream, state);
    }
  }
  return need;
}

function endWritable(stream, state, cb) {
  state.ending = true;
  finishMaybe(stream, state);
  if (cb) {
    if (state.finished) processNextTick(cb);else stream.once('finish', cb);
  }
  state.ended = true;
  stream.writable = false;
}

// It seems a linked list but it is not
// there will be only 2 of these for each stream
function CorkedRequest(state) {
  var _this = this;

  this.next = null;
  this.entry = null;

  this.finish = function (err) {
    var entry = _this.entry;
    _this.entry = null;
    while (entry) {
      var cb = entry.callback;
      state.pendingcb--;
      cb(err);
      entry = entry.next;
    }
    if (state.corkedRequestsFree) {
      state.corkedRequestsFree.next = _this;
    } else {
      state.corkedRequestsFree = _this;
    }
  };
}
}).call(this,require('_process'))
},{"./_stream_duplex":18,"_process":17,"buffer":7,"core-util-is":9,"events":10,"inherits":12,"process-nextick-args":16,"util-deprecate":26}],23:[function(require,module,exports){
var Stream = (function (){
  try {
    return require('st' + 'ream'); // hack to fix a circular dependency issue when used with browserify
  } catch(_){}
}());
exports = module.exports = require('./lib/_stream_readable.js');
exports.Stream = Stream || exports;
exports.Readable = exports;
exports.Writable = require('./lib/_stream_writable.js');
exports.Duplex = require('./lib/_stream_duplex.js');
exports.Transform = require('./lib/_stream_transform.js');
exports.PassThrough = require('./lib/_stream_passthrough.js');

},{"./lib/_stream_duplex.js":18,"./lib/_stream_passthrough.js":19,"./lib/_stream_readable.js":20,"./lib/_stream_transform.js":21,"./lib/_stream_writable.js":22}],24:[function(require,module,exports){
// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

var Buffer = require('buffer').Buffer;

var isBufferEncoding = Buffer.isEncoding
  || function(encoding) {
       switch (encoding && encoding.toLowerCase()) {
         case 'hex': case 'utf8': case 'utf-8': case 'ascii': case 'binary': case 'base64': case 'ucs2': case 'ucs-2': case 'utf16le': case 'utf-16le': case 'raw': return true;
         default: return false;
       }
     }


function assertEncoding(encoding) {
  if (encoding && !isBufferEncoding(encoding)) {
    throw new Error('Unknown encoding: ' + encoding);
  }
}

// StringDecoder provides an interface for efficiently splitting a series of
// buffers into a series of JS strings without breaking apart multi-byte
// characters. CESU-8 is handled as part of the UTF-8 encoding.
//
// @TODO Handling all encodings inside a single object makes it very difficult
// to reason about this code, so it should be split up in the future.
// @TODO There should be a utf8-strict encoding that rejects invalid UTF-8 code
// points as used by CESU-8.
var StringDecoder = exports.StringDecoder = function(encoding) {
  this.encoding = (encoding || 'utf8').toLowerCase().replace(/[-_]/, '');
  assertEncoding(encoding);
  switch (this.encoding) {
    case 'utf8':
      // CESU-8 represents each of Surrogate Pair by 3-bytes
      this.surrogateSize = 3;
      break;
    case 'ucs2':
    case 'utf16le':
      // UTF-16 represents each of Surrogate Pair by 2-bytes
      this.surrogateSize = 2;
      this.detectIncompleteChar = utf16DetectIncompleteChar;
      break;
    case 'base64':
      // Base-64 stores 3 bytes in 4 chars, and pads the remainder.
      this.surrogateSize = 3;
      this.detectIncompleteChar = base64DetectIncompleteChar;
      break;
    default:
      this.write = passThroughWrite;
      return;
  }

  // Enough space to store all bytes of a single character. UTF-8 needs 4
  // bytes, but CESU-8 may require up to 6 (3 bytes per surrogate).
  this.charBuffer = new Buffer(6);
  // Number of bytes received for the current incomplete multi-byte character.
  this.charReceived = 0;
  // Number of bytes expected for the current incomplete multi-byte character.
  this.charLength = 0;
};


// write decodes the given buffer and returns it as JS string that is
// guaranteed to not contain any partial multi-byte characters. Any partial
// character found at the end of the buffer is buffered up, and will be
// returned when calling write again with the remaining bytes.
//
// Note: Converting a Buffer containing an orphan surrogate to a String
// currently works, but converting a String to a Buffer (via `new Buffer`, or
// Buffer#write) will replace incomplete surrogates with the unicode
// replacement character. See https://codereview.chromium.org/121173009/ .
StringDecoder.prototype.write = function(buffer) {
  var charStr = '';
  // if our last write ended with an incomplete multibyte character
  while (this.charLength) {
    // determine how many remaining bytes this buffer has to offer for this char
    var available = (buffer.length >= this.charLength - this.charReceived) ?
        this.charLength - this.charReceived :
        buffer.length;

    // add the new bytes to the char buffer
    buffer.copy(this.charBuffer, this.charReceived, 0, available);
    this.charReceived += available;

    if (this.charReceived < this.charLength) {
      // still not enough chars in this buffer? wait for more ...
      return '';
    }

    // remove bytes belonging to the current character from the buffer
    buffer = buffer.slice(available, buffer.length);

    // get the character that was split
    charStr = this.charBuffer.slice(0, this.charLength).toString(this.encoding);

    // CESU-8: lead surrogate (D800-DBFF) is also the incomplete character
    var charCode = charStr.charCodeAt(charStr.length - 1);
    if (charCode >= 0xD800 && charCode <= 0xDBFF) {
      this.charLength += this.surrogateSize;
      charStr = '';
      continue;
    }
    this.charReceived = this.charLength = 0;

    // if there are no more bytes in this buffer, just emit our char
    if (buffer.length === 0) {
      return charStr;
    }
    break;
  }

  // determine and set charLength / charReceived
  this.detectIncompleteChar(buffer);

  var end = buffer.length;
  if (this.charLength) {
    // buffer the incomplete character bytes we got
    buffer.copy(this.charBuffer, 0, buffer.length - this.charReceived, end);
    end -= this.charReceived;
  }

  charStr += buffer.toString(this.encoding, 0, end);

  var end = charStr.length - 1;
  var charCode = charStr.charCodeAt(end);
  // CESU-8: lead surrogate (D800-DBFF) is also the incomplete character
  if (charCode >= 0xD800 && charCode <= 0xDBFF) {
    var size = this.surrogateSize;
    this.charLength += size;
    this.charReceived += size;
    this.charBuffer.copy(this.charBuffer, size, 0, size);
    buffer.copy(this.charBuffer, 0, 0, size);
    return charStr.substring(0, end);
  }

  // or just emit the charStr
  return charStr;
};

// detectIncompleteChar determines if there is an incomplete UTF-8 character at
// the end of the given buffer. If so, it sets this.charLength to the byte
// length that character, and sets this.charReceived to the number of bytes
// that are available for this character.
StringDecoder.prototype.detectIncompleteChar = function(buffer) {
  // determine how many bytes we have to check at the end of this buffer
  var i = (buffer.length >= 3) ? 3 : buffer.length;

  // Figure out if one of the last i bytes of our buffer announces an
  // incomplete char.
  for (; i > 0; i--) {
    var c = buffer[buffer.length - i];

    // See http://en.wikipedia.org/wiki/UTF-8#Description

    // 110XXXXX
    if (i == 1 && c >> 5 == 0x06) {
      this.charLength = 2;
      break;
    }

    // 1110XXXX
    if (i <= 2 && c >> 4 == 0x0E) {
      this.charLength = 3;
      break;
    }

    // 11110XXX
    if (i <= 3 && c >> 3 == 0x1E) {
      this.charLength = 4;
      break;
    }
  }
  this.charReceived = i;
};

StringDecoder.prototype.end = function(buffer) {
  var res = '';
  if (buffer && buffer.length)
    res = this.write(buffer);

  if (this.charReceived) {
    var cr = this.charReceived;
    var buf = this.charBuffer;
    var enc = this.encoding;
    res += buf.slice(0, cr).toString(enc);
  }

  return res;
};

function passThroughWrite(buffer) {
  return buffer.toString(this.encoding);
}

function utf16DetectIncompleteChar(buffer) {
  this.charReceived = buffer.length % 2;
  this.charLength = this.charReceived ? 2 : 0;
}

function base64DetectIncompleteChar(buffer) {
  this.charReceived = buffer.length % 3;
  this.charLength = this.charReceived ? 3 : 0;
}

},{"buffer":7}],25:[function(require,module,exports){
var undefined = (void 0); // Paranoia

// Beyond this value, index getters/setters (i.e. array[0], array[1]) are so slow to
// create, and consume so much memory, that the browser appears frozen.
var MAX_ARRAY_LENGTH = 1e5;

// Approximations of internal ECMAScript conversion functions
var ECMAScript = (function() {
  // Stash a copy in case other scripts modify these
  var opts = Object.prototype.toString,
      ophop = Object.prototype.hasOwnProperty;

  return {
    // Class returns internal [[Class]] property, used to avoid cross-frame instanceof issues:
    Class: function(v) { return opts.call(v).replace(/^\[object *|\]$/g, ''); },
    HasProperty: function(o, p) { return p in o; },
    HasOwnProperty: function(o, p) { return ophop.call(o, p); },
    IsCallable: function(o) { return typeof o === 'function'; },
    ToInt32: function(v) { return v >> 0; },
    ToUint32: function(v) { return v >>> 0; }
  };
}());

// Snapshot intrinsics
var LN2 = Math.LN2,
    abs = Math.abs,
    floor = Math.floor,
    log = Math.log,
    min = Math.min,
    pow = Math.pow,
    round = Math.round;

// ES5: lock down object properties
function configureProperties(obj) {
  if (getOwnPropNames && defineProp) {
    var props = getOwnPropNames(obj), i;
    for (i = 0; i < props.length; i += 1) {
      defineProp(obj, props[i], {
        value: obj[props[i]],
        writable: false,
        enumerable: false,
        configurable: false
      });
    }
  }
}

// emulate ES5 getter/setter API using legacy APIs
// http://blogs.msdn.com/b/ie/archive/2010/09/07/transitioning-existing-code-to-the-es5-getter-setter-apis.aspx
// (second clause tests for Object.defineProperty() in IE<9 that only supports extending DOM prototypes, but
// note that IE<9 does not support __defineGetter__ or __defineSetter__ so it just renders the method harmless)
var defineProp
if (Object.defineProperty && (function() {
      try {
        Object.defineProperty({}, 'x', {});
        return true;
      } catch (e) {
        return false;
      }
    })()) {
  defineProp = Object.defineProperty;
} else {
  defineProp = function(o, p, desc) {
    if (!o === Object(o)) throw new TypeError("Object.defineProperty called on non-object");
    if (ECMAScript.HasProperty(desc, 'get') && Object.prototype.__defineGetter__) { Object.prototype.__defineGetter__.call(o, p, desc.get); }
    if (ECMAScript.HasProperty(desc, 'set') && Object.prototype.__defineSetter__) { Object.prototype.__defineSetter__.call(o, p, desc.set); }
    if (ECMAScript.HasProperty(desc, 'value')) { o[p] = desc.value; }
    return o;
  };
}

var getOwnPropNames = Object.getOwnPropertyNames || function (o) {
  if (o !== Object(o)) throw new TypeError("Object.getOwnPropertyNames called on non-object");
  var props = [], p;
  for (p in o) {
    if (ECMAScript.HasOwnProperty(o, p)) {
      props.push(p);
    }
  }
  return props;
};

// ES5: Make obj[index] an alias for obj._getter(index)/obj._setter(index, value)
// for index in 0 ... obj.length
function makeArrayAccessors(obj) {
  if (!defineProp) { return; }

  if (obj.length > MAX_ARRAY_LENGTH) throw new RangeError("Array too large for polyfill");

  function makeArrayAccessor(index) {
    defineProp(obj, index, {
      'get': function() { return obj._getter(index); },
      'set': function(v) { obj._setter(index, v); },
      enumerable: true,
      configurable: false
    });
  }

  var i;
  for (i = 0; i < obj.length; i += 1) {
    makeArrayAccessor(i);
  }
}

// Internal conversion functions:
//    pack<Type>()   - take a number (interpreted as Type), output a byte array
//    unpack<Type>() - take a byte array, output a Type-like number

function as_signed(value, bits) { var s = 32 - bits; return (value << s) >> s; }
function as_unsigned(value, bits) { var s = 32 - bits; return (value << s) >>> s; }

function packI8(n) { return [n & 0xff]; }
function unpackI8(bytes) { return as_signed(bytes[0], 8); }

function packU8(n) { return [n & 0xff]; }
function unpackU8(bytes) { return as_unsigned(bytes[0], 8); }

function packU8Clamped(n) { n = round(Number(n)); return [n < 0 ? 0 : n > 0xff ? 0xff : n & 0xff]; }

function packI16(n) { return [(n >> 8) & 0xff, n & 0xff]; }
function unpackI16(bytes) { return as_signed(bytes[0] << 8 | bytes[1], 16); }

function packU16(n) { return [(n >> 8) & 0xff, n & 0xff]; }
function unpackU16(bytes) { return as_unsigned(bytes[0] << 8 | bytes[1], 16); }

function packI32(n) { return [(n >> 24) & 0xff, (n >> 16) & 0xff, (n >> 8) & 0xff, n & 0xff]; }
function unpackI32(bytes) { return as_signed(bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3], 32); }

function packU32(n) { return [(n >> 24) & 0xff, (n >> 16) & 0xff, (n >> 8) & 0xff, n & 0xff]; }
function unpackU32(bytes) { return as_unsigned(bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3], 32); }

function packIEEE754(v, ebits, fbits) {

  var bias = (1 << (ebits - 1)) - 1,
      s, e, f, ln,
      i, bits, str, bytes;

  function roundToEven(n) {
    var w = floor(n), f = n - w;
    if (f < 0.5)
      return w;
    if (f > 0.5)
      return w + 1;
    return w % 2 ? w + 1 : w;
  }

  // Compute sign, exponent, fraction
  if (v !== v) {
    // NaN
    // http://dev.w3.org/2006/webapi/WebIDL/#es-type-mapping
    e = (1 << ebits) - 1; f = pow(2, fbits - 1); s = 0;
  } else if (v === Infinity || v === -Infinity) {
    e = (1 << ebits) - 1; f = 0; s = (v < 0) ? 1 : 0;
  } else if (v === 0) {
    e = 0; f = 0; s = (1 / v === -Infinity) ? 1 : 0;
  } else {
    s = v < 0;
    v = abs(v);

    if (v >= pow(2, 1 - bias)) {
      e = min(floor(log(v) / LN2), 1023);
      f = roundToEven(v / pow(2, e) * pow(2, fbits));
      if (f / pow(2, fbits) >= 2) {
        e = e + 1;
        f = 1;
      }
      if (e > bias) {
        // Overflow
        e = (1 << ebits) - 1;
        f = 0;
      } else {
        // Normalized
        e = e + bias;
        f = f - pow(2, fbits);
      }
    } else {
      // Denormalized
      e = 0;
      f = roundToEven(v / pow(2, 1 - bias - fbits));
    }
  }

  // Pack sign, exponent, fraction
  bits = [];
  for (i = fbits; i; i -= 1) { bits.push(f % 2 ? 1 : 0); f = floor(f / 2); }
  for (i = ebits; i; i -= 1) { bits.push(e % 2 ? 1 : 0); e = floor(e / 2); }
  bits.push(s ? 1 : 0);
  bits.reverse();
  str = bits.join('');

  // Bits to bytes
  bytes = [];
  while (str.length) {
    bytes.push(parseInt(str.substring(0, 8), 2));
    str = str.substring(8);
  }
  return bytes;
}

function unpackIEEE754(bytes, ebits, fbits) {

  // Bytes to bits
  var bits = [], i, j, b, str,
      bias, s, e, f;

  for (i = bytes.length; i; i -= 1) {
    b = bytes[i - 1];
    for (j = 8; j; j -= 1) {
      bits.push(b % 2 ? 1 : 0); b = b >> 1;
    }
  }
  bits.reverse();
  str = bits.join('');

  // Unpack sign, exponent, fraction
  bias = (1 << (ebits - 1)) - 1;
  s = parseInt(str.substring(0, 1), 2) ? -1 : 1;
  e = parseInt(str.substring(1, 1 + ebits), 2);
  f = parseInt(str.substring(1 + ebits), 2);

  // Produce number
  if (e === (1 << ebits) - 1) {
    return f !== 0 ? NaN : s * Infinity;
  } else if (e > 0) {
    // Normalized
    return s * pow(2, e - bias) * (1 + f / pow(2, fbits));
  } else if (f !== 0) {
    // Denormalized
    return s * pow(2, -(bias - 1)) * (f / pow(2, fbits));
  } else {
    return s < 0 ? -0 : 0;
  }
}

function unpackF64(b) { return unpackIEEE754(b, 11, 52); }
function packF64(v) { return packIEEE754(v, 11, 52); }
function unpackF32(b) { return unpackIEEE754(b, 8, 23); }
function packF32(v) { return packIEEE754(v, 8, 23); }


//
// 3 The ArrayBuffer Type
//

(function() {

  /** @constructor */
  var ArrayBuffer = function ArrayBuffer(length) {
    length = ECMAScript.ToInt32(length);
    if (length < 0) throw new RangeError('ArrayBuffer size is not a small enough positive integer');

    this.byteLength = length;
    this._bytes = [];
    this._bytes.length = length;

    var i;
    for (i = 0; i < this.byteLength; i += 1) {
      this._bytes[i] = 0;
    }

    configureProperties(this);
  };

  exports.ArrayBuffer = exports.ArrayBuffer || ArrayBuffer;

  //
  // 4 The ArrayBufferView Type
  //

  // NOTE: this constructor is not exported
  /** @constructor */
  var ArrayBufferView = function ArrayBufferView() {
    //this.buffer = null;
    //this.byteOffset = 0;
    //this.byteLength = 0;
  };

  //
  // 5 The Typed Array View Types
  //

  function makeConstructor(bytesPerElement, pack, unpack) {
    // Each TypedArray type requires a distinct constructor instance with
    // identical logic, which this produces.

    var ctor;
    ctor = function(buffer, byteOffset, length) {
      var array, sequence, i, s;

      if (!arguments.length || typeof arguments[0] === 'number') {
        // Constructor(unsigned long length)
        this.length = ECMAScript.ToInt32(arguments[0]);
        if (length < 0) throw new RangeError('ArrayBufferView size is not a small enough positive integer');

        this.byteLength = this.length * this.BYTES_PER_ELEMENT;
        this.buffer = new ArrayBuffer(this.byteLength);
        this.byteOffset = 0;
      } else if (typeof arguments[0] === 'object' && arguments[0].constructor === ctor) {
        // Constructor(TypedArray array)
        array = arguments[0];

        this.length = array.length;
        this.byteLength = this.length * this.BYTES_PER_ELEMENT;
        this.buffer = new ArrayBuffer(this.byteLength);
        this.byteOffset = 0;

        for (i = 0; i < this.length; i += 1) {
          this._setter(i, array._getter(i));
        }
      } else if (typeof arguments[0] === 'object' &&
                 !(arguments[0] instanceof ArrayBuffer || ECMAScript.Class(arguments[0]) === 'ArrayBuffer')) {
        // Constructor(sequence<type> array)
        sequence = arguments[0];

        this.length = ECMAScript.ToUint32(sequence.length);
        this.byteLength = this.length * this.BYTES_PER_ELEMENT;
        this.buffer = new ArrayBuffer(this.byteLength);
        this.byteOffset = 0;

        for (i = 0; i < this.length; i += 1) {
          s = sequence[i];
          this._setter(i, Number(s));
        }
      } else if (typeof arguments[0] === 'object' &&
                 (arguments[0] instanceof ArrayBuffer || ECMAScript.Class(arguments[0]) === 'ArrayBuffer')) {
        // Constructor(ArrayBuffer buffer,
        //             optional unsigned long byteOffset, optional unsigned long length)
        this.buffer = buffer;

        this.byteOffset = ECMAScript.ToUint32(byteOffset);
        if (this.byteOffset > this.buffer.byteLength) {
          throw new RangeError("byteOffset out of range");
        }

        if (this.byteOffset % this.BYTES_PER_ELEMENT) {
          // The given byteOffset must be a multiple of the element
          // size of the specific type, otherwise an exception is raised.
          throw new RangeError("ArrayBuffer length minus the byteOffset is not a multiple of the element size.");
        }

        if (arguments.length < 3) {
          this.byteLength = this.buffer.byteLength - this.byteOffset;

          if (this.byteLength % this.BYTES_PER_ELEMENT) {
            throw new RangeError("length of buffer minus byteOffset not a multiple of the element size");
          }
          this.length = this.byteLength / this.BYTES_PER_ELEMENT;
        } else {
          this.length = ECMAScript.ToUint32(length);
          this.byteLength = this.length * this.BYTES_PER_ELEMENT;
        }

        if ((this.byteOffset + this.byteLength) > this.buffer.byteLength) {
          throw new RangeError("byteOffset and length reference an area beyond the end of the buffer");
        }
      } else {
        throw new TypeError("Unexpected argument type(s)");
      }

      this.constructor = ctor;

      configureProperties(this);
      makeArrayAccessors(this);
    };

    ctor.prototype = new ArrayBufferView();
    ctor.prototype.BYTES_PER_ELEMENT = bytesPerElement;
    ctor.prototype._pack = pack;
    ctor.prototype._unpack = unpack;
    ctor.BYTES_PER_ELEMENT = bytesPerElement;

    // getter type (unsigned long index);
    ctor.prototype._getter = function(index) {
      if (arguments.length < 1) throw new SyntaxError("Not enough arguments");

      index = ECMAScript.ToUint32(index);
      if (index >= this.length) {
        return undefined;
      }

      var bytes = [], i, o;
      for (i = 0, o = this.byteOffset + index * this.BYTES_PER_ELEMENT;
           i < this.BYTES_PER_ELEMENT;
           i += 1, o += 1) {
        bytes.push(this.buffer._bytes[o]);
      }
      return this._unpack(bytes);
    };

    // NONSTANDARD: convenience alias for getter: type get(unsigned long index);
    ctor.prototype.get = ctor.prototype._getter;

    // setter void (unsigned long index, type value);
    ctor.prototype._setter = function(index, value) {
      if (arguments.length < 2) throw new SyntaxError("Not enough arguments");

      index = ECMAScript.ToUint32(index);
      if (index >= this.length) {
        return undefined;
      }

      var bytes = this._pack(value), i, o;
      for (i = 0, o = this.byteOffset + index * this.BYTES_PER_ELEMENT;
           i < this.BYTES_PER_ELEMENT;
           i += 1, o += 1) {
        this.buffer._bytes[o] = bytes[i];
      }
    };

    // void set(TypedArray array, optional unsigned long offset);
    // void set(sequence<type> array, optional unsigned long offset);
    ctor.prototype.set = function(index, value) {
      if (arguments.length < 1) throw new SyntaxError("Not enough arguments");
      var array, sequence, offset, len,
          i, s, d,
          byteOffset, byteLength, tmp;

      if (typeof arguments[0] === 'object' && arguments[0].constructor === this.constructor) {
        // void set(TypedArray array, optional unsigned long offset);
        array = arguments[0];
        offset = ECMAScript.ToUint32(arguments[1]);

        if (offset + array.length > this.length) {
          throw new RangeError("Offset plus length of array is out of range");
        }

        byteOffset = this.byteOffset + offset * this.BYTES_PER_ELEMENT;
        byteLength = array.length * this.BYTES_PER_ELEMENT;

        if (array.buffer === this.buffer) {
          tmp = [];
          for (i = 0, s = array.byteOffset; i < byteLength; i += 1, s += 1) {
            tmp[i] = array.buffer._bytes[s];
          }
          for (i = 0, d = byteOffset; i < byteLength; i += 1, d += 1) {
            this.buffer._bytes[d] = tmp[i];
          }
        } else {
          for (i = 0, s = array.byteOffset, d = byteOffset;
               i < byteLength; i += 1, s += 1, d += 1) {
            this.buffer._bytes[d] = array.buffer._bytes[s];
          }
        }
      } else if (typeof arguments[0] === 'object' && typeof arguments[0].length !== 'undefined') {
        // void set(sequence<type> array, optional unsigned long offset);
        sequence = arguments[0];
        len = ECMAScript.ToUint32(sequence.length);
        offset = ECMAScript.ToUint32(arguments[1]);

        if (offset + len > this.length) {
          throw new RangeError("Offset plus length of array is out of range");
        }

        for (i = 0; i < len; i += 1) {
          s = sequence[i];
          this._setter(offset + i, Number(s));
        }
      } else {
        throw new TypeError("Unexpected argument type(s)");
      }
    };

    // TypedArray subarray(long begin, optional long end);
    ctor.prototype.subarray = function(start, end) {
      function clamp(v, min, max) { return v < min ? min : v > max ? max : v; }

      start = ECMAScript.ToInt32(start);
      end = ECMAScript.ToInt32(end);

      if (arguments.length < 1) { start = 0; }
      if (arguments.length < 2) { end = this.length; }

      if (start < 0) { start = this.length + start; }
      if (end < 0) { end = this.length + end; }

      start = clamp(start, 0, this.length);
      end = clamp(end, 0, this.length);

      var len = end - start;
      if (len < 0) {
        len = 0;
      }

      return new this.constructor(
        this.buffer, this.byteOffset + start * this.BYTES_PER_ELEMENT, len);
    };

    return ctor;
  }

  var Int8Array = makeConstructor(1, packI8, unpackI8);
  var Uint8Array = makeConstructor(1, packU8, unpackU8);
  var Uint8ClampedArray = makeConstructor(1, packU8Clamped, unpackU8);
  var Int16Array = makeConstructor(2, packI16, unpackI16);
  var Uint16Array = makeConstructor(2, packU16, unpackU16);
  var Int32Array = makeConstructor(4, packI32, unpackI32);
  var Uint32Array = makeConstructor(4, packU32, unpackU32);
  var Float32Array = makeConstructor(4, packF32, unpackF32);
  var Float64Array = makeConstructor(8, packF64, unpackF64);

  exports.Int8Array = exports.Int8Array || Int8Array;
  exports.Uint8Array = exports.Uint8Array || Uint8Array;
  exports.Uint8ClampedArray = exports.Uint8ClampedArray || Uint8ClampedArray;
  exports.Int16Array = exports.Int16Array || Int16Array;
  exports.Uint16Array = exports.Uint16Array || Uint16Array;
  exports.Int32Array = exports.Int32Array || Int32Array;
  exports.Uint32Array = exports.Uint32Array || Uint32Array;
  exports.Float32Array = exports.Float32Array || Float32Array;
  exports.Float64Array = exports.Float64Array || Float64Array;
}());

//
// 6 The DataView View Type
//

(function() {
  function r(array, index) {
    return ECMAScript.IsCallable(array.get) ? array.get(index) : array[index];
  }

  var IS_BIG_ENDIAN = (function() {
    var u16array = new(exports.Uint16Array)([0x1234]),
        u8array = new(exports.Uint8Array)(u16array.buffer);
    return r(u8array, 0) === 0x12;
  }());

  // Constructor(ArrayBuffer buffer,
  //             optional unsigned long byteOffset,
  //             optional unsigned long byteLength)
  /** @constructor */
  var DataView = function DataView(buffer, byteOffset, byteLength) {
    if (arguments.length === 0) {
      buffer = new exports.ArrayBuffer(0);
    } else if (!(buffer instanceof exports.ArrayBuffer || ECMAScript.Class(buffer) === 'ArrayBuffer')) {
      throw new TypeError("TypeError");
    }

    this.buffer = buffer || new exports.ArrayBuffer(0);

    this.byteOffset = ECMAScript.ToUint32(byteOffset);
    if (this.byteOffset > this.buffer.byteLength) {
      throw new RangeError("byteOffset out of range");
    }

    if (arguments.length < 3) {
      this.byteLength = this.buffer.byteLength - this.byteOffset;
    } else {
      this.byteLength = ECMAScript.ToUint32(byteLength);
    }

    if ((this.byteOffset + this.byteLength) > this.buffer.byteLength) {
      throw new RangeError("byteOffset and length reference an area beyond the end of the buffer");
    }

    configureProperties(this);
  };

  function makeGetter(arrayType) {
    return function(byteOffset, littleEndian) {

      byteOffset = ECMAScript.ToUint32(byteOffset);

      if (byteOffset + arrayType.BYTES_PER_ELEMENT > this.byteLength) {
        throw new RangeError("Array index out of range");
      }
      byteOffset += this.byteOffset;

      var uint8Array = new exports.Uint8Array(this.buffer, byteOffset, arrayType.BYTES_PER_ELEMENT),
          bytes = [], i;
      for (i = 0; i < arrayType.BYTES_PER_ELEMENT; i += 1) {
        bytes.push(r(uint8Array, i));
      }

      if (Boolean(littleEndian) === Boolean(IS_BIG_ENDIAN)) {
        bytes.reverse();
      }

      return r(new arrayType(new exports.Uint8Array(bytes).buffer), 0);
    };
  }

  DataView.prototype.getUint8 = makeGetter(exports.Uint8Array);
  DataView.prototype.getInt8 = makeGetter(exports.Int8Array);
  DataView.prototype.getUint16 = makeGetter(exports.Uint16Array);
  DataView.prototype.getInt16 = makeGetter(exports.Int16Array);
  DataView.prototype.getUint32 = makeGetter(exports.Uint32Array);
  DataView.prototype.getInt32 = makeGetter(exports.Int32Array);
  DataView.prototype.getFloat32 = makeGetter(exports.Float32Array);
  DataView.prototype.getFloat64 = makeGetter(exports.Float64Array);

  function makeSetter(arrayType) {
    return function(byteOffset, value, littleEndian) {

      byteOffset = ECMAScript.ToUint32(byteOffset);
      if (byteOffset + arrayType.BYTES_PER_ELEMENT > this.byteLength) {
        throw new RangeError("Array index out of range");
      }

      // Get bytes
      var typeArray = new arrayType([value]),
          byteArray = new exports.Uint8Array(typeArray.buffer),
          bytes = [], i, byteView;

      for (i = 0; i < arrayType.BYTES_PER_ELEMENT; i += 1) {
        bytes.push(r(byteArray, i));
      }

      // Flip if necessary
      if (Boolean(littleEndian) === Boolean(IS_BIG_ENDIAN)) {
        bytes.reverse();
      }

      // Write them
      byteView = new exports.Uint8Array(this.buffer, byteOffset, arrayType.BYTES_PER_ELEMENT);
      byteView.set(bytes);
    };
  }

  DataView.prototype.setUint8 = makeSetter(exports.Uint8Array);
  DataView.prototype.setInt8 = makeSetter(exports.Int8Array);
  DataView.prototype.setUint16 = makeSetter(exports.Uint16Array);
  DataView.prototype.setInt16 = makeSetter(exports.Int16Array);
  DataView.prototype.setUint32 = makeSetter(exports.Uint32Array);
  DataView.prototype.setInt32 = makeSetter(exports.Int32Array);
  DataView.prototype.setFloat32 = makeSetter(exports.Float32Array);
  DataView.prototype.setFloat64 = makeSetter(exports.Float64Array);

  exports.DataView = exports.DataView || DataView;

}());

},{}],26:[function(require,module,exports){
(function (global){

/**
 * Module exports.
 */

module.exports = deprecate;

/**
 * Mark that a method should not be used.
 * Returns a modified function which warns once by default.
 *
 * If `localStorage.noDeprecation = true` is set, then it is a no-op.
 *
 * If `localStorage.throwDeprecation = true` is set, then deprecated functions
 * will throw an Error when invoked.
 *
 * If `localStorage.traceDeprecation = true` is set, then deprecated functions
 * will invoke `console.trace()` instead of `console.error()`.
 *
 * @param {Function} fn - the function to deprecate
 * @param {String} msg - the string to print to the console when `fn` is invoked
 * @returns {Function} a new "deprecated" version of `fn`
 * @api public
 */

function deprecate (fn, msg) {
  if (config('noDeprecation')) {
    return fn;
  }

  var warned = false;
  function deprecated() {
    if (!warned) {
      if (config('throwDeprecation')) {
        throw new Error(msg);
      } else if (config('traceDeprecation')) {
        console.trace(msg);
      } else {
        console.warn(msg);
      }
      warned = true;
    }
    return fn.apply(this, arguments);
  }

  return deprecated;
}

/**
 * Checks `localStorage` for boolean values for the given `name`.
 *
 * @param {String} name
 * @returns {Boolean}
 * @api private
 */

function config (name) {
  // accessing global.localStorage can trigger a DOMException in sandboxed iframes
  try {
    if (!global.localStorage) return false;
  } catch (_) {
    return false;
  }
  var val = global.localStorage[name];
  if (null == val) return false;
  return String(val).toLowerCase() === 'true';
}

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{}]},{},[1])(1)
});
