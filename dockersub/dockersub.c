#define _GNU_SOURCE
#define _XOPEN_SOURCE 700

#include <err.h>
#include <curl/curl.h>
#include <string.h>
#include <nats/nats.h>
#include <unistd.h>
#include <dirent.h>
#include <ftw.h>

size_t write_data(void * buffer, size_t size, size_t nmemb, void * userp) {
    return size * nmemb;
}

int main(int argc, char *argv[])
{

    int ret;
    size_t read_bytes;

    int i = 0;
//    natsConnection *nc = NULL;
//
//    natsStatus s = natsConnection_ConnectTo(&nc, NATS_DEFAULT_URL);
//    if (NATS_OK != s) {
//        printf("error: %s\n", natsStatus_GetText(s));
//        exit(1);
//    }
//
//    for (;;) {
//
//        for (i = 0; i < sizeof(HOSTS)/sizeof(struct host); i++) {
//            check_host(nc, HOSTS[i]);
//        }
//        sleep(30);
//    }


    DIR * d = opendir("/sys/fs/cgroup/cpu/docker");
    if (NULL == d) {
        printf("docker does not appear to be running.\n");
        return 0;
    }

    struct dirent * ent = readdir(d);
    while (NULL != ent) {
        printf("name: %s\n", ent->d_name);
        printf("type: %i\n", ent->d_type);
        ent = readdir(d);
    }
    closedir(d);

//    natsConnection_Close(nc);
//    natsConnection_Destroy(nc);

    return 0;
}
