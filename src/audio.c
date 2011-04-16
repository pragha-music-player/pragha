/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>			 */
/* Copyright (C) 2009 matias <mati86dl@gmail.com>			 */
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

/**************/
/* ALSA mixer */
/**************/

#include <alsa/asoundlib.h>

#define ALSA_MIXER_DEV  "default"
#define ALSA_MIXER_ELEM_PCM "PCM"
#define ALSA_MIXER_ELEM_MASTER "Master"

static snd_mixer_t *alsa_mixer = NULL;
static snd_mixer_elem_t *alsa_elem = NULL;
static GIOChannel **alsa_chans = NULL;
static int n_alsa_chans = 0;

static int alsa_elem_cb(snd_mixer_elem_t *alsa_elem, unsigned int mask)
{
	glong curr_vol;
	struct con_win *cwin;

	if (mask == SND_CTL_EVENT_MASK_REMOVE)
		return 0;

	if (mask & SND_CTL_EVENT_MASK_VALUE) {
		cwin = snd_mixer_elem_get_callback_private(alsa_elem);
		snd_mixer_selem_get_playback_volume(alsa_elem,
						    SND_MIXER_SCHN_FRONT_LEFT,
						    &curr_vol);
		cwin->cmixer->curr_vol = SCALE_UP_VOL(curr_vol);
		gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button),
					   cwin->cmixer->curr_vol);
	}

	return 0;
}

static gboolean alsa_io_cb(GIOChannel *chan, GIOCondition condition, gpointer data)
{
	switch(condition) {
	case G_IO_OUT:
	case G_IO_IN:
	case G_IO_PRI:
		snd_mixer_handle_events(alsa_mixer);
		return TRUE;
	case G_IO_ERR:
	case G_IO_HUP:
	case G_IO_NVAL:
	default:
		g_warning("ALSA mixer polling error");
		return FALSE;
	}
}

static snd_mixer_elem_t* get_alsa_elem(gchar *m_elem)
{
	snd_mixer_elem_t *elem = NULL;
	const gchar *elem_name = NULL;

	elem = snd_mixer_first_elem(alsa_mixer);

	while (elem) {
		if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE) {
			elem_name = snd_mixer_selem_get_name(elem);
			if (!g_ascii_strcasecmp(elem_name, m_elem)) {
				CDEBUG(DBG_INFO, "Found ALSA elem: %s", m_elem);
				break;
			}
		}
		elem = snd_mixer_elem_next(elem);
	}
	return elem;
}

static gint alsa_init_mixer(struct con_win *cwin)
{
#define POLL_FLAGS G_IO_IN | G_IO_OUT | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL

	gint ret = 0, cnt = 0, i;
	struct pollfd *fds;

	if (snd_mixer_open(&alsa_mixer, 0) < 0) {
		g_critical("Unable to open Alsa mixer");
		return -1;
	}

	if (snd_mixer_attach(alsa_mixer, ALSA_MIXER_DEV) < 0) {
		g_critical("Unable to attach to alsa dev : %s",
			   ALSA_MIXER_DEV);
		ret = -1;
		goto bad;
	}

	if (snd_mixer_selem_register(alsa_mixer, NULL, NULL) < 0) {
		g_critical("Unable to register alsa mixer simple element");
		ret = -1;
		goto bad;
	}

	if (snd_mixer_load(alsa_mixer) < 0) {
		g_critical("Unable to load mixer");
		ret = -1;
		goto bad;
	}

	/* Search for cmdline mixer element first */

	if (cwin->cmixer->mixer_elem) {
		alsa_elem = get_alsa_elem(cwin->cmixer->mixer_elem);
		if (!alsa_elem) {
			g_critical("Invalid ALSA mixer element: %s",
				   cwin->cmixer->mixer_elem);
			ret = -1;
			goto bad;
		}
	}

	/* Search for PCM next */

	if (!alsa_elem)
		alsa_elem = get_alsa_elem(ALSA_MIXER_ELEM_PCM);

	/* If PCM is not found, search for Master */

	if (!alsa_elem)
		alsa_elem = get_alsa_elem(ALSA_MIXER_ELEM_MASTER);

	if (!alsa_elem) {
		g_critical("Unable to find a valid ALSA mixer element");
		ret = -1;
		goto bad;
	}

	snd_mixer_selem_get_playback_volume_range(alsa_elem,
						  &cwin->cmixer->min_vol,
						  &cwin->cmixer->max_vol);
	snd_mixer_selem_get_playback_volume(alsa_elem,
					    SND_MIXER_SCHN_FRONT_LEFT,
					    &cwin->cmixer->curr_vol);
	CDEBUG(DBG_INFO, "Max vol: %ld, Curr vol: %ld",
	       cwin->cmixer->max_vol, cwin->cmixer->curr_vol);

	cnt = snd_mixer_poll_descriptors_count(alsa_mixer);
	if (!cnt) {
		g_warning("No ALSA poll descriptors");
		goto exit;
	}

	fds = g_new0(struct pollfd, cnt);

	if (snd_mixer_poll_descriptors(alsa_mixer, fds, cnt) < 0) {
		g_warning("Unable to get ALSA poll descriptors");
		g_free(fds);
		goto exit;
	}

	snd_mixer_elem_set_callback(alsa_elem, alsa_elem_cb);
	snd_mixer_elem_set_callback_private(alsa_elem, cwin);

	alsa_chans = g_new0(GIOChannel*, cnt);

	for (i=0; i<cnt; i++) {
		alsa_chans[i] = g_io_channel_unix_new(fds[i].fd);
		g_io_add_watch(alsa_chans[i], POLL_FLAGS, alsa_io_cb, cwin);
	}
	g_free(fds);
	n_alsa_chans = cnt;

#if 0
	/* Have to really analyze this one ... */

	gint err = 0;
	if ((err = snd_config_update_free_global() < 0)) {
		g_critical("ALSA error: %s", snd_strerror(err));
		ret = -1;
	}
#endif

	CDEBUG(DBG_INFO, "Poll ALSA for %d descriptors", cnt);

	goto exit;
bad:
	snd_mixer_close(alsa_mixer);
exit:
	return ret;

#undef POLL_FLAGS
}

