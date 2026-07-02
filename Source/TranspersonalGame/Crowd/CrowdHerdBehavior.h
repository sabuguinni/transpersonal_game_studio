// CrowdHerdBehavior.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric herd behavior system for herbivore dinosaur groups
// Uses Mass AI concepts adapted for UE5 actor-based simulation

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "CrowdHerdBehavior.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing         UMETA(DisplayName = "Grazing"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Alert           UMETA(DisplayName = "Alert"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Regrouping      UMETA(DisplayName = "Regrouping")
};

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Leader          UMETA(DisplayName = "Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Follower        UMETA(DisplayName = "Follower"),
    Juvenile        UMETA(DisplayName = "Juvenile"),
    Elder           UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Iguanodon       UMETA(DisplayName = "Iguanodon"),
    Gallimimus      UMETA(DisplayName = "Gallimimus")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    AActor* MemberActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdRole Role = ECrowd_HerdRole::Follower;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float HungerLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float ThirstLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float DistanceToLeader = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector TargetLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdSpecies Species = ECrowd_HerdSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MinHerdSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MaxHerdSize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlignmentWeight = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float RunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatDetectionRadius = 1500.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_FlockingForces
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    FVector Alignment = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    FVector Cohesion = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    FVector Separation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    FVector ThreatAvoidance = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    FVector ResultantForce = FVector::ZeroVector;
};

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCrowd_HerdBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_HerdBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Herd Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_HerdConfig HerdConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentState = ECrowd_HerdState::Grazing;

    // ---- Herd Members ----
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    AActor* HerdLeader = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    FVector HerdCentroid = FVector::ZeroVector;

    // ---- Threat Detection ----
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Threat")
    AActor* CurrentThreat = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Threat")
    float ThreatDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Threat")
    float PanicDecayRate = 0.05f;

    // ---- Migration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bMigrationEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FVector> MigrationWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Migration")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float WaypointAcceptanceRadius = 300.0f;

    // ---- LOD Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD0_Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD1_Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD2_Distance = 10000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|LOD")
    int32 CurrentLODLevel = 0;

    // ---- Public Methods ----
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerdMember(AActor* MemberActor, ECrowd_HerdRole Role);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RemoveHerdMember(AActor* MemberActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerFleeResponse(AActor* ThreatActor, float ThreatIntensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerStampede(FVector StampedeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetMigrationWaypoints(const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector ComputeFlockingForce(const FCrowd_HerdMember& Member) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector GetHerdCentroid() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    int32 GetHerdSize() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    float GetAveragePanicLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void ElectNewLeader();

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void UpdateLODLevel(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    ECrowd_HerdState GetCurrentState() const { return CurrentState; }

private:
    void UpdateHerdCentroid();
    void UpdateMemberDistances();
    void ProcessGrazingState(float DeltaTime);
    void ProcessAlertState(float DeltaTime);
    void ProcessFleeingState(float DeltaTime);
    void ProcessStampedeState(float DeltaTime);
    void ProcessMigration(float DeltaTime);
    void DecayPanicLevels(float DeltaTime);
    void ScanForThreats();
    FVector ComputeAlignmentForce(const FCrowd_HerdMember& Member) const;
    FVector ComputeCohesionForce(const FCrowd_HerdMember& Member) const;
    FVector ComputeSeparationForce(const FCrowd_HerdMember& Member) const;

    float TimeSinceLastScan = 0.0f;
    float ScanInterval = 0.5f;
    float TimeSinceLastMigrationCheck = 0.0f;
    float MigrationCheckInterval = 2.0f;
};
