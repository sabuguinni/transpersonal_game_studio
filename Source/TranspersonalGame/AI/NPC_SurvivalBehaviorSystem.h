#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_SurvivalBehaviorSystem.generated.h"

// Forward declarations
class ANPCBaseActor;
class UNPCBehaviorComponent;

UENUM(BlueprintType)
enum class ENPC_SurvivalNeed : uint8
{
    None        UMETA(DisplayName = "None"),
    Water       UMETA(DisplayName = "Water"),
    Food        UMETA(DisplayName = "Food"),
    Shelter     UMETA(DisplayName = "Shelter"),
    Safety      UMETA(DisplayName = "Safety"),
    Social      UMETA(DisplayName = "Social"),
    Rest        UMETA(DisplayName = "Rest"),
    Warmth      UMETA(DisplayName = "Warmth")
};

UENUM(BlueprintType)
enum class ENPC_SurvivalUrgency : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical"),
    Emergency   UMETA(DisplayName = "Emergency")
};

USTRUCT(BlueprintType)
struct FNPC_SurvivalNeedData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ENPC_SurvivalNeed NeedType = ENPC_SurvivalNeed::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float CurrentLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0"))
    float DecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    ENPC_SurvivalUrgency UrgencyLevel = ENPC_SurvivalUrgency::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0"))
    float TimeSinceLastSatisfied = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    bool bIsBeingSatisfied = false;

    FNPC_SurvivalNeedData()
    {
        NeedType = ENPC_SurvivalNeed::None;
        CurrentLevel = 100.0f;
        MaxLevel = 100.0f;
        DecayRate = 1.0f;
        UrgencyLevel = ENPC_SurvivalUrgency::None;
        TimeSinceLastSatisfied = 0.0f;
        bIsBeingSatisfied = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SurvivalResourceLocation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    ENPC_SurvivalNeed ResourceType = ENPC_SurvivalNeed::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Quality = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0.0"))
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0.0"))
    float LastVisitedTime = 0.0f;

    FNPC_SurvivalResourceLocation()
    {
        Location = FVector::ZeroVector;
        ResourceType = ENPC_SurvivalNeed::None;
        Quality = 50.0f;
        Distance = 0.0f;
        bIsOccupied = false;
        LastVisitedTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class TRANSPERSONALGAME_API UNPC_SurvivalBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SurvivalBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core survival needs management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Needs")
    TMap<ENPC_SurvivalNeed, FNPC_SurvivalNeedData> SurvivalNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Resources")
    TArray<FNPC_SurvivalResourceLocation> KnownResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    ENPC_SurvivalNeed CurrentPriorityNeed = ENPC_SurvivalNeed::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    FVector CurrentTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Behavior")
    bool bIsSeekingResource = false;

    // Survival behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Config", meta = (ClampMin = "100.0"))
    float ResourceSearchRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Config", meta = (ClampMin = "0.0"))
    float SurvivalUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Config", meta = (ClampMin = "0.0"))
    float ResourceMemoryDuration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Config")
    bool bEnableAdvancedSurvival = true;

    // Survival behavior functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void InitializeSurvivalNeeds();

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ENPC_SurvivalNeed GetMostUrgentNeed() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SatisfyNeed(ENPC_SurvivalNeed NeedType, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetNeedLevel(ENPC_SurvivalNeed NeedType) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ENPC_SurvivalUrgency GetNeedUrgency(ENPC_SurvivalNeed NeedType) const;

    // Resource management functions
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void DiscoverResource(FVector Location, ENPC_SurvivalNeed ResourceType, float Quality);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    FNPC_SurvivalResourceLocation FindBestResource(ENPC_SurvivalNeed NeedType) const;

    UFUNCTION(BlueprintCallable, Category = "Resources")
    void UpdateResourceMemory(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Resources")
    bool IsResourceStillValid(const FNPC_SurvivalResourceLocation& Resource) const;

    // Behavior decision functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void EvaluateSurvivalPriorities();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetSurvivalTarget(ENPC_SurvivalNeed NeedType, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldSeekResource(ENPC_SurvivalNeed NeedType) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteSurvivalBehavior();

private:
    float LastSurvivalUpdate = 0.0f;
    float LastResourceUpdate = 0.0f;

    void CalculateNeedUrgency(FNPC_SurvivalNeedData& NeedData);
    void UpdateResourceDistances();
    void CleanupOldResources();
};