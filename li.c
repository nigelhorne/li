/*
 * Author Nigel Horne: njh@bandsman.co.uk
 * Copyright (C) 1997-2016, Nigel Horne

 * Usage is subject to licence terms.
 * The licence terms of this software are as follows:
 * Personal single user, single computer use: GPL2
 * All other users (including Commercial, Charity, Educational, Government)
 * must apply in writing for a licence for use from Nigel Horne at the
 * above e-mail.

 * li:
 *	Find duplicate files, DOS and UNIX. The UNIX version will optionally
 *	change these duplicates into links.
 *		e.g. li -r0l /usr/bin
 * Usage:
 *	li [-i] [-v] [-l] [-r] [-0] [ -d dirname ] dirname....
 * Options:
 *	r:	recursive (look in subdirectories)
 *	v:	verbose
 *	l:	(* only) change file copies into links
 *	i:	(* only) ask before changing each file into a link
 *	0:	ignore empty files
 *	d:	directory list
 *
 * 1.1 30/4/97: added the '-i' flag
 * 1.2 25/6/99:	added the '-0' flag
 * 1.3 28/9/01: if a file is relinked there's no need to check it again
 * 1.4 19/10/01: wasn't comparing 0 length files if -0 not given
 * 1.5: 6/6/02: ported to MACOS
 * 1.6:	10/4/03: another check we don't compare a file with itself
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef	MSDOS
#include <vmemory.h>
#include <sys/dir.h>
#include <direct.h>
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <signal.h>
#else
#include <dirent.h>
#ifdef	M_XENIX
#include <prototypes.h>
#endif
#endif
#ifdef	__APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <string.h>
#include <stdlib.h>

#ifdef	FILENAME_MAX
#define	_MAX_DIR	FILENAME_MAX
#endif

#ifndef	_MAX_DIR
#define	_MAX_DIR	128
#endif

#if	defined(__STDC__) || defined(MSDOS)
#define	ANSI
#else
#define	const
#define	cdecl
#endif

#ifdef	__GNUC__
#define	cdecl	__attribute__ ((cdecl))
#endif

static	void	li();

#ifdef	MSDOS
static	int	vmstarted;
void	cdecl	catcher(void);
#endif

int
#ifdef	MSDOS
cmp(const char far *file1, const char *file2)
#else
cmp(file1, file2)
char *file1, *file2;
#endif
{
	register FILE *fp1, *fp2;
	register int c, d;
#ifdef	MSDOS
#ifndef	M_I86LM
	char filename[_MAX_FNAME];

	_fstrcpy(filename, file1);
	fp1 = fopen(filename, "rb");
#else	/*!M_I86LM*/
	fp1 = fopen(file1, "rb");
#endif
#else	/*!MSDOS */
	fp1 = fopen(file1, "r");
#endif

	if(fp1 == (FILE *)NULL) {
#if	defined(MSDOS) && !defined(M_I86LM)
		perror(filename);
#else
		perror(file1);
#endif
		exit(5);
	}
#ifdef	MSDOS
	fp2 = fopen(file2, "rb");
#else
	fp2 = fopen(file2, "r");
#endif
	if(fp2 == (FILE *)NULL) {
		fclose(fp1);
		perror(file2);
		exit(6);
	}
	do {
		c = getc(fp1);
		d = getc(fp2);
		if(c != d) {
			fclose(fp1);
			fclose(fp2);
			return(1);
		}
	} while(c != EOF);

	fclose(fp1);
	fclose(fp2);
	return(0);
}

#ifdef	MSDOS

/*#define NULL	0
#define EOF	(-1)*/
#define ERR(s, c)	if(opterr){\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	(void) write(2, s, (unsigned)strlen(s));\
	(void) write(2, errbuf, 2);}

int	opterr = 1;
int	optind = 1;
int	optopt = 0;
const	char	*optarg = 0;

int
getopt(argc, argv, opts)
int argc;
const char **argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ",c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ",c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

void cdecl
catcher(void)
{
	if(vmstarted)
		_vheapterm();
	exit(SIGINT);
}
#endif	/*MSDOS*/

