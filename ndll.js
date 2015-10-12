var process = require('process');
if (process.platform == "android")
	process.dlopen(module, './libndll.so');
else
	module.exports = require('bindings')('ndll');