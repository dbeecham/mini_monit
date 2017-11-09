#include <nats/nats.h>
#include <time.h>
#include <signal.h>
#include <err.h>
#include <jsmn.h>
#include <string.h>

void its_dead(void) {
    printf("SERVICE IS DOWN! ABANDON SHIP! CALL THE FIRE DEPARTMENT!\n");
}

void notify(union sigval s) {
    its_dead();
}

static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void * closure) {

    const char * connect_time = 0; 
    const char * fin_connect_time = 0; 
    const char * response_code = 0; 
    const char * fin_response_code = 0; 
    const char * ssl_connect_time = 0; 
    const char * fin_ssl_connect_time = 0; 
    const char * ttfb = 0;
    const char * fin_ttfb = 0;
    const char * total_time = 0;
    const char * fin_total_time = 0;

    %%{
        machine myjson;

        action connect_time { connect_time = p; }
        action fin_connect_time { fin_connect_time = p; }
        action response_code { response_code = p; }
        action fin_response_code { fin_response_code = p; }
        action ssl_connect_time { ssl_connect_time = p; }
        action fin_ssl_connect_time { fin_ssl_connect_time = 0; }
        action ttfb { ttfb = 0; }
        action fin_ttfb { fin_ttfb = 0; }
        action total_time { total_time = 0; }
        action fin_total_time { fin_total_time = 0; }
 
        response_code = '"response_code":' space* digit+ >response_code ',' @ fin_response_code;

        connect_time = '"connect_time":' space* digit+ >connect_time '.' digit+ ',' @fin_connect_time;

        ssl_connect_time = '"ssl_connect_time":' space* digit+ >ssl_connect_time '.' digit+ ',' @fin_ssl_connect_time;

        ttfb = '"ttfb":' space* digit+ >ttfb '.' digit+ ',' @fin_ttfb;

        total_time = '"total_time":'
                     space*
                     digit+ >total_time
                     '.'
                     digit+
                     ',' @fin_total_time;

        json_options = (response_code | connect_time | ssl_connect_time | ttfb | total_time)+;

        main := '{' json_options;

    }%%

    timer_t * timerid = (timer_t*)closure;

    struct itimerspec its = {0};
    its.it_value.tv_sec = 60;
    timer_delete(*timerid);
    struct sigevent sev = {0};
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = notify;
    if (-1 == timer_create(CLOCK_REALTIME, &sev, timerid)) {
        err(1, "timer_create");
    }
    if (-1 == timer_settime(*timerid, 0, &its, NULL)) {
        err(1, "timer_settime");
    }

    const char * const data = natsMsg_GetData(msg);
    const char *p = data;
    const char *pe = data + strlen(data);
    int cs;

    %% write data;
    %% write init;
    %% write exec;

    int i_response_code = atoi(response_code);

    if (200 != i_response_code) {
        its_dead();
    }

    printf("msg: %s - %.*s\n", natsMsg_GetSubject(msg), natsMsg_GetDataLength(msg), natsMsg_GetData(msg));
    natsMsg_Destroy(msg);
}


int main(int argc, char *argv[])
{
    natsConnection * conn = NULL;
    natsOptions * opts = NULL;
    natsSubscription * sub = NULL;
    natsStatistics * stats = NULL;
    natsMsg * msg = NULL;
    natsStatus s;

    s = natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
    if (NATS_OK != s) {
        printf("error: %s\n", natsStatus_GetText(s));
        return 1;
    }

    struct sigevent sev = {0};
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = notify;

    timer_t timerid = 0;
    if (-1 == timer_create(CLOCK_REALTIME, &sev, &timerid)) {
        err(1, "timer_create");
    }

    s = natsConnection_Subscribe(&sub, conn, "s.http.pluspole.se", onMsg, &timerid);

    struct itimerspec its = {0};
    its.it_value.tv_sec = 60;
    if (-1 == timer_settime(timerid, 0, &its, NULL)) {
        err(1, "timer_settime");
    }

    while (NATS_OK == s) {
        nats_Sleep(100);
    }

    return 0;
}
