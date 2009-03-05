/*
 * Written by Ben Bergen
 * Computational Physics Group (CCS-2)
 * Computer, Computational, and Statistical Sciences Division
 * Los Alamos National Laboratory
 * Original - February 2008
*/

/* FIXME: WHY IS THERE A REDUNDANT COPY OF THIS IN CELL/SPE/SRC? */

#include <spu_mfcio.h>
#include <pipelines.h>

#define VERBOSE 0
#if VERBOSE
#include <stdio.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Main SPU program
//
// This will eventually replace the main in the particle directory, and will
// use overlays to call various operations, e.g., particle advance, sort, and
// field solve.  We should be able to do this with a single region under the
// current design.  Some of the structure of VPIC will need to be changed so
// that this event loop can be started outside of the particle advance.  This
// is the current structure.
////////////////////////////////////////////////////////////////////////////////

/* Must match definition in pipelines_spu.c; Geddy Lee rules */
#define SPU_COMPLETE ((uint32_t)2112)

typedef void
(*spe_pipeline_t)( MEM_PTR( void, 128 ) args,
                   int pipeline_rank,
                   int n_pipeline );

int
main( uint64_t id,
      uint64_t argp,
      uint64_t envp ) {
  spe_pipeline_t pipeline;
  MEM_PTR( void, 128 )args;
  int pipeline_rank;
  int n_pipeline;

# if VERBOSE
  fprintf(stderr, "######## Spinning up SPE %lld\n", id);
  fflush(stderr);
# endif

  // Read mailbox and interpret message as a function pointer
  // in the SPE symbol space
  for(;;) {
#   if VERBOSE
	fprintf(stderr, "######## Root Segment reading function...");
	fflush(stderr);
#   endif
    pipeline = (spe_pipeline_t)spu_read_in_mbox();
#   if VERBOSE
	fprintf(stderr, "######## Root Segment read %p...", pipeline);
	fflush(stderr);
#   endif
    if( pipeline==NULL ) break;

#   if VERBOSE
	fprintf(stderr, "######## Root Segment reading arguments...");
	fflush(stderr);
#   endif
    args = (((uint64_t)spu_read_in_mbox())<<32) |
            ((uint64_t)spu_read_in_mbox());
    pipeline_rank = spu_read_in_mbox();
    n_pipeline    = spu_read_in_mbox();

    // Execute pipeline
#   if VERBOSE
	fprintf(stderr, "######## Root Segment execute pipeline...");
	fflush(stderr);
#   endif
    pipeline( args, pipeline_rank, n_pipeline );

    // Notify caller of completion
#   if VERBOSE
	fprintf(stderr, "######## Root Segment write completion...");
	fflush(stderr);
#   endif
    spu_write_out_mbox(SPU_COMPLETE);
  } // for

  return 0;
} // main
