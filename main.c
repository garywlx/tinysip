/* $Id$ */
/*
 * Copyright (C) 2010 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Sample CLI application
 */
#include <pjlib-util/cli.h>
#include <pjlib-util/cli_imp.h>
#include <pjlib-util/cli_console.h>
#include <pjlib-util/cli_telnet.h>
#include <pjlib-util/errno.h>
#include <pjlib.h>
#include <pjsua-lib/pjsua.h>
#include <time.h>

#define THIS_FILE "clidemo.c"

#define ACC_CFG "acc.ini"
#define LOG_LEVEL 0

/* Set this to 1 if you want to let the system assign a port
 * for the CLI telnet daemon.
 * Default: 1
 */
#define USE_RANDOM_PORT 1

struct cmd_xml_t {
    char *xml;
    pj_cli_cmd_handler handler;
};

/*
 * Declaration of system specific main loop, which will be defined in
 * a separate file.
 */
pj_status_t app_main(pj_cli_t *cli);
pjsua_acc_id main_account; //main account


#define print_msg(arg)                                                         \
    do {                                                                       \
        unsigned d = pj_log_get_decor();                                       \
        pj_log_set_decor(0);                                                   \
        PJ_LOG(1, arg);                                                        \
        pj_log_set_decor(d);                                                   \
    } while (0)

static pj_cli_t *cli = NULL;

static pj_status_t quit_app(pj_cli_cmd_val *cval) {
    PJ_UNUSED_ARG(cval);
    pj_cli_quit(cval->sess->fe->cli, cval->sess, PJ_FALSE);

    return PJ_CLI_EEXIT;
}

static pj_status_t list_account(pj_cli_cmd_val *cval) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(ACC_CFG, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int _i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        _i++;
        printf("ACC %d: %s", _i, line);
    }

    fclose(fp);
    if (line)
        free(line);
    return PJ_SUCCESS;
}

/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata) {
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

//    PJ_LOG(3, (THIS_FILE, "Got Incoming call from %.*s!!", (int)ci.remote_info.slen,
//               ci.remote_info.ptr));

    /* Automatically answer incoming calls with 200/OK */
    printf("Auto answer with code 200");
    pjsua_call_answer(call_id, 200, NULL, NULL);
    return PJ_SUCCESS;
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e) {
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);
//    PJ_LOG(3, (THIS_FILE, "Call %d state=%.*s", call_id,
//               (int)ci.state_text.slen, ci.state_text.ptr));
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id) {
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // When media is active, connect call to sound device.
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status) {
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}


static pj_status_t register_account(pj_cli_cmd_val *cval) {
    {
        print_msg(("", "Registering account sip:%.*s@%.*s  ... ",
                   (int)cval->argv[1].slen, cval->argv[1].ptr,
                   (int)cval->argv[3].slen, cval->argv[3].ptr));

        pjsua_acc_config cfg;
        pj_status_t status;

        pjsua_acc_config_default(&cfg);
        pj_str_t acc_name = cval->argv[1];
        pj_str_t acc_pwd = cval->argv[2];
        pj_str_t acc_domain = cval->argv[3];

        char buff1[acc_name.slen+acc_domain.slen+20];
        pj_str_t id;
        id.ptr = buff1;
        pj_bzero(buff1, sizeof(buff1));
        pj_strcat2(&id, "sip:");
        pj_strcat(&id, &acc_name);
        pj_strcat2(&id, "@");
        pj_strcat(&id, &acc_domain);

        char buff2[acc_domain.slen+20];
        pj_str_t reg_uri;
        reg_uri.ptr = buff2;
        pj_bzero(buff2, sizeof(buff2));
        pj_strcat2(&reg_uri, "sip:");
        pj_strcat(&reg_uri, &acc_domain);

//        char id[acc_name.slen+acc_domain.slen+10];
//        char reg_uri[acc_domain.slen+10];
//        pj_ansi_sprintf(id, "sip:%s@%s",
//                acc_name.ptr, acc_domain.ptr);
//        pj_ansi_sprintf(reg_uri, "sip:%s",
//                acc_domain.ptr);

        cfg.id = id;
        cfg.reg_uri = reg_uri;
        cfg.cred_count = 1;
        cfg.cred_info[0].realm = pj_str("*"); //or set to acc_domain
        cfg.cred_info[0].scheme = pj_str("digest");
        cfg.cred_info[0].username = acc_name;
        cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
        cfg.cred_info[0].data = acc_pwd;

        status = pjsua_acc_add(&cfg, PJ_TRUE, &main_account);
        printf("done \r\n");
        if (status != PJ_SUCCESS)
            error_exit("Error adding account", status);
    }
    return PJ_SUCCESS;
}
static pj_status_t make_call(pj_cli_cmd_val *cval) {
    print_msg(("", "Making a call to %.*s!\r\n", (int)cval->argv[1].slen,
               cval->argv[1].ptr));
    return PJ_SUCCESS;
}


static void get_cmd_list(pj_cli_dyn_choice_param *param) {
    //printf("...");
    return PJ_SUCCESS;
}

static struct cmd_xml_t cmd_xmls[] = {

    {"<CMD name='list' id='1' sc='ls' desc='List account'>"
     "</CMD>",
     &list_account},
    {"<CMD name='register' id='2' sc='r,reg,re' desc='Register account for calling'>"
     "    <ARG name='username' type='text' desc='Account user'/>"
     "    <ARG name='password' type='text' desc='Account password'/>"
     "    <ARG name='domain' type='text' desc='SIP domain'/>"
     "</CMD>",
     &register_account},
    {"<CMD name='call' id='3' sc='c,ca' desc='Call account ID'>"
     "    <ARG name='acc_id' type='int' desc=''/>"
     "</CMD>",
     &make_call},
    {"<CMD name='quit_app' id='999' sc='q,qa' desc='Quit the application'>"
     "</CMD>",
     &quit_app},
};

