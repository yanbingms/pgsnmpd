/*-------------------------------------------------------------------------
 * rdbmsDbParamTable_interface.c
 *
 *      RDBMSMIB mib module.
 *
 *      Copyright (c) 2004-2007, PostgreSQL Global Development Group
 *      Author: Joshua Tolley
 *
 * $Id: rdbmsDbParamTable_interface.c,v 1.3 2007/09/13 14:20:43 h-saito Exp $
 *
 *-------------------------------------------------------------------------
 */

#include "pgsnmpd.h"

/* include our parent header */
#include "rdbmsDbParamTable.h"
#include "rdbmsDbParamTable_interface.h"

/**********************************************************************
 **********************************************************************
 ***
 *** Table rdbmsDbParamTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * rdbmsDbParamTable is subid 3 of rdbmsObjects.
 * Its status is Current.
 * OID: .1.3.6.1.2.1.39.1.3, length: 9
*/
typedef struct rdbmsDbParamTable_interface_ctx_s {

   netsnmp_container              *container;
   netsnmp_cache                  *cache; /* optional cache */

   rdbmsDbParamTable_registration_ptr      user_ctx;
   
   netsnmp_table_registration_info  tbl_info;

   netsnmp_baby_steps_access_methods access_multiplexer;

} rdbmsDbParamTable_interface_ctx;

static rdbmsDbParamTable_interface_ctx rdbmsDbParamTable_if_ctx;

static void _rdbmsDbParamTable_container_init(
    rdbmsDbParamTable_interface_ctx *if_ctx);


static Netsnmp_Node_Handler _mfd_rdbmsDbParamTable_pre_request;
static Netsnmp_Node_Handler _mfd_rdbmsDbParamTable_post_request;
static Netsnmp_Node_Handler _mfd_rdbmsDbParamTable_object_lookup;
static Netsnmp_Node_Handler _mfd_rdbmsDbParamTable_get_values;
/**
 * @internal
 * Initialize the table rdbmsDbParamTable 
 *    (Define its contents and how it's structured)
 */
