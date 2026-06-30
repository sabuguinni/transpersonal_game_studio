#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationTypes.h"
#include "CrowdHerdBehavior.generated.h"

// Herd behavior states for prehistoric herbivore groups
UENUM(BlueprintType)
enum class ECrowd_HerdBehaviorState : uint8
{
    Grazing         UMETA(DisplayName = "Grazing"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Resting         UMETA(DisplayName = "Resting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Migrating       UMETA(DisplayName = "Migrating")
};

// Species types for herd members
UENUM(BlueprintType)
enum class ECrowd_HerbivoreType : uint8
{
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pachycephalosaurus  UMETA(DisplayName = "Pachycephalosaurus")
};

// Data for a single herd member
USTRUCT(BlueprintType)
struct FCrowd_HerdMemberData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerbivoreType Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FearLevel;

    FCrowd_HerdMemberData()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Health(100.f)
        , bIsAlpha(false)
        , Species(ECrowd_HerbivoreType::Triceratops)
        , FearLevel(0.f)
    {}
};

// Herd configuration
USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MinHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationDistance;

    FCrowd_HerdConfig()
        : MinHerdSize(8)
        , MaxHerdSize(30)
        , WanderRadius(2000.f)
        , AlertRadius(1500.f)
        , StampedeSpeed(1200.f)
        , CohesionStrength(0.6f)
        , SeparationDistance(300.f)
    {}
};

/**
 * UCrowd_HerdBehaviorComponent
 * Manages flocking/herd behavior for groups of prehistoric herbivores.
 * Uses Reynolds boids rules: separation, alignment, cohesion.
 * Supports stampede triggers when predators or player are detected.
 */
UCLASS(ClassGroup = "Crowd", meta = (BlueprintSpawnableComponent), DisplayName = "Herd Behavior Component")
class TRANSPERSONALGAME_API UCrowd_HerdBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_HerdBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_HerdConfig HerdConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerbivoreType HerbivoreSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdBehaviorState CurrentState;

    // --- Herd Members ---
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMemberData> HerdMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    int32 HerdSize;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    FVector HerdCentroid;

    // --- Fear / Alert ---
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    float CollectiveFearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeTriggerFear;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(int32 NumMembers, FVector SpawnCenter);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerAlert(FVector ThreatLocation, float ThreatIntensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerStampede(FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void CalmHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector GetHerdCentroid() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    bool IsStampeding() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    float GetCollectiveFear() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    int32 GetHerdSize() const;

private:
    void UpdateBoidsBehavior(float DeltaTime);
    void UpdateHerdCentroid();
    void UpdateFearDecay(float DeltaTime);
    FVector ComputeSeparation(int32 MemberIndex) const;
    FVector ComputeAlignment(int32 MemberIndex) const;
    FVector ComputeCohesion(int32 MemberIndex) const;
    FVector ComputeStampedeForce(int32 MemberIndex) const;

    FVector StampedeDirection;
    float StampedeTimer;
    bool bStampedeActive;
};

/**
 * ACrowd_HerdActor
 * Spawnable actor that represents a herd of prehistoric herbivores.
 * Manages visual representation and behavior for the entire group.
 */
UCLASS(BlueprintType, Blueprintable, DisplayName = "Prehistoric Herd Actor")
class TRANSPERSONALGAME_API ACrowd_HerdActor : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_HerdActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Herd")
    UCrowd_HerdBehaviorComponent* HerdBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerbivoreType DefaultSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 InitialHerdSize;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SpawnHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void OnPredatorDetected(FVector PredatorLocation);
};