#if	__i386__
cdecl
#endif
main(argc, argv)
const char **argv;
{
	register const char *dirname = "";
	register int i;
	register int lflag = 0;
	register int vflag = 0;
	register int rflag = 0;
	register int iflag = 0;
	register int zeroflag = 0;
#ifdef	__BEOS__
	extern char *optarg;
#else
	extern const char *optarg;
#endif
	extern int optind;
#ifdef	MSDOS
	char olddir[_MAX_DIR];

	_getcwd(olddir, sizeof(olddir));
#endif

#if	defined(MSDOS) || defined(__BEOS__)
	while((i = getopt(argc, argv, "d:rv0")) != EOF)
#else
	while((i = getopt(argc, argv, "d:ilrv0")) != EOF)
#endif
		switch(i) {
#if	!defined(MSDOS) && !defined(__BEOS__)
			case 'l':
				lflag++;
				break;
			case 'i':
				iflag++;
				break;
#endif
			case 'v':
				vflag++;
				break;
			case 'd':
				dirname = optarg;
				break;
			case 'r':
				rflag++;
				break;
			case '0':
				zeroflag++;
				break;
			case '?':
#if	defined(MSDOS)
				cprintf("Usage: %s [-v] [-d dirname] [-r] [-0]\r\n", argv[0]);
#elif	defined(__BEOS)
				fprintf(stderr, "Usage: %s [-v] [-r] [ -d dirname ] [-0]\n", argv[0]);
#else
				fprintf(stderr, "Usage: %s [-i] [-v] [-l] [-r] [ -d dirname ] [-0]\n", argv[0]);
#endif
				return(1);
		}

#ifdef	MSDOS
	signal(SIGINT, catcher);
#endif
	if(*dirname)
		li(dirname, lflag, vflag, rflag, iflag, zeroflag);
	else if(optind == argc)
		li(".", lflag, vflag, rflag, iflag, zeroflag);
	else
		do
			li(argv[optind], lflag, vflag, rflag, iflag, zeroflag);
		while(++optind < argc);

#ifdef	MSDOS
	if(vmstarted)
		_vheapterm();

	if(*dirname)
		chdir(olddir);
#endif
	return(0);
}

