#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPC_AdvancedTerritorialSystem.generated.h"

// Forward declarations
class ANPC_DinosaurAIController;
class UNPC_DinosaurSensorySystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TerritoryBoundary
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    EDinosaurSpecies OwnerSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritorialStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsActiveTerritory;

    FNPC_TerritoryBoundary()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        Height = 500.0f;
        OwnerSpecies = EDinosaurSpecies::TRex;
        TerritorialStrength = 1.0f;
        bIsActiveTerritory = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TerritorialConflict
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    AActor* IntruderActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    AActor* TerritoryOwner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    FVector ConflictLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    float ConflictIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    float ConflictDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
    bool bIsActiveConflict;

    FNPC_TerritorialConflict()
    {
        IntruderActor = nullptr;
        TerritoryOwner = nullptr;
        ConflictLocation = FVector::ZeroVector;
        ConflictIntensity = 0.0f;
        ConflictDuration = 0.0f;
        bIsActiveConflict = false;
    }
};

UENUM(BlueprintType)
enum class ENPC_TerritorialResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Warning         UMETA(DisplayName = "Warning Display"),
    Aggressive      UMETA(DisplayName = "Aggressive Posturing"),
    Attack          UMETA(DisplayName = "Direct Attack"),
    Retreat         UMETA(DisplayName = "Tactical Retreat"),
    Negotiate       UMETA(DisplayName = "Social Negotiation")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AdvancedTerritorialSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AdvancedTerritorialSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void EstablishTerritory(const FVector& CenterLocation, float Radius, float Height);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void ExpandTerritory(float ExpansionRadius);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void ContractTerritory(float ContractionRadius);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    bool IsLocationInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    float GetDistanceFromTerritoryCenter(const FVector& Location) const;

    // Intrusion Detection
    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void DetectIntrusion(AActor* PotentialIntruder);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    ENPC_TerritorialResponse DetermineResponse(AActor* Intruder, float IntrusionSeverity);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void ExecuteTerritorialResponse(ENPC_TerritorialResponse Response, AActor* Target);

    // Conflict Management
    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void InitiateTerritorialConflict(AActor* Intruder, float ConflictIntensity);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void ResolveTerritorialConflict(const FNPC_TerritorialConflict& Conflict, bool bOwnerVictorious);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void UpdateActiveConflicts(float DeltaTime);

    // Territory Sharing and Negotiation
    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    bool CanShareTerritoryWith(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void NegotiateTerritorySharing(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void EstablishTerritoryBoundary(AActor* NeighborActor);

    // Territory Maintenance
    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void PatrolTerritory();

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void MarkTerritoryBoundaries();

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void ReinforceTerritory(float StrengthIncrease);

    // Advanced Territorial Behaviors
    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void AdaptTerritoryToEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void SeasonalTerritoryAdjustment(float SeasonalFactor);

    UFUNCTION(BlueprintCallable, Category = "Territorial System")
    void HandleTerritoryInheritance(AActor* PreviousOwner);

protected:
    // Territory Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Configuration")
    FNPC_TerritoryBoundary TerritoryBoundary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Configuration")
    float TerritorialAggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Configuration")
    float TerritoryMaintenanceInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Configuration")
    bool bAllowTerritorySharing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Configuration")
    float MaxConflictDuration;

    // Active Conflicts
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory State")
    TArray<FNPC_TerritorialConflict> ActiveConflicts;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory State")
    TArray<AActor*> KnownIntruders;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory State")
    TArray<AActor*> TerritoryAllies;

    // Timing and State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory State")
    float LastPatrolTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory State")
    float LastBoundaryMarkingTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory State")
    bool bIsTerritoryEstablished;

private:
    // Internal helper functions
    void UpdateTerritoryPatrol(float DeltaTime);
    void ProcessIntrusionDetection();
    void CalculateOptimalTerritorySize();
    void HandleTerritoryOverlap(AActor* OverlappingActor);
    float CalculateIntrusionSeverity(AActor* Intruder) const;
    bool IsActorThreatening(AActor* Actor) const;
    void BroadcastTerritorialWarning(AActor* Target);
    void ExecuteAggressivePosturing(AActor* Target);
    void InitiateDirectAttack(AActor* Target);
    void PerformTacticalRetreat();
    void AttemptSocialNegotiation(AActor* Target);

    // Component references
    UPROPERTY()
    UNPC_DinosaurSensorySystem* SensorySystem;

    UPROPERTY()
    ANPC_DinosaurAIController* AIController;
};