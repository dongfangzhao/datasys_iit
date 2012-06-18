/**
 * param.h
 * 
 * Desc: SCHFS header file
 * Author: DFZ
 * Last updated : 1:59 AM 3/13/2012
 */

// There are a couple of symbols that need to be #defined before
// #including all the headers.

#ifndef _PARAMS_H_
#define _PARAMS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 28
#define FUSE_USE_VERSION 28

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

// just some handy numbers
#define ONEG (1<<30)
#define ONEK 1024
#define ONEM (1<<20)

// ===================
// IMPORTANT NOTE:
// 		CHANGE THE FOLLOWING BEFORE DEPLOYMENT!!!!!!!!
// ===================
#define ROOTSYMSIZE 4096 //ssd mount point size; not in use now.
#define SSD_TOT (ONEG) //the threshold; <= SSD_Capacity - Max_File_size
#define MODE_LRU 1 //LRU caching by default
#define LOG_OFF 1 //turn off the log; faster your system!
#define CHKSSD_POSIX 0 //use command line to check SSD usage, seems to have performance degradatoin. Not in use now.
// ======OK you are all set============

// default source folders
#define DEFAULT_SSD "/ssd"
#define DEFAULT_HDD "/hdd"

// default mount points
#define SSD_MOUNT "ssd"
#define HDD_MOUNT "hdd"

// maintain schfs state in here
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>

#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

// schfs extentions:
#include <search.h>
#include <time.h>
#include <ftw.h>

/**
 * Data structures
 */

// inode info for files
typedef struct _inode_t 
{
	//element pointers
	struct _inode_t *next;
	struct _inode_t *prev;
	
	char fname[NAME_MAX]; //file name, absolute path, unique
	int inSSD; // I'm still thinking if this's necessary
	int freq; //for LFU
	time_t atime; //last access time

} 
inode_t;

// System state
struct schfs_state 
{
    FILE *logfile; //log file handle
    char *rootdir; //schfs root mount
    char ssd[PATH_MAX]; //ssd mount point
    char hdd[PATH_MAX]; //hdd mount point
	int ssd_total; //total space of ssd
	int ssd_used; //space that has been used in ssd
	
	//LRU Q:
	inode_t *lru_head;
    inode_t *lru_tail;
    inode_t *lfu_head;
    inode_t *lfu_tail;
	
	//add more if needed	
};

/**
 * Useful macros
 */
#define SCHFS_DATA ((struct schfs_state *) fuse_get_context()->private_data)
#define SCH_SSD \
    (((struct schfs_state *) fuse_get_context()->private_data)->ssd)
#define SCH_HDD \
    (((struct schfs_state *) fuse_get_context()->private_data)->hdd)

/**
 * forward declaration
 */ 

// SSD utilities 
int ssd_is_full();
int ssd_is_full_2();
void get_ssd_path(char ssd_fpath[PATH_MAX], const char *hdd_fpath);
void get_hdd_path(char hdd_fpath[PATH_MAX], const char *ssd_fpath);
int move_file_ssd(const char *ssd_fpath);
int move_file_hdd(const char *hdd_fpath);
void copy_dir_ssd(const char *ssd_fpath, mode_t mode);
int get_used_ssd();
int sum_size_ssd(const char *name, const struct stat *sb, int type);
int is_symlink_ssd(const char *fpath_ssd);

// LRU utilities
void insque_lru(inode_t *elem);
void remque_lru();
void rmelem_lru(const char* fname);
inode_t* findelem_lru(const char *fname);

// LFU utilities
void insque_lfu(inode_t *elem);
void remque_lfu();
void rmelem_lfu(const char *fname);
inode_t* get_pos_lfu(inode_t *elem);
inode_t* findelem_lfu(const char *fname);

// Debug utilities
void print_lru();
void print_lfu();
void print_debug();
void print_used_ssd();
int schfs_error(char *str); 
 
#endif

