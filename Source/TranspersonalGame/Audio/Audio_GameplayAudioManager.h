#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_GameplayAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_GameplayAudioType : uint8
{
    PlayerFootsteps     UMETA(DisplayName = "Player Footsteps"),
    CraftingSuccess     UMETA(DisplayName = "Crafting Success"),
    CraftingFail        UMETA(DisplayName = "Crafting Fail"),
    ItemPickup          UMETA(DisplayName = "Item Pickup"),
    InventoryOpen       UMETA(DisplayName = "Inventory Open"),
    InventoryClose      UMETA(DisplayName = "Inventory Close"),
    HealthLow           UMETA(DisplayName = "Health Low"),
    StaminaEmpty        UMETA(DisplayName = "Stamina Empty"),
    HungerAlert         UMETA(DisplayName = "Hunger Alert"),
    ThirstAlert         UMETA(DisplayName = "Thirst Alert"),
    DangerDetected      UMETA(DisplayName = "Danger Detected"),
    SafetyReached       UMETA(DisplayName = "Safety Reached")
};

USTRUCT(BlueprintType)
struct FAudio_GameplayAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance = 1000.0f;

    FAudio_GameplayAudioConfig()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = false;
        MaxDistance = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_GameplayAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_GameplayAudioManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Gameplay audio playback
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayGameplayAudio(EAudio_GameplayAudioType AudioType, FVector Location = FVector::ZeroVector, AActor* SourceActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopGameplayAudio(EAudio_GameplayAudioType AudioType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllGameplayAudio();

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetGameplayAudioVolume(EAudio_GameplayAudioType AudioType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterGameplayVolume(float Volume);

    // Audio configuration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetGameplayAudioConfig(EAudio_GameplayAudioType AudioType, const FAudio_GameplayAudioConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FAudio_GameplayAudioConfig GetGameplayAudioConfig(EAudio_GameplayAudioType AudioType) const;

    // Player state audio triggers
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerPlayerFootstep(FVector Location, bool bIsRunning = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerCraftingResult(bool bSuccess, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerSurvivalAlert(EAudio_GameplayAudioType AlertType, float Severity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDangerState(bool bInDanger, FVector ThreatLocation = FVector::ZeroVector);

protected:
    // Audio configuration map
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudio_GameplayAudioType, FAudio_GameplayAudioConfig> AudioConfigs;

    // Active audio components
    UPROPERTY()
    TMap<EAudio_GameplayAudioType, UAudioComponent*> ActiveAudioComponents;

    // Master volume
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterGameplayVolume = 1.0f;

    // Footstep timing
    UPROPERTY()
    float LastFootstepTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RunningFootstepInterval = 0.3f;

private:
    void InitializeAudioConfigs();
    UAudioComponent* CreateAudioComponent(const FAudio_GameplayAudioConfig& Config, FVector Location, AActor* SourceActor);
    void CleanupFinishedAudioComponents();
};