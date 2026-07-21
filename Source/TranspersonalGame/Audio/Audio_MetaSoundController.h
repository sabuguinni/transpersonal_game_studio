#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundController.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"), 
    River       UMETA(DisplayName = "River"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Cave        UMETA(DisplayName = "Cave"),
    Swamp       UMETA(DisplayName = "Swamp")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbienceZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_AmbienceType AmbienceType = EAudio_AmbienceType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = true;

    FAudio_AmbienceZone()
    {
        AmbienceType = EAudio_AmbienceType::Forest;
        VolumeMultiplier = 1.0f;
        FadeDistance = 2000.0f;
        bLooping = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_MetaSoundController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Ambience zone management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambience")
    TArray<FAudio_AmbienceZone> AmbienceZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambience")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambience")
    float AmbienceFadeSpeed = 2.0f;

    // Footstep audio system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    TSoftObjectPtr<UMetaSoundSource> PlayerFootstepMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    TSoftObjectPtr<UMetaSoundSource> DinosaurFootstepMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    float FootstepVolumeRange = 1500.0f;

    // Dynamic audio mixing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Mixing")
    float DangerProximityThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Mixing")
    float TensionVolumeMultiplier = 1.5f;

    // Audio functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbienceForZone(EAudio_AmbienceType ZoneType, FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerFootstepAudio(FVector FootstepLocation, bool bIsPlayerFootstep = true);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateDynamicMixing(float DangerLevel, float StaminaLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetGlobalAudioVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_AmbienceType GetCurrentAmbienceType() const { return CurrentAmbienceType; }

private:
    // Internal state
    UPROPERTY()
    EAudio_AmbienceType CurrentAmbienceType = EAudio_AmbienceType::Forest;

    UPROPERTY()
    float CurrentAmbienceVolume = 1.0f;

    UPROPERTY()
    float TargetAmbienceVolume = 1.0f;

    // Audio component references
    UPROPERTY()
    class UAudioComponent* CurrentAmbienceComponent;

    UPROPERTY()
    TArray<class UAudioComponent*> FootstepComponents;

    // Helper functions
    void UpdateAmbienceVolume(float DeltaTime);
    void CleanupFinishedAudioComponents();
    FAudio_AmbienceZone* FindAmbienceZoneByType(EAudio_AmbienceType ZoneType);
};