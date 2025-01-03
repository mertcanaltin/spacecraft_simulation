#include <stdio.h>
#include <stdbool.h>
#include <unistd.h> // sleep() fonksiyonu için
#include <math.h>   // Trigonometrik hesaplamalar için
#include <time.h>   // Zamanlama için

// Mekik Durumları
typedef enum
{
    IDLE,
    LAUNCH,
    ORBIT,
    RETURN,
    LANDED
} SpacecraftState;

// Mekik Verileri
typedef struct
{
    SpacecraftState state;
    double fuel_level;    // % olarak yakıt seviyesi
    double altitude;      // km cinsinden irtifa
    double velocity;      // km/s cinsinden hız
    bool systems_nominal; // Sistemlerin durumunu takip eder
    double latitude;      // Enlem
    double longitude;     // Boylam
    time_t fault_time;    // Arıza zamanı
} Spacecraft;

void emergency_protocol(Spacecraft *craft);
void calculate_safe_return(Spacecraft *craft);

// Hata Günlüğü
void log_error(const char *message)
{
    FILE *log_file = fopen("error_log.txt", "a");
    if (log_file)
    {
        fprintf(log_file, "[%ld] %s\n", time(NULL), message);
        fclose(log_file);
    }
    printf("[ERROR] %s\n", message);
}

// Yedek Sistem
bool activate_backup_system(Spacecraft *craft)
{
    printf("[BACKUP] Yedek sistem devreye alınıyor...\n");
    craft->systems_nominal = true; // Yedek sistem aktif
    return true;
}

// Yakıt Tüketimi Fonksiyonu
void consume_fuel(Spacecraft *craft, double base_consumption)
{
    double dynamic_consumption = base_consumption + (craft->velocity * 0.1) + (craft->altitude * 0.01);
    if (craft->fuel_level > dynamic_consumption)
    {
        craft->fuel_level -= dynamic_consumption;
    }
    else
    {
        craft->fuel_level = 0;
        craft->systems_nominal = false;
        log_error("Yakıt kritik seviyede bitti.");
    }
}

// Telemetri Gönderimi
void send_telemetry(Spacecraft *craft)
{
    printf("[TELEMETRY] Durum: %s, Enlem: %.4f, Boylam: %.4f, Yükseklik: %.1f km, Hız: %.1f km/s, Yakıt: %.1f%%\n",
           craft->state == IDLE ? "Bekleme" : craft->state == LAUNCH ? "Kalkış"
                                          : craft->state == ORBIT    ? "Yörüngede"
                                          : craft->state == RETURN   ? "Dönüş"
                                                                     : "İniş",
           craft->latitude, craft->longitude, craft->altitude, craft->velocity, craft->fuel_level);
}

// Güvenlik Kontrolleri
void safety_checks(Spacecraft *craft)
{
    if (craft->fuel_level <= 10.0)
    {
        log_error("Yakıt kritik seviyeye düştü!");
        emergency_protocol(craft);
    }
    if (craft->altitude > 300.0)
    {
        log_error("İrtifa güvenli limitlerin üzerinde!");
        emergency_protocol(craft);
    }
    if (!craft->systems_nominal)
    {
        log_error("Sistem nominal değil!");
        emergency_protocol(craft);
    }
}

// Acil Durum Protokolü
void emergency_protocol(Spacecraft *craft)
{
    printf("[EMERGENCY] Acil durum tespit edildi! Güvenli eve dönüş başlatılıyor...\n");
    calculate_safe_return(craft);
}

// Güvenli Eve Dönüş Rotası
void calculate_safe_return(Spacecraft *craft)
{
    if (craft->fuel_level <= 0)
    {
        printf("[CRITICAL] Yakıt tükendi! Alternatif prosedürler devreye alınıyor...\n");
        craft->velocity = 0.0;                           // Hız kesiliyor
        craft->altitude = fmax(craft->altitude - 50, 0); // Yörüngede kalmayı simüle et
        send_telemetry(craft);
        log_error("Yakıt tükendi, yörünge koruma protokolü başlatıldı.");
        return;
    }

    printf("[SAFETY] Güvenli eve dönüş rotası hesaplanıyor...\n");
    craft->velocity = 1.0;
    craft->altitude = 100;
    while (craft->altitude > 0)
    {
        consume_fuel(craft, 1.0);
        craft->altitude -= 10;
        craft->velocity -= 0.1;
        if (craft->velocity < 0)
            craft->velocity = 0;
        send_telemetry(craft);
        sleep(1);
    }
    craft->state = LANDED;
    printf("[SAFETY] Mekik güvenli şekilde Dünya'ya indi.\n");
}