static void log_writer(int level, const char *buffer, int len) {
    if (cli)
        pj_cli_write_log(cli, level, buffer, len);
}

void read_account_file() {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(ACC_CFG, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);
    }

    fclose(fp);
    if (line)
        free(line);

    return PJ_SUCCESS;
}

int main() {
    pj_caching_pool cp;
    pj_cli_cfg cli_cfg;
    pj_cli_telnet_cfg tcfg;
    pj_str_t xml;
    pj_status_t status;
    pj_status_t cli_status;
    int i;
    pj_log_set_level(LOG_LEVEL);
    print_msg(("", "INITING .... \n"));
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    if (status != PJ_SUCCESS) {
        print_msg(("", "PJ INIT FAILED \n"));
    }

    pj_caching_pool_init(&cp, NULL, 0);
    pjlib_util_init();
    /*
     * Create CLI app.
     */
    pj_cli_cfg_default(&cli_cfg);

    print_msg(("", "INITING CLI CFG....  \n"));
    cli_cfg.pf = &cp.factory;
    cli_cfg.name = pj_str("tinysipcli");
    cli_cfg.title = pj_str("TINYSIP");

    cli_status = pj_cli_create(&cli_cfg, &cli);
    print_msg(("", "AFTER INITING CLI CFG....  \n"));

    if (cli_status != PJ_SUCCESS) {
        print_msg(("", "CLI FAILED \n"));
        goto on_return;
    }

    /*
     * Register some commands.
     */
    for (i = 0; i < sizeof(cmd_xmls) / sizeof(cmd_xmls[0]); i++) {
        xml = pj_str(cmd_xmls[i].xml);
//        print_msg(("",cmd_xmls[i].xml));
        status = pj_cli_add_cmd_from_xml(cli, NULL, &xml, cmd_xmls[i].handler,
                                         NULL, get_cmd_list);
        if (status != PJ_SUCCESS)
            continue;
//        goto on_return;
    }
    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS)
        error_exit("Error in pjsua_create()", status);

    /* Init pjsua */
    {
        pjsua_logging_config log_cfg;
        pjsua_logging_config_default(&log_cfg);
        log_cfg.console_level = LOG_LEVEL;

        pjsua_config cfg;
        pjsua_config_default(&cfg);


        cfg.cb.on_incoming_call = &on_incoming_call;
        cfg.cb.on_call_media_state = &on_call_media_state;
        cfg.cb.on_call_state = &on_call_state;

        status = pjsua_init(&cfg, &log_cfg, NULL);
        if (status != PJ_SUCCESS)
            error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
        pjsua_transport_config cfg;

        pjsua_transport_config_default(&cfg);
        cfg.port = 5060;
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
        if (status != PJ_SUCCESS)
            error_exit("Error creating transport", status);
    }


    /*
     * Start telnet daemon
     */
    pj_cli_telnet_cfg_default(&tcfg);
//    tcfg.passwd = pj_str("pjsip");
#if USE_RANDOM_PORT
    tcfg.port = 0;
#else
    tcfg.port = 2233;
#endif
    tcfg.prompt_str = pj_str("CMD % ");
    status = pj_cli_telnet_create(cli, &tcfg, NULL);
    if (status != PJ_SUCCESS)
        goto on_return;

    /*
     * Run the system specific main loop.
     */
    status = app_main(cli);

on_return:

    print_msg(("", "There're some issues, application is going down now!\n"));
    /*
     * Destroy
     */
    pj_cli_destroy(cli);
    cli = NULL;
    pj_caching_pool_destroy(&cp);
    pj_shutdown();

    return (status != PJ_SUCCESS ? 1 : 0);
}

/*xxxxxxxxxxxxxxxxxxxxxxxxxxxxx main_console.c xxxxxxxxxxxxxxxxxxxxxxxxxxxx */
/*
 * Simple implementation of app_main() for console targets
 */
pj_status_t app_main(pj_cli_t *cli) {
    print_msg(("", "APP STARTED \n \n \n"));
    print_msg(("", "======================================================================\n"));
    pj_status_t status;
    pj_cli_sess *sess;
    pj_cli_console_cfg console_cfg;

    pj_cli_console_cfg_default(&console_cfg);
    console_cfg.prompt_str = pj_str("CMD> ");

    /*
     * Create the console front end
     */
    status = pj_cli_console_create(cli, &console_cfg, &sess, NULL);
    if (status != PJ_SUCCESS)
        return status;

    pj_log_set_log_func(&log_writer);

    /*
     * Main loop.
     */
    for (;;) {
        char cmdline[PJ_CLI_MAX_CMDBUF];
        pj_status_t status;

        status = pj_cli_console_process(sess, &cmdline[0], sizeof(cmdline));
        if (status != PJ_SUCCESS)
            break;

        // pj_ansi_strcpy(cmdline, "sayhello {Teluu Inc.}");
        if (status == PJ_CLI_EEXIT) {
            /* exit is called */
            break;
        } else if (status != PJ_SUCCESS) {
            /* Something wrong with the cmdline */
            PJ_PERROR(1, (THIS_FILE, status, "Exec error"));
        }
    }

    return PJ_SUCCESS;
}
