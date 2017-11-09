#include <err.h>
#include <curl/curl.h>
#include <string.h>
#include <nats/nats.h>
#include <unistd.h>

struct host {
    const char * const url;
    const char * const endpoint;
};


struct host HOSTS[] = {
    {
        .url = "http://pluspole.se",
        .endpoint = "s.http.pluspole.se"
    },
    {
        .url = "http://parans.pluspole.se",
        .endpoint = "s.http.parans.pluspole.se"
    }
};

size_t write_data(void * buffer, size_t size, size_t nmemb, void * userp) {
    return size * nmemb;
}

int check_host(natsConnection * nc, struct host host) {
    int ret;
    char buf[512];

    CURL *curl = curl_easy_init();
    if (NULL == curl) {
        err(1, "curl_easy_init");
    }

    curl_easy_setopt(curl, CURLOPT_URL, host.url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    ret = curl_easy_perform(curl);
    if (CURLE_OK != ret) {
        curl_easy_cleanup(curl);
        warn("curl_easy_perform");
        return -1;
    }

    long response_code;
    ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (CURLE_OK != ret) {
        warn("curl_easy_getinfo");
        return -1;
    }

    double connect_time;
    ret = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect_time);
    if (CURLE_OK != ret) {
        warn("curl_easy_getinfo");
        return -1;
    }

    double appconnect_time;
    ret = curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &appconnect_time);
    if (CURLE_OK != ret) {
        warn("curl_easy_getinfo");
        return -1;
    }

    double starttransfer_time;
    ret = curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &starttransfer_time);
    if (CURLE_OK != ret) {
        warn("curl_easy_getinfo");
        return -1;
    }

    double total_time;
    ret = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
    if (CURLE_OK != ret) {
        warn("curl_easy_getinfo");
        return -1;
    }

    snprintf(buf, 512, "{\"response_code\": %li, \"connect_time\": %lf, \"ssl_connect_time\": %lf, \"ttfb\": %lf, \"total_time\": %lf}", 
            response_code,
            connect_time,
            appconnect_time,
            starttransfer_time,
            total_time);

    printf("publishing \"%s\" to \"%s\"\n", buf, host.endpoint);
    natsStatus s = natsConnection_PublishString(nc, host.endpoint, buf);
    if (NATS_OK != s) {
        nats_PrintLastErrorStack(stderr);
        err(1, "natsConnection_PublishString");
    } else {
        printf("ok!\n");
    }

    curl_easy_cleanup(curl);

    return 0;

}

int main(int argc, char *argv[])
{

    int ret;
    size_t read_bytes;

    int i = 0;
    natsConnection *nc = NULL;

    natsStatus s = natsConnection_ConnectTo(&nc, NATS_DEFAULT_URL);
    if (NATS_OK != s) {
        printf("error: %s\n", natsStatus_GetText(s));
        exit(1);
    }

    for (;;) {

        for (i = 0; i < sizeof(HOSTS)/sizeof(struct host); i++) {
            check_host(nc, HOSTS[i]);
        }
        sleep(30);
    }

    natsConnection_Close(nc);
    natsConnection_Destroy(nc);

    return 0;
}
