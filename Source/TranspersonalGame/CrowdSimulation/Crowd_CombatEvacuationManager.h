#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Crowd_CombatEvacuationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_EvacuationState : uint8
{
    Normal UMETA(DisplayName = "Normal"),
    Alert UMETA(DisplayName = "Alert"),
    Panic UMETA(DisplayName = "Panic"),
    Evacuation UMETA(DisplayName = "Evacuation"),
    Scattered UMETA(DisplayName = "Scattered")
};

USTRUCT(BlueprintType)
struct FCrowd_EvacuationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    ECrowd_EvacuationState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evacuation")
    TArray<AActor*> NPCsInZone;

    FCrowd_EvacuationZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        CurrentState = ECrowd_EvacuationState::Normal;
        ThreatLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_NPCEvacuationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    AActor* NPCActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FVector OriginalPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FVector EvacuationTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float PanicLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsEvacuating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float EvacuationSpeed;

    FCrowd_NPCEvacuationData()
    {
        NPCActor = nullptr;
        OriginalPosition = FVector::ZeroVector;
        EvacuationTarget = FVector::ZeroVector;
        PanicLevel = 0.0f;
        bIsEvacuating = false;
        EvacuationSpeed = 600.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_CombatEvacuationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_CombatEvacuationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat evacuation system
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void TriggerEvacuation(FVector CombatLocation, float ThreatRadius, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void UpdateEvacuationZones(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void RegisterNPCForEvacuation(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void UnregisterNPCFromEvacuation(AActor* NPCActor);

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void CreateEvacuationZone(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void RemoveEvacuationZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    FCrowd_EvacuationZone GetEvacuationZone(int32 ZoneIndex);

    // NPC behavior control
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void SetNPCPanicLevel(AActor* NPCActor, float PanicLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    FVector CalculateEvacuationTarget(AActor* NPCActor, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void ExecuteEvacuationMovement(AActor* NPCActor, float DeltaTime);

    // Combat integration
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void OnCombatStarted(FVector CombatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void OnCombatEnded(FVector CombatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void OnDinosaurSpotted(AActor* DinosaurActor, FVector Location);

    // Crowd density management
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    float GetCrowdDensityAtLocation(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void AdjustCrowdDensityForCombat(FVector CombatLocation, float Radius);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    bool IsLocationSafe(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    TArray<AActor*> GetNPCsInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Evacuation")
    void ResetAllEvacuations();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Evacuation")
    TArray<FCrowd_EvacuationZone> EvacuationZones;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Evacuation")
    TArray<FCrowd_NPCEvacuationData> RegisteredNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxEvacuationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PanicDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EvacuationSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CrowdDensityThreshold;

private:
    void UpdateNPCEvacuation(FCrowd_NPCEvacuationData& NPCData, float DeltaTime);
    void CalculateZoneThreatLevel(FCrowd_EvacuationZone& Zone);
    FVector FindSafeEvacuationPoint(FVector StartLocation, FVector ThreatLocation);
    bool IsNPCInEvacuationZone(AActor* NPCActor, const FCrowd_EvacuationZone& Zone);
};