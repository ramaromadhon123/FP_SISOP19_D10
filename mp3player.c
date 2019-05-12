#define BITS 8
#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <dirent.h>

char command;

int play_trigger = 0;
int pause_trigger = 0;
int stop_trigger = 0;
int close_trigger = 0;

int menu_display = 0;
int play_display = 0;
int playlist_display = 0;

int i, j = 0;
char song[1000][1000];
char current[1000];
pthread_t tid1, tid2, tid3, tid4;

int getch()
{
    struct termios oldt, newt;
    int ch;

    tcgetattr (STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr (STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

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

void* Player (void *arg)
{
    while(1)
    {
        menu_display = 1;
        //1. Play Song
        //2. Show Playlist
        //3. Close

        command = getch(); //nunggu input
        if (command == '1') //Play Song
        {
            menu_display = 0;
            play_display = 1;

            int tracks = 0, title = 0;

            while(1)
            {
                printf("\r");
                title = 0;

                while (song[tracks][title] != '\0')
                {
                    sprintf(current, "%s%c", current, song[tracks][title]);
                    title++;
                }

                play_trigger = 1;
                play_display = 1;
                //1: Play/Pause
                //2: Next
                //3: Previous
                //4: Stop

                command = getch();  //nunggu input
                if (command == '1') //Pause
                {
                    if (pause_trigger == 0) //jika sedang di play
                    {
                        pause_trigger = 1;
                        printf ("Paused.\n");
                        current[0]='\0';
                    }

                    else if (pause_trigger == 1) //jika sedang di pause
                    {
                        pause_trigger = 0;
                        current[0]='\0';
                    }

                }

                else if (command == '2') //Next Song
                {
                    stop_trigger = 1;
                    current[0] = '\0';
                    tracks++;
                }

                else if (command == '3') //Previous Song
                {
                    stop_trigger = 1;
                    current[0]='\0';
                    tracks--;
                }

                else if (command == '4') //Stop Song
                {
                    stop_trigger = 1;
                    play_trigger = 0;
                    current[0]='\0';
                    break;
                }
            }
        }

        else if (command == '2') //Show Playlist
        {
            menu_display = 0;
            system ("clear");
            printf("\r");
            playlist_display = 1;

            command = getch(); //nunggu input
            if (command == '1') //Back to menu
            {
                playlist_display = 0;
                menu_display = 1;
            }
        }

        else if (command == '3') //Close
        {
            printf("Music Player Closed.\n\n");
            printf("\r");
            close_trigger = 1;
            break;
        }
    }
}

void* play_song (void *arg)
{
    while(1)
    {
        printf("\r");
        if (play_trigger == 1)
            play(current);
    }
}

void* display (void *arg)
{
    while (1)
    {
        printf ("\r");
        if (menu_display == 1)
        {
            printf("----- D10 Music Player -----\n");
            printf("1. Play Song\n");
            printf("2. Show Playlist\n");
            printf("3. Close\n\n");
            printf("Choose a command:\n");

            sleep(1);
            system("clear");
        }

        else if (play_display == 1)
        {
            printf("Now Playing : %s\n", current);
            printf("1. Pause or Play\n");
            printf("2. Next Song\n");
            printf("3. Previous Song\n");
            printf("4. Stop\n");
            sleep(1);
            system("clear");
        }

        else if (playlist_display == 1)
        {
            printf("\r");
            printf("Song List :\n");

            DIR *d;
            struct dirent *dir;
            d = opendir("."); //dir folder hasil fuse

            if(d)
            {
                while ((dir = readdir(d)) != NULL) //check directory for mp3
                {
                    char files[100];

                    strcpy (files, dir->d_name);
                    if ((files [strlen(files)-3] == 'm') && (files [strlen(files)-2] == 'p') && (files [strlen(files)-1] == '3'))
                    {
                        for(i = 0; i < strlen(files); i++) //scan songs
                            song[j][i] = files[i];

                        for(i = 0; i < strlen(files); i++) //print songs
                            printf("%c", song[j][i]);

                        j++;
                        printf("\n");
                    }
                }
                closedir(d);
            }

            printf("\nType 1 to return to main menu.\n");
            sleep(1);
            system("clear");
        }
    }
}

void* close_player (void *arg)
{
    printf("\r");
    while (close_trigger == 0)
    {
        printf("\r");

        if(close_trigger == 1)
            break;
    }

    pthread_kill(tid1, SIGKILL);
    pthread_kill(tid2, SIGKILL);
    pthread_kill(tid3, SIGKILL);
    pthread_kill(tid4, SIGKILL);
}

int main(void)
{
    pthread_create(&(tid1), NULL, Player, NULL);
    pthread_create(&(tid2), NULL, play_song, NULL);
    pthread_create(&(tid3), NULL, display, NULL);
    pthread_create(&(tid4), NULL, close_player, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);

    return 0;

    //run code: gcc -O2 -pthread -o mp3player mp3player.c -lmpg123 -lao
}


