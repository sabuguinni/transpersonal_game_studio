#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Zone System — Agent #16 Audio Agent
// Spatial audio management for MinPlayableMap zones
// Integrates with DialogueSystem (Agent #15) tone enum
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Safe        UMETA(DisplayName = "Safe Zone"),       // Camp, shelter — calm ambience
    Caution     UMETA(DisplayName = "Caution Zone"),    // Canyon, ridge — tense, wind
    Danger      UMETA(DisplayName = "Danger Zone"),     // Dino nest — rumble, heavy breathing
    Water       UMETA(DisplayName = "Water Zone"),      // Riverbed — flowing water, insects
    Open        UMETA(DisplayName = "Open Plains"),     // Grassland — wind, distant calls
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FName ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Open;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicIntensity = 0.0f;  // 0=silent, 1=full danger music

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLoopAmbient = true;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Falloff = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 800.0f;  // Distance at which shake triggers
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UAudioZoneSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioZoneSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void RegisterZone(const FAudio_ZoneConfig& ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void EnterZone(FName ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ExitZone(FName ZoneID);

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_ZoneType GetCurrentZoneType() const { return CurrentZoneType; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    float GetCurrentMusicIntensity() const { return CurrentMusicIntensity; }

    // Screen shake (T-Rex proximity, damage)
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerScreenShake(const FAudio_ScreenShakeConfig& ShakeConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerDamageFlash(float Intensity = 1.0f);

    // Footstep system
    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void PlayFootstep(bool bIsRunning, bool bIsHeavyCreature);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void SetSurfaceType(FName SurfaceType);

    // Day/night audio adaptation
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void UpdateTimeOfDay(float NormalizedTimeOfDay);  // 0=midnight, 0.5=noon, 1=midnight

    UFUNCTION(BlueprintPure, Category = "Audio|DayNight")
    bool IsNighttime() const { return bIsNighttime; }

    // Freesound asset references (populated from search results)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString CampfireAmbientURL = "https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString CampfireNightURL = "https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    int32 CampfireSoundID_1 = 681366;  // Freesound ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    int32 CampfireSoundID_2 = 688994;  // Freesound ID with crickets

private:
    UPROPERTY()
    TArray<FAudio_ZoneConfig> RegisteredZones;

    UPROPERTY()
    TArray<FName> ActiveZoneIDs;

    UPROPERTY()
    EAudio_ZoneType CurrentZoneType = EAudio_ZoneType::Open;

    UPROPERTY()
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::None;

    UPROPERTY()
    float CurrentMusicIntensity = 0.0f;

    UPROPERTY()
    float TargetMusicIntensity = 0.0f;

    UPROPERTY()
    float MusicBlendSpeed = 2.0f;

    UPROPERTY()
    FName CurrentSurfaceType = FName("Dirt");

    UPROPERTY()
    bool bIsNighttime = false;

    UPROPERTY()
    float TimeOfDay = 0.5f;  // 0.5 = noon

    void RecalculateZoneState();
    void BlendMusicIntensity(float DeltaTime);
    void InitializeDefaultZones();
};
