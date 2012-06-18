/**
 * schfs.c
 *
 * Desc: Implementation of FUSE interfaces
 * Author: DFZ
 * Last update: 12:28 AM 3/13/2012
 *
 * Possible FUSE options when mounting: 
 *		-o direct_io 
 *		-o big_writes 
 *		-o large_read 
 *		-o max_read=131072 
 *		-o max_write=131072
 *		-s
 */ 

#include "params.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "log.h"

/** 
 * Log the error 
 */
int schfs_error(char *str) 
{
	int ret = -errno;

	log_msg("    ERROR %s: %s\n", str, strerror(errno));

	return ret;
}

/** 
 * Return the physical mount point given a path 
 *
 * Since we are using symbolic link between SSD and HDD, the fullpath
 * 		is always the absolute path on SSD
 */
static void schfs_fullpath( char fpath[PATH_MAX], const char *path) 
{
	strcpy(fpath, SCHFS_DATA->ssd);	//ssd is always the entry point
	strncat(fpath, path, PATH_MAX); // ridiculously long paths will break here

	log_msg("    schfs_fullpath:  ssd = \"%s\", path = \"%s\", fpath = \"%s\"\n",
		SCHFS_DATA->ssd, path, fpath);		
}

/** 
 * Get file attributes. 
 *
 * Given a file with (char *path), return the stat in the (struct stat*statbuf)
 *		buffer. If the file on SSD is a symbolic link and there's exactly a
 *		same path on HDD, then we should return the stat of the HDD file.
 *
 * This is one of the most frequently called functions. Try to make it 
 *		as efficient as possible.
 */
int schfs_getattr(const char *path, struct stat *statbuf) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
			path, statbuf);

	schfs_fullpath(fpath, path);

	retstat = lstat(fpath, statbuf);
	if (retstat != 0) 
	{
		retstat = schfs_error("schfs_getattr lstat");
	}	
	
	//get the attribute of HDD file if there is one
	//		This can also be done by the new function is_symlink_ssd()
	char fname_hdd[PATH_MAX];	
	get_hdd_path(fname_hdd, fpath);
	if (S_ISLNK(statbuf->st_mode) 
		&& access(fname_hdd, F_OK) != -1) 
	{
		retstat = lstat(fname_hdd, statbuf);
		if (retstat != 0) 
		{
			retstat = schfs_error("schfs_getattr lstat");	
		}			
	}
	
	log_stat(statbuf);

	//print some debug information
	log_msg("\n============ DFZ DEBUG ==============\n");
	print_debug();	
	
	return retstat;
}

/** 
 * Read the target of a symbolic link 
 *
 * (copied from fuls.h)
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character. If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 *
 * Since in Schfs we are hiding the SSD symbolic link from the users
 * this code should work fine without modification
 */
int schfs_readlink(const char *path, char *link, size_t size) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("schfs_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
			path, link, size);

	schfs_fullpath(fpath, path);

	retstat = readlink(fpath, link, size - 1);
	if (retstat < 0)
		retstat = schfs_error("schfs_readlink readlink");
	else 
	{
		link[retstat] = '\0';
		retstat = 0;
	}

	return retstat;
}

/** 
 * Create a file node 
 *
 * This one is no use; it's only called when _create() is undefined.
 */
int schfs_mknod(const char *path, mode_t mode, dev_t dev) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
			path, mode, dev);

	schfs_fullpath(fpath, path);

	// On Linux this could just be 'mknod(path, mode, rdev)' but this
	//  is more portable
	if (S_ISREG(mode)) 
	{
		retstat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (retstat < 0)
			retstat = schfs_error("schfs_mknod open");
		else 
		{
			retstat = close(retstat);
			if (retstat < 0)
				retstat = schfs_error("schfs_mknod close");
		}
	} 
	else if (S_ISFIFO(mode)) 
	{
		retstat = mkfifo(fpath, mode);
		if (retstat < 0)
			retstat = schfs_error("schfs_mknod mkfifo");
	} 
	else 
	{
		retstat = mknod(fpath, mode, dev);
		if (retstat < 0)
			retstat = schfs_error("schfs_mknod mknod");
	}

	return retstat;
}

