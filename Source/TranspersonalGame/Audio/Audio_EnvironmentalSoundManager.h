#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Subsystems/WorldSubsystem.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EnvironmentalZone : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    River           UMETA(DisplayName = "River"),
    Plains          UMETA(DisplayName = "Plains"),
    Cave            UMETA(DisplayName = "Cave"),
    Swamp           UMETA(DisplayName = "Swamp")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentalZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    EAudio_EnvironmentalZone ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float FadeOutTime = 2.0f;

    FAudio_EnvironmentalZoneData()
    {
        ZoneType = EAudio_EnvironmentalZone::Forest;
        Volume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

/**
 * Manages environmental audio zones and ambient soundscapes
 * Handles smooth transitions between different biome audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_EnvironmentalSoundManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudio_EnvironmentalSoundManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetEnvironmentalZone(EAudio_EnvironmentalZone NewZone, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdatePlayerLocation(FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterEnvironmentalVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    EAudio_EnvironmentalZone GetCurrentZone() const { return CurrentZone; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<FAudio_EnvironmentalZoneData> ZoneConfigurations;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental Audio")
    EAudio_EnvironmentalZone CurrentZone;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental Audio")
    UAudioComponent* CurrentAmbientComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float MasterEnvironmentalVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float ZoneTransitionRadius = 1000.0f;

private:
    void InitializeZoneConfigurations();
    void TransitionToZone(const FAudio_EnvironmentalZoneData& ZoneData, FVector Location);
    FAudio_EnvironmentalZoneData* GetZoneConfiguration(EAudio_EnvironmentalZone Zone);
};