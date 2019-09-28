
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include "stream.h"
#include "period.h"
#include "connection.h"
#include "bigint.h"

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
  bigint_t test;

  bigint_test();

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

  bigint_t zero;
  bigint_init(&zero, 0u);
  Stream_Init(&stream, 10000000, &zero, 441);

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
  frames = 256;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

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

  //snd_output_t* out;
  //snd_output_stdio_attach(&out, stderr, 0);
  //snd_pcm_dump_sw_setup(handle, out);
  static uint8 firstStart = TRUE;
  static bigint_t lastPlayingTime;
  bigint_init(&lastPlayingTime, 0u);

  while (TRUE) {
	snd_pcm_sframes_t delayFrames;
	bigint_t playingTime;

    snd_pcm_delay(handle, &delayFrames);

    //printf("del: %ld length: %d\n", delayFrames, Stream_Length(&stream));

    while (Stream_Length(&stream) < 1000000 || delayFrames > 44100) {
    	snd_pcm_delay(handle, &delayFrames);
    	usleep(1000);
    }
    //printf("Buffer hat %d bytes\n", Stream_Length(&stream));

    time_t ads;

    getPlayingTime(&playingTime);

	//printf("Stream at soll: %d:%2d\n", playingTime/1000/60, (playingTime/1000) % 60);
	//printf("Stream at ist: %d:%2d\n", stream.position/44100/60/8, (stream.position/44100/8) % 60);

    //printf("diff: %d\n", playingTime - stream.position / stream.stepsPerTenMs * 10);
    //printf("delay cycles %ld\n", delayFrames);

    char out[30], out2[30], out3[30];
    bigint_t sollZeit, istZeit, delayBytes;
    bigint_clone(&sollZeit, &playingTime);
    bigint_multUint32(&sollZeit, 441*4);
    bigint_divUint32(&sollZeit, 10);
    bigint_toString(&sollZeit, &out[0]);
    bigint_toString(&stream.position, &out2[0]);

    bigint_clone(&istZeit, &stream.position);

    snd_pcm_delay(handle, &delayFrames);
    bigint_init(&delayBytes, delayFrames);
    bigint_multUint32(&delayBytes, 4u);
    bigint_sub(&istZeit, &delayBytes);

    if (bigint_greatherThan(&sollZeit, &istZeit, TRUE)) {
    	bigint_sub(&sollZeit, &istZeit);
    	bigint_toString(&sollZeit, &out3[0]);
        printf("Stream ist %s hinterher\n", &out3[0]);
        uint32 korr = bigint_toUint32(&sollZeit);
        korr -= (korr % 4u);

        if (TRUE == firstStart) {
        	//printf("Korrigiere %d\n", korr);
        	Stream_Seek(&stream, korr); //Hier kann der Stream bei Laufzeitunterscieden angepasst werden
			firstStart = FALSE;
		}

        if (korr > 1000u) {
        	Stream_Seek(&stream, 64); //Hier kann der Stream bei Laufzeitunterscieden angepasst werden
        }
    } else {
    	bigint_sub(&istZeit, &sollZeit);
    	bigint_toString(&istZeit, &out3[0]);

        printf("Stream ist %s vorneweg\n", &out3[0]);
    }


    //printf("Stream is at position: %s\n", &out2[0]);
    //printf("Stream should be at  : %s\n", &out[0]);


    rc = Stream_Get(&stream, buffer, size);
    bigint_clone(&lastPlayingTime, &playingTime);

    if (rc == 0) {
      fprintf(stderr, "end of file on input\n");
      break;
    } else if (rc != size) {
      fprintf(stderr,
              "short read: read %d bytes\n", rc);
    }

    if (snd_pcm_avail(handle) > frames) {
        rc = snd_pcm_writei(handle, buffer, frames);
        Stream_Seek(&stream, size);
    } else {
    	rc = frames;
    }

    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames) {
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
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 512);
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