/** 
 * Create a directory 
 *
 * We are synchronizing the directory trees between SSD and HDD.
 * It introduces space overhead, but only 4K for each direcotry...
 */
int schfs_mkdir(const char *path, mode_t mode) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_mkdir(path=\"%s\", mode=0%3o)\n",
			path, mode);

	schfs_fullpath(fpath, path);

	retstat = mkdir(fpath, mode);
	if (retstat < 0)
		retstat = schfs_error("schfs_mkdir mkdir");

	// We need to replicate this directory in hdd also:
	copy_dir_ssd(fpath, mode);
		
	return retstat;
}

/** 
 * Remove a file 
 *
 * If we happen to remove a SSD symbolic link we need to remove
 * the HDD file also
 */
int schfs_unlink(const char *path) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("schfs_unlink(path=\"%s\")\n",
			path);

	schfs_fullpath(fpath, path);
		
	// Need to unlink the file in hdd also, if applicable
	if (is_symlink_ssd(fpath)) 
	{
		char hdd_fpath[PATH_MAX];
		get_hdd_path(hdd_fpath, fpath);

		retstat = unlink(hdd_fpath);
		if (retstat < 0)
			retstat = schfs_error("schfs_unlink unlink hdd");		
	}

	// Update the cache Q:
	if (MODE_LRU) 
	{ 
		rmelem_lru(fpath);
	}
	else
	{
		rmelem_lfu(fpath);		
	}

	retstat = unlink(fpath);
	if (retstat < 0)
		retstat = schfs_error("schfs_unlink unlink ssd");
	
	// This is a key function, show debug info:
	log_msg("\n============ DFZ DEBUG ==============\n");
	print_debug();
	
	return retstat;
}

/** 
 * Remove a directory 
 *
 * Again, this needs to take care of the case where the SSD
 * file is a symbolic link. 
 */
int schfs_rmdir(const char *path) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("schfs_rmdir(path=\"%s\")\n",
			path);

	schfs_fullpath(fpath, path);

	retstat = rmdir(fpath);
	if (retstat < 0)
		retstat = schfs_error("schfs_rmdir rmdir ssd");
		
	// Need to remove dir in hdd
	// Note that we don't need to check if a directory is a symbolic link
	char hdd_fpath[PATH_MAX];
	get_hdd_path(hdd_fpath, fpath);
	
	retstat = rmdir(hdd_fpath);
	if (retstat < 0)
		retstat = schfs_error("schfs_rmdir rmdir hdd");	

	return retstat;
}

/** 
 * Create a symbolic link 
 *
 * It's ok to have link of link, but note that char *path is
 * where it points to, not the virtual path. Here's what BBFS
 * said:
 */ 
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int schfs_symlink(const char *path, const char *link) 
{
	int retstat = 0;
	char flink[PATH_MAX];

	log_msg("\nschfs_symlink(path=\"%s\", link=\"%s\")\n",
			path, link);

	schfs_fullpath(flink, link);

	retstat = symlink(path, flink);
	if (retstat < 0)
		retstat = schfs_error("schfs_symlink symlink");

	return retstat;
}

/** 
 * Rename a file 
 */
