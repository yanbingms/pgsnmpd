/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.12 $ of : mfd-data-access.m2c,v $
 *
 * $Id: pgsqlPgAmTable_data_access.h,v 1.1 2008/01/17 12:29:36 eggyknap Exp $
 */
#ifndef PGSQLPGAMTABLE_DATA_ACCESS_H
#define PGSQLPGAMTABLE_DATA_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif


/* *********************************************************************
 * function declarations
 */

/* *********************************************************************
 * Table declarations
 */
/**********************************************************************
 **********************************************************************
 ***
 *** Table pgsqlPgAmTable
 ***
 **********************************************************************
 **********************************************************************/
/*
 * pgsqlPgAmTable is subid 2 of pgsqlCatalogTables.
 * Its status is Current.
 * OID: .1.3.6.1.4.1.27645.1.2.2, length: 10
*/


    int pgsqlPgAmTable_init_data(pgsqlPgAmTable_registration_ptr pgsqlPgAmTable_reg);


/*
 * TODO:180:o: Review pgsqlPgAmTable cache timeout.
 * The number of seconds before the cache times out
 */
#define PGSQLPGAMTABLE_CACHE_TIMEOUT   60

void pgsqlPgAmTable_container_init(netsnmp_container **container_ptr_ptr,
                             netsnmp_cache *cache);
int pgsqlPgAmTable_cache_load(netsnmp_container *container);
void pgsqlPgAmTable_cache_free(netsnmp_container *container);

    /*
    ***************************************************
    ***             START EXAMPLE CODE              ***
    ***---------------------------------------------***/
/* *********************************************************************
 * Since we have no idea how you really access your data, we'll go with
 * a worst case example: a flat text file.
 */
#define MAX_LINE_SIZE 256
    /*
    ***---------------------------------------------***
    ***              END  EXAMPLE CODE              ***
    ***************************************************/
    int pgsqlPgAmTable_row_prep( pgsqlPgAmTable_rowreq_ctx *rowreq_ctx);


#ifdef __cplusplus
}
#endif

#endif /* PGSQLPGAMTABLE_DATA_ACCESS_H */
