#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Audio_ProximityAlertSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_AlertType : uint8
{
    TRexProximity       UMETA(DisplayName = "T-Rex Proximity"),
    VolcanicHazard      UMETA(DisplayName = "Volcanic Hazard"),
    StormWarning        UMETA(DisplayName = "Storm Warning"),
    PredatorPack        UMETA(DisplayName = "Predator Pack"),
    EnvironmentalDanger UMETA(DisplayName = "Environmental Danger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AlertConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
    EAudio_AlertType AlertType = EAudio_AlertType::TRexProximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
    float AlertCooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
    bool bRequiresLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert Config")
    USoundBase* AlertSound = nullptr;

    FAudio_AlertConfiguration()
    {
        AlertType = EAudio_AlertType::TRexProximity;
        DetectionRadius = 1000.0f;
        AlertCooldown = 30.0f;
        bRequiresLineOfSight = false;
        VolumeMultiplier = 1.0f;
        AlertSound = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ProximityAlertSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ProximityAlertSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AlertAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Alert Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    FAudio_AlertConfiguration AlertConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    TArray<TSubclassOf<APawn>> TargetClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alert System")
    float MinimumThreatDistance = 500.0f;

    // Alert State
    UPROPERTY(BlueprintReadOnly, Category = "Alert State")
    bool bAlertActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Alert State")
    float LastAlertTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Alert State")
    TArray<AActor*> DetectedThreats;

    // Alert Functions
    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void TriggerAlert(AActor* ThreatActor, EAudio_AlertType AlertType);

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void DeactivateAlert();

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    bool CanTriggerAlert() const;

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    void SetAlertConfiguration(const FAudio_AlertConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Alert System")
    float GetDistanceToNearestThreat() const;

    // Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool IsValidThreat(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool HasLineOfSightToThreat(AActor* ThreatActor) const;

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAlertTriggered, EAudio_AlertType, AlertType, AActor*, ThreatActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAlertDeactivated);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAlertTriggered OnAlertTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAlertDeactivated OnAlertDeactivated;

protected:
    // Internal Functions
    void UpdateDetection();
    void ProcessThreatDistance(AActor* ThreatActor);
    void PlayAlertSound(EAudio_AlertType AlertType);
    USoundBase* GetAlertSoundForType(EAudio_AlertType AlertType) const;
    
    // Overlap Events
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // Internal State
    float ScanTimer = 0.0f;
    float ScanInterval = 1.0f;
    TMap<EAudio_AlertType, USoundBase*> AlertSoundMap;
    
    void InitializeAlertSounds();
    void ValidateConfiguration();
};