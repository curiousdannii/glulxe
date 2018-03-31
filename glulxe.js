/*

Emglken port of Glulxe
======================

Copyright (c) 2018 Dannii Willis
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

	// Code copied from GlkApi's glk_stream_open_memory and gli_new_stream
	create_fake_stream( buf, writeable )
	{
		var str = {}
		str.type = 3
		str.rock = 0
		str.disprock = undefined

		str.unicode = false
		/* isbinary is only meaningful for Resource and streaming-File streams */
		//str.isbinary = false
		str.streaming = false
		//str.ref = null
		//str.win = null
		//str.file = null

		/* for buffer mode */
		//str.buf = null
		//str.bufpos = 0
		//str.buflen = 0
		//str.bufeof = 0
		str.timer_id = null
		str.flush_func = null

		/* for streaming mode */
		str.fstream = null

		str.readcount = 0
		str.writecount = 0
		str.readable = !writeable
		str.writable = writeable

		/*str.prev = null
		str.next = gli_streamlist
		gli_streamlist = str
		if (str.next)
			str.next.prev = str*/

		if ( this.options.GiDispa )
		{
			this.options.GiDispa.class_register( 'stream', str )
		}

		if ( buf )
		{
			str.buf = buf
			str.buflen = buf.length
			str.bufpos = 0
			str.bufeof = writeable ? 0 : str.buflen
			if ( this.options.GiDispa )
			{
				this.options.GiDispa.retain_array( buf )
			}
		}

		return str
	}

    do_autosave( save )
    {
        if ( !this.options.Dialog )
        {
            throw new Error( 'A reference to Dialog is required' )
        }

        let snapshot = null
        if ( ( save || 0 ) >= 0 )
        {
            const Glk = this.options.Glk
            const stream_results = new Glk.RefStruct()

            // Set up the streams to pass to the VM
            const ram_buffer = new Uint8Array( this.options.autosavelen )
            const ram_stream = Glk.glk_stream_open_memory( ram_buffer, 1, 0 )
            const misc_buffer = new Uint32Array( 256 )
            const misc_stream = Glk.glk_stream_open_memory_uni( misc_buffer, 1, 0 )

            // Call into the VM
            this.vm['_emautosave']( ram_stream.disprock, misc_stream.disprock )

            // Retrieve the RAM/savefile
            Glk.glk_stream_close( ram_stream, stream_results )
            const ram_len = stream_results.get_field( 1 )
            if ( !ram_len )
            {
                return
            }

            // Retrieve the misc autosave data
            Glk.glk_stream_close( misc_stream, stream_results )

            // Finally save the Glk state
            snapshot = {
                glk: Glk.save_allstate(),
                misc: Array.from( misc_buffer.slice( 0, stream_results.get_field( 1 ) ) ),
                ram: ram_buffer.slice( 0, ram_len ),
            }

            // For now manually filter out the gamefile stream
            //snapshot.glk.streams = snapshot.glk.streams.filter( str => !str.buf || str.buf.len < 2048 )
        }

        this.options.Dialog.autosave_write( this.signature, snapshot )
    }

	start()
	{
		//const data_stream = this.options.Glk.glk_stream_open_memory( this.data, 2, 0 )
		const data_stream = this.create_fake_stream( this.data, 0 )
		this.vm['_emglulxeen']( data_stream.disprock, 0, 0 )
		delete this.data
	}

}

module.exports = Glulxe