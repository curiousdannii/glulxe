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

			profile_stream: 0,
			profcalls: 0,
		}
	}
	
	async start()
	{
		const data_stream = await this.options.Glk.glk_stream_open_memory( this.data, 2, 0 )
		this.vm['_emglulxeen']( data_stream.disprock, 0, 0 )
		delete this.data
	}

}

module.exports = Glulxe