void
_rdbmsDbParamTable_initialize_interface(rdbmsDbParamTable_registration_ptr reg_ptr,  u_long flags)
{
    netsnmp_baby_steps_access_methods *access_multiplexer =
        &rdbmsDbParamTable_if_ctx.access_multiplexer;
    netsnmp_table_registration_info *tbl_info = &rdbmsDbParamTable_if_ctx.tbl_info;
    netsnmp_handler_registration *reginfo;
    netsnmp_mib_handler *handler;
    int    mfd_modes = 0;

    DEBUGMSGTL(("internal:rdbmsDbParamTable:_rdbmsDbParamTable_initialize_interface","called\n"));


    /*************************************************
     *
     * save interface context for rdbmsDbParamTable
     */
    /*
     * Setting up the table's definition
     */
    netsnmp_table_helper_add_indexes(tbl_info,
                                  ASN_INTEGER, /** index: rdbmsDbIndex */
                                  ASN_OCTET_STR, /** index: rdbmsDbParamName */
                                  ASN_INTEGER, /** index: rdbmsDbParamSubIndex */
                             0);

    /*  Define the minimum and maximum accessible columns.  This
        optimizes retrival. */
    tbl_info->min_column = RDBMSDBPARAMTABLE_MIN_COL;
    tbl_info->max_column = RDBMSDBPARAMTABLE_MAX_COL;

    /*
     * save users context
     */
    rdbmsDbParamTable_if_ctx.user_ctx = reg_ptr;

    /*
     * call data access initialization code
     */
    rdbmsDbParamTable_init_data(reg_ptr);

    /*
     * set up the container
     */
    _rdbmsDbParamTable_container_init(&rdbmsDbParamTable_if_ctx);
    if (NULL == rdbmsDbParamTable_if_ctx.container) {
        snmp_log(LOG_ERR,"could not initialize container for rdbmsDbParamTable\n");
        return;
    }
    
    /*
     * access_multiplexer: REQUIRED wrapper for get request handling
     */
    access_multiplexer->object_lookup = _mfd_rdbmsDbParamTable_object_lookup;
    access_multiplexer->get_values = _mfd_rdbmsDbParamTable_get_values;

    /*
     * no wrappers yet
     */
    access_multiplexer->pre_request = _mfd_rdbmsDbParamTable_pre_request;
    access_multiplexer->post_request = _mfd_rdbmsDbParamTable_post_request;


    /*************************************************
     *
     * Create a registration, save our reg data, register table.
     */
    DEBUGMSGTL(("rdbmsDbParamTable:init_rdbmsDbParamTable",
                "Registering rdbmsDbParamTable as a mibs-for-dummies table.\n"));		 
    handler = netsnmp_baby_steps_access_multiplexer_get(access_multiplexer);
    reginfo = netsnmp_handler_registration_create("rdbmsDbParamTable", handler,
                                                  rdbmsDbParamTable_oid,
                                                  rdbmsDbParamTable_oid_size,
                                                  HANDLER_CAN_BABY_STEP |
                                                  HANDLER_CAN_RONLY
                                                  );
    if(NULL == reginfo) {
        snmp_log(LOG_ERR,"error registering table rdbmsDbParamTable\n");
        return;
    }
    reginfo->my_reg_void = &rdbmsDbParamTable_if_ctx;

    /*************************************************
     *
     * set up baby steps handler, create it and inject it
     */
    if( access_multiplexer->object_lookup )
        mfd_modes |= BABY_STEP_OBJECT_LOOKUP;
    if( access_multiplexer->set_values )
        mfd_modes |= BABY_STEP_SET_VALUES;
    if( access_multiplexer->irreversible_commit )
        mfd_modes |= BABY_STEP_IRREVERSIBLE_COMMIT;
    if( access_multiplexer->object_syntax_checks )
        mfd_modes |= BABY_STEP_CHECK_OBJECT;

    if( access_multiplexer->pre_request )
        mfd_modes |= BABY_STEP_PRE_REQUEST;
    if( access_multiplexer->post_request )
        mfd_modes |= BABY_STEP_POST_REQUEST;
    
    if( access_multiplexer->undo_setup )
        mfd_modes |= BABY_STEP_UNDO_SETUP;
    if( access_multiplexer->undo_cleanup )
        mfd_modes |= BABY_STEP_UNDO_CLEANUP;
    if( access_multiplexer->undo_sets )
        mfd_modes |= BABY_STEP_UNDO_SETS;
    
    if( access_multiplexer->row_creation )
        mfd_modes |= BABY_STEP_ROW_CREATE;
    if( access_multiplexer->consistency_checks )
        mfd_modes |= BABY_STEP_CHECK_CONSISTENCY;
    if( access_multiplexer->commit )
        mfd_modes |= BABY_STEP_COMMIT;
    if( access_multiplexer->undo_commit )
        mfd_modes |= BABY_STEP_UNDO_COMMIT;
    
    handler = netsnmp_baby_steps_handler_get(mfd_modes);
    netsnmp_inject_handler(reginfo, handler);

    /*************************************************
     *
     * inject row_merge helper with prefix rootoid_len + 2 (entry.col)
     */
    handler = netsnmp_get_row_merge_handler(reginfo->rootoid_len + 2);
    netsnmp_inject_handler(reginfo, handler);

    /*************************************************
     *
     * inject container_table helper
     */
    handler =
        netsnmp_container_table_handler_get(tbl_info,
                                            rdbmsDbParamTable_if_ctx.container,
                                            TABLE_CONTAINER_KEY_NETSNMP_INDEX);
    netsnmp_inject_handler( reginfo, handler );

    /*************************************************
     *
     * inject cache helper
     */
    if(NULL != rdbmsDbParamTable_if_ctx.cache) {
        handler = netsnmp_cache_handler_get(rdbmsDbParamTable_if_ctx.cache);
        netsnmp_inject_handler( reginfo, handler );
    }

    /*
     * register table
     */
    netsnmp_register_table(reginfo, tbl_info);
} /* _rdbmsDbParamTable_initialize_interface */

void
rdbmsDbParamTable_valid_columns_set(netsnmp_column_info *vc)
{
    rdbmsDbParamTable_if_ctx.tbl_info.valid_columns = vc;
} /* rdbmsDbParamTable_valid_columns_set */

