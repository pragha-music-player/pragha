/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009-2010 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "pragha.h"
#include "md5.h"

static struct {
	enum track_source src;
	gchar c;
} track_source_c[] = {
	{ USER_SOURCE, 'P' },
	{ BROADCAST_SOURCE, 'R' },
	{ RECO_SOURCE, 'E' },
	{ LASTFM_SOURCE, 'L'},
	{ UNKNOWN_SOURCE, 'U'}
};

/* TODO: Handle FAILED properly */
static void lastfm_handshake_error_handler(gchar *err_str, struct con_win *cwin)
{
	if (!g_ascii_strncasecmp(err_str, "BANNED", 6))
		g_warning("BANNED: Client is banned from the last.fm server");
	else if (!g_ascii_strncasecmp(err_str, "BADAUTH", 7))
		g_warning("BADAUTH: Authentication failure,"
			  "check username and password");
	else if (!g_ascii_strncasecmp(err_str, "BADTIME", 7))
		g_warning("BADTIME: Timestamp is skewed");
	else if (!g_ascii_strncasecmp(err_str, "FAILED", 6))
		g_warning("FAILED: Server outage ?");
}

static void lastfm_submit_error_handler(gchar *str, struct con_win *cwin)
{
	if (g_strrstr_len(str, 10, "BADSESSION")) {
		g_warning("BADSESSION: Session ID invalid");
		goto re_handshake;
	} else if (g_strrstr_len(str, 6, "FAILED")) {
		g_warning("FAILED: Submission failure");
		cwin->cstate->lastfm_hard_failure++;
	} else {
		g_warning("Unknown Last.fm error, submission failed");
		cwin->cstate->lastfm_hard_failure++;
	}

	if (cwin->cstate->lastfm_hard_failure < 3)
		return;

re_handshake:
	CDEBUG(DBG_INFO, "Re-establishing last.fm connection");
	lastfm_init_thread(cwin);
}

static size_t lastfm_handshake_cb(gpointer data, size_t size, size_t n,
				  gpointer user_data)
{
	struct con_win *cwin = (struct con_win *)user_data;
	gint i = 0;
	gchar **tokens;

	if (!size || !n)
		return 0;

	tokens = g_strsplit((const gchar *)data, "\n", 5);

	while (tokens[i]) {
		switch (i) {
		case 0:
			if (g_ascii_strncasecmp(tokens[i], "OK", 2)) {
				lastfm_handshake_error_handler(tokens[i], cwin);
				goto exit;
			}
			break;
		case 1:
			cwin->clastfm->session_id = g_strdup(tokens[i]);
			break;
		case 2:
			break;
		case 3:
			cwin->clastfm->submission_url = g_strdup(tokens[i]);
			break;
		default:
			break;
		}
		i++;
	}

	if (i < 4) {
		g_warning("Malformed last.fm handshake response");
		cwin->clastfm->state |= LASTFM_NOT_CONNECTED;
		goto exit;
	}

	CDEBUG(DBG_INFO, "Last.fm handshake succeeded");
	cwin->clastfm->state |= LASTFM_CONNECTED;
	cwin->cstate->lastfm_hard_failure = 0;

exit:
	g_strfreev(tokens);
	return (n * size);
}

static size_t lastfm_submission_cb(gpointer data, size_t size, size_t n,
				   gpointer user_data)
{
	struct con_win *cwin = (struct con_win *)user_data;

	if (!size || !n)
		return 0;

	if (g_strrstr_len((const gchar *)data, 2, "OK") == NULL) {
		lastfm_submit_error_handler((gchar *)data, cwin);
		goto exit;
	}

	if (cwin->cstate->lastfm_hard_failure)
		cwin->cstate->lastfm_hard_failure--;

	CDEBUG(DBG_INFO, "Last.fm submission succeeded");
exit:
	return (n * size);
}