int schfs_rename(const char *path, const char *newpath) 
{
	int retstat = 0;
	char fpath[PATH_MAX];
	char fnewpath[PATH_MAX];

	log_msg("\nschfs_rename(fpath=\"%s\", newpath=\"%s\")\n",
			path, newpath);
	schfs_fullpath(fpath, path);
	schfs_fullpath(fnewpath, newpath);

	// If fpath is a symbolic link, we need to update the HDD file name
	if (is_symlink_ssd(fpath))
	{
		// Rename the hdd file
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
		
		char fnewpath_hdd[PATH_MAX];
		get_hdd_path(fnewpath_hdd, fnewpath);
		
		retstat = rename(fpath_hdd, fnewpath_hdd);
		if (retstat < 0)
			retstat = schfs_error("schfs_rename rename");
		
		// Relink the hdd file
		retstat = unlink(fnewpath);
		if (retstat < 0)
			retstat = schfs_error("schfs_rename rename");
		
		retstat = symlink(fnewpath_hdd, fnewpath);
		if (retstat < 0)
			retstat = schfs_error("schfs_rename rename");
	}
	// If fpath is not a symbolic link, it means fpath is in SSD which
	// indicates we should update the cache Q
	else
	{	
		// Need to update the cache Q
		if (MODE_LRU) 
		{
			inode_t *inode = findelem_lru(fpath);
			strcpy(inode->fname, fnewpath);
		}
		else
		{
			inode_t *inode = findelem_lfu(fpath);
			strcpy(inode->fname, fnewpath);		
		}		
	}

	retstat = rename(fpath, fnewpath);
	if (retstat < 0)
		retstat = schfs_error("schfs_rename rename");	
		
	return retstat;
}

/** 
 * Create a hard link to a file 
 *
 * It doesn't make sense to hard link a symbolic link,
 * so we need to take care of the HDD file.
 */
int schfs_link(const char *path, const char *newpath) 
{
	int retstat = 0;
	char fpath[PATH_MAX], fnewpath[PATH_MAX];

	log_msg("\nschfs_link(path=\"%s\", newpath=\"%s\")\n",
			path, newpath);
	schfs_fullpath(fpath, path);
	schfs_fullpath(fnewpath, newpath);

	// Clearly we don't want to hard link a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
		strcpy(fpath, fpath_hdd);	
	}

	retstat = link(fpath, fnewpath);
	if (retstat < 0)
		retstat = schfs_error("schfs_link link");

	return retstat;
}

/** 
 * Change the permission bits of a file 
 *
 * We need to change the permission of the underlying HDD file also
 */
int schfs_chmod(const char *path, mode_t mode) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_chmod(fpath=\"%s\", mode=0%03o)\n",
			path, mode);
	schfs_fullpath(fpath, path);

	// Update HDD file if SSD file is a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
			
		retstat = chmod(fpath_hdd, mode);
		if (retstat < 0)
			retstat = schfs_error("schfs_chmod chmod");			
	}

	retstat = chmod(fpath, mode);
	if (retstat < 0)
		retstat = schfs_error("schfs_chmod chmod");

	return retstat;
}

/** 
 * Change the owner and group of a file 
 *
 * We need to make sure the HDD file gets changed also
 */
int schfs_chown(const char *path, uid_t uid, gid_t gid) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_chown(path=\"%s\", uid=%d, gid=%d)\n",
			path, uid, gid);
	schfs_fullpath(fpath, path);

	// update HDD file if SSD file is a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
					
		retstat = chown(fpath_hdd, uid, gid);
		if (retstat < 0)
			retstat = schfs_error("schfs_chown chown");
	}

	retstat = chown(fpath, uid, gid);
	if (retstat < 0)
		retstat = schfs_error("schfs_chown chown");

	return retstat;
}

/** 
 * Change the size of a file 
 *
 * If it's a symbolic link we need to change the size of HDD file,
 * rather than the symblic link
 */
int schfs_truncate(const char *path, off_t newsize) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_truncate(path=\"%s\", newsize=%lld)\n",
			path, newsize);
	schfs_fullpath(fpath, path);

	//if it's a symbol link:
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
		strcpy(fpath, fpath_hdd);
	}

	retstat = truncate(fpath, newsize);
	if (retstat < 0)
		schfs_error("schfs_truncate truncate");

	return retstat;
}

/**
 * Change the access and/or modification times of a file
 *
 * If the SSD file is a symbolic link, we need to update the HDD file also
 */
