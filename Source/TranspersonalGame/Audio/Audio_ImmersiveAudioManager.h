#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/AmbientSound.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Character.h"
#include "Audio_ImmersiveAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Exploration     UMETA(DisplayName = "Exploration"),
    Danger          UMETA(DisplayName = "Danger"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Night           UMETA(DisplayName = "Night"),
    Day             UMETA(DisplayName = "Day"),
    Combat          UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 2.0f;

    FAudio_BiomeAudioConfig()
    {
        BiomeType = EAudio_BiomeType::Savanna;
        VolumeMultiplier = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_ProximityAlert
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    FString DinosaurName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float AlertCooldown = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    TSoftObjectPtr<USoundCue> AlertSound;

    UPROPERTY(BlueprintReadOnly, Category = "Proximity")
    float LastAlertTime = 0.0f;

    FAudio_ProximityAlert()
    {
        DinosaurName = TEXT("Unknown");
        DetectionRadius = 1500.0f;
        AlertCooldown = 10.0f;
        LastAlertTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ImmersiveAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ImmersiveAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Audio Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_BiomeAudioConfig> BiomeConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Audio")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Savanna;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Audio")
    UAudioComponent* CurrentAmbientAudio;

    // Proximity Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Detection")
    TArray<FAudio_ProximityAlert> ProximityAlerts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Detection")
    float ProximityCheckInterval = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Proximity Detection")
    float LastProximityCheck = 0.0f;

    // Music State Management
    UPROPERTY(BlueprintReadOnly, Category = "Music")
    EAudio_MusicState CurrentMusicState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TMap<EAudio_MusicState, TSoftObjectPtr<USoundCue>> MusicStateCues;

    UPROPERTY(BlueprintReadOnly, Category = "Music")
    UAudioComponent* MusicAudioComponent;

    // Resource Discovery Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Audio")
    TSoftObjectPtr<USoundCue> ResourceDiscoverySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Audio")
    float ResourceDiscoveryVolume = 0.8f;

public:
    // Biome Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateBiomeAudio(const FVector& PlayerLocation);

    // Proximity Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Proximity")
    void CheckProximityAlerts(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Proximity")
    void TriggerProximityAlert(const FString& DinosaurName, float Distance);

    // Music State Functions
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void TransitionToMusicState(EAudio_MusicState NewState, float FadeTime = 2.0f);

    // Resource Discovery
    UFUNCTION(BlueprintCallable, Category = "Resource Audio")
    void PlayResourceDiscoveryAudio(const FVector& Location, const FString& ResourceType);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    EAudio_BiomeType DetermineBiomeFromLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    void InitializeAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Audio Utility")
    void CleanupAudioSystems();

private:
    // Internal helper functions
    void InitializeBiomeConfigs();
    void InitializeProximityAlerts();
    void InitializeMusicStates();
    
    void FadeOutCurrentAmbient();
    void FadeInNewAmbient(const FAudio_BiomeAudioConfig& Config);
    
    bool IsValidAudioComponent(UAudioComponent* AudioComp) const;
    UAudioComponent* CreateAudioComponent(USoundCue* SoundCue);
    
    // Cached references
    UPROPERTY()
    ACharacter* PlayerCharacter;
    
    UPROPERTY()
    UWorld* CachedWorld;
};