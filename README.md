# FP_SISOP19_D10 </br>

## Soal:</br>

Buatlah sebuah music player dengan bahasa C yang memiliki fitur play nama_lagu, pause, next, prev, list lagu. Selain music player juga terdapat FUSE untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam FUSE yang tersebar pada direktori /home/user. Ketika FUSE dijalankan, direktori hasil FUSE hanya berisi file .mp3 tanpa ada direktori lain di dalamnya. Asal file tersebut bisa tersebar dari berbagai folder dan subfolder. program mp3 mengarah ke FUSE untuk memutar musik.</br>


## Solusi:</br>

### 1. Music Player:</br>
1. Mempunyai 4 fungsi, yaitu **Player, play_song, display, close_player** yang dibuat menggunakan thread.</br>

   a. _Player_</br>
      Merupakan fungsi utama yang menyimpan segala fitur music player serta mengatur input dan output.</br>
      Fitur music player memanfaatkan 4 trigger, yaitu:</br>
      - _play_trigger_ : menentukan status play lagu</br>
      - _pause_trigger_ : menentukan status pause lagu</br>
      - _stop_trigger_ : menentukan status stop lagu</br>
      - _close_trigger_ : mengarah ke fungsi close untuk menutup music player</br>
      
   b. _play_song_</br>
      Dipengaruhi oleh play_trigger. Berfungsi untuk memutar lagu.</br>
     
   c. _display_</br>
      Dipengaruhi oleh: </br>
      - _menu_display_ untuk menampilkan menu:</br>
        ```
        1. Play Song
        2. Show Playlist
        3. Close
        ```
     
      - _play_display_ untuk menampilkan menu:</br>
        ```
        1: Play/Pause
        2: Next
        3: Previous
        4: Stop
        ```
      - _playlist_display_ untuk menampilkan daftar lagu yang ada.</br>

   d. close_player</br>
      Merupakan fungsi utama yang menyimpan segala fitur music player serta mengatur input dan output.</br>   

2. Agar bisa memutar lagu menggunakan fungsi **play**</br>
  ```c
    int play(char argv[])
  {
      while(1)
      {
          mpg123_handle *mh;
          unsigned char *buffer;
          size_t buffer_size;
          size_t done;
          int err;

          int driver;
          ao_device *dev;

          ao_sample_format format;
          int channels, encoding;
          long rate;

          // initializations
          ao_initialize ();
          driver = ao_default_driver_id ();
          mpg123_init ();
          mh = mpg123_new (NULL, &err);
          buffer_size = mpg123_outblock(mh);
          buffer = (unsigned char*) malloc (buffer_size * sizeof(unsigned char));

          // open the file and get the decoding format
          mpg123_open (mh, argv);
          mpg123_getformat (mh, &rate, &channels, &encoding);

          // set the output format and open the output device
          format.bits = mpg123_encsize(encoding) * BITS;
          format.rate = rate;
          format.channels = channels;
          format.byte_format = AO_FMT_NATIVE;
          format.matrix = 0;
          dev = ao_open_live (driver, &format, NULL);

          // decode and play
          while (mpg123_read (mh, buffer, buffer_size, &done) == MPG123_OK)
          {
              ao_play(dev, buffer, done);
              if(stop_trigger == 1)
                  break;

              while (pause_trigger == 1)
              {
                  printf("\r");
                  if(pause_trigger == 0)
                      break;
              }
          }
          stop_trigger = 0;

          // clean up
          free(buffer);
          ao_close(dev);
          mpg123_close(mh);
          mpg123_delete(mh);
          mpg123_exit();
          ao_shutdown();
          break;
      }
  }
  ```
  
### 2. FUSE:</br>
