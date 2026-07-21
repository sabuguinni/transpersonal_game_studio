#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinosaurHerdBehavior.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Resting     UMETA(DisplayName = "Resting"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersing  UMETA(DisplayName = "Dispersing")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float GrazingDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float WaypointRadius;

    FCrowd_HerdWaypoint()
        : Location(FVector::ZeroVector)
        , GrazingDuration(30.f)
        , WaypointRadius(500.f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMemberData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector OffsetFromCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeSpeed;

    FCrowd_HerdMemberData()
        : OffsetFromCenter(FVector::ZeroVector)
        , StampedeSpeed(1200.f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurHerdBehavior : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurHerdBehavior();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Herd Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FCrowd_HerdWaypoint> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeTriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float HerdSpreadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatDetectionRange;

    // --- Herd State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Herd")
    ECrowd_HerdState HerdState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMemberData> HerdMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Herd")
    int32 CurrentWaypointIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Herd")
    FVector StampedeDirection;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void AddHerdMember(APawn* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RemoveHerdMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerStampede(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void CalmHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void AdvanceToNextWaypoint();

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    bool IsStampeding() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    FVector GetHerdCenter() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    int32 GetHerdSize() const;

private:
    void UpdateGrazing(float DeltaTime);
    void UpdateMigration(float DeltaTime);
    void UpdateStampede(float DeltaTime);
    void CheckForThreats();
    bool HasReachedWaypoint() const;
    FVector ComputeStampedeDirection(FVector ThreatLocation) const;

    float TimeAtCurrentWaypoint;
    float StampedeTimeRemaining;
};