int schfs_utime(const char *path, struct utimbuf *ubuf) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_utime(path=\"%s\", ubuf=0x%08x)\n",
			path, ubuf);
	schfs_fullpath(fpath, path);

	// Update HDD file if SSD file is a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
				
		retstat = utime(fpath_hdd, ubuf);
		if (retstat < 0)
			retstat = schfs_error("schfs_utime utime");		
	}

	retstat = utime(fpath, ubuf);
	if (retstat < 0)
		retstat = schfs_error("schfs_utime utime");

	return retstat;
}

/** 
 * File open operation 
 *
 * This is a VERY important function. It involves manipulating
 * the LRU Q and swapping between HDD and SSD
 */
int schfs_open(const char *path, struct fuse_file_info *fi) {
	int retstat = 0;
	int fd;
	char fpath[PATH_MAX];

	log_msg("\nschfs_open(path\"%s\", fi=0x%08x)\n",
			path, fi);
	schfs_fullpath(fpath, path);

	// If the requested file is in HDD, swap it from HDD to SSD	
	if (is_symlink_ssd(fpath))
	{		
		// If SSD usage is still too high, swap ssd->hdd
		if (MODE_LRU) 
		{
			while (ssd_is_full() && (SCHFS_DATA->lru_head != NULL)) 
			{
				// Move LRU Q head file to ssd
				move_file_ssd(SCHFS_DATA->lru_head->fname);
				
				// Update LRU
				remque_lru();
			}	
		}
		else
		{
			while (ssd_is_full() && (SCHFS_DATA->lfu_head != NULL)) 
			{
				// Move LFU Q head file to ssd
				move_file_ssd(SCHFS_DATA->lfu_head->fname);
				
				// Update LFU
				remque_lfu();
			}				
		}
			
		// And if the used SSD space is still too high, pop up a warning
		if (ssd_is_full())
		{
			log_msg("\n\t Warning: SSD space is close to limit. \n ");	
		}
		
		// Now we are ready to move the file from HDD to SSD
		move_file_hdd(fpath);		
	}
	
	// Now we can open it
	fd = open(fpath, fi->flags);
	if (fd < 0)
		retstat = schfs_error("schfs_open open");

	fi->fh = fd;
	log_fi(fi);

	// Need to update the LRU Q
	inode_t *elem = (inode_t *)malloc(sizeof(inode_t));
	strcpy(elem->fname, fpath);
	
	if (MODE_LRU) 
	{
		insque_lru(elem);
	}
	else
	{
		elem->freq = 1;
		insque_lfu(elem);	
	}
	
	return retstat;
}

/**
 * Read data from an open file
 *
 * Since the mapping between SSD and HDD is done in _open(),
 * we don't really need to do much here.
 */
int schfs_read(const char *path, char *buf, size_t size, off_t offset, 
				struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
			path, buf, size, offset, fi);

	// no need to get fpath on this one, since I work from fi->fh not the path
	log_fi(fi);

	retstat = pread(fi->fh, buf, size, offset);
	if (retstat < 0)
		retstat = schfs_error("schfs_read read");

	return retstat;
}

/**
 * Write data to an open file
 *
 * Since the mapping between SSD and HDD is done in _open(),
 * we don't really need to do much here. 
 */
int schfs_write(const char *path, const char *buf, size_t size, off_t offset,
				struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
			path, buf, size, offset, fi);
			
	// no need to get fpath on this one, since I work from fi->fh not the path
	log_fi(fi);

	retstat = pwrite(fi->fh, buf, size, offset);
	if (retstat < 0)
		retstat = schfs_error("schfs_write pwrite");

	return retstat;
}

/**
 * Get file system statistics
 *
 * If the SSD path is symbolic link, we want to map it to the HDD file
 */
int schfs_statfs(const char *path, struct statvfs *statv) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_statfs(path=\"%s\", statv=0x%08x)\n",
			path, statv);
			
	schfs_fullpath(fpath, path);

	// Bypass if the SSD file is a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
		strcpy(fpath, fpath_hdd);	
	}

	// get stats for underlying filesystem
	retstat = statvfs(fpath, statv);
	if (retstat < 0)
		retstat = schfs_error("schfs_statfs statvfs");

	log_statvfs(statv);

	return retstat;
}

