/*
** Copyright (C) 2008-2010 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#ifndef __MU_UTIL_H__
#define __MU_UTIL_H__

#include <glib.h>
#include <dirent.h>

G_BEGIN_DECLS

/**
 * do system-specific initialization. should be called before anything
 * else. Initializes the locale and Gtype. Note: this function is
 * called by mu_runtime_init.
 * 
 * @return TRUE if is succeeds, FALSE otherwise
 */
gboolean mu_util_init_system (void);

/**
 * get the expanded path; ie. perform shell expansion on the path. the
 * path does not have to exist
 *
 * @param path path to expand
 * 
 * @return the expanded path as a newly allocated string, or NULL in
 * case of error
 */
char* mu_util_dir_expand (const char* path)
    G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

/**
 * guess the maildir; first try $MAILDIR; if it is unset or
 * non-existant, try ~/Maildir if both fail, return NULL
 * 
 * @return full path of the guessed Maildir, or NULL; must be freed (gfree)
 */
char* mu_util_guess_maildir (void)
   G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;


/**
 * guess the place of the mu homedir (typically, ~/.mu). Note, this
 * directory does not necessarily exist. mu_util_check_dir can be use
 * to check that
 *  
 * @return the guessed mu homedir, which needs to be freed with g_free
 * when no longer needed.
 */
gchar* mu_util_guess_mu_homedir (void)
    G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

/**
 * if path exists, check that's a read/writeable dir; otherwise try to
 * create it (with perms 0700)
 * 
 * @param path path to the dir 
 * 
 * @return TRUE if a read/writeable directory `path' exists after
 * leaving this function, FALSE otherwise
 */
gboolean mu_util_create_dir_maybe (const gchar *path)
    G_GNUC_WARN_UNUSED_RESULT;


/**
 * check whether path is a directory, and optionally, if it's readable
 * and/or writeable
 *  
 * @param path dir path
 * @param readable check for readability
 * @param writeable check for writability
 * 
 * @return TRUE if dir exist and has the specified properties
 */
gboolean mu_util_check_dir (const gchar* path, gboolean readable,
			    gboolean writeable)
    G_GNUC_WARN_UNUSED_RESULT;



/**
 * create a writeable file and return its file descriptor (which
 * you'll need to close(2) when done with it.)
 * 
 * @param filename the filename
 * @param dir the target directory, or NULL for the current
 * @param overwrite should we allow for overwriting existing files?
 * 
 * @return a file descriptor, or -1 in case of error. If it's a fily
 * system error, 'errno' may have more info. use 'close()' when done
 * with the file descriptor
 */
int mu_util_create_writeable_fd (const char* filename, const char* dir,
				 gboolean overwrite)
    G_GNUC_WARN_UNUSED_RESULT;

/**
 * convert a string array in to a string, with the elements separated
 * by ' '
 * 
 * @param params a non-NULL, NULL-terminated string array
 * 
 * @return a newly allocated string
 */
gchar* mu_util_str_from_strv (const gchar **params)
        G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

/*
 * for OSs with out support for direntry->d_type, like Solaris
 */
#ifndef DT_UNKNOWN
enum {
	DT_UNKNOWN = 0,
# define DT_UNKNOWN     DT_UNKNOWN
	DT_FIFO = 1,
# define DT_FIFO        DT_FIFO
	DT_CHR = 2,
# define DT_CHR         DT_CHR
	DT_DIR = 4,
# define DT_DIR         DT_DIR
	DT_BLK = 6,
# define DT_BLK         DT_BLK
	DT_REG = 8,
# define DT_REG         DT_REG
	DT_LNK = 10,
# define DT_LNK         DT_LNK
	DT_SOCK = 12,
# define DT_SOCK        DT_SOCK
	DT_WHT = 14
# define DT_WHT         DT_WHT
};
#endif /*DT_UNKNOWN*/


/**
 * get the d_type (as in direntry->d_type) for the file at path, using
 * lstat(3)
 * 
 * @param path full path
 * 
 * @return DT_REG, DT_DIR, DT_LNK, or DT_UNKNOWN (other values are not
 * supported currently )
 */
unsigned char mu_util_get_dtype_with_lstat (const char *path);


/**
 * 
 * don't repeat these catch blocks everywhere...
 * 
 */
#define MU_XAPIAN_CATCH_BLOCK						\
	catch (const Xapian::Error &xerr) {				\
                g_critical ("%s: caught xapian exception '%s'",		\
			__FUNCTION__, xerr.get_msg().c_str());		\
        } catch (...) {							\
                g_critical ("%s: caught exception", __FUNCTION__);	\
        }

#define MU_XAPIAN_CATCH_BLOCK_G_ERROR(GE,E)				\
	catch (const Xapian::Error &xerr) {				\
		g_set_error ((GE),0,(E),				\
			     "%s: caught xapian exception '%s'",	\
			__FUNCTION__, xerr.get_msg().c_str());		\
        } catch (...) {							\
		g_set_error ((GE),0,(MU_ERROR_INTERNAL),		\
			     "%s: caught exception", __FUNCTION__);	\
        }



#define MU_XAPIAN_CATCH_BLOCK_RETURN(R)					\
	catch (const Xapian::Error &xerr) {				\
                g_critical ("%s: caught xapian exception '%s'",		\
			   __FUNCTION__, xerr.get_msg().c_str());	\
		return (R);						\
        } catch (...) {							\
                g_critical ("%s: caught exception", __FUNCTION__);	\
		return (R);						\
        }

#define MU_XAPIAN_CATCH_BLOCK_G_ERROR_RETURN(GE,E,R)			\
	catch (const Xapian::Error &xerr) {				\
		g_set_error ((GE),0,(E),				\
			     "%s: caught xapian exception '%s'",	\
			   __FUNCTION__, xerr.get_msg().c_str());	\
		return (R);						\
        } catch (...) {							\
		g_set_error ((GE),0,(MU_ERROR_INTERNAL),		\
			     "%s: caught exception", __FUNCTION__);	\
		return (R);						\
        }


/* the name of the (leaf) dir which has the xapian database */
#define MU_XAPIAN_DIR_NAME    "xapian"
#define MU_XAPIAN_VERSION_KEY "db_version"

/* name of the bookmark file */
#define MU_BOOKMARK_FILENAME "bookmarks"

/**
 * log something in the log file; note, we use G_LOG_LEVEL_INFO
 * for such messages
 */
#define MU_WRITE_LOG(...)					     \
	G_STMT_START {						     \
		g_log (G_LOG_DOMAIN,				     \
		       G_LOG_LEVEL_INFO,			     \
		       __VA_ARGS__);					     \
	} G_STMT_END

G_END_DECLS

#endif /*__MU_UTIL_H__*/
