#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bootLib.h"
#include "gev.h"
#include "NVRAMaccess.h"

#define GEV_SIZE 3592

typedef struct gev_entry gev_entry_t;

struct gev_entry {
    char *start;
    size_t name_len;
    size_t value_len;
    gev_entry_t *prev;
};

#define value_of(e) (e->start + e->name_len + 1)
#define entry_size(e) (e->name_len + e->value_len + 2)

static struct {
    gev_entry_t *motscript;

    struct {
        gev_entry_t *cipa;
        gev_entry_t *sipa;
        gev_entry_t *gipa;
        gev_entry_t *snma;
        gev_entry_t *file;
    } enet[2];

    gev_entry_t *dns_server;
    gev_entry_t *dns_name;
    gev_entry_t *client_name;
    gev_entry_t *epics_script;
    gev_entry_t *epics_nfsmount;
    gev_entry_t *epics_ntpserver;
    gev_entry_t *epics_tz;

    gev_entry_t *rsh_user;
    gev_entry_t *tftp_pw;
    gev_entry_t *host_name;
    gev_entry_t *boot_flags;

    gev_entry_t *last;

    char data[GEV_SIZE];
} gev_index;

struct gev_special_entry {
    const char *name;
    gev_entry_t **entry;
};

/* NOTE: keep this array sorted by name */
static struct gev_special_entry gev_special[] = {
    { "boot-flags",             &gev_index.boot_flags },
    { "epics-nfsmount",         &gev_index.epics_nfsmount },
    { "epics-ntpserver",        &gev_index.epics_ntpserver },
    { "epics-script",           &gev_index.epics_script },
    { "epics-tz",               &gev_index.epics_tz },
    { "host-name",              &gev_index.host_name },
    { "mot-/dev/enet0-cipa",    &gev_index.enet[0].cipa },
    { "mot-/dev/enet0-file",    &gev_index.enet[0].file },
    { "mot-/dev/enet0-gipa",    &gev_index.enet[0].gipa },
    { "mot-/dev/enet0-sipa",    &gev_index.enet[0].sipa },
    { "mot-/dev/enet0-snma",    &gev_index.enet[0].snma },
    { "mot-/dev/enet1-cipa",    &gev_index.enet[1].cipa },
    { "mot-/dev/enet1-file",    &gev_index.enet[1].file },
    { "mot-/dev/enet1-gipa",    &gev_index.enet[1].gipa },
    { "mot-/dev/enet1-sipa",    &gev_index.enet[1].sipa },
    { "mot-/dev/enet1-snma",    &gev_index.enet[1].snma },
    { "mot-script-boot",        &gev_index.motscript },
    { "rsh-user",               &gev_index.rsh_user },
    { "rtems-client-name",      &gev_index.client_name },
    { "rtems-dns-domainname",   &gev_index.dns_name },
    { "rtems-dns-server",       &gev_index.dns_server },
    { "tftp-pw",                &gev_index.tftp_pw },
};

#define NUM_SPECIAL (sizeof(gev_special)/sizeof(struct gev_special_entry))

static void sstrncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n);
    if (n > 0) {
        dest[n - 1] = 0;
    }
}

static void dump_gev_entry(gev_entry_t *entry)
{
    printf("entry='%s',name_len=%d,value_len=%d\n",
        entry->start, entry->name_len, entry->value_len);
}

void dump_gev_index(void)
{
    gev_entry_t *e;
    for (e=gev_index.last; e; e = e->prev) {
        dump_gev_entry(e);
    }
}

static int compare_special(const void *key, const void *val)
{
    gev_entry_t *entry = (gev_entry_t *)key;
    struct gev_special_entry *special = (struct gev_special_entry *)val;
    return strncmp(entry->start, special->name, entry->name_len);
}

static struct gev_special_entry *find_special(gev_entry_t *entry)
{
    return bsearch(entry, gev_special, NUM_SPECIAL,
        sizeof(struct gev_special_entry), compare_special);
}

static void check_special(gev_entry_t *entry)
{
    struct gev_special_entry *found;

    found = find_special(entry);
    if (found) {
        *found->entry = entry;
    }
}

static gev_entry_t *new_gev_entry(void)
{
    gev_entry_t *entry = calloc(1, sizeof(gev_entry_t));
    entry->prev = gev_index.last;
    gev_index.last = entry;
    return entry;
}

