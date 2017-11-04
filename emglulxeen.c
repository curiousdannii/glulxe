// Startup code for Glulxe

#include <stdlib.h>
#include <stdio.h>
#include "emglulxeen.h"
#include "glulxe.h"

// Copied from gitMain from git.c, with an additional glk_exit call
void emglulxeen (char * data, glui32 dataSize)
{
    unsigned char buf[12];
    int res;

     // Initialise emglken
    init_emglken();

    // Set up the game stream
    gamefile = glk_stream_open_memory( data, dataSize, filemode_Read, 0 );

    /* Now we have to check to see if it's a Blorb file. */

    glk_stream_set_position( gamefile, 0, seekmode_Start );
    res = glk_get_buffer_stream( gamefile, (char *)buf, 12 );
    if ( !res )
    {
        fatal_error( "The data in this stand-alone game is too short to read." );
        return;
    }

    if ( buf[0] == 'G' && buf[1] == 'l' && buf[2] == 'u' && buf[3] == 'l' )
    {
        /* Load game directly from file. */
        locate_gamefile( FALSE );
    }
    else if ( buf[0] == 'F' && buf[1] == 'O' && buf[2] == 'R' && buf[3] == 'M'
    && buf[8] == 'I' && buf[9] == 'F' && buf[10] == 'R' && buf[11] == 'S' )
    {
        /* Load game from a chunk in the Blorb file. */
        locate_gamefile( TRUE );

        #if VM_DEBUGGER
        /* Load the debug info from the Blorb, if it wasn't loaded from a file. */
        if (!gameinfoloaded) {
            glui32 giblorb_ID_Dbug = giblorb_make_id('D', 'b', 'u', 'g');
            giblorb_err_t err;
            giblorb_result_t blorbres;
            err = giblorb_load_chunk_by_type(giblorb_get_resource_map(), 
            giblorb_method_FilePos, 
            &blorbres, giblorb_ID_Dbug, 0);
            if (!err) {
            int bres = debugger_load_info_chunk(gamefile, blorbres.data.startpos, blorbres.length);
            if (!bres)
                nonfatal_warning("Unable to parse game info.");
            else
                gameinfoloaded = TRUE;
            }
        }
        #endif /* VM_DEBUGGER */
    }
    else
    {
        fatal_error( "This is neither a Glulx game file nor a Blorb file "
            "which contains one." );
        return;
    }

    glk_main();
	glk_exit();
}