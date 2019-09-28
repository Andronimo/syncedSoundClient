
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include "stream.h"
#include "period.h"
#include "connection.h"

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);

int main(int argc, char **argv) {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  uint8 *buffer;
  stream_t stream;

  if (argc == 2) {
	  connection_setServer(argv[1], 2220);
  } else if (argc == 3) {
	  int port = atol(argv[2]);

	  if (0u == port) {
		  printf("Usage: syncedSoundClient server-url [port]\n");
		  return 1;
	  }
	  connection_setServer(argv[1], port);
  } else {
	  printf("Usage: syncedSoundClient server-url [port]\n");
	  return 1;
  }

  Stream_Init(&stream, 10000000, 0, 441);

  startCyclic(&stream);

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 32 frames. */
  frames = 512;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);
  
  snd_pcm_uframes_t framesll;
  snd_pcm_hw_params_get_buffer_size_max(params, &framesll);
  //printf("max: %d\n", framesll);

  // Probably needed for embedded device
  rc = snd_pcm_hw_params_set_buffer_size(handle,
                              params, 4096);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set buffer size: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);

  snd_pcm_sw_params_t * sw_params;
  snd_pcm_sw_params_alloca(&sw_params);
  if ((rc = set_swparams(handle, sw_params)) < 0) {
          printf("Setting of swparams failed: %s\n", snd_strerror(rc));
          exit(EXIT_FAILURE);
  }

  size = frames * 4; /* 2 bytes/sample, 2 channels */
  buffer = (uint8 *) malloc(size);

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  /* 5 seconds in microseconds divided by
   * period time */
  loops = 50000000 / val;

  snd_output_t* out;
  snd_output_stdio_attach(&out, stderr, 0);
  snd_pcm_dump_sw_setup(handle, out);

  uint8 firstStart = TRUE;

  while (TRUE) {
	snd_pcm_sframes_t delayFrames;
	uint32 playingTime;
	static uint32 lastPlayingTime = 0;

    snd_pcm_delay(handle, &delayFrames);

    //printf("del: %ld length: %d\n", delayFrames, Stream_Length(&stream));

    while (Stream_Length(&stream) < 1000000 || delayFrames > 88200) {
    	snd_pcm_delay(handle, &delayFrames);
    	usleep(1000);
    }

    time_t ads;

    playingTime = getPlayingTime();

    uint32 sollZeit = playingTime*441*4/10;
    uint32 istZeit = stream.position;

    //printf("soll: %d, ist: %d\n", sollZeit, istZeit);
    //printf("length %d, delay %ld, avail: %d, state: %d\n", Stream_Length(&stream), delayFrames, snd_pcm_avail(handle), snd_pcm_state(handle));

    snd_pcm_delay(handle, &delayFrames);
    istZeit -= delayFrames*4;
        	
    //printf("Ãt = %d\n", sollZeit-istZeit);

    if (sollZeit >= istZeit) {
    	uint32 vorZeit = sollZeit - istZeit;
        vorZeit -= (vorZeit % 4u);

        if (TRUE == firstStart) {
        	printf("Korrigiere Vorlauf %d\n", vorZeit);
        	Stream_Seek(&stream, vorZeit);
			firstStart = FALSE;
	}

        if (vorZeit > 1000u) {
        	Stream_Seek(&stream, 64);
        }
    } else {
    	uint32 hintZeit = istZeit - sollZeit;
	hintZeit -= (hintZeit % 4u);

    	if (TRUE == firstStart) {
		printf("Korrigiere Nachlauf %d\n", hintZeit);
		Stream_Rewind(&stream, hintZeit);
		firstStart = FALSE;
	}

        if (hintZeit > 1000u) {
        //	Stream_Rewind(&stream, 64);
        }
    }

    rc = Stream_Get(&stream, buffer, size);
    lastPlayingTime = playingTime;

    if (rc == 0) {
      fprintf(stderr, "end of file on input\n");
      break;
    } else if (rc != size) {
      fprintf(stderr,
              "short read: read %d bytes\n", rc);
    }

    //printf("%d: avail %ld\n", playingTime, snd_pcm_avail(handle));

    uint32 newFrames = snd_pcm_avail(handle); 
    if (newFrames > frames) {
	
        //printf("try %d frames.....  ", frames);
    	rc = snd_pcm_writei(handle, buffer, frames);
    	Stream_Seek(&stream, size);
    } else {
	if (newFrames < 60) {
	     newFrames = 0u;
	}
	//printf("try %d frames.....  ", newFrames);
    	rc = snd_pcm_writei(handle, buffer, newFrames );
    	Stream_Seek(&stream, newFrames*4);
    }
    //printf("wrote %d\n", rc);

    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames && newFrames > 0u) {
      fprintf(stderr,
              "short write, write %d frames\n", rc);
    }
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  closeCyclic();
  Stream_Close(&stream);

  return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int err;
    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    //err = snd_pcm_sw_params_set_avail_min(handle, swparams, 512);
    //if (err < 0) {
    //    printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
    //    return err;
    //}
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 1024);
    if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, 2000000);
    if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }

    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
