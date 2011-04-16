#include <time.h>
#include <stdio.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#ifndef LASTFM_H
#define LASTFM_H

/* Audio scrobbler URL */
#define SCROBBLER_VERSION	"1.2.1"
#define SCROBBLER_ROOT		"http://post.audioscrobbler.com:80/"
#define API_ROOT		"http://ws.audioscrobbler.com/2.0/"

#define LASTFM_IMAGE_SIZES_COUNT	6
static const char LASTFM_IMAGE_SIZES[LASTFM_IMAGE_SIZES_COUNT][16] =	
		{"original", "mega", "extralarge","large","medium","small" };


enum LASTFM_PERIOD {
	LASTFM_PERIOD_7DAY=0,
	LASTFM_PERIOD_3MONTH,
	LASTFM_PERIOD_6MONTH,
	LASTFM_PERIOD_12MONTH,
	LASTFM_PERIOD_OVERALL,
	LASTFM_PERIOD_COUNT,
};

static const char LASTFM_PERIOD_STRINGS[LASTFM_PERIOD_COUNT][16] = 
	{ "7day", "3month", "6month", "12month","overall" };


enum LASTFM_STATUS_CODES{
	/* Everything is just fine and dandy */
	LASTFM_STATUS_OK=0,
	/* Fetch Error */
	LASTFM_STATUS_ERROR,
	/* Invalid argument/parameters supplied */
	LASTFM_STATUS_INVALID
};

/* Characters needed to store a 128bit MD5 string 
 * 1 char for every 4 bits + 1 for NULL terminator */
#define MD5_BUFFER		33	

/* Convenience sizes used internally */
#define SMALL_BUFFER		100
#define LARGE_BUFFER		1024

/*** liblastfm.so session handle ***/
/* Please note that all fields except 'status' and 'fraction' should be 
   considered private */
typedef struct {
	/* Curl handle */
	CURL *curl;

	/* The status of the last call to lastfm */
	char status[SMALL_BUFFER];

	/* Clear text username */
	char *username;
	/* MD5(password) */
	char password[MD5_BUFFER];
	
	/* Progress % (between 0.0 to 1.0), -1 means not doing anything */
	double fraction;
	
	/* MD5(username + MD5(password)) */
	char auth_token[MD5_BUFFER];
	/* Use it to tell last.fm who we are */
	char api_key[MD5_BUFFER];
	/* Secrets used to get auth.getSession */
	char secret[MD5_BUFFER];
	/* The very important user authorised session key */
	char session_key[MD5_BUFFER];

	/* Scrobbler session ID */
	char session_id[MD5_BUFFER];

	/* The URL that should be used for a now-playing request */
	char *playing_url;
	/* The URL that should be used for submissions */
	char *submission_url;
} LASTFM_SESSION;


/*** Album Information struct ***/
typedef struct {
        char            *name;          // Album name
        char            *artist;        // Artist name 
        char            *summary;       // Album summary
        char            *releasedate;   // Album release date 
	unsigned	playcount;	// Playcount if applicable
        unsigned char   *image;         // Actual image file (binary)
        size_t		image_size;     // Size in bytes
} LASTFM_ALBUM_INFO;

/*** Artist Information struct ***/
typedef struct {
	char		*name;		// Artist name
	char		*summary;	// Summary information
	unsigned	playcount;	// Playcount if applicable
	unsigned char	*image;		// Artist image (binary)
	size_t		image_size;	// Size in bytes of the image
	char		**similar;	// NULL terminated Array of null terminated similar artist name strings
} LASTFM_ARTIST_INFO;


/*** Track Information struct ***/
typedef struct {
	char		*name;		// Track title
	char		*artist;	// Artist name
	unsigned	playcount;	// Playcount
} LASTFM_TRACK_INFO;

typedef struct {
	unsigned char	*image;
	size_t		image_size;
	int		thumbs_up;
	int		thumbs_down;
	char		*title;
	long long	time;
} LASTFM_IMAGE_INFO;
	

/*********** liblastfm.so service API ***********/

/* init - Creates a new lastfm session 
 *
 * api_key = A 32 character (+1 null) string representing your API Key
 * secret = A 32 character (+1 null) string representing your API Secret 
 * Return: A new liblastfm.so session handle. Dinitialise with LASTFM_dinit() */
LASTFM_SESSION *LASTFM_init(const char *api_key, const char *secret);

/* login - Authenticates a user lastfm session 
 *
 * user = Null terminated lastfm username string
 * pass = Null terminated lastfm password string (clear text)
 * Return: 0 on success, non-zero otherwise */
int LASTFM_login(LASTFM_SESSION *s, const char *user,const char *pass);

/* login - Authenticates a user lastfm session 
 *
 * user = Null terminated lastfm username string
 * pass = Null terminated md5 string of a lastfm password
 * Return: 0 on success, non-zero otherwise */
