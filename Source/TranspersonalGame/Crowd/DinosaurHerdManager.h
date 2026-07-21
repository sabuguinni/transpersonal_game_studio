#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinosaurHerdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Resting     UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_HerdSpecies : uint8
{
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Gallimimus      UMETA(DisplayName = "Gallimimus")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 MemberIndex;

    FCrowd_HerdMember()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Health(100.f)
        , bIsLeader(false)
        , MemberIndex(0)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float AlignmentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatDetectionRadius;

    FCrowd_HerdConfig()
        : Species(ECrowd_HerdSpecies::Brachiosaurus)
        , HerdSize(12)
        , CohesionRadius(800.f)
        , SeparationRadius(200.f)
        , AlignmentRadius(600.f)
        , MoveSpeed(400.f)
        , StampedeSpeed(1200.f)
        , ThreatDetectionRadius(2000.f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurHerdManager : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurHerdManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Herd configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_HerdConfig HerdConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentHerdState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector HerdCenterOfMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMember> HerdMembers;

    // Stampede trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bStampedeActive;

    // Boid simulation weights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Boids")
    float LeaderFollowWeight;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(int32 HerdSizeOverride = 0);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void UpdateHerdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerStampede(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SetMigrationTarget(FVector NewTarget);

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    FVector GetHerdCenterOfMass() const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    int32 GetHerdLeaderIndex() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void ApplyBoidRules(int32 MemberIdx, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void CheckForThreats();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TransitionHerdState(ECrowd_HerdState NewState);

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    FString GetHerdStateAsString() const;

private:
    float StampedeTimer;
    float BehaviorTickAccumulator;
    static constexpr float BehaviorTickInterval = 0.1f;
};
