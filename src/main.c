#define _POSIX_C_SOURCE 200809L
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

int drivers(void) {
    int driverinput;
    
    printf("[1] AMD/ATI\n"
           "[2] Intel\n"
           "[3] Nvidia [Official Proprietary 'nvidia' driver]\n"
           "[4] Nvidia (Open-Source Community 'nouveau' driver)\n\n");
    printf("[?] > ");

    if (scanf("%d", &driverinput) != 1) {
        fprintf(stderr, "Invalid input.\n");
        // clear the bad input from stdin so it doesn't loop forever elsewhere
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        return 1;
    }

    switch (driverinput) {
        case 1:
            /* 
            echo 'LIBVA_DRIVER_NAME=radeonsi' >> /etc/environment
            echo 'VDPAU_DRIVER=va_gl' >> /etc/environment 
            */
            printf("Selected: AMD/ATI\n");
            sleep(3);
            TRY("/usr/bin/xbps-install -Sy linux-firmware-amd mesa-dri vulkan-loader mesa-vulkan-radeon amdvlk xf86-video-amdgpu xf86-video-ati mesa-vaapi libvdpau-va-gl");
            TRY("grep -qxF 'LIBVA_DRIVER_NAME=radeonsi' /etc/environment || echo 'LIBVA_DRIVER_NAME=radeonsi' >> /etc/environment"); // made by ai
            TRY("grep -qxF 'VDPAU_DRIVER=va_gl' /etc/environment || echo 'VDPAU_DRIVER=va_gl' >> /etc/environment"); // made by ai
            break;
        
        case 2:
            printf("Selected: Intel\n");
            sleep(3);
            // again starting ai cuz i am too dumb
            TRY("/usr/bin/xbps-install -Sy linux-firmware-intel mesa-dri vulkan-loader mesa-vulkan-intel intel-video-accel libvdpau-va-gl");
            TRY("grep -qxF 'VDPAU_DRIVER=va_gl' /etc/environment || echo 'VDPAU_DRIVER=va_gl' >> /etc/environment");
            // end ai
            break;
        
        case 3:
            printf("Selected: Nvidia (Proprietary)\n");
            // start ai cuz i am too lazy and dumb
            sleep(2);
    
            printf("Detected GPU info:\n");
            TRY("/usr/bin/lspci -k -d ::03xx | grep -i nvidia");
        
            FILE *fp = popen("/usr/bin/lspci -d ::03xx | grep -i nvidia", "r");
            if (!fp) { perror("popen failed"); break; }
        
            char line[512] = {0};
            fgets(line, sizeof(line), fp);
            pclose(fp);
        
            const char *pkg = NULL;
            
            // p.s. with titan cards may be bug
            
            if (strstr(line, "RTX 20") || strstr(line, "RTX 30") || strstr(line, "RTX 40") ||
                strstr(line, "RTX 50") || strstr(line, "GTX 16") || strstr(line, "TITAN RTX")) {
                pkg = "nvidia"; // Turing and newer
            } else if (strstr(line, "GTX 9") || strstr(line, "GTX 10") || strstr(line, "TITAN X") ||
                       strstr(line, "TITAN V")) {
                pkg = "nvidia580"; // Maxwell - Volta
            } else if (strstr(line, "GTX 7") || strstr(line, "GTX 6") || strstr(line, "TITAN\n") ||
                       strstr(line, "TITAN Z") || strstr(line, "TITAN Black")) {
                pkg = "nvidia470"; // Kepler 
            } else if (strstr(line, "GTX 5") || strstr(line, "GTX 4") || strstr(line, "GT 5") ||
                       strstr(line, "GT 4")) {
                pkg = "nvidia390"; // Fermi
            }
        
            if (!pkg) {
                printf("Could not auto-detect GPU family from: %s\n", line);
                printf("Please check manually: lspci -k -d ::03xx\n");
                printf("And consult: https://nouveau.freedesktop.org/CodeNames.html\n");
                break;
            }
        
            printf("Detected family package: %s\n", pkg);
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "/usr/bin/xbps-install -S %s", pkg);
            sleep(1);
            TRY(cmd);
            break;
        // end ai
        
        case 4:
            printf("Selected: Nvidia (Nouveau)\n");
            sleep(3);
            // start ai cuz i am too lazy and dumb x2
            TRY("/usr/bin/xbps-install -Sy mesa-dri vulkan-loader mesa-vulkan-nouveau xf86-video-nouveau mesa-vaapi libvdpau-va-gl");
            TRY("grep -qxF 'LIBVA_DRIVER_NAME=nouveau' /etc/environment || echo 'LIBVA_DRIVER_NAME=nouveau' >> /etc/environment");
            TRY("grep -qxF 'VDPAU_DRIVER=va_gl' /etc/environment || echo 'VDPAU_DRIVER=va_gl' >> /etc/environment");
            // end ai
            break;
        default:
            printf("Invalid selection: %d\n", driverinput);
            sleep(1);
            return drivers(); // recursive bla bla bla i am too dumb to make int done = 0;
            break;
    }
    return 0;
}

