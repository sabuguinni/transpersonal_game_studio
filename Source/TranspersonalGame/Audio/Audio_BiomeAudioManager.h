#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerBox.h"
#include "Audio_BiomeAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Pantano     UMETA(DisplayName = "Swamp"),
    Floresta    UMETA(DisplayName = "Forest"), 
    Savana      UMETA(DisplayName = "Savanna"),
    Deserto     UMETA(DisplayName = "Desert"),
    Montanha    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> WildlifeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 1.5f;

    FAudio_BiomeAudioData()
    {
        BiomeType = EAudio_BiomeType::Savana;
        BaseVolume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_BiomeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_BiomeAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") 
    class UAudioComponent* WindAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WildlifeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_BiomeAudioData> BiomeAudioDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeType TargetBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransitionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BiomeDetectionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 0.5f;

private:
    float LastUpdateTime;
    class APawn* PlayerPawn;

    FAudio_BiomeAudioData* GetBiomeAudioData(EAudio_BiomeType BiomeType);
    EAudio_BiomeType DetectCurrentBiome(const FVector& PlayerLocation);
    void StartBiomeTransition(EAudio_BiomeType NewBiome);
    void UpdateAudioTransition(float DeltaTime);
    void SetBiomeAudio(EAudio_BiomeType BiomeType, float VolumeMultiplier = 1.0f);

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintImplementableEvent, Category = "Biome Audio")
    void OnBiomeChanged(EAudio_BiomeType OldBiome, EAudio_BiomeType NewBiome);
};