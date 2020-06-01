/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef IPC_SEND_H_INCLUDED
#define IPC_SEND_H_INCLUDED

#include "ch4_impl.h"
#include "mpidimpl.h"
#include "shm_control.h"
#include "ipc_types.h"
#include "ipc_mem.h"
#include "ipc_p2p.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_IPC_mpi_isend(const void *buf, MPI_Aint count,
                                                 MPI_Datatype datatype, int rank, int tag,
                                                 MPIR_Comm * comm, int context_offset,
                                                 MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    bool dt_contig;
    MPI_Aint true_lb;
    size_t data_sz;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_MPI_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_MPI_ISEND);

    MPIDI_Datatype_check_contig_size_lb(datatype, count, dt_contig, data_sz, true_lb);

    MPIDI_IPCI_mem_attr_t attr;
    MPIDI_IPCI_get_mem_attr((char *) buf + true_lb, &attr);

    if (rank != comm->rank && attr.ipc_type != MPIDI_IPCI_TYPE__NONE &&
        data_sz >= attr.threshold.send_lmt_sz) {
        if (dt_contig) {
            mpi_errno = MPIDI_IPCI_send_contig_lmt(buf, count, datatype, data_sz, rank, tag, comm,
                                                   context_offset, addr, attr, request);
            MPIR_ERR_CHECK(mpi_errno);
            goto fn_exit;
        }
    }

    mpi_errno = MPIDI_POSIX_mpi_isend(buf, count, datatype, rank, tag, comm,
                                      context_offset, addr, request);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_MPI_ISEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* IPC_SEND_H_INCLUDED */