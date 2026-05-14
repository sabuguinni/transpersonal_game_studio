#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Audio_ProximityAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float TriggerDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    TSoftObjectPtr<USoundCue> ProximitySoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    FString TriggerTag;

    FAudio_ProximityTrigger()
    {
        TriggerDistance = 1000.0f;
        VolumeMultiplier = 1.0f;
        bLooping = true;
        TriggerTag = TEXT("Default");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ProximityAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ProximityAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Proximity audio configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TArray<FAudio_ProximityTrigger> ProximityTriggers;

    // Player reference for distance calculations
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    class APawn* PlayerPawn;

    // Audio components for different proximity zones
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* TRexProximityAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* RaptorPackAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientForestAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* FootstepAudio;

    // T-Rex specific proximity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Audio")
    float TRexProximityDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Audio")
    float TRexFootstepInterval = 3.0f;

    // Raptor pack proximity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor Audio")
    float RaptorPackDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor Audio")
    int32 MinRaptorPackSize = 3;

    // Audio state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bTRexNearby = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bRaptorPackNearby = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float LastFootstepTime = 0.0f;

    // Audio management functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateProximityAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void TriggerTRexProximityAudio(bool bEnable, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void TriggerRaptorPackAudio(bool bEnable, int32 PackSize);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayFootstepAudio(FVector Location, bool bIsLarge = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateAmbientAudio(const FString& BiomeType);

    // Dinosaur detection functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Detection")
    TArray<AActor*> GetNearbyDinosaurs(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Detection")
    bool IsTRexNearby(float& OutDistance);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Detection")
    bool IsRaptorPackNearby(int32& OutPackSize, float& OutDistance);

private:
    // Internal audio management
    void InitializeAudioComponents();
    void UpdateTRexAudio();
    void UpdateRaptorAudio();
    void UpdateAmbientAudio();
    
    // Audio fade and transition functions
    void FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime);
    void CrossfadeAudio(UAudioComponent* FromComp, UAudioComponent* ToComp, float FadeTime);

    // Distance calculation helpers
    float GetDistanceToPlayer(const FVector& Location);
    bool IsActorInRange(AActor* Actor, float Range);
};