/**
 * Possibly flush cached data
 *
 * From fuse.h document it seems this is just a post-processing
 * of close(). Ignored for now, go read the document if interested.
 */
int schfs_flush(const char *path, struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
	// no need to get fpath on this one, since I work from fi->fh not the path
	log_fi(fi);

	return retstat;
}

/** 
 * Release an open file
 *
 * This function is working with the file handler, and it should have been
 * taken care of by the _open() function. So no need to change specifically
 * for SSd symbolic link 
 */
int schfs_release(const char *path, struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_release(path=\"%s\", fi=0x%08x)\n",
			path, fi);
	log_fi(fi);

	// We need to close the file.  Had we allocated any resources
	// (buffers etc) we'd need to free them here as well.
	retstat = close(fi->fh);

	// This is a frequently called function, print debug here.
	log_msg("\n============ DFZ DEBUG ==============\n");
	//==================== Put debug info here =========================
	print_debug();

	return retstat;
}

/**
 * Synchronize file contents
 *
 * We don't want to fsync() a symbolic link
 */
int schfs_fsync(const char *path, int datasync, struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
			path, datasync, fi);
	log_fi(fi);

	// Need to check if it's in HDD or SSD
	char fpath_ssd[PATH_MAX];
	char fpath_hdd[PATH_MAX];
	schfs_fullpath(fpath_ssd, path);
	get_hdd_path(fpath_hdd, fpath_ssd);
	
	struct stat sb;
	if (lstat(fpath_ssd, &sb) == -1) 
	{
		log_msg("    lstat error line#%d", __LINE__);
	}
	
	if (S_ISLNK(sb.st_mode)
		&& (access(fpath_hdd, F_OK) != -1)) 
	{
		int fd = open(fpath_hdd, O_RDONLY);
		
		if (datasync)
			retstat = fdatasync(fd);
		else
			retstat = fsync(fd);
		
		close(fd);		
	}
	else 
	{
		if (datasync)
			retstat = fdatasync(fi->fh);
		else
			retstat = fsync(fi->fh);
	}
	
	if (retstat < 0)
		schfs_error("schfs_fsync fsync");

	return retstat;
}

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */ 

/** 
 * Set extended attributes 
 *
 * Need to check SSD symbolic links
 */
int schfs_setxattr(const char *path, const char *name, const char *value, 
					size_t size, int flags) 
{
	int retstat = 0;
	char fpath[PATH_MAX];
	struct stat sb;
	
	log_msg("\nschfs_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
			path, name, value, size, flags);
	schfs_fullpath(fpath, path);

	// Again, we need to redirect this to HDD is applicable
	char fpath_hdd[PATH_MAX];
	get_hdd_path(fpath_hdd, fpath);
	if (lstat(fpath, &sb) == -1) {
		log_msg("    stat error line#%d", __LINE__);
	}
	if (S_ISLNK(sb.st_mode)
		&& (access(fpath_hdd, F_OK) != -1)) 
	{
		retstat = lsetxattr(fpath_hdd, name, value, size, flags);
	}
	else 
	{
		retstat = lsetxattr(fpath, name, value, size, flags);
	}		

	if (retstat < 0)
		retstat = schfs_error("schfs_setxattr lsetxattr");

	return retstat;
}

/** 
 * Get extended attributes 
 */
int schfs_getxattr(const char *path, const char *name, char *value, size_t size) 
{
	int retstat = 0;
	char fpath[PATH_MAX];
	struct stat sb;

	log_msg("\nschfs_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
			path, name, value, size);
	schfs_fullpath(fpath, path);

	// Again, we need to redirect this to HDD is applicable
	char fpath_hdd[PATH_MAX];
	get_hdd_path(fpath_hdd, fpath);
	if (lstat(fpath, &sb) == -1) 
	{
		log_msg("    stat error line#%d", __LINE__);
	}
	if (S_ISLNK(sb.st_mode)
		&& (access(fpath_hdd, F_OK) != -1)) 
	{
		retstat = lgetxattr(fpath_hdd, name, value, size);
	}
	else 
	{
		retstat = lgetxattr(fpath, name, value, size);
	}	
	
	if (retstat < 0)
		retstat = schfs_error("schfs_getxattr lgetxattr");
	else
		log_msg("    value = \"%s\"\n", value);

	return retstat;
}

