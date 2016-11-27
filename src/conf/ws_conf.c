#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <ws_cycle.h>
#include <ws_conf.h>
#include <ws_mem.h>

#define YMZ_CONF_PATH_RESERVE						256
#define YMZ_CONF_FILE_SIZE_MAX						1024
#define YMZ_CONF_ITEM_LIMIT							100

typedef struct ws_conf_s ws_conf_t;
typedef struct ws_conf_item_s ws_conf_item_t;


ws_char_t* ws_conf_path = 0;

struct ws_conf_item_s
{
	const ws_char_t*		name;
	ws_int32_t				name_l;

	const ws_char_t*		value;
	ws_int32_t				value_l;
};


struct ws_conf_s
{
	ws_char_t*						buffer;
	ws_int32_t							size;

	ws_char_t*						path;

	ws_conf_item_t				item[ YMZ_CONF_ITEM_LIMIT ];
	ws_int32_t							item_n;
};



static ws_conf_t*			ws_conf = 0;

ws_char_t* ws_conf_get_path(  ws_cycle_t* cycle )
{
	ws_mem_pool_t*			pool;
	ws_mem_buf_t*			buf;
	ws_char_t*					path;
	ws_int32_t						pos;
	ws_int32_t						flag;
	ws_int32_t						size;

	
	pool = cycle->pool;
	buf = ws_mem_buf_alloc( pool, YMZ_CONF_PATH_RESERVE );
	if( !buf ){
		return 0;
	}

	path = buf->data;

#ifdef _WIN32
    pos = 0;
#else
    if (getcwd(path, YMZ_CONF_PATH_RESERVE - 1) == NULL) {
        return 0;
    }

    pos = strlen(path) - 2;

    while (pos >= 0) {
        if (path[pos] == '\/') {
            break;
        }

        --pos;
    }

    if (path[pos] != '\/') {
        printf("the conf path %s is error\n", path);
        return 0;
    }

    if (pos + sizeof("conf/whatsserver.conf") + 1 > YMZ_CONF_PATH_RESERVE) {
        printf("the conf path %s is error\n", path);
        return 0;
    }

    path[++pos] = 0;
#endif

	strcpy( path + pos, "conf/whatsserver.conf" );

	return path;
	
}

ws_int32_t ws_conf_module_init(  ws_cycle_t* cycle  )
{
	ws_mem_pool_t*			pool;
	ws_mem_buf_t*			buf;
	ws_char_t*					data;
	FILE*					f;
	ws_int32_t						flag;
	ws_int32_t						size;
	ws_conf_t*				conf;
	struct stat				st;

	size = sizeof( ws_conf_t );
	pool = cycle->pool;
	buf = ws_mem_buf_alloc( pool, size );
	if( !buf ){
		return YMZ_ERROR;
	}

	conf = buf->data;
	memset( conf, 0, size );

	if( !ws_conf_path ){
		ws_conf_path = ws_conf_get_path( cycle );
		if( !ws_conf_path ){
			return YMZ_ERROR;
		}
	}

	if( stat( ws_conf_path, &st ) == -1 ){
		printf( "the file %s has error\n", ws_conf_path );
		return YMZ_OK;
	}

	size = st.st_size;

	if( size + 1 > YMZ_CONF_FILE_SIZE_MAX ){
		printf( "the size of conf file is too big\n" );
		return YMZ_ERROR;
	}


	f = fopen( ws_conf_path, "rt" );
	if( !f ){
		printf( "the file %s is not exist\n", ws_conf_path );
		return YMZ_ERROR;
	}

	buf = ws_mem_buf_alloc( pool, size + 1 );
	if( !buf ){
		fclose( f );
		return YMZ_ERROR;
	}

	data = buf->data;

	flag = fread( data, 1, size, f );
	fclose( f );
	if( flag != size ){
		printf( "the file is changed \n" );
		return YMZ_ERROR;
	}

	data[ size ] = 0;

	if( ws_conf_parse( data, size, conf ) ){
		return YMZ_ERROR;
	}


	ws_conf = conf;

	return YMZ_OK;

}

ws_int32_t ws_conf_parse( ws_char_t* data, ws_int32_t len, ws_conf_t* conf )
{
	ws_int32_t				pos;
	ws_int32_t				n;
	ws_int32_t				cp;

	n = 0;
	cp = 0;
	pos = 0;

	enum{
		ST_START = 0,
		ST_EQ = 1,
		ST_VL = 2,
		ST_ED = 3,
	}state;

	state = ST_START;

	while( pos < len ){

		switch( state ){
		case ST_START:
			conf->item[ n ].name = data + pos;
			state = ST_EQ;
			cp = pos;
			break;
		case ST_EQ:
			if( data[ pos ] != '\=' ){
				break;
			}
			data[ pos ] = 0;
			conf->item[ n ].name_l = pos - cp;
			state = ST_VL;
			break;
		case ST_VL:
			conf->item[ n ].value = data + pos;
			cp = pos;
			state = ST_ED;
			break;
		case ST_ED:
			if( data[ pos ] != '\r' && data[ pos ] != '\n' && data[ pos ] != '\0' ){
				break;
			}
			switch( data[ pos ] ){
			case '\r':
			data[ pos ] = 0;
			conf->item[ n ].value_l = pos - cp;
			if( ++n >= YMZ_CONF_ITEM_LIMIT ){
				printf( "the conf is too many items\n" );
				return YMZ_ERROR;
			}
			++pos;
			state = ST_START;
			break;
			case '\n':
			data[ pos ] = 0;
			conf->item[ n ].value_l = pos - cp;
			if( ++n >= YMZ_CONF_ITEM_LIMIT ){
				printf( "the conf is too many items\n" );
				return YMZ_ERROR;
			}
			state = ST_START;
			break;
			case '\0':
			data[ pos ] = 0;
			conf->item[ n ].value_l = pos - cp;
			if( ++n >= YMZ_CONF_ITEM_LIMIT ){
				printf( "the conf is too many items\n" );
				return YMZ_ERROR;
			}
			state = ST_START;
			break;
			}
			break;
		default:
			return YMZ_ERROR;

		}
		++pos;
	}

	switch( state ){
	case ST_ED:
		++n;
	case ST_START:
		break;
	default:
		printf( "the conf has error\n" );
		abort();
		return YMZ_ERROR;
	}

	if( !n ){
		printf( "the conf has no item\n" );
		abort();
		return YMZ_ERROR;
	}

	conf->item_n = n;

	return YMZ_OK;
}


ws_int32_t ws_conf_get( const ws_char_t* name, const ws_char_t** value, ws_int32_t* len )
{
	ws_conf_item_t*					item;
	ws_int32_t								i;
	ws_int32_t								n;

	item = ws_conf->item;
	n = ws_conf->item_n;
	*len = 0;
	*value = 0;

	for( i = 0; i < n; ++i ){
		if( strncmp( name, item->name, item->name_l ) ){
			++item;
			continue;
		}

		*value = item->value;
		*len = item->value_l;
		return YMZ_OK;
	}

	return YMZ_ERROR;

}