static int build_index(void)
{
    static int index_built = 0;
    char *cur;
    char *end;

    if (index_built) {
        return 0;
    }
    index_built = 1;
    if (read_gev(gev_index.data, GEV_SIZE) < 0) {
        return -1;
    }
    cur = gev_index.data;
    end = cur + GEV_SIZE;

    while (cur < end && *cur) {
        char *sep = strchr(cur, '=');
        if (!sep) {
            /* no '=' found => syntax error, skip */
            fprintf(stderr, "gev_build_index: dropping malformed item '%s'\n", cur);
            cur = strchr(cur, 0) + 1;
        } else {
            gev_entry_t *entry = new_gev_entry();
            entry->start = cur;
            entry->name_len = sep - cur;
            entry->value_len = strnlen(sep + 1, end - sep);
            check_special(entry);
            cur += entry_size(entry);
        }
    }
    if (cur < end) {
        memset(cur, 0, end - cur);
    }
    return 0;
}

/* API */

void readNVram(BOOT_PARAMS *boot_params)
{
    int bootdev;
    char *tmp;

    build_index();

    if (gev_index.motscript)            /* get boot device */
        getsubstr(value_of(gev_index.motscript), boot_params->bootDev,
            BOOT_DEV_LEN, "-d/dev/");
    else
        sstrncpy(boot_params->bootDev, "enet0", BOOT_DEV_LEN);

    tmp = strpbrk(boot_params->bootDev, "0123456789");
    boot_params->unitNum = atol(tmp);   /* parse unit number */
    boot_params->bootDev[tmp - boot_params->bootDev] = 0; /* truncate */

    bootdev = boot_params->unitNum;
    if (bootdev == 0 || bootdev == 1) {

        if (gev_index.enet[bootdev].cipa) /* get client IP */
            sstrncpy(boot_params->ead, value_of(gev_index.enet[bootdev].cipa),
                BOOT_TARGET_ADDR_LEN);
        else
            getsubstr(value_of(gev_index.motscript), boot_params->ead,
                BOOT_TARGET_ADDR_LEN, "-c");

        boot_params->bad[0] = 0;        /* not used: backplane IP addr */
        boot_params->procNum = 0;       /* allways processor number 0 */

        if (gev_index.enet[bootdev].sipa) /* get host IP */
            sstrncpy(boot_params->had, value_of(gev_index.enet[bootdev].sipa),
                BOOT_ADDR_LEN);
        else
            getsubstr(value_of(gev_index.motscript), boot_params->had,
                BOOT_ADDR_LEN, "-s");

        if (gev_index.enet[bootdev].gipa) /* get gateway IP */
            sstrncpy(boot_params->gad, value_of(gev_index.enet[bootdev].gipa),
                BOOT_ADDR_LEN);
        else
            getsubstr(value_of(gev_index.motscript), boot_params->gad,
                BOOT_ADDR_LEN, "-g");

        if (gev_index.enet[bootdev].cipa->value_len + 9 <
            BOOT_TARGET_ADDR_LEN) {
            char buf[32];

            if (gev_index.enet[bootdev].snma) /* get subnetmask */
                sstrncpy(buf, value_of(gev_index.enet[bootdev].snma),
                    sizeof(buf));
            else
                getsubstr(value_of(gev_index.motscript), buf, sizeof(buf),
                    "-m");
            cvrtsmask(buf, buf + 1);
            buf[0] = ':';
            strcat(boot_params->ead, buf);
        }

        if (gev_index.enet[bootdev].file) /* get bootfile */
            sstrncpy(boot_params->bootFile,
                value_of(gev_index.enet[bootdev].file), BOOT_FILE_LEN);
        else
            getsubstr(value_of(gev_index.motscript), boot_params->bootFile,
                64, "-f");
    }

    if (gev_index.client_name)          /* get client name */
        sstrncpy(boot_params->targetName, value_of(gev_index.client_name),
            BOOT_HOST_LEN);
    else
        sstrncpy(boot_params->targetName, boot_params->ead, BOOT_HOST_LEN);

    if (gev_index.epics_script)         /* get startup script */
        sstrncpy(boot_params->startupScript,
            value_of(gev_index.epics_script), BOOT_FILE_LEN);
    else
        sstrncpy(boot_params->startupScript, "", BOOT_FILE_LEN);

    if (gev_index.epics_nfsmount)       /* get nfs server mount */
        sstrncpy(boot_params->other, value_of(gev_index.epics_nfsmount), BOOT_OTHER_LEN);
    else
        sstrncpy(boot_params->other, "", BOOT_OTHER_LEN);

    if (gev_index.rsh_user)             /* get user name */
        sstrncpy(boot_params->usr, value_of(gev_index.rsh_user), BOOT_USR_LEN);
    else
        sstrncpy(boot_params->usr, "", BOOT_USR_LEN);

    if (gev_index.tftp_pw)              /* get password */
        sstrncpy(boot_params->passwd, value_of(gev_index.tftp_pw), BOOT_PASSWORD_LEN);
    else
        sstrncpy(boot_params->passwd, "", BOOT_PASSWORD_LEN);

    if (gev_index.boot_flags)           /* get bootflags */
        boot_params->flags = strtol(value_of(gev_index.boot_flags), NULL, 0);
    else
        boot_params->flags = 0;

    if (gev_index.host_name)            /* get host_name */
        sstrncpy(boot_params->hostName, value_of(gev_index.host_name), BOOT_HOST_LEN);
    else
        sstrncpy(boot_params->hostName, "", BOOT_HOST_LEN);
}