int LASTFM_login_MD5(LASTFM_SESSION *s,const char *user,const char *pass_hash);

/* dinit - Shutdowns and deallocates memory of an existing lastfm session
 *
 * s = LASTFM_SESSION handle
 * Return: 0 on success, non-zero otherwise */
int	LASTFM_dinit(LASTFM_SESSION *s);

/* print_session - Prints out session information 
 *
 * out = open FILE handle with write permission
 * s = LASTFM_SESSION handle */
void	LASTFM_print_session(FILE *out, LASTFM_SESSION *s);

/* print_album_info - Prints out a LASTFM_ALBUM_INFO struct
 *
 * out = open FILE handle with write permission
 * s = LASTFM_SESSION handle */
void    LASTFM_print_album_info(FILE *out, LASTFM_ALBUM_INFO *a);

/* print_artist_info - Prints out a LASTFM_ALBUM_INFO struct
 * out = open FILE handle with write permission
 * s = LASTFM_SESSION handle */
void LASTFM_print_artist_info(FILE *out, LASTFM_ARTIST_INFO *a);

/* free_artist_info - Deallocates a LASTFM_ARTIST_INFO struct
 * a = A LASTFM_ARTIST_INFO struct to deallocate */
int LASTFM_free_artist_info(LASTFM_ARTIST_INFO *a);

/* free_artist_info_array - 
 * Deallocates a Null-terminated arrayof LASTFM_ARTIST_INFO strcuts.
 * a = The null terminated array */
int LASTFM_free_artist_info_array(LASTFM_ARTIST_INFO **a);

/* Same as the above three but for LASTFM_TRACK_INFO structs */
int LASTFM_free_track_info(LASTFM_TRACK_INFO *a);
int LASTFM_free_track_info_array(LASTFM_TRACK_INFO **a);
void LASTFM_print_track_info(FILE *out, LASTFM_TRACK_INFO *a);

/* free_image_info_array - Frees memory allocated by LASTFM_artist_get_images
 * a = A null terminated LASTFM_IMAGE_INFO array */
int LASTFM_free_image_info_array(LASTFM_IMAGE_INFO **a);

void LASTFM_print_image_info_array(FILE *out, LASTFM_IMAGE_INFO **a);

/* free_album_info - Deallocates a LASTFM_ALBUM_INFO struct
 * a = A LASTFM_ALBUM_INFO struct to deallocate */
int LASTFM_free_album_info(LASTFM_ALBUM_INFO *a);

/* free_album_info_array - Deallocates a Null-terminated array 
 * of LASTFM_ALBUM_INFO strcuts.
 * a = The null terminated array */
int LASTFM_free_album_info_array(LASTFM_ALBUM_INFO **a);
/*********** Last.fm Web API Methods ***********/

/*** Album API ***/

/* Album.getInfo 
 * s = LASTFM_SESSION handle created with LASTFM_init
 * artist = Null terminated Artist name string
 * album = Null terminated Album name string
 * Return: A newly allocated LASTFM_ALBUM_INFO struct. 
 *         Free with LASTFM_free_album_info  */
LASTFM_ALBUM_INFO *LASTFM_album_get_info(LASTFM_SESSION *s,
        const char *artist, const char *album);

/* Album.search
 * s = LASTFM_SESSION handle created with LASTFM_init
 * album = Null terminated Album name string
 * images = Flag to download images '1' or not '0'
 * limit = Limit the maximum number of search results
 *         Pass 0 for default (maximum) 30 results.
 * Return: A newly allocated, null terminated array of 
 * 	   LASTFM_ALBUM_INFO structs. Note, this array
 *         may not contain all album_info fields.
 *         Free with LASTFM_free_album_info_array  */
LASTFM_ALBUM_INFO **LASTFM_album_search(LASTFM_SESSION *s,
	unsigned short images, unsigned short limit, const char *album);

/*** Artist API ***/

/* Artist.getInfo 
 * s = LASTFM_SESSION handle created with LASTFM_init
 * artist = Null terminated Artist name string
 * Return: A newly allocated LASTFM_ARTIST_INFO struct. 
 *         Free with LASTFM_free_artist_info  */
LASTFM_ARTIST_INFO *LASTFM_artist_get_info(LASTFM_SESSION *s, const char *artist);

/* Artist.getImages
 * s = LASTFM_SESSION handle created with LASTFM_init
 * artist = Null terminate Artist name string
 * max = Maximum number of images to get
 * Return: A newly allocated, null terminate array of LASTFM_IMAGE_INFO structs
 *         of size no larger than 'max'.
 *         Free with LASTFM_free_image_info_array */