gint lastfm_handshake(struct con_win *cwin)
{
	gchar *url, tstamp[64];
	gint i = 0, ret = 0;
	GTimeVal time;
	md5_state_t md5_s;
	md5_byte_t pass_digest[16], auth_digest[16];
	char pass_digest_hex[33], auth_digest_hex[33];

	if (!cwin->cpref->lw.lastfm_support)
		return 0;

	memset(tstamp, 0, sizeof(tstamp));
	memset(pass_digest, 0, sizeof(pass_digest));
	memset(auth_digest, 0, sizeof(auth_digest));
	memset(pass_digest_hex, 0, sizeof(pass_digest_hex));
	memset(auth_digest_hex, 0, sizeof(auth_digest_hex));

	cwin->clastfm->state &= ~LASTFM_CONNECTED;
	cwin->clastfm->state |= LASTFM_NOT_CONNECTED;

	g_get_current_time(&time);
	g_sprintf(tstamp, "%ld", time.tv_sec);

	md5_init(&md5_s);
	md5_append(&md5_s, (const md5_byte_t *)cwin->cpref->lw.lastfm_pass,
		   strlen(cwin->cpref->lw.lastfm_pass));
	md5_finish(&md5_s, pass_digest);
	for (i = 0; i < 16; ++i)
		g_sprintf(pass_digest_hex + i * 2, "%02x", pass_digest[i]);

	md5_init(&md5_s);
	md5_append(&md5_s, (const md5_byte_t *)pass_digest_hex, 32);
	md5_append(&md5_s, (const md5_byte_t *)tstamp, strlen(tstamp));
	md5_finish(&md5_s, auth_digest);
	for (i = 0; i < 16; ++i)
		g_sprintf(auth_digest_hex + i * 2, "%02x", auth_digest[i]);

	url = g_strdup_printf("%s:%d/?hs=%s&p=%s&c=%s&v=%s&u=%s&t=%ld&a=%s",
			      LASTFM_URL, LASTFM_PORT, "true",
			      LASTFM_SUBMISSION_PROTOCOL,
			      LASTFM_CLIENT_ID,
			      LASTFM_CLIENT_VERSION,
			      cwin->cpref->lw.lastfm_user,
			      time.tv_sec,
			      auth_digest_hex);

	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_URL, url);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_PORT, LASTFM_PORT);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_HTTPGET, 1);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_CONNECTTIMEOUT, LASTFM_CONN_TIMEOUT);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_WRITEFUNCTION, lastfm_handshake_cb);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_WRITEDATA, cwin);

	ret = curl_easy_perform(cwin->clastfm->curl_handle);
	if (ret) {
		g_warning("CURL err: %s", curl_easy_strerror(ret));
		g_free(url);
		return -1;
	}

	g_free(url);
	return 0;
}

gint lastfm_submit(struct con_win *cwin, gchar *title, gchar *artist,
		   gchar *album, gint track_no, gint track_len,
		   GTimeVal start_time, enum track_source source)
{
	gchar *post_data;
	gchar track_no_s[8];
	gint ret = 0;

	if (!(cwin->clastfm->state & LASTFM_CONNECTED)) {
		g_warning("No connection Last.fm has been established");
		return -1;
	}

	if (!cwin->clastfm->session_id || !artist || !title || !start_time.tv_sec) {
		g_warning("Invalid Last.fm submission parameters");
		return -1;
	}

	if ((source == USER_SOURCE) && !track_len) {
		g_warning("Track length missing (Required for USER_SOURCE)");
		return -1;
	}

	memset(track_no_s, 0, sizeof(track_no_s));
	g_sprintf(track_no_s, "%d", track_no);

	post_data = g_strdup_printf("s=%s&a[0]=%s&t[0]=%s&i[0]=%ld&o[0]=%c&"
				    "r[0]=%s&l[0]=%d&b[0]=%s&n[0]=%s&m[0]=%s",
				    cwin->clastfm->session_id,
				    artist, title, start_time.tv_sec,
				    track_source_c[USER_SOURCE].c, "", track_len,
				    (album) ? album : "",
				    (track_no) ? track_no_s : "", "");

	CDEBUG(DBG_VERBOSE, "Submit data: %s\n", post_data);

	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_URL, cwin->clastfm->submission_url);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_PORT, LASTFM_PORT);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_POST, 1);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_CONNECTTIMEOUT, LASTFM_CONN_TIMEOUT);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_TIMEOUT, LASTFM_OPER_TIMEOUT);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_POSTFIELDS, post_data);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_WRITEFUNCTION, lastfm_submission_cb);
	curl_easy_setopt(cwin->clastfm->curl_handle,
			 CURLOPT_WRITEDATA, cwin);

	ret = curl_easy_perform(cwin->clastfm->curl_handle);
	if (ret) {
		g_warning("CURL err: %s", curl_easy_strerror(ret));
		g_free(post_data);
		return -1;
	}

	g_free(post_data);
	return 0;
}
