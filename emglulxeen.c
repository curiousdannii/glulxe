// Startup code for Glulxe

#include <stdlib.h>
#include <stdio.h>
#include "emglulxeen.h"
#include "glk.h"
#include "glulxe.h"

void emautorestore_hook( void );
void emselect_hook( glui32 ev );
void print_json_property( char *prop, glui32 num, int last );
static int parse_partial_operand(int *opmodes);

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

    #ifndef VM_PROFILING
        set_library_autorestore_hook( emautorestore_hook );
        set_library_select_hook( emselect_hook );
    #endif

    glk_main();
    glk_exit();
}

// Stub functions for profiling mode
#if VM_PROFILING

void emautosave( glui32 jsonStreamTag, glui32 ramStreamTag ) {}

#else /* VM_PROFILING */

static glui32 eventaddr;

void emautorestore_hook( void )
{
    glui32 ramStreamTag = 0;
    glui32 iosysmode;
    glui32 iosysrock;
    emautorestore( &ramStreamTag, &iosysmode, &iosysrock, &protectend, &protectstart, &stringtable );

    if ( ramStreamTag )
    {
        strid_t ramstream = gli_new_stream( strtype_Memory, ramStreamTag, 0 );
        perform_restore( ramstream, 1 );
        glk_stream_close( ramstream, NULL );
        pop_callstub( 0 );
        stream_set_iosys( iosysmode, iosysrock );
    }
}

void emselect_hook( glui32 ev )
{
    eventaddr = ev;
}

void emautosave( glui32 jsonStreamTag, glui32 ramStreamTag )
{
    // Save the RAM/savefile
    // This code copied from iosstarm.m

    /* When the save file is autorestored, the VM will restart the @glk opcode. That means that the Glk argument (the event structure address) must be waiting on the stack. Possibly also the @glk opcode's operands -- these might or might not have come off the stack. */
    int res;
    int opmodes[3];
    res = parse_partial_operand(opmodes);
    if (!res)
        return;

    /* Push all the necessary arguments for the @glk opcode. */
    glui32 origstackptr = stackptr;
    int stackvals = 0;
    /* The event structure address: */
    stackvals++;
    if (stackptr+4 > stacksize)
        fatal_error("Stack overflow in autosave callstub.");
    StkW4(stackptr, eventaddr);
    stackptr += 4;
    if (opmodes[1] == 8) {
        /* The number of Glk arguments (1): */
        stackvals++;
        if (stackptr+4 > stacksize)
            fatal_error("Stack overflow in autosave callstub.");
        StkW4(stackptr, 1);
        stackptr += 4;
    }
    if (opmodes[0] == 8) {
        /* The Glk call selector (0x00C0): */
        stackvals++;
        if (stackptr+4 > stacksize)
            fatal_error("Stack overflow in autosave callstub.");
        StkW4(stackptr, 0x00C0); /* glk_select */
        stackptr += 4;
    }

    /* Push a temporary callstub which contains the *last* PC -- the address of the @glk(select) invocation. */
    if (stackptr+16 > stacksize)
        fatal_error("Stack overflow in autosave callstub.");
    StkW4(stackptr+0, 0);
    StkW4(stackptr+4, 0);
    StkW4(stackptr+8, prevpc);
    StkW4(stackptr+12, frameptr);
    stackptr += 16;

    strid_t ramstream = gli_new_stream( strtype_Memory, ramStreamTag, 0 );
    perform_save( ramstream );

    stackptr -= 16; // discard the temporary callstub
    stackptr -= 4 * stackvals; // discard the temporary arguments
    if (origstackptr != stackptr)
        fatal_error("Stack pointer mismatch in autosave");

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
}

/* Backtrack through the current opcode (at prevpc), and figure out whether its input arguments are on the stack or not. This will be important when setting up the saved VM state for restarting its opcode.

    The opmodes argument must be an array int[3]. Returns YES on success.
 */
static int parse_partial_operand(int *opmodes)
{
    glui32 addr = prevpc;

    /* Fetch the opcode number. */
    glui32 opcode = Mem1(addr);
    addr++;
    if (opcode & 0x80) {
        /* More than one-byte opcode. */
        if (opcode & 0x40) {
            /* Four-byte opcode */
            opcode &= 0x3F;
            opcode = (opcode << 8) | Mem1(addr);
            addr++;
            opcode = (opcode << 8) | Mem1(addr);
            addr++;
            opcode = (opcode << 8) | Mem1(addr);
            addr++;
        }
        else {
            /* Two-byte opcode */
            opcode &= 0x7F;
            opcode = (opcode << 8) | Mem1(addr);
            addr++;
        }
    }

    if (opcode != 0x130) { /* op_glk */
        //NSLog(@"iosglk_startup_code: parsed wrong opcode: %d", opcode);
        return 0;
    }

    /* @glk has operands LLS. */
    opmodes[0] = Mem1(addr) & 0x0F;
    opmodes[1] = (Mem1(addr) >> 4) & 0x0F;
    opmodes[2] = Mem1(addr+1) & 0x0F;

    return 1;
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