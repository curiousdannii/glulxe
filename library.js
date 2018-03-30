// Glulxe library functions

var glulxe = {

    emautorestore: function( ramStreamTag, iosysmode, iosysrock, protectend, protectstart, stringtable )
    {
        var vm = Module.vm
        var Dialog = vm.options.Dialog
        if ( Dialog )
        {
            if ( vm.options.clear_vm_autosave )
            {
                Dialog.autosave_write( vm.signature, null )
            }
            else if ( vm.options.do_vm_autosave )
            {
                try
                {
                    var snapshot = Dialog.autosave_read( vm.signature )
                    if ( snapshot )
                    {
                        // Restore the Glk state
                        Glk.restore_allstate( snapshot.glk )

                        // Write out the misc properties
                        {{{ makeSetValue( 'iosysmode', '0', 'snapshot.iosysmode', 'i32' ) }}}
                        {{{ makeSetValue( 'iosysrock', '0', 'snapshot.iosysrock', 'i32' ) }}}
                        {{{ makeSetValue( 'protectend', '0', 'snapshot.protectend', 'i32' ) }}}
                        {{{ makeSetValue( 'protectstart', '0', 'snapshot.protectstart', 'i32' ) }}}
                        {{{ makeSetValue( 'stringtable', '0', 'snapshot.stringtable', 'i32' ) }}}

                        // Create the ram stream
                        var ramstr = Glk.glk_stream_open_memory( new Uint8Array( snapshot.ram ), 2, 0 )
                        {{{ makeSetValue( 'ramStreamTag', '0', '_stream_to_id( ramstr )', 'i32' ) }}}
                    }
                }
                catch (ex)
                {
                    //this.log( 'Autorestore failed, deleting it' )
                    Dialog.autosave_write( vm.signature, null )
                }
            }
        }
    },

}

mergeInto( LibraryManager.library, glulxe )