void validate_return_route(Spacecraft *craft)
{
    if (craft->latitude > 90.0)
        craft->latitude = 90.0;
    if (craft->latitude < -90.0)
        craft->latitude = -90.0;
    if (craft->longitude > 180.0)
        craft->longitude -= 360.0;
    if (craft->longitude < -180.0)
        craft->longitude += 360.0;

    if (craft->altitude > 300.0 || craft->altitude < 0.0)
    {
        printf("[VALIDATION] Rotada sapma tespit edildi! Dünya'ya yönlendiriliyor...\n");
        craft->altitude = 100.0;
        craft->velocity = 1.0;
    }
    send_telemetry(craft);
}

void initialize_spacecraft(Spacecraft *craft)
{
    craft->state = IDLE;
    craft->fuel_level = 100.0; // %100 dolu yakıt
    craft->altitude = 0.0;
    craft->velocity = 0.0;
    craft->systems_nominal = true;
    craft->latitude = 37.1054;  // Muğla, Akyaka başlangıç enlemi
    craft->longitude = 28.3271; // Muğla, Akyaka başlangıç boylamı
    craft->fault_time = 0;
}

void launch(Spacecraft *craft)
{
    printf("[LAUNCH] Kalkış başlıyor!\n");
    craft->state = LAUNCH;
    while (craft->altitude < 100 && craft->fuel_level > 0)
    { // 100 km yörünge sınırı
        consume_fuel(craft, 2.0);
        craft->altitude += 10;
        craft->velocity += 1;
        send_telemetry(craft);
        safety_checks(craft);
        sleep(1);
    }
    if (craft->fuel_level > 0)
    {
        printf("[LAUNCH] Mekik yörüngeye ulaştı!\n");
        craft->state = ORBIT;
    }
    else
    {
        printf("[ERROR] Yakıt bitti, yörüngeye ulaşılamadı.\n");
        craft->state = IDLE;
    }
}

void update_coordinates(Spacecraft *craft)
{
    craft->longitude += craft->velocity * 0.01; // Boylam değişimi
    craft->latitude += craft->velocity * 0.005; // Enlem değişimi

    if (craft->longitude > 180.0)
        craft->longitude -= 360.0;
    if (craft->longitude < -180.0)
        craft->longitude += 360.0;
    if (craft->latitude > 90.0)
        craft->latitude = 90.0;
    if (craft->latitude < -90.0)
        craft->latitude = -90.0;
}

void orbit(Spacecraft *craft)
{
    printf("[ORBIT] Yörüngede stabilizasyon sağlanıyor.\n");
    for (int i = 0; i < 5; i++)
    {
        if (!craft->systems_nominal)
        {
            printf("[ERROR] Sistem arızası! Yörünge stabilizasyonu başarısız.\n");
            craft->fault_time = time(NULL);
            return;
        }
        consume_fuel(craft, 1.0);
        update_coordinates(craft);
        validate_return_route(craft);
        send_telemetry(craft);
        sleep(1);
    }
    printf("[ORBIT] Yörünge stabilizasyon tamamlandı. Eve dönüş için komut bekleniyor...\n");
}

void return_to_earth(Spacecraft *craft)
{
    printf("[RETURN] Dünya'ya dönüş başlıyor.\n");
    craft->state = RETURN;
    while (craft->altitude > 0)
    {
        consume_fuel(craft, 1.5);
        craft->altitude -= 10;
        craft->velocity -= 0.5;
        if (craft->velocity < 0)
            craft->velocity = 0;
        validate_return_route(craft);
        send_telemetry(craft);
        sleep(1);
    }

    if (craft->altitude <= 0)
    {
        printf("[RETURN] Mekik güvenli şekilde Dünya'ya indi.\n");
        printf("Son Koordinatlar: (Enlem: %.4f, Boylam: %.4f)\n", craft->latitude, craft->longitude);
        craft->state = LANDED;
    }
}

int main()
{
    Spacecraft craft;
    initialize_spacecraft(&craft);

    // Görev Aşamaları
    launch(&craft);

    if (craft.state == ORBIT)
    {
        orbit(&craft);

        if (!craft.systems_nominal && difftime(time(NULL), craft.fault_time) >= 30 * 60)
        {
            emergency_protocol(&craft);
        }
        else
        {
            char command;
            printf("[COMMAND] Eve dönüş için 'r' tuşuna basın: ");
            scanf(" %c", &command);

            if (command == 'r' || command == 'R')
            {
                char confirm;
                printf("[CONFIRM] Eve dönüşü onaylıyor musunuz? (y/n): ");
                scanf(" %c", &confirm);

                if (confirm == 'y' || confirm == 'Y')
                {
                    return_to_earth(&craft);
                }
                else
                {
                    printf("[CONFIRM] Eve dönüş iptal edildi. Mekik yörüngede kalmaya devam ediyor.\n");
                }
            }
            else
            {
                printf("[COMMAND] Geçersiz komut. Mekik yörüngede kalmaya devam ediyor.\n");
            }
        }
    }

    if (craft.state == LANDED)
    {
        printf("[MISSION COMPLETE] Görev başarıyla tamamlandı!\n");
    }
    else
    {
        printf("[MISSION FAILED] Görev tamamlanamadı.\n");
    }

    return 0;
}