void update_gev_entry(gev_entry_t **pentry, const char *name, const char *value)
{
    gev_entry_t *last = gev_index.last;
    char *end = last ? last->start + entry_size(last) : gev_index.data;
    int available = GEV_SIZE - (end - gev_index.data);
    int value_len = strlen(value);

    if (*pentry) {
        gev_entry_t *entry = *pentry;
        int diff = (int)value_len - (int)entry->value_len;
        char *next_start = entry->start + entry_size(entry);
        gev_entry_t *e;

#if 0
        fprintf(stderr, "new = '%s', diff = %d\n", value, diff);
#endif
        if (diff > available) {
            fprintf(stderr, "update_gev_entry: not enough space available in GEV\n"
                "while trying to write '%s=%s'\n", name, value);
        } else {
            memmove(next_start + diff, next_start, end - next_start);
            if (last && diff < 0) {
                /* zero out the freed area */
                memset(end + diff, 0, -diff);
            }
            for (e = gev_index.last; e && e != entry; e = e->prev) {
                e->start += diff;
            }
            strcpy(value_of(entry), value);
            e->value_len += diff;
        }
    } else {
        gev_entry_t *entry = new_gev_entry();
        int used = snprintf(end, available, "%s=%s", name, value);

        entry->start = end;
        if (used >= available) {
            fprintf(stderr, "update_gev_entry: not enough space available in GEV\n"
                "while trying to write '%s=%s'\n", name, value);
        } else {
            entry->name_len = strlen(name);
            entry->value_len = value_len;
            *pentry = entry;
        }
    }
}

#define ENET_NAME(n) "mot-/dev/enet" #n "-"

