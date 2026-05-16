#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "GameFramework/Pawn.h"
#include "Combat_ThreatDetection.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector ThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    APawn* ThreatSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatIntensity;

    FCombat_ThreatData()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        ThreatRadius = 1000.0f;
        ThreatLocation = FVector::ZeroVector;
        ThreatSource = nullptr;
        ThreatIntensity = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnThreatDetected, FCombat_ThreatData, ThreatData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnThreatLevelChanged, ECombat_ThreatLevel, NewThreatLevel);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_ThreatDetection : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_ThreatDetection();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Threat Detection Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    TArray<TSubclassOf<APawn>> ThreatActorClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Detection")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Threat Detection")
    TArray<FCombat_ThreatData> ActiveThreats;

    // Combat Zone Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zones")
    TArray<ATriggerVolume*> CombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Zones")
    TArray<ATriggerVolume*> SafeZones;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Zones")
    bool bIsInCombatZone;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Zones")
    bool bIsInSafeZone;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Threat Events")
    FCombat_OnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Threat Events")
    FCombat_OnThreatLevelChanged OnThreatLevelChanged;

    // Threat Detection Methods
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void UpdateThreatDetection();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    ECombat_ThreatLevel CalculateThreatLevel(const TArray<FCombat_ThreatData>& Threats);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void AddThreat(APawn* ThreatActor, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void RemoveThreat(APawn* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    bool IsActorThreatening(APawn* Actor);

    // Combat Zone Methods
    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void RegisterCombatZone(ATriggerVolume* Zone);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void RegisterSafeZone(ATriggerVolume* Zone);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void UpdateZoneStatus();

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    bool IsLocationInCombatZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    bool IsLocationInSafeZone(const FVector& Location);

    // Utility Methods
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    FVector GetNearestSafeLocation();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    APawn* GetHighestThreatActor();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    float GetTotalThreatIntensity();

private:
    float LastThreatUpdate;
    ECombat_ThreatLevel PreviousThreatLevel;

    void CleanupInvalidThreats();
    void BroadcastThreatEvents();
};

#include "Combat_ThreatDetection.generated.h"