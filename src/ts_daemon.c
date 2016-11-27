#ifdef __linux__
#include <ws_std.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#ifdef __linux__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif
#include <ws_os.h>
#include <ws_cycle.h>
#include <ws_log.h>
#include <ws_daemon.h>
#include <ws_process.h>

ws_int32_t ws_create_shim();

ws_int32_t main( ws_int32_t argc, ws_char_t** argv )
{
	ws_cycle_t*		cycle;
	
    setbuf( stdout, 0 );
    
	
	if( ( cycle = ws_cycle_create() ) == 0 ){
		return YMZ_ERROR;
	}

	if( ws_cycle_daemon_init( cycle ) ){
		return YMZ_ERROR;
	}

	ws_log_init();
	
	if( ws_create_shim() ){
		return YMZ_ERROR;
	}


	ws_init_signals();
	ws_daemon();

	ws_os_daemon_process();


	return 0;


}

ws_int32_t ws_create_shim()
{
#ifdef __linux__
	ws_int32_t								fd;
	void*							shm;
	ws_int32_t								i;
	char							buffer[512];

#define YMZ_WORKER_SHARE_CRC_SIZE		1024

	fd = shm_open( ws_cycle->share_memory_name, O_RDWR,  0666 );
	if( fd == -1 ){
		fd = shm_open(ws_cycle->share_memory_name, O_CREAT | O_RDWR, 0666);
		if (fd == -1){
			return YMZ_ERROR;
		}

		ftruncate(fd, ws_cycle->share_memory_size);
		shm = mmap(0, YMZ_WORKER_SHARE_CRC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (!shm){
			return YMZ_ERROR;
		}

		memset(shm, 0, YMZ_WORKER_SHARE_CRC_SIZE);
	}

	

	for (i = 0; i < ws_cycle->app_reserve; ++i){
		sprintf(buffer, "%s_user_%d", ws_cycle->share_memory_name, i);
		fd = shm_open(buffer, O_RDWR, 0666);
		if (fd == -1){
			fd = shm_open(buffer, O_CREAT | O_RDWR, 0666);
			if (fd == -1){
				return YMZ_ERROR;
			}
			ftruncate(fd, ws_cycle->share_memory_size);
			shm = mmap(0, YMZ_WORKER_SHARE_CRC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (!shm){
				return YMZ_ERROR;
			}

			memset(shm, 0, YMZ_WORKER_SHARE_CRC_SIZE);
		}
		sprintf(buffer, "%s_ship_%d", ws_cycle->share_memory_name, i);
		fd = shm_open(buffer, O_RDWR, 0666);
		if (fd == -1){
			fd = shm_open(buffer, O_CREAT | O_RDWR, 0666);
			if (fd == -1){
				return YMZ_ERROR;
			}
			ftruncate(fd, ws_cycle->share_memory_size);
			shm = mmap(0, YMZ_WORKER_SHARE_CRC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (!shm){
				return YMZ_ERROR;
			}

			memset(shm, 0, YMZ_WORKER_SHARE_CRC_SIZE);
		}
	}

#endif
	return YMZ_OK;
	
}


#endif