#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "CrowdHerdBehavior.generated.h"

// ============================================================
// Enums — must be at global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Leader      UMETA(DisplayName = "Leader"),
    Scout       UMETA(DisplayName = "Scout"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Core        UMETA(DisplayName = "Core Member"),
    Straggler   UMETA(DisplayName = "Straggler"),
    Juvenile    UMETA(DisplayName = "Juvenile")
};

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Resting     UMETA(DisplayName = "Resting"),
    Alert       UMETA(DisplayName = "Alert"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Defending   UMETA(DisplayName = "Defending")
};

// ============================================================
// Structs — must be at global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdRole Role = ECrowd_HerdRole::Core;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float DistanceToLeader = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float AlertLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsVisible = true;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MinHerdSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MaxHerdSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlignmentWeight = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionWeight = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationWeight = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlertPropagationRadius = 800.0f;
};

// ============================================================
// UCrowdHerdBehavior — Boids-based herd simulation component
// ============================================================

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent), DisplayName = "Crowd Herd Behavior")
class TRANSPERSONALGAME_API UCrowdHerdBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdBehavior();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_HerdConfig HerdConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentHerdState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    AActor* HerdLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationTarget;

    // Public API
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerdMember(AActor* Agent, ECrowd_HerdRole Role);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void UnregisterHerdMember(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetMigrationTarget(FVector Target);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector ComputeBoidsVelocity(AActor* Agent) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void PropagateAlert(FVector ThreatLocation, float AlertStrength);

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    int32 GetHerdSize() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    ECrowd_HerdState GetHerdState() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    AActor* GetHerdLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void ElectNewLeader();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FVector ComputeSeparation(AActor* Agent) const;
    FVector ComputeAlignment(AActor* Agent) const;
    FVector ComputeCohesion(AActor* Agent) const;
    FVector ComputeLeaderFollow(AActor* Agent) const;
    void UpdateHerdMemberRoles();
    void TickGrazingBehavior(float DeltaTime);
    void TickMigrationBehavior(float DeltaTime);
    void TickAlertBehavior(float DeltaTime);

    float StateTransitionTimer;
    float GrazingDuration;
};
