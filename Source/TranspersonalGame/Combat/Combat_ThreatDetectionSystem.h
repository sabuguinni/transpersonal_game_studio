#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Combat_ThreatDetectionSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_DetectionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Searching   UMETA(DisplayName = "Searching"),
    Detected    UMETA(DisplayName = "Target Detected"),
    Tracking    UMETA(DisplayName = "Tracking"),
    Engaging    UMETA(DisplayName = "Engaging")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    FCombat_ThreatInfo()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_ThreatDetectionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_ThreatDetectionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Detection Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float AttackRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float FieldOfView = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float DetectionUpdateRate = 0.2f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Threat Detection")
    ECombat_DetectionState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Detection")
    TArray<FCombat_ThreatInfo> DetectedThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Detection")
    AActor* PrimaryThreat;

    // Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void UpdateThreatDetection();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* ThreatActor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    bool IsActorInFieldOfView(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void AddThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    FCombat_ThreatInfo GetHighestThreat();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatDetected, AActor*, ThreatActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLost, AActor*, ThreatActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLevelChanged, ECombat_ThreatLevel, NewThreatLevel);

    UPROPERTY(BlueprintAssignable)
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable)
    FOnThreatLost OnThreatLost;

    UPROPERTY(BlueprintAssignable)
    FOnThreatLevelChanged OnThreatLevelChanged;

private:
    float LastDetectionUpdate;
    ECombat_ThreatLevel LastThreatLevel;

    void CleanupOldThreats();
    bool CanSeeActor(AActor* TargetActor);
};