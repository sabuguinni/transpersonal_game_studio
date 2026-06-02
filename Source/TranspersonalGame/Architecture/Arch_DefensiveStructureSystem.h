#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Arch_DefensiveStructureSystem.generated.h"

/**
 * Defensive Structure Types for Cretaceous Period
 * Primitive fortifications and defensive positions
 */
UENUM(BlueprintType)
enum class EArch_DefensiveStructureType : uint8
{
    Watchtower          UMETA(DisplayName = "Stone Watchtower"),
    Barricade          UMETA(DisplayName = "Wooden Barricade"),
    PitTrap            UMETA(DisplayName = "Concealed Pit Trap"),
    RockWall           UMETA(DisplayName = "Defensive Rock Wall"),
    SpikePalisade      UMETA(DisplayName = "Sharpened Spike Palisade"),
    HidingSpot         UMETA(DisplayName = "Natural Hiding Spot"),
    EscapeRoute        UMETA(DisplayName = "Emergency Escape Route"),
    LookoutPost        UMETA(DisplayName = "Elevated Lookout Post")
};

/**
 * Defensive Structure Configuration
 * Settings for each defensive structure type
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DefensiveStructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_DefensiveStructureType StructureType = EArch_DefensiveStructureType::Watchtower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(300.0f, 300.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DefensiveRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float VisibilityRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesConcealment = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesElevation = false;

    FArch_DefensiveStructureConfig()
    {
        StructureType = EArch_DefensiveStructureType::Watchtower;
        Dimensions = FVector(300.0f, 300.0f, 500.0f);
        DefensiveRadius = 1000.0f;
        VisibilityRange = 2000.0f;
        MaxOccupants = 2;
        StructuralIntegrity = 100.0f;
        bCanBeDestroyed = true;
        bProvidesConcealment = false;
        bProvidesElevation = false;
    }
};

/**
 * Threat Detection Data
 * Information about detected threats from defensive positions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ThreatDetectionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float DetectionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bIsHostile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bRequiresAlert = false;

    FArch_ThreatDetectionData()
    {
        ThreatActor = nullptr;
        ThreatLocation = FVector::ZeroVector;
        ThreatDistance = 0.0f;
        ThreatLevel = 0.0f;
        DetectionTime = 0.0f;
        bIsHostile = false;
        bRequiresAlert = false;
    }
};

/**
 * Defensive Position Status
 * Current status and occupancy of defensive structures
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DefensivePositionStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    int32 CurrentOccupants = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float CurrentIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    bool bIsUnderAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    bool bNeedsRepair = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float LastMaintenanceTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    TArray<FArch_ThreatDetectionData> DetectedThreats;

    FArch_DefensivePositionStatus()
    {
        bIsOccupied = false;
        CurrentOccupants = 0;
        CurrentIntegrity = 100.0f;
        bIsUnderAttack = false;
        bNeedsRepair = false;
        LastMaintenanceTime = 0.0f;
        DetectedThreats.Empty();
    }
};

/**
 * Defensive Structure System Component
 * Manages defensive structures and threat detection in Cretaceous environments
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_DefensiveStructureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_DefensiveStructureSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core defensive structure management
    UFUNCTION(BlueprintCallable, Category = "Defensive Structures")
    void InitializeDefensiveSystem();

    UFUNCTION(BlueprintCallable, Category = "Defensive Structures")
    bool CreateDefensiveStructure(EArch_DefensiveStructureType StructureType, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Defensive Structures")
    void RemoveDefensiveStructure(int32 StructureIndex);

    UFUNCTION(BlueprintCallable, Category = "Defensive Structures")
    void UpdateStructureIntegrity(int32 StructureIndex, float IntegrityChange);

    // Threat detection and monitoring
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    TArray<FArch_ThreatDetectionData> GetDetectedThreats() const;

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    bool IsLocationDefended(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    AActor* GetNearestDefensiveStructure(const FVector& Location) const;

    // Occupancy and positioning
    UFUNCTION(BlueprintCallable, Category = "Positioning")
    bool OccupyDefensivePosition(int32 StructureIndex, AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Positioning")
    void VacateDefensivePosition(int32 StructureIndex, AActor* Occupant);

    UFUNCTION(BlueprintCallable, Category = "Positioning")
    FVector GetOptimalDefensivePosition(const FVector& ThreatDirection) const;

    UFUNCTION(BlueprintCallable, Category = "Positioning")
    TArray<FVector> GetAvailableDefensivePositions() const;

    // Maintenance and repair
    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    void RepairStructure(int32 StructureIndex, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    void PerformMaintenanceCheck();

    UFUNCTION(BlueprintCallable, Category = "Maintenance")
    TArray<int32> GetStructuresNeedingRepair() const;

    // Configuration and status
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetDefensiveConfiguration(const FArch_DefensiveStructureConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FArch_DefensiveStructureConfig GetDefensiveConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "Status")
    FArch_DefensivePositionStatus GetStructureStatus(int32 StructureIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Status")
    int32 GetTotalDefensiveStructures() const;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    FArch_DefensiveStructureConfig DefensiveConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float ThreatScanInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float MaintenanceCheckInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bAutoRepairEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float AutoRepairRate = 1.0f;

    // Runtime data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> DefensiveStructures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_DefensivePositionStatus> StructureStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    TArray<FArch_ThreatDetectionData> CurrentThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    float LastThreatScan = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    float LastMaintenanceCheck = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    bool bSystemInitialized = false;

private:
    // Internal helper methods
    void CreateWatchtower(const FVector& Location, const FRotator& Rotation);
    void CreateBarricade(const FVector& Location, const FRotator& Rotation);
    void CreatePitTrap(const FVector& Location, const FRotator& Rotation);
    void CreateRockWall(const FVector& Location, const FRotator& Rotation);
    void CreateSpikePalisade(const FVector& Location, const FRotator& Rotation);
    void CreateHidingSpot(const FVector& Location, const FRotator& Rotation);
    void CreateEscapeRoute(const FVector& Location, const FRotator& Rotation);
    void CreateLookoutPost(const FVector& Location, const FRotator& Rotation);

    void DetectThreatsInRange(AActor* DefensiveStructure, float Range);
    void UpdateThreatLevels();
    void ProcessThreatAlerts();

    bool IsValidDefensiveLocation(const FVector& Location) const;
    float CalculateDefensiveValue(const FVector& Location) const;
    void OptimizeDefensiveLayout();
};