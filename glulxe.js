/*

Emglken port of Glulxe
======================

Copyright (c) 2017 Dannii Willis
MIT licenced
https://github.com/curiousdannii/emglken

*/

const EmglkenVM = require( '../emglken/emglken_vm.js' )

class Glulxe extends EmglkenVM
{

	default_options()
	{
		return {
			dirname: __dirname,
			emptfile: 'glulxe-core.js.bin',
			module: require( './glulxe-core.js' ),
		}
	}
	
	start()
	{
		this.vm.ccall(
			'emglulxeen',
			null,
			[ 'array', 'number' ],
			[ this.data, this.data.length ],
			{ async: true }
		)
		delete this.data
	}

}

module.exports = Glulxe