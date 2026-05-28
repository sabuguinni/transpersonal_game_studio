#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_SurvivalBehaviorSystem.generated.h"

// Forward declarations
class ANPC_SurvivalNPC;
class UNPC_SurvivalBehaviorComponent;

UENUM(BlueprintType)
enum class ENPC_SurvivalNeed : uint8
{
    None = 0,
    Water = 1,
    Food = 2,
    Shelter = 3,
    Safety = 4,
    Rest = 5
};

UENUM(BlueprintType)
enum class ENPC_SurvivalState : uint8
{
    Satisfied = 0,
    Mild = 1,
    Moderate = 2,
    Severe = 3,
    Critical = 4
};

UENUM(BlueprintType)
enum class ENPC_SurvivalBehavior : uint8
{
    Idle = 0,
    Seeking = 1,
    Consuming = 2,
    Resting = 3,
    Fleeing = 4,
    Gathering = 5
};

USTRUCT(BlueprintType)
struct FNPC_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Safety = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 20.0f;

    FNPC_SurvivalStats()
    {
        Hunger = 100.0f;
        Thirst = 100.0f;
        Energy = 100.0f;
        Safety = 100.0f;
        Temperature = 20.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_ResourceLocation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    ENPC_SurvivalNeed ResourceType = ENPC_SurvivalNeed::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float Quality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float DiscoveryTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsAvailable = true;

    FNPC_ResourceLocation()
    {
        Location = FVector::ZeroVector;
        ResourceType = ENPC_SurvivalNeed::None;
        Quality = 1.0f;
        DiscoveryTime = 0.0f;
        bIsAvailable = true;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SurvivalBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SurvivalBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core survival behavior functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ENPC_SurvivalNeed GetMostUrgentNeed() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SeekResource(ENPC_SurvivalNeed NeedType);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeResource(const FNPC_ResourceLocation& Resource);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool FindNearestResource(ENPC_SurvivalNeed NeedType, FNPC_ResourceLocation& OutResource) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddKnownResource(const FNPC_ResourceLocation& Resource);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSurvivalBehavior(ENPC_SurvivalBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ENPC_SurvivalState GetSurvivalState(ENPC_SurvivalNeed NeedType) const;

    // Environmental awareness
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalFactors();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsInSafeArea() const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector GetSafeLocation() const;

    // Behavior state management
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessSurvivalBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldSeekShelter() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldRest() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsInDanger() const;

protected:
    // Survival stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    FNPC_SurvivalStats SurvivalStats;

    // Current behavior state
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    ENPC_SurvivalBehavior CurrentBehavior = ENPC_SurvivalBehavior::Idle;

    // Known resource locations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_ResourceLocation> KnownResources;

    // Current target resource
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    FNPC_ResourceLocation CurrentTarget;

    // Survival parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float HungerDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float ThirstDecayRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float EnergyDecayRate = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float ResourceSearchRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float CriticalNeedThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float SafetyRadius = 1000.0f;

    // Behavior timing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (AllowPrivateAccess = "true"))
    float BehaviorUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (AllowPrivateAccess = "true"))
    float LastBehaviorUpdate = 0.0f;

private:
    // Internal helper functions
    void DecaySurvivalStats(float DeltaTime);
    void UpdateResourceAvailability();
    void EvaluateThreatLevel();
    bool IsResourceAccessible(const FNPC_ResourceLocation& Resource) const;
    void MoveTowardsTarget();
};