/**
 * @internal
 * convert the index component stored in the context to an oid
 */
int
rdbmsDbParamTable_index_to_oid(netsnmp_index *oid_idx,
                         rdbmsDbParamTable_mib_index *mib_idx)
{
    int err = SNMP_ERR_NOERROR;
    
    /*
     * temp storage for parsing indexes
     */
    /*
     * rdbmsDbIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/a/w/e/R/d/h
     */
    netsnmp_variable_list var_rdbmsDbIndex;
    /*
     * rdbmsDbParamName(1)/DisplayString/ASN_OCTET_STR/char(char)//L/a/w/e/R/d/H
     */
    netsnmp_variable_list var_rdbmsDbParamName;
    /*
     * rdbmsDbParamSubIndex(2)/INTEGER/ASN_INTEGER/long(long)//l/a/w/e/R/d/h
     */
    netsnmp_variable_list var_rdbmsDbParamSubIndex;

    /*
     * set up varbinds
     */
    memset( &var_rdbmsDbIndex, 0x00, sizeof(var_rdbmsDbIndex) );
    var_rdbmsDbIndex.type = ASN_INTEGER;
    memset( &var_rdbmsDbParamName, 0x00, sizeof(var_rdbmsDbParamName) );
    var_rdbmsDbParamName.type = ASN_OCTET_STR;
    memset( &var_rdbmsDbParamSubIndex, 0x00, sizeof(var_rdbmsDbParamSubIndex) );
    var_rdbmsDbParamSubIndex.type = ASN_INTEGER;

    /*
     * chain temp index varbinds together
     */
    var_rdbmsDbIndex.next_variable =  &var_rdbmsDbParamName; var_rdbmsDbParamName.next_variable =  &var_rdbmsDbParamSubIndex; var_rdbmsDbParamSubIndex.next_variable =  NULL;


    DEBUGMSGTL(("verbose:rdbmsDbParamTable:rdbmsDbParamTable_index_to_oid","called\n"));

        /* rdbmsDbIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/a/w/e/R/d/h */
    snmp_set_var_value(&var_rdbmsDbIndex, (u_char*)&mib_idx->rdbmsDbIndex,
                       sizeof(mib_idx->rdbmsDbIndex));

        /* rdbmsDbParamName(1)/DisplayString/ASN_OCTET_STR/char(char)//L/a/w/e/R/d/H */
    snmp_set_var_value(&var_rdbmsDbParamName, (u_char*)&mib_idx->rdbmsDbParamName,
                       mib_idx->rdbmsDbParamName_len * sizeof(mib_idx->rdbmsDbParamName[0]));

        /* rdbmsDbParamSubIndex(2)/INTEGER/ASN_INTEGER/long(long)//l/a/w/e/R/d/h */
    snmp_set_var_value(&var_rdbmsDbParamSubIndex, (u_char*)&mib_idx->rdbmsDbParamSubIndex,
                       sizeof(mib_idx->rdbmsDbParamSubIndex));


    err = build_oid_noalloc(oid_idx->oids, oid_idx->len, (size_t *)&oid_idx->len,
                           NULL, 0, &var_rdbmsDbIndex);
    if(err)
        snmp_log(LOG_ERR,"error %d converting index to oid\n", err);

    /*
     * parsing may have allocated memory. free it.
     */
    snmp_reset_var_buffers( &var_rdbmsDbIndex );

    return err;
} /* rdbmsDbParamTable_index_to_oid */

/**
 * extract rdbmsDbParamTable indexes from a netsnmp_index
 *
 * @retval SNMP_ERR_NOERROR  : no error
 * @retval SNMP_ERR_GENERR   : error
 */