/** 
 * List extended attributes
 *
 * Need to check SSD symbolic links 
 */
int schfs_listxattr(const char *path, char *list, size_t size) 
{
	int retstat = 0;
	char fpath[PATH_MAX];
	char *ptr;

	log_msg("schfs_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
			path, list, size);

	schfs_fullpath(fpath, path);

	// Update HDD path if SSD path is a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
		strcpy(fpath, fpath_hdd);	
	}

	retstat = llistxattr(fpath, list, size);
	if (retstat < 0)
		retstat = schfs_error("schfs_listxattr llistxattr");

	log_msg("    returned attributes (length %d):\n", retstat);
	for (ptr = list; ptr < list + retstat; ptr += strlen(ptr) + 1)
		log_msg("    \"%s\"\n", ptr);

	return retstat;
}

/** 
 * Remove extended attributes
 *
 * Need to check SSD symbolic links 
 */
int schfs_removexattr(const char *path, const char *name) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_removexattr(path=\"%s\", name=\"%s\")\n",
			path, name);
	schfs_fullpath(fpath, path);

	//update HDD path if SSD path is a symbolic link
	if (is_symlink_ssd(fpath))
	{
		char fpath_hdd[PATH_MAX];
		get_hdd_path(fpath_hdd, fpath);
		strcpy(fpath, fpath_hdd);	
	}

	retstat = lremovexattr(fpath, name);
	if (retstat < 0)
		retstat = schfs_error("schfs_removexattr lrmovexattr");

	return retstat;
}
#endif /* HAVE_SETXATTR */

/** 
 * Open directory 
 *
 * Since it is dealing with the file handle, we do not need to 
 * manipulate the SSD symbolic link here.
 */
int schfs_opendir(const char *path, struct fuse_file_info *fi) 
{
	DIR *dp;
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_opendir(path=\"%s\", fi=0x%08x)\n",
			path, fi);
	schfs_fullpath(fpath, path);

	dp = opendir(fpath);
	if (dp == NULL)
		retstat = schfs_error("schfs_opendir opendir");

	fi->fh = (intptr_t) dp;

	log_fi(fi);

	return retstat;
}

/** 
 * Read directory 
 *
 * Similarly to _opendir(), this should work fine with SSD symbolic links
 */
int schfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, 
					off_t offset, struct fuse_file_info *fi) 
{
	int retstat = 0;
	DIR *dp;
	struct dirent *de;
	
	log_msg("\nschfs_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
			path, buf, filler, offset, fi);
	// once again, no need for fullpath -- but note that I need to cast fi->fh
	dp = (DIR *) (uintptr_t) fi->fh;

	de = readdir(dp);
	if (de == 0) 
	{
		retstat = schfs_error("schfs_readdir readdir");
		return retstat;
	}

	do {
		log_msg("calling filler with name %s\n", de->d_name);

		if (filler(buf, de->d_name, NULL, 0) != 0) 
		{
			log_msg("    ERROR schfs_readdir filler:  buffer full");
			return -ENOMEM;			
		}			

	} while ((de = readdir(dp)) != NULL);

	log_fi(fi);
		
	return retstat;
}

/** 
 * Release directory 
 *
 * This is nothing but getting out of the directory.
 */
int schfs_releasedir(const char *path, struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_releasedir(path=\"%s\", fi=0x%08x)\n",
			path, fi);
	log_fi(fi);

	closedir((DIR *) (uintptr_t) fi->fh);

	// This is one of the most frequently called functions
	log_msg("\n============ DFZ DEBUG ==============\n");
	//==================== Put debug info here =========================
	print_debug();

	return retstat;
}

