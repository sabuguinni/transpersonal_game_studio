#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Grazing UMETA(DisplayName = "Grazing"),
    Hunting UMETA(DisplayName = "Hunting"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Drinking UMETA(DisplayName = "Drinking"),
    Socializing UMETA(DisplayName = "Socializing")
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Raptor UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Territoriality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float FlightResponse;

    FDinosaurPersonality()
    {
        Aggression = 0.5f;
        Curiosity = 0.3f;
        Sociability = 0.4f;
        Territoriality = 0.6f;
        FlightResponse = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSinceThreatSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastMealTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastDrinkTime;

    FDinosaurMemory()
    {
        LastKnownThreatLocation = FVector::ZeroVector;
        TimeSinceThreatSeen = 0.0f;
        HomeTerritory = FVector::ZeroVector;
        CurrentPatrolIndex = 0;
        LastMealTime = 0.0f;
        LastDrinkTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDinosaurBehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FDinosaurPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FDinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SmellRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float ThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float EnergyLevel;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool CanDetectThreat();

    UFUNCTION(BlueprintCallable, Category = "Detection")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory();

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteSpeciesBehavior();

private:
    void HandleIdleState(float DeltaTime);
    void HandleGrazingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleAggressiveState(float DeltaTime);
    void HandleSleepingState(float DeltaTime);
    void HandleDrinkingState(float DeltaTime);

    void InitializeSpeciesTraits();
    void CreatePatrolRoute();
    void MoveTowardsTarget(const FVector& Target, float Speed);
    bool IsNearTarget(const FVector& Target, float Threshold = 200.0f);
    FVector FindNearestWaterSource();
    FVector FindNearestFoodSource();
    
    float StateTimer;
    float BehaviorUpdateInterval;
};