int installaudio(void) {
    // ai yes aiii iksdfjkldfjklfjkl s i am lazy and dumb and this really working
    const char *sudo_user = getenv("SUDO_USER");
    char cmd[512];

    if (sudo_user && strcmp(sudo_user, "root") != 0) {
        snprintf(cmd, sizeof(cmd),
            "sudo -u %s mkdir -p ~%s/.config/autostart", sudo_user, sudo_user);
        TRY(cmd);

        snprintf(cmd, sizeof(cmd),
            "sudo -u %s ln -sf /usr/share/applications/pipewire.desktop ~%s/.config/autostart/",
            sudo_user, sudo_user);
        TRY(cmd);

        snprintf(cmd, sizeof(cmd),
            "sudo -u %s ln -sf /usr/share/applications/pipewire-pulse.desktop ~%s/.config/autostart/",
            sudo_user, sudo_user);
        TRY(cmd);
    } else {
        TRY("mkdir -p ~/.config/autostart");
        TRY("ln -sf /usr/share/applications/pipewire.desktop ~/.config/autostart/");
        TRY("ln -sf /usr/share/applications/pipewire-pulse.desktop ~/.config/autostart/");
    }

    return 0;
}

void IMPORTANT(void) {     // VERY FUCKING IMPORTANT
    TRY("/usr/bin/xbps-install -Sy wget");
    TRY("/usr/bin/wget https://cdn.displate.com/artwork/270x380/2025-12-08/a1141783-44d3-41bc-acdf-4e8fd041cf36.jpg");
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
    
    TRY("/usr/bin/xbps-install -Sy void-repo-nonfree"); // install repo for propietary nvidia driver
    
    TRY("/usr/bin/xbps-install -Sy");

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
    
    printf("installing kde6 (plasma), sddm...\n");
    TRY("/usr/bin/xbps-install -y xorg kde5 kde5-baseapps sddm");
    
    if (drivers() != 0) {
        fprintf(stderr, "driver installation failed\n");
        sleep(3);
        return 1;
    }
    
    if (ask_user("install firefox?")) {
        TRY("/usr/bin/xbps-install -y firefox");
    }

    printf("installing sddm symlink...\n");
    TRY("/usr/bin/ln -sf /etc/sv/sddm /var/service");

    printf("installing audio...\n");
    installaudio(); 
    
    printf("INSTALLING IMPORTANT FILE!!!1!1 PLEASE DONT TYPE CTRL + C THIS FILE IS VER4Y IMPORTANT\n");
    IMPORTANT(); // VERY FUCKING IMPORTANT

    if (ask_user("install os prober?")) {
        TRY("/usr/bin/xbps-install -Sy os-prober");
        TRY("grep -qxF 'GRUB_DISABLE_OS_PROBER=false' /etc/default/grub || "
        "echo 'GRUB_DISABLE_OS_PROBER=false' >> /etc/default/grub");
        TRY("/usr/bin/grub-mkconfig -o /boot/grub/grub.cfg");
    }
    
    printf("installation finished successfully!\n");
    
    if (ask_user("reboot?")) {
        TRY("/usr/bin/reboot");
    }
    
    return 0;
}