/**
 * Synchronize Directory contents
 *
 * Sync a dir? Does this really happen in real world?
 */
int schfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
			path, datasync, fi);
	log_fi(fi);

	return retstat;
}

/**
 * Initialize filesystem
 */
void *schfs_init(struct fuse_conn_info *conn) 
{
	log_msg("\nschfs_init()\n");
	
	return SCHFS_DATA;
}

/** 
 * Clean up filesystem
 *
 * This is the very end of the system, and nothing interesting 
 */
void schfs_destroy(void *userdata) 
{
	log_msg("\nschfs_destroy(userdata=0x%08x)\n", userdata);
}

/** 
 * Check file access permissions 
 *
 * This seems only getting called on the root path '/', so far
 *
 * This should work OK on SSD/HDD since both versions share
 * the same mask
 *
 */
int schfs_access(const char *path, int mask) 
{
	int retstat = 0;
	char fpath[PATH_MAX];

	log_msg("\nschfs_access(path=\"%s\", mask=0%o)\n",
			path, mask);
	schfs_fullpath(fpath, path);

	retstat = access(fpath, mask);

	if (retstat < 0)
		retstat = schfs_error("schfs_access access");

	return retstat;
}
 
/**
 * Create and open a file 
 *
 * This is corelated to _open(), refer to that function also.
 */
int schfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) 
{
	int retstat = 0;
	char fpath[PATH_MAX];
	int fd;

	log_msg("\nschfs_create(path=\"%s\", mode=0%03o, fi=0x%08x)\n",
			path, mode, fi);
	schfs_fullpath(fpath, path);

	// Before creating the file, make sure the SSD has sufficient space
	if (MODE_LRU)
	{
		while (ssd_is_full() && (SCHFS_DATA->lru_head != NULL)) 
		{
			//move LRU Q head file to ssd
			move_file_ssd(SCHFS_DATA->lru_head->fname);
			
			//update LRU
			remque_lru();
		}		
	}
	else //LFU
	{
		while (ssd_is_full() && (SCHFS_DATA->lfu_head != NULL)) 
		{
			//move LFU Q head file to ssd
			move_file_ssd(SCHFS_DATA->lfu_head->fname);
			
			//update LFU
			remque_lfu();
		}			
	}
	
	// And if the used SSD space is still too high, pop up a warning
	if (ssd_is_full())
	{
		log_msg("\n\t Warning: SSD space is reaching limit. \n ");	
	}	
	
	// We always want to create the new file on SSD
	fd = creat(fpath, mode);
	if (fd < 0)
		retstat = schfs_error("schfs_create creat");

	fi->fh = fd;

	log_fi(fi);

	// A newly created file should be inserted into the Q
	inode_t *elem = (inode_t *)malloc(sizeof(inode_t));
	strcpy(elem->fname, fpath);
	
	if (MODE_LRU) 
	{
		insque_lru(elem);
	}
	else //LFU
	{
		elem->freq = 1; //initialized to 1
		insque_lfu(elem);
	}

	return retstat;
}

/**
 * Change the size of an open file
 *
 * The *fi and _open() should handle the symbolic issue,
 * so no change is needed for now
 */
int schfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
			path, offset, fi);
	log_fi(fi);

	retstat = ftruncate(fi->fh, offset);
	if (retstat < 0)
		retstat = schfs_error("schfs_ftruncate ftruncate");

	return retstat;
}

/**
 * Get attributes from an open file
 *
 * I am not sure if this function takes *fi from _open(), 
 * but I will just check the SSD sybolic link anyways.
 */
int schfs_fgetattr(const char *path, struct stat *statbuf, 
					struct fuse_file_info *fi) 
{
	int retstat = 0;

	log_msg("\nschfs_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
			path, statbuf, fi);
	log_fi(fi);

	retstat = fstat(fi->fh, statbuf);
	if (retstat < 0)
		retstat = schfs_error("schfs_fgetattr fstat");

	// Similarly to _getattr:
	// get the attribute of HDD file if there is one
	char fpath[PATH_MAX];
	schfs_fullpath(fpath, path);
	