static gint alsa_mute_mixer(struct con_win *cwin)
{
	return 0;
}

static void alsa_set_volume(struct con_win *cwin)
{
	CDEBUG(DBG_VERBOSE, "Setting ALSA volume to %ld",
	       SCALE_DOWN_VOL(cwin->cmixer->curr_vol));

	if (snd_mixer_selem_set_playback_volume_all(alsa_elem,
						    SCALE_DOWN_VOL(cwin->cmixer->curr_vol)) < 0)
		g_critical("Unable to set alsa volume : %ld",
			   SCALE_DOWN_VOL(cwin->cmixer->curr_vol));
}

static void alsa_inc_volume(struct con_win *cwin)
{
	if (SCALE_DOWN_VOL(cwin->cmixer->curr_vol) < cwin->cmixer->max_vol)
		cwin->cmixer->curr_vol+=5;

	CDEBUG(DBG_VERBOSE, "Setting ALSA volume to %ld",
	       SCALE_DOWN_VOL(cwin->cmixer->curr_vol));

	if (snd_mixer_selem_set_playback_volume_all(alsa_elem,
						    SCALE_DOWN_VOL(cwin->cmixer->curr_vol)) < 0)
		g_critical("Unable to set alsa volume : %ld",
			   SCALE_DOWN_VOL(cwin->cmixer->curr_vol));
}

static void alsa_dec_volume(struct con_win *cwin)
{
	if (SCALE_DOWN_VOL(cwin->cmixer->curr_vol) > cwin->cmixer->min_vol)
		cwin->cmixer->curr_vol-=5;

	CDEBUG(DBG_VERBOSE, "Setting ALSA volume to %ld",
	       SCALE_DOWN_VOL(cwin->cmixer->curr_vol));

	if (snd_mixer_selem_set_playback_volume_all(alsa_elem,
						    SCALE_DOWN_VOL(cwin->cmixer->curr_vol)) < 0)
		g_critical("Unable to set alsa volume : %ld",
			   SCALE_DOWN_VOL(cwin->cmixer->curr_vol));
}

static void alsa_deinit_mixer(struct con_win *cwin)
{
	GError *err = NULL;
	int i;

	for (i=0; i<n_alsa_chans; i++) {
		g_io_channel_shutdown(alsa_chans[i], FALSE, &err);
		if (err) {
			g_warning("Unable to shutdown IO Channel for ALSA");
			g_error_free(err);
			err = NULL;
		}
		else {
			CDEBUG(DBG_INFO, "Shutting down ALSA IO channel");
		}
		g_io_channel_unref(alsa_chans[i]);
	}
	g_free(alsa_chans);

	if (snd_mixer_detach(alsa_mixer, ALSA_MIXER_DEV))
		g_warning("Unable to deinit alsa mixer");

	snd_mixer_free(alsa_mixer);
}

/*************/
/* OSS Mixer */
/*************/

#include <sys/ioctl.h>
#include <linux/soundcard.h>

#define OSS_MIXER_DEV "/dev/mixer"
#define OSS_POLL_TIMER 3

static gint oss_mixer_fd;
static guint g_poll_id;

