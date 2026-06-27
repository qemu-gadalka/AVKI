#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TRY(cmd) do { \
    if (system(cmd) != 0) { \
        fprintf(stderr, "error: command failed -> %s\n", cmd); \
        return 1; \
    } \
} while(0)

int ask_user(const char *question) {
    char answer;
    printf("%s [y/n]: ", question);
    if (scanf(" %c", &answer) != 1) {
        return 0;
    }
    return (answer == 'y' || answer == 'Y');
}

int main() {
    if (getuid() != 0) {
        fprintf(stderr, "error: this installer must be run as root (sudo)!\n");
        return 1;
    }

    printf("checking internet connection...\n");
    if (system("/usr/bin/ping -c 1 -W 2 1.1.1.1 > /dev/null 2>&1") != 0) {
        fprintf(stderr, "error: no internet connection!\n");
        return 1;
    }
    
    if (ask_user("update repos?")) {
        TRY("/usr/bin/xbps-install -S");
    }

    printf("installing xdg-user-dirs\n");
    TRY("/usr/bin/xbps-install -y xdg-user-dirs");

    printf("updating xdg-user-dirs...\n");
    
    const char *sudo_user = getenv("SUDO_USER");
    if (sudo_user && strcmp(sudo_user, "root") != 0) {
        char xdg_cmd[256];
        snprintf(xdg_cmd, sizeof(xdg_cmd), "sudo -u %s /usr/bin/xdg-user-dirs-update", sudo_user);
        TRY(xdg_cmd);
    } else {
        TRY("/usr/bin/xdg-user-dirs-update");
    }

    printf("installing xdg-utils, net-tools\n");
    TRY("/usr/bin/xbps-install -y xdg-utils net-tools");

    printf("installing base components\n");
    TRY("/usr/bin/xbps-install -y elogind dbus-elogind polkit-elogind rtkit NetworkManager pipewire wireplumber");

    printf("making symlinks...\n");
    TRY("/usr/bin/ln -sf /etc/sv/NetworkManager /var/service");
    TRY("/usr/bin/ln -sf /etc/sv/dbus /var/service");
    TRY("/usr/bin/ln -sf /etc/sv/polkitd /var/service");
    TRY("/usr/bin/ln -sf /etc/sv/rtkit /var/service");
    TRY("/usr/bin/ln -sf /etc/sv/pipewire /var/service");
    TRY("/usr/bin/ln -sf /etc/sv/wireplumber /var/service");
    
    printf("installing kde6 (plasma), sddm...\n");
    TRY("/usr/bin/xbps-install -y xorg-minimal kde5 kde5-baseapps sddm");

    if (ask_user("install firefox?")) {
        TRY("/usr/bin/xbps-install -y firefox");
    }

    printf("installing sddm symlink...\n");
    TRY("/usr/bin/ln -sf /etc/sv/sddm /var/service");

    printf("installation finished successfully!\nIf SDDM does not start, you can try rebooting.\n");
    return 0;
}
