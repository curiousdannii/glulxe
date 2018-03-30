// Startup code for Glulxe

#include <stdlib.h>
#include <stdio.h>
#include "emglulxeen.h"
#include "glk.h"
#include "glulxe.h"

// Adapted from glkunix_startup_code() from unixstrt.c
void emglulxeen ( glui32 gameStreamTag, glui32 profileStreamTag, glui32 profcalls )
{
    unsigned char buf[12];
    int res;

     // Initialise emglken
    init_emglken();

    #if VM_PROFILING
        if ( profileStreamTag )
        {
            strid_t profstr = gli_new_stream( strtype_File, profileStreamTag, 0 );
            if ( profstr )
            {
                setup_profile( profstr, NULL );
            }
        }
        if ( profcalls )
        {
            profile_set_call_counts( TRUE );
        }
    #endif /* VM_PROFILING */

    // Set up the game stream
    gamefile = gli_new_stream( strtype_File, gameStreamTag, 0 );

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

// Stub functions for profiling mode
#if VM_PROFILING

void emautosave( glui32 jsonStreamTag, glui32 ramStreamTag ) {}

#else /* VM_PROFILING */

void print_json_property( char *prop, glui32 num, int last );

void emautosave( glui32 jsonStreamTag, glui32 ramStreamTag )
{
    // Prepare for outputting JSON
    strid_t oldstr = glk_stream_get_current();
    strid_t jsonstream = gli_new_stream( strtype_Memory, jsonStreamTag, 0 );
    glk_stream_set_current( jsonstream );
    glk_put_char( '{' );

    // Output the number properties
    glui32 iosysmode;
    glui32 iosysrock;
    stream_get_iosys( &iosysmode, &iosysrock );
    print_json_property( "iosysmode", iosysmode, 0 );
    print_json_property( "iosysrock", iosysrock, 0 );
    print_json_property( "protectend", protectend, 0 );
    print_json_property( "protectstart", protectstart, 0 );
    print_json_property( "stringtable", stringtable, 1 );

    // Finish up the JSON
    glk_put_char( '}' );
    glk_stream_set_current( oldstr );

    // Save the RAM/savefile
    strid_t ramstream = gli_new_stream( strtype_Memory, ramStreamTag, 0 );
    perform_save( ramstream );
}

void print_json_property( char *prop, glui32 num, int last )
{
    glk_put_char( '"' );
    glk_put_string( prop );
    glk_put_string( "\":" );
    stream_num( num, FALSE, 0 );
    if ( !last )
    {
        glk_put_char( ',' );
    }
}

#endif /* VM_PROFILING */