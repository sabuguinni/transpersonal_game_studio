#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "RaptorPackManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_RaptorPackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating")
};

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha   UMETA(DisplayName = "Alpha"),
    Flanker UMETA(DisplayName = "Flanker"),
    Ambush  UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_RaptorPackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    bool bPlayerLocationKnown = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    float PackAlertLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    ECombat_RaptorPackState PackState = ECombat_RaptorPackState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    float LastHuntTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FCombat_FlankPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Flanking")
    FVector WorldPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Flanking")
    ECombat_RaptorRole AssignedRole = ECombat_RaptorRole::Flanker;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Flanking")
    bool bOccupied = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Flanking")
    AActor* AssignedRaptor = nullptr;
};

/**
 * URaptorPackManager — shared pack coordination component.
 * Attached to the Alpha raptor. All pack members query this for:
 *   - Last known player position
 *   - Assigned flank positions
 *   - Pack state transitions
 */
UCLASS(ClassGroup = "Combat", meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Pack Manager")
class TRANSPERSONALGAME_API URaptorPackManager : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Pack State ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Combat|RaptorPack",
        meta = (AllowPrivateAccess = "true"))
    FCombat_RaptorPackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    float FlankRadius = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    float AlertDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    float CoordinationInterval = 1.5f;

    // ── Pack Registration ────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void RegisterPackMember(AActor* Raptor, ECombat_RaptorRole Role);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void UnregisterPackMember(AActor* Raptor);

    // ── Player Detection Sharing ─────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ReportPlayerSighting(FVector PlayerLocation, AActor* ReportingRaptor);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ReportPlayerLost();

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    bool GetLastKnownPlayerLocation(FVector& OutLocation) const;

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    ECombat_RaptorPackState GetPackState() const;

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    float GetPackAlertLevel() const;

    // ── Flanking Coordination ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Combat|Flanking")
    TArray<FCombat_FlankPosition> CalculateFlankPositions(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Flanking")
    bool AssignFlankPosition(AActor* Raptor, FVector& OutAssignedPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat|Flanking")
    void ReleaseFlankPosition(AActor* Raptor);

    // ── Pack Commands ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void IssueHuntCommand(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void IssueRetreatCommand();

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void IssueAttackCommand(AActor* Target);

private:
    UPROPERTY()
    TArray<FCombat_FlankPosition> ActiveFlankPositions;

    UPROPERTY()
    TMap<AActor*, ECombat_RaptorRole> MemberRoles;

    float TimeSinceLastCoordination = 0.0f;

    void CoordinatePackBehavior(float DeltaTime);
    void UpdatePackState();
    void BroadcastPackCommand(ECombat_RaptorPackState NewState);
    FVector CalculateFlankOffset(int32 FlankIndex, FVector PlayerLocation, int32 TotalFlankers);
};
