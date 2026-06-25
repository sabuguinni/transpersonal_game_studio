#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    NightTime       UMETA(DisplayName = "Night Time")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    float AmbientVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    float MusicVolume = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    float ReverbAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    bool bEnableInsects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    bool bEnableBirds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    bool bEnableWind = true;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bCausesScreenShake = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ScreenShakeRadius = 1500.0f;
};

/**
 * UAudio_ProximityComponent — attaches to dinosaur actors, manages spatial audio
 * and screen shake triggers when player enters proximity radius.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float ProximityRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float ScreenShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    bool bIsLargeCreature = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    FAudio_DinosaurSoundConfig SoundConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    void TriggerFootstep(bool bLeftFoot);

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    bool IsPlayerInProximity() const;

private:
    float TimeSinceLastRoar = 0.0f;
    float RoarCooldown = 8.0f;
    bool bPlayerWasInRange = false;
};

/**
 * AAudio_SystemManager — world actor that manages all adaptive audio,
 * biome transitions, day/night music changes, and danger level music.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === BIOME AUDIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    TArray<FAudio_BiomeAudioConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    EAudio_BiomeZone CurrentBiome = EAudio_BiomeZone::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Biome")
    float BiomeTransitionSpeed = 2.0f;

    // === DANGER AUDIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float DangerMusicIntensity = 0.0f;

    // === DAY/NIGHT AUDIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    bool bIsNightTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float TimeOfDay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DayNightCycleSpeed = 0.01f;

    // === AUDIO COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* MusicAudioComponent;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void SetBiome(EAudio_BiomeZone NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void SetDangerLevel(EAudio_DangerLevel NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void OnDinosaurRoar(FVector RoarLocation, float RoarRadius, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void OnPlayerTakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void OnCraftingAction(FName ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    void OnFireLit(FVector FireLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    float GetCurrentMusicIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    EAudio_BiomeZone GetCurrentBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|System")
    bool GetIsNightTime() const;

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateDangerFade(float DeltaTime);
    void UpdateBiomeTransition(float DeltaTime);

    float TargetDangerIntensity = 0.0f;
    EAudio_BiomeZone TargetBiome = EAudio_BiomeZone::OpenSavanna;
    float BiomeBlendAlpha = 1.0f;
};