	char fname_hdd[PATH_MAX];	
	get_hdd_path(fname_hdd, fpath);
	
	if (S_ISLNK(statbuf->st_mode) 
		&& access(fname_hdd, F_OK) != -1) 
	{
		retstat = lstat(fname_hdd, statbuf);
		if (retstat != 0) 
		{
			retstat = schfs_error("schfs_getattr lstat");	
		}			
	}	

	log_stat(statbuf);

	return retstat;
}

/**
 * Callback list
 */
struct fuse_operations schfs_oper = 
{
	.getattr = schfs_getattr,
	.readlink = schfs_readlink,
	// no .getdir -- that's deprecated
	.getdir = NULL,
	.mknod = schfs_mknod,
	.mkdir = schfs_mkdir,
	.unlink = schfs_unlink,
	.rmdir = schfs_rmdir,
	.symlink = schfs_symlink,
	.rename = schfs_rename,
	.link = schfs_link,
	.chmod = schfs_chmod,
	.chown = schfs_chown,
	.truncate = schfs_truncate,
	.utime = schfs_utime,
	.open = schfs_open,
	.read = schfs_read,
	.write = schfs_write,
	/** Just a placeholder, don't set */ // huh???
	.statfs = schfs_statfs,
	.flush = schfs_flush,
	.release = schfs_release,
	.fsync = schfs_fsync,
#ifdef HAVE_SETXATTR	
	.setxattr = schfs_setxattr,
	.getxattr = schfs_getxattr,
	.listxattr = schfs_listxattr,
	.removexattr = schfs_removexattr,
#endif	
	.opendir = schfs_opendir,
	.readdir = schfs_readdir,
	.releasedir = schfs_releasedir,
	.fsyncdir = schfs_fsyncdir,
	.init = schfs_init,
	.destroy = schfs_destroy,
	.access = schfs_access,
	.create = schfs_create,
	.ftruncate = schfs_ftruncate,
	.fgetattr = schfs_fgetattr
};

/**
 * Argument hint
 */
void schfs_usage() 
{
	fprintf(stderr, "usage:  schfs rootdir mountdir \n");
	abort();
}

int main(int argc, char *argv[]) 
{
	int i;
	int fuse_stat;
	struct schfs_state *state_data;

	if ((getuid() == 0) || (geteuid() == 0)) 
	{
		fprintf(stderr, "Running SCHFS as root opens unnacceptable security holes\n");
		return 1;
	}

	state_data = calloc(sizeof (struct schfs_state), 1);
	if (state_data == NULL) 
	{
		perror("main calloc");
		abort();
	}

	state_data->logfile = log_open();

	for (i = 1; (i < argc) && (argv[i][0] == '-'); i++)
		if (argv[i][1] == 'o') i++; // -o takes a parameter; need to
	// skip it too.  This doesn't
	// handle "squashed" parameters

	//check number of arguments
	if ((argc - i) != 2) 
	{
		schfs_usage();
	}		

	//store the root paths
	state_data->rootdir = realpath(argv[i], NULL);
	
	//store the ssd path
	strcpy(state_data->ssd, state_data->rootdir);
	strcat(state_data->ssd, DEFAULT_SSD);
	
	//store the hdd path
	strcpy(state_data->hdd, state_data->rootdir);
	strcat(state_data->hdd, DEFAULT_HDD);
	
	//setup other system initial statuses
	state_data->lru_head = NULL;
	state_data->lru_tail = NULL;
	state_data->lfu_head = NULL;
	state_data->lfu_tail = NULL;

	//reset arguments for standard FUSE main
	argv[i] = argv[i + 1];
	argc--;

	// Initilize some other values in the system state e.g. SSD capacity
	state_data->ssd_total = SSD_TOT;
	
	fprintf(stderr, "Schfs started successfully. \n");
	fuse_stat = fuse_main(argc, argv, &schfs_oper, state_data);
	fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

	return fuse_stat;
}