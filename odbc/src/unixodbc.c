#include "string.h"
#ifdef WINDOWS
#define SQL_NTS			(-3)

typedef const char*         LPCSTR;

struct con_pair
{
    char            *keyword;
    char            *attribute;
    char            *identifier;
    struct con_pair *next;
};

struct con_struct
{
    int             count;
    struct con_pair *list;
};

static size_t INI_MAX_PROPERTY_VALUE = 1000;

int __append_pair( struct con_struct *con_str, char *kword, char *value )
{
struct con_pair *ptr, *end;

    /* check that the keyword is not already in the list */

    end = NULL;
    if ( con_str -> count > 0 )
    {
        ptr = con_str -> list;
        while( ptr )
        {
            if( strcasecmp( kword, ptr -> keyword ) == 0 )
            {
                free( ptr -> attribute );
                ptr -> attribute = malloc( strlen( value ) + 1 );
                strcpy( ptr -> attribute, value );
                return 0;
            }
            end = ptr;
            ptr = ptr -> next;
        }
    }

    ptr = malloc( sizeof( *ptr ));

    ptr -> keyword = malloc( strlen( kword ) + 1 );
    strcpy( ptr -> keyword, kword );

    ptr -> attribute = malloc( strlen( value ) + 1 );
    strcpy( ptr -> attribute, value );

    con_str -> count ++;

    if ( con_str -> list )
    {
        end -> next = ptr;
        ptr -> next = NULL;
    }
    else
    {
        ptr -> next = NULL;
        con_str -> list = ptr;
    }

    return 0;
}

void __get_attr( char ** cp, char ** keyword, char ** value )
{
char * ptr;
int len;

    *keyword = *value = NULL;

    while ( isspace( **cp ) || **cp == ';' )
    {
        (*cp)++;
    }

    if ( !**cp )
        return;

    ptr = *cp;

    /* 
     * To handle the case attribute in which the attribute is of the form
     * "ATTR;" instead of "ATTR=VALUE;"
     */

    while ( **cp && **cp != '=' )
    {
        (*cp)++;
    }

    if ( !**cp )
        return;

    len = *cp - ptr;
    *keyword = malloc( len + 1 );
    memcpy( *keyword, ptr, len );
    (*keyword)[ len ] = '\0';

    (*cp)++;

    if ( **cp == '{' )
    {
        /* escaped with '{' - all characters until next '}' not followed by '}', or
           end of string, are part of the value */
        int i = 0;
        ptr = ++*cp;
        while ( **cp && (**cp != '}' || (*cp)[1] == '}') )
        {
            if ( **cp == '}' )
                (*cp)++;
            (*cp)++;
        }
        len = *cp - ptr;
        *value = malloc( len + 1 );
        while( ptr < *cp )
        {
            if ( ((*value)[i++] = *ptr++) == '}')
            {
                ptr++;
            }
        }
        (*value)[i] = 0;
        if ( **cp == '}' )
        {
            (*cp)++;
        }
    }
    else
    {
        /* non-escaped: all characters until ';' or end of string are value */
        ptr = *cp;
        while ( **cp && **cp != ';' )
        {
            (*cp)++;
        }
        len = *cp - ptr;
        *value = malloc( len + 1 );
        memcpy( *value, ptr, len );
        (*value)[ len ] = 0;
    }
}


struct con_pair * __get_pair( char ** cp )
{
char *keyword, *value;
struct con_pair * con_p;

    __get_attr( cp, &keyword, &value );
    if ( keyword )
    {
        con_p = malloc( sizeof( *con_p ));
        con_p -> keyword = keyword;
        con_p -> attribute = value;
        return con_p;
    }
    else
    {
        return NULL;
    }
}

int __parse_connection_string_ex( struct con_struct *con_str,
    char *str, int str_len, int exclude )
{
struct con_pair *cp;
char *local_str, *ptr;
int got_dsn = 0;    /* if we have a DSN then ignore any DRIVER or FILEDSN */
int got_driver = 0;    /* if we have a DRIVER or FILEDSN then ignore any DSN */

    con_str -> count = 0;
    con_str -> list = NULL;

    if ( str_len != SQL_NTS )
    {
        local_str = (char *)malloc( str_len + 1 );
        memcpy( local_str, str, str_len );
        local_str[ str_len ] = '\0';
    }
    else
    {
        local_str = str;
    }

    if ( !local_str || strlen( local_str ) == 0 ||
        ( strlen( local_str ) == 1 && *local_str == ';' ))
    {
        /* connection-string ::= empty-string [;] */
        if ( str_len != SQL_NTS )
            free( local_str );
        return 0;
    }

    ptr = local_str;

    while(( cp = __get_pair( &ptr )) != NULL )
    {
        if ( strcasecmp( cp -> keyword, "DSN" ) == 0 )
        {
            if ( got_driver && exclude ) {
                /* 11-29-2010 JM Modify to free the allocated memory before continuing. */
                free( cp -> keyword );
                free( cp -> attribute );
                free( cp );
                continue;
            }

            got_dsn = 1;
        }
        else if ( strcasecmp( cp -> keyword, "DRIVER" ) == 0 ||
            strcasecmp( cp -> keyword, "FILEDSN" ) == 0 )
        {
            if ( got_dsn && exclude ) {
                /* 11-29-2010 JM Modify to free the allocated memory before continuing. */
                free( cp -> keyword );
                free( cp -> attribute );
                free( cp );
                continue;
            }

            got_driver = 1;
        }

        __append_pair( con_str, cp -> keyword, cp -> attribute );
        free( cp -> keyword );
        free( cp -> attribute );
        free( cp );
    }

    if ( str_len != SQL_NTS )
        free( local_str );

    return 0;
}

int __parse_connection_string( struct con_struct *con_str, char *str, int str_len )
{
    return  __parse_connection_string_ex( con_str, str, str_len, 1 );
}

char * __get_attribute_value( struct con_struct * con_str, char * keyword )
{
struct con_pair *cp;

    if ( con_str -> count == 0 )
        return NULL;

    cp = con_str -> list;
    while( cp )
    {
        if( strcasecmp( keyword, cp -> keyword ) == 0 )
        {
            if ( cp -> attribute )
                return cp -> attribute;
            else
                return "";
        }
        cp = cp -> next;
    }
    return NULL;
}

void __release_conn( struct con_struct *con_str )
{
    struct con_pair *cp = con_str -> list;
    struct con_pair *save;

    while( cp )
    {
        free( cp -> attribute );
        free( cp -> keyword );
        save = cp;
        cp = cp -> next;
        free( save );
    }

    con_str -> count = 0;
}
#endif