LASTFM_IMAGE_INFO **LASTFM_artist_get_images(LASTFM_SESSION *s, const char *artist, 
	int max);

/*** User API ***/

/* user.shout
 * s = LASTFM_SESSION handle created with LASTFM_init
 * user = Null terminated username string of the shout recipient
 * msg = Null terminated shout message
 * Return: 0 on success, non-zero otherwise */
int LASTFM_user_shout(LASTFM_SESSION *s,const char *user,const char *msg);

/* user.getTopAlbums
 * s = LASTFM_SESSION handle created with LASTFM_init
 * user = Null terminated username string or NULL to use current user
 * period = See LASTFM_PERIOD_ values
 * Return: A newly allocated, null terminate array of LASTFM_ALBUM_INFO structs
 *         Free with LASTFM_free_album_info_array */
LASTFM_ALBUM_INFO **LASTFM_user_get_top_albums(LASTFM_SESSION *s,const char *user, 
	int period);

/* user.getTopArtists
 * s = LASTFM_SESSION handle created with LASTFM_init
 * user = Null terminated username string or NULL to use current user
 * period = See LASTFM_PERIOD_ values
 * Return: A newly allocated, null terminate array of LASTFM_ARTIST_INFO structs
 *         Free with LASTFM_free_artist_info_array */
LASTFM_ARTIST_INFO **LASTFM_user_get_top_artists(LASTFM_SESSION *s,const char *user,
	int period);

/* user.getTopArtists
 * s = LASTFM_SESSION handle created with LASTFM_init
 * user = Null terminated username string or NULL to use current user
 * period = See LASTFM_PERIOD_ values
 * Return: A newly allocated, null terminate array of LASTFM_TRACK_INFO structs
 *         Free with LASTFM_free_artist_info_array */
LASTFM_TRACK_INFO **LASTFM_user_get_top_tracks(LASTFM_SESSION *s,const char *user,
	int period);

/*** XXX DEPRECATED XXX Audio Scrobbler API v1.2 ***/

/* Hanshake - (first step in scrobbling)
 * s = LASTFM_SESSION handle created with LASTFM_init
 * client_id = Null terminated Last.fm supplied client identifier string
 * client_ver = Null terminated application version number string
 * Return: 0 on success, non-zero otherwise */
int LASTFM_scrobbler_handshake(	LASTFM_SESSION *s,char *client_id,
		char *client_ver) __attribute__ ((deprecated));

/* NowPlaying - (hint to what you are currently playing)
 * s = LASTFM_SESSION handle created with LASTFM_init
 * title = Null terminated song title string
 * album = Null terminated album name string
 * artist = Null terminated artist name string
 * length = Duration in seconds of the track
 * trackno = Track number on album
 * mbtrack_id = MusicBrainz Track ID */
int LASTFM_scrobbler_now_playing(LASTFM_SESSION *s,
		char *title,char *album, char *artist,
		unsigned int length, unsigned short trackno, 
		unsigned int mbtrack_id) __attribute__ ((deprecated));

/* Submission - (scrobbles the track)
 * s = LASTFM_SESSION handle created with LASTFM_init
 * title = Null terminated song title string
 * album = Null terminated album name string
 * artist = Null terminated artist name string
 * start = Unix epoch time of when the track started playing
 * length = Duration in seconds of the track
 * trackno = Track number on album
 * mbtrack_id = MusicBrainz Track ID */
int LASTFM_scrobbler_submit(LASTFM_SESSION *s,
		char *title, char *album, char *artist,
		time_t start,unsigned int length, unsigned int trackno,
		unsigned int mbtrack_id) __attribute__ ((deprecated));


/*** Track API ***/

/* track.updateNowPlaying - (hint to what you are currently playing)
 * s = LASTFM_SESSION handle created with LASTFM_init
 * title = Null terminated song title string
 * album = Null terminated album name string
 * artist = Null terminated artist name string
 * length = Duration in seconds of the track
 * trackno = Track number on album
 * mbtrack_id = MusicBrainz Track ID */
int LASTFM_track_update_now_playing(LASTFM_SESSION *s,
		char *title,char *album, char *artist,
		unsigned int length, unsigned short trackno, 
		unsigned int mbtrack_id);

/* track.scrobble - (scrobbles the track)
 s = LASTFM_SESSION handle created with LASTFM_init
 * title = Null terminated song title string
 * album = Null terminated album name string
 * artist = Null terminated artist name string
 * start = Unix epoch time of when the track started playing
 * length = Duration in seconds of the track
 * trackno = Track number on album
 * mbtrack_id = MusicBrainz Track ID */
int LASTFM_track_scrobble(LASTFM_SESSION *s,
		char *title, char *album, char *artist,
		time_t start_time,unsigned int length, unsigned int trackno,
		unsigned int mbtrack_id);

#endif
