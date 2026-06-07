#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TriggerVolume.h"
#include "Audio_PrehistoricAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Plains      UMETA(DisplayName = "Plains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Mountains   UMETA(DisplayName = "Mountains")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeDistance;

    FAudio_AmbientZone()
    {
        BiomeType = EAudio_BiomeType::Forest;
        AmbientSound = nullptr;
        Volume = 1.0f;
        FadeDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_PrehistoricAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_PrehistoricAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FAudio_AmbientZone> AmbientZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    class USoundCue* TRexRoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    class USoundCue* RaptorCallSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    class USoundCue* HeavyFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Audio")
    class USoundCue* PlayerFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Audio")
    class USoundCue* HeartbeatSound;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurRoar(FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(FVector Location, bool bIsHeavy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientZone(EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayThreatMusic(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopThreatMusic();

private:
    void UpdateSpatialAudio();
    void ProcessAmbientZones();

    UPROPERTY()
    class APawn* PlayerPawn;

    float CurrentThreatLevel;
    bool bThreatMusicPlaying;
};

#include "Audio_PrehistoricAudioManager.generated.h"