int
rdbmsDbParamTable_index_from_oid(netsnmp_index *oid_idx,
                         rdbmsDbParamTable_mib_index *mib_idx)
{
    int err = SNMP_ERR_NOERROR;
    
    /*
     * temp storage for parsing indexes
     */
    /*
     * rdbmsDbIndex(1)/INTEGER/ASN_INTEGER/long(long)//l/a/w/e/R/d/h
     */
    netsnmp_variable_list var_rdbmsDbIndex;
    /*
     * rdbmsDbParamName(1)/DisplayString/ASN_OCTET_STR/char(char)//L/a/w/e/R/d/H
     */
    netsnmp_variable_list var_rdbmsDbParamName;
    /*
     * rdbmsDbParamSubIndex(2)/INTEGER/ASN_INTEGER/long(long)//l/a/w/e/R/d/h
     */
    netsnmp_variable_list var_rdbmsDbParamSubIndex;

    /*
     * set up varbinds
     */
    memset( &var_rdbmsDbIndex, 0x00, sizeof(var_rdbmsDbIndex) );
    var_rdbmsDbIndex.type = ASN_INTEGER;
    memset( &var_rdbmsDbParamName, 0x00, sizeof(var_rdbmsDbParamName) );
    var_rdbmsDbParamName.type = ASN_OCTET_STR;
    memset( &var_rdbmsDbParamSubIndex, 0x00, sizeof(var_rdbmsDbParamSubIndex) );
    var_rdbmsDbParamSubIndex.type = ASN_INTEGER;

    /*
     * chain temp index varbinds together
     */
    var_rdbmsDbIndex.next_variable =  &var_rdbmsDbParamName; var_rdbmsDbParamName.next_variable =  &var_rdbmsDbParamSubIndex; var_rdbmsDbParamSubIndex.next_variable =  NULL;


    DEBUGMSGTL(("verbose:rdbmsDbParamTable:rdbmsDbParamTable_index_from_oid","called\n"));

    /*
     * parse the oid into the individual index components
     */
    err = parse_oid_indexes( oid_idx->oids, oid_idx->len,
                             &var_rdbmsDbIndex );
    if (err == SNMP_ERR_NOERROR) {
        /*
         * copy out values
         */
    mib_idx->rdbmsDbIndex = *((long *)var_rdbmsDbIndex.val.string);
    /*
     * NOTE: val_len is in bytes, rdbmsDbParamName_len might not be
     */
         if(var_rdbmsDbParamName.val_len > sizeof(mib_idx->rdbmsDbParamName))
             err = SNMP_ERR_GENERR;
         else {
             memcpy(mib_idx->rdbmsDbParamName, var_rdbmsDbParamName.val.string, var_rdbmsDbParamName.val_len);
             mib_idx->rdbmsDbParamName_len = var_rdbmsDbParamName.val_len / sizeof(mib_idx->rdbmsDbParamName[0]);
         }
    mib_idx->rdbmsDbParamSubIndex = *((long *)var_rdbmsDbParamSubIndex.val.string);


    }

    /*
     * parsing may have allocated memory. free it.
     */
    snmp_reset_var_buffers( &var_rdbmsDbIndex );

    return err;
} /* rdbmsDbParamTable_index_from_oid */


/* *********************************************************************
 * @internal
 * allocate resources for a rdbmsDbParamTable_rowreq_ctx
 */
rdbmsDbParamTable_rowreq_ctx *
rdbmsDbParamTable_allocate_rowreq_ctx(void)
{
    rdbmsDbParamTable_rowreq_ctx *rowreq_ctx =
                  SNMP_MALLOC_TYPEDEF(rdbmsDbParamTable_rowreq_ctx);

    DEBUGMSGTL(("internal:rdbmsDbParamTable:rdbmsDbParamTable_allocate_rowreq_ctx","called\n"));

    if(NULL == rowreq_ctx) {
        snmp_log(LOG_ERR,"Couldn't allocate memory for a "
                 "rdbmsDbParamTable_rowreq_ctx.\n");
    }

    rowreq_ctx->oid_idx.oids = rowreq_ctx->oid_tmp;

    rowreq_ctx->rdbmsDbParamTable_data_list = NULL;
    rowreq_ctx->rdbmsDbParamTable_reg = rdbmsDbParamTable_if_ctx.user_ctx;


    return rowreq_ctx;
} /* rdbmsDbParamTable_allocate_rowreq_ctx */

