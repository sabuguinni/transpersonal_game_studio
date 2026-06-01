#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Audio_ProximityDetector.generated.h"

UENUM(BlueprintType)
enum class EAudio_ProximityType : uint8
{
    Player      UMETA(DisplayName = "Player"),
    Dinosaur    UMETA(DisplayName = "Dinosaur"),
    Environment UMETA(DisplayName = "Environment"),
    Danger      UMETA(DisplayName = "Danger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    bool bUseDistanceAttenuation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float MinVolume = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float MaxVolume = 1.0f;

    FAudio_ProximitySettings()
    {
        DetectionRadius = 1000.0f;
        VolumeMultiplier = 1.0f;
        bUseDistanceAttenuation = true;
        MinVolume = 0.1f;
        MaxVolume = 1.0f;
    }
};

UCLASS(BlueprintType, ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityDetector : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityDetector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Proximity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Settings")
    EAudio_ProximityType ProximityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Settings")
    FAudio_ProximitySettings ProximitySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Settings")
    TArray<FString> TargetActorTags;

    // Detection sphere
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    // Audio management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ProximitySound;

    // Detection functions
    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    void UpdateProximityDetection();

    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    float CalculateVolumeByDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    TArray<AActor*> GetNearbyActors();

    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    void SetDetectionRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    void EnableProximityDetection(bool bEnable);

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Proximity Events")
    void OnActorEnterProximity(AActor* Actor, float Distance);

    UFUNCTION(BlueprintImplementableEvent, Category = "Proximity Events")
    void OnActorExitProximity(AActor* Actor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Proximity Events")
    void OnProximityVolumeChanged(float NewVolume);

private:
    // Internal tracking
    TArray<AActor*> CurrentNearbyActors;
    TArray<AActor*> PreviousNearbyActors;
    
    float LastDetectionUpdate;
    float DetectionUpdateInterval;
    
    bool bIsDetectionEnabled;
    
    // Helper functions
    void CheckForNewActors();
    void CheckForRemovedActors();
    void UpdateAudioVolume();
    bool IsValidTargetActor(AActor* Actor);
};