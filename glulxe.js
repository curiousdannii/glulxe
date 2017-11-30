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
	
	start()
	{
		const data_stream = this.options.Glk.glk_stream_open_memory( this.data, 2, 0 )
		const profile_stream_tag = this.options.profile_stream ? this.options.profile_stream.disprock : 0
		this.vm['_emglulxeen']( data_stream.disprock, profile_stream_tag, this.options.profcalls )
		delete this.data
	}

}

module.exports = Glulxe