/*
 * @internal
 * release resources for a rdbmsDbParamTable_rowreq_ctx
 */
void
rdbmsDbParamTable_release_rowreq_ctx(rdbmsDbParamTable_rowreq_ctx *rowreq_ctx)
{
    DEBUGMSGTL(("internal:rdbmsDbParamTable:rdbmsDbParamTable_release_rowreq_ctx","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);
    

    /*
     * free index oid pointer
     */
    if(rowreq_ctx->oid_idx.oids != rowreq_ctx->oid_tmp)
        free(rowreq_ctx->oid_idx.oids);

    SNMP_FREE(rowreq_ctx);
} /* rdbmsDbParamTable_release_rowreq_ctx */

/**
 * @internal
 * wrapper
 */
static int
_mfd_rdbmsDbParamTable_pre_request(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *agtreq_info,
                            netsnmp_request_info *requests)
{
    int rc = rdbmsDbParamTable_pre_request(rdbmsDbParamTable_if_ctx.user_ctx);
    if (MFD_SUCCESS != rc) {
        /*
         * nothing we can do about it but log it
         */
        DEBUGMSGTL(("internal:rdbmsDbParamTable","error %d from "
                    "rdbmsDbParamTable_pre_request\n", rc));
        netsnmp_request_set_error_all(requests, SNMP_VALIDATE_ERR(rc));
    }
    
    return SNMP_ERR_NOERROR;
} /* _mfd_rdbmsDbParamTable_pre_request */

/**
 * @internal
 * wrapper
 */
static int
_mfd_rdbmsDbParamTable_post_request(netsnmp_mib_handler *handler,
                             netsnmp_handler_registration *reginfo,
                             netsnmp_agent_request_info *agtreq_info,
                             netsnmp_request_info *requests)
{
    rdbmsDbParamTable_rowreq_ctx *rowreq_ctx;
    int rc = rdbmsDbParamTable_post_request(rdbmsDbParamTable_if_ctx.user_ctx);
    if (MFD_SUCCESS != rc) {
        /*
         * nothing we can do about it but log it
         */
        DEBUGMSGTL(("internal:rdbmsDbParamTable","error %d from "
                    "rdbmsDbParamTable_post_request\n", rc));
    }
    
    /*
     * if there are no errors, check for and handle row creation/deletion
     */
    rc = netsnmp_check_requests_error(requests);
    if ((SNMP_ERR_NOERROR == rc) &&
        (NULL !=
         (rowreq_ctx = netsnmp_container_table_row_extract(requests)))) {
        if (rowreq_ctx->rowreq_flags & MFD_ROW_CREATED) {
            rowreq_ctx->rowreq_flags &= ~MFD_ROW_CREATED;
            CONTAINER_INSERT(rdbmsDbParamTable_if_ctx.container, rowreq_ctx);
        }
        else if (rowreq_ctx->rowreq_flags & MFD_ROW_DELETED) {
            CONTAINER_REMOVE(rdbmsDbParamTable_if_ctx.container, rowreq_ctx);
            rdbmsDbParamTable_release_rowreq_ctx(rowreq_ctx);
        }
    }

    return SNMP_ERR_NOERROR;
} /* _mfd_rdbmsDbParamTable_post_request */

/**
 * @internal
 * wrapper
 */
static int
_mfd_rdbmsDbParamTable_object_lookup(netsnmp_mib_handler *handler,
                         netsnmp_handler_registration *reginfo,
                         netsnmp_agent_request_info *agtreq_info,
                         netsnmp_request_info *requests)
{
    rdbmsDbParamTable_rowreq_ctx *rowreq_ctx =
                  netsnmp_container_table_row_extract(requests);
    
    DEBUGMSGTL(("internal:rdbmsDbParamTable:_mfd_rdbmsDbParamTable_object_lookup","called\n"));

    /*
     * get our context from mfd
     * rdbmsDbParamTable_interface_ctx *if_ctx =
     *             (rdbmsDbParamTable_interface_ctx *)reginfo->my_reg_void;
     */

    if(NULL == rowreq_ctx) {
        netsnmp_request_set_error_all(requests, SNMP_ERR_NOCREATION);
    }
    else {
        rdbmsDbParamTable_row_prep(rowreq_ctx);
    }

    return SNMP_ERR_NOERROR;
} /* _mfd_rdbmsDbParamTable_object_lookup */

/***********************************************************************
 *
 * GET processing
 *
 ***********************************************************************/
/*
 * @internal
 * Retrieve the value for a particular column
 */
NETSNMP_STATIC_INLINE int
_rdbmsDbParamTable_get_column( rdbmsDbParamTable_rowreq_ctx *rowreq_ctx,
                       netsnmp_variable_list *var, int column )
{
    int rc = SNMPERR_SUCCESS;
    
    DEBUGMSGTL(("internal:rdbmsDbParamTable:_mfd_rdbmsDbParamTable_get_column","called\n"));


    netsnmp_assert(NULL != rowreq_ctx);

    switch(column) {

    /* rdbmsDbParamID(3)/AutonomousType/ASN_OBJECT_ID/oid(oid)//L/A/w/e/r/d/h */
    case COLUMN_RDBMSDBPARAMID:
    var->type = ASN_OBJECT_ID;
rc = rdbmsDbParamID_get(rowreq_ctx, (oid **)&var->val.string, &var->val_len );
        break;

    /* rdbmsDbParamCurrValue(4)/DisplayString/ASN_OCTET_STR/char(char)//L/A/W/e/R/d/H */
    case COLUMN_RDBMSDBPARAMCURRVALUE:
    var->type = ASN_OCTET_STR;
rc = rdbmsDbParamCurrValue_get(rowreq_ctx, (char **)&var->val.string, &var->val_len );
        break;

    /* rdbmsDbParamComment(5)/DisplayString/ASN_OCTET_STR/char(char)//L/A/W/e/R/d/H */
    case COLUMN_RDBMSDBPARAMCOMMENT:
    var->type = ASN_OCTET_STR;
rc = rdbmsDbParamComment_get(rowreq_ctx, (char **)&var->val.string, &var->val_len );
        break;

     default:
         snmp_log(LOG_ERR,"unknown column %d in _rdbmsDbParamTable_get_column\n", column);
         break;
    }

    return rc;
} /* _rdbmsDbParamTable_get_column */

int
_mfd_rdbmsDbParamTable_get_values(netsnmp_mib_handler *handler,
                         netsnmp_handler_registration *reginfo,
                         netsnmp_agent_request_info *agtreq_info,
                         netsnmp_request_info *requests)
{
    rdbmsDbParamTable_rowreq_ctx *rowreq_ctx =
                  netsnmp_container_table_row_extract(requests);
    netsnmp_table_request_info * tri;
    u_char                     * old_string;
    void                      (*dataFreeHook)(void *);
    int                        rc;

    DEBUGMSGTL(("internal:rdbmsDbParamTable:_mfd_rdbmsDbParamTable_get_values","called\n"));

    netsnmp_assert(NULL != rowreq_ctx);
    
    for(;requests; requests = requests->next) {
        /*
         * save old pointer, so we can free it if replaced
         */
        old_string = requests->requestvb->val.string;
        dataFreeHook = requests->requestvb->dataFreeHook;
        if(NULL == requests->requestvb->val.string) {
            requests->requestvb->val.string = requests->requestvb->buf;
            requests->requestvb->val_len = sizeof(requests->requestvb->buf);
        }
        else if(requests->requestvb->buf == requests->requestvb->val.string) {
            if(requests->requestvb->val_len != sizeof(requests->requestvb->buf))
                requests->requestvb->val_len = sizeof(requests->requestvb->buf);
        }

        /*
         * get column data
         */
        tri = netsnmp_extract_table_info(requests);
        if(NULL == tri)
            continue;
        
        rc = _rdbmsDbParamTable_get_column(rowreq_ctx, requests->requestvb, tri->colnum);
        if(rc) {
            if(MFD_SKIP == rc) {
                requests->requestvb->type = ASN_PRIV_RETRY;
                rc = SNMP_ERR_NOERROR;
            }
        }
        else if (NULL == requests->requestvb->val.string) {
            snmp_log(LOG_ERR,"NULL varbind data pointer!\n");
            rc = SNMP_ERR_GENERR;
        }
        if(rc)
            netsnmp_request_set_error(requests, SNMP_VALIDATE_ERR(rc));

        /*
         * if the buffer wasn't used previously for the old data (i.e. it
         * was allcoated memory)  and the get routine replaced the pointer,
         * we need to free the previous pointer.
         */
        if(old_string && (old_string != requests->requestvb->buf) &&
           (requests->requestvb->val.string != old_string)) {
            if(dataFreeHook)
                (*dataFreeHook)(old_string);
            else
                free(old_string);
        }
    } /* for results */

    return SNMP_ERR_NOERROR;
} /* _mfd_rdbmsDbParamTable_get_values */

/***********************************************************************
 *
 * SET processing
 *
 ***********************************************************************/

/*
 * NOT APPLICABLE (per MIB or user setting)
 */
/***********************************************************************
 *
 * DATA ACCESS
 *
 ***********************************************************************/
/**
 * @internal
 */
static int
_cache_load(netsnmp_cache *cache, void *vmagic)
{
    DEBUGMSGTL(("internal:rdbmsDbParamTable:_cache_load","called\n"));

    if((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache for rdbmsDbParamTable_cache_load\n");
        return -1;
    }

    /** should only be called for an invalid or expired cache */
    netsnmp_assert((0 == cache->valid) || (1 == cache->expired));
    
    /*
     * call user code
     */
    return rdbmsDbParamTable_cache_load((netsnmp_container*)cache->magic);
} /* _cache_load */

/**
 * @internal
 */
static void
_cache_item_free(rdbmsDbParamTable_rowreq_ctx *rowreq_ctx, void *context)
{
    DEBUGMSGTL(("internal:rdbmsDbParamTable:_cache_item_free","called\n"));

    if(NULL == rowreq_ctx)
        return;

    rdbmsDbParamTable_release_rowreq_ctx(rowreq_ctx);
} /* _cache_item_free */

/**
 * @internal
 */
static void
_cache_free(netsnmp_cache *cache, void *magic)
{
    netsnmp_container *container;

    DEBUGMSGTL(("internal:rdbmsDbParamTable:_cache_free","called\n"));

    if((NULL == cache) || (NULL == cache->magic)) {
        snmp_log(LOG_ERR, "invalid cache in rdbmsDbParamTable_cache_free\n");
        return;
    }

    container = (netsnmp_container*)cache->magic;

    /*
     * call user code
     */
    rdbmsDbParamTable_cache_free(container);
    
    /*
     * free all items. inefficient, but easy.
     */
    CONTAINER_CLEAR(container,
                    (netsnmp_container_obj_func *)_cache_item_free,
                    NULL);
} /* _cache_free */

/**
 * @internal
 * initialize the iterator container with functions or wrappers
 */
void
_rdbmsDbParamTable_container_init(rdbmsDbParamTable_interface_ctx *if_ctx)
{
    DEBUGMSGTL(("internal:rdbmsDbParamTable:_rdbmsDbParamTable_container_init","called\n"));
    
    /*
     * set up the cache
     */
    if_ctx->cache = netsnmp_cache_create(30, /* timeout in seconds */
                                         _cache_load, _cache_free,
                                         rdbmsDbParamTable_oid,
                                         rdbmsDbParamTable_oid_size);

    if(NULL == if_ctx->cache) {
        snmp_log(LOG_ERR, "error creating cache for rdbmsDbParamTable\n");
        return;
    }

    if_ctx->cache->flags = NETSNMP_CACHE_DONT_INVALIDATE_ON_SET;

    rdbmsDbParamTable_container_init(&if_ctx->container, if_ctx->cache);
    if(NULL == if_ctx->container)
        if_ctx->container = netsnmp_container_find("rdbmsDbParamTable:table_container");
    if(NULL == if_ctx->container) {
        snmp_log(LOG_ERR,"error creating container in "
                 "rdbmsDbParamTable_container_init\n");
        return;
    }
    if_ctx->cache->magic = (void*)if_ctx->container;
} /* _rdbmsDbParamTable_container_init */