static void
li(dirname, lflag, vflag, rflag, iflag, zeroflag)
const char *dirname;
{
#ifdef	MSDOS
	register struct direct *dirent;
	register char far *buffer;
	_vmhnd_t vhandle;
#else
	register struct dirent *dirent;
#ifndef	__BEOS__
	register int doit;
#endif
#endif
	struct stat statb;
	register DIR *dirp;
	char filename[_MAX_DIR];
	static struct data {
#ifndef	MSDOS
		ino_t	inum;
		mode_t	mode;
#endif
		off_t	size;
#ifdef	MSDOS
		_vmhnd_t	v_name;
#else
		const	char	*name;
#endif
		struct	data	*next;
	} *top, *last;
	register struct data *item;

	if(vflag)
		printf("checking %s\n", dirname);

	dirp = opendir(dirname);
	if(dirp == (DIR *)NULL) {
		perror(dirname);
		return;
	}

#ifdef	MSDOS
	if(!vmstarted) {
		if(!_vheapinit(0, (unsigned int)1280, _VM_DISK|_VM_EMS|_VM_XMS)) {
printf("%d: ", __LINE__);
			puts("No more core");
			closedir(dirp);
			return;
		}
		vmstarted = 1;
	}
#endif
	if(strcmp(dirname, "/") == 0)
		dirname = "";

	while((dirent = readdir(dirp)) != NULL) {
#ifdef	__APPLE__
		/* Ignore the system files held by MAC/OS */
		if(dirent->d_name[0] == '.')
			continue;
#else
		if((strcmp(dirent->d_name, "..") == 0) ||
		   (strcmp(dirent->d_name, ".") == 0))
			continue;
#endif
		sprintf(filename, "%s/%s", dirname, dirent->d_name);
#ifdef	S_IFLNK
		if(lstat(filename, &statb) == -1) {
#else
		if(stat(filename, &statb) == -1) {
#endif
			perror(dirent->d_name);
			break;
		}
		if(rflag && ((statb.st_mode&S_IFMT) == S_IFDIR)) {
			li(filename, lflag, vflag, rflag, iflag, zeroflag);
			continue;
		}
		if((statb.st_mode&S_IFMT) != S_IFREG)
			continue;
#ifdef	S_IFLNK
		if((statb.st_mode&S_IFLNK) == S_IFLNK)
			continue;
#endif
		for(item = top; item; item = item->next) {
			if(statb.st_size != item->size)
				continue;
			if(zeroflag && (statb.st_size == 0))
				continue;
#ifdef	MSDOS
			buffer = _vload(item->v_name, _VM_CLEAN);
			if(cmp(buffer, filename) == 0)
				printf("%Fs and %s are identical\n",
					buffer, filename);
#else
			if(statb.st_ino == item->inum)
				continue;
			if(strcmp(item->name, filename) == 0)	/* 1.6 */
				continue;
			if(cmp(item->name, filename) == 0) {
				printf("%s and %s %s",
					item->name, filename,
					(statb.st_mode == item->mode) ? "are identical" : "differ only in mode");
				if(statb.st_mode != item->mode) {
					putchar('\n');
					continue;
				}

				if(!iflag)
					putchar('\n');

#ifndef	__BEOS__
				if(lflag) {
					if(iflag) {
						putchar('?');
						doit = 0;
						switch(getchar()) {
							case '\n':
								break;
							case 'y':
							case 'Y':
								doit = 1;
								/* fall through */
							default:
								while(getchar() != '\n')
									;
						}
					} else
						doit = 1;

					if(doit) {
						if(unlink(filename) < 0)
							perror(filename);
						else if(link(item->name, filename) < 0)
							perror(dirent->d_name);
						else
							/*
							 * No need to check it
							 * against anything else
							 */
							break;
					}
				}
#endif	/*__BEOS__*/
			}
#endif
		}
		if(last == NULL)
			top = last = malloc(sizeof(struct data));
		else {
			last->next = malloc(sizeof(struct data));
			last = last->next;
		}
		last->next = NULL;
		last->size = statb.st_size;
#ifdef	MSDOS
		last->v_name = vhandle = _vmalloc(strlen(filename) + 1);
		if(vhandle == _VM_NULL) {
			puts("No more core");
			break;
		}
		buffer = (char far *)_vload(vhandle, _VM_DIRTY);
		_fstrcpy(buffer, (char far *)filename);
#else
		last->inum = statb.st_ino;
		last->mode = statb.st_mode;
		if((last->name = strdup(filename)) == NULL) {
			puts("No more core");
			break;
		}
#endif
	}
	closedir(dirp);
}

#ifdef	MSDOS
#ifndef	NULL
#define	NULL	0
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	255
#endif

#define	A_RONLY		0x01
#define	A_HIDDEN	0x02
#define	A_SYSTEM	0x04
#define	A_LABEL		0x08
#define	A_DIR		0x10
#define	A_ARCHIVE	0x20

#define	DOSI_FINDF	0x4e
#define	DOSI_FINDN	0x4f
#define	DOSI_SDTA	0x1a

#define	Newisnull(a, t)	((a = (t *)malloc(sizeof(t))) == (t *)NULL)
#define	ATTRIBUTES	(A_DIR | A_HIDDEN | A_SYSTEM)

typedef struct {
	char d_buf[21];
	char d_attribute;
	unsigned short d_time;
	unsigned short d_date;
	long d_size;
	char d_name[13];
} Dta_buf;

static	char	*getdirent();
static	void	setdta();
static	void	free_dircontents();

static	Dta_buf	dtabuf;
static	Dta_buf	*dtapnt = &dtabuf;
static	union REGS	reg, nreg;

static	void	free_dircontents(struct _dircontents *);
static	char	*getdirent(char *);
static	void	setdta(void);

#if	defined(M_I86LM)
static	struct SREGS	sreg;
#endif

DIR *
opendir(name)
const char *name;
{
	register DIR *dirp;
	register struct _dircontents *dp;
	char c, *s;
	char nbuf[MAXPATHLEN + 1];
	struct stat statb;

	if((stat(name, &statb) < 0) || ((statb.st_mode & S_IFMT) != S_IFDIR))
		return((DIR *)NULL);
	if(Newisnull(dirp, DIR))
		return((DIR *)NULL);
	if(*name && (c = name[strlen(name) - 1]) != '\\' && c != '/')
		(void)strcat(strcpy(nbuf, name), "\\*.*");
	else
		(void)strcat(strcpy(nbuf, name), "*.*");
	dirp->dd_loc = 0;
	setdta();
	dirp->dd_contents = dirp->dd_cp = (struct _dircontents *)NULL;

	if((s = getdirent(nbuf)) == (char *)NULL)
		return(dirp);
	do {
		if(Newisnull(dp, struct _dircontents) ||
		  (dp->_d_entry = malloc((unsigned)(strlen(s) + 1))) == (char *)NULL) {
			if(dp)
				free((char *)dp);
			free_dircontents(dirp->dd_contents);
			return((DIR *)NULL);
		}
		if(dirp->dd_contents)
			dirp->dd_cp = dirp->dd_cp->_d_next = dp;
		else
			dirp->dd_contents = dirp->dd_cp = dp;
		(void)strcpy(dp->_d_entry, s);
		dp->_d_next = (struct _dircontents *)NULL;
	} while ((s = getdirent((char *)NULL)) != (char *)NULL);
	dirp->dd_cp = dirp->dd_contents;

	return(dirp);
}

void
closedir(dirp)
DIR *dirp;
{
	free_dircontents(dirp->dd_contents);
	free((char *)dirp);
}

void
seekdir(dirp, off)
DIR *dirp;
long off;
{
	register struct _dircontents *dp;
	long i = off;

	if(off < 0L)
		return;
	for(dp = dirp->dd_contents; (--i >= 0) && dp; dp = dp->_d_next)
		;
	dirp->dd_loc = off - (i + 1);
	dirp->dd_cp = dp;
}

struct direct *
readdir(dirp)
DIR *dirp;
{
	static struct direct dp;

	if(dirp->dd_cp == (struct _dircontents *)NULL)
		return((struct direct *)NULL);
	dp.d_namlen = dp.d_reclen =
		(ino_t)strlen(strcpy(dp.d_name, dirp->dd_cp->_d_entry));
	dp.d_ino = 0;
	dirp->dd_cp = dirp->dd_cp->_d_next;
	dirp->dd_loc++;
	return(&dp);
}

static void
free_dircontents(dp)
struct _dircontents *dp;
{
	register struct _dircontents *odp;

	while(dp) {
		if(dp->_d_entry)
			free(dp->_d_entry);
		dp = (odp = dp)->_d_next;
		free((char *)odp);
	}
}

static char *
getdirent(dir)
char *dir;
{
	if(dir != (char *)NULL) {	/* get first entry */
		reg.h.ah = DOSI_FINDF;
		reg.h.cl = ATTRIBUTES;
#if	defined(M_I86LM)
		reg.x.dx = FP_OFF(dir);
		sreg.ds = FP_SEG(dir);
#else
		reg.x.dx = (unsigned) dir;
#endif
	} else {			/* get next entry */
		reg.h.ah = DOSI_FINDN;
#if	defined(M_I86LM)
		reg.x.dx = FP_OFF(dtapnt);
		sreg.ds = FP_SEG(dtapnt);
#else
		reg.x.dx = (unsigned)dtapnt;
#endif
	}
#if	defined(M_I86LM)
	intdosx(&reg, &nreg, &sreg);
#else
	intdos(&reg, &nreg);
#endif
	if(nreg.x.cflag)
		return((char *)NULL);
	return(dtabuf.d_name);
}

static void
setdta()
{
	reg.h.ah = DOSI_SDTA;
#if	defined(M_I86LM)
	reg.x.dx = FP_OFF(dtapnt);
	sreg.ds = FP_SEG(dtapnt);
	intdosx(&reg, &nreg, &sreg);
#else
	reg.x.dx = (int)dtapnt;
	intdos(&reg, &nreg);
#endif
}
#endif	/*MSDOS*/