void writeNVram(BOOT_PARAMS * boot_params)
{
    char *netmask;
    int bootdev;
    struct {
        const char *cipa;
        const char *file;
        const char *gipa;
        const char *sipa;
        const char *snma;
    } enet_names;
    if (boot_params->unitNum != 1)
        boot_params->unitNum = 0;
    bootdev = boot_params->unitNum;
    if (bootdev) {
        enet_names.cipa = ENET_NAME(1) "cipa";
        enet_names.file = ENET_NAME(1) "file";
        enet_names.gipa = ENET_NAME(1) "gipa";
        enet_names.sipa = ENET_NAME(1) "sipa";
        enet_names.snma = ENET_NAME(1) "snma";
    } else {
        enet_names.cipa = ENET_NAME(0) "cipa";
        enet_names.file = ENET_NAME(0) "file";
        enet_names.gipa = ENET_NAME(0) "gipa";
        enet_names.sipa = ENET_NAME(0) "sipa";
        enet_names.snma = ENET_NAME(0) "snma";
    }

    /* set client IP */
    netmask = strchr(boot_params->ead, ':');
    if (netmask) {
        *netmask = 0;
    }
    update_gev_entry(&gev_index.enet[bootdev].cipa, enet_names.cipa, boot_params->ead);
    if (netmask) {
        *netmask = ':';
    }

    /* set host IP */
    update_gev_entry(&gev_index.enet[bootdev].sipa, enet_names.sipa, boot_params->had);

    /* set gateway IP */
    update_gev_entry(&gev_index.enet[bootdev].gipa, enet_names.gipa, boot_params->gad);

    /* set subnet mask */
    if (netmask) {
        uint32_t addr = strtoul(netmask + 1, NULL, 16);
        char buf[16];
        snprintf(buf, 16, "%lu.%lu.%lu.%lu",
            (addr >> 24), (addr >> 16) & 0xFF, (addr >> 8) & 0xFF,
            addr & 0xFF);
        update_gev_entry(&gev_index.enet[bootdev].snma, enet_names.snma, buf);
    } else {
        update_gev_entry(&gev_index.enet[bootdev].snma, enet_names.snma, "255.255.255.0");
    }

    /* set bootfile */
    update_gev_entry(&gev_index.enet[bootdev].file, enet_names.file, boot_params->bootFile);

    /* set client name */
    update_gev_entry(&gev_index.client_name, "rtems-client-name", boot_params->targetName);

    /* set script */
    update_gev_entry(&gev_index.epics_script, "epics-script", boot_params->startupScript);

    /* set other */
    update_gev_entry(&gev_index.epics_nfsmount, "epics-nfsmount", boot_params->other);

    /* set rsh_user */
    update_gev_entry(&gev_index.rsh_user, "rsh-user", boot_params->usr);

    /* set password */
    update_gev_entry(&gev_index.tftp_pw, "tftp-pw", boot_params->passwd);

    /* set hostname */
    update_gev_entry(&gev_index.host_name, "host-name", boot_params->hostName);

    /* set bootflags */
    {
        char buf[10];
        snprintf(buf, 10, "0x%i", boot_params->flags);
        update_gev_entry(&gev_index.boot_flags, "boot-flags", buf);
    }

    /* set motscript */
    {
        char value[sizeof(MOTSCRIPT_PART1)-1+BOOT_DEV_LEN+5+sizeof(MOTSCRIPT_PART2)-1];
        /* unitnum needs max 5 chars */

        sprintf(value, "%s%s%i%s", MOTSCRIPT_PART1,
            boot_params->bootDev, boot_params->unitNum,
            MOTSCRIPT_PART2);
        update_gev_entry(&gev_index.motscript, "mot-script-boot", value);
    }

    write_gev(gev_index.data, GEV_SIZE);
}

void gevShow(void)
{
    char tmp[GEV_SIZE];
    read_gev(tmp, GEV_SIZE);
    char *cur = tmp;
    char *end = tmp + GEV_SIZE;

    while (cur < end && *cur) {
        printf("%s\n", cur);
        cur += strlen(cur) + 1;
    }
}

void gevDelete(char *name)
{
    gev_entry_t *e = gev_index.last;
    gev_entry_t *h = 0;
    struct gev_special_entry *special;

    for (e = gev_index.last; e && strncmp(e->start, name, e->name_len)!=0; e = e->prev) {
        h = e;
    }
    if (e) {
        int diff = entry_size(e);
        char *end = gev_index.last->start + entry_size(gev_index.last);
#if 0
        dump_gev_entry(e);
#endif
        if (h) {
            h->prev = e->prev;
        }
        h = e->prev;
        special = find_special(e);
        if (special)
            *special->entry = 0;
        memmove(e->start, e->start + diff, end - (e->start + diff));
        /* zero out the freed area */
        memset(end - diff, 0, diff);
        free(e);
        for (e = gev_index.last; e && e != h; e = e->prev) {
            e->start -= diff;
        }
        write_gev(gev_index.data, GEV_SIZE);
    } else {
        fprintf(stderr, "gevDelete: key '%s' not found\n", name);
    }
}

void gevUpdate(const char *name, const char *value)
{
    gev_entry_t *e = gev_index.last;

    while (e && strncmp(e->start, name, e->name_len)!=0) {
        e = e->prev;
    }
    update_gev_entry(&e, name, value);
    write_gev(gev_index.data, GEV_SIZE);
}
