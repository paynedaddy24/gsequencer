/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ags/object/ags_soundcard.h>

#include <math.h>

void ags_soundcard_base_init(AgsSoundcardInterface *interface);

GType
ags_soundcard_get_type()
{
  static GType ags_type_soundcard = 0;

  if(!ags_type_soundcard){
    static const GTypeInfo ags_soundcard_info = {
      sizeof(AgsSoundcardInterface),
      (GBaseInitFunc) ags_soundcard_base_init,
      NULL, /* base_finalize */
    };

    ags_type_soundcard = g_type_register_static(G_TYPE_INTERFACE,
						"AgsSoundcard\0", &ags_soundcard_info,
						0);
  }

  return(ags_type_soundcard);
}


GQuark
ags_soundcard_error_quark()
{
  return(g_quark_from_static_string("ags-soundcard-error-quark\0"));
}

void
ags_soundcard_base_init(AgsSoundcardInterface *interface)
{
  /* empty */
}

/**
 * ags_soundcard_set_presets:
 * @soundcard: an #AgsSoundcard
 * @presets: the presets to set
 *
 * Set presets. 
 *
 * Since: 0.4.2
 */
void
ags_soundcard_set_presets(AgsSoundcard *soundcard,
			  guint channels,
			  guint rate,
			  guint buffer_size,
			  guint format)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->set_presets);
  soundcard_interface->set_presets(soundcard,
				   channels,
				   rate,
				   buffer_size,
				   format);
}

/**
 * ags_soundcard_list_cards:
 * @soundcard: an #AgsSoundcard
 * @card_id: a list containing card ids
 * @card_name: a list containing card names
 * @error: an error that may occure
 *
 * Retrieve @card_id and @card_name as a list of strings.
 *
 * Since: 0.4.2
 */
void
ags_soundcard_list_cards(AgsSoundcard *soundcard,
			 GList **card_id, GList **card_name)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->list_cards);
  soundcard_interface->list_cards(soundcard, card_id, card_name);
}

/**
 * ags_soundcard_pcm_info:
 * @soundcard: an #AgsSoundcard
 * @card_id: the selected soundcard by its string identifier
 * @channels_min: min channels supported
 * @channels_max: max channels supported
 * @rate_min: min samplerate supported
 * @rate_max: max samplerate supported
 * @buffer_size_min: min buffer size supported by soundcard
 * @buffer_size_max: max buffer size supported by soundcard
 * @error: an error that may occure
 *
 * Retrieve detailed information of @card_id soundcard.
 *
 * Since: 0.4.2
 */
void
ags_soundcard_pcm_info(AgsSoundcard *soundcard, gchar *card_id,
		       guint *channels_min, guint *channels_max,
		       guint *rate_min, guint *rate_max,
		       guint *buffer_size_min, guint *buffer_size_max,
		       GError **error)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->pcm_info);
  soundcard_interface->pcm_info(soundcard, card_id,
				channels_min, channels_max,
				rate_min, rate_max,
				buffer_size_min, buffer_size_max,
				error);
}

/**
 * ags_soundcard_is_starting:
 * @soundcard: an #AgsSoundcard
 *
 * Get starting.
 *
 * Returns: %TRUE if starting, else %FALSE
 *
 * Since: 0.4.3
 */
gboolean
ags_soundcard_is_starting(AgsSoundcard *soundcard)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_val_if_fail(AGS_IS_SOUNDCARD(soundcard), FALSE);
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_val_if_fail(soundcard_interface->is_starting, FALSE);

  return(soundcard_interface->is_starting(soundcard));
}

/**
 * ags_soundcard_is_playing:
 * @soundcard: an #AgsSoundcard
 *
 * Get playing.
 *
 * Returns: %TRUE if playing, else %FALSE
 *
 * Since: 0.4.3
 */
gboolean
ags_soundcard_is_playing(AgsSoundcard *soundcard)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_val_if_fail(AGS_IS_SOUNDCARD(soundcard), FALSE);
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_val_if_fail(soundcard_interface->is_playing, FALSE);

  return(soundcard_interface->is_playing(soundcard));
}

/**
 * ags_soundcard_play:
 * @soundcard: an #AgsSoundcard
 * @error: an error that may occure
 *
 * Initializes the soundcard for playback.
 *
 * Since: 0.4.2
 */
void
ags_soundcard_play_init(AgsSoundcard *soundcard,
			GError **error)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->play_init);
  soundcard_interface->play_init(soundcard,
				 error);
}

/**
 * ags_soundcard_play:
 * @soundcard: an #AgsSoundcard
 * @error: an error that may occure
 *
 * Plays the current buffer of soundcard.
 *
 * Since: 0.4.2
 */
void
ags_soundcard_play(AgsSoundcard *soundcard,
		   GError **error)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->play);
  soundcard_interface->play(soundcard,
			    error);
}

/**
 * ags_soundcard_stop:
 * @soundcard: an #AgsSoundcard
 *
 * Stops the soundcard from playing to it.
 *
 * Since: 0.4.2
 */
void
ags_soundcard_stop(AgsSoundcard *soundcard)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->stop);
  soundcard_interface->stop(soundcard);
}

/**
 * ags_soundcard_tic:
 * @soundcard: an #AgsSoundcard
 *
 * Every call to play may generate a tic. 
 *
 * Since: 0.4.2
 */
void
ags_soundcard_tic(AgsSoundcard *soundcard)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->tic);
  soundcard_interface->tic(soundcard);
}

/**
 * ags_soundcard_tic:
 * @soundcard: an #AgsSoundcard
 *
 * Callback when counter expires minor note offset.
 *
 * Since: 0.4.2
 */
void
ags_soundcard_offset_changed(AgsSoundcard *soundcard,
			     guint note_offset)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_if_fail(AGS_IS_SOUNDCARD(soundcard));
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_if_fail(soundcard_interface->offset_changed);
  soundcard_interface->offset_changed(soundcard,
				      note_offset);
}

/**
 * ags_soundcard_get_buffer:
 * @soundcard: an #AgsSoundcard
 *
 * Get current playback buffer. 
 *
 * Returns: current playback buffer
 *
 * Since: 0.4.2
 */
signed short*
ags_soundcard_get_buffer(AgsSoundcard *soundcard)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_val_if_fail(AGS_IS_SOUNDCARD(soundcard), NULL);
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_val_if_fail(soundcard_interface->get_buffer, NULL);
  return(soundcard_interface->get_buffer(soundcard));
}

/**
 * ags_soundcard_get_next_buffer:
 * @soundcard: an #AgsSoundcard
 *
 * Get future playback buffer.
 *
 * Returns: next playback buffer
 *
 * Since: 0.4.2
 */
signed short*
ags_soundcard_get_next_buffer(AgsSoundcard *soundcard)
{
  AgsSoundcardInterface *soundcard_interface;

  g_return_val_if_fail(AGS_IS_SOUNDCARD(soundcard), NULL);
  soundcard_interface = AGS_SOUNDCARD_GET_INTERFACE(soundcard);
  g_return_val_if_fail(soundcard_interface->get_next_buffer, NULL);
  return(soundcard_interface->get_next_buffer(soundcard));
}