static gboolean oss_mixer_poll(gpointer data)
{
	gint curr_vol;
	struct con_win *cwin = data;

	if (ioctl(oss_mixer_fd, SOUND_MIXER_READ_VOLUME, &curr_vol) == -1) {
		g_critical("Unable to get current oss volume: %s",
			   strerror(errno));
		return FALSE;
	}

	if ((curr_vol & 0xff) != cwin->cmixer->curr_vol) {
		cwin->cmixer->curr_vol = curr_vol & 0xff;
		gtk_scale_button_set_value(GTK_SCALE_BUTTON(cwin->vol_button),
					   cwin->cmixer->curr_vol);
	}

	return TRUE;
}

static gint oss_init_mixer(struct con_win *cwin)
{
	gint curr_vol;

	if (cwin->cmixer->mixer_elem)
		oss_mixer_fd = g_open(cwin->cmixer->mixer_elem, O_RDONLY);
	else
		oss_mixer_fd = g_open(OSS_MIXER_DEV, O_RDONLY);

	if (oss_mixer_fd == -1) {
		g_critical("Unable to open a valid OSS mixer: %s",
			   strerror(errno));
		return -1;
	}

	if (ioctl(oss_mixer_fd, SOUND_MIXER_READ_VOLUME, &curr_vol) == -1) {
		g_critical("Unable to get current oss volume: %s",
			   strerror(errno));
		return -1;
	}

	cwin->cmixer->min_vol = 0;
	cwin->cmixer->max_vol = 100;
	cwin->cmixer->curr_vol = curr_vol & 0xff;

	CDEBUG(DBG_INFO, "Max vol: %ld, Curr vol: %ld",
	       cwin->cmixer->max_vol, cwin->cmixer->curr_vol);

	g_poll_id = g_timeout_add_seconds(OSS_POLL_TIMER, oss_mixer_poll, cwin);

	return 0;
}

static gint oss_mute_mixer(struct con_win *cwin)
{
	return 0;
}

static void oss_set_volume(struct con_win *cwin)
{
	gint curr_vol;

	curr_vol = cwin->cmixer->curr_vol;
	CDEBUG(DBG_VERBOSE, "Setting OSS volume to %ld", cwin->cmixer->curr_vol);

	if (ioctl(oss_mixer_fd, SOUND_MIXER_WRITE_VOLUME, &curr_vol) == -1)
		g_critical("Unable to set OSS volume : %ld, %s",
			   cwin->cmixer->curr_vol, strerror(errno));
}

static void oss_inc_volume(struct con_win *cwin)
{
	gint curr_vol;

	if (cwin->cmixer->curr_vol < cwin->cmixer->max_vol)
		cwin->cmixer->curr_vol+=5;

	curr_vol = cwin->cmixer->curr_vol;
	CDEBUG(DBG_VERBOSE, "Setting OSS volume to %d", curr_vol);

	if (ioctl(oss_mixer_fd, SOUND_MIXER_WRITE_VOLUME, &curr_vol) == -1)
		g_critical("Unable to set OSS volume : %d, %s",
			   curr_vol, strerror(errno));
}

static void oss_dec_volume(struct con_win *cwin)
{
	gint curr_vol;

	if (cwin->cmixer->curr_vol > cwin->cmixer->min_vol)
		cwin->cmixer->curr_vol-=5;

	curr_vol = cwin->cmixer->curr_vol;
	CDEBUG(DBG_VERBOSE, "Setting OSS volume to %d", curr_vol);

	if (ioctl(oss_mixer_fd, SOUND_MIXER_WRITE_VOLUME, &curr_vol) == -1)
		g_critical("Unable to set OSS volume : %d, %s",
			   curr_vol, strerror(errno));
}

static void oss_deinit_mixer(struct con_win *cwin)
{
	GSource *src;

	CDEBUG(DBG_INFO, "Deinit OSS mixer");

	src = g_main_context_find_source_by_id(NULL, g_poll_id);
	if (src) {
		g_source_remove(g_poll_id);
		g_source_destroy(src);
	}

	if (close(oss_mixer_fd) == -1)
		g_warning("Unable to close OSS mixer: %s", strerror(errno));
}

/******************/
/* Software Mixer */
/******************/

static gint soft_init_mixer(struct con_win *cwin)
{
	cwin->cmixer->min_vol = 0;
	cwin->cmixer->max_vol = 100;
	cwin->cmixer->curr_vol = 100;

	CDEBUG(DBG_INFO, "Max vol: %ld, Curr vol: %ld",
	       cwin->cmixer->max_vol, cwin->cmixer->curr_vol);

	return 0;
}

static gint soft_mute_mixer(struct con_win *cwin)
{
	return 0;
}

static void soft_set_volume(struct con_win *cwin)
{
}

static void soft_inc_volume(struct con_win *cwin)
{
	if (cwin->cmixer->curr_vol < cwin->cmixer->max_vol)
		cwin->cmixer->curr_vol+=5;

	CDEBUG(DBG_VERBOSE, "Setting SOFT volume to %ld",
	       cwin->cmixer->curr_vol);
}

