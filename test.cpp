#include <sys/types.h>
#include <stdio.h>
#include <db.h>


#define	DATABASE "access.db"


    int main()
    {
    	DB *dbp;
    	int ret;


    	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
    		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
    		exit (1);
    	}
    	if ((ret = dbp->open(dbp,
    	    NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
    		dbp->err(dbp, ret, "%s", DATABASE);
    		goto err;
    	}