static void soft_dec_volume(struct con_win *cwin)
{
	if (cwin->cmixer->curr_vol > cwin->cmixer->min_vol)
		cwin->cmixer->curr_vol-=5;

	CDEBUG(DBG_VERBOSE, "Setting SOFT volume to %ld",
	       cwin->cmixer->curr_vol);
}

static void soft_deinit_mixer(struct con_win *cwin)
{
	return;
}

/********************************/
/* Externally visible functions */
/********************************/

void set_alsa_mixer(struct con_win *cwin, gchar *mixer_elem)
{
	CDEBUG(DBG_INFO, "Initializing ALSA audio mixer");
	cwin->cmixer->mixer_elem = mixer_elem;
	cwin->cmixer->set_volume   = alsa_set_volume;
	cwin->cmixer->inc_volume   = alsa_inc_volume;
	cwin->cmixer->dec_volume   = alsa_dec_volume;
	cwin->cmixer->init_mixer   = alsa_init_mixer;
	cwin->cmixer->deinit_mixer = alsa_deinit_mixer;
	cwin->cmixer->mute_mixer   = alsa_mute_mixer;
}

void set_oss_mixer(struct con_win *cwin, gchar *mixer_elem)
{
	CDEBUG(DBG_INFO, "Initializing OSS audio mixer");
	cwin->cmixer->mixer_elem = mixer_elem;
	cwin->cmixer->set_volume   = oss_set_volume;
	cwin->cmixer->inc_volume   = oss_inc_volume;
	cwin->cmixer->dec_volume   = oss_dec_volume;
	cwin->cmixer->init_mixer   = oss_init_mixer;
	cwin->cmixer->deinit_mixer = oss_deinit_mixer;
	cwin->cmixer->mute_mixer   = oss_mute_mixer;
}

void set_soft_mixer(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Initializing SOFT audio mixer");
	cwin->cmixer->set_volume   = soft_set_volume;
	cwin->cmixer->inc_volume   = soft_inc_volume;
	cwin->cmixer->dec_volume   = soft_dec_volume;
	cwin->cmixer->init_mixer   = soft_init_mixer;
	cwin->cmixer->deinit_mixer = soft_deinit_mixer;
	cwin->cmixer->mute_mixer   = soft_mute_mixer;
}

/* Credit: mpd */
void soft_volume_apply(gchar *buffer, gint buflen, struct con_win *cwin)
{
	gint32 c;
	gint16 *buf = (gint16 *)buffer;

	while(buflen > 0) {
		c = *buf;
		c *= cwin->cmixer->curr_vol;
		c /= 100;
		*buf++ = c;
		buflen -= 2;
	}
}

/* Returns a list of possible ALSA PCM devices, caller has to free the list.
 * Credit: alsa-utils */

GSList* alsa_pcm_devices(struct con_win *cwin)
{
	snd_ctl_t *handle;
	snd_pcm_info_t *pcminfo;
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	gint card, err, dev;
	GSList *devices = NULL;
	gchar *device;

	snd_pcm_info_alloca(&pcminfo);

	card = -1;
	if (snd_card_next(&card) < 0 || card < 0)
		return NULL;

	while (card >= 0) {
		char name[32];

		sprintf(name, "hw:%d", card);
		if ((err = snd_ctl_open(&handle, name, 0)) < 0)
			goto next_card;

		dev = -1;
		while (1) {
			snd_ctl_pcm_next_device(handle, &dev);
			if (dev < 0)
				break;

			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);

			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0)
				continue;

			device = g_strdup_printf("hw:%d,%d", card, dev);
			devices = g_slist_append(devices, device);
		}
		snd_ctl_close(handle);

	next_card:
		if (snd_card_next(&card) < 0)
			break;
	}

	return devices;
}

gint open_audio_device(gint samplerate, gint channels,
		       gboolean resume, struct con_win *cwin)
{
	if (!resume) {
		if (!samplerate || !channels)
			return -1;

		if (cwin->clibao->ao_dev) {
			CDEBUG(DBG_INFO, "Closing existing AO connection");
			ao_close(cwin->clibao->ao_dev);
		}

		cwin->clibao->format.byte_format = AO_FMT_LITTLE;
		cwin->clibao->format.bits = 16;
		cwin->clibao->format.rate = samplerate;
		cwin->clibao->format.channels = channels;
	}

	CDEBUG(DBG_INFO, "Opening AO, samplerate: %d, channels: %d",
	       samplerate, channels);

	cwin->clibao->ao_dev = ao_open_live(cwin->clibao->ao_driver_id,
					    &cwin->clibao->format,
					    cwin->clibao->ao_opts);
	if (cwin->clibao->ao_dev == NULL) {
		g_critical("Unable to open audio device");
		return -1;
	